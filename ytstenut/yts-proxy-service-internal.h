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

#ifndef YTS_PROXY_SERVICE_INTERNAL_H
#define YTS_PROXY_SERVICE_INTERNAL_H

#include <ytstenut/yts-proxy-service.h>
#include <ytstenut/yts-service-internal.h>

#define YTS_PROXY_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_PROXY_SERVICE, YtsProxyServiceClass))

#define YTS_IS_PROXY_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_PROXY_SERVICE))

#define YTS_PROXY_SERVICE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_PROXY_SERVICE, YtsProxyServiceClass))

struct YtsProxyService {
  YtsService parent;
};

typedef struct {
  YtsServiceClass parent;
} YtsProxyServiceClass;

bool
yts_proxy_service_dispatch_event (YtsProxyService *self,
                                  char const      *capability,
                                  char const      *aspect,
                                  GVariant        *arguments);

bool
yts_proxy_service_dispatch_response (YtsProxyService  *self,
                                     char const       *capability,
                                     char const       *invocation_id,
                                     GVariant         *response);

#endif /* YTS_PROXY_SERVICE_INTERNAL_H */

