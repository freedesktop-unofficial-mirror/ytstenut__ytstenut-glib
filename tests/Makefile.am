TESTS = message status metadata-service local-connection

AM_CPPFLAGS = 					\
	-DG_DISABLE_DEPRECATED 			\
	-DG_DISABLE_SINGLE_INCLUDES		\
	-DG_LOG_DOMAIN=\"YtstenutGlib\" 	\
	$(YTSG_DEBUG_CFLAGS)		\
	$(NULL)

AM_CFLAGS = $(YTSG_CFLAGS)

AM_LDFLAGS = ../ytstenut-glib/libytstenut-glib-@YTSG_API_VERSION@.la

check_PROGRAMS = $(TESTS)

message_SOURCES          = message.c
message_LDADD            = $(YTSG_LIBS)

status_SOURCES           = status.c
status_LDADD             = $(YTSG_LIBS)

metadata_service_SOURCES = metadata-service.c
metadata_service_LDADD   = $(YTSG_LIBS)

local_connection_SOURCES = local-connection.c
local_connection_LDADD   = $(YTSG_LIBS)
