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

#include "ytsg-vs-playable.h"
#include "ytsg-vs-player.h"

G_DEFINE_INTERFACE (YtsgVSPlayer, ytsg_vs_player, G_TYPE_OBJECT)

static void
ytsg_vs_player_default_init (YtsgVSPlayerInterface *interface)
{
  g_object_interface_install_property (interface,
                                       g_param_spec_object ("playable", "", "",
                                                            YTSG_VS_TYPE_PLAYABLE,
                                                            G_PARAM_READWRITE));

  g_object_interface_install_property (interface,
                                       g_param_spec_boolean ("playing", "", "",
                                                             false,
                                                             G_PARAM_READWRITE));

  g_object_interface_install_property (interface,
                                       g_param_spec_double ("volume", "", "",
                                                            0.0, 1.0, 0.5,
                                                            G_PARAM_READWRITE));
}

YtsgVSPlayable *
ytsg_vs_player_get_playable (YtsgVSPlayer *self)
{
  YtsgVSPlayable *playable;

  g_return_val_if_fail (YTSG_VS_IS_PLAYER (self), NULL);

  playable = NULL;
  g_object_get (G_OBJECT (self), "playable", &playable, NULL);
  return playable;
}

void
ytsg_vs_player_set_playable (YtsgVSPlayer   *self,
                             YtsgVSPlayable *playable)
{
  g_return_if_fail (YTSG_VS_IS_PLAYER (self));

  g_object_set (G_OBJECT (self), "playable", playable, NULL);
}

bool
ytsg_vs_player_get_playing (YtsgVSPlayer *self)
{
  bool playing;

  g_return_val_if_fail (YTSG_VS_IS_PLAYER (self), false);

  playing = false;
  g_object_get (G_OBJECT (self), "playing", &playing, NULL);
  return playing;
}

void
ytsg_vs_player_set_playing (YtsgVSPlayer *self,
                            bool          playing)
{
  g_return_if_fail (YTSG_VS_IS_PLAYER (self));

  g_object_set (G_OBJECT (self), "playing", playing, NULL);
}

double
ytsg_vs_player_get_volume (YtsgVSPlayer *self)
{
  double volume;

  g_return_val_if_fail (YTSG_VS_IS_PLAYER (self), 0.0);

  volume = 0.0;
  g_object_get (G_OBJECT (self), "volume", &volume, NULL);
  return volume;
}

void
ytsg_vs_player_set_volume (YtsgVSPlayer *self,
                           double        volume)
{
  g_return_if_fail (YTSG_VS_IS_PLAYER (self));

  g_object_set (G_OBJECT (self), "volume", volume, NULL);
}

