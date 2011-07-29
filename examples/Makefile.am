
AM_CPPFLAGS = 					\
	-DG_DISABLE_DEPRECATED 			\
	-DG_DISABLE_SINGLE_INCLUDES		\
	-DG_LOG_DOMAIN=\"YtstenutGlib\" 	\
	$(YTSG_DEBUG_CFLAGS)		\
	$(NULL)

AM_CFLAGS = $(YTSG_CFLAGS)

AM_LDFLAGS = ../ytstenut-glib/libytstenut-glib-@YTSG_API_VERSION@.la

echo_LDADD = $(YTSG_LIBS)

bin_PROGRAMS = \
  echo \
  status \
  $(NULL)

echo_SOURCES = \
  echo.c \
  $(NULL)

status_SOURCES = \
  status.c \
  $(NULL)

