
SUBDIRS = video-profile

AM_CPPFLAGS = \
  -I$(top_srcdir) \
	-DG_DISABLE_DEPRECATED \
	$(NULL)

AM_CFLAGS = $(YTS_CFLAGS)

AM_LDFLAGS = ../ytstenut/libytstenut-@YTS_API_VERSION@.la

LDADD = $(YTS_LIBS)

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

-include $(top_srcdir)/git.mk
