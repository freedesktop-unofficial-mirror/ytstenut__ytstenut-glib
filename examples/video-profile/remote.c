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

typedef enum {
  COMMAND_NONE,
  COMMAND_PLAY,
  COMMAND_PAUSE,
  COMMAND_PREV,
  COMMAND_NEXT
} PlayerCommand;

static PlayerCommand  _command = COMMAND_NONE;
static GMainLoop     *_mainloop = NULL;

static void
_client_authenticated (YtsgClient *client,
                       void       *data)
{
  g_debug ("%s()", __FUNCTION__);
}

static void
_client_ready (YtsgClient     *client,
               PlayerCommand   command)
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

static void
_roster_service_added (YtsgRoster   *roster,
                       YtsgService  *service,
                       void         *data)
{
  YtsgProxy   *proxy;
  char const  *uid;
  char const  *jid;

  uid = ytsg_service_get_uid (service);
  jid = ytsg_service_get_jid (service);

  g_debug ("%s() %s %s", __FUNCTION__, uid, jid);

  if (0 == g_strcmp0 (uid, "org.freedesktop.ytstenut.MockPlayer")) {

    proxy = ytsg_proxy_service_create_proxy (YTSG_PROXY_SERVICE (service),
                                             YTSG_VP_PLAYER_CAPABILITY);
    g_return_if_fail (proxy);

    switch (_command) {
      case COMMAND_PLAY:
        ytsg_vp_player_play (YTSG_VP_PLAYER (proxy));
        break;
      case COMMAND_PAUSE:
        ytsg_vp_player_pause (YTSG_VP_PLAYER (proxy));
        break;
      case COMMAND_NEXT:
        ytsg_vp_player_next (YTSG_VP_PLAYER (proxy));
        break;
      case COMMAND_PREV:
        ytsg_vp_player_prev (YTSG_VP_PLAYER (proxy));
        break;
      default:
        g_debug ("%s : command %i not handled", G_STRLOC, _command);
    }

    g_object_unref (proxy);
  }
}

int
main (int     argc,
      char  **argv)
{
  GOptionContext  *context;
  YtsgClient      *client;
  YtsgRoster      *roster;
  GError          *error = NULL;

  bool         play = false;
  bool         pause = false;
  bool         next = false;
  bool         prev = false;
  GOptionEntry entries[] = {
    { "play", 'p', 0, G_OPTION_ARG_NONE, &play, "Invoke 'play'", NULL },
    { "pause", 'a', 0, G_OPTION_ARG_NONE, &pause, "Invoke 'pause'", NULL },
    { "next", 'n', 0, G_OPTION_ARG_NONE, &next, "Invoke 'next'", NULL },
    { "prev", 'r', 0, G_OPTION_ARG_NONE, &prev, "Invoke 'prev'", NULL },
    { NULL }
  };

  context = g_option_context_new ("- mock player remote");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, ytsg_get_option_group ());
  g_option_context_parse (context, &argc, &argv, &error);
  if (error) {
    g_warning ("%s : %s", G_STRLOC, error->message);
    g_clear_error (&error);
    return EXIT_FAILURE;
  }

  if (play) {
    _command = COMMAND_PLAY;
  } else if (pause) {
    _command = COMMAND_PAUSE;
  } else if (next) {
    _command = COMMAND_NEXT;
  } else if (prev) {
    _command = COMMAND_PREV;
  } else {
    g_debug ("No command given, use --help to display commands.");
  }

  client = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP,
                            "org.freedesktop.ytstenut.MockPlayerRemote");
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_client_authenticated), NULL);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_client_ready), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_client_disconnected), NULL);
  g_signal_connect (client, "message",
                    G_CALLBACK (_client_message), NULL);

  roster = ytsg_client_get_roster (client);
  g_signal_connect (roster, "service-added",
                    G_CALLBACK (_roster_service_added), NULL);

  ytsg_client_connect (client);

  _mainloop = g_main_loop_new (NULL, false);
  g_main_loop_run (_mainloop);
  g_main_loop_unref (_mainloop);

  return EXIT_SUCCESS;
}

