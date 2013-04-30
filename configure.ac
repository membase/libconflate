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

AC_HEADER_STDBOOL

AC_ARG_WITH([rest],
  [AS_HELP_STRING([--without-rest],
    [Use XMPP instead of REST (yes|no) @<:@default=no@:>@])],
  [with_rest=$withval],
  [with_rest=yes])

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
