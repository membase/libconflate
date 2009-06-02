#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

#include <conflate.h>

static conflate_handle_t *handle;
static conflate_config_t conf;

static void handle_setup(void) {
    init_conflate(&conf);
    handle = calloc(1, sizeof(conflate_handle_t));
    fail_if(handle == NULL, "calloc failed");
    handle->conf = &conf;
}

static void handle_teardown(void) {
    free(handle);
}

START_TEST (test_empty_private_get)
{
    fail_unless(conflate_get_private(handle, "some key", ":memory:") == NULL,
                "Expected NULLness from an empty get_private");
}
END_TEST

START_TEST (test_empty_private_delete)
{
    fail_unless(conflate_delete_private(handle, "some key", ":memory:"),
                "Expected a delete to fail");
}
END_TEST

START_TEST (test_private_save)
{
    fail_unless(conflate_save_private(handle,"some key", "some value",
                                      ":memory:"),
                "Failed to save a new value.");
}
END_TEST

START_TEST (test_private_delete)
{
    fail_unless(conflate_save_private(handle, "some key", "some value",
                                      ":memory:"),
                "Failed to save a new value.");
    fail_unless(conflate_delete_private(handle, "some key", ":memory:"),
                "Failed to delete.");
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
    tcase_add_test(tc, test_private_save);
    tcase_add_test(tc, test_private_delete);

    suite_add_tcase(s, tc);

    return s;
}

int
main (void)
{
    int number_failed;
    Suite *s = persist_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
