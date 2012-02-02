/*
 * Copyright © 2012 Intel Corp.
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
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include <ytstenut/ytstenut.h>

#define CLIENT_UID "org.freedesktop.ytstenut.FileTransferClient"
#define CLIENT_JID "ytstenut2@test.collabora.co.uk0"

#define SERVER_UID "org.freedesktop.ytstenut.FileTransferServer"
#define SERVER_JID "ytstenut1@test.collabora.co.uk0"

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
_client_text_message (YtsClient   *client,
                      char const  *text,
                      void         *data)
{
  g_debug ("%s()", __FUNCTION__);
  g_debug ("Message is \"%s\"", text);
}

static void
_outgoing_error (YtsOutgoingFile  *outgoing,
                 GError           *error,
                 void             *data)
{
  g_debug ("%s()", __FUNCTION__);

  g_critical ("%s", error->message);
  g_object_unref (outgoing);
}

static void
_outgoing_notify_progress (YtsOutgoingFile  *outgoing,
                           GParamSpec       *pspec,
                           void             *data)
{
  float progress;

  progress = yts_file_transfer_get_progress (YTS_FILE_TRANSFER (outgoing));

  if (progress < 0) {

    /* Error, handled above. */

  } else if (progress <= 1.0) {

    printf ("%.2f .. ", progress);
    fflush (stdout);

  } else {

    printf ("done\n");
    g_object_unref (outgoing);
  }
}

static void
_client_roster_service_added (YtsRoster   *roster,
                              YtsService  *service,
                              char const  *path)
{
  static bool _is_sent = false;
  char const *service_id;

  service_id = yts_service_get_id (service);

  if (!_is_sent &&
      0 == g_strcmp0 (service_id, SERVER_UID)) {

    GError *error = NULL;
    GFile *file = g_file_new_for_path (path);
    YtsOutgoingFile *outgoing = yts_service_send_file (service,
                                                       file,
                                                       "Hello, like file?",
                                                       &error);
    if (error) {
      g_critical ("%s", error->message);
      g_clear_error (&error);
      return;
    }

    g_signal_connect (outgoing, "error",
                      G_CALLBACK (_outgoing_error), NULL);
    g_signal_connect (outgoing, "notify::progress",
                      G_CALLBACK (_outgoing_notify_progress), NULL);

    g_object_unref (file);
    _is_sent = true;
  }
}

static int
run_client (bool         p2p,
            char const  *path)
{
  YtsClient  *client;
  YtsRoster  *roster;
  GMainLoop   *mainloop;

  if (p2p)
    client = yts_client_new_p2p (CLIENT_UID);
  else
    client = yts_client_new_c2s (CLIENT_JID, CLIENT_UID);

  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_client_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_client_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_client_disconnected), NULL);
  g_signal_connect (client, "text-message",
                    G_CALLBACK (_client_text_message), NULL);

  roster = yts_client_get_roster (client);
  g_signal_connect (roster, "service-added",
                    G_CALLBACK (_client_roster_service_added), (void *) path);

  yts_client_connect (client);

  mainloop = g_main_loop_new (NULL, false);
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  return EXIT_SUCCESS;
}

/*
 * Server
 */

typedef struct {
  YtsService *service;
} ServerData;

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
                      ServerData   *self)
{
  g_debug ("%s() know client: %s", __FUNCTION__,
                                  self->service ? "yes" : "no");

  if (self->service) {
    g_debug ("%s() echoing \"%s\"", __FUNCTION__, text);
    yts_service_send_text (YTS_SERVICE (self->service), text);
  }
}

static void
_server_roster_service_added (YtsRoster  *roster,
                              YtsService *service,
                              ServerData  *self)
{
  char const *uid;

  uid = yts_service_get_id (service);

  g_debug ("%s() %s", __FUNCTION__, uid);

  /* FIXME, possible race condition when client sends message before
   * it shows up in our roster? */
  if (0 == g_strcmp0 (uid, CLIENT_UID)) {
    /* Should probably take a weak ref here. */
    self->service = service;
  }
}

static int
run_server (bool p2p)
{
  YtsClient  *client;
  YtsRoster  *roster;
  GMainLoop   *mainloop;
  ServerData   self = { NULL, };

  if (p2p)
    client = yts_client_new_p2p (SERVER_UID);
  else
    client = yts_client_new_c2s (SERVER_JID, SERVER_UID);

  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_server_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_server_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_server_disconnected), NULL);
  g_signal_connect (client, "text-message",
                    G_CALLBACK (_server_text_message), &self);

  roster = yts_client_get_roster (client);
  g_signal_connect (roster, "service-added",
                    G_CALLBACK (_server_roster_service_added), &self);

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
  bool p2p = false;
  char const *path = NULL;
  GOptionEntry entries[] = {
    { "client", 'c', 0, G_OPTION_ARG_NONE, &client, "Run as client", NULL },
    { "server", 's', 0, G_OPTION_ARG_NONE, &server, "Run as server (default)", NULL },
    { "p2p", 'p', 0, G_OPTION_ARG_NONE, &p2p, "Run in p2p mode", NULL },
    { "file", 'f', 0, G_OPTION_ARG_STRING, &path, "Path to read or save file", NULL },
    { NULL, }
  };

  GError          *error = NULL;
  GOptionContext  *context;
  int ret;

  g_type_init ();

  context = g_option_context_new ("- Ytstenut file-transfer example");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, &argc, &argv, &error);
  if (error) {
    g_warning ("%s : %s", G_STRLOC, error->message);
    g_clear_error (&error);
  }

  if (client) {
    g_message ("Running as client ...");
    ret = run_client (p2p, path);
  } else if (server) {
    g_message ("Running as server ... not implemented yet");
    /* ret = run_server (p2p); */
    ret = -1;
  } else {
    g_warning ("%s : Not running as server or client, quitting", G_STRLOC);
    ret = -1;
  }

  return ret;
}
