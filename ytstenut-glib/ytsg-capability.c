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

#include "ytsg-capability.h"

G_DEFINE_INTERFACE (YtsgCapability,
                    ytsg_capability,
                    G_TYPE_OBJECT)

static void
ytsg_capability_default_init (YtsgCapabilityInterface *interface)
{
  GParamSpec *pspec;

  pspec = g_param_spec_string ("fqc-id", "", "",
                               NULL,
                               G_PARAM_READABLE |
                               G_PARAM_STATIC_STRINGS);
  g_object_interface_install_property (interface, pspec);
}

char *
ytsg_capability_get_fqc_id (YtsgCapability *self)
{
  char *fqc_id;

  g_return_val_if_fail (YTSG_IS_CAPABILITY (self), NULL);

  fqc_id = NULL;
  g_object_get (self, "fqc-id", &fqc_id, NULL);

  return fqc_id;
}

