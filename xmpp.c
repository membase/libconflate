#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "conflate.h"

static void add_and_release(xmpp_stanza_t* parent, xmpp_stanza_t* child)
{
    xmpp_stanza_add_child(parent, child);
    xmpp_stanza_release(child);
}

/* Copy an attribute from one stanza to another iff it exists in the
   src. */
static void copy_attr(xmpp_ctx_t *ctx,
                      xmpp_stanza_t* src, xmpp_stanza_t* dest,
                      const char* attr)
{
    assert(src);
    assert(dest);
    assert(attr);

    char *val = xmpp_stanza_get_attribute(src, attr);
    if (val) {
        xmpp_stanza_set_attribute(dest, attr, val);
    }
}

/* Create an IQ response */
static xmpp_stanza_t* create_reply(xmpp_ctx_t* ctx, xmpp_stanza_t* stanza)
{
    xmpp_stanza_t* reply = xmpp_stanza_new(ctx);

    assert(reply);

    xmpp_stanza_set_name(reply, "iq");
    xmpp_stanza_set_type(reply, "result");
    xmpp_stanza_set_id(reply, xmpp_stanza_get_id(stanza));
    xmpp_stanza_set_attribute(reply, "to",
                              xmpp_stanza_get_attribute(stanza, "from"));

    return reply;
}

/* Create an XMPP command response */
static xmpp_stanza_t* create_cmd_response(xmpp_ctx_t* ctx,
                                          xmpp_stanza_t* cmd_stanza)
{
    xmpp_stanza_t* cmd_res = xmpp_stanza_new(ctx);
    assert(cmd_res);

    xmpp_stanza_set_name(cmd_res, "command");
    copy_attr(ctx, cmd_stanza, cmd_res, "xmlns");
    copy_attr(ctx, cmd_stanza, cmd_res, "node");
    copy_attr(ctx, cmd_stanza, cmd_res, "sessionid");
    xmpp_stanza_set_attribute(cmd_res, "status", "completed");

    return cmd_res;
}

static int version_handler(xmpp_conn_t * const conn,
                           xmpp_stanza_t * const stanza,
                           void * const userdata)
{
    xmpp_stanza_t *reply, *query, *name, *version, *text;
    conflate_handle_t *handle = (conflate_handle_t*) userdata;
    xmpp_ctx_t *ctx = handle->ctx;
    char *ns;

    printf("Received version request from %s\n", xmpp_stanza_get_attribute(stanza, "from"));

    reply = xmpp_stanza_new(ctx);
    assert(reply);
    xmpp_stanza_set_name(reply, "iq");
    xmpp_stanza_set_type(reply, "result");
    xmpp_stanza_set_id(reply, xmpp_stanza_get_id(stanza));
    xmpp_stanza_set_attribute(reply, "to", xmpp_stanza_get_attribute(stanza, "from"));

    query = xmpp_stanza_new(ctx);
    assert(query);
    xmpp_stanza_set_name(query, "query");
    ns = xmpp_stanza_get_ns(xmpp_stanza_get_children(stanza));
    if (ns) {
        xmpp_stanza_set_ns(query, ns);
    }

    name = xmpp_stanza_new(ctx);
    assert(name);
    xmpp_stanza_set_name(name, "name");
    add_and_release(query, name);

    text = xmpp_stanza_new(ctx);
    assert(text);
    xmpp_stanza_set_text(text, handle->conf->software);
    add_and_release(name, text);

    version = xmpp_stanza_new(ctx);
    assert(version);
    xmpp_stanza_set_name(version, "version");
    add_and_release(query, version);

    text = xmpp_stanza_new(ctx);
    assert(text);
    xmpp_stanza_set_text(text, handle->conf->version);
    add_and_release(version, text);

    add_and_release(reply, query);

    xmpp_send(conn, reply);
    xmpp_stanza_release(reply);
    return 1;
}

