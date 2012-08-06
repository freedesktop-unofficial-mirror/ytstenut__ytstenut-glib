
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
  yts-capability.h \
  yts-client.h \
  yts-contact.h \
  yts-file-transfer.h \
  yts-incoming-file.h \
  yts-outgoing-file.h \
  yts-roster.h \
  yts-service.h \
  \
  yts-proxy.h \
  yts-proxy-service.h \
  \
  video-profile/yts-vp-content.h \
  video-profile/yts-vp-playable.h \
  video-profile/yts-vp-player.h \
  video-profile/yts-vp-query.h \
  video-profile/yts-vp-transcript.h \
  video-profile/yts-vp-transfer.h \
  video-profile/yts-vp-transmission.h \
  $(NULL)

nodist_libhdr_la_SOURCES = \
  yts-version.h \
  $(NULL)

# Sources
libsrc_la_SOURCES = \
  ytstenut.c \
  \
  yts-capability.c \
  yts-client.c \
  yts-client-status.c \
  yts-contact.c \
  yts-contact-impl.c \
  yts-error.c \
  yts-message.c \
  yts-metadata.c \
  yts-roster.c \
  yts-roster-impl.c \
  yts-service.c \
  yts-service-impl.c \
  \
  yts-adapter-factory.c \
  yts-error-message.c \
  yts-event-message.c \
  yts-factory.c \
  yts-incoming-file.c \
  yts-invocation-message.c \
  yts-service-emitter.c \
  yts-file-transfer.c \
  yts-outgoing-file.c \
  yts-proxy.c \
  yts-proxy-factory.c \
  yts-proxy-service.c \
  yts-proxy-service-impl.c \
  yts-response-message.c \
  yts-service-adapter.c \
  yts-service-factory.c \
  \
  profile/yts-profile.c \
  profile/yts-profile-impl.c \
  profile/yts-profile-proxy.c \
  profile/yts-profile-adapter.c \
  \
  video-profile/yts-vp-content.c \
  video-profile/yts-vp-playable.c \
  video-profile/yts-vp-playable-proxy.c \
  video-profile/yts-vp-player.c \
  video-profile/yts-vp-player-adapter.c \
  video-profile/yts-vp-player-proxy.c \
  video-profile/yts-vp-query.c \
  video-profile/yts-vp-transcript.c \
  video-profile/yts-vp-transcript-adapter.c \
  video-profile/yts-vp-transcript-proxy.c \
  video-profile/yts-vp-transfer.c \
  video-profile/yts-vp-transmission.c \
  $(NULL)

nodist_libsrc_la_SOURCES = \
  yts-enum-types.c \
  yts-marshal.c \
  $(NULL)

# Private headers
libprv_la_SOURCES = \
  ytstenut-internal.h \
  \
  yts-adapter-factory.h \
  yts-client-internal.h \
  yts-client-status.h \
  yts-contact-impl.h \
  yts-contact-internal.h \
  yts-error.h \
  yts-factory.h \
  yts-incoming-file-internal.h \
  yts-metadata-internal.h \
  yts-outgoing-file-internal.h \
  yts-proxy-factory.h \
  yts-proxy-internal.h \
  yts-proxy-service-impl.h \
  yts-proxy-service-internal.h \
  yts-roster-impl.h \
  yts-roster-internal.h \
  yts-service-adapter.h \
  yts-service-emitter.h \
  yts-service-factory.h \
  yts-service-impl.h \
  yts-service-internal.h \
  yts-xml.h \
  \
  yts-message.h \
  yts-metadata.h \
  yts-error-message.h \
  yts-event-message.h \
  yts-invocation-message.h \
  yts-response-message.h \
  \
  profile/yts-profile.h \
  profile/yts-profile-impl.h \
  profile/yts-profile-proxy.h \
  profile/yts-profile-adapter.h \
  \
  video-profile/yts-vp-playable-proxy.h \
  video-profile/yts-vp-player-adapter.h \
  video-profile/yts-vp-player-proxy.h \
  video-profile/yts-vp-transcript-adapter.h \
  video-profile/yts-vp-transcript-proxy.h \
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
  ytstenut.h \
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
	$(AM_V_GEN)( cd $(srcdir) && $(GLIB_MKENUMS) \
			--template $(srcdir)/yts-enum-types.h.in \
		$(libhdr_la_SOURCES) ) >> xgen-ceth && \
	(cmp -s xgen-ceth yts-enum-types.h || cp xgen-ceth yts-enum-types.h) && \
	rm -f xgen-ceth && \
	echo timestamp > $(@F)

yts-enum-types.c: yts-enum-types.h
	$(AM_V_GEN)( cd $(srcdir) && $(GLIB_MKENUMS) \
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
		$(patsubst %,$(srcdir)/%,$(libhdr_la_SOURCES)) \
		$(nodist_libhdr_la_SOURCES) \
		$(patsubst %,$(srcdir)/%,$(libsrc_la_SOURCES)) \
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
