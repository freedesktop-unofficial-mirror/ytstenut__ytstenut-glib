
SUBDIRS = video-service

AM_CPPFLAGS = 					\
	-DG_DISABLE_DEPRECATED 			\
	-DG_DISABLE_SINGLE_INCLUDES		\
	-DG_LOG_DOMAIN=\"YtstenutGlib\" 	\
	$(YTSG_DEBUG_CFLAGS)		\
	$(NULL)

AM_CFLAGS = $(YTSG_CFLAGS)

AM_LDFLAGS = ../ytstenut-glib/libytstenut-glib-@YTSG_API_VERSION@.la

LDADD = $(YTSG_LIBS)

noinst_PROGRAMS = \
  invocation-message \
  echo \
  status \
  $(NULL)

invocation_message_SOURCES = \
  invocation-message.c \
  $(NULL)

echo_SOURCES = \
  echo.c \
  $(NULL)

status_SOURCES = \
  status.c \
  $(NULL)

