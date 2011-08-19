/*
 * Copyright (c) 2011 Intel Corp.
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

#include <ytstenut-glib/ytstenut-glib.h>

#include "mock-player.h"

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
                 YtsgMessage  *message,
                 void         *data)
{
  char *message_xml;

  message_xml = ytsg_metadata_to_string (YTSG_METADATA (message));
  g_debug ("%s() %s", __FUNCTION__, message_xml);
  g_free (message_xml);
}

int
main (int     argc,
      char  **argv)
{
  GOptionContext  *context;
  YtsgClient      *client;
  MockPlayer      *player;
  GMainLoop       *mainloop;
  GError          *error = NULL;
  GOptionEntry     entries[] = {
    { NULL, }
  };

  context = g_option_context_new ("- mock player");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, ytsg_get_option_group ());
  g_option_context_parse (context, &argc, &argv, &error);
  if (error) {
    g_warning ("%s : %s", G_STRLOC, error->message);
    g_clear_error (&error);
    return EXIT_FAILURE;
  }

  client = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP,
                            "org.freedesktop.ytstenut.MockPlayer");
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_client_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_client_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_client_disconnected), NULL);
  g_signal_connect (client, "message",
                    G_CALLBACK (_client_message), NULL);

  player = mock_player_new ();
  ytsg_client_register_service (client, G_OBJECT (player));

  ytsg_client_connect_to_mesh (client);

  mainloop = g_main_loop_new (NULL, false);
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  return EXIT_SUCCESS;
}

