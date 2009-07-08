#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <check.h>
#include <alarm.h>

START_TEST(test_simple_alarm)
{
	alarm_queue_t *alarmqueue;
	alarm_t in_alarm;
	alarmqueue = calloc(1, sizeof(alarm_queue_t));
	init_alarmqueue(alarmqueue);

	add_alarm(alarmqueue, 0, 1, 0, 0, "This is a test message 1");
	in_alarm = get_alarm(alarmqueue);
	fail_unless(in_alarm.open == 1, "Didn't receive alarm message 1.");
	fail_unless(strcmp(in_alarm.msg, "This is a test message 1") == 0,
                "Didn't get the right message for message 1.");

	add_alarm(alarmqueue, 0, 1, 0, 0, "This is a test message 2");
	add_alarm(alarmqueue, 0, 1, 0, 0, "This is a test message 3");
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

	free(alarmqueue);
}
END_TEST

static Suite* alarm_suite(void)
{
	Suite *s = suite_create("alarm");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_simple_alarm);
	suite_add_tcase(s, tc_core);
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
