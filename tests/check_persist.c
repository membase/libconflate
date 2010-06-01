#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <check.h>

#include <conflate.h>
#include <conflate_internal.h>

#include "test_common.h"

#ifdef CONFLATE_USE_SQLITE

static conflate_handle_t *handle;
static conflate_config_t conf;

char db_loc[L_tmpnam];

static void null_logger(__attribute__((unused))void *userdata,
                        __attribute__((unused)) enum conflate_log_level lvl,
                        __attribute__((unused))const char *msg, ...)
{
    /* Don't log during tests. */
}

static void handle_setup(void) {
    init_conflate(&conf);
    conf.log = null_logger; /* Comment this line out for logging */
    handle = calloc(1, sizeof(conflate_handle_t));
    fail_if(handle == NULL, "calloc failed");
    handle->conf = &conf;

    fail_if(tmpnam(db_loc) == NULL, "Couldn't make a tmp db.");
}

static void handle_teardown(void) {
    free(handle);
    unlink(db_loc);
}

START_TEST (test_empty_private_get)
{
    fail_unless(conflate_get_private(handle, "some key", db_loc) == NULL,
                "Expected NULLness from an empty get_private");
}
END_TEST

START_TEST (test_empty_private_delete)
{
    fail_unless(conflate_delete_private(handle, "some key", db_loc),
                "Expected a delete to fail");
}
END_TEST

START_TEST (test_private_save)
{
    fail_unless(conflate_save_private(handle,"some key", "some value",
                                      db_loc),
                "Failed to save a new value.");
}
END_TEST

START_TEST (test_private_delete)
{
    fail_unless(conflate_save_private(handle, "some key", "some value",
                                      db_loc),
                "Failed to save a new value.");
    fail_unless(conflate_delete_private(handle, "some key", db_loc),
                "Failed to delete.");
}
END_TEST

START_TEST (test_private_retrieve)
{
    fail_unless(conflate_save_private(handle, "some key", "some value",
                                      db_loc),
                "Failed to save a new value.");
    char *val = conflate_get_private(handle, "some key", db_loc);
    fail_if(val == NULL, "Did not retrieve a value.");
    fail_unless(strcmp(val, "some value") == 0,
                "Incorrect value returned.");
    free(val);
}
END_TEST

START_TEST (test_kvpair_storage)
{
    char* args[] = {"arg1", "arg2", NULL};
    kvpair_t *pair = mk_kvpair("some_key", args);

    fail_if(pair == NULL, "Didn't create a pair.");
    fail_unless(strcmp(pair->key, "some_key") == 0, "Key is broken.");
    fail_unless(strcmp(pair->values[0], "arg1") == 0, "First value is broken.");
    fail_unless(strcmp(pair->values[1], "arg2") == 0, "Second value is broken.");
    fail_unless(pair->used_values == 2, "Wrong number of used values.");
    fail_unless(pair->allocated_values >= pair->used_values,
                "Allocated values can't be smaller than used values.");
    fail_unless(pair->next == NULL, "Next pointer is non-null.");

    fail_unless(save_kvpairs(handle, pair, db_loc),
                "Failed to save kv pairs.");

    kvpair_t *dbpair = load_kvpairs(handle, db_loc);

    check_pair_equality(pair, dbpair);

    free_kvpair(pair);
    free_kvpair(dbpair);
}
END_TEST

static Suite* persist_suite(void)
{
    Suite *s = suite_create("persist");

    /* Core test case */
    TCase *tc = tcase_create("persist");

    tcase_add_checked_fixture(tc, handle_setup, handle_teardown);

    tcase_add_test(tc, test_empty_private_get);
    tcase_add_test(tc, test_empty_private_delete);
    tcase_add_test(tc, test_kvpair_storage);
    tcase_add_test(tc, test_private_save);
    tcase_add_test(tc, test_private_delete);
    tcase_add_test(tc, test_private_retrieve);

    suite_add_tcase(s, tc);

    return s;
}

#endif // CONFLATE_USE_SQLITE

int
main (void)
{
    int number_failed = 0;

#ifdef CONFLATE_USE_SQLITE
    Suite *s = persist_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
#endif // CONFLATE_USE_SQLITE

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
