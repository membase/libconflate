#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

static void* test_get_alarm(void *arg)
{
    int *counter_p = ((int*)arg);
    /* Just enough to get a bit of contention */
    sleep(1);

    for (*counter_p = 0; *counter_p < 101; (*counter_p)++) {
        get_alarm(alarmqueue);
    }
    return counter_p;
}

START_TEST(test_full_queue)
{
    for (int i = 0; i < 100; i++) {
        add_alarm(alarmqueue, "Test alarm message.");
    }

    pthread_t thread;
    int counter = 0;
    fail_unless(pthread_create(&thread, NULL, test_get_alarm, &counter) == 0,
                "Failed to start thread.");

    /* At this point, the queue should be full, and now we should
       block on insertion. */
    add_alarm(alarmqueue, "Overflow!");

    fail_unless(pthread_join(thread, NULL) == 0, "Failed to join thread.");

    fail_unless(counter == 101, "Didn't count the right number of alarms.");
}
END_TEST

static Suite* alarm_suite(void)
{
    Suite *s = suite_create("alarm");
    TCase *tc = tcase_create("Core");

    tcase_add_checked_fixture(tc, handle_setup, handle_teardown);

    tcase_add_test(tc, test_simple_alarm);
    tcase_add_test(tc, test_giant_alarm);
    tcase_add_test(tc, test_full_queue);

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
