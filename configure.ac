AC_PREREQ(2.59)
m4_include([m4/version.m4])
AC_INIT(libconflate, VERSION_NUMBER, support@couchbase.com)
AC_CONFIG_SRCDIR([conflate.h])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_DISABLE_STATIC
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
ac_cv_func_malloc_0_nonnull=yes
ac_cv_func_realloc_0_nonnull=yes
PANDORA_CANONICAL_TARGET(less-warnings, force-gcc42)
PANDORA_REQUIRE_PTHREAD
PANDORA_REQUIRE_LIBCURL
PANDORA_HAVE_LIBSQLITE3
PANDORA_HAVE_LIBSTROPHE

AC_HEADER_STDBOOL

AC_ARG_WITH([check],
        [AS_HELP_STRING([--with-check=yes], [look for check tool for unit tests])])

if test "x$with_check" = "xno"; then
  AC_MSG_NOTICE([skipping check unit-test dependency])
else
  PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
fi

AC_ARG_WITH([rest],
  [AS_HELP_STRING([--without-rest],
    [Use XMPP instead of REST (yes|no) @<:@default=no@:>@])],
  [with_rest=$withval],
  [with_rest=yes])

AC_ARG_WITH([sqlite],
  [AS_HELP_STRING([--with-sqlite], [Use sqlite to store configs locally @<:@default=no@:>@])],
  [with_sqlite=$withval],
  [with_sqlite=no])

AS_IF([test "x$with_sqlite" = "xyes" ],
      [ AS_IF([test "x${ac_cv_libsqlite3}" != "xyes"],
               AC_MSG_ERROR([libsqlite3 is required for ${PACKAGE}]))
       AM_CFLAGS="$AM_CFLAGS -DCONFLATE_USE_SQLITE"
       LIBCONFLATE_LINKFLAGS="$LTLIBSQLITE3"
      ],
      [AM_CFLAGS="$AM_CFLAGS -DCONFLATE_NO_SQLITE"])

AS_IF([test "x$with_rest" = "xyes"],
      [AM_CFLAGS="$AM_CFLAGS -DCONFLATE_USE_REST"],
      [AS_IF([test "x${ac_cv_libstrophe}" != "xyes"],
              AC_MSG_ERROR([libstrophe is required for ${PACKAGE}]))
       AM_CFLAGS="$AM_CFLAGS -DCONFLATE_USE_XMPP"
       LIBCONFLATE_LINKFLAGS="$LIBCONFLATE_LINKFLAGS $LTLIBSTROPHE"
      ])

AC_SUBST(LIBCONFLATE_LINKFLAGS)

AC_CHECK_HEADERS_ONCE([syslog.h])
AC_CHECK_FUNCS_ONCE(strsep)

AH_BOTTOM([
/* There is a problem with some of the macros in typecheck-gcc.h that
** generates warnings with -Wlogical-op. Just disable the checks instead
*/
#define __CURL_TYPECHECK_GCC_H 1

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#define pause(a) while (1) { Sleep(100000); }
#define sleep(a) Sleep(a * 1000)
#endif
])

AC_CONFIG_FILES(Makefile tests/Makefile libconflate.pc)
AC_OUTPUT
