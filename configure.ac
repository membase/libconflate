AC_PREREQ(2.52)
AC_INIT(libconflate, 1.0, dustin@spy.net)
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR(conflate.h)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AC_PROG_LIBTOOL

AC_PROG_CC
AC_PROG_CC_C99

dnl **********************************************************************
dnl DETECT_ICC ([ACTION-IF-YES], [ACTION-IF-NO])
dnl
dnl check if this is the Intel ICC compiler, and if so run the ACTION-IF-YES
dnl sets the $ICC variable to "yes" or "no"
dnl **********************************************************************
AC_DEFUN([DETECT_ICC],
[
    ICC="no"
    AC_MSG_CHECKING([for icc in use])
    if test "$GCC" = "yes"; then
       dnl check if this is icc acting as gcc in disguise
       AC_EGREP_CPP([^__INTEL_COMPILER], [__INTEL_COMPILER],
         AC_MSG_RESULT([no])
         [$2],
         AC_MSG_RESULT([yes])
         [$1]
         ICC="yes")
    else
       AC_MSG_RESULT([no])
       [$2]
    fi
])

DETECT_ICC([], [])

dnl **********************************************************************
dnl DETECT_SUNCC ([ACTION-IF-YES], [ACTION-IF-NO])
dnl
dnl check if this is the Sun Studio compiler, and if so run the ACTION-IF-YES
dnl sets the $SUNCC variable to "yes" or "no"
dnl **********************************************************************
AC_DEFUN([DETECT_SUNCC],
[
    SUNCC="no"
    AC_MSG_CHECKING([for Sun cc in use])
    AC_EGREP_CPP([^__SUNPRO_C], [__SUNPRO_C],
         AC_MSG_RESULT([no])
         [$2],
         AC_MSG_RESULT([yes])
         [$1]
         SUNCC="yes")
])

AS_IF(test "$GCC" != "yes", DETECT_SUNCC([CFLAGS="-mt $CFLAGS"], []))

AM_PROG_CC_C_O
AC_PROG_INSTALL

AC_SUBST(PROFILER_LDFLAGS)

AC_ARG_ENABLE(coverage,
  [AS_HELP_STRING([--enable-coverage],[Enable code coverage])])

if test "x$enable_coverage" == "xyes"; then
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

AC_SEARCH_LIBS(pthread_create, pthread,
                               [], AC_MSG_ERROR([Can't enable pthreads.]))

AC_ARG_WITH([curl],
        [AS_HELP_STRING([--with-curl=yes], [search libs for curl])])

if test "x$with_curl" != "xno"; then
  AC_SEARCH_LIBS(curl_global_init, curl, [],
                               AC_MSG_ERROR([Can't find curl]))
else
  AC_MSG_NOTICE([skipping search for curl library])
fi

AC_ARG_WITH([check],
        [AS_HELP_STRING([--with-check=yes], [look for check tool for unit tests])])

if test "x$with_check" = "xno"; then
  AC_MSG_NOTICE([skipping check unit-test dependency])
else
  PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
fi

AC_HEADER_STDBOOL

AC_ARG_WITH([rest],
  [AS_HELP_STRING([--without-rest],
    [Use XMPP instead of REST (yes|no) @<:@default=no@:>@])],
  [with_rest=$withval],
  [with_rest=yes])

AC_ARG_WITH([sqlite],
        [AS_HELP_STRING([--with-sqlite=no], [use sqlite to store configs locally])])

AC_ARG_WITH([bundled-libstrophe],
        [AS_HELP_STRING([--with-bundled-libstrophe=no], [force building with bundled libstrophe])])

if test "x$with_sqlite" = "xyes"; then
  CFLAGS="$CFLAGS -DCONFLATE_USE_SQLITE"
  AC_SEARCH_LIBS(sqlite3_open, sqlite3, [],
                               AC_MSG_ERROR([Can't locate sqlite3]))
else
  CFLAGS="$CFLAGS -DCONFLATE_NO_SQLITE"
fi

if test "x$with_rest" = "xyes"; then
  CFLAGS="$CFLAGS -DCONFLATE_USE_REST"
else
  CFLAGS="$CFLAGS -DCONFLATE_USE_XMPP"

  if test "x$with_bundled_libstrophe" = "xyes"; then
    libstrophe_pkg_missing=yes
  else
    PKG_CHECK_MODULES([libstrophe], [libstrophe], [], [
      libstrophe_pkg_missing=yes
    ])
  fi

  if test "x$libstrophe_pkg_missing" = "xyes"; then
    if test -f "$srcdir/libstrophe/configure"; then
      if test "x$with_bundled_libstrophe" = "xyes"; then
        AC_MSG_NOTICE([forced use of bundled libstrophe])
      else
        AC_MSG_NOTICE([will use bundled libstrophe])
      fi
      AC_CONFIG_SUBDIRS([libstrophe])
      libstrophe_CFLAGS='-I$(top_srcdir)/libstrophe'
      libstrophe_LIBS="-Llibstrophe -lstrophe"
      AC_SUBST([MAYBE_LIBSTROPHE], [libstrophe])
    else
      if test "x$with_bundled_libstrophe" = "xyes"; then
        AC_MSG_FAILURE([use of bundled libstrophe is requested, but libstrophe/configure is missing])
      fi
      AC_MSG_CHECKING([for old version of libstrophe])
      AC_CHECK_HEADER(strophe.h,
              [], AC_MSG_ERROR([Ensure strophe.h is available.]))
      AC_SEARCH_LIBS(xmpp_initialize, strophe,
                                    [], AC_MSG_ERROR([Can't find libstrophe.]))
    fi
  fi
fi

AM_CONDITIONAL([BUILD_BUNDLED_LIBSTROPHE], [test "x$build_bundled_libstrophe" = xyes])

AC_C_CONST

dnl Let the compiler be a bit more picky. Please note that you cannot
dnl specify these flags to the compiler before AC_CHECK_FUNCS, because
dnl the test program will generate a compilation warning and hence fail
dnl to detect the function ;-)
dnl Lines borrowed from memcached.
if test "$ICC" = "yes"
then
   dnl ICC trying to be gcc.
   CFLAGS="$CFLAGS -w2 -Wcheck -Wall -Werror"
elif test "$GCC" = "yes"
then
  CFLAGS="$CFLAGS -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls"
elif test "$SUNCC" = "yes"
then
  CFLAGS="$CFLAGS -errfmt=error -erroff=E_ATTRIBUTE_UNKNOWN"
fi

AC_CONFIG_FILES(Makefile tests/Makefile libconflate.pc)
AC_OUTPUT
