#!/bin/bash
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="ytstenut-glib"
REQUIRED_AUTOMAKE_VERSION=1.10

(test -f $srcdir/configure.ac \
  && test -d $srcdir/ytstenut-glib) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level ytstenut-glib directory"
    exit 1
}

which gnome-autogen.sh || {
    echo "You need to install gnome-common from GNOME Subversion (or from"
    echo "your distribution's package manager)."
    exit 1
}
USE_GNOME2_MACROS=1 USE_COMMON_DOC_BUILD=yes . gnome-autogen.sh
