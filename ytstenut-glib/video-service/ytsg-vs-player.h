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

#ifndef YTSG_VS_PLAYER_H
#define YTSG_VS_PLAYER_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut-glib/video-service/ytsg-vs-playable.h>

G_BEGIN_DECLS

#define YTSG_VS_PLAYER_CAPABILITY "org.freedesktop.ytstenut.videoservice.Player"

#define YTSG_VS_TYPE_PLAYER \
  (ytsg_vs_player_get_type ())

#define YTSG_VS_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VS_TYPE_PLAYER, YtsgVSPlayer))

#define YTSG_VS_IS_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VS_TYPE_PLAYER))

#define YTSG_VS_PLAYER_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTSG_VS_TYPE_PLAYER, YtsgVSPlayerInterface))

typedef struct YtsgVSPlayer YtsgVSPlayer;
typedef struct YtsgVSPlayerInterface YtsgVSPlayerInterface;

struct YtsgVSPlayerInterface {

  /*< private >*/
  GTypeInterface parent;

  void
  (*play) (YtsgVSPlayer *self);

  void
  (*pause) (YtsgVSPlayer *self);

  void
  (*next) (YtsgVSPlayer *self);

  void
  (*prev) (YtsgVSPlayer *self);
};

GType
ytsg_vs_player_get_type (void) G_GNUC_CONST;

YtsgVSPlayable *
ytsg_vs_player_get_playable (YtsgVSPlayer *self);

void
ytsg_vs_player_set_playable (YtsgVSPlayer   *self,
                             YtsgVSPlayable *playable);

bool
ytsg_vs_player_get_playing (YtsgVSPlayer *self);

void
ytsg_vs_player_set_playing (YtsgVSPlayer *self,
                            bool          playing);

double
ytsg_vs_player_get_volume (YtsgVSPlayer *self);

void
ytsg_vs_player_set_volume (YtsgVSPlayer *self,
                           double        volume);


void
ytsg_vs_player_play (YtsgVSPlayer *self);

void
ytsg_vs_player_pause (YtsgVSPlayer *self);

void
ytsg_vs_player_next (YtsgVSPlayer *self);

void
ytsg_vs_player_prev (YtsgVSPlayer *self);

G_END_DECLS

#endif /* YTSG_VS_PLAYER_H */

