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

#ifndef YTSG_VP_QUERY_H
#define YTSG_VP_QUERY_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut/ytsg-types.h>

G_BEGIN_DECLS

#define YTSG_VP_TYPE_QUERY  (ytsg_vp_query_get_type ())

#define YTSG_VP_QUERY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VP_TYPE_QUERY, YtsgVPQuery))

#define YTSG_VP_IS_QUERY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VP_TYPE_QUERY))

#define YTSG_VP_QUERY_GET_INTERFACE(obj)                  \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                  \
                                  YTSG_VP_TYPE_QUERY,     \
                                  YtsgVPQueryInterface))

typedef struct YtsgVPQuery YtsgVPQuery;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

  /* Signals */
  bool
  (*result) (YtsgVPQuery  *self,
             GList        *playables,
             unsigned      progress);

} YtsgVPQueryInterface;

GType
ytsg_vp_query_get_type (void) G_GNUC_CONST;

unsigned
ytsg_vp_get_max_results (YtsgVPQuery *self);

unsigned
ytsg_vp_get_progress (YtsgVPQuery *self);

GList *
ytsg_vp_get_results (YtsgVPQuery *self);

YtsgVPQueryResultOrder
ytsg_vp_get_result_order (YtsgVPQuery *self);

G_END_DECLS

#endif /* YTSG_VP_QUERY_H */

