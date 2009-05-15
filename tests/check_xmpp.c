#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

#include <conflate.h>

static void conf_cb(void* userdata, kvpair_t* conf)
{
}

static void stats_cb(void* userdata, void* opaque,
                     char *type, kvpair_t *form, conflate_add_stat add_stat)
{
}

static void reset_stats_cb(void* userdata, char *type, kvpair_t *form)
{
}

static void ping_test_cb(void* userdata, void* opaque,
                         kvpair_t *conf, conflate_add_ping_report cb)
{
}

static void init_config(conflate_config_t* conf)
{
    conf->jid = "user@example.com";
    conf->pass = "somepass";
    conf->host = NULL;
    conf->software = "test";
    conf->version = "1.x";
    conf->save_path = "/tmp/something.db";
    conf->userdata = "some user data";
    conf->new_config = conf_cb;
    conf->get_stats = stats_cb;
    conf->reset_stats = reset_stats_cb;
    conf->ping_test = ping_test_cb;
}

static int count_nulls(conflate_config_t* conf)
{
    int num = sizeof(conflate_config_t) / sizeof(void*);
    int rv = 0, i = 0;
    void** p = (void**) conf;

    for (i = 0; i < num; i++, p++) {
        if (*p == NULL) {
            rv++;
        }
    }

    return rv;
}

static void verify_conf(conflate_config_t* a, conflate_config_t* b)
{
    fail_unless(count_nulls(a) == count_nulls(b),
                "Differing number of NULLs between a and b");

    fail_unless(strcmp(a->jid, b->jid) == 0,
                "jids are different.");
    fail_unless(strcmp(a->pass, b->pass) == 0,
                "passwords are different.");
    if (a->host == NULL && b->host == NULL) {
        /* OK */
    } else if (a->host == NULL) {
        fail("a was null and b was not null");
    } else if (b->host == NULL) {
        fail("b was null and a was not null");
    } else {
        fail_unless(strcmp(a->host, b->host) == 0,
                    "hosts are different.");
    }
    fail_unless(strcmp(a->software, b->software) == 0,
                "software is different.");
    fail_unless(strcmp(a->version, b->version) == 0,
                "versions are different.");

    fail_unless(a->userdata == b->userdata, "userdata is different.");
    fail_unless(a->new_config == b->new_config, "new_config is different.");
    fail_unless(a->get_stats == b->get_stats, "get_stats is different.");
    fail_unless(a->reset_stats == b->reset_stats, "reset_stats is different.");
}

START_TEST (test_dup_with_null_host)
{
    conflate_config_t conf;

    init_config(&conf);
    fail_unless(count_nulls(&conf) == 1, "Expected exactly one null.");

    verify_conf(&conf, dup_conf(conf));
}
END_TEST

START_TEST (test_dup_with_host)
{
    conflate_config_t conf;

    init_config(&conf);
    conf.host = "example.com";
    fail_unless(count_nulls(&conf) == 0, "Expected exactly one null.");

    verify_conf(&conf, dup_conf(conf));
}
END_TEST

static Suite* xmpp_suite (void)
{
    Suite *s = suite_create ("xmpp");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_dup_with_null_host);
    tcase_add_test (tc_core, test_dup_with_host);
    suite_add_tcase (s, tc_core);

    return s;
}

int
main (void)
{
    int number_failed;
    Suite *s = xmpp_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
