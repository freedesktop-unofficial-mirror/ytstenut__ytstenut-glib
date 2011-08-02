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

#ifndef YTSG_VS_PLAYABLE_H
#define YTSG_VS_PLAYABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_VS_TYPE_PLAYABLE \
  (ytsg_vs_playable_get_type ())

#define YTSG_VS_PLAYABLE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VS_TYPE_PLAYABLE, YtsgVSPlayable))

#define YTSG_VS_IS_PLAYABLE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VS_TYPE_PLAYABLE))

#define YTSG_VS_PLAYABLE_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTSG_VS_TYPE_PLAYABLE, YtsgVSPlayableInterface))

typedef struct YtsgVSPlayable YtsgVSPlayable;
typedef struct YtsgVSPlayableInterface YtsgVSPlayableInterface;

struct YtsgVSPlayableInterface {

  /*< private >*/
  GTypeInterface parent;
};

GType
ytsg_vs_playable_get_type (void) G_GNUC_CONST;

char *
ytsg_vs_playable_get_uri (YtsgVSPlayable *self);

char *
ytsg_vs_playable_get_title (YtsgVSPlayable *self);

double
ytsg_vs_playable_get_duration (YtsgVSPlayable *self);

double
ytsg_vs_playable_get_position (YtsgVSPlayable *self);

void
ytsg_vs_playable_set_position (YtsgVSPlayable *self,
                               double          position);

GHashTable *
ytsg_vs_playable_get_metadata (YtsgVSPlayable *self);

char const *
ytsg_vs_playable_get_metadata_attribute (YtsgVSPlayable *self,
                                         char const     *attribute);

G_END_DECLS

#endif /* YTSG_VS_PLAYABLE_H */

