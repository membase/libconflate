TESTS = check_kvpair check_alarm
check_PROGRAMS = check_kvpair check_alarm

check_kvpair_SOURCES = check_kvpair.c test_common.c $(top_builddir)/conflate.h test_common.h
check_kvpair_LDADD = $(top_builddir)/libconflate.la

check_alarm_SOURCES = check_alarm.c test_common.c $(top_builddir)/alarm.h test_common.h
check_alarm_LDADD = $(top_builddir)/libconflate.la
