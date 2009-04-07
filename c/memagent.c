#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "memagent.h"

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
	xmpp_stanza_set_name(reply, "iq");
	xmpp_stanza_set_type(reply, "result");
	xmpp_stanza_set_id(reply, xmpp_stanza_get_id(stanza));
	xmpp_stanza_set_attribute(reply, "to", xmpp_stanza_get_attribute(stanza, "from"));

	query = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(query, "query");
    ns = xmpp_stanza_get_ns(xmpp_stanza_get_children(stanza));
    if (ns) {
        xmpp_stanza_set_ns(query, ns);
    }

	name = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(name, "name");
	xmpp_stanza_add_child(query, name);

	text = xmpp_stanza_new(ctx);
	xmpp_stanza_set_text(text, handle->conf->software);
	xmpp_stanza_add_child(name, text);

	version = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(version, "version");
	xmpp_stanza_add_child(query, version);

	text = xmpp_stanza_new(ctx);
	xmpp_stanza_set_text(text, handle->conf->version);
	xmpp_stanza_add_child(version, text);

	xmpp_stanza_add_child(reply, query);

	xmpp_send(conn, reply);
	xmpp_stanza_release(reply);
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
        /*
        xmpp_handler_add(conn, message_handler, NULL, "message", NULL, ctx);
        */

        /* Send initial <presence/> so that we appear online to contacts */
        pres = xmpp_stanza_new(handle->ctx);
        xmpp_stanza_set_name(pres, "presence");
        xmpp_send(conn, pres);
        xmpp_stanza_release(pres);
    }
    else {
        fprintf(stderr, "DEBUG: disconnected\n");
        xmpp_stop(handle->ctx);
    }
}

void run_agent(agent_handle_t *handle) {
    /* Run forever */
    xmpp_connect_client(handle->conn, NULL, 0, conn_handler, handle);
    xmpp_run(handle->ctx);
    fprintf(stderr, "Bailing\n");
}

agent_config_t* dup_conf(agent_config_t c) {
    agent_config_t *rv = calloc(sizeof(agent_config_t), 1);
    assert(rv);
    assert(c.jid);
    assert(c.pass);
    assert(c.software);
    assert(c.version);
    assert(c.save_path);

    rv->jid = strdup(c.jid);
    rv->pass = strdup(c.pass);
    rv->software = strdup(c.software);
    rv->version = strdup(c.version);
    rv->save_path = strdup(c.save_path);

    assert(rv->jid);
    assert(rv->pass);
    assert(rv->software);
    assert(rv->version);
    assert(rv->save_path);
    return rv;
}

bool start_agent(agent_config_t conf, agent_handle_t* handle) {
    xmpp_initialize();

    handle->conf = dup_conf(conf);

    handle->log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);
    handle->ctx = xmpp_ctx_new(NULL, handle->log);

    handle->conn = xmpp_conn_new(handle->ctx);

    xmpp_conn_set_jid(handle->conn, conf.jid);
    xmpp_conn_set_pass(handle->conn, conf.pass);

    if (pthread_create(&handle->thread, NULL, run_agent, handle) == 0) {
        return true;
    } else {
        perror("pthread_create");
    }

    return false;
}
