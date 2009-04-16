#ifndef MEM_AGENT_H
#define MEM_AGENT_H 1

#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>

#include <strophe.h>

/**
 * \defgroup Core
 * @{
 */

/**
 * Callback for agent stats.
 *
 * The opaque argument will be passed to the client and must be passed
 * back in as-is.  The key and value parameters represent the stat
 * name and value that should be reported.
 */
typedef void (*agent_add_stat)(void* opaque, const char *k, const char *v);

/**
 * A linked list of keys each which may have zero or more values.
 */
typedef struct kvpair {
    /**
     * The key in this kv pair.
     */
    char*  key;
    /**
     * A NULL-terminated list of values.
     */
    char** values;

    /** \private */
    int    allocated_values;
    /** \private */
    int    used_values;

    /**
     * The next kv pair in this list.  NULL if this is the last.
     */
    struct kvpair* next;
} kvpair_t;

/**
 * Configuration for an agent.
 */
typedef struct {

    /** The XMPP JID (typically excludes a resource definition) */
    char *jid;
    /** The XMPP Password */
    char *pass;
    /**
     * The XMPP server address.
     *
     * This is optional -- setting this to NULL will allow for normal
     * XMPP SRV lookups to locate the server.
     */
    char *host;

    /**
     * Name of the software using this agent.
     *
     * For example:  awesomeproxy
     */
    char *software;
    /**
     * Version number of the software running this agent.
     *
     * For example:  1.3.3-8-gee0c3d5
     */
    char *version;

    /**
     * Path to persist configuration for faster/more reliable restarts.
     */
    char *save_path;

    /**
     * User defined data to be passed to callbacks.
     */
    void *userdata;
    /**
     * Callback issued when a new configuration is to be activated.
     *
     * This callback will receive the caller's userdata and a kvpair_t
     * of all of the keys and values received when things changed.
     *
     * The new config *may* be the same as the previous config.  It's
     * up to the client to detect and decide what to do in this case.
     */
    void (*new_config)(void*, kvpair_t*);
    /**
     * Callback issued when the agent wants to report stats.
     *
     * The client is expected to call the agent_add_stat callback once
     * for every stat it wishes to report along with the given opaque,
     * followed by one invocation with two NULLs.
     */
    void (*get_stats)(void*, void*, agent_add_stat);

} agent_config_t;

/**
 * @}
 */

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

/**
 * \defgroup Core
 * @{
 */

/**
 * The main entry point for starting a configuration agent.
 *
 * @return true if the agent was able to properly initialize itself
 */
bool start_agent(agent_config_t conf);

/**
 * @}
 */

/* Misc */
char* safe_strdup(const char*);
void free_string_list(char **);

/**
 * \defgroup Persistence
 * @{
 */

/**
 * Load the key/value pairs from the file at the given path.
 *
 * @return NULL if the config could not be loaded for any reason.
 */
kvpair_t* load_kvpairs(const char *filename);

/**
 * Save a config at the given path.
 *
 * @return false if the configuration could not be saved for any reason
 */
bool save_kvpairs(kvpair_t* pairs, const char *filename);

/**
 * @}
 */

#endif /* MEM_AGENT_H */