static xmpp_stanza_t* error_unknown_command(const char *cmd,
                                            xmpp_conn_t* const conn,
                                            xmpp_stanza_t * const stanza,
                                            void * const userdata,
                                            bool direct)
{
    xmpp_stanza_t *reply, *error, *condition;
    conflate_handle_t *handle = (conflate_handle_t*) userdata;
    xmpp_ctx_t *ctx = handle->ctx;

    fprintf(stderr, "Unknown command:  %s\n", cmd);

    reply = xmpp_stanza_new(ctx);
    assert(reply);
    xmpp_stanza_set_name(reply, "iq");
    xmpp_stanza_set_type(reply, "error");
    xmpp_stanza_set_id(reply, xmpp_stanza_get_id(stanza));
    xmpp_stanza_set_attribute(reply, "to", xmpp_stanza_get_attribute(stanza, "from"));

    /* Error element */
    error = xmpp_stanza_new(ctx);
    assert(error);
    xmpp_stanza_set_name(error, "error");
    add_and_release(reply, error);
    xmpp_stanza_set_attribute(error, "code", "404");
    xmpp_stanza_set_attribute(error, "type", "modify");

    /* Error condition */
    condition = xmpp_stanza_new(ctx);
    assert(condition);
    xmpp_stanza_set_name(condition, "item-not-found");
    xmpp_stanza_set_ns(condition, "urn:ietf:params:xml:ns:xmpp-stanzas");
    add_and_release(error, condition);

    return reply;
}

static char **get_form_values(xmpp_stanza_t *t) {
    xmpp_stanza_t *current = NULL;
    int i = 0, allocated = 8;
    char **rv = calloc(allocated, sizeof(char*));
    assert(rv);

    current = xmpp_stanza_get_child_by_name(t, "value");
    while (current) {
        xmpp_stanza_t *val = xmpp_stanza_get_children(current);
        /* xmpp_stanza_get_children allocates */
        char *v = xmpp_stanza_get_text(val);

        if (i + 1 >= allocated) {
            int new_allocated = allocated << 1;
            rv = realloc(rv, sizeof(char*) * new_allocated);
            assert(rv);
        }

        rv[i++] = v;

        current = xmpp_stanza_get_next(current);
    }

    /* List terminator */
    rv[i] = NULL;

    return rv;
}

static char **get_specific_form_values(xmpp_stanza_t *field, const char *var) {
    char **rv = NULL;

    while (field && strcmp(xmpp_stanza_get_attribute(field, "var"), var) != 0) {
        field = xmpp_stanza_get_next(field);
    }

    if (field) {
        rv = get_form_values(field);
    }

    return rv;
}

static kvpair_t *grok_form(xmpp_stanza_t *fields)
{
    kvpair_t *rv = NULL;

    /* walk the field peers and grab their values and stuff */
    while (fields) {
        if (xmpp_stanza_is_tag(fields)) {
            char* k = xmpp_stanza_get_attribute(fields, "var");
            char **vals = get_specific_form_values(fields, k);
            kvpair_t* thispair = mk_kvpair(k, vals);
            free_string_list(vals);
            thispair->next = rv;
            rv = thispair;
        }
        fields = xmpp_stanza_get_next(fields);
    }

    return rv;
}

static xmpp_stanza_t* process_serverlist(const char *cmd,
                                         xmpp_stanza_t* cmd_stanza,
                                         xmpp_conn_t * const conn,
                                         xmpp_stanza_t * const stanza,
                                         void * const userdata,
                                         bool direct)
{
    xmpp_stanza_t *reply = NULL, *x = NULL, *fields = NULL;
    conflate_handle_t *handle = (conflate_handle_t*) userdata;
    xmpp_ctx_t *ctx = handle->ctx;
    kvpair_t* conf = NULL;

    fprintf(stderr, "Processing a serverlist.\n");

    x = xmpp_stanza_get_child_by_name(cmd_stanza, "x");
    assert(x);

    fields = xmpp_stanza_get_child_by_name(x, "field");
    assert(fields);

    conf = grok_form(fields);

    /* Persist the config lists */
    if (!save_kvpairs(conf, handle->conf->save_path)) {
        fprintf(stderr, "Can not safe config to %s\n",
                handle->conf->save_path);
    }

    /* Send the config to the callback */
    handle->conf->new_config(handle->conf->userdata, conf);

    free_kvpair(conf);

    if (direct) {
        reply = create_reply(ctx, stanza);
    }

    return reply;
}

