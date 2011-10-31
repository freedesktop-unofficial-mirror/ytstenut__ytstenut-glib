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

#ifndef YTS_SERVICE_IMPL_H
#define YTS_SERVICE_IMPL_H

#include <glib-object.h>
#include <ytstenut/yts-service-internal.h>

G_BEGIN_DECLS

#define YTS_TYPE_SERVICE_IMPL  (yts_service_impl_get_type ())

#define YTS_SERVICE_IMPL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_SERVICE_IMPL, YtsServiceImpl))

#define YTS_SERVICE_IMPL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_SERVICE_IMPL, YtsServiceImplClass))

#define YTS_IS_SERVICE_IMPL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_SERVICE_IMPL))

#define YTS_IS_SERVICE_IMPL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_SERVICE_IMPL))

#define YTS_SERVICE_IMPL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_SERVICE_IMPL, YtsServiceImplClass))

typedef struct {
  YtsService parent;
} YtsServiceImpl;

typedef struct {
  YtsServiceClass parent;
} YtsServiceImplClass;

GType
yts_service_impl_get_type (void) G_GNUC_CONST;

YtsService *
yts_service_impl_new (char const *service_id,
                      char const *type,
                      char const *const *fqc_ids,
                      GHashTable *names,
                      GHashTable *statuses);

G_END_DECLS

#endif /* YTS_SERVICE_IMPL_H */

