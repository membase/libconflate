lib_LIBRARIES = libconflate.a
pkginclude_HEADERS = conflate.h
noinst_PROGRAMS = examples/bot

BUILT_SOURCES=

SUBDIRS= . tests

examples_bot_DEPENDENCIES = libconflate.a
examples_bot_SOURCES = examples/bot.c
examples_bot_LDADD = -L. -lconflate

libconflate_a_SOURCES = conflate.h kvpair.c util.c persist.c xmpp.c

docs:
	(cat Doxyfile ; echo "PROJECT_NUMBER=`cat version.num`") | doxygen -

