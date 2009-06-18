#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sysexits.h>

#include "conflate.h"

static void display_config(void* userdata, kvpair_t* conf)
{
    printf("Hey.  I received a new config (userdata: %s):\n",
           (char*)userdata);

    while (conf) {
        int i = 0;
        printf("\t%s\n", conf->key);

        for (i = 0; conf->values[i]; i++) {
            printf("\t\t%s\n", conf->values[i]);
        }

        conf = conf->next;
    }
}

static enum conflate_mgmt_cb_result process_stats(void *opaque,
                                                  conflate_handle_t *handle,
                                                  const char *cmd,
                                                  bool direct,
                                                  kvpair_t *form,
                                                  conflate_form_result *r)
{
    /* Only direct stat requests are handled. */
    assert(direct);

    char *subtype = get_simple_kvpair_val(form, "-subtype-");

    fprintf(stderr, "Handling stats request with subtype:  %s\n",
            subtype ? subtype : "(null)");

    conflate_add_field(r, "stat1", "val1");
    conflate_add_field(r, "stat2", "val2");

    return RV_OK;
}


static enum conflate_mgmt_cb_result process_reset_stats(void *opaque,
                                                        conflate_handle_t *handle,
                                                        const char *cmd,
                                                        bool direct,
                                                        kvpair_t *form,
                                                        conflate_form_result *r)
{
    char *subtype = get_simple_kvpair_val(form, "-subtype-");

    fprintf(stderr, "Handling stats reset with subtype:  %s\n",
            subtype ? subtype : "(null)");

    return RV_OK;
}

static enum conflate_mgmt_cb_result process_ping_test(void *opaque,
                                                      conflate_handle_t *handle,
                                                      const char *cmd,
                                                      bool direct,
                                                      kvpair_t *form,
                                                      conflate_form_result *r)
{
    kvpair_t *servers_p = find_kvpair(form, "servers");
    if (!servers_p) {
        return RV_BADARG;
    }
    char **servers = servers_p->values;

    for (int i = 0; servers[i]; i++) {
        conflate_next_fieldset(r);
        conflate_add_field(r, "-set-", servers[i]);

        const char *vals[3] = { "val1", "val2", NULL };
        conflate_add_field_multi(r, "test1", vals);

        conflate_add_field(r, "test2", "some val");
    }

    return RV_OK;
}

int main(int argc, char **argv) {

    conflate_config_t conf;

    if (argc < 3) {
        fprintf(stderr, "Usage: bot <jid> <pass> [host]\n");
        exit(EX_USAGE);
    }

    /* Initialize callbacks for provided functionality */
    conflate_register_mgmt_cb("client_stats", "Retrieves stats from the agent",
                              process_stats);
    conflate_register_mgmt_cb("reset_stats", "Reset stats on the agent",
                              process_reset_stats);

    conflate_register_mgmt_cb("ping_test", "Perform a ping test",
                              process_ping_test);

    init_conflate(&conf);

    conf.jid = argv[1];
    conf.pass = argv[2];
    conf.host = (argc == 4 ? argv[3] : NULL);
    conf.software = "conflate sample bot";
    conf.version = "1.0";
    conf.save_path = "/tmp/conflate.db";
    conf.log = conflate_stderr_logger;

    conf.userdata = "something awesome";
    conf.new_config = display_config;

    if(!start_conflate(conf)) {
        fprintf(stderr, "Couldn't initialize libconflate.\n");
        exit(1);
    }

    pause();
}
