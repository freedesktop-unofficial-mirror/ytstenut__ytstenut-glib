/*
 * Copyright © 2011 Intel Corp.
 *
 * This  library is free  software; you can  redistribute it and/or
 * modify it  under  the terms  of the  GNU Lesser  General  Public
 * License  as published  by the Free  Software  Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed  in the hope that it will be useful,
 * but  WITHOUT ANY WARRANTY; without even  the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Authored by: Rob Staudinger <robsta@linux.intel.com>
 */

#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>

#include <ytstenut/ytstenut.h>

#define CAPABILITY "org.freedesktop.ytstenut.StatusExample"

#define CLIENT_UID "org.freedesktop.ytstenut.StatusExampleClient"
#define SERVER_UID "org.freedesktop.ytstenut.StatusExampleServer"

/*
 * Client
 */

static void
_client_authenticated (YtsClient *client,
                       void       *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_client_ready (YtsClient *client,
               void       *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_client_disconnected (YtsClient  *client,
                      void        *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_client_raw_message (YtsClient  *client,
                     char const *xml_payload,
                     void       *data)
{
  g_debug ("%s() %s", __FUNCTION__, xml_payload);

}

static gboolean
_client_incoming_file (YtsClient  *client,
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
_server_status (YtsClient  *client,
                YtsStatus  *status,
                void        *data)
{
  char *dump;

  g_debug ("%s()", __FUNCTION__);

  dump = yts_metadata_to_string (YTS_METADATA (status));
  g_debug ("%s", dump);
  g_free (dump);
}

static void
_client_roster_service_added (YtsRoster  *roster,
                              YtsService *service,
                              void        *data)
{
  char const *uid;

  uid = yts_service_get_service_id (service);

  if (0 == g_strcmp0 (uid, SERVER_UID)) {

    char const text[] = "ping pong";

    /* Hook up to server status changes. */
    g_signal_connect (service, "status",
                      G_CALLBACK (_server_status), NULL);


    g_debug ("%s() %s", __FUNCTION__, uid);
    g_debug ("Sending message \"%s\"", text);

    yts_service_send_text (service, text);
  }
}

static int
run_client (void)
{
  YtsClient  *client;
  YtsRoster  *roster;
  GMainLoop   *mainloop;

  client = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP, CLIENT_UID);
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_client_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_client_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_client_disconnected), NULL);
  g_signal_connect (client, "raw-message",
                    G_CALLBACK (_client_raw_message), NULL);
  g_signal_connect (client, "incoming-file",
                    G_CALLBACK (_client_incoming_file), NULL);

  roster = yts_client_get_roster (client);
  g_signal_connect (roster, "service-added",
                    G_CALLBACK (_client_roster_service_added), NULL);

  yts_client_connect (client);

  mainloop = g_main_loop_new (NULL, false);
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  return EXIT_SUCCESS;
}

/*
 * Server
 */

static void
_server_authenticated (YtsClient *client,
                       void       *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_server_ready (YtsClient *client,
               void       *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_server_disconnected (YtsClient  *client,
                      void        *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_server_text_message (YtsClient   *client,
                      char const  *text,
                      void        *data)
{
  char const *property_name;

  g_debug ("%s()", __FUNCTION__);

  /* Got pinged, set some status */

  property_name = "urn:ytstenut:capabilities:" CAPABILITY;

  yts_client_set_status_by_capability (client,
                                        property_name, "Foo");
}

static gboolean
_server_incoming_file (YtsClient  *client,
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
  YtsClient    *client;
  GMainLoop     *mainloop;

  client = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP, SERVER_UID);
  yts_client_set_capabilities (client,
                                g_quark_from_static_string (CAPABILITY));
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_server_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_server_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_server_disconnected), NULL);
  g_signal_connect (client, "text-message",
                    G_CALLBACK (_server_text_message), NULL);
  g_signal_connect (client, "incoming-file",
                    G_CALLBACK (_server_incoming_file), NULL);

  yts_client_connect (client);

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
  g_option_context_add_group (context, yts_get_option_group ());
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
