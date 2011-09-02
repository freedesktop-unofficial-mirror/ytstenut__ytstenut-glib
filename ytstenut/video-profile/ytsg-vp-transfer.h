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

#ifndef YTSG_VP_TRANSFER_H
#define YTSG_VP_TRANSFER_H

#include <glib-object.h>
#include <ytstenut/video-profile/ytsg-vp-transmission.h>

G_BEGIN_DECLS

#define YTSG_VP_TRANSFER_FQC_ID \
  "org.freedesktop.ytstenut.VideoProfile.Transfer"

#define YTSG_VP_TYPE_TRANSFER (ytsg_vp_transfer_get_type ())

#define YTSG_VP_TRANSFER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VP_TYPE_TRANSFER, YtsgVPTransfer))

#define YTSG_VP_IS_TRANSFER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VP_TYPE_TRANSFER))

#define YTSG_VP_TRANSFER_GET_INTERFACE(obj)                 \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                    \
                                  YTSG_VP_TYPE_TRANSFER,    \
                                  YtsgVPTransferInterface))

typedef struct YtsgVPTransfer YtsgVPTransfer;

typedef struct  {

  /*< private >*/
  GTypeInterface parent;

  YtsgVPTransmission *
  (*download) (YtsgVPTransfer *self,
               char const     *uri);

  YtsgVPTransmission *
  (*upload) (YtsgVPTransfer *self,
             char const     *uri);

} YtsgVPTransferInterface;

GType
ytsg_vp_transfer_get_type (void) G_GNUC_CONST;

YtsgVPTransmission *
ytsg_vp_transfer_download (YtsgVPTransfer *self,
                           char const     *uri);

YtsgVPTransmission *
ytsg_vp_transfer_upload (YtsgVPTransfer *self,
                         char const     *uri);

G_END_DECLS

#endif /* YTSG_VP_TRANSFER_H */

