#include <stdio.h>
#include <string.h>

#include "memagent.h"

void run_agent(agent_handle_t *handle) {
    /* Run forever */
    xmpp_run(handle->ctx);
}

void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status,
                  const int error, xmpp_stream_error_t * const stream_error,
                  void * const userdata)
{
    agent_handle_t *handle = (agent_handle_t *)userdata;

    if (status == XMPP_CONN_CONNECT) {
        xmpp_stanza_t* pres;
        fprintf(stderr, "DEBUG: connected\n");
        /*
        xmpp_handler_add(conn, version_handler, "jabber:iq:version", "iq", NULL, ctx);
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

bool start_agent(agent_config_t conf, agent_handle_t* handle) {
    xmpp_initialize();

    handle->log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);
    handle->ctx = xmpp_ctx_new(NULL, handle->log);

    handle->conn = xmpp_conn_new(handle->ctx);

    xmpp_conn_set_jid(handle->conn, conf.jid);
    xmpp_conn_set_pass(handle->conn, conf.pass);

    xmpp_connect_client(handle->conn, NULL, 0, conn_handler, handle);

    if (pthread_create(&handle->thread, NULL, run_agent, handle) == 0) {
        return true;
    } else {
        perror("pthread_create");
    }

    return false;
}
