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
#include <telepathy-glib/contact.h>

#include <ytstenut/yts-error.h>
#include <ytstenut/yts-service.h>

G_BEGIN_DECLS

#define YTS_TYPE_CONTACT (yts_contact_get_type())

#define YTS_CONTACT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_CONTACT, YtsContact))

#define YTS_CONTACT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_CONTACT, YtsContactClass))

#define YTS_IS_CONTACT(obj) \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_CONTACT))

#define YTS_IS_CONTACT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_CONTACT))

#define YTS_CONTACT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_CONTACT, YtsContactClass))

typedef struct {

  /*< private >*/
  GObject parent;

} YtsContact;

/**
 * YtsContactClass:
 * @service_added: virtual function for the #YtsContact::service-added signal.
 * @service_removed: virtual function for the #YtsContact::service-removed signal.
 *
 * Deprecated: the class handlers for signals are deprecated and will be
 *             removed in 0.4.
 */
typedef struct {

  /*< private >*/
  GObjectClass parent;

  /*< public >*/
  void
  (*service_added) (YtsContact *self,
                    YtsService *service);

  void
  (*service_removed) (YtsContact *self,
                      YtsService *service);

} YtsContactClass;

GType
yts_contact_get_type (void) G_GNUC_CONST;

char const *
yts_contact_get_id (YtsContact const *self);

char const *
yts_contact_get_name (YtsContact const *self);

TpContact *const
yts_contact_get_tp_contact (YtsContact const  *self);

GFile *
yts_contact_get_icon (YtsContact const  *self,
                      char const        **mime);

YtsError
yts_contact_send_file (YtsContact *self,
                       GFile      *file);

bool
yts_contact_cancel_file (YtsContact *self,
                         GFile      *file);

G_END_DECLS

#endif /* YTS_CONTACT_H */

