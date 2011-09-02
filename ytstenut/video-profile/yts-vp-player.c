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

#include "yts-capability.h"
#include "yts-marshal.h"
#include "yts-vp-playable.h"
#include "yts-vp-player.h"

G_DEFINE_INTERFACE (YtsVPPlayer,
                    yts_vp_player,
                    YTS_TYPE_CAPABILITY)

enum {
  SIG_NEXT_RESPONSE,
  SIG_PREV_RESPONSE,
  N_SIGNALS
};

static unsigned _signals[N_SIGNALS] = { 0, };

static void
_play (YtsVPPlayer *self)
{
  g_critical ("%s : Method YtsVPPlayer.play() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
}

static void
_pause (YtsVPPlayer *self)
{
  g_critical ("%s : Method YtsVPPlayer.pause() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
}

static void
_next (YtsVPPlayer *self,
       char const   *invocation_id)
{
  g_critical ("%s : Method YtsVPPlayer.next() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
}

static void
_prev (YtsVPPlayer *self,
       char const   *invocation_id)
{
  g_critical ("%s : Method YtsVPPlayer.prev() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
}

static void
yts_vp_player_default_init (YtsVPPlayerInterface *interface)
{
  GParamSpec *pspec;

  interface->play = _play;
  interface->pause = _pause;
  interface->next = _next;
  interface->prev = _prev;

  pspec = g_param_spec_object ("playable", "", "",
                               YTS_VP_TYPE_PLAYABLE,
                               G_PARAM_READWRITE |
                               G_PARAM_STATIC_STRINGS);
  g_object_interface_install_property (interface, pspec);

  pspec = g_param_spec_boolean ("playing", "", "",
                                false,
                                G_PARAM_READWRITE |
                                G_PARAM_STATIC_STRINGS);
  g_object_interface_install_property (interface, pspec);

  pspec = g_param_spec_double ("volume", "", "",
                               0.0, 1.0, 0.5,
                               G_PARAM_READWRITE |
                               G_PARAM_STATIC_STRINGS);
  g_object_interface_install_property (interface, pspec);

  pspec = g_param_spec_string ("playable-uri", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_STATIC_STRINGS);
  g_object_interface_install_property (interface, pspec);

  /* Signals */

  _signals[SIG_NEXT_RESPONSE] = g_signal_new ("next-response",
                                              YTS_VP_TYPE_PLAYER,
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
                                              yts_marshal_VOID__STRING_BOOLEAN,
                                              G_TYPE_NONE,
                                              2, G_TYPE_STRING, G_TYPE_BOOLEAN);

  _signals[SIG_PREV_RESPONSE] = g_signal_new ("prev-response",
                                               YTS_VP_TYPE_PLAYER,
                                               G_SIGNAL_RUN_LAST,
                                               0, NULL, NULL,
                                               yts_marshal_VOID__STRING_BOOLEAN,
                                               G_TYPE_NONE,
                                               2, G_TYPE_STRING, G_TYPE_BOOLEAN);
}

YtsVPPlayable *
yts_vp_player_get_playable (YtsVPPlayer *self)
{
  YtsVPPlayable *playable;

  g_return_val_if_fail (YTS_VP_IS_PLAYER (self), NULL);

  playable = NULL;
  g_object_get (G_OBJECT (self), "playable", &playable, NULL);
  return playable;
}

void
yts_vp_player_set_playable (YtsVPPlayer   *self,
                             YtsVPPlayable *playable)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  g_object_set (G_OBJECT (self), "playable", playable, NULL);
}

bool
yts_vp_player_get_playing (YtsVPPlayer *self)
{
  bool playing;

  g_return_val_if_fail (YTS_VP_IS_PLAYER (self), false);

  playing = false;
  g_object_get (G_OBJECT (self), "playing", &playing, NULL);
  return playing;
}

void
yts_vp_player_set_playing (YtsVPPlayer *self,
                            bool          playing)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  g_object_set (G_OBJECT (self), "playing", playing, NULL);
}

double
yts_vp_player_get_volume (YtsVPPlayer *self)
{
  double volume;

  g_return_val_if_fail (YTS_VP_IS_PLAYER (self), 0.0);

  volume = 0.0;
  g_object_get (G_OBJECT (self), "volume", &volume, NULL);
  return volume;
}

void
yts_vp_player_set_volume (YtsVPPlayer *self,
                           double        volume)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  g_object_set (G_OBJECT (self), "volume", volume, NULL);
}

char *
yts_vp_player_get_playable_uri (YtsVPPlayer *self)
{
  char *playable_uri;

  g_return_val_if_fail (YTS_VP_IS_PLAYER (self), NULL);

  playable_uri = NULL;
  g_object_get (G_OBJECT (self), "playable-uri", &playable_uri, NULL);
  return playable_uri;
}

void
yts_vp_player_set_playable_uri (YtsVPPlayer *self,
                                 char const   *playable_uri)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  g_object_set (G_OBJECT (self), "playable-uri", playable_uri, NULL);
}

void
yts_vp_player_play (YtsVPPlayer *self)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  YTS_VP_PLAYER_GET_INTERFACE (self)->play (self);
}

void
yts_vp_player_pause (YtsVPPlayer *self)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  YTS_VP_PLAYER_GET_INTERFACE (self)->pause (self);
}

void
yts_vp_player_next (YtsVPPlayer *self,
                     char const   *invocation_id)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  YTS_VP_PLAYER_GET_INTERFACE (self)->next (self, invocation_id);
}

void
yts_vp_player_next_return (YtsVPPlayer  *self,
                            char const    *invocation_id,
                            bool           response)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  g_signal_emit (self, _signals[SIG_NEXT_RESPONSE], 0,
                 invocation_id, response);
}

void
yts_vp_player_prev (YtsVPPlayer *self,
                     char const   *invocation_id)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  YTS_VP_PLAYER_GET_INTERFACE (self)->prev (self, invocation_id);
}

void
yts_vp_player_prev_return (YtsVPPlayer  *self,
                            char const    *invocation_id,
                            bool           response)
{
  g_return_if_fail (YTS_VP_IS_PLAYER (self));

  g_signal_emit (self, _signals[SIG_PREV_RESPONSE], 0,
                 invocation_id, response);
}

