#ifndef MEM_AGENT_H
#define MEM_AGENT_H 1

#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>

#include <strophe.h>

typedef void (*agent_add_stat)(void* opaque, const char *k, const char *v);

typedef struct kvpair {
    char*  key;
    char** values;

    /* For internal use */
    int    allocated_values;
    int    used_values;

    struct kvpair* next;
} kvpair_t;

typedef struct {

    char *jid;
    char *pass;
    char *host;

    /* These two are for version replies */
    char *software;
    char *version;

    char *save_path;

    void *userdata;
    void (*new_config)(void*, kvpair_t*);
    void (*get_stats)(void*, void*, agent_add_stat);

} agent_config_t;

typedef struct {

    xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;
    xmpp_log_t *log;

    agent_config_t *conf;

    pthread_t thread;

} agent_handle_t;

/* Key/value pair management */

kvpair_t* mk_kvpair(const char* k, char** v);
void add_kvpair_value(kvpair_t* kvpair, const char* value);
void free_kvpair(kvpair_t* pair);

bool start_agent(agent_config_t conf);

/* Misc */
char* safe_strdup(const char*);
void free_string_list(char **);

/* Persistence */
kvpair_t* load_kvpairs(const char *filename);
bool save_kvpairs(kvpair_t* pairs, const char *filename);

#endif /* MEM_AGENT_H */
