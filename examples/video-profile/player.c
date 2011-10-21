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

#include "mock-player.h"

/*
 * Client object signal handlers for illustration and debugging purpose.
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

/* Messages that are not handled by any service are emitted by the client. */
static void
_client_raw_message (YtsClient  *client,
                     char const *xml_payload,
                     void       *data)
{
  char *message_xml;

  g_debug ("%s() %s", __FUNCTION__, xml_payload);
}

/*
 * Roster object signal handlers for illustration and debugging purpose.
 */

static void
_roster_contact_added (YtsRoster   *roster,
                       YtsContact  *contact,
                       void         *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_roster_contact_removed (YtsRoster   *roster,
                         YtsContact  *contact,
                         void         *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_roster_service_added (YtsRoster   *roster,
                       YtsService  *service,
                       void         *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_roster_service_removed (YtsRoster   *roster,
                         YtsService  *contact,
                         void         *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_player_notify_current_text (MockPlayer *player,
                             GParamSpec *pspec,
                             void       *data)
{
  char *current_text;

  current_text = yts_vp_transcript_get_current_text (
                    YTS_VP_TRANSCRIPT (player));

  g_debug ("%s() '%s'", __FUNCTION__, current_text);

  g_free (current_text);
}

/*
 * Main. What else.
 */

int
main (int     argc,
      char  **argv)
{
  GOptionContext  *context;
  YtsClient      *client;
  YtsRoster      *roster;
  MockPlayer      *player;
  GMainLoop       *mainloop;
  GError          *error = NULL;
  GOptionEntry     entries[] = {
    { NULL, }
  };

  /* Initialisation and command-line argument handling. */
  context = g_option_context_new ("- mock player");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, &argc, &argv, &error);
  if (error) {
    g_warning ("%s : %s", G_STRLOC, error->message);
    g_clear_error (&error);
    return EXIT_FAILURE;
  }

  /* The client object represents an ytstenut application. */
  client = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP,
                            "org.freedesktop.ytstenut.MockPlayer");
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_client_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_client_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_client_disconnected), NULL);
  g_signal_connect (client, "raw-message",
                    G_CALLBACK (_client_raw_message), NULL);

  /* The roster object tracks other devices and services as they come and go. */
  roster = yts_client_get_roster (client);
  g_signal_connect (roster, "contact-added",
                    G_CALLBACK (_roster_contact_added), NULL);
  g_signal_connect (roster, "contact-removed",
                    G_CALLBACK (_roster_contact_removed), NULL);
  g_signal_connect (roster, "service-added",
                    G_CALLBACK (_roster_service_added), NULL);
  g_signal_connect (roster, "service-removed",
                    G_CALLBACK (_roster_service_removed), NULL);

  /* Instantiate and publish example player object so others can access it. */
  player = mock_player_new ();
  yts_client_publish_service (client, YTS_CAPABILITY (player));

  g_signal_connect (player, "notify::current-text",
                    G_CALLBACK (_player_notify_current_text), NULL);

  /* Activate the client. */
  yts_client_connect (client);

  /* Run application. */
  mainloop = g_main_loop_new (NULL, false);
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  return EXIT_SUCCESS;
}

