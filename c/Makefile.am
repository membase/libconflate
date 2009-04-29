lib_LIBRARIES = libconflate.a
pkginclude_HEADERS = conflate.h
noinst_PROGRAMS = examples/bot

BUILT_SOURCES=

examples_bot_SOURCES = examples/bot.c
examples_bot_LDADD = -lconflate

libconflate_a_SOURCES = conflate.h kvpair.c util.c persist.c xmpp.c

docs:
	(cat Doxyfile ; echo "PROJECT_NUMBER=`cat version.num`") | doxygen -

