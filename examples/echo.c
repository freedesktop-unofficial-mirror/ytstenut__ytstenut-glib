
#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>

#include <ytstenut-glib/ytstenut-glib.h>

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
_client_roster_service_added_cb (YtsgRoster   *roster,
                                 YtsgService  *service,
                                 gpointer      data)
{
  YtsgMetadata  *message;
  const char *payload[] = {
      "message", "hello world",
      NULL
  };

  message = (YtsgMetadata*)ytsg_message_new ((const char**)&payload);
  ytsg_metadata_service_send_metadata ((YtsgMetadataService *)service,
                                       message);  

}
  
static int
run_client (void)
{
  YtsgClient  *client;
  YtsgRoster  *roster;
  GMainLoop   *mainloop;
  
  client = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP,
                            "org.freedesktop.ytstenut.EchoClient");
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
                    G_CALLBACK (_client_roster_service_added_cb), NULL);
  
  ytsg_client_connect_to_mesh (client);
    
  mainloop = g_main_loop_new (NULL, false);
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  return EXIT_SUCCESS;
}

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
  char *payload;
  
  g_debug ("%s()", __FUNCTION__);

  g_return_if_fail (YTSG_IS_MESSAGE (msg));

  payload = ytsg_metadata_to_string (YTSG_METADATA (msg));
  g_debug (payload);
  g_free (payload);
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
  YtsgClient  *client;
  GMainLoop   *mainloop;
  
  client = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP,
                            "com.meego.ytstenut.EchoServer");
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

//  ytsg_client_set_capabilities (client, YTSG_CAPS_CONTROL);
//  ytsg_client_set_status_by_capability (client, YTSG_CAPS_S_CONTROL, "idle");

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
  
  context = g_option_context_new ("- Ytstenut echo test");
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
