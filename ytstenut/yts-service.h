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
 *              Rob Staudinger <robsta@linux.intel.com>
 */

#ifndef YTS_SERVICE_H
#define YTS_SERVICE_H

#include <stdbool.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define YTS_TYPE_SERVICE  (yts_service_get_type ())

#define YTS_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_SERVICE, YtsService))

#define YTS_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_SERVICE, YtsServiceClass))

#define YTS_IS_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_SERVICE))

#define YTS_IS_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_SERVICE))

#define YTS_SERVICE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_SERVICE, YtsServiceClass))

/**
 * YtsService:
 *
 * Abstract base class for XPMN services; see #YtsMetadataService.
 */
typedef struct {
  /*< private >*/
  GObject parent;
} YtsService;

/**
 * YtsServiceClass:
 *
 * #YtsService class.
 */
typedef struct {
  /*< private >*/
  GObjectClass parent;

  void
  (*message) (YtsService  *self,
              char const  *xml_payload);
} YtsServiceClass;

GType
yts_service_get_type (void) G_GNUC_CONST;

char const *
yts_service_get_uid (YtsService *self);

char const *
yts_service_get_jid (YtsService *self);

char const *
yts_service_get_service_type (YtsService *self);

GHashTable *const
yts_service_get_names (YtsService *self);

char const *
yts_service_get_status_xml (YtsService *self);

G_END_DECLS

#endif /* YTS_SERVICE_H */
