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

#include <glib-object.h>
#include <gio/gio.h>
#include <telepathy-glib/contact.h>

#include <ytstenut/yts-caps.h>
#include <ytstenut/yts-error.h>
#include <ytstenut/yts-types.h>

G_BEGIN_DECLS

#define YTS_TYPE_CONTACT                                               \
   (yts_contact_get_type())
#define YTS_CONTACT(obj)                                               \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTS_TYPE_CONTACT,                      \
                                YtsContact))
#define YTS_CONTACT_CLASS(klass)                                       \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTS_TYPE_CONTACT,                         \
                             YtsContactClass))
#define YTS_IS_CONTACT(obj)                                            \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTS_TYPE_CONTACT))
#define YTS_IS_CONTACT_CLASS(klass)                                    \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTS_TYPE_CONTACT))
#define YTS_CONTACT_GET_CLASS(obj)                                     \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTS_TYPE_CONTACT,                       \
                               YtsContactClass))

typedef struct _YtsContactClass   YtsContactClass;
typedef struct _YtsContactPrivate YtsContactPrivate;

/**
 * YtsContactClass:
 * @service_added: signal handler for #YtsContact::service-added
 * @service_removed: signal handler for #YtsContact::service-removed
 *
 * #YtsContact class.
 */
struct _YtsContactClass
{
  GObjectClass parent_class;

  void (*service_added)   (YtsContact *contact, YtsService *service);
  void (*service_removed) (YtsContact *contact, YtsService *service);
};

/**
 * YtsContact:
 *
 * Represents a single XMPP connection (usually a device) in the Ytstenut
 * mesh. One or more #YtsService<!-- -->s will be available throug a given
 * contact.
 */
struct _YtsContact
{
  /*< private >*/
  GObject parent;

  /*< private >*/
  YtsContactPrivate *priv;
};

GType yts_contact_get_type (void) G_GNUC_CONST;

YtsClient *yts_contact_get_client         (const YtsContact  *contact);
const char *yts_contact_get_jid            (const YtsContact  *contact);
const char *yts_contact_get_name           (const YtsContact  *contact);
TpContact  *yts_contact_get_tp_contact     (const YtsContact  *contact);
GFile      *yts_contact_get_icon           (const YtsContact  *contact,
                                             const char        **mime);
gboolean    yts_contact_has_capability     (const YtsContact  *item,
                                             YtsCaps            cap);

YtsError   yts_contact_send_file          (const YtsContact *item,
                                             GFile *gfile);
gboolean    yts_contact_cancel_file        (const YtsContact *item,
                                             GFile             *gfile);

G_END_DECLS

#endif /* YTS_CONTACT_H */
