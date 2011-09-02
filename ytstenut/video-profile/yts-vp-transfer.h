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

#ifndef YTS_VP_TRANSFER_H
#define YTS_VP_TRANSFER_H

#include <glib-object.h>
#include <ytstenut/video-profile/yts-vp-transmission.h>

G_BEGIN_DECLS

#define YTS_VP_TRANSFER_FQC_ID \
  "org.freedesktop.ytstenut.VideoProfile.Transfer"

#define YTS_VP_TYPE_TRANSFER (yts_vp_transfer_get_type ())

#define YTS_VP_TRANSFER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_VP_TYPE_TRANSFER, YtsVPTransfer))

#define YTS_VP_IS_TRANSFER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_VP_TYPE_TRANSFER))

#define YTS_VP_TRANSFER_GET_INTERFACE(obj)                 \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                    \
                                  YTS_VP_TYPE_TRANSFER,    \
                                  YtsVPTransferInterface))

typedef struct YtsVPTransfer YtsVPTransfer;

typedef struct  {

  /*< private >*/
  GTypeInterface parent;

  YtsVPTransmission *
  (*download) (YtsVPTransfer *self,
               char const     *uri);

  YtsVPTransmission *
  (*upload) (YtsVPTransfer *self,
             char const     *uri);

} YtsVPTransferInterface;

GType
yts_vp_transfer_get_type (void) G_GNUC_CONST;

YtsVPTransmission *
yts_vp_transfer_download (YtsVPTransfer *self,
                           char const     *uri);

YtsVPTransmission *
yts_vp_transfer_upload (YtsVPTransfer *self,
                         char const     *uri);

G_END_DECLS

#endif /* YTS_VP_TRANSFER_H */

