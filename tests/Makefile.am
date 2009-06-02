TESTS = check_kvpair check_xmpp check_persist
check_PROGRAMS = check_kvpair check_xmpp check_persist

check_kvpair_SOURCES = check_kvpair.c $(top_builddir)/conflate.h
check_kvpair_CFLAGS = @CHECK_CFLAGS@
check_kvpair_LDADD = $(top_builddir)/libconflate.a @CHECK_LIBS@ @PROFILER_LDFLAGS@

check_xmpp_SOURCES = check_xmpp.c $(top_builddir)/conflate.h
check_xmpp_CFLAGS = @CHECK_CFLAGS@
check_xmpp_LDADD = $(top_builddir)/libconflate.a @CHECK_LIBS@ @PROFILER_LDFLAGS@

check_persist_SOURCES = check_persist.c $(top_builddir)/conflate.h
check_persist_CFLAGS = @CHECK_CFLAGS@
check_persist_LDADD = $(top_builddir)/libconflate.a @CHECK_LIBS@ @PROFILER_LDFLAGS@
