#!/bin/sh

mkdir -p m4
libtoolize

ACLOCAL=`which aclocal-1.10 || which aclocal-1.9 || which aclocal19 || which aclocal-1.7 || which aclocal17 || which aclocal-1.5 || which aclocal15 || which aclocal`
# ACLOCALFLAGS may need to be defined if you have macros in non-standard
# locations, i.e. MacOS
$ACLOCAL $ACLOCALFLAGS || exit 1

AUTOHEADER=${AUTOHEADER:-autoheader}
$AUTOHEADER || exit 1

AUTOMAKE=`which automake-1.10 || which automake-1.9 || which automake-1.7 || exit 1`
$AUTOMAKE --foreign --add-missing || automake --gnu --add-missing || exit 1

AUTOCONF=${AUTOCONF:-autoconf}
$AUTOCONF || exit 1

if (test -f libstrophe/bootstrap.sh) && !(test -f libstrophe/configure); then
    echo "libstrophe..."
    (cd libstrophe; ./bootstrap.sh)
fi
