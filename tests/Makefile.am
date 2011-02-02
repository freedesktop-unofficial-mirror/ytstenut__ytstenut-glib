TESTS = message status

AM_CPPFLAGS = 					\
	-DG_DISABLE_DEPRECATED 			\
	-DG_DISABLE_SINGLE_INCLUDES		\
	-DG_LOG_DOMAIN=\"YtstenutGlib\" 	\
	$(YTSG_DEBUG_CFLAGS)		\
	$(NULL)

AM_CFLAGS = $(YTSG_CFLAGS)

AM_LDFLAGS = ../ytstenut-glib/libytstenut-glib-@YTSG_API_VERSION@.la

check_PROGRAMS = $(TESTS)

message_SOURCES = message.c
status_SOURCES  = status.c
