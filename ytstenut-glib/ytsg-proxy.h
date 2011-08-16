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

#ifndef YTSG_PROXY_H
#define YTSG_PROXY_H

#include <glib-object.h>
#include <ytstenut-glib/ytsg-proxy-service.h>

G_BEGIN_DECLS

#define YTSG_TYPE_PROXY ytsg_proxy_get_type()

#define YTSG_PROXY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_TYPE_PROXY, YtsgProxy))

#define YTSG_PROXY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTSG_TYPE_PROXY, YtsgProxyClass))

#define YTSG_IS_PROXY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_TYPE_PROXY))

#define YTSG_IS_PROXY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTSG_TYPE_PROXY))

#define YTSG_PROXY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTSG_TYPE_PROXY, YtsgProxyClass))

typedef struct {
  YtsgService parent;
} YtsgProxy;

typedef struct {
  YtsgServiceClass parent;

  /*< private >*/

  /* Signals */

  void
  (*service_response) (YtsgProxy  *self,
                       char const *invocation_id,
                       GVariant   *response);

} YtsgProxyClass;

GType
ytsg_proxy_get_type (void);

YtsgProxy *
ytsg_proxy_new (char const *capability);

void
ytsg_proxy_invoke (YtsgProxy  *self,
                   char const *invocation_id,
                   char const *aspect,
                   GVariant   *arguments);

G_END_DECLS

#endif /* YTSG_PROXY_H */

