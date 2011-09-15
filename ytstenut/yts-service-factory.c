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

#include "yts-metadata-service-internal.h"
#include "yts-proxy-service-internal.h"
#include "yts-service-factory.h"
#include "config.h"

G_DEFINE_TYPE (YtsServiceFactory, yts_service_factory, YTS_TYPE_FACTORY)

static GObject *
_constructor (GType                  type,
              unsigned               n_properties,
              GObjectConstructParam *properties)
{
  static GObject *_self = NULL;

  if (NULL == _self) {
    _self = G_OBJECT_CLASS (yts_service_factory_parent_class)
                ->constructor (type, n_properties, properties);
  }

  return _self;
}

static void
_dispose (GObject *object)
{
  G_OBJECT_CLASS (yts_service_factory_parent_class)->dispose (object);
}

static void
yts_service_factory_class_init (YtsServiceFactoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = _constructor;
  object_class->dispose = _dispose;
}

static void
yts_service_factory_init (YtsServiceFactory *self)
{
}

YtsServiceFactory * const
yts_service_factory_new (void)
{
  return g_object_new (YTS_TYPE_SERVICE_FACTORY, NULL);
}

YtsServiceFactory * const
yts_service_factory_get_default (void)
{
  return yts_service_factory_new ();
}

YtsService *
yts_service_factory_create_service (YtsServiceFactory *self,
                                    char const *const *fqc_ids,
                                    char const        *service_id,
                                    char const        *type,
                                    GHashTable        *names,
                                    GHashTable        *statuses)
{
  unsigned  i;

  g_return_val_if_fail (fqc_ids, NULL);

  /* PONDERING
   * This is a bit of a kludge, if we have a proxy for this FQC then
   * we'll create a proxy-service, otherwise a metadata-service.
   * Will go away when we remove metadata-service. */

  for (i = 0; fqc_ids[i]; i++) {
    GType proxy_gtype = yts_factory_get_proxy_gtype_for_fqc_id (
                            YTS_FACTORY (self),
                            fqc_ids[i]);
    if (proxy_gtype != G_TYPE_INVALID) {
      return yts_proxy_service_new (service_id,
                                    type, 
                                    fqc_ids,
                                    names,
                                    statuses);
    }
  }

  return yts_metadata_service_new (service_id,
                                   type,
                                   fqc_ids,
                                   names,
                                   statuses);
}

