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

#ifndef YTSG_VP_PLAYER_ADAPTER_H
#define YTSG_VP_PLAYER_ADAPTER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_VP_TYPE_PLAYER_ADAPTER ytsg_vp_player_adapter_get_type()

#define YTSG_VP_PLAYER_ADAPTER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VP_TYPE_PLAYER_ADAPTER, YtsgVPPlayerAdapter))

#define YTSG_VP_PLAYER_ADAPTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTSG_VP_TYPE_PLAYER_ADAPTER, YtsgVPPlayerAdapterClass))

#define YTSG_VP_IS_PLAYER_ADAPTER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VP_TYPE_PLAYER_ADAPTER))

#define YTSG_VP_IS_PLAYER_ADAPTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTSG_VP_TYPE_PLAYER_ADAPTER))

#define YTSG_VP_PLAYER_ADAPTER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTSG_VP_TYPE_PLAYER_ADAPTER, YtsgVPPlayerAdapterClass))

typedef struct {
  YtsgServiceAdapter parent;
} YtsgVPPlayerAdapter;

typedef struct {
  YtsgServiceAdapterClass parent;
} YtsgVPPlayerAdapterClass;

GType
ytsg_vp_player_adapter_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* YTSG_VP_PLAYER_ADAPTER_H */

