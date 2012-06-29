exampleexecdir = $(libdir)/ytstenut/examples

SUBDIRS = video-profile

AM_CPPFLAGS = \
  -I$(top_srcdir) \
	-DG_DISABLE_DEPRECATED \
	$(NULL)

AM_CFLAGS = $(YTS_CFLAGS)

AM_LDFLAGS = ../ytstenut/libytstenut-@YTS_API_VERSION@.la

LDADD = $(YTS_LIBS)

exampleexec_PROGRAMS = \
  dictionary-message \
  echo \
  file-transfer \
  status \
  $(NULL)

dictionary_message_SOURCES = \
  dictionary-message.c \
  $(NULL)

echo_SOURCES = \
  echo.c \
  $(NULL)

file_transfer_SOURCES = \
  file-transfer.c \
  $(NULL)

status_SOURCES = \
  status.c \
  $(NULL)

-include $(top_srcdir)/git.mk
