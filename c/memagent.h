#ifndef MEM_AGENT_H
#define MEM_AGENT_H 1

#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>

#include <strophe.h>

typedef struct {

    char *host;
    int port;

} memcached_server_t;

typedef struct {

    char *name;
    int binding;

    size_t server_allocation;
    size_t server_next;
    memcached_server_t** servers;

} memcached_server_list_t;

typedef void (*agent_add_stat)(void* opaque,
                               const char *k, size_t klen,
                               const char *v, size_t vlen);

typedef struct {

    char *jid;
    char *pass;
    char *host;

    /* These two are for version replies */
    char *software;
    char *version;

    char *save_path;

    void (*new_serverlist)(memcached_server_list_t**);
    void (*get_stats)(void*, agent_add_stat);

} agent_config_t;

typedef struct {

    xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;
    xmpp_log_t *log;

    agent_config_t *conf;

    pthread_t thread;

} agent_handle_t;

bool start_agent(agent_config_t conf);

/* Server list stuff */
void free_server_list(memcached_server_list_t* server_list);

memcached_server_list_t* create_server_list(const char *name, int port);
memcached_server_t* append_server(memcached_server_list_t *in, char* url);

/* Misc */
char* safe_strdup(const char*);

/* Persistence */
memcached_server_list_t** load_server_lists(const char *filename);
bool save_server_lists(memcached_server_list_t** lists, const char *filename);

#endif /* MEM_AGENT_H */