struct stat_context {
    xmpp_conn_t*   conn;
    xmpp_ctx_t*    ctx;
    xmpp_stanza_t* reply;
    xmpp_stanza_t* container;
    bool           complete;
};

static void stat_adder(void* opaque,
                       const char* key, const char* val)
{
    struct stat_context* scontext = (struct stat_context*)opaque;

    if (key) {
        xmpp_stanza_t* field = xmpp_stanza_new(scontext->ctx);
        xmpp_stanza_t* value = xmpp_stanza_new(scontext->ctx);
        xmpp_stanza_t* text = xmpp_stanza_new(scontext->ctx);

        assert(field);
        assert(value);
        assert(text);

        xmpp_stanza_set_name(field, "field");
        xmpp_stanza_set_attribute(field, "var", key);
        add_and_release(scontext->container, field);

        xmpp_stanza_set_name(value, "value");
        add_and_release(field, value);

        xmpp_stanza_set_text(text, val);
        add_and_release(value, text);
    } else {
        scontext->complete = true;
    }
}

static xmpp_stanza_t* process_stats(const char *cmd,
                                    xmpp_stanza_t* cmd_stanza,
                                    xmpp_conn_t * const conn,
                                    xmpp_stanza_t * const stanza,
                                    void * const userdata,
                                    bool direct)
{
    xmpp_stanza_t *cmd_res = NULL;
    conflate_handle_t *handle = (conflate_handle_t*) userdata;
    xmpp_ctx_t *ctx = handle->ctx;

    /* Only direct stat requests are handled. */
    assert(direct);

    struct stat_context scontext = { .conn = conn,
                                     .ctx = ctx,
                                     .reply = NULL,
                                     .container = xmpp_stanza_new(ctx),
                                     .complete = false};

    assert(scontext.container);

    scontext.reply = create_reply(ctx, stanza);
    cmd_res = create_cmd_response(ctx, cmd_stanza);

    add_and_release(scontext.reply, cmd_res);

    /* X data in the command response */
    xmpp_stanza_set_name(scontext.container, "x");
    xmpp_stanza_set_attribute(scontext.container, "xmlns", "jabber:x:data");
    xmpp_stanza_set_type(scontext.container, "result");
    add_and_release(cmd_res, scontext.container);

    handle->conf->get_stats(handle->conf->userdata, &scontext, stat_adder);

    assert(scontext.complete);

    return scontext.reply;
}

static xmpp_stanza_t* process_reset_stats(const char *cmd,
                                          xmpp_stanza_t* cmd_stanza,
                                          xmpp_conn_t * const conn,
                                          xmpp_stanza_t * const stanza,
                                          void * const userdata,
                                          bool direct)
{
    conflate_handle_t *handle = (conflate_handle_t*) userdata;
    xmpp_ctx_t *ctx = handle->ctx;

    /* Only direct stat requests are handled. */
    assert(direct);

    handle->conf->reset_stats(handle->conf->userdata);

    xmpp_stanza_t* cmd_res = xmpp_stanza_new(ctx);

    assert(cmd_res);

    xmpp_stanza_t* reply = create_reply(ctx, stanza);

    add_and_release(reply, create_cmd_response(ctx, cmd_stanza));

    return reply;
}

static xmpp_stanza_t* command_dispatch(xmpp_conn_t * const conn,
                                       xmpp_stanza_t * const stanza,
                                       void * const userdata,
                                       const char* cmd,
                                       xmpp_stanza_t *req,
                                       bool direct)
{
    conflate_handle_t *handle = (conflate_handle_t*) userdata;
    xmpp_stanza_t *reply = NULL;

    if (strcmp(cmd, "serverlist") == 0) {
        reply = process_serverlist(cmd, req, conn, stanza, handle, direct);
    } else if (strcmp(cmd, "client_stats") == 0) {
        reply = process_stats(cmd, req, conn, stanza, handle, direct);
    } else if (strcmp(cmd, "reset_stats") == 0) {
        reply = process_reset_stats(cmd, req, conn, stanza, handle, direct);
    } else {
        reply = error_unknown_command(cmd, conn, stanza, handle, direct);
    }

    return reply;
}

