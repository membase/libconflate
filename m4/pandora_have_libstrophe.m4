dnl  Copyright (C) 2010 NorthScale
dnl This file is free software; NorthScale
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
AC_DEFUN([_PANDORA_SEARCH_LIBSTROPHE],[
  AC_REQUIRE([AC_LIB_PREFIX])

  dnl --------------------------------------------------------------------
  dnl  Check for libstrophe
  dnl --------------------------------------------------------------------

  AC_ARG_ENABLE([libstrophe],
    [AS_HELP_STRING([--disable-libstrophe],
      [Build with libstrophe support @<:@default=on@:>@])],
    [ac_enable_libstrophe="$enableval"],
    [ac_enable_libstrophe="yes"])

  AS_IF([test "x$ac_enable_libstrophe" = "xyes"],[
    AC_LIB_HAVE_LINKFLAGS(strophe,,[
      #include <stdio.h>
      #include <strophe.h>
    ],[
      xmpp_conn_t *conn = NULL;
      xmpp_ctx_t *ctx = NULL;
      xmpp_stanza_t *reply = NULL;
      xmpp_initialize();
    ])
  ],[
    ac_cv_libstrophe="no"
  ])

  AM_CONDITIONAL(HAVE_LIBSTROPHE, [test "x${ac_cv_libstrophe}" = "xyes"])
])

AC_DEFUN([PANDORA_HAVE_LIBSTROPHE],[
  AC_REQUIRE([_PANDORA_SEARCH_LIBSTROPHE])
])

AC_DEFUN([PANDORA_REQUIRE_LIBSTROPHE],[
  AC_REQUIRE([PANDORA_HAVE_LIBSTROPHE])
  AS_IF([test x$ac_cv_libstrophe = xno],
      AC_MSG_ERROR([libstrophe is required for ${PACKAGE}]))
])
