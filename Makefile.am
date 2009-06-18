lib_LIBRARIES = libconflate.a
pkginclude_HEADERS = conflate.h
noinst_PROGRAMS = examples/bot

BUILT_SOURCES=

SUBDIRS= . tests

examples_bot_DEPENDENCIES = libconflate.a
examples_bot_SOURCES = examples/bot.c
examples_bot_LDADD = -L. -lconflate @PROFILER_LDFLAGS@

libconflate_a_CFLAGS = @PROFILER_FLAGS@
libconflate_a_SOURCES = conflate.h conflate_internal.h conflate_convenience.h \
	kvpair.c util.c persist.c xmpp.c logging.c

.PHONY: docs
docs:
	(cat Doxyfile ; echo "PROJECT_NUMBER=`cat version.num`") | doxygen -

