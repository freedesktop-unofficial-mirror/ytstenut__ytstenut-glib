m4_define([ytsg_major_version], [0])
m4_define([ytsg_minor_version], [3])
m4_define([ytsg_micro_version], [0])
m4_define([ytsg_version],
          [ytsg_major_version.ytsg_minor_version.ytsg_micro_version])

m4_define([ytsg_api_version], [1.0])

# increase the interface age of 2 for each release
# set to 0 if the API changes
m4_define([ytsg_interface_age], [0])
m4_define([ytsg_binary_age], [m4_eval(100 * ytsg_minor_version + ytsg_micro_version)])

m4_define([ytsg_lt_current], [m4_eval(100 * ytsg_minor_version + ytsg_micro_version - ytsg_interface_age)])
m4_define([ytsg_lt_revision], [ytsg_interface_age])
m4_define([ytsg_lt_age], [m4_eval(ytsg_binary_age - ytsg_interface_age)])

# defaults
m4_define([default_enable_cache],    [no])

AC_PREREQ([2.59])

AC_INIT([ytstenut-glib], [ytsg_version], [http://ytstenut.meego.com])
AC_CONFIG_MACRO_DIR([build/autotools])

AC_CONFIG_SRCDIR(ytstenut-glib/ytstenut-glib.h)

AM_INIT_AUTOMAKE([tar-ustar dist-bzip2])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])],[])

YTSG_MAJOR_VERSION=ytsg_major_version
YTSG_MINOR_VERSION=ytsg_minor_version
YTSG_MICRO_VERSION=ytsg_micro_version
YTSG_VERSION=ytsg_version
YTSG_API_VERSION=ytsg_api_version
AC_SUBST(YTSG_MAJOR_VERSION)
AC_SUBST(YTSG_MINOR_VERSION)
AC_SUBST(YTSG_MICRO_VERSION)
AC_SUBST(YTSG_VERSION)
AC_SUBST(YTSG_API_VERSION)

AM_CONFIG_HEADER([config.h])

AC_PROG_CC
AM_PROG_CC_C_O
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

GTK_DOC_CHECK([1.11], [--flavour no-tmpl])

AM_CONDITIONAL([BUILD_GTK_DOC], [test "x$enable_gtk_doc" = "xyes" || test ! -f "autogen.sh"])

# prefixes for fixing gtk-doc references
GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
AC_SUBST(GLIB_PREFIX)

TELEPATHY_VERSION=">= 0.11.7"
AC_SUBST(TELEPATHY_VERSION)

YTSG_PC_MODULES="$YTSG_PC_MODULES telepathy-glib $TELEPATHY_VERSION telepathy-ytstenut-glib gio-unix-2.0 rest-0.7 glib-2.0 gobject-2.0"

AC_ARG_ENABLE([docs],
              [AC_HELP_STRING([--enable-docs=@<:@no/yes@:>@],
                              [Build optional documentation.])],
              [enable_docs=$enableval],
              [enable_docs=no])

AM_CONDITIONAL(ENABLE_DOCS, [test "x$enable_docs" = "xyes"])

m4_define([debug_default], [m4_if(m4_eval(ytsg_minor_version % 2), [1], [yes], [minimum])])
AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
                              [Control Ytstenut debugging level @<:@default=debug_default@:>@])],
              [],
              [enable_debug=debug_default])

AS_CASE([$enable_debug],

        [yes],
        [
          test "$cflags_set" = set || CFLAGS="$CFLAGS -g -O0"
          YTSG_DEBUG_CFLAGS="-DYTSG_ENABLE_DEBUG"
        ],

        [minimum],
        [YTSG_DEBUG_CFLAGS="-DYTSG_ENABLE_DEBUG -DG_DISABLE_CAST_CHECKS"],

        [no],
        [YTSG_DEBUG_CFLAGS="-DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"],

        [AC_MSG_ERROR([Unknown argument for --enable-debug])]
)

AC_SUBST(YTSG_DEBUG_CFLAGS)

have_introspection=no
AC_ARG_WITH(introspection,
  AC_HELP_STRING([--with-introspection],
                 [enable the use of GObject introspection]),
		 [with_introspection=$withval],
		 [with_introspection=auto])

if test x$with_introspection != xno; then
  PKG_CHECK_MODULES(INTROSPECTION, gobject-introspection-1.0, have_introspection=yes, have_introspection=no)
  if test x$have_introspection = xyes; then
    YTSG_PC_MODULES="$YTSG_PC_MODULES gobject-introspection-1.0"
    AC_DEFINE(HAVE_INTROSPECTION, 1, [Define if GObject introspection is available])
    G_IR_SCANNER=`$PKG_CONFIG --variable=g_ir_scanner gobject-introspection-1.0`
    AC_SUBST(G_IR_SCANNER)
    G_IR_COMPILER=`$PKG_CONFIG --variable=g_ir_compiler gobject-introspection-1.0`
    AC_SUBST(G_IR_COMPILER)
    G_IR_GENERATE=`$PKG_CONFIG --variable=g_ir_generate gobject-introspection-1.0`
    AC_SUBST(G_IR_GENERATE)
    GIRDIR=`$PKG_CONFIG --variable=girdir gobject-introspection-1.0`
    AC_SUBST(GIRDIR)
    TYPELIBDIR="$($PKG_CONFIG --variable=typelibdir gobject-introspection-1.0)"
    AC_SUBST(TYPELIBDIR)
  else
    if test x$with_introspection = xyes; then
       AC_ERROR($INTROSPECTION_PKG_ERRORS)
    fi
  fi
fi
AM_CONDITIONAL(WITH_INTROSPECTION, test "$have_introspection" = "yes")

PKG_CHECK_MODULES(YTSG, $YTSG_PC_MODULES)

AC_ARG_ENABLE([profile],
              [AC_HELP_STRING([--enable-profile],
                              [Enable profiling])],
              [],
              [enable_profile=no])

CFLAGS="$CFLAGS -Wall"

AS_IF([test "x$enable_profile" = "xyes"], [CFLAGS="$CFLAGS -pg"])

# glib-genmarshal
GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)
GLIB_MKENUMS=`$PKG_CONFIG --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_MKENUMS)

YTSG_LT_CURRENT=ytsg_lt_current
YTSG_LT_REV=ytsg_lt_revision
YTSG_AGE=ytsg_lt_age
YTSG_LT_VERSION="$YTSG_LT_CURRENT:$YTSG_LT_REV:$YTSG_LT_AGE"

YTSG_LT_LDFLAGS="-version-info $YTSG_LT_VERSION"

AC_SUBST(YTSG_LT_VERSION)
AC_SUBST(YTSG_LT_LDFLAGS)

AC_OUTPUT([
Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/ytstenut-glib/Makefile
docs/reference/ytstenut-glib/ytstenut-glib-docs.xml
examples/Makefile
examples/video-profile/Makefile
ytstenut-glib/Makefile
ytstenut-glib/ytsg-version.h
ytstenut-glib/ytstenut-glib.pc
tests/Makefile
])

echo "
        ytstenut-glib
        ===================================================

        Configuration:
            Prefix           :    ${prefix}
	    Debug            :    ${enable_debug}
	    Profile          :    ${enable_profile}
            API Reference    :    ${enable_gtk_doc}
            Introspection    :    ${have_introspection}

	    CFLAGS: ${CFLAGS}

        Type make to build.
"
