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

#ifndef YTS_VP_QUERY_H
#define YTS_VP_QUERY_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut/yts-types.h>

G_BEGIN_DECLS

#define YTS_VP_TYPE_QUERY  (yts_vp_query_get_type ())

#define YTS_VP_QUERY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_VP_TYPE_QUERY, YtsVPQuery))

#define YTS_VP_IS_QUERY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_VP_TYPE_QUERY))

#define YTS_VP_QUERY_GET_INTERFACE(obj)                  \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                  \
                                  YTS_VP_TYPE_QUERY,     \
                                  YtsVPQueryInterface))

typedef struct YtsVPQuery YtsVPQuery;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

  /* Signals */
  bool
  (*result) (YtsVPQuery  *self,
             GList        *playables,
             unsigned      progress);

} YtsVPQueryInterface;

GType
yts_vp_query_get_type (void) G_GNUC_CONST;

unsigned
yts_vp_query_get_max_results (YtsVPQuery *self);

unsigned
yts_vp_query_get_progress (YtsVPQuery *self);

GList *
yts_vp_query_get_results (YtsVPQuery *self);

YtsVPQueryResultOrder
yts_vp_query_get_result_order (YtsVPQuery *self);

G_END_DECLS

#endif /* YTS_VP_QUERY_H */

