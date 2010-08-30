TESTS = check_kvpair check_xmpp check_persist check_alarm
check_PROGRAMS = check_kvpair check_xmpp check_persist check_alarm

check_kvpair_SOURCES = check_kvpair.c test_common.c $(top_builddir)/conflate.h test_common.h
check_kvpair_CFLAGS = @CHECK_CFLAGS@
check_kvpair_LDADD = $(top_builddir)/libconflate.la @CHECK_LIBS@ $(GCOV_LIBS)

check_xmpp_SOURCES = check_xmpp.c $(top_builddir)/conflate.h test_common.h
check_xmpp_CFLAGS = @CHECK_CFLAGS@
check_xmpp_LDADD = $(top_builddir)/libconflate.la @CHECK_LIBS@ $(GCOV_LIBS)

check_persist_SOURCES = check_persist.c test_common.c $(top_builddir)/conflate.h test_common.h
check_persist_CFLAGS = @CHECK_CFLAGS@
check_persist_LDADD = $(top_builddir)/libconflate.la @CHECK_LIBS@ $(GCOV_LIBS)

check_alarm_SOURCES = check_alarm.c $(top_builddir)/alarm.h test_common.h
check_alarm_CFLAGS = @CHECK_CFLAGS@
check_alarm_LDADD = $(top_builddir)/libconflate.la @CHECK_LIBS@ $(GCOV_LIBS)
