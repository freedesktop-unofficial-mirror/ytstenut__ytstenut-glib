/*
 * Copyright (c) 2011 Intel Corp.
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

#ifndef YTSG_PROXY_SERVICE_H
#define YTSG_PROXY_SERVICE_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut-glib/ytsg-proxy.h>
#include <ytstenut-glib/ytsg-service.h>

G_BEGIN_DECLS

#define YTSG_TYPE_PROXY_SERVICE ytsg_proxy_service_get_type()

#define YTSG_PROXY_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_TYPE_PROXY_SERVICE, YtsgProxyService))

#define YTSG_PROXY_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTSG_TYPE_PROXY_SERVICE, YtsgProxyServiceClass))

#define YTSG_IS_PROXY_SERVICE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_TYPE_PROXY_SERVICE))

#define YTSG_IS_PROXY_SERVICE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTSG_TYPE_PROXY_SERVICE))

#define YTSG_PROXY_SERVICE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTSG_TYPE_PROXY_SERVICE, YtsgProxyServiceClass))

typedef struct {
  YtsgService parent;
} YtsgProxyService;

typedef struct {
  YtsgServiceClass parent;
} YtsgProxyServiceClass;

GType
ytsg_proxy_service_get_type (void);

YtsgService *
ytsg_proxy_service_new (YtsgContact  *contact,
                        char const   *service_uid,
                        char const   *type,
                        char const  **capabilities,
                        GHashTable   *names);

YtsgProxy *
ytsg_proxy_service_create_proxy (YtsgProxyService *self,
                                 char const       *capability);

/* FIXME private */

bool
ytsg_proxy_service_dispatch_event (YtsgProxyService *self,
                                   char const       *capability,
                                   char const       *aspect,
                                   GVariant         *arguments);

bool
ytsg_proxy_service_dispatch_response (YtsgProxyService  *self,
                                      char const        *capability,
                                      char const        *invocation_id,
                                      GVariant          *response);

G_END_DECLS

#endif /* YTSG_PROXY_SERVICE_H */

