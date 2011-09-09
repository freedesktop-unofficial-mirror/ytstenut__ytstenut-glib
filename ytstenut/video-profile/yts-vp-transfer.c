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
#include "yts-vp-transfer.h"
#include "config.h"

G_DEFINE_INTERFACE (YtsVPTransfer,
                    yts_vp_transfer,
                    YTS_TYPE_CAPABILITY)

static YtsVPTransmission *
_download (YtsVPTransfer *self,
           char const     *uri)
{
  g_critical ("%s : Method YtsVPTransfer.download() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

static YtsVPTransmission *
_upload (YtsVPTransfer *self,
         char const     *uri)
{
  g_critical ("%s : Method YtsVPTransfer.download() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

static void
yts_vp_transfer_default_init (YtsVPTransferInterface *interface)
{
  /* Methods */
  interface->download = _download;
  interface->upload = _upload;
}

YtsVPTransmission *
yts_vp_transfer_download (YtsVPTransfer *self,
                           char const     *uri)
{
  g_return_val_if_fail (YTS_VP_IS_TRANSFER (self), NULL);

  return YTS_VP_TRANSFER_GET_INTERFACE (self)->download (self, uri);
}

YtsVPTransmission *
yts_vp_transfer_upload (YtsVPTransfer *self,
                         char const     *uri)
{
  g_return_val_if_fail (YTS_VP_IS_TRANSFER (self), NULL);

  return YTS_VP_TRANSFER_GET_INTERFACE (self)->upload (self, uri);
}

