/* Internal data structures that should be opaque to outside viewers */
#include <strophe.h>

struct _conflate_handle {

    xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;

    conflate_config_t *conf;

	alarm_queue_t *alarms;

    pthread_t thread;
};

void conflate_init_commands(void);

