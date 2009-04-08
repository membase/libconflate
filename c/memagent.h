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

typedef struct {

    char *host;
    int port;

} memcached_server_t;

typedef struct {

    char *name;

    size_t server_allocation;
    size_t server_next;
    memcached_server_t** servers;

} memcached_server_list_t;

bool start_agent(agent_config_t conf, agent_handle_t* handle);

/* Server list stuff */
void free_server(memcached_server_t* server);
void free_server_list(memcached_server_list_t* server_list);

#endif /* MEM_AGENT_H */
