#ifndef MEM_AGENT_H
#define MEM_AGENT_H 1

#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>

#include <strophe.h>

typedef struct {

    char *jid;
    char *pass;

    /* These two are for version replies */
    char *software;
    char *version;

    char *save_path;

} agent_config_t;

typedef struct {

    xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;
    xmpp_log_t *log;

    agent_config_t *conf;

    pthread_t thread;

} agent_handle_t;

bool start_agent(agent_config_t conf, agent_handle_t* handle);

#endif /* MEM_AGENT_H */
