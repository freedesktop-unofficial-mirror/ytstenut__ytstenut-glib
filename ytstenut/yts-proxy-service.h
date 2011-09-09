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

#ifndef YTS_PROXY_SERVICE_H
#define YTS_PROXY_SERVICE_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut/yts-service.h>

G_BEGIN_DECLS

#define YTS_TYPE_PROXY_SERVICE yts_proxy_service_get_type()

#define YTS_PROXY_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_PROXY_SERVICE, YtsProxyService))

#define YTS_PROXY_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_PROXY_SERVICE, YtsProxyServiceClass))

#define YTS_IS_PROXY_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_PROXY_SERVICE))

#define YTS_IS_PROXY_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_PROXY_SERVICE))

#define YTS_PROXY_SERVICE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_PROXY_SERVICE, YtsProxyServiceClass))

typedef struct {
  YtsService parent;
} YtsProxyService;

typedef struct {
  YtsServiceClass parent;
} YtsProxyServiceClass;

GType
yts_proxy_service_get_type (void);

bool
yts_proxy_service_create_proxy (YtsProxyService *self,
                                 char const       *capability);

G_END_DECLS

#endif /* YTS_PROXY_SERVICE_H */

