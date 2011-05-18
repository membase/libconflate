#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

#include <conflate.h>

static conflate_result conf_cb(__attribute__((unused))void* userdata,
                                kvpair_t* __attribute__((unused))conf)
{
    return CONFLATE_SUCCESS;
}

static void log_cb(__attribute__((unused))void *userdata,
                   __attribute__((unused))enum conflate_log_level level,
                   __attribute__((unused))const char *fmt, ...)
{
}

static void init_config(conflate_config_t* conf)
{
    init_conflate(conf);
    conf->jid = "user@example.com";
    conf->pass = "somepass";
    conf->host = NULL;
    conf->software = "test";
    conf->version = "1.x";
    conf->save_path = "/tmp/something.db";
    conf->userdata = "some user data";
    conf->log = log_cb;
    conf->new_config = conf_cb;
}

static int count_nulls(conflate_config_t* conf)
{
    int num = sizeof(conflate_config_t) / sizeof(void*);
    void *buf[num];
    int rv = 0, i = 0;
    // cannot simply cast conf to (void **),
    // 'cause it breaks pointer aliasing rules
    // memcpy to temporary buffer seems to be the best solution for
    // this not peformance-critical place
    memcpy(buf, conf, sizeof(conflate_config_t));
    void** p = buf;

    for (i = 0; i < num; i++, p++) {
        if (*p == NULL) {
            rv++;
        }
    }

    return rv;
}

static void verify_conf(conflate_config_t* a, conflate_config_t* b)
{
    int ncount1, ncount2;

    ncount1 = count_nulls(a);
    ncount2 = count_nulls(b);
    fail_unless(ncount1 == ncount2,
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
}

START_TEST (test_dup_with_null_host)
{
    conflate_config_t conf;

    init_config(&conf);
    int count = count_nulls(&conf);
    fail_unless(count == 1, "Expected exactly one null. Got: %d", count);

    verify_conf(&conf, dup_conf(conf));
}
END_TEST

START_TEST (test_dup_with_host)
{
    conflate_config_t conf;

    init_config(&conf);
    conf.host = "example.com";
    int count = count_nulls(&conf);
    fail_unless(count == 0, "Expected exactly one null. Got: %d", count);

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
