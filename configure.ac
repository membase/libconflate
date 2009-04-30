AC_PREREQ(2.52)
AC_INIT(memcached, 1.0, dustin@spy.net)
AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR(conflate.h)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_RANLIB

AM_PROG_CC_C_O
AC_PROG_INSTALL

AC_SUBST(PROFILER_LDFLAGS)

AC_ARG_ENABLE(coverage,
  [AS_HELP_STRING([--disable-coverage],[Disable code coverage])])

if test "x$enable_coverage" != "xno"; then
   if test "$ICC" = "yes"
   then
      :
      dnl ICC trying to be gcc, but not well
   elif test "$GCC" = "yes"
   then
      AC_PATH_PROG([PROFILER], [gcov], "no", [$PATH])
      if test "x$PROFILER" != "xno"; then
         PROFILER_FLAGS="-fprofile-arcs -ftest-coverage"
         PROFILER_LDFLAGS="-lgcov"
      fi
   elif test "$SUNCC" = "yes"
   then
      AC_PATH_PROG([PROFILER], [tcov], "no", [$PATH])
      if test "x$PROFILER" != "xno"; then
         PROFILER_FLAGS=-xprofile=tcov
      fi
   fi
fi
AC_SUBST(PROFILER_FLAGS)

AC_SEARCH_LIBS(sqlite3_open, sqlite3)
AC_SEARCH_LIBS(res_9_query, resolv)
AC_SEARCH_LIBS(xmpp_initialize, strophe,
                                [], AC_MSG_ERROR([Can't find libstrophe.]))
AC_SEARCH_LIBS(XML_ParserCreate, expat)

AC_SEARCH_LIBS(pthread_create, pthread,
                               [], AC_MSG_ERROR([Can't enable pthreads.]))
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

AC_HEADER_STDBOOL

AC_CHECK_HEADER(strophe.h,
        [], AC_MSG_ERROR([Ensure strophe.h is available.]))

AC_C_CONST

AC_CONFIG_FILES(Makefile tests/Makefile)
AC_OUTPUT
