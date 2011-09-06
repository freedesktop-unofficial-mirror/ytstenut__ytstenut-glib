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
#include <string.h>
#include <glib.h>

#include <ytstenut/ytstenut.h>

typedef enum {
  COMMAND_NONE = 0,
  COMMAND_PLAYER_PLAYING,
  COMMAND_PLAYER_VOLUME,
  COMMAND_PLAYER_PLAYABLE_URI,
  COMMAND_PLAYER_PLAY,
  COMMAND_PLAYER_PAUSE,
  COMMAND_PLAYER_PREV,
  COMMAND_PLAYER_NEXT
} Command;

typedef struct {

  char const  *remote_id;

  Command command;

  /* Player */
  char const  *player_playing;
  double       player_volume;
  char const  *player_playable_uri;
  bool         player_play;
  bool         player_pause;
  bool         player_next;
  bool         player_prev;

  YtsVPPlayer     *player;
  YtsVPTranscript *transcript;

} Remote;

static GMainLoop     *_mainloop = NULL;

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
_client_message (YtsClient   *client,
                 YtsMessage  *message,
                 void         *data)
{
  char *message_xml;

  message_xml = yts_metadata_to_string (YTS_METADATA (message));
  g_debug ("%s() %s", __FUNCTION__, message_xml);
  g_free (message_xml);
}

static void
_player_notify_playing (YtsVPPlayer  *player,
                        GParamSpec    *pspec,
                        void          *data)
{
  g_debug ("YtsVPPlayer.playing = %s",
           yts_vp_player_get_playing (player) ? "true" : "false");
}

static void
_player_notify_volume (YtsVPPlayer *player,
                       GParamSpec   *pspec,
                       void         *data)
{
  g_debug ("YtsVPPlayer.volume = %.2f", yts_vp_player_get_volume (player));
}

static void
_player_notify_playable_uri (YtsVPPlayer *player,
                             GParamSpec   *pspec,
                             void         *data)
{
  char *playable_uri;

  playable_uri = yts_vp_player_get_playable_uri (player);
  g_debug ("YtsVPPlayer.playable-uri = %s", playable_uri);
  g_free (playable_uri);
}

static void
_player_next_response (YtsVPPlayer *player,
                       char const   *invocation_id,
                       bool          return_value,
                       void         *data)
{
  g_debug ("YtsVPPlayer.next() returned %s", return_value ? "true" : "false");
}

static void
_player_prev_response (YtsVPPlayer *player,
                       char const   *invocation_id,
                       bool          return_value,
                       void         *data)
{
  g_debug ("YtsVPPlayer.prev() returned %s", return_value ? "true" : "false");
}

static void
use_player (Remote *remote)
{
  g_signal_connect (remote->player, "notify::playing",
                    G_CALLBACK (_player_notify_playing), NULL);
  g_signal_connect (remote->player, "notify::volume",
                    G_CALLBACK (_player_notify_volume), NULL);
  g_signal_connect (remote->player, "notify::playable-uri",
                    G_CALLBACK (_player_notify_playable_uri), NULL);
  g_signal_connect (remote->player, "next-response",
                    G_CALLBACK (_player_next_response), NULL);
  g_signal_connect (remote->player, "prev-response",
                    G_CALLBACK (_player_prev_response), NULL);

  switch (remote->command) {
    case COMMAND_PLAYER_PLAYING:
      if (0 == g_strcmp0 (remote->player_playing, "true")) {
        yts_vp_player_set_playing (YTS_VP_PLAYER (remote->player), true);
      } else if (0 == g_strcmp0 (remote->player_playing, "false")) {
        yts_vp_player_set_playing (YTS_VP_PLAYER (remote->player), false);
      } else {
        bool playing = yts_vp_player_get_playing (remote->player);
        g_debug ("YtsVPPlayer.playing = %s", playing ? "true" : "false");
      }
      break;
    case COMMAND_PLAYER_VOLUME:
      if (remote->player_volume >= 0.0) {
        yts_vp_player_set_volume (remote->player, remote->player_volume);
      } else {
        double volume = yts_vp_player_get_volume (remote->player);
        g_debug ("YtsVPPlayer.volume = %.2f", volume);
      }
      break;
    case COMMAND_PLAYER_PLAYABLE_URI:
      if (remote->player_playable_uri) {
        yts_vp_player_set_playable_uri (remote->player,
                                        remote->player_playable_uri);
      } else {
        char *playable_uri = yts_vp_player_get_playable_uri (
                                remote->player);
        g_debug ("YtsVPPlayer.playable_uri = %s", playable_uri);
        g_free (playable_uri);
      }
      break;
    case COMMAND_PLAYER_PLAY:
      yts_vp_player_play (remote->player);
      break;
    case COMMAND_PLAYER_PAUSE:
      yts_vp_player_pause (remote->player);
      break;
    case COMMAND_PLAYER_NEXT:
      yts_vp_player_next (remote->player, NULL);
      break;
    case COMMAND_PLAYER_PREV:
      yts_vp_player_prev (remote->player, NULL);
      break;
    default:
      g_debug ("%s : command %i not handled", G_STRLOC, remote->command);
  }
}

