#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "conflate.h"
#include "alarm.h"
#include "conflate_internal.h"


/*
 * This program is both an example of most of the public API of
 * libconflate as well as a sample program to interact with it.
 */

/* ************************************************************ */

/*
 * kvpair visitor callback to display a received config parameter.
 */
static bool config_visitor(void *opaque, const char *key, const char **values)
{
    (void)opaque;
    printf("\t%s\n", key);

    for (int i = 0; values[i]; i++) {
        printf("\t\t%s\n", values[i]);
    }

    return true;
}

/*
 * Example callback to handle a newly receive configuration.
 */
static conflate_result display_config(void* userdata, kvpair_t* conf)
{
    printf("Hey.  I received a new config (userdata: %s):\n",
           (char*)userdata);

    walk_kvpair(conf, NULL, config_visitor);

    return CONFLATE_SUCCESS;
}

/*
 * Example callback with a simple form result.
 */
static enum conflate_mgmt_cb_result process_stats(void *opaque,
                                                  conflate_handle_t *handle,
                                                  const char *cmd,
                                                  bool direct,
                                                  kvpair_t *form,
                                                  conflate_form_result *r)
{
    (void)opaque;
    (void)handle;
    (void)cmd;

    /* Only direct stat requests are handled. */
    assert(direct);

    char *subtype = get_simple_kvpair_val(form, "-subtype-");

    fprintf(stderr, "Handling stats request with subtype:  %s\n",
            subtype ? subtype : "(null)");

    conflate_add_field(r, "stat1", "val1");
    conflate_add_field(r, "stat2", "val2");

    return RV_OK;
}

/*
 * Example callback that has an empty result.
 */
static enum conflate_mgmt_cb_result process_reset_stats(void *opaque,
                                                        conflate_handle_t *handle,
                                                        const char *cmd,
                                                        bool direct,
                                                        kvpair_t *form,
                                                        conflate_form_result *r)
{
    (void)opaque;
    (void)handle;
    (void)cmd;
    (void)direct;
    (void)r;
    char *subtype = get_simple_kvpair_val(form, "-subtype-");

    fprintf(stderr, "Handling stats reset with subtype:  %s\n",
            subtype ? subtype : "(null)");

    return RV_OK;
}

/*
 * Example callback that returns a list of forms.
 */
static enum conflate_mgmt_cb_result process_ping_test(void *opaque,
                                                      conflate_handle_t *handle,
                                                      const char *cmd,
                                                      bool direct,
                                                      kvpair_t *form,
                                                      conflate_form_result *r)
{
    (void)opaque;
    (void)handle;
    (void)cmd;
    (void)direct;

    kvpair_t *servers_p = find_kvpair(form, "servers");
    if (!servers_p) {
        return RV_BADARG;
    }
    char **servers = servers_p->values;

    for (int i = 0; servers[i]; i++) {
        /* For each result we wish to send back, we begin a field set */
        conflate_next_fieldset(r);

        /* All fields added now fall within the current fieldset */
        conflate_add_field(r, "-set-", servers[i]);

        const char *vals[3] = { "val1", "val2", NULL };
        conflate_add_field_multi(r, "test1", vals);

        conflate_add_field(r, "test2", "some val");
    }

    return RV_OK;
}

static enum conflate_mgmt_cb_result process_alarm_create(void *opaque,
                                                         conflate_handle_t *handle,
                                                         const char *cmd,
                                                         bool direct,
                                                         kvpair_t *form,
                                                         conflate_form_result *r)
{
    (void)opaque;
    (void)handle;
    (void)cmd;
    (void)direct;
    (void)form;
    (void)r;

    if(add_alarm(handle->alarms, "test", "This is a test alarm!")) {
        fprintf(stderr, "Created alarm!\n");
    } else {
        fprintf(stderr, "Error queueing an alarm.\n");
    }
        return RV_OK;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        fprintf(stderr, "Usage: bot <jid> <pass> [host]\n");
        exit(EXIT_FAILURE);
    }

    /* Initialize callbacks for provided functionality.
     *
     * These callbacks are global and thus initialized once for your
     * entire application.
     */
    conflate_register_mgmt_cb("client_stats", "Retrieves stats from the agent",
                              process_stats);
    conflate_register_mgmt_cb("reset_stats", "Reset stats on the agent",
                              process_reset_stats);

    conflate_register_mgmt_cb("ping_test", "Perform a ping test",
                              process_ping_test);

    conflate_register_mgmt_cb("alarm", "Create an alarm",
                              process_alarm_create);

    /* Perform default configuration initialization. */
    conflate_config_t conf;
    init_conflate(&conf);

    /* Specify application-specific configuration parameters. */
    conf.jid = argv[1];
    conf.pass = argv[2];
    conf.host = (argc == 4 ? argv[3] : NULL);
    conf.software = "conflate sample bot";
    conf.version = "1.0";
    conf.save_path = "/tmp/conflate.db";
    conf.log = conflate_stderr_logger;

    conf.userdata = "something awesome";
    conf.new_config = display_config;

    /* Begin the connection. */
    if(!start_conflate(conf)) {
        fprintf(stderr, "Couldn't initialize libconflate.\n");
        exit(EXIT_FAILURE);
    }

    /* Typically, your application would go about its business here.
       Since this is just a demo, the caller has nothing else to do,
       so we wait forever. */
    pause();
}
