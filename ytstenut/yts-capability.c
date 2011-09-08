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

#include "yts-capability.h"

G_DEFINE_INTERFACE (YtsCapability,
                    yts_capability,
                    G_TYPE_OBJECT)

static void
yts_capability_default_init (YtsCapabilityInterface *interface)
{
  GParamSpec *pspec;

  pspec = g_param_spec_boxed ("fqc-ids", "", "",
                              G_TYPE_STRV,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);
  g_object_interface_install_property (interface, pspec);
}

char **
yts_capability_get_fqc_ids (YtsCapability *self)
{
  char **fqc_ids;

  g_return_val_if_fail (YTS_IS_CAPABILITY (self), NULL);

  fqc_ids = NULL;
  g_object_get (self, "fqc-ids", &fqc_ids, NULL);

  return fqc_ids;
}