static void
_transcript_notify_current_text (YtsVPTranscript *transcript,
                                 GParamSpec       *pspec,
                                 void             *data)
{
  char *current_text;

  current_text = yts_vp_transcript_get_current_text (transcript);
  g_debug ("YtsVPTranscript.current-text = '%s'", current_text);
  g_free (current_text);
}

static void
use_transcript (Remote *remote)
{
  char **locales;
  char  *locales_str;
  char  *locale;

  locales = yts_vp_transcript_get_available_locales (remote->transcript);
  locales_str = g_strjoinv (", ", locales);

  locale = yts_vp_transcript_get_locale (remote->transcript);

  g_signal_connect (remote->transcript, "notify::current-text",
                    G_CALLBACK (_transcript_notify_current_text), NULL);

  g_debug ("Transcript create with locales %s, currently using %s",
           locales_str, locale);

  g_free (locale);
  g_free (locales_str);
  g_strfreev (locales);
}

static void
_proxy_service_proxy_created (YtsProxyService  *service,
                              YtsProxy         *proxy,
                              Remote          *remote)
{
  if (YTS_VP_IS_PLAYER (proxy)) {

    remote->player = g_object_ref (proxy);

    if (remote->command != COMMAND_NONE) {
      use_player (remote);
    } else {
      g_debug ("No command given, not using the player.");
    }

  } else if (YTS_VP_IS_TRANSCRIPT (proxy)) {

    remote->transcript = g_object_ref (proxy);
    use_transcript (remote);

  } else {

    g_warning ("%s : Proxy of type %s not handled",
               G_STRLOC,
               G_OBJECT_TYPE_NAME (proxy));
  }
}

static void
_roster_service_added (YtsRoster   *roster,
                       YtsService  *service,
                       Remote      *remote)
{
  char const  *uid;
  char const  *jid;

  uid = yts_service_get_uid (service);
  jid = yts_service_get_jid (service);

  g_debug ("%s() %s %s", __FUNCTION__, uid, jid);

  if (0 == g_strcmp0 (uid, remote->remote_id) ||
      0 == g_strcmp0 (uid, "org.freedesktop.ytstenut.MockPlayer")) {

    bool ret;

    g_signal_connect (service, "proxy-created",
                      G_CALLBACK (_proxy_service_proxy_created), remote);

    ret = yts_proxy_service_create_proxy (YTS_PROXY_SERVICE (service),
                                           YTS_VP_PLAYER_FQC_ID);
    if (!ret) {
      g_critical ("%s : Failed to create player", G_STRLOC);
    }

    ret = yts_proxy_service_create_proxy (YTS_PROXY_SERVICE (service),
                                           YTS_VP_TRANSCRIPT_FQC_ID);
    if (!ret) {
      g_critical ("%s : Failed to create transcript", G_STRLOC);
    }
  }
}

