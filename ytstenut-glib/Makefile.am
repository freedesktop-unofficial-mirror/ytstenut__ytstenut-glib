SUBDIRS=
BUILT_SOURCES=
CLEANFILES=
BUILT_GIRSOURCES=
INCLUDES_FOR_GIR=-I..

AM_CPPFLAGS = 					\
	-DG_DISABLE_DEPRECATED 			\
	-DG_DISABLE_SINGLE_INCLUDES		\
	-DG_LOG_DOMAIN=\"YtstenutGlib\" 	\
	$(YTSG_DEBUG_CFLAGS)		\
	$(NULL)

AM_CFLAGS = $(YTSG_CFLAGS)

source_h =						\
		$(srcdir)/ytsg-caps.h			\
		$(srcdir)/ytsg-client.h			\
		$(srcdir)/ytsg-contact.h		\
		$(srcdir)/ytsg-debug.h			\
		$(srcdir)/ytsg-main.h			\
		$(srcdir)/ytsg-message.h		\
		$(srcdir)/ytsg-metadata.h		\
		$(srcdir)/ytsg-metadata-service.h	\
		$(srcdir)/ytsg-roster.h			\
		$(srcdir)/ytsg-service.h		\
		$(srcdir)/ytsg-status.h			\
		$(srcdir)/ytsg-version.h		\
		$(NULL)

source_c =						\
		$(srcdir)/ytsg-client.c			\
		$(srcdir)/ytsg-contact.c		\
		$(srcdir)/ytsg-enum-types.c		\
		$(srcdir)/ytsg-main.c			\
		$(srcdir)/ytsg-message.c		\
		$(srcdir)/ytsg-metadata.c		\
		$(srcdir)/ytsg-metadata-service.c	\
		$(srcdir)/ytsg-roster.c			\
		$(srcdir)/ytsg-service.c		\
		$(srcdir)/ytsg-status.c			\
		$(NULL)


private_h =
		$(NULL)

libytstenut_glib_@YTSG_API_VERSION@_la_SOURCES  = 	$(source_h) 	\
							$(private_h)	\
							$(source_c)

STAMPS = stamp-marshal.h stamp-ytsg-enum-types.h

ENUMS = ytsg-enum-types.h \
	ytsg-enum-types.c

ytsg-enum-types.h: stamp-ytsg-enum-types.h
	@true
stamp-ytsg-enum-types.h: $(source_h) Makefile
	$(QUIET_GEN)( $(GLIB_MKENUMS) \
	  	--template $(srcdir)/ytsg-enum-types.h.in \
	  $(source_h) ) >> xgen-ceth && \
	(cmp -s xgen-ceth ytsg-enum-types.h || cp xgen-ceth ytsg-enum-types.h) && \
	rm -f xgen-ceth && \
	echo timestamp > $(@F)

ytsg-enum-types.c: ytsg-enum-types.h
	$(QUIET_GEN)( $(GLIB_MKENUMS) \
	  	--template $(srcdir)/ytsg-enum-types.c.in \
	  $(source_h) ) >> xgen-cetc && \
	cp xgen-cetc ytsg-enum-types.c && \
	rm -f xgen-cetc

MARSHALS = ytsg-marshal.c ytsg-marshal.h

ytsg-marshal.h: stamp-marshal.h
	@true
stamp-marshal.h: marshal.list
	$(QUIET_GEN)$(GLIB_GENMARSHAL) \
		--prefix=ytsg_marshal \
		--header \
	  $(srcdir)/marshal.list > xgen-cmh \
	&& (cmp -s xgen-cmh ytsg-marshal.h || cp xgen-cmh ytsg-marshal.h ) \
	&& rm -f xgen-cmh \
	&& echo timestamp > $(@F)

ytsg-marshal.c: ytsg-marshal.h Makefile
	$(QUIET_GEN)( echo "#include \"ytsg-marshal.h\"" ; \
	  $(GLIB_GENMARSHAL) \
	  	--prefix=ytsg_marshal \
		--body \
	  $(srcdir)/marshal.list --body ) >> xgen-cmc \
	&& cp xgen-cmc ytsg-marshal.c \
	&& rm -f xgen-cmc

libytstenut_glib_@YTSG_API_VERSION@_la_LDFLAGS  = $(YTSG_LT_LDFLAGS) \
						-no-undefined
libytstenut_glib_@YTSG_API_VERSION@_la_LIBADD   = $(YTSG_LIBS)

lib_LTLIBRARIES = libytstenut-glib-@YTSG_API_VERSION@.la

ytstenutheadersdir = $(includedir)/ytstenut-$(YTSG_API_VERSION)/ytstenut
ytstenutheaders_HEADERS =		\
	$(source_h)			\
	$(srcdir)/ytsg-enum-types.h	\
	$(srcdir)/ytsg-marshal.h	\
	$(srcdir)/ytstenut-glib.h	\
	$(NULL)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ytstenut-glib.pc

if WITH_INTROSPECTION
Ytsg-@YTSG_API_VERSION@.gir: $(G_IR_SCANNER) libytstenut-glib-@YTSG_API_VERSION@.la
	$(QUIET_GEN) $(G_IR_SCANNER) -v \
		--namespace=Ytsg --nsversion=@YTSG_API_VERSION@ \
		--strip-prefix=Ytsg \
		$(INCLUDES_FOR_GIR) \
		$(YTSG_CFLAGS) \
		$(AM_CPPFLAGS) \
		--add-init-section="ytsg_init(0,NULL);"\
		--c-include='ytstenut-glib/ytstenut-glib.h' \
		--include=GObject-2.0 \
		--include=TelepathyGLib-0.12 \
		--library=libytstenut-glib-@YTSG_API_VERSION@.la \
		--libtool="$(top_builddir)/libtool" \
	        --pkg gobject-2.0 \
	        --output $@ \
		$(ytstenutheaders_HEADERS) \
		$(source_c)

BUILT_GIRSOURCES += Ytsg-@YTSG_API_VERSION@.gir

# G_IR_GIRDIR/G_IR_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/

typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(G_IR_COMPILER)
	$(QUIET_GEN) \
	LD_LIBRARY_PATH=.libs$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH} \
	$(G_IR_COMPILER) \
		--includedir=$(srcdir) \
		--includedir=. \
		$(G_IR_COMPILER_OPTS) $< -o $(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
endif # HAVE_INTROSPECTION

BUILT_SOURCES += $(MARSHALS) $(ENUMS)

CLEANFILES += $(BUILT_SOURCES) $(STAMPS) ytstenut-glib.pc

EXTRA_DIST =	marshal.list 		\
		ytsg-enum-types.h.in	\
		ytsg-enum-types.c.in	\
		ytsg-version.h.in	\
		ytstenut-glib.pc.in	\
		$(NULL)

DISTCLEANFILES= ytsg-version.h		\
		$(NULL)
