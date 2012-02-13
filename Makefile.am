ACLOCAL_AMFLAGS = -I build

DISTCHECK_CONFIGURE_FLAGS = \
  --disable-silent-rules \
  --enable-gtk-doc \
  $(NULL)

SUBDIRS = ytstenut docs tests examples

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ytstenut-@YTS_API_VERSION@.pc

DISTCLEANFILES = \
  ytstenut.pc \
  ytstenut-@YTS_API_VERSION@.pc \
  $(NULL)

EXTRA_DIST = \
  ytstenut.pc.in \
  $(NULL)

GITIGNOREFILES = \
  aclocal.m4 \
  build \
  config.h.in \
  gtk-doc.make \
  omf.make \
  xmldocs.make \
  $(NULL)

ytstenut-@YTS_API_VERSION@.pc: ytstenut.pc
	$(AM_V_GEN)cp $< $@

-include $(top_srcdir)/git.mk
