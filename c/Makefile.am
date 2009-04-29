lib_LIBRARIES = libconflate.a
pkginclude_HEADERS = conflate.h
noinst_PROGRAMS = bot

BUILT_SOURCES=

bot_SOURCES = bot.c
bot_LDADD = -lconflate

libconflate_a_SOURCES = conflate.h kvpair.c util.c persist.c xmpp.c

docs:
	(cat Doxyfile ; echo "PROJECT_NUMBER=`cat version.num`") | doxygen -

