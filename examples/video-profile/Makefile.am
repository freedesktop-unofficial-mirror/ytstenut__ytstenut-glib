
AM_CPPFLAGS = \
  -I$(top_srcdir) \
	-DG_DISABLE_DEPRECATED \
	$(YTSG_DEBUG_CFLAGS) \
	$(NULL)

AM_CFLAGS = $(YTSG_CFLAGS)

AM_LDFLAGS = ../../ytstenut-glib/libytstenut-glib-@YTSG_API_VERSION@.la

LDADD = $(YTSG_LIBS)

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

