#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

#include <conflate.h>

#include "test_common.h"

static kvpair_t *pair = NULL;

static void setup(void) {
    pair = NULL;
}

static void teardown(void) {
    free_kvpair(pair);
}

START_TEST (test_mk_pair_with_arg)
{
    char* args[] = {"arg1", "arg2", NULL};
    pair = mk_kvpair("some_key", args);

    fail_if(pair == NULL, "Didn't create a pair.");
    fail_unless(strcmp(pair->key, "some_key") == 0, "Key is broken.");
    fail_unless(strcmp(pair->values[0], "arg1") == 0, "First value is broken.");
    fail_unless(strcmp(pair->values[1], "arg2") == 0, "Second value is broken.");
    fail_unless(pair->used_values == 2, "Wrong number of used values.");
    fail_unless(pair->allocated_values >= pair->used_values,
                "Allocated values can't be smaller than used values.");
    fail_unless(pair->next == NULL, "Next pointer is non-null.");
}
END_TEST

START_TEST (test_mk_pair_without_arg)
{
    pair = mk_kvpair("some_key", NULL);

    fail_if(pair == NULL, "Didn't create a pair.");
    fail_unless(strcmp(pair->key, "some_key") == 0, "Key is broken.");
    fail_unless(pair->used_values == 0, "Has values?");
    fail_unless(pair->allocated_values >= pair->used_values,
                "Allocated values can't be smaller than used values.");
    fail_unless(pair->values[0] == NULL, "First value isn't null.");
    fail_unless(pair->next == NULL, "Next pointer is non-null.");
}
END_TEST

START_TEST (test_add_value_to_empty_values)
{
    pair = mk_kvpair("some_key", NULL);

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
}
END_TEST

START_TEST (test_add_value_to_existing_values)
{
    char* args[] = {"arg1", "arg2", NULL};
    pair = mk_kvpair("some_key", args);

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
}
END_TEST

START_TEST (test_find_from_null)
{
    fail_unless(find_kvpair(NULL, "some_key") == NULL, "Couldn't find from NULL.");
}
END_TEST

START_TEST (test_find_first_item)
{
    pair = mk_kvpair("some_key", NULL);
    fail_unless(find_kvpair(pair, "some_key") == pair, "Identity search failed.");
}
END_TEST

START_TEST (test_find_second_item)
{
    kvpair_t *pair1 = mk_kvpair("some_key", NULL);
    pair = mk_kvpair("some_other_key", NULL);
    pair->next = pair1;

    fail_unless(find_kvpair(pair, "some_key") == pair1, "Depth search failed.");
}
END_TEST

START_TEST (test_find_missing_item)
{
    kvpair_t *pair1 = mk_kvpair("some_key", NULL);
    pair = mk_kvpair("some_other_key", NULL);
    pair->next = pair1;

    fail_unless(find_kvpair(pair, "missing_key") == NULL, "Negative search failed.");
}
END_TEST

START_TEST (test_simple_find_from_null)
{
    fail_unless(get_simple_kvpair_val(NULL, "some_key") == NULL,
                "Couldn't find from NULL.");
}
END_TEST

START_TEST (test_simple_find_first_item)
{
    char *val[2] = { "someval", NULL };
    pair = mk_kvpair("some_key", val);
    fail_unless(strcmp(get_simple_kvpair_val(pair, "some_key"),
                       "someval") == 0, "Identity search failed.");
}
END_TEST

START_TEST (test_simple_find_second_item)
{
    char *val[2] = { "someval", NULL };
    kvpair_t *pair1 = mk_kvpair("some_key", val );
    pair = mk_kvpair("some_other_key", NULL);
    pair->next = pair1;

    fail_unless(strcmp(get_simple_kvpair_val(pair, "some_key"),
                       "someval") == 0, "Depth search failed.");
}
END_TEST

START_TEST (test_simple_find_missing_item)
{
    kvpair_t *pair1 = mk_kvpair("some_key", NULL);
    pair = mk_kvpair("some_other_key", NULL);
    pair->next = pair1;

    fail_unless(get_simple_kvpair_val(pair, "missing_key") == NULL,
                "Negative search failed.");
}
END_TEST

START_TEST (test_copy_pair)
{
    char *args1[] = {"arg1", "arg2", NULL};
    char *args2[] = {"other", NULL};
    kvpair_t *pair1 = mk_kvpair("some_key", args1);
    pair = mk_kvpair("some_other_key", args2);
    pair->next = pair1;

    kvpair_t *copy = dup_kvpair(pair);
    fail_if(copy == NULL, "Copy failed.");
    fail_if(copy == pair, "Copy something not an identity.");

    fail_unless(strcmp(copy->key, pair->key) == 0, "Keys don't match.");
    fail_if(copy->key == pair->key, "Keys were identical.");
    check_pair_equality(pair, copy);

    free_kvpair(copy);
}
END_TEST

static bool walk_incr_count_true(void *opaque,
                                 __attribute__((unused))const char *key,
                                 __attribute__((unused))const char **values)
{
    (*(int*)opaque)++;
    return true;
}

static bool walk_incr_count_false(void *opaque,
                                  __attribute__((unused))const char *key,
                                  __attribute__((unused))const char **values)
{
    (*(int*)opaque)++;
    return false;
}

START_TEST (test_walk_true)
{
    char *args1[] = {"arg1", "arg2", NULL};
    char *args2[] = {"other", NULL};
    kvpair_t *pair1 = mk_kvpair("some_key", args1);
    pair = mk_kvpair("some_other_key", args2);
    pair->next = pair1;

    int count = 0;
    walk_kvpair(pair, &count, walk_incr_count_true);

    fail_unless(count == 2, "Count was not two");
}
END_TEST

START_TEST (test_walk_false)
{
    char *args1[] = {"arg1", "arg2", NULL};
    char *args2[] = {"other", NULL};
    kvpair_t *pair1 = mk_kvpair("some_key", args1);
    pair = mk_kvpair("some_other_key", args2);
    pair->next = pair1;

    int count = 0;
    walk_kvpair(pair, &count, walk_incr_count_false);

    printf("Count was %d\n", count);
    fail_unless(count == 1, "Count was not one");
}
END_TEST

static Suite* kvpair_suite (void)
{
    Suite *s = suite_create ("kvpair");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);

    tcase_add_test(tc_core, test_mk_pair_with_arg);
    tcase_add_test(tc_core, test_mk_pair_without_arg);
    tcase_add_test(tc_core, test_add_value_to_existing_values);
    tcase_add_test(tc_core, test_add_value_to_empty_values);
    tcase_add_test(tc_core, test_find_from_null);
    tcase_add_test(tc_core, test_find_first_item);
    tcase_add_test(tc_core, test_find_second_item);
    tcase_add_test(tc_core, test_find_missing_item);
    tcase_add_test(tc_core, test_simple_find_from_null);
    tcase_add_test(tc_core, test_simple_find_first_item);
    tcase_add_test(tc_core, test_simple_find_second_item);
    tcase_add_test(tc_core, test_simple_find_missing_item);
    tcase_add_test(tc_core, test_copy_pair);
    tcase_add_test(tc_core, test_walk_true);
    tcase_add_test(tc_core, test_walk_false);
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
