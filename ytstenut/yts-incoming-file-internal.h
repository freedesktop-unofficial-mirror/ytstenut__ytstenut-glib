/*
 * Copyright © 2012 Intel Corp.
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

#ifndef YTS_INCOMING_FILE_INTERNAL_H
#define YTS_INCOMING_FILE_INTERNAL_H

#include <gio/gio.h>
#include <telepathy-glib/telepathy-glib.h>
#include <ytstenut/yts-incoming-file.h>

G_BEGIN_DECLS

#define YTS_INCOMING_FILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_INCOMING_FILE, YtsIncomingFileClass))

#define YTS_IS_INCOMING_FILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_INCOMING_FILE))

#define YTS_INCOMING_FILE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_INCOMING_FILE, YtsIncomingFileClass))

struct YtsIncomingFile {
  GObject parent;
};

typedef struct {
  GObjectClass parent;
} YtsIncomingFileClass;

YtsIncomingFile *
yts_incoming_file_new (TpFileTransferChannel *tp_channel);

GFile *const
yts_incoming_file_get_file (YtsIncomingFile *self);

G_END_DECLS

#endif /* YTS_INCOMING_FILE_INTERNAL_H */