int
main (int     argc,
      char  **argv)
{
  GOptionContext  *context;
  GOptionGroup    *group;
  YtsClient      *client;
  YtsRoster      *roster;
  GError          *error = NULL;

  Remote remote;

  GOptionEntry player_entries[] = {
    { "remote-id", 0, 0, G_OPTION_ARG_STRING, &remote.remote_id, "Remote-id to connect to", "<remote-id>" },

    { "playing", 0, 0, G_OPTION_ARG_STRING, &remote.player_playing, "Property 'playing'", "<true/false/get>" },
    { "volume", 0, 0, G_OPTION_ARG_DOUBLE, &remote.player_volume, "Property 'volume'", NULL },
    { "playable-uri", 0, 0, G_OPTION_ARG_STRING, &remote.player_playable_uri, "Property 'playable-uri'", NULL },

    { "play", 0, 0, G_OPTION_ARG_NONE, &remote.player_play, "Invoke 'play'", NULL },
    { "pause", 0, 0, G_OPTION_ARG_NONE, &remote.player_pause, "Invoke 'pause'", NULL },
    { "next", 0, 0, G_OPTION_ARG_NONE, &remote.player_next, "Invoke 'next'", NULL },
    { "prev", 0, 0, G_OPTION_ARG_NONE, &remote.player_prev, "Invoke 'prev'", NULL },
    { NULL }
  };

  memset (&remote, 0, sizeof (remote));
  remote.player_volume = -1.0;

  context = g_option_context_new ("- mock player remote");

  group = g_option_group_new ("player",
                              "VideoProfile.Player remote",
                              "This set of remote can be used to exercise the Player interface.",
                              NULL,
                              NULL);
  g_option_group_add_entries (group, player_entries);
  g_option_context_set_main_group (context, group);

  g_option_context_add_group (context, yts_get_option_group ());
  g_option_context_parse (context, &argc, &argv, &error);
  if (error) {
    g_warning ("%s : %s", G_STRLOC, error->message);
    g_clear_error (&error);
    return EXIT_FAILURE;
  }

  if (remote.player_playing) {
    remote.command = COMMAND_PLAYER_PLAYING;
  } else if (remote.player_volume >= 0.0) {
    remote.command = COMMAND_PLAYER_VOLUME;
  } else if (remote.player_playable_uri) {
    remote.command = COMMAND_PLAYER_PLAYABLE_URI;
  } else if (remote.player_play) {
    remote.command = COMMAND_PLAYER_PLAY;
  } else if (remote.player_pause) {
    remote.command = COMMAND_PLAYER_PAUSE;
  } else if (remote.player_next) {
    remote.command = COMMAND_PLAYER_NEXT;
  } else if (remote.player_prev) {
    remote.command = COMMAND_PLAYER_PREV;
  } else {
    g_debug ("No command given, use --help to display commands.");
  }

  client = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP,
                            "org.freedesktop.ytstenut.MockPlayerRemote");
  g_signal_connect (client, "authenticated",
                    G_CALLBACK (_client_authenticated), &remote);
  g_signal_connect (client, "ready",
                    G_CALLBACK (_client_ready), &remote);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (_client_disconnected), &remote);
  g_signal_connect (client, "message",
                    G_CALLBACK (_client_message), &remote);

  roster = yts_client_get_roster (client);
  g_signal_connect (roster, "service-added",
                    G_CALLBACK (_roster_service_added), &remote);

  yts_client_connect (client);

  _mainloop = g_main_loop_new (NULL, false);
  g_main_loop_run (_mainloop);
  g_main_loop_unref (_mainloop);

  if (remote.player)
    g_object_unref (remote.player);
  if (remote.transcript)
    g_object_unref (remote.transcript);

  return EXIT_SUCCESS;
}

