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

#ifndef YTS_INCOMING_FILE_H
#define YTS_INCOMING_FILE_H

#include <stdbool.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define YTS_TYPE_INCOMING_FILE yts_incoming_file_get_type()

#define YTS_INCOMING_FILE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_INCOMING_FILE, YtsIncomingFile))

#define YTS_IS_INCOMING_FILE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_INCOMING_FILE))

typedef struct YtsIncomingFile YtsIncomingFile;

GType
yts_incoming_file_get_type (void) G_GNUC_CONST;

#define YTS_INCOMING_FILE_ERROR g_quark_from_static_string (__FILE__)

enum {
  YTS_INCOMING_FILE_ERROR_NO_CHANNEL,
  YTS_INCOMING_FILE_ERROR_ACCEPT_FAILED,
  YTS_INCOMING_FILE_ERROR_ALREADY_ACCEPTED,
  YTS_INCOMING_FILE_ERROR_LOCAL,
  YTS_INCOMING_FILE_ERROR_REMOTE
};

bool
yts_incoming_file_accept (YtsIncomingFile  *self,
                          GFile            *file,
                          GError          **error);

bool
yts_incoming_file_reject (YtsIncomingFile  *self,
                          GError          **error);

G_END_DECLS

#endif /* YTS_INCOMING_FILE_H */