static int command_handler(xmpp_conn_t * const conn,
                           xmpp_stanza_t * const stanza,
                           void * const userdata)
{
    xmpp_stanza_t *reply = NULL, *req = NULL;
    char *cmd = NULL;

    /* Figure out what the command is */
    req = xmpp_stanza_get_child_by_name(stanza, "command");
    assert(req);
    assert(strcmp(xmpp_stanza_get_name(req), "command") == 0);
    cmd = xmpp_stanza_get_attribute(req, "node");
    assert(cmd);

    fprintf(stderr, "Command:  %s\n", cmd);

    reply = command_dispatch(conn, stanza, userdata, cmd, req, true);

    if (reply) {
        xmpp_send(conn, reply);
        xmpp_stanza_release(reply);
    }

    return 1;
}

static int keepalive_handler(xmpp_conn_t * const conn, void * const userdata)
{
    xmpp_send_raw(conn, " ", 1);
    return 1;
}

static void add_disco_item(xmpp_ctx_t* ctx, xmpp_stanza_t* query,
                           const char* jid, char* node, char* name)
{
    xmpp_stanza_t* item = xmpp_stanza_new(ctx);
    assert(item);
    assert(ctx);
    assert(query);
    assert(jid);
    assert(node);
    assert(name);

    xmpp_stanza_set_name(item, "item");
    xmpp_stanza_set_attribute(item, "jid", jid);
    xmpp_stanza_set_attribute(item, "node", node);
    xmpp_stanza_set_attribute(item, "name", name);

    add_and_release(query, item);
}

static int disco_items_handler(xmpp_conn_t * const conn,
                               xmpp_stanza_t * const stanza,
                               void * const userdata)
{
    xmpp_stanza_t *reply, *query;
    const char* myjid = xmpp_conn_get_jid(conn);
    conflate_handle_t *handle = (conflate_handle_t*) userdata;

    assert(conn);
    assert(myjid);
    assert(stanza);
    assert(userdata);

    reply = xmpp_stanza_new(handle->ctx);
    assert(reply);
    xmpp_stanza_set_name(reply, "iq");
    xmpp_stanza_set_type(reply, "result");
    xmpp_stanza_set_id(reply, xmpp_stanza_get_id(stanza));
    xmpp_stanza_set_attribute(reply, "to",
                              xmpp_stanza_get_attribute(stanza, "from"));
    xmpp_stanza_set_attribute(reply, "from", myjid);

    query = xmpp_stanza_new(handle->ctx);
    assert(query);
    xmpp_stanza_set_name(query, "query");
    xmpp_stanza_set_attribute(query, "xmlns", XMPP_NS_DISCO_ITEMS);
    xmpp_stanza_set_attribute(query, "node", "http://jabber.org/protocol/commands");
    add_and_release(reply, query);

    add_disco_item(handle->ctx, query, myjid,
                   "client_stats", "Get the client stats");
    add_disco_item(handle->ctx, query, myjid,
                   "reset_stats", "Reset the client stats");

    xmpp_send(conn, reply);
    xmpp_stanza_release(reply);

    return 1;
}

static int message_handler(xmpp_conn_t * const conn,
                           xmpp_stanza_t * const stanza,
                           void * const userdata)
{
    xmpp_stanza_t* event = NULL, *items = NULL, *item = NULL,
        *command = NULL, *reply = NULL;
    fprintf(stderr, "Got a message from %s\n",
            xmpp_stanza_get_attribute(stanza, "from"));

    event = xmpp_stanza_get_child_by_name(stanza, "event");
    assert(event);
    items = xmpp_stanza_get_child_by_name(event, "items");
    assert(items);
    item = xmpp_stanza_get_child_by_name(items, "item");

    if (item) {
        command = xmpp_stanza_get_child_by_name(item, "command");
        assert(command);

        fprintf(stderr, "Pubsub command:  %s\n",
                xmpp_stanza_get_attribute(command, "command"));

        reply = command_dispatch(conn, stanza, userdata,
                                 xmpp_stanza_get_attribute(command, "command"),
                                 command, false);

        if (reply) {
            xmpp_stanza_release(reply);
        }
    } else {
        fprintf(stderr, "Received pubsub event with no items.\n");
    }

    return 1;
}

