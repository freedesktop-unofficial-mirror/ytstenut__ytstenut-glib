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

#ifndef YTS_ROSTER_H
#define YTS_ROSTER_H

#include <glib-object.h>
#include <ytstenut/yts-contact.h>

G_BEGIN_DECLS

#define YTS_TYPE_ROSTER (yts_roster_get_type())

#define YTS_ROSTER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_ROSTER, YtsRoster))

#define YTS_ROSTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_ROSTER, YtsRosterClass))

#define YTS_IS_ROSTER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_ROSTER))

#define YTS_IS_ROSTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_ROSTER))

#define YTS_ROSTER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_ROSTER, YtsRosterClass))

typedef struct {

  /*< private >*/
  GObject parent;

} YtsRoster;

typedef struct {

  /*< private >*/
  GObjectClass parent_class;

} YtsRosterClass;

GType
yts_roster_get_type (void) G_GNUC_CONST;

GHashTable *const
yts_roster_get_contacts (YtsRoster const *self);

YtsContact *const
yts_roster_find_contact_by_jid (YtsRoster const *self,
                                char const      *jid);

G_END_DECLS

#endif /* YTS_ROSTER_H */

