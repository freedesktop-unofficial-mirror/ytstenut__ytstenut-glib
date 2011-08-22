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

#include "ytsg-vp-transfer.h"

G_DEFINE_INTERFACE (YtsgVPTransfer,
                    ytsg_vp_transfer,
                    G_TYPE_OBJECT)

static YtsgVPTransmission *
_download (YtsgVPTransfer *self,
           char const     *uri)
{
  g_critical ("%s : Method YtsgVPTransfer.download() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

static YtsgVPTransmission *
_upload (YtsgVPTransfer *self,
         char const     *uri)
{
  g_critical ("%s : Method YtsgVPTransfer.download() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

static void
ytsg_vp_transfer_default_init (YtsgVPTransferInterface *interface)
{
  /* Methods */
  interface->download = _download;
  interface->upload = _upload;

  /* Only to hold the default value */
  g_object_interface_install_property (interface,
                                       g_param_spec_string ("capability", "", "",
                                                            YTSG_VP_TRANSFER_CAPABILITY,
                                                            G_PARAM_STATIC_NAME |
                                                            G_PARAM_STATIC_NICK |
                                                            G_PARAM_STATIC_BLURB));
}

YtsgVPTransmission *
ytsg_vp_transfer_download (YtsgVPTransfer *self,
                           char const     *uri)
{
  g_return_val_if_fail (YTSG_VP_IS_TRANSFER (self), NULL);

  return YTSG_VP_TRANSFER_GET_INTERFACE (self)->download (self, uri);
}

YtsgVPTransmission *
ytsg_vp_transfer_upload (YtsgVPTransfer *self,
                         char const     *uri)
{
  g_return_val_if_fail (YTSG_VP_IS_TRANSFER (self), NULL);

  return YTSG_VP_TRANSFER_GET_INTERFACE (self)->upload (self, uri);
}

