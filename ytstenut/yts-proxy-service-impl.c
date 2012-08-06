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

#include "config.h"

#include "yts-service-emitter.h"
#include "yts-proxy-service-impl.h"

static void
_service_emitter_interface_init (YtsServiceEmitter *interface);

G_DEFINE_TYPE_WITH_CODE (YtsProxyServiceImpl,
                         yts_proxy_service_impl,
                         YTS_TYPE_PROXY_SERVICE,
                         G_IMPLEMENT_INTERFACE (YTS_TYPE_SERVICE_EMITTER,
                                                _service_emitter_interface_init))

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0proxy-service-impl\0"G_STRLOC

/*
 * YtsProxyServiceEmitter
 */

static void
_service_emitter_interface_init (YtsServiceEmitter *interface)
{
  /* Nothing to do, it's just about using the "send-message" signal. */
}

/*
 * YtsProxyServiceImpl
 */

static void
yts_proxy_service_impl_class_init (YtsProxyServiceImplClass *klass)
{
}

static void
yts_proxy_service_impl_init (YtsProxyServiceImpl *self)
{
}

YtsService *
yts_proxy_service_impl_new (char const        *service_id,
                            char const        *type,
                            char const *const *fqc_ids,
                            GHashTable        *names,
                            GHashTable        *statuses)
{
  g_return_val_if_fail (service_id, NULL);
  g_return_val_if_fail (*service_id, NULL);

  return g_object_new (YTS_TYPE_PROXY_SERVICE_IMPL,
                       "fqc-ids", fqc_ids,
                       "id", service_id,
                       "type", type,
                       "names", names,
                       "statuses", statuses,
                       NULL);
}

