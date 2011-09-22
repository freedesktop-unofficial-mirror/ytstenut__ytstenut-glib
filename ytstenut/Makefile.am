SUBDIRS=
BUILT_SOURCES=
CLEANFILES=
BUILT_GIRSOURCES=
INCLUDES_FOR_GIR=-I..

AM_CPPFLAGS = \
  -I$(top_srcdir) \
	-DG_DISABLE_DEPRECATED \
	-DG_LOG_DOMAIN=\"YtstenutGlib\" \
	$(NULL)

AM_CFLAGS = $(YTS_CFLAGS)

source_h =						\
		$(srcdir)/empathy-tp-file.h		\
		$(srcdir)/yts-capability.h			\
		$(srcdir)/yts-caps.h			\
		$(srcdir)/yts-client.h			\
		$(srcdir)/yts-contact.h		\
		$(srcdir)/yts-debug.h			\
		$(srcdir)/yts-error.h			\
		$(srcdir)/yts-main.h			\
		$(srcdir)/yts-metadata.h		\
		$(srcdir)/yts-metadata-service.h	\
		$(srcdir)/yts-roster.h			\
		$(srcdir)/yts-service.h		\
		$(srcdir)/yts-status.h			\
		$(srcdir)/yts-types.h			\
		$(srcdir)/yts-version.h		\
		\
		$(srcdir)/yts-capability-status.h \
		$(srcdir)/yts-proxy.h \
		$(srcdir)/yts-proxy-service.h \
		$(srcdir)/yts-service-adapter.h \
		\
		$(srcdir)/video-profile/yts-vp-content.h \
		$(srcdir)/video-profile/yts-vp-playable.h \
		$(srcdir)/video-profile/yts-vp-player.h \
		$(srcdir)/video-profile/yts-vp-query.h \
		$(srcdir)/video-profile/yts-vp-transcript.h \
		$(srcdir)/video-profile/yts-vp-transfer.h \
		$(srcdir)/video-profile/yts-vp-transmission.h \
		$(NULL)

source_c =						\
		$(srcdir)/empathy-tp-file.c		\
		$(srcdir)/yts-capability.c			\
		$(srcdir)/yts-client.c			\
		$(srcdir)/yts-contact.c		\
		$(srcdir)/yts-enum-types.c		\
		$(srcdir)/yts-error.c			\
		$(srcdir)/yts-main.c			\
		$(srcdir)/yts-marshal.c		\
		$(srcdir)/yts-message.c		\
		$(srcdir)/yts-metadata.c		\
		$(srcdir)/yts-metadata-service.c	\
		$(srcdir)/yts-roster.c			\
		$(srcdir)/yts-service.c		\
		$(srcdir)/yts-status.c			\
		\
		$(srcdir)/yts-adapter-factory.c \
		$(srcdir)/yts-capability-status.c \
		$(srcdir)/yts-error-message.c \
		$(srcdir)/yts-event-message.c \
		$(srcdir)/yts-factory.c \
		$(srcdir)/yts-invocation-message.c \
		$(srcdir)/yts-proxy.c \
		$(srcdir)/yts-proxy-factory.c \
		$(srcdir)/yts-proxy-service.c \
		$(srcdir)/yts-response-message.c \
		$(srcdir)/yts-service-adapter.c \
		$(srcdir)/yts-service-factory.c \
		\
		$(srcdir)/profile/yts-profile.c \
		$(srcdir)/profile/yts-profile-impl.c \
		$(srcdir)/profile/yts-profile-proxy.c \
		$(srcdir)/profile/yts-profile-adapter.c \
		\
		$(srcdir)/video-profile/yts-vp-content.c \
		$(srcdir)/video-profile/yts-vp-playable.c \
		$(srcdir)/video-profile/yts-vp-playable-proxy.c \
		$(srcdir)/video-profile/yts-vp-player.c \
		$(srcdir)/video-profile/yts-vp-player-adapter.c \
		$(srcdir)/video-profile/yts-vp-player-proxy.c \
		$(srcdir)/video-profile/yts-vp-query.c \
		$(srcdir)/video-profile/yts-vp-transcript.c \
		$(srcdir)/video-profile/yts-vp-transcript-adapter.c \
		$(srcdir)/video-profile/yts-vp-transcript-proxy.c \
		$(srcdir)/video-profile/yts-vp-transfer.c \
		$(srcdir)/video-profile/yts-vp-transmission.c \
		$(NULL)

private_h = \
		$(srcdir)/yts-adapter-factory.h \
		$(srcdir)/yts-client-internal.h \
		$(srcdir)/yts-contact-internal.h \
		$(srcdir)/yts-contact-internal.h \
		$(srcdir)/yts-factory.h \
		$(srcdir)/yts-metadata-internal.h \
		$(srcdir)/yts-metadata-service-internal.h \
		$(srcdir)/yts-proxy-factory.h \
		$(srcdir)/yts-proxy-internal.h \
		$(srcdir)/yts-proxy-service-internal.h \
		$(srcdir)/yts-roster-internal.h \
		$(srcdir)/yts-service-factory.h \
		\
		$(srcdir)/yts-message.h		\
		$(srcdir)/yts-error-message.h \
		$(srcdir)/yts-event-message.h \
		$(srcdir)/yts-invocation-message.h \
		$(srcdir)/yts-response-message.h \
		\
		$(srcdir)/profile/yts-profile.h \
		$(srcdir)/profile/yts-profile-impl.h \
		$(srcdir)/profile/yts-profile-proxy.h \
		$(srcdir)/profile/yts-profile-adapter.h \
		$(srcdir)/profile/yts-service-internal.h \
		\
		$(srcdir)/video-profile/yts-vp-playable-proxy.h \
		$(srcdir)/video-profile/yts-vp-player-adapter.h \
		$(srcdir)/video-profile/yts-vp-player-proxy.h \
		$(srcdir)/video-profile/yts-vp-transcript-adapter.h \
		$(srcdir)/video-profile/yts-vp-transcript-proxy.h \
		$(NULL)

