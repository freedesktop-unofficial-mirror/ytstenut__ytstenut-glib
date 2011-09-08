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

#ifndef YTS_VP_PLAYER_H
#define YTS_VP_PLAYER_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut/video-profile/yts-vp-playable.h>

G_BEGIN_DECLS

#define YTS_VP_PLAYER_FQC_ID "org.freedesktop.ytstenut.VideoProfile.Player"

#define YTS_VP_TYPE_PLAYER (yts_vp_player_get_type ())

#define YTS_VP_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_VP_TYPE_PLAYER, YtsVPPlayer))

#define YTS_VP_IS_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_VP_TYPE_PLAYER))

#define YTS_VP_PLAYER_GET_INTERFACE(obj)                 \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                  \
                                  YTS_VP_TYPE_PLAYER,    \
                                  YtsVPPlayerInterface))

typedef struct YtsVPPlayer YtsVPPlayer;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

  void
  (*play) (YtsVPPlayer *self);

  void
  (*pause) (YtsVPPlayer *self);

  void
  (*next) (YtsVPPlayer *self,
           char const   *invocation_id);

  void
  (*prev) (YtsVPPlayer *self,
           char const   *invocation_id);

} YtsVPPlayerInterface;

GType
yts_vp_player_get_type (void) G_GNUC_CONST;

YtsVPPlayable *
yts_vp_player_get_playable (YtsVPPlayer *self);

void
yts_vp_player_set_playable (YtsVPPlayer   *self,
                             YtsVPPlayable *playable);

bool
yts_vp_player_get_playing (YtsVPPlayer *self);

void
yts_vp_player_set_playing (YtsVPPlayer *self,
                            bool          playing);

char *
yts_vp_player_get_playable_uri (YtsVPPlayer *self);

void
yts_vp_player_set_playable_uri (YtsVPPlayer *self,
                                 char const   *playable_uri);

double
yts_vp_player_get_volume (YtsVPPlayer *self);

void
yts_vp_player_set_volume (YtsVPPlayer *self,
                           double        volume);


void
yts_vp_player_play (YtsVPPlayer *self);

void
yts_vp_player_pause (YtsVPPlayer *self);

void
yts_vp_player_next (YtsVPPlayer *self,
                     char const   *invocation_id);

void
yts_vp_player_prev (YtsVPPlayer *self,
                     char const   *invocation_id);

/* Protected */

void
yts_vp_player_next_return (YtsVPPlayer  *self,
                            char const    *invocation_id,
                            bool           response);

void
yts_vp_player_prev_return (YtsVPPlayer  *self,
                            char const    *invocation_id,
                            bool           response);

G_END_DECLS

#endif /* YTS_VP_PLAYER_H */

