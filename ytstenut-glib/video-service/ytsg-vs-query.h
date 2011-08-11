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

#ifndef YTSG_VS_QUERY_H
#define YTSG_VS_QUERY_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut-glib/ytsg-types.h>

G_BEGIN_DECLS

#define YTSG_VS_TYPE_QUERY \
  (ytsg_vs_query_get_type ())

#define YTSG_VS_QUERY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VS_TYPE_QUERY, YtsgVSQuery))

#define YTSG_VS_IS_QUERY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VS_TYPE_QUERY))

#define YTSG_VS_QUERY_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTSG_VS_TYPE_QUERY, YtsgVSQueryInterface))

typedef struct YtsgVSQuery YtsgVSQuery;
typedef struct YtsgVSQueryInterface YtsgVSQueryInterface;

struct YtsgVSQueryInterface {

  /*< private >*/
  GTypeInterface parent;

  /* Signals */
  bool
  (*result) (YtsgVSQuery  *self,
             GList        *playables,
             unsigned int  progress);
};

GType
ytsg_vs_query_get_type (void) G_GNUC_CONST;

unsigned int
ytsg_vs_get_max_results (YtsgVSQuery *self);

unsigned int
ytsg_vs_get_progress (YtsgVSQuery *self);

GList *
ytsg_vs_get_results (YtsgVSQuery *self);

YtsgVSQueryResultOrder
ytsg_vs_get_result_order (YtsgVSQuery *self);

G_END_DECLS

#endif /* YTSG_VS_QUERY_H */

