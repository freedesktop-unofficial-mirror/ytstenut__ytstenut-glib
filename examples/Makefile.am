
SUBDIRS = video-profile

AM_CPPFLAGS = \
  -I$(top_srcdir) \
	-DG_DISABLE_DEPRECATED \
	$(YTSG_DEBUG_CFLAGS) \
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

