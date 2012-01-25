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

#ifndef YTS_CLIENT_STATUS_H
#define YTS_CLIENT_STATUS_H

#include <stdbool.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define YTS_TYPE_CLIENT_STATUS yts_client_status_get_type()

#define YTS_CLIENT_STATUS(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_CLIENT_STATUS, YtsClientStatus))

#define YTS_CLIENT_STATUS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_CLIENT_STATUS, YtsClientStatusClass))

#define YTS_IS_CLIENT_STATUS(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_CLIENT_STATUS))

#define YTS_IS_CLIENT_STATUS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_CLIENT_STATUS))

#define YTS_CLIENT_STATUS_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_CLIENT_STATUS, YtsClientStatusClass))

typedef struct {
  GObject parent;
} YtsClientStatus;

typedef struct {
  GObjectClass parent;
} YtsClientStatusClass;

GType
yts_client_status_get_type (void);

YtsClientStatus *
yts_client_status_new (char const *service_id);

void
yts_client_status_add_capability (YtsClientStatus *self,
                                  char const      *capability);

void
yts_client_status_revoke_capability (YtsClientStatus  *self,
                                     char const       *capability);

char const *
yts_client_status_set (YtsClientStatus    *self,
                       char const         *capability,
                       char const *const  *attribs,
                       char const         *xml_payload);

bool
yts_client_status_clear (YtsClientStatus  *self,
                         char const       *capability);

typedef bool 
(*YtsClientStatusIterator) (YtsClientStatus const *self,
                            char const            *capability,
                            char const            *status_xml,
                            void                  *data);

bool
yts_client_status_foreach (YtsClientStatus          *self,
                           YtsClientStatusIterator   iterator,
                           void                     *user_data);

G_END_DECLS

#endif /* YTS_CLIENT_STATUS_H */

