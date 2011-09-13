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

#include "yts-adapter-factory.h"
#include "config.h"

G_DEFINE_TYPE (YtsAdapterFactory, yts_adapter_factory, YTS_TYPE_FACTORY)

static GObject *
_constructor (GType                  type,
              unsigned               n_properties,
              GObjectConstructParam *properties)
{
  static GObject *_self = NULL;

  if (NULL == _self) {
    _self = G_OBJECT_CLASS (yts_adapter_factory_parent_class)
                ->constructor (type, n_properties, properties);
  }

  return _self;
}

static void
_dispose (GObject *object)
{
  G_OBJECT_CLASS (yts_adapter_factory_parent_class)->dispose (object);
}

static void
yts_adapter_factory_class_init (YtsAdapterFactoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = _constructor;
  object_class->dispose = _dispose;
}

static void
yts_adapter_factory_init (YtsAdapterFactory *self)
{
}

YtsAdapterFactory * const
yts_adapter_factory_new (void)
{
  return g_object_new (YTS_TYPE_ADAPTER_FACTORY, NULL);
}

YtsAdapterFactory * const
yts_adapter_factory_get_default (void)
{
  return yts_adapter_factory_new ();
}

YtsServiceAdapter *
yts_adapter_factory_create_adapter_for_service (YtsAdapterFactory *self,
                                                YtsCapability     *service,
                                                char const        *fqc_id)
{
  GType adapter_gtype;
  bool  has_fqc_id;

  has_fqc_id = yts_capability_has_fqc_id (service, fqc_id);
  g_return_val_if_fail (has_fqc_id, NULL);

  adapter_gtype = yts_factory_get_adapter_gtype_for_fqc_id (YTS_FACTORY (self),
                                                            fqc_id);
  g_return_val_if_fail (adapter_gtype != G_TYPE_INVALID, NULL);

  return g_object_new (adapter_gtype,
                       "service", service,
                       NULL);
}

