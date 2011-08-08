
#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>

#include <ytstenut-glib/ytstenut-glib.h>

#define CAPABILITY "org.freedesktop.ytstenut.InvocationMessage"

#define CLIENT_UID "org.freedesktop.ytstenut.InvocationMessageClient"
#define SERVER_UID "org.freedesktop.ytstenut.InvocationMessageServer"

/*
 * Client
 */

static void
_client_authenticated (YtsgClient *client,
                       void       *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_client_ready (YtsgClient *client,
               void       *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_client_disconnected (YtsgClient  *client,
                      void        *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_client_message (YtsgClient   *client,
                 YtsgMessage  *msg,
                 void         *data)
{
  g_debug ("%s()", __FUNCTION__);

}

static gboolean
_client_incoming_file (YtsgClient  *client,
                       const char  *from,
                       const char  *name,
                       guint64      size,
                       guint64      offset,
                       TpChannel   *channel)
{
  g_debug ("%s()", __FUNCTION__);
  return false;
}

static void
_client_roster_service_added (YtsgRoster  *roster,
                              YtsgService *service,
                              void        *data)
{
  char const *uid;

  uid = ytsg_service_get_uid (service);

  if (0 == g_strcmp0 (uid, SERVER_UID)) {

    GVariant *args = g_variant_new_parsed (
                                "[ {\"arg1\", <1>}, {\"arg2\", <\"two\">} ]");

    YtsgMetadata *message = ytsg_invocation_message_new ("1",
                                                         CAPABILITY,
                                                         "method1",
                                                         args);
    ytsg_metadata_service_send_metadata (YTSG_METADATA_SERVICE (service),
                                         message);
  }
}

static int
run_client (void)
{
  YtsgClient  *client;
  YtsgRoster  *roster;
  GMainLoop   *mainloop;

  client = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP, CLIENT_UID);
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_client_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_client_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_client_disconnected), NULL);
  g_signal_connect (client, "message",
                    G_CALLBACK (_client_message), NULL);
  g_signal_connect (client, "incoming-file",
                    G_CALLBACK (_client_incoming_file), NULL);

  roster = ytsg_client_get_roster (client);
  g_signal_connect (roster, "service-added",
                    G_CALLBACK (_client_roster_service_added), NULL);

  ytsg_client_connect_to_mesh (client);

  mainloop = g_main_loop_new (NULL, false);
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  return EXIT_SUCCESS;
}

/*
 * Server
 */

static void
_server_authenticated (YtsgClient *client,
                       void       *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_server_ready (YtsgClient *client,
               void       *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_server_disconnected (YtsgClient  *client,
                      void        *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_server_message (YtsgClient   *client,
                 YtsgMessage  *msg,
                 void         *data)
{
  char const  *method;
  char const  *args;
  char        *unescaped_args;

  method = ytsg_metadata_get_attribute (YTSG_METADATA (msg), "aspect");
  args = ytsg_metadata_get_attribute (YTSG_METADATA (msg), "arguments");
  unescaped_args = g_uri_unescape_string (args, NULL);

  g_debug ("%s() %s %s", __FUNCTION__, method, unescaped_args);

  g_free (unescaped_args);
}

static gboolean
_server_incoming_file (YtsgClient  *client,
                       const char  *from,
                       const char  *name,
                       guint64      size,
                       guint64      offset,
                       TpChannel   *channel)
{
  g_debug ("%s()", __FUNCTION__);
  return false;
}

static int
run_server (void)
{
  YtsgClient    *client;
  GMainLoop     *mainloop;

  client = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP, SERVER_UID);
  ytsg_client_set_capabilities (client,
                                g_quark_from_static_string (CAPABILITY));
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_server_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_server_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_server_disconnected), NULL);
  g_signal_connect (client, "message",
                    G_CALLBACK (_server_message), NULL);
  g_signal_connect (client, "incoming-file",
                    G_CALLBACK (_server_incoming_file), NULL);

  ytsg_client_connect_to_mesh (client);

  mainloop = g_main_loop_new (NULL, false);
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  return EXIT_SUCCESS;
}

int
main (int     argc,
      char  **argv)
{
  bool client = false;
  bool server = true;
  GOptionEntry entries[] = {
    { "client", 'c', 0, G_OPTION_ARG_NONE, &client, "Run as client", NULL },
    { "server", 's', 0, G_OPTION_ARG_NONE, &server, "Run as server (default)", NULL },
    { NULL, }
  };

  GError          *error = NULL;
  GOptionContext  *context;
  int ret;

  context = g_option_context_new ("- Ytstenut status test");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, ytsg_get_option_group ());
  g_option_context_parse (context, &argc, &argv, &error);
  if (error) {
    g_warning ("%s : %s", G_STRLOC, error->message);
    g_clear_error (&error);
  }

  if (client) {
    g_message ("Running as client ...");
    ret = run_client ();
  } else if (server) {
    g_message ("Running as server ...");
    ret = run_server ();
  } else {
    g_warning ("%s : Not running as server or client, quitting", G_STRLOC);
    ret = -1;
  }

  return ret;
}
