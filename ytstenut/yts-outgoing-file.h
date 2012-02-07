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

#ifndef YTS_OUTGOING_FILE_H
#define YTS_OUTGOING_FILE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTS_TYPE_OUTGOING_FILE yts_outgoing_file_get_type()

#define YTS_OUTGOING_FILE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_OUTGOING_FILE, YtsOutgoingFile))

#define YTS_IS_OUTGOING_FILE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_OUTGOING_FILE))

typedef struct YtsOutgoingFile YtsOutgoingFile;

GType
yts_outgoing_file_get_type (void) G_GNUC_CONST;

#define YTS_OUTGOING_FILE_ERROR g_quark_from_static_string (__FILE__)

enum {
  YTS_OUTGOING_FILE_ERROR_NO_ACCOUNT,
  YTS_OUTGOING_FILE_ERROR_NO_CONNECTION,
  YTS_OUTGOING_FILE_ERROR_NO_FILE,
  YTS_OUTGOING_FILE_ERROR_READ_FAILED,
  YTS_OUTGOING_FILE_ERROR_NO_RECIPIENT_CONTACT,
  YTS_OUTGOING_FILE_ERROR_NO_RECIPIENT_SERVICE,
  YTS_OUTGOING_FILE_ERROR_CHANNEL_FAILED,
  YTS_OUTGOING_FILE_ERROR_TRANSFER_FAILED,
  YTS_OUTGOING_FILE_ERROR_LOCAL,
  YTS_OUTGOING_FILE_ERROR_REMOTE,
  YTS_OUTGOING_FILE_ERROR_CHANNEL_CLOSE_FAILED
};

char const *
yts_outgoing_file_get_description (YtsOutgoingFile *self);

G_END_DECLS

#endif /* YTS_OUTGOING_FILE_H */

