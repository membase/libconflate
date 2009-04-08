#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "memagent.h"

char* safe_strdup(const char* in) {
    char *rv = NULL;
    assert(in);
    rv = strdup(in);
    assert(rv);
    return rv;
}

int version_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza,
                    void * const userdata)
{
    xmpp_stanza_t *reply, *query, *name, *version, *text;
    char *buf;
    size_t len;
    agent_handle_t *handle = (agent_handle_t*) userdata;
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
    xmpp_stanza_add_child(query, name);

    text = xmpp_stanza_new(ctx);
    assert(text);
    xmpp_stanza_set_text(text, handle->conf->software);
    xmpp_stanza_add_child(name, text);

    version = xmpp_stanza_new(ctx);
    assert(version);
    xmpp_stanza_set_name(version, "version");
    xmpp_stanza_add_child(query, version);

    text = xmpp_stanza_new(ctx);
    assert(text);
    xmpp_stanza_set_text(text, handle->conf->version);
    xmpp_stanza_add_child(version, text);

    xmpp_stanza_add_child(reply, query);

    xmpp_send(conn, reply);
    xmpp_stanza_release(reply);
    return 1;
}

xmpp_stanza_t* error_unknown_command(const char *cmd, xmpp_conn_t* const conn,
                                     xmpp_stanza_t * const stanza,
                                     void * const userdata)
{
    xmpp_stanza_t *reply, *error, *condition;
    agent_handle_t *handle = (agent_handle_t*) userdata;
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
    xmpp_stanza_add_child(reply, error);
    xmpp_stanza_set_attribute(error, "code", "404");
    xmpp_stanza_set_attribute(error, "type", "modify");

    /* Error condition */
    condition = xmpp_stanza_new(ctx);
    assert(condition);
    xmpp_stanza_set_name(condition, "item-not-found");
    xmpp_stanza_set_ns(condition, "urn:ietf:params:xml:ns:xmpp-stanzas");
    xmpp_stanza_add_child(error, condition);

    return reply;
}

char **get_form_values(xmpp_stanza_t *t) {
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

void free_form_values(char **v) {
    int i = 0;

    for (i = 0; v[i]; i++) {
        free(v[i]);
    }

    free(v);
}

char **get_specific_form_values(xmpp_stanza_t *field, const char *var) {
    char **rv = NULL;

    while (field && strcmp(xmpp_stanza_get_attribute(field, "var"), var) != 0) {
        field = xmpp_stanza_get_next(field);
    }

    if (field) {
        rv = get_form_values(field);
    }

    return rv;
}

memcached_server_list_t* create_server_list(const char *name)
{
    memcached_server_list_t* rv = calloc(sizeof(memcached_server_list_t), 1);
    assert(rv);
    rv->name = safe_strdup(name);
    rv->server_allocation = 8;
    rv->servers = calloc(sizeof(memcached_server_t*), rv->server_allocation);
    return rv;
}

memcached_server_t* create_server_from_url(const char *url)
{
    char *u = safe_strdup(url);
    char *pos = NULL, *cur = NULL;
    memcached_server_t* rv = calloc(sizeof(memcached_server_t), 1);

    assert(rv);

    cur = strtok_r(u, ":", &pos);
    rv->host = safe_strdup(cur);

    cur = strtok_r(NULL, ":", &pos);
    if (cur && strlen(cur) > 0) {
        rv->port = strtol(cur, NULL, 10);
        /* Default to the memcached port */
        if (rv->port < 1 || rv->port > 65535) {
            rv->port = 11211;
        }
    }

    free(u);
    return rv;
}

/* Returns the newly created server object */
memcached_server_t* append_server(memcached_server_list_t *in, char* url)
{
    assert(in);
    assert(url);

    memcached_server_t* rv = create_server_from_url(url);

    if (in->server_next >= in->server_allocation) {
        in->server_allocation <<= 1;
        in->servers = realloc(in->servers,
                              in->server_allocation * sizeof(memcached_server_t*));
        assert(in->servers);
    }

    in->servers[in->server_next++] = rv;
    in->servers[in->server_next] = NULL;

    return rv;
}

void free_server(memcached_server_t* server)
{
    assert(server);
    assert(server->host);

    free(server->host);
    free(server);
}

void free_server_list(memcached_server_list_t* server_list)
{
    int i = 0;

    assert(server_list);
    assert(server_list->name);

    free(server_list->name);
    for (i = 0; server_list->servers[i]; i++) {
        free_server(server_list->servers[i]);
    }
    free(server_list->servers);
    free(server_list);
}

xmpp_stanza_t* process_serverlist(const char *cmd,
                                  xmpp_stanza_t* cmd_stanza,
                                  xmpp_conn_t * const conn,
                                  xmpp_stanza_t * const stanza,
                                  void * const userdata)
{
    xmpp_stanza_t *reply, *req, *x, *fields;
    char *buf;
    size_t len;
    agent_handle_t *handle = (agent_handle_t*) userdata;
    xmpp_ctx_t *ctx = handle->ctx;
    char **pools;
    int i = 0, num_lists = 0;
    memcached_server_list_t** lists;

    fprintf(stderr, "Processing a serverlist.\n");

    x = xmpp_stanza_get_child_by_name(cmd_stanza, "x");
    assert(x);

    fields = xmpp_stanza_get_child_by_name(x, "field");
    assert(fields);

    pools = get_specific_form_values(fields, "-pools-");

    /* Count the number of lists */
    for (num_lists = 0; pools[num_lists]; num_lists++);

    lists = calloc(sizeof(memcached_server_list_t*), num_lists+1);

    for (i = 0; pools[i]; i++) {
        int j = 0;
        memcached_server_list_t* slist = create_server_list(pools[i]);
        char **urls = get_specific_form_values(fields, pools[i]);
        for (j = 0; urls[j]; j++) {
            append_server(slist, urls[j]);
        }
        free_form_values(urls);

        lists[--num_lists] = slist;
    }

    free_form_values(pools);

    handle->conf->new_serverlist(lists);

    for (i = 0; lists[i]; i++) {
        free_server_list(lists[i]);
    }
    free(lists);

    reply = xmpp_stanza_new(ctx);
    assert(reply);

    xmpp_stanza_set_name(reply, "iq");
    xmpp_stanza_set_type(reply, "result");
    xmpp_stanza_set_id(reply, xmpp_stanza_get_id(stanza));
    xmpp_stanza_set_attribute(reply, "to",
                              xmpp_stanza_get_attribute(stanza, "from"));

    return reply;
}

int command_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza,
                    void * const userdata)
{
    xmpp_stanza_t *reply, *req;
    char *buf;
    size_t len;
    agent_handle_t *handle = (agent_handle_t*) userdata;
    xmpp_ctx_t *ctx = handle->ctx;
    char *cmd;

    fprintf(stderr, "Received a command from %s\n",
            xmpp_stanza_get_attribute(stanza, "from"));

    /* Figure out what the command is */
    req = xmpp_stanza_get_children(stanza);
    assert(req);
    cmd = xmpp_stanza_get_attribute(req, "node");
    assert(cmd);

    fprintf(stderr, "Command:  %s\n", cmd);

    if (strcmp(cmd, "serverlist") == 0) {
        reply = process_serverlist(cmd, req, conn, stanza, handle);
    } else {
        reply = error_unknown_command(cmd, conn, stanza, handle);
    }

    if (reply) {
        xmpp_send(conn, reply);
        xmpp_stanza_release(reply);
    }

    return 1;
}

