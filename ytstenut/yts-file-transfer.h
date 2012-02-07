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

#ifndef YTS_FILE_TRANSFER_H
#define YTS_FILE_TRANSFER_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define YTS_TYPE_FILE_TRANSFER yts_file_transfer_get_type ()

#define YTS_FILE_TRANSFER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_FILE_TRANSFER, YtsFileTransfer))

#define YTS_IS_FILE_TRANSFER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_FILE_TRANSFER))

#define YTS_FILE_TRANSFER_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTS_TYPE_FILE_TRANSFER, YtsFileTransferInterface))

typedef struct YtsFileTransfer YtsFileTransfer;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

} YtsFileTransferInterface;

GType
yts_file_transfer_get_type (void) G_GNUC_CONST;

GFile *const
yts_file_transfer_get_file (YtsFileTransfer *self);

float
yts_file_transfer_get_progress (YtsFileTransfer *self);

G_END_DECLS

#endif /* YTS_FILE_TRANSFER_H */

