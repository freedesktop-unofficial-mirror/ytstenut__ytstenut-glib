
AM_CPPFLAGS = 					\
	-DG_DISABLE_DEPRECATED 			\
	-DG_DISABLE_SINGLE_INCLUDES		\
	-DG_LOG_DOMAIN=\"YtstenutGlib\" 	\
	$(YTSG_DEBUG_CFLAGS)		\
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
  $(NULL)

remote_SOURCES = \
  remote.c \
  $(NULL)

