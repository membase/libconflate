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

START_TEST(test_giant_alarm)
{
    char *msg="This is a really large message that exceeds the 256 or "
        "so bytes allocated for messages to see what happens when a "
        "message is too big.  Hopefully it's fine.  "
        "It turns out that 256 bytes or whatever the current value is ends "
        "up being quite a bit of bytes.  I suppose that makes sense if "
        "I do a bit of math, but I'm too lazy to do anything other than assert.";

    fail_unless(strlen(msg) > ALARM_MSG_MAXLEN,
                "Message string was too short to blow up.");
    add_alarm(alarmqueue, msg);

    alarm_t in_alarm = get_alarm(alarmqueue);
    fail_unless(in_alarm.open, "Didn't receive a large alarm.");

    fail_unless(strlen(in_alarm.msg) == ALARM_MSG_MAXLEN,
            "Alarm message is too long.");

    fail_unless(strncmp(msg, in_alarm.msg, ALARM_MSG_MAXLEN) == 0,
                "Alarm message didn't match.");
}
END_TEST

static Suite* alarm_suite(void)
{
    Suite *s = suite_create("alarm");
    TCase *tc = tcase_create("Core");

    tcase_add_checked_fixture(tc, handle_setup, handle_teardown);

    tcase_add_test(tc, test_simple_alarm);
    tcase_add_test(tc, test_giant_alarm);

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
