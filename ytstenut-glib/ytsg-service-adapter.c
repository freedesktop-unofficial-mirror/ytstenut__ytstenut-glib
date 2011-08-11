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

#include "ytsg-marshal.h"
#include "ytsg-service-adapter.h"

G_DEFINE_INTERFACE (YtsgServiceAdapter, ytsg_service_adapter, G_TYPE_OBJECT)

enum {
  RESPONSE_SIGNAL,
  ERROR_SIGNAL,

  N_SIGNALS
};

static unsigned int _signals[N_SIGNALS] = { 0, };

static void
ytsg_service_adapter_default_init (YtsgServiceAdapterInterface *interface)
{
  GParamFlags param_flags;

  /* Properties */

  param_flags = G_PARAM_READABLE |
                G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB;

  g_object_interface_install_property (interface,
                                       g_param_spec_string ("capability", "", "",
                                                            NULL,
                                                            param_flags));

  g_object_interface_install_property (interface,
                                       g_param_spec_variant ("status", "", "",
                                                             g_variant_type_new ("a{sv}"),
                                                             NULL,
                                                             param_flags));

  /* Signals */

  _signals[RESPONSE_SIGNAL] = g_signal_new ("response",
                                            YTSG_TYPE_SERVICE_ADAPTER,
                                            G_SIGNAL_RUN_LAST,
                                            G_STRUCT_OFFSET (YtsgServiceAdapterInterface,
                                                             response),
                                            NULL, NULL,
                                            ytsg_marshal_VOID__STRING_BOXED,
                                            G_TYPE_NONE, 2,
                                            G_TYPE_STRING, G_TYPE_VARIANT);

  _signals[ERROR_SIGNAL] = g_signal_new ("error",
                                         YTSG_TYPE_SERVICE_ADAPTER,
                                         G_SIGNAL_RUN_LAST,
                                         G_STRUCT_OFFSET (YtsgServiceAdapterInterface,
                                                          error),
                                         NULL, NULL,
                                         ytsg_marshal_VOID__STRING_BOXED,
                                         G_TYPE_NONE, 2,
                                         G_TYPE_STRING, G_TYPE_ERROR);
}

