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

#include "ytsg-enum-types.h"
#include "ytsg-marshal.h"
#include "ytsg-vp-query.h"

G_DEFINE_INTERFACE (YtsgVPQuery,
                    ytsg_vp_query,
                    G_TYPE_OBJECT)

enum {
  RESULT_SIGNAL,

  N_SIGNALS
};

static unsigned int _signals[N_SIGNALS] = { 0, };

static void
ytsg_vp_query_default_init (YtsgVPQueryInterface *interface)
{
  /* 0 means no limit */
  g_object_interface_install_property (interface,
                                       g_param_spec_uint ("max-results", "", "",
                                                          0, G_MAXUINT, 0,
                                                          G_PARAM_READABLE));

  g_object_interface_install_property (interface,
                                       g_param_spec_uint ("progress", "", "",
                                                          0, 100, 0,
                                                          G_PARAM_READABLE));

  /* GList*<YtsgVPPlayable> */
  g_object_interface_install_property (interface,
                                       g_param_spec_pointer ("results", "", "",
                                                             G_PARAM_READABLE));

  g_object_interface_install_property (interface,
                                       g_param_spec_enum ("result-order", "", "",
                                                          YTSG_TYPE_VP_QUERY_RESULT_ORDER,
                                                          YTSG_VP_QUERY_NONE,
                                                          G_PARAM_READABLE));

  /* GPtrArray<char const *> */
  g_object_interface_install_property (interface,
                                       g_param_spec_boxed ("search-tokens", "", "",
                                                           G_TYPE_PTR_ARRAY,
                                                           G_PARAM_READABLE));

  _signals[RESULT_SIGNAL] = g_signal_new ("result",
                                          YTSG_VP_TYPE_QUERY,
                                          G_SIGNAL_RUN_LAST,
                                          G_STRUCT_OFFSET (YtsgVPQueryInterface, result),
                                          NULL, NULL,
                                          ytsg_marshal_BOOLEAN__POINTER_UINT,
                                          G_TYPE_BOOLEAN, 2,
                                          G_TYPE_POINTER, G_TYPE_UINT);
}

unsigned int
ytsg_vp_get_max_results (YtsgVPQuery *self)
{
  unsigned int max_results;

  g_return_val_if_fail (YTSG_VP_IS_QUERY (self), 0);

  max_results = 0;
  g_object_get (G_OBJECT (self), "max-results", &max_results, NULL);
  return max_results;
}

unsigned int
ytsg_vp_get_progress (YtsgVPQuery *self)
{
  unsigned int progress;

  g_return_val_if_fail (YTSG_VP_IS_QUERY (self), 0);

  progress = 0;
  g_object_get (G_OBJECT (self), "progress", &progress, NULL);
  return progress;
}

GList *
ytsg_vp_get_results (YtsgVPQuery *self)
{
  GList *results;

  g_return_val_if_fail (YTSG_VP_IS_QUERY (self), NULL);

  results = NULL;
  g_object_get (G_OBJECT (self), "results", &results, NULL);
  return results;
}

YtsgVPQueryResultOrder
ytsg_vp_get_result_order (YtsgVPQuery *self)
{
  YtsgVPQueryResultOrder result_order;

  g_return_val_if_fail (YTSG_VP_IS_QUERY (self), YTSG_VP_QUERY_NONE);

  result_order = YTSG_VP_QUERY_NONE;
  g_object_get (G_OBJECT (self), "result-order", &result_order, NULL);
  return result_order;
}

