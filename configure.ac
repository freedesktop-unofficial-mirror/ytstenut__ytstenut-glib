AC_PREREQ(2.59)

m4_define([yts_major_version], [0])
m4_define([yts_minor_version], [3])
m4_define([yts_micro_version], [0])
m4_define([yts_version],
  [yts_major_version.yts_minor_version.yts_micro_version])

AC_INIT(ytstenut, [yts_version],
  [https://bugs.freedesktop.org/enter_bug.cgi?product=Ytstenut])
AC_CONFIG_SRCDIR(ytstenut/ytstenut.h)
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build])
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I build")

AC_SUBST(YTS_MAJOR_VERSION, yts_major_version)
AC_SUBST(YTS_MINOR_VERSION, yts_minor_version)
AC_SUBST(YTS_MICRO_VERSION, yts_micro_version)

dnl PONDERING maybe rename to YTS_EPOCH
AC_SUBST(YTS_API_VERSION, [1])

dnl Version info for libraries = CURRENT:REVISION:AGE
dnl
dnl Within each x.y.*, ABI is maintained backward compatible.
dnl We strive for forward compatibility too, except that we sometimes
dnl add new functions.
dnl
m4_define([version_iface],
	m4_eval(100 * yts_major_version + yts_minor_version))

AC_SUBST([VERSION_INFO], [version_iface:yts_micro_version:0])

AM_INIT_AUTOMAKE(dist-bzip2)

AC_CONFIG_HEADERS(config.h)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_SUBST(ACLOCAL_AMFLAGS, $ACLOCAL_FLAGS)

AC_PROG_CC
AM_PROG_CC_C_O
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

AC_CHECK_TOOL([NM], [nm])

GTK_DOC_CHECK([1.11], [--flavour no-tmpl])

AM_CONDITIONAL([BUILD_GTK_DOC], [test "x$enable_gtk_doc" = "xyes" || test ! -f "autogen.sh"])

# prefixes for fixing gtk-doc references
GLIB_PREFIX="`$PKG_CONFIG --variable=prefix glib-2.0`"
AC_SUBST(GLIB_PREFIX)

TELEPATHY_VERSION=">= 0.11.7"
AC_SUBST(TELEPATHY_VERSION)

YTS_PC_MODULES="$YTS_PC_MODULES telepathy-glib $TELEPATHY_VERSION telepathy-ytstenut-glib >= 0.2.0 gio-unix-2.0 rest-0.7 glib-2.0 gobject-2.0"

AC_ARG_ENABLE([docs],
              [AC_HELP_STRING([--enable-docs=@<:@no/yes@:>@],
                              [Build optional documentation.])],
              [enable_docs=$enableval],
              [enable_docs=no])

AM_CONDITIONAL(ENABLE_DOCS, [test "x$enable_docs" = "xyes"])

AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug],
                              [Enable debugging code])],
              [ enable_debug=$enableval ]
)
AC_MSG_CHECKING([whether to enable debugging code])
AC_MSG_RESULT([$enable_debug])
if test "$enable_debug" != "no"; then
  AC_DEFINE([YTS_ENABLE_DEBUG], [1], [Defined when debugging code is enabled])
fi

have_introspection=no
AC_ARG_WITH(introspection,
  AC_HELP_STRING([--with-introspection],
                 [enable the use of GObject introspection]),
		 [with_introspection=$withval],
		 [with_introspection=auto])

if test x$with_introspection != xno; then
  PKG_CHECK_MODULES(INTROSPECTION, gobject-introspection-1.0, have_introspection=yes, have_introspection=no)
  if test x$have_introspection = xyes; then
    YTS_PC_MODULES="$YTS_PC_MODULES gobject-introspection-1.0"
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

PKG_CHECK_MODULES(YTS, $YTS_PC_MODULES)

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

AC_OUTPUT([
  Makefile
  docs/Makefile
  docs/reference/Makefile
  docs/reference/ytstenut/Makefile
  docs/reference/ytstenut/ytstenut-docs.xml
  examples/Makefile
  examples/video-profile/Makefile
  ytstenut/Makefile
  ytstenut/yts-version.h
  ytstenut.pc
  tests/Makefile
])

echo "
        ytstenut
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
