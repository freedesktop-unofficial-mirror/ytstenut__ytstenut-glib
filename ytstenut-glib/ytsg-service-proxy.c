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

#include "ytsg-marshal.h"
#include "ytsg-service-proxy.h"

G_DEFINE_INTERFACE (YtsgServiceProxy, ytsg_service_proxy, G_TYPE_OBJECT)

static void
_receive (YtsgServiceProxy  *self,
          YtsgMetadata      *datagram)
{
  g_critical ("%s : Method YtsgServiceProxy.receive() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
}

static void
ytsg_service_proxy_default_init (YtsgServiceProxyInterface *interface)
{
  interface->receive = _receive;
}

void
ytsg_service_proxy_receive (YtsgServiceProxy  *self,
                            YtsgMetadata      *datagram)
{
  g_return_if_fail (YTSG_IS_SERVICE_PROXY (self));

  return YTSG_SERVICE_PROXY_GET_INTERFACE (self)->receive (self, datagram);
}

