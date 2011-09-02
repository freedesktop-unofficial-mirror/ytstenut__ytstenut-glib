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

#ifndef YTSG_VP_PLAYER_H
#define YTSG_VP_PLAYER_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut/video-profile/ytsg-vp-playable.h>

G_BEGIN_DECLS

#define YTSG_VP_PLAYER_FQC_ID "org.freedesktop.ytstenut.VideoProfile.Player"

#define YTSG_VP_TYPE_PLAYER (ytsg_vp_player_get_type ())

#define YTSG_VP_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VP_TYPE_PLAYER, YtsgVPPlayer))

#define YTSG_VP_IS_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VP_TYPE_PLAYER))

#define YTSG_VP_PLAYER_GET_INTERFACE(obj)                 \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                  \
                                  YTSG_VP_TYPE_PLAYER,    \
                                  YtsgVPPlayerInterface))

typedef struct YtsgVPPlayer YtsgVPPlayer;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

  void
  (*play) (YtsgVPPlayer *self);

  void
  (*pause) (YtsgVPPlayer *self);

  void
  (*next) (YtsgVPPlayer *self,
           char const   *invocation_id);

  void
  (*prev) (YtsgVPPlayer *self,
           char const   *invocation_id);

} YtsgVPPlayerInterface;

GType
ytsg_vp_player_get_type (void) G_GNUC_CONST;

YtsgVPPlayable *
ytsg_vp_player_get_playable (YtsgVPPlayer *self);

void
ytsg_vp_player_set_playable (YtsgVPPlayer   *self,
                             YtsgVPPlayable *playable);

bool
ytsg_vp_player_get_playing (YtsgVPPlayer *self);

void
ytsg_vp_player_set_playing (YtsgVPPlayer *self,
                            bool          playing);

char *
ytsg_vp_player_get_playable_uri (YtsgVPPlayer *self);

void
ytsg_vp_player_set_playable_uri (YtsgVPPlayer *self,
                                 char const   *playable_uri);

double
ytsg_vp_player_get_volume (YtsgVPPlayer *self);

void
ytsg_vp_player_set_volume (YtsgVPPlayer *self,
                           double        volume);


void
ytsg_vp_player_play (YtsgVPPlayer *self);

void
ytsg_vp_player_pause (YtsgVPPlayer *self);

void
ytsg_vp_player_next (YtsgVPPlayer *self,
                     char const   *invocation_id);

void
ytsg_vp_player_prev (YtsgVPPlayer *self,
                     char const   *invocation_id);

/* Protected */

void
ytsg_vp_player_next_return (YtsgVPPlayer  *self,
                            char const    *invocation_id,
                            bool           response);

void
ytsg_vp_player_prev_return (YtsgVPPlayer  *self,
                            char const    *invocation_id,
                            bool           response);

G_END_DECLS

#endif /* YTSG_VP_PLAYER_H */

