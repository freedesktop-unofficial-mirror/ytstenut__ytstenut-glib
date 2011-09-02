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

#include "yts-vp-transmission.h"

G_DEFINE_INTERFACE (YtsVPTransmission,
                    yts_vp_transmission,
                    G_TYPE_OBJECT)

static void
yts_vp_transmission_default_init (YtsVPTransmissionInterface *interface)
{
  g_object_interface_install_property (interface,
                                       g_param_spec_string ("local-uri", "", "",
                                                            NULL,
                                                            G_PARAM_READABLE));

  g_object_interface_install_property (interface,
                                       g_param_spec_uint ("progress", "", "",
                                                           0, 100, 0,
                                                           G_PARAM_READABLE));

  g_object_interface_install_property (interface,
                                       g_param_spec_string ("remote-uri", "", "",
                                                            NULL,
                                                            G_PARAM_READABLE));
}

char *
yts_vp_transmission_get_local_uri (YtsVPTransmission *self)
{
  char *local_uri;

  g_return_val_if_fail (YTS_VP_IS_TRANSMISSION (self), NULL);

  local_uri = NULL;
  g_object_get (G_OBJECT (self), "local-uri", &local_uri, NULL);
  return local_uri;
}

unsigned
yts_vp_transmission_get_progress (YtsVPTransmission *self)
{
  unsigned progress;

  g_return_val_if_fail (YTS_VP_IS_TRANSMISSION (self), 0);

  progress = 0;
  g_object_get (G_OBJECT (self), "progress", &progress, NULL);
  return progress;
}

char *
yts_vp_transmission_get_remote_uri (YtsVPTransmission *self)
{
  char *remote_uri;

  g_return_val_if_fail (YTS_VP_IS_TRANSMISSION (self), NULL);

  remote_uri = NULL;
  g_object_get (G_OBJECT (self), "remote-uri", &remote_uri, NULL);
  return remote_uri;
}

