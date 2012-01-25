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

#define CAPABILITY "org.freedesktop.ytstenut.DictionaryMessage"

#define CLIENT_UID "org.freedesktop.ytstenut.DictionaryMessageClient"
#define SERVER_UID "org.freedesktop.ytstenut.DictionaryMessageServer"

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
  g_debug ("%s() '%s'", __FUNCTION__, xml_payload);
}

static void
_client_roster_service_added (YtsRoster  *roster,
                              YtsService *service,
                              void        *data)
{
  char const *uid;
  char const *const dict[] = {
    "arg1", "1",
    "arg2", "two",
    NULL
  };

  uid = yts_service_get_id (service);

  if (0 == g_strcmp0 (uid, SERVER_UID)) {

    yts_service_send_dictionary (service, dict, -1);
  }
}

static int
run_client (void)
{
  YtsClient  *client;
  YtsRoster  *roster;
  GMainLoop   *mainloop;

  client = yts_client_new_p2p (CLIENT_UID);
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_client_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_client_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_client_disconnected), NULL);
  g_signal_connect (client, "raw-message",
                    G_CALLBACK (_client_raw_message), NULL);

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
_server_dictionary_message (YtsClient         *client,
                            char const *const *dictionary,
                            void              *data)
{
  char const *const *iter;

  g_debug ("%s()", __FUNCTION__);

  if (NULL != (iter = dictionary)) {
    do {
      char const *name = *iter++;
      char const *value = *iter++;
      g_debug ("  %s = %s", name, value);
    } while (*iter);
  }
}

static int
run_server (void)
{
  YtsClient    *client;
  GMainLoop     *mainloop;

  client = yts_client_new_p2p (SERVER_UID);
  yts_client_add_capability (client, CAPABILITY, YTS_CAPABILITY_MODE_PROVIDED);
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_server_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_server_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_server_disconnected), NULL);
  g_signal_connect (client, "dictionary-message",
                    G_CALLBACK (_server_dictionary_message), NULL);

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

  g_type_init ();

  context = g_option_context_new ("- Ytstenut status test");
  g_option_context_add_main_entries (context, entries, NULL);
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
