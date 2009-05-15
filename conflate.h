#ifndef CONFLATE_H
#define CONFLATE_H 1

#include <stdbool.h>
#include <sys/types.h>
#include <pthread.h>

#include <strophe.h>

/*! \mainpage libconflate
 *
 * \section intro_sec Introduction
 *
 * libconflate is a library that helps you configure large
 * installations of applications easily.
 *
 * It's designed to be easy to use, safe, and autonomous while still
 * leveraging centralized resources to allow you to have your clusters
 * easily adapt to change.
 *
 * \section docs_sec API Documentation
 *
 * Jump right into <a href="modules.html">the modules docs</a> to get started.
 */

/**
 * \defgroup Core
 */

/**
 * \defgroup kvpairs
 * @{
 */

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
 * Create a kvpair_t.
 *
 * When building out a kvpair chain, one would presumably set the
 * `next' param to the first item in your existing chain.
 *
 * @param k the key for this kvpair
 * @param v (optional) the list of values for this key.
 * @return a newly allocated kvpair_t
 */
kvpair_t* mk_kvpair(const char* k, char** v)
    __attribute__ ((warn_unused_result, nonnull (1)));

/**
 * Add a value to a kvpair_t.
 *
 * @param kvpair the current kvpair that needs a new value
 * @param value the new value
 */
void add_kvpair_value(kvpair_t* kvpair, const char* value)
    __attribute__ ((nonnull (1, 2)));

/**
 * Find a kvpair with the given key.
 *
 * @param pair start of a pair chain
 * @param key the desired key
 *
 * @return the pair with the given key, or NULL if no such pair is found
 */
kvpair_t* find_kvpair(kvpair_t* pair, const char* key)
    __attribute__ ((warn_unused_result, nonnull (2)));

/**
 * Copy a chain of kvpairs.
 *
 * @param pair the pair to duplicate (recursively)
 *
 * @return a complete deep copy of the kvpair structure
 */
kvpair_t *dup_kvpair(kvpair_t *pair)
    __attribute__ ((warn_unused_result, nonnull (1)));

/**
 * Free a chain of kvpairs.
 *
 * @param pair the pair to free (recursively)
 */
void free_kvpair(kvpair_t* pair)
    __attribute__ ((nonnull (1)));

/**
 * @}
 */

/**
 * \defgroup Core
 * @{
 */

/**
 * Callback for conflate stats.
 *
 * The key and value parameters represent the stat name and value that
 * should be reported.
 *
 * @param opaque value given in the stat callback
 * @param k a stat key (may not be NULL)
 * @param v a stat value (may not be NULL)
 */
typedef void (*conflate_add_stat)(void* opaque, const char *k, const char *v);


/**
 * Callback for conflate ping reports.
 *
 * When performing a ping test, results are given back in named sets,
 * one for each input under test.
 *
 * @param opaque value given in the ping report callback
 * @param set a key indicating which of the calling sets is being reported
 * @param pair detailed information about this set's test
 */
typedef void (*conflate_add_ping_report)(void* opaque,
                                         const char* set,
                                         const kvpair_t* pair);

/**
 * Configuration for a conflatee.
 */
typedef struct {

    /** The XMPP JID (typically excludes a resource definition) */
    char *jid;
    /** The XMPP Password */
    char *pass;
    /**
     * The XMPP server address (may be NULL).
     *
     * This is optional -- setting this to NULL will allow for normal
     * XMPP SRV lookups to locate the server.
     */
    char *host;

    /**
     * Name of the software using libconflate.
     *
     * For example:  awesomeproxy
     */
    char *software;
    /**
     * Version number of the software running libconflate.
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
     * Callback issued when libconflate wants to report stats.
     *
     * The client is expected to call the conflate_add_stat callback
     * once for every stat it wishes to report along with the given
     * opaque, followed by one invocation with two NULLs.
     *
     * @param udata The client's custom user data
     * @param opaque an opaque value that must be supplied to the callback
     * @param type primary stat type (may be NULL)
     * @param form detailed stat request form (may be NULL)
     * @param cb the callback into which stats will be fed
     */
    void (*get_stats)(void *udata, void *opaque, char *type, kvpair_t *form,
                      conflate_add_stat cb);

    /**
     * Callback issued when libconflate wants to reset stats.
     *
     * @param udata the client's custom user data
     * @param type stat type to clear (may be NULL)
     * @param form detailed stat request form (may be NULL)
     */
    void (*reset_stats)(void *udata, char *type, kvpair_t *form);

    /**
     * Callback issued when libconflate would like to run a ping test
     * against a series of servers.
     *
     * @param userdata user's callback info
     * @param opaque an opaque that must be given to the callback
     *        unmodified
     * @param servers null-terminated list of null-terminated strings
     * @param cb callback to issue at the end of each test result
     */
    void (*ping_test)(void* userdata, void* opaque,
                      kvpair_t *form, conflate_add_ping_report cb);

} conflate_config_t;

/**
 * @}
 */

typedef struct {

    xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;
    xmpp_log_t *log;

    conflate_config_t *conf;

    pthread_t thread;

} conflate_handle_t;

/**
 * \defgroup Core
 * @{
 */

/**
 * The main entry point for starting a conflate agent.
 *
 * @param conf configuration for libconflate
 *
 * @return true if libconflate was able to properly initialize itself
 */
bool start_conflate(conflate_config_t conf) __attribute__ ((warn_unused_result));

/**
 * @}
 */

/* Misc */
char* safe_strdup(const char*);
void free_string_list(char **);

/**
 * Create a copy of a config.
 *
 * @param c the configuration to duplicate
 *
 * @return a deep copy of the configuration
 */
conflate_config_t* dup_conf(conflate_config_t c);

/**
 * \defgroup Persistence
 * @{
 */

/**
 * Load the key/value pairs from the file at the given path.
 *
 * @param filename the path from which the config should be read
 *
 * @return the config, or NULL if the config could not be read for any reason
 */
kvpair_t* load_kvpairs(const char *filename)
    __attribute__ ((warn_unused_result, nonnull(1)));

/**
 * Save a config at the given path.
 *
 * @param pairs the kvpairs to store
 * @param filename the path to which the config should be written
 *
 * @return false if the configuration could not be saved for any reason
 */
bool save_kvpairs(kvpair_t* pairs, const char *filename)
    __attribute__ ((warn_unused_result, nonnull(1, 2)));

/**
 * @}
 */

#endif /* CONFLATE_H */
