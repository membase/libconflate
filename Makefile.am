ACLOCAL_AMFLAGS=-I m4
SHELL=/bin/sh
lib_LTLIBRARIES = libconflate.la
pkginclude_HEADERS = conflate.h alarm.h
noinst_PROGRAMS = examples/bot

EXTRA_DIST=LICENSE.txt README.markdown Doxyfile libconflate.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libconflate.pc

BUILT_SOURCES=

SUBDIRS= $(MAYBE_LIBSTROPHE) . tests
DIST_SUBDIRS = $(MAYBE_LIBSTROPHE) . tests

examples_bot_DEPENDENCIES = libconflate.la
examples_bot_SOURCES = examples/bot.c
examples_bot_CFLAGS = $(libstrophe_CFLAGS)
examples_bot_LDADD = -L. -lconflate @PROFILER_LDFLAGS@

libconflate_la_CFLAGS = @PROFILER_FLAGS@ $(libstrophe_CFLAGS)
libconflate_la_SOURCES = conflate.h conflate_internal.h conflate_convenience.h alarm.h rest.h\
	alarm.c kvpair.c util.c persist.c xmpp.c logging.c adhoc_commands.c \
    conflate.c rest.c
libconflate_la_LIBADD = $(libstrophe_LIBS)

MOSTLYCLEANFILES = *.gcov *.gcno *.gcda *.tcov

.PHONY: docs
docs:
	(cat Doxyfile ; echo "PROJECT_NUMBER=`git describe`") | doxygen -

test: check
	@for f in *.gcda; do gcov $$f; done | egrep -v ':creating'