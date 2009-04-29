#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

#include <conflate.h>

START_TEST (test_mk_pair_with_arg)
{
    char* args[] = {"arg1", "arg2", NULL};
    kvpair_t* pair = mk_kvpair("some_key", args);

    fail_if(pair == NULL, "Didn't create a pair.");
    fail_unless(strcmp(pair->key, "some_key") == 0, "Key is broken.");
    fail_unless(strcmp(pair->values[0], "arg1") == 0, "First value is broken.");
    fail_unless(strcmp(pair->values[1], "arg2") == 0, "Second value is broken.");
    fail_unless(pair->used_values == 2, "Wrong number of used values.");
    fail_unless(pair->allocated_values >= pair->used_values,
                "Allocated values can't be smaller than used values.");
    fail_unless(pair->next == NULL, "Next pointer is non-null.");

    free_kvpair(pair);
}
END_TEST

START_TEST (test_mk_pair_without_arg)
{
    kvpair_t* pair = mk_kvpair("some_key", NULL);

    fail_if(pair == NULL, "Didn't create a pair.");
    fail_unless(strcmp(pair->key, "some_key") == 0, "Key is broken.");
    fail_unless(pair->used_values == 0, "Has values?");
    fail_unless(pair->allocated_values >= pair->used_values,
                "Allocated values can't be smaller than used values.");
    fail_unless(pair->values[0] == NULL, "First value isn't null.");
    fail_unless(pair->next == NULL, "Next pointer is non-null.");

    free_kvpair(pair);
}
END_TEST

START_TEST (test_add_value_to_empty_values)
{
    kvpair_t* pair = mk_kvpair("some_key", NULL);

    fail_if(pair == NULL, "Didn't create a pair.");
    fail_unless(strcmp(pair->key, "some_key") == 0, "Key is broken.");
    fail_unless(pair->used_values == 0, "Has values?");
    fail_unless(pair->allocated_values >= pair->used_values,
                "Allocated values can't be smaller than used values.");
    fail_unless(pair->values[0] == NULL, "First value isn't null.");
    fail_unless(pair->next == NULL, "Next pointer is non-null.");

    add_kvpair_value(pair, "newvalue1");
    fail_unless(pair->used_values == 1, "Value at 1");
    fail_unless(strcmp(pair->values[0], "newvalue1") == 0, "Unexpected value at 0");

    add_kvpair_value(pair, "newvalue2");
    fail_unless(pair->used_values == 2, "Value at 2");
    fail_unless(strcmp(pair->values[1], "newvalue2") == 0, "Unexpected value at 1");

    free_kvpair(pair);
}
END_TEST

START_TEST (test_add_value_to_existing_values)
{
    char* args[] = {"arg1", "arg2", NULL};
    kvpair_t* pair = mk_kvpair("some_key", args);

    fail_if(pair == NULL, "Didn't create a pair.");
    fail_unless(strcmp(pair->key, "some_key") == 0, "Key is broken.");
    fail_unless(pair->used_values == 2, "Has values?");
    fail_unless(pair->allocated_values >= pair->used_values,
                "Allocated values can't be smaller than used values.");
    fail_unless(pair->next == NULL, "Next pointer is non-null.");

    add_kvpair_value(pair, "newvalue1");
    fail_unless(pair->used_values == 3, "Value at 3");
    add_kvpair_value(pair, "newvalue2");
    fail_unless(pair->used_values == 4, "Value at 4");

    fail_unless(strcmp(pair->values[0], "arg1") == 0, "Unexpected value at 0");
    fail_unless(strcmp(pair->values[1], "arg2") == 0, "Unexpected value at 1");
    fail_unless(strcmp(pair->values[2], "newvalue1") == 0, "Unexpected value at 2");
    fail_unless(strcmp(pair->values[3], "newvalue2") == 0, "Unexpected value at 3");

    free_kvpair(pair);
}
END_TEST

Suite* kvpair_suite (void)
{
    Suite *s = suite_create ("kvpair");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_mk_pair_with_arg);
    tcase_add_test (tc_core, test_mk_pair_without_arg);
    tcase_add_test (tc_core, test_add_value_to_existing_values);
    tcase_add_test (tc_core, test_add_value_to_empty_values);
    suite_add_tcase (s, tc_core);

    return s;
}

int
main (void)
{
    int number_failed;
    Suite *s = kvpair_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
