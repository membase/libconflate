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

static void do_stats(void* userdata, conflate_form_result *r,
                     char* type, kvpair_t *form)
{
    conflate_add_field(r, "stat1", "val1");
    conflate_add_field(r, "stat2", "val2");
}

static void do_reset_stats(void* userdata, char *type, kvpair_t *form)
{
    printf("Resetting stats...\n");
}

static void do_ping_test(void* userdata, conflate_form_result *r,
                         kvpair_t *form)
{
    kvpair_t *servers_p = find_kvpair(form, "servers");
    assert(servers_p);
    char **servers = servers_p->values;

    for (int i = 0; servers[i]; i++) {
        conflate_next_fieldset(r);
        conflate_add_field(r, "-set-", servers[i]);

        const char *vals[3] = { "val1", "val2", NULL };
        conflate_add_field_multi(r, "test1", vals);

        conflate_add_field(r, "test2", "some val");
    }
}

int main(int argc, char **argv) {

    conflate_config_t conf;

    if (argc < 3) {
        fprintf(stderr, "Usage: bot <jid> <pass> [host]\n");
        exit(EX_USAGE);
    }

    init_conflate(&conf);

    conf.jid = argv[1];
    conf.pass = argv[2];
    conf.host = (argc == 4 ? argv[3] : NULL);
    conf.software = "conflate sample bot";
    conf.version = "1.0";
    conf.save_path = "/tmp/conflate.db";

    conf.userdata = "something awesome";
    conf.new_config = display_config;
    conf.get_stats = do_stats;
    conf.reset_stats = do_reset_stats;
    conf.ping_test = do_ping_test;

    if(!start_conflate(conf)) {
        fprintf(stderr, "Couldn't initialize libconflate.\n");
        exit(1);
    }

    pause();
}
