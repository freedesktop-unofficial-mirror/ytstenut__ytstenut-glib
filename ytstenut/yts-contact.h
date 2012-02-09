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
 * Authored by: Tomas Frydrych <tf@linux.intel.com>
 */

#ifndef YTS_CONTACT_H
#define YTS_CONTACT_H

#include <stdbool.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <ytstenut/yts-service.h>

G_BEGIN_DECLS

#define YTS_TYPE_CONTACT (yts_contact_get_type ())

#define YTS_CONTACT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_CONTACT, YtsContact))

#define YTS_IS_CONTACT(obj) \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_CONTACT))

typedef struct YtsContact YtsContact;

GType
yts_contact_get_type (void) G_GNUC_CONST;

char const *
yts_contact_get_id (YtsContact const *self);

char const *
yts_contact_get_name (YtsContact const *self);

/**
 * YtsContactServiceIterator:
 * @self: object owning @service.
 * @service_id: service ID.
 * @service: service instance.
 * @user_data: data passed to yts_contact_foreach_service().
 *
 * Callback signature for iterating a an #YtsContact's services.
 *
 * Returns: <literal>false</literal> to abort the iteration.
 *
 * Since: 0.4
 */
typedef bool
(*YtsContactServiceIterator) (YtsContact   *self,
                              char const  *service_id,
                              YtsService  *service,
                              void        *user_data);

bool
yts_contact_foreach_service (YtsContact                 *self,
                             YtsContactServiceIterator   iterator,
                             void                       *user_data);

G_END_DECLS

#endif /* YTS_CONTACT_H */

