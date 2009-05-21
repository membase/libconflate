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

START_TEST (test_find_from_null)
{
    fail_unless(find_kvpair(NULL, "some_key") == NULL, "Couldn't find from NULL.");
}
END_TEST

START_TEST (test_find_first_item)
{
    kvpair_t* pair = mk_kvpair("some_key", NULL);

    fail_unless(find_kvpair(pair, "some_key") == pair, "Identity search failed.");
}
END_TEST

START_TEST (test_find_second_item)
{
    kvpair_t* pair1 = mk_kvpair("some_key", NULL);
    kvpair_t* pair2 = mk_kvpair("some_other_key", NULL);
    pair2->next = pair1;

    fail_unless(find_kvpair(pair2, "some_key") == pair1, "Depth search failed.");
}
END_TEST

START_TEST (test_find_missing_item)
{
    kvpair_t* pair1 = mk_kvpair("some_key", NULL);
    kvpair_t* pair2 = mk_kvpair("some_other_key", NULL);
    pair2->next = pair1;

    fail_unless(find_kvpair(pair2, "missing_key") == NULL, "Negative search failed.");
}
END_TEST

static void check_pair_equality(kvpair_t *one, kvpair_t *two)
{
    fail_if(one == NULL, "one is null.");
    fail_if(one == two, "Comparing identical pairs.");
    fail_unless(strcmp(one->key, two->key) == 0, "Keys don't match.");

    int i = 0;
    for (i=0; one->values[i]; i++) {
        fail_unless(two->values[i] != NULL, "Value at one, but not two");
        fail_unless(strcmp(one->values[i], two->values[i]) == 0,
                    "Values don't match.");
        fail_unless((one->values[i+1] != NULL && two->values[i+1] != NULL)
                    || (one->values[i+1] == NULL && two->values[i+1] == NULL),
                    "Unbalanced values.");

    }

    if (one->next) {
        fail_unless(two->next != NULL, "No two->next.");
        check_pair_equality(one->next, two->next);
    } else {
        fail_if(two->next, "No one->next, but a two->next");
    }
}

START_TEST (test_copy_pair)
{
    char *args1[] = {"arg1", "arg2", NULL};
    char *args2[] = {"other", NULL};
    kvpair_t *pair1 = mk_kvpair("some_key", args1);
    kvpair_t *pair2 = mk_kvpair("some_other_key", args2);
    pair2->next = pair1;

    kvpair_t *copy = dup_kvpair(pair2);
    fail_if(copy == NULL, "Copy failed.");
    fail_if(copy == pair2, "Copy something not an identity.");

    fail_unless(strcmp(copy->key, pair2->key) == 0, "Keys don't match.");
    fail_if(copy->key == pair2->key, "Keys were identical.");
    check_pair_equality(pair2, copy);
}
END_TEST

static bool walk_incr_count_true(void *opaque,
                                 const char *key, const char **values)
{
    (*(int*)opaque)++;
    return true;
}

static bool walk_incr_count_false(void *opaque,
                                  const char *key, const char **values)
{
    (*(int*)opaque)++;
    return false;
}

START_TEST (test_walk_true)
{
    char *args1[] = {"arg1", "arg2", NULL};
    char *args2[] = {"other", NULL};
    kvpair_t *pair1 = mk_kvpair("some_key", args1);
    kvpair_t *pair2 = mk_kvpair("some_other_key", args2);
    pair2->next = pair1;

    int count = 0;
    walk_kvpair(pair2, &count, walk_incr_count_true);

    fail_unless(count == 2, "Count was not two");
}
END_TEST

START_TEST (test_walk_false)
{
    char *args1[] = {"arg1", "arg2", NULL};
    char *args2[] = {"other", NULL};
    kvpair_t *pair1 = mk_kvpair("some_key", args1);
    kvpair_t *pair2 = mk_kvpair("some_other_key", args2);
    pair2->next = pair1;

    int count = 0;
    walk_kvpair(pair2, &count, walk_incr_count_false);

    printf("Count was %d\n", count);
    fail_unless(count == 1, "Count was not one");
}
END_TEST

static Suite* kvpair_suite (void)
{
    Suite *s = suite_create ("kvpair");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_mk_pair_with_arg);
    tcase_add_test (tc_core, test_mk_pair_without_arg);
    tcase_add_test (tc_core, test_add_value_to_existing_values);
    tcase_add_test (tc_core, test_add_value_to_empty_values);
    tcase_add_test (tc_core, test_find_from_null);
    tcase_add_test (tc_core, test_find_first_item);
    tcase_add_test (tc_core, test_find_second_item);
    tcase_add_test (tc_core, test_find_missing_item);
    tcase_add_test (tc_core, test_copy_pair);
    tcase_add_test (tc_core, test_walk_true);
    tcase_add_test (tc_core, test_walk_false);
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
