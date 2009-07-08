#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <check.h>
#include <alarm.h>

static alarm_queue_t *alarmqueue = NULL;

static void handle_setup(void) {
    alarmqueue = init_alarmqueue();
    fail_if(alarmqueue == NULL, "Failed to set up alarm queue");
}

static void handle_teardown(void) {
    destroy_alarmqueue(alarmqueue);
    alarmqueue = NULL;
}

START_TEST(test_simple_alarm)
{
    alarm_t in_alarm;

    add_alarm(alarmqueue, "This is a test message 1");
    in_alarm = get_alarm(alarmqueue);
    fail_unless(in_alarm.open == 1, "Didn't receive alarm message 1.");
    fail_unless(strcmp(in_alarm.msg, "This is a test message 1") == 0,
                "Didn't get the right message for message 1.");

    add_alarm(alarmqueue, "This is a test message 2");
    add_alarm(alarmqueue, "This is a test message 3");
    in_alarm = get_alarm(alarmqueue);
    fail_unless(in_alarm.open == 1, "Didn't receive alarm message 2.");
    fail_unless(strcmp(in_alarm.msg, "This is a test message 2") == 0,
                "Didn't get the right message for message 2.");

    in_alarm = get_alarm(alarmqueue);
    fail_unless(in_alarm.open == 1, "Didn't receive alarm message 1.");
    fail_unless(strcmp(in_alarm.msg, "This is a test message 3") == 0,
                "Didn't get the right message for message 3.");
    in_alarm = get_alarm(alarmqueue);
    fail_unless(in_alarm.open == 0, "Shouldn't have recieved open alarm.");
}
END_TEST

static Suite* alarm_suite(void)
{
    Suite *s = suite_create("alarm");
    TCase *tc = tcase_create("Core");

    tcase_add_checked_fixture(tc, handle_setup, handle_teardown);

    tcase_add_test(tc, test_simple_alarm);

    suite_add_tcase(s, tc);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s = alarm_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
