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

#ifndef YTS_OUTGOING_FILE_INTERNAL_H
#define YTS_OUTGOING_FILE_INTERNAL_H

#include <gio/gio.h>
#include <telepathy-glib/account.h>
#include <ytstenut/yts-outgoing-file.h>

G_BEGIN_DECLS

#define YTS_OUTGOING_FILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_OUTGOING_FILE, YtsOutgoingFileClass))

#define YTS_IS_OUTGOING_FILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_OUTGOING_FILE))

#define YTS_OUTGOING_FILE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_OUTGOING_FILE, YtsOutgoingFileClass))

struct YtsOutgoingFile {
  GObject parent;
};

typedef struct {
  GObjectClass parent;
} YtsOutgoingFileClass;

YtsOutgoingFile *
yts_outgoing_file_new (TpAccount  *tp_account,
                       GFile      *file,
                       char const *sender_service_id,
                       char const *recipient_contact_id,
                       char const *recipient_service_id,
                       char const *description);

GFile *const
yts_outgoing_file_get_file (YtsOutgoingFile *self);

G_END_DECLS

#endif /* YTS_OUTGOING_FILE_INTERNAL_H */

