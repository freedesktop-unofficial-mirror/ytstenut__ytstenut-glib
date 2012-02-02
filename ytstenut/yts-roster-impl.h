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

#ifndef YTS_ROSTER_IMPL_H
#define YTS_ROSTER_IMPL_H

#include <glib-object.h>
#include <gio/gio.h>
#include <ytstenut/yts-roster-internal.h>
#include <ytstenut/yts-metadata.h>
#include <ytstenut/yts-outgoing-file.h>

G_BEGIN_DECLS

#define YTS_TYPE_ROSTER_IMPL  (yts_roster_impl_get_type ())

#define YTS_ROSTER_IMPL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_ROSTER_IMPL, YtsRosterImpl))

#define YTS_ROSTER_IMPL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_ROSTER_IMPL, YtsRosterImplClass))

#define YTS_IS_ROSTER_IMPL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_ROSTER_IMPL))

#define YTS_IS_ROSTER_IMPL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_ROSTER_IMPL))

#define YTS_ROSTER_IMPL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_ROSTER_IMPL, YtsRosterImplClass))

typedef struct {
  YtsRoster parent;
} YtsRosterImpl;

typedef struct {
  YtsRosterClass parent;
} YtsRosterImplClass;

GType
yts_roster_impl_get_type (void) G_GNUC_CONST;

YtsRoster *
yts_roster_impl_new (void);

void
yts_roster_impl_send_message (YtsRosterImpl *self,
                              YtsContact    *contact,
                              YtsService    *service,
                              YtsMetadata   *message);

YtsOutgoingFile *
yts_roster_impl_send_file (YtsRosterImpl   *self,
                           YtsContact      *contact,
                           YtsService      *service,
                           GFile           *file,
                           char const      *description,
                           GError         **error_out);

G_END_DECLS

#endif /* YTS_ROSTER_IMPL_H */