libytstenut_@YTS_API_VERSION@_la_SOURCES  = 	$(source_h) 	\
							$(private_h)	\
							$(source_c)

libytstenut_@YTS_API_VERSION@_la_DEPENDENCIES = \
  ytstenut.sym \
  $(NULL)

STAMPS = stamp-marshal.h stamp-yts-enum-types.h

ENUMS = yts-enum-types.h \
	yts-enum-types.c

yts-enum-types.h: stamp-yts-enum-types.h
	@true
stamp-yts-enum-types.h: $(source_h) Makefile
	$(QUIET_GEN)( $(GLIB_MKENUMS) \
	  	--template $(srcdir)/yts-enum-types.h.in \
	  $(source_h) ) >> xgen-ceth && \
	(cmp -s xgen-ceth yts-enum-types.h || cp xgen-ceth yts-enum-types.h) && \
	rm -f xgen-ceth && \
	echo timestamp > $(@F)

yts-enum-types.c: yts-enum-types.h
	$(QUIET_GEN)( $(GLIB_MKENUMS) \
	  	--template $(srcdir)/yts-enum-types.c.in \
	  $(source_h) ) >> xgen-cetc && \
	cp xgen-cetc yts-enum-types.c && \
	rm -f xgen-cetc

MARSHALS = yts-marshal.c yts-marshal.h

yts-marshal.h: stamp-marshal.h
	@true
stamp-marshal.h: marshal.list
	$(QUIET_GEN)$(GLIB_GENMARSHAL) \
		--prefix=yts_marshal \
		--header \
	  $(srcdir)/marshal.list > xgen-cmh \
	&& (cmp -s xgen-cmh yts-marshal.h || cp xgen-cmh yts-marshal.h ) \
	&& rm -f xgen-cmh \
	&& echo timestamp > $(@F)

yts-marshal.c: yts-marshal.h Makefile
	$(QUIET_GEN)( echo "#include \"yts-marshal.h\"" ; \
	  $(GLIB_GENMARSHAL) \
	  	--prefix=yts_marshal \
		--body \
	  $(srcdir)/marshal.list --body ) >> xgen-cmc \
	&& cp xgen-cmc yts-marshal.c \
	&& rm -f xgen-cmc

libytstenut_@YTS_API_VERSION@_la_LDFLAGS = \
  -version-info $(VERSION_INFO) \
  -export-symbols $(srcdir)/ytstenut.sym \
  -no-undefined \
  $(NULL)

libytstenut_@YTS_API_VERSION@_la_LIBADD = \
  $(YTS_LIBS) \
  $(NULL)

lib_LTLIBRARIES = libytstenut-@YTS_API_VERSION@.la

ytstenutheadersdir = $(includedir)/ytstenut-$(YTS_API_VERSION)/ytstenut
nobase_ytstenutheaders_HEADERS =		\
	$(source_h)			\
	$(srcdir)/yts-enum-types.h	\
	$(srcdir)/yts-marshal.h	\
	$(srcdir)/ytstenut.h	\
	$(NULL)

if WITH_INTROSPECTION
Yts-@YTS_API_VERSION@.gir: $(G_IR_SCANNER) libytstenut-@YTS_API_VERSION@.la
	$(QUIET_GEN) $(G_IR_SCANNER) -v \
		--namespace=Yts --nsversion=@YTS_API_VERSION@ \
		--strip-prefix=Yts \
		$(INCLUDES_FOR_GIR) \
		$(YTS_CFLAGS) \
		$(AM_CPPFLAGS) \
		--add-init-section="yts_init(0,NULL);"\
		--c-include='ytstenut/ytstenut.h' \
		--include=GObject-2.0 \
		--include=TelepathyGLib-0.12 \
		--library=libytstenut-@YTS_API_VERSION@.la \
		--libtool="$(top_builddir)/libtool" \
	        --pkg gobject-2.0 \
	        --output $@ \
		$(nobase_ytstenutheaders_HEADERS) \
		$(source_c)

BUILT_GIRSOURCES += Yts-@YTS_API_VERSION@.gir

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

CLEANFILES += $(BUILT_SOURCES) $(STAMPS)

EXTRA_DIST =	marshal.list 		\
		yts-enum-types.h.in	\
		yts-enum-types.c.in	\
		yts-version.h.in	\
		ytstenut.sym \
		$(NULL)

DISTCLEANFILES= yts-version.h		\
		$(NULL)

# PONDERING should only run on platforms we know it works.
# Use "nm" to get the public symbols from libytstenut, and then
# check that every symbol from ytstenut.sym actually shows up in the library.
all-local:
	$(AM_V_GEN) b=`tput bold` && \
	n=`tput sgr0` && \
	NM_CMD="$(NM) --defined-only --extern-only" && \
	exported_syms=`$${NM_CMD} ".libs/libytstenut-$(YTS_API_VERSION).so" | cut -d' ' -f3` && \
	for sym in `cat $(srcdir)/ytstenut.sym`; do \
		if [ "`echo $${exported_syms} | grep $${sym}`" = "" ]; then \
			echo "$${b}Error: $${sym} exported in ytstenut.sym but not found in libytstenut.$${n}" && \
			exit 1; \
		fi \
	done

-include $(top_srcdir)/git.mk
