ACLOCAL_AMFLAGS=-I m4
lib_LTLIBRARIES = libconflate.la
pkginclude_HEADERS = conflate.h alarm.h

EXTRA_DIST=LICENSE.txt README.markdown libconflate.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libconflate.pc

libconflate_la_SOURCES = adhoc_commands.c \
                         alarm.c \
                         alarm.h \
                         conflate.c \
                         conflate.h \
                         conflate_convenience.h \
                         conflate_internal.h \
                         kvpair.c \
                         logging.c \
                         persist.c \
                         rest.c \
                         rest.h \
                         util.c \
                         xmpp.c

libconflate_la_LIBADD = $(LTLIBCURL)

test: check

TESTS = tests/check_kvpair tests/check_alarm
check_PROGRAMS = $(TESTS)

tests_check_kvpair_SOURCES = tests/check_kvpair.c tests/test_common.c \
                             $(top_builddir)/conflate.h tests/test_common.h
tests_check_kvpair_DEPENDENCIES = libconflate.la
tests_check_kvpair_LDADD = libconflate.la

tests_check_alarm_SOURCES = tests/check_alarm.c tests/test_common.c \
                            $(top_builddir)/alarm.h tests/test_common.h
tests_check_alarm_DEPENDENCIES = libconflate.la
tests_check_alarm_LDADD = libconflate.la
