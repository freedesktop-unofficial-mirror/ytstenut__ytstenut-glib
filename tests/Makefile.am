TESTS = message status metadata-service local-connection message-send status-set

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

## File transfer can't be tested this way, because it is not possible to do
## FT to self (i.e., there would need to be two separate contacts, but as we
## only have one contact per device, that would mean two machines ...
#file_transfer_SOURCES    = file-transfer.c
#file_transfer_LDADD      = $(YTSG_LIBS)

message_send_SOURCES     = message-send.c
message_send_LDADD       = $(YTSG_LIBS)

status_set_SOURCES     = status-set.c
status_set_LDADD       = $(YTSG_LIBS)