void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
                  const int error, xmpp_stream_error_t * const stream_error,
                  void * const userdata)
{
    agent_handle_t *handle = (agent_handle_t *)userdata;

    if (status == XMPP_CONN_CONNECT) {
        xmpp_stanza_t* pres;
        fprintf(stderr, "DEBUG: connected\n");
        xmpp_handler_add(conn, version_handler, "jabber:iq:version", "iq", NULL, handle);
        xmpp_handler_add(conn, command_handler, "http://jabber.org/protocol/commands",
                         "iq", NULL, handle);
        /*
          xmpp_handler_add(conn, message_handler, NULL, "message", NULL, ctx);
        */

        /* Send initial <presence/> so that we appear online to contacts */
        pres = xmpp_stanza_new(handle->ctx);
        assert(pres);
        xmpp_stanza_set_name(pres, "presence");
        xmpp_send(conn, pres);
        xmpp_stanza_release(pres);
    }
    else {
        fprintf(stderr, "DEBUG: disconnected\n");
        xmpp_stop(handle->ctx);
    }
}

void* run_agent(void *arg) {
    agent_handle_t* handle = (agent_handle_t*)arg;
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

agent_config_t* dup_conf(agent_config_t c) {
    agent_config_t *rv = calloc(sizeof(agent_config_t), 1);
    assert(rv);

    rv->jid = safe_strdup(c.jid);
    rv->pass = safe_strdup(c.pass);
    if (c.host) {
        rv->host = safe_strdup(c.host);
    }
    rv->software = safe_strdup(c.software);
    rv->version = safe_strdup(c.version);
    rv->save_path = safe_strdup(c.save_path);
    rv->new_serverlist = c.new_serverlist;

    return rv;
}

bool start_agent(agent_config_t conf, agent_handle_t* handle) {
    xmpp_initialize();

    handle->conf = dup_conf(conf);

    handle->log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);

    if (pthread_create(&handle->thread, NULL, run_agent, handle) == 0) {
        return true;
    } else {
        perror("pthread_create");
    }

    return false;
}
