ACLOCAL_AMFLAGS=-I m4
lib_LTLIBRARIES = libconflate.la
pkginclude_HEADERS = conflate.h alarm.h

EXTRA_DIST=LICENSE.txt README.markdown Doxyfile libconflate.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libconflate.pc

BUILT_SOURCES=

SUBDIRS= . tests
DIST_SUBDIRS = . tests

# This breaks on windows o/s.
#
# noinst_PROGRAMS = examples/bot
#
# examples_bot_DEPENDENCIES = libconflate.la
# examples_bot_SOURCES = examples/bot.c
# examples_bot_LDADD = libconflate.la $(GCOV_LIBS)

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

libconflate_la_LIBADD = $(LIBCONFLATE_LINKFLAGS) $(LTLIBCURL)

MOSTLYCLEANFILES = *.gcov *.gcno *.gcda *.tcov

.PHONY: docs
docs:
	(cat Doxyfile ; echo "PROJECT_NUMBER=`git describe`") | doxygen -

test: check
	@for f in *.gcda; do gcov $$f; done | egrep -v ':creating'