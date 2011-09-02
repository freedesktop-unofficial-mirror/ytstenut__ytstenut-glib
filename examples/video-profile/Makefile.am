
AM_CPPFLAGS = \
  -I$(top_srcdir) \
	-DG_DISABLE_DEPRECATED \
	$(YTS_DEBUG_CFLAGS) \
	$(NULL)

AM_CFLAGS = $(YTS_CFLAGS)

AM_LDFLAGS = ../../ytstenut/libytstenut-@YTS_API_VERSION@.la

LDADD = $(YTS_LIBS)

noinst_PROGRAMS = \
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

