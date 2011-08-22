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

#ifndef YTSG_VP_PLAYABLE_H
#define YTSG_VP_PLAYABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_VP_TYPE_PLAYABLE \
  (ytsg_vp_playable_get_type ())

#define YTSG_VP_PLAYABLE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VP_TYPE_PLAYABLE, YtsgVPPlayable))

#define YTSG_VP_IS_PLAYABLE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VP_TYPE_PLAYABLE))

#define YTSG_VP_PLAYABLE_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTSG_VP_TYPE_PLAYABLE, YtsgVPPlayableInterface))

typedef struct YtsgVPPlayable YtsgVPPlayable;
typedef struct YtsgVPPlayableInterface YtsgVPPlayableInterface;

struct YtsgVPPlayableInterface {

  /*< private >*/
  GTypeInterface parent;
};

GType
ytsg_vp_playable_get_type (void) G_GNUC_CONST;

double
ytsg_vp_playable_get_duration (YtsgVPPlayable *self);

GHashTable *
ytsg_vp_playable_get_metadata (YtsgVPPlayable *self);

char const *
ytsg_vp_playable_get_metadata_attribute (YtsgVPPlayable *self,
                                         char const     *attribute);

double
ytsg_vp_playable_get_position (YtsgVPPlayable *self);

void
ytsg_vp_playable_set_position (YtsgVPPlayable *self,
                               double          position);

char *
ytsg_vp_playable_get_thumbnail (YtsgVPPlayable *self);

char *
ytsg_vp_playable_get_title (YtsgVPPlayable *self);

char *
ytsg_vp_playable_get_uri (YtsgVPPlayable *self);

G_END_DECLS

#endif /* YTSG_VP_PLAYABLE_H */

