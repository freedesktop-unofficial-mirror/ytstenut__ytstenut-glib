
AM_CPPFLAGS = \
  -I$(top_srcdir) \
  -DG_LOG_DOMAIN=\"$(PACKAGE)\\0unspecified\" \
  -DG_DISABLE_DEPRECATED \
  $(NULL)

AM_CFLAGS = $(YTS_CFLAGS)

#
# Use automake names for headers and sources variables, since external tools
# like anjuta understand them.
#

EXTRA_LTLIBRARIES = libsrc.la libhdr.la libprv.la

# Public headers
libhdr_la_SOURCES = \
  $(srcdir)/yts-capability.h \
  $(srcdir)/yts-client.h \
  $(srcdir)/yts-contact.h \
  $(srcdir)/yts-file-transfer.h \
  $(srcdir)/yts-incoming-file.h \
  $(srcdir)/yts-outgoing-file.h \
  $(srcdir)/yts-roster.h \
  $(srcdir)/yts-service.h \
  \
  $(srcdir)/yts-proxy.h \
  $(srcdir)/yts-proxy-service.h \
  \
  $(srcdir)/video-profile/yts-vp-content.h \
  $(srcdir)/video-profile/yts-vp-playable.h \
  $(srcdir)/video-profile/yts-vp-player.h \
  $(srcdir)/video-profile/yts-vp-query.h \
  $(srcdir)/video-profile/yts-vp-transcript.h \
  $(srcdir)/video-profile/yts-vp-transfer.h \
  $(srcdir)/video-profile/yts-vp-transmission.h \
  $(NULL)

nodist_libhdr_la_SOURCES = \
  yts-version.h \
  $(NULL)

# Sources
libsrc_la_SOURCES = \
  $(srcdir)/ytstenut.c \
  \
  $(srcdir)/yts-capability.c \
  $(srcdir)/yts-client.c \
  $(srcdir)/yts-client-status.c \
  $(srcdir)/yts-contact.c \
  $(srcdir)/yts-contact-impl.c \
  $(srcdir)/yts-error.c \
  $(srcdir)/yts-message.c \
  $(srcdir)/yts-metadata.c \
  $(srcdir)/yts-roster.c \
  $(srcdir)/yts-roster-impl.c \
  $(srcdir)/yts-service.c \
  $(srcdir)/yts-service-impl.c \
  \
  $(srcdir)/yts-adapter-factory.c \
  $(srcdir)/yts-error-message.c \
  $(srcdir)/yts-event-message.c \
  $(srcdir)/yts-factory.c \
  $(srcdir)/yts-incoming-file.c \
  $(srcdir)/yts-invocation-message.c \
  $(srcdir)/yts-service-emitter.c \
  $(srcdir)/yts-file-transfer.c \
  $(srcdir)/yts-outgoing-file.c \
  $(srcdir)/yts-proxy.c \
  $(srcdir)/yts-proxy-factory.c \
  $(srcdir)/yts-proxy-service.c \
  $(srcdir)/yts-proxy-service-impl.c \
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

nodist_libsrc_la_SOURCES = \
  yts-enum-types.c \
  yts-marshal.c \
  $(NULL)

# Private headers
libprv_la_SOURCES = \
  $(srcdir)/ytstenut-internal.h \
  \
  $(srcdir)/yts-adapter-factory.h \
  $(srcdir)/yts-client-internal.h \
  $(srcdir)/yts-client-status.h \
  $(srcdir)/yts-contact-impl.h \
  $(srcdir)/yts-contact-internal.h \
  $(srcdir)/yts-error.h \
  $(srcdir)/yts-factory.h \
  $(srcdir)/yts-incoming-file-internal.h \
  $(srcdir)/yts-metadata-internal.h \
  $(srcdir)/yts-outgoing-file-internal.h \
  $(srcdir)/yts-proxy-factory.h \
  $(srcdir)/yts-proxy-internal.h \
  $(srcdir)/yts-proxy-service-impl.h \
  $(srcdir)/yts-proxy-service-internal.h \
  $(srcdir)/yts-roster-impl.h \
  $(srcdir)/yts-roster-internal.h \
  $(srcdir)/yts-service-adapter.h \
  $(srcdir)/yts-service-emitter.h \
  $(srcdir)/yts-service-factory.h \
  $(srcdir)/yts-service-impl.h \
  $(srcdir)/yts-service-internal.h \
  $(srcdir)/yts-xml.h \
  \
  $(srcdir)/yts-message.h \
  $(srcdir)/yts-metadata.h \
  $(srcdir)/yts-error-message.h \
  $(srcdir)/yts-event-message.h \
  $(srcdir)/yts-invocation-message.h \
  $(srcdir)/yts-response-message.h \
  \
  $(srcdir)/profile/yts-profile.h \
  $(srcdir)/profile/yts-profile-impl.h \
  $(srcdir)/profile/yts-profile-proxy.h \
  $(srcdir)/profile/yts-profile-adapter.h \
  \
  $(srcdir)/video-profile/yts-vp-playable-proxy.h \
  $(srcdir)/video-profile/yts-vp-player-adapter.h \
  $(srcdir)/video-profile/yts-vp-player-proxy.h \
  $(srcdir)/video-profile/yts-vp-transcript-adapter.h \
  $(srcdir)/video-profile/yts-vp-transcript-proxy.h \
  $(NULL)

