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

#ifndef YTSG_SERVICE_PROXY_H
#define YTSG_SERVICE_PROXY_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut-glib/ytsg-metadata.h>

G_BEGIN_DECLS

#define YTSG_TYPE_SERVICE_PROXY \
  (ytsg_service_proxy_get_type ())

#define YTSG_SERVICE_PROXY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_TYPE_SERVICE_PROXY, YtsgServiceProxy))

#define YTSG_IS_SERVICE_PROXY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_TYPE_SERVICE_PROXY))

#define YTSG_SERVICE_PROXY_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTSG_TYPE_SERVICE_PROXY, YtsgServiceProxyInterface))

typedef struct YtsgServiceProxy YtsgServiceProxy;
typedef struct YtsgServiceProxyInterface YtsgServiceProxyInterface;

struct YtsgServiceProxyInterface {

  /*< private >*/
  GTypeInterface parent;

  void
  (*receive) (YtsgServiceProxy  *self,
              YtsgMetadata      *datagram);
};

GType
ytsg_service_proxy_get_type (void) G_GNUC_CONST;

void
ytsg_service_proxy_receive (YtsgServiceProxy  *self,
                            YtsgMetadata      *datagram);

G_END_DECLS

#endif /* YTSG_SERVICE_PROXY_H */

