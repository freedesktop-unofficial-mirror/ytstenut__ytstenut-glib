exampleexecdir = $(libdir)/ytstenut/examples

AM_CPPFLAGS = \
  -I$(top_srcdir) \
	-DG_DISABLE_DEPRECATED \
	$(NULL)

AM_CFLAGS = $(YTS_CFLAGS)

AM_LDFLAGS = ../../ytstenut/libytstenut-@YTS_API_VERSION@.la

LDADD = $(YTS_LIBS)

exampleexec_PROGRAMS = \
  player \
  remote \
  $(NULL)

player_SOURCES = \
  player.c \
  mock-player.c \
  mock-player.h \
  $(NULL)

remote_SOURCES = \
  remote.c \
  $(NULL)

-include $(top_srcdir)/git.mk