#
# Libytstenut
#

lib_LTLIBRARIES = libytstenut-@YTS_API_VERSION@.la

libytstenut_@YTS_API_VERSION@_la_SOURCES = \
  $(libhdr_la_SOURCES) \
  $(libprv_la_SOURCES) \
  $(libsrc_la_SOURCES) \
  $(NULL)

nodist_libytstenut_@YTS_API_VERSION@_la_SOURCES = \
  $(nodist_libhdr_la_SOURCES) \
  $(nodist_libsrc_la_SOURCES) \
  $(NULL)

libytstenut_@YTS_API_VERSION@_la_DEPENDENCIES = \
  ytstenut.sym \
  $(NULL)

libytstenut_@YTS_API_VERSION@_la_LDFLAGS = \
  -version-info $(VERSION_INFO) \
  -export-symbols $(srcdir)/ytstenut.sym \
  -no-undefined \
  $(NULL)

libytstenut_@YTS_API_VERSION@_la_LIBADD = \
  $(YTS_LIBS) \
  $(NULL)

libytstenut_@YTS_API_VERSION@_ladir = $(includedir)/ytstenut-$(YTS_API_VERSION)/ytstenut
libytstenut_@YTS_API_VERSION@_la_HEADERS = \
  $(libhdr_la_SOURCES) \
  $(srcdir)/ytstenut.h \
  $(NULL)

nodist_libytstenut_@YTS_API_VERSION@_la_HEADERS = \
  yts-enum-types.h \
  yts-marshal.h \
  $(NULL)

BUILT_SOURCES = \
  $(ENUMS) \
  $(MARSHALS) \
  $(NULL)

CLEANFILES = \
  $(BUILT_SOURCES) \
  $(STAMPS) \
  $(NULL)

EXTRA_DIST = \
  marshal.list \
  yts-enum-types.h.in \
  yts-enum-types.c.in \
  yts-version.h.in \
  ytstenut.sym \
  $(NULL)

DISTCLEANFILES = \
  yts-version.h \
  $(NULL)

# Glib infrastructure

ENUMS = \
  yts-enum-types.h \
  yts-enum-types.c \
  $(NULL)

MARSHALS = \
  yts-marshal.c \
  yts-marshal.h \
  $(NULL)

STAMPS = \
  stamp-marshal.h \
  stamp-yts-enum-types.h \
  $(NULL)

yts-enum-types.h: stamp-yts-enum-types.h
	@true

stamp-yts-enum-types.h: $(libhdr_la_SOURCES) Makefile
	$(AM_V_GEN)( $(GLIB_MKENUMS) \
			--template $(srcdir)/yts-enum-types.h.in \
		$(libhdr_la_SOURCES) ) >> xgen-ceth && \
	(cmp -s xgen-ceth yts-enum-types.h || cp xgen-ceth yts-enum-types.h) && \
	rm -f xgen-ceth && \
	echo timestamp > $(@F)

yts-enum-types.c: yts-enum-types.h
	$(AM_V_GEN)( $(GLIB_MKENUMS) \
			--template $(srcdir)/yts-enum-types.c.in \
	  $(libhdr_la_SOURCES) ) >> xgen-cetc && \
	cp xgen-cetc yts-enum-types.c && \
	rm -f xgen-cetc

yts-marshal.h: stamp-marshal.h
	@true

