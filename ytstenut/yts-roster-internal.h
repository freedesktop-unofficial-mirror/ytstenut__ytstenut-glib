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

#ifndef YTS_ROSTER_INTERNAL_H
#define YTS_ROSTER_INTERNAL_H

#include <stdbool.h>
#include <telepathy-glib/connection.h>
#include <ytstenut/yts-contact.h>
#include <ytstenut/yts-roster.h>

#define YTS_ROSTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_ROSTER, YtsRosterClass))

#define YTS_IS_ROSTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_ROSTER))

#define YTS_ROSTER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_ROSTER, YtsRosterClass))

struct YtsRoster {
  GObject parent;
};

typedef struct {
  GObjectClass parent;
} YtsRosterClass;

void
yts_roster_add_contact (YtsRoster   *roster,
                        YtsContact  *contact);

void
yts_roster_remove_contact (YtsRoster  *roster,
                           YtsContact *contact,
                           bool        dispose);

void
yts_roster_add_service (YtsRoster         *roster,
                        TpConnection      *tp_connection,
                        char const        *contact_id,
                        char const        *service_id,
                        char const        *type,
                        char const *const *caps,
                        GHashTable        *names,
                        GHashTable        *statuses);

void
yts_roster_remove_service_by_id (YtsRoster  *roster,
                                 char const *contact_id,
                                 char const *service_id);

void
yts_roster_clear (YtsRoster *roster);

YtsContact *
yts_roster_find_contact_by_handle (YtsRoster  *roster,
                                   unsigned    handle);

void
yts_roster_update_contact_status (YtsRoster   *self,
                                  char const  *constact_id,
                                  char const  *service_id,
                                  char const  *fqc_id,
                                  char const  *status_xml);

#endif /* YTS_ROSTER_INTERNAL_H */

