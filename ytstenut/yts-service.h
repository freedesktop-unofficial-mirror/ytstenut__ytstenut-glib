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

#ifndef YTS_SERVICE_H
#define YTS_SERVICE_H

#include <glib-object.h>
#include <ytstenut/yts-contact.h>
#include <ytstenut/yts-types.h>

G_BEGIN_DECLS

#define YTS_TYPE_SERVICE                                               \
   (yts_service_get_type())
#define YTS_SERVICE(obj)                                               \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTS_TYPE_SERVICE,                      \
                                YtsService))
#define YTS_SERVICE_CLASS(klass)                                       \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTS_TYPE_SERVICE,                         \
                             YtsServiceClass))
#define YTS_IS_SERVICE(obj)                                            \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTS_TYPE_SERVICE))
#define YTS_IS_SERVICE_CLASS(klass)                                    \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTS_TYPE_SERVICE))
#define YTS_SERVICE_GET_CLASS(obj)                                     \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTS_TYPE_SERVICE,                       \
                               YtsServiceClass))

typedef struct _YtsServiceClass   YtsServiceClass;
typedef struct _YtsServicePrivate YtsServicePrivate;

/**
 * YtsServiceClass:
 *
 * #YtsService class.
 */
struct _YtsServiceClass
{
  /*< private >*/
  GObjectClass parent_class;

  void (*message) (YtsService  *self,
                   char const   *xml_payload);
};

/**
 * YtsService:
 *
 * Abstract base class for XPMN services; see #YtsMetadataService.
 */
struct _YtsService
{
  /*< private >*/
  GObject parent;

  /*< private >*/
  YtsServicePrivate *priv;
};

GType yts_service_get_type (void) G_GNUC_CONST;

const char  *  yts_service_get_uid     (YtsService *service);
const char  *  yts_service_get_jid     (YtsService *service);
YtsContact *  yts_service_get_contact (YtsService *service);
const char  *  yts_service_get_service_type    (YtsService *service);
const char  ** yts_service_get_caps    (YtsService *service);
GHashTable  *  yts_service_get_names   (YtsService *service);
const char  *  yts_service_get_status_xml (YtsService *service);

gboolean       yts_service_has_capability (YtsService *self,
                                            char const  *capability);

G_END_DECLS

#endif /* YTS_SERVICE_H */