stamp-marshal.h: marshal.list
	$(AM_V_GEN)$(GLIB_GENMARSHAL) \
		--prefix=yts_marshal \
		--header \
	  $(srcdir)/marshal.list > xgen-cmh \
	&& (cmp -s xgen-cmh yts-marshal.h || cp xgen-cmh yts-marshal.h ) \
	&& rm -f xgen-cmh \
	&& echo timestamp > $(@F)

yts-marshal.c: yts-marshal.h Makefile
	$(AM_V_GEN)( echo "#include \"yts-marshal.h\"" ; \
		$(GLIB_GENMARSHAL) \
			--prefix=yts_marshal \
		--body \
		$(srcdir)/marshal.list --body ) >> xgen-cmc \
	&& cp xgen-cmc yts-marshal.c \
	&& rm -f xgen-cmc

#
# GObject Introspection
#

BUILT_GIRSOURCES =
INCLUDES_FOR_GIR = -I..

if WITH_INTROSPECTION
Ytstenut-@YTS_API_VERSION@.gir: $(G_IR_SCANNER) libytstenut-@YTS_API_VERSION@.la
	$(AM_V_GEN) $(G_IR_SCANNER) -v \
		--namespace=Ytstenut --nsversion=@YTS_API_VERSION@ \
		--strip-prefix=Yts \
		$(INCLUDES_FOR_GIR) \
		$(YTS_CFLAGS) \
		$(AM_CPPFLAGS) \
		--c-include='ytstenut/ytstenut.h' \
		--include=GObject-2.0 \
		--include=TelepathyGLib-0.12 \
		--library=libytstenut-@YTS_API_VERSION@.la \
		--libtool="$(top_builddir)/libtool" \
	        --pkg gobject-2.0 \
	        --output $@ \
		$(libhdr_la_SOURCES) \
		$(nodist_libhdr_la_SOURCES) \
		$(libsrc_la_SOURCES) \
		$(nodist_libsrc_la_SOURCES) \
		$(NULL)

BUILT_GIRSOURCES += Ytstenut-@YTS_API_VERSION@.gir

# G_IR_GIRDIR/G_IR_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/

typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(G_IR_COMPILER)
	$(AM_V_GEN) \
	LD_LIBRARY_PATH=.libs$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH} \
	$(G_IR_COMPILER) \
		--includedir=$(srcdir) \
		--includedir=. \
		$(G_IR_COMPILER_OPTS) $< -o $(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
endif # WITH_INTROSPECTION

# PONDERING should only run on platforms we know it works.
# Use "nm" to get the public symbols from libytstenut, and then
# check that every symbol from ytstenut.sym actually shows up in the library.

if OS_WINDOWS
  SOEXT=".dll"
  OUT_VERSION=@YTS_API_VERSION@-@YTS_MINOR_VERSION@
else
  SOEXT=".so"
  OUT_VERSION=@YTS_API_VERSION@
endif

check-local:
	$(AM_V_GEN) b=`tput bold` && \
	n=`tput sgr0` && \
	NM_CMD="$(NM) --defined-only --extern-only" && \
	exported_syms=`$${NM_CMD} ".libs/libytstenut-$(OUT_VERSION)$(SOEXT)" | cut -d' ' -f3` && \
	for sym in `cat $(srcdir)/ytstenut.sym`; do \
		if [ "`echo $${exported_syms} | grep $${sym}`" = "" ]; then \
			echo "$${b}Error: $${sym} exported in ytstenut.sym but not found in libytstenut.$${n}" && \
			exit 1; \
		fi \
	done

Android.mk: Makefile.am $(BUILT_SOURCES)
		androgenizer -:PROJECT ytstenut-glib \
		-:SHARED ytstenut -:TAGS eng debug \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:SOURCES $(libytstenut_@YTS_API_VERSION@_la_SOURCES) \
		-:CFLAGS $(DEFS) $(CFLAGS) $(DEFAULT_INCLUDES) $(INCLUDES)  \
		$(AM_CFLAGS) -I../../librest \
		-:CPPFLAGS $(CPPFLAGS) $(AM_CPPFLAGS) \
		-:LDFLAGS $(AM_LDFLAGS) $(YTS_LT_LDFLAGS) \
		$(libytstenut_@YTS_API_VERSION@_la_LIBADD) \
		-ltelepathy-ytstenut -lxml2 -lrest -lglib-2.0\
		> $@

-include $(top_srcdir)/git.mk
