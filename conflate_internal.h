/* Internal data structures that should be opaque to outside viewers */

struct _conflate_handle {

    xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;

    conflate_config_t *conf;

    pthread_t thread;
};

void conflate_init_commands(void);