static void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
                         const int error, xmpp_stream_error_t * const stream_error,
                         void * const userdata)
{
    conflate_handle_t *handle = (conflate_handle_t *)userdata;

    if (status == XMPP_CONN_CONNECT) {
        xmpp_stanza_t* pres = NULL, *priority = NULL, *pri_text = NULL;
        fprintf(stderr, "DEBUG: connected\n");
        xmpp_handler_add(conn, version_handler, "jabber:iq:version", "iq", NULL, handle);
        xmpp_handler_add(conn, command_handler, "http://jabber.org/protocol/commands",
                         "iq", NULL, handle);
        xmpp_handler_add(conn, disco_items_handler,
                         "http://jabber.org/protocol/disco#items", "iq", NULL, handle);
        xmpp_handler_add(conn, message_handler, NULL, "message", NULL, handle);
        xmpp_timed_handler_add(conn, keepalive_handler, 60000, handle);

        /* Send initial <presence/> so that we appear online to contacts */
        pres = xmpp_stanza_new(handle->ctx);
        assert(pres);
        xmpp_stanza_set_name(pres, "presence");

        priority = xmpp_stanza_new(handle->ctx);
        assert(priority);
        xmpp_stanza_set_name(priority, "priority");
        add_and_release(pres, priority);

        pri_text = xmpp_stanza_new(handle->ctx);
        assert(pri_text);
        xmpp_stanza_set_text(pri_text, "5");
        add_and_release(priority, pri_text);

        xmpp_send(conn, pres);
        xmpp_stanza_release(pres);
    }
    else {
        fprintf(stderr, "DEBUG: disconnected\n");
        xmpp_stop(handle->ctx);
    }
}

static void* run_conflate(void *arg) {
    conflate_handle_t* handle = (conflate_handle_t*)arg;

    /* Before connecting and all that, load the stored config */
    kvpair_t* conf = load_kvpairs(handle->conf->save_path);
    if (conf) {
        handle->conf->new_config(handle->conf->userdata, conf);
        free_kvpair(conf);
    }

    /* Run forever */
    for (;;) {
        handle->ctx = xmpp_ctx_new(NULL, handle->log);
        assert(handle->ctx);

        handle->conn = xmpp_conn_new(handle->ctx);
        assert(handle->conn);

        xmpp_conn_set_jid(handle->conn, handle->conf->jid);
        xmpp_conn_set_pass(handle->conn, handle->conf->pass);

        xmpp_connect_client(handle->conn, handle->conf->host, 0,
                            conn_handler, handle);
        xmpp_run(handle->ctx);
        fprintf(stderr, "xmpp_run exited...\n");

        xmpp_conn_release(handle->conn);
        xmpp_ctx_free(handle->ctx);

        sleep(5);
        fprintf(stderr, "Reconnecting...\n");
    }
    fprintf(stderr, "Bailing\n");
    return NULL;
}

conflate_config_t* dup_conf(conflate_config_t c) {
    conflate_config_t *rv = calloc(sizeof(conflate_config_t), 1);
    assert(rv);

    rv->jid = safe_strdup(c.jid);
    rv->pass = safe_strdup(c.pass);
    if (c.host) {
        rv->host = safe_strdup(c.host);
    }
    rv->software = safe_strdup(c.software);
    rv->version = safe_strdup(c.version);
    rv->save_path = safe_strdup(c.save_path);
    rv->userdata = c.userdata;
    rv->new_config = c.new_config;
    rv->get_stats = c.get_stats;
    rv->reset_stats = c.reset_stats;
    rv->ping_test = c.ping_test;

    return rv;
}

bool start_conflate(conflate_config_t conf) {
    conflate_handle_t *handle = calloc(1, sizeof(conflate_handle_t));
    assert(handle);

    xmpp_initialize();

    handle->conf = dup_conf(conf);

    handle->log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);

    if (pthread_create(&handle->thread, NULL, run_conflate, handle) == 0) {
        return true;
    } else {
        perror("pthread_create");
    }

    return false;
}
