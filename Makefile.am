ACLOCAL_AMFLAGS=-I m4
SHELL=/bin/sh
lib_LTLIBRARIES = libconflate.la
include_HEADERS = conflate.h alarm.h
noinst_PROGRAMS = examples/bot

EXTRA_DIST=LICENSE.txt README.markdown Doxyfile

BUILT_SOURCES=

SUBDIRS= . tests

examples_bot_DEPENDENCIES = libconflate.la
examples_bot_SOURCES = examples/bot.c
examples_bot_LDADD = -L. -lconflate @PROFILER_LDFLAGS@

libconflate_la_CFLAGS = @PROFILER_FLAGS@
libconflate_la_SOURCES = conflate.h conflate_internal.h conflate_convenience.h alarm.h\
	alarm.c kvpair.c util.c persist.c xmpp.c logging.c adhoc_commands.c

MOSTLYCLEANFILES = *.gcov *.gcno *.gcda *.tcov

.PHONY: docs
docs:
	(cat Doxyfile ; echo "PROJECT_NUMBER=`git describe`") | doxygen -

test: check
	@for f in *.gcda; do gcov $$f; done | egrep -v ':creating'