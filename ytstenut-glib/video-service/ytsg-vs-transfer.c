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

#include "ytsg-vs-transfer.h"

G_DEFINE_INTERFACE (YtsgVSTransfer, ytsg_vs_transfer, G_TYPE_OBJECT)

static YtsgVSTransmission *
_download (YtsgVSTransfer *self,
           char const     *uri)
{
  g_critical ("%s : Method YtsgVSTransfer.download() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

static YtsgVSTransmission *
_upload (YtsgVSTransfer *self,
         char const     *uri)
{
  g_critical ("%s : Method YtsgVSTransfer.download() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

static void
ytsg_vs_transfer_default_init (YtsgVSTransferInterface *interface)
{
  /* Methods */
  interface->download = _download;
  interface->upload = _upload;

  /* Only to hold the default value */
  g_object_interface_install_property (interface,
                                       g_param_spec_string ("capability", "", "",
                                                            YTSG_VS_TRANSFER_CAPABILITY,
                                                            G_PARAM_STATIC_NAME |
                                                            G_PARAM_STATIC_NICK |
                                                            G_PARAM_STATIC_BLURB));
}

YtsgVSTransmission *
ytsg_vs_transfer_download (YtsgVSTransfer *self,
                           char const     *uri)
{
  g_return_val_if_fail (YTSG_VS_IS_TRANSFER (self), NULL);

  return YTSG_VS_TRANSFER_GET_INTERFACE (self)->download (self, uri);
}

YtsgVSTransmission *
ytsg_vs_transfer_upload (YtsgVSTransfer *self,
                         char const     *uri)
{
  g_return_val_if_fail (YTSG_VS_IS_TRANSFER (self), NULL);

  return YTSG_VS_TRANSFER_GET_INTERFACE (self)->upload (self, uri);
}

