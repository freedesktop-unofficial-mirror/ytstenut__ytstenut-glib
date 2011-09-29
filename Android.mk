LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

YTSTENUT_GLIB_BUILT_SOURCES := \
	ytstenut/Android.mk

ytstenut-glib-configure-real:
	cd $(YTSTENUT_GLIB_TOP) ; \
	CXX="$(CONFIGURE_CXX)" \
	CC="$(CONFIGURE_CC)" \
	CFLAGS="$(CONFIGURE_CFLAGS)" \
	LD=$(TARGET_LD) \
	LDFLAGS="$(CONFIGURE_LDFLAGS)" \
	CPP=$(CONFIGURE_CPP) \
	CPPFLAGS="$(CONFIGURE_CPPFLAGS)" \
	PKG_CONFIG_LIBDIR=$(CONFIGURE_PKG_CONFIG_LIBDIR) \
	PKG_CONFIG_TOP_BUILD_DIR=$(PKG_CONFIG_TOP_BUILD_DIR) \
	$(YTSTENUT_GLIB_TOP)/$(CONFIGURE) --host=arm-linux-androideabi \
		--disable-spec-documentation --disable-qt4 \
		--disable-Werror && \
	for file in $(YTSTENUT_GLIB_BUILT_SOURCES); do \
		rm -f $$file && \
		make -C $$(dirname $$file) $$(basename $$file) ; \
	done

ytstenut-glib-configure: ytstenut-glib-configure-real

.PHONY: ytstenut-glib-configure

CONFIGURE_TARGETS += ytstenut-glib-configure

#include all the subdirs...
-include $(YTSTENUT_GLIB_TOP)/ytstenut/Android.mk
