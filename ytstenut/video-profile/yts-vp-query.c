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

#include "yts-enum-types.h"
#include "yts-marshal.h"
#include "yts-vp-query.h"
#include "config.h"

G_DEFINE_INTERFACE (YtsVPQuery,
                    yts_vp_query,
                    G_TYPE_OBJECT)

enum {
  RESULT_SIGNAL,

  N_SIGNALS
};

static unsigned _signals[N_SIGNALS] = { 0, };

static void
yts_vp_query_default_init (YtsVPQueryInterface *interface)
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

  /* GList*<YtsVPPlayable> */
  g_object_interface_install_property (interface,
                                       g_param_spec_pointer ("results", "", "",
                                                             G_PARAM_READABLE));

  g_object_interface_install_property (interface,
                                       g_param_spec_enum ("result-order", "", "",
                                                          YTS_TYPE_VP_QUERY_RESULT_ORDER,
                                                          YTS_VP_QUERY_NONE,
                                                          G_PARAM_READABLE));

  /* GPtrArray<char const *> */
  g_object_interface_install_property (interface,
                                       g_param_spec_boxed ("search-tokens", "", "",
                                                           G_TYPE_PTR_ARRAY,
                                                           G_PARAM_READABLE));

  _signals[RESULT_SIGNAL] = g_signal_new ("result",
                                          YTS_VP_TYPE_QUERY,
                                          G_SIGNAL_RUN_LAST,
                                          G_STRUCT_OFFSET (YtsVPQueryInterface, result),
                                          NULL, NULL,
                                          yts_marshal_BOOLEAN__POINTER_UINT,
                                          G_TYPE_BOOLEAN, 2,
                                          G_TYPE_POINTER, G_TYPE_UINT);
}

unsigned
yts_vp_query_get_max_results (YtsVPQuery *self)
{
  unsigned max_results;

  g_return_val_if_fail (YTS_VP_IS_QUERY (self), 0);

  max_results = 0;
  g_object_get (G_OBJECT (self), "max-results", &max_results, NULL);
  return max_results;
}

unsigned
yts_vp_query_get_progress (YtsVPQuery *self)
{
  unsigned progress;

  g_return_val_if_fail (YTS_VP_IS_QUERY (self), 0);

  progress = 0;
  g_object_get (G_OBJECT (self), "progress", &progress, NULL);
  return progress;
}

GList *
yts_vp_query_get_results (YtsVPQuery *self)
{
  GList *results;

  g_return_val_if_fail (YTS_VP_IS_QUERY (self), NULL);

  results = NULL;
  g_object_get (G_OBJECT (self), "results", &results, NULL);
  return results;
}

YtsVPQueryResultOrder
yts_vp_query_get_result_order (YtsVPQuery *self)
{
  YtsVPQueryResultOrder result_order;

  g_return_val_if_fail (YTS_VP_IS_QUERY (self), YTS_VP_QUERY_NONE);

  result_order = YTS_VP_QUERY_NONE;
  g_object_get (G_OBJECT (self), "result-order", &result_order, NULL);
  return result_order;
}

