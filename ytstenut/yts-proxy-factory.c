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

#include "yts-proxy-factory.h"

G_DEFINE_TYPE (YtsProxyFactory, yts_proxy_factory, YTS_TYPE_FACTORY)

static GObject *
_constructor (GType                  type,
              unsigned               n_properties,
              GObjectConstructParam *properties)
{
  static GObject *_self = NULL;

  if (NULL == _self) {
    _self = G_OBJECT_CLASS (yts_proxy_factory_parent_class)
                ->constructor (type, n_properties, properties);
  }

  return _self;
}

static void
_dispose (GObject *object)
{
  G_OBJECT_CLASS (yts_proxy_factory_parent_class)->dispose (object);
}

static void
yts_proxy_factory_class_init (YtsProxyFactoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = _constructor;
  object_class->dispose = _dispose;
}

static void
yts_proxy_factory_init (YtsProxyFactory *self)
{
}

YtsProxyFactory * const
yts_proxy_factory_new (void)
{
  return g_object_new (YTS_TYPE_PROXY_FACTORY, NULL);
}

YtsProxyFactory * const
yts_proxy_factory_get_default (void)
{
  return yts_proxy_factory_new ();
}

YtsProxy *
yts_proxy_factory_create_proxy (YtsProxyFactory *self,
                                char const      *fqc_id)
{
  GType proxy_gtype;

  proxy_gtype = yts_factory_get_proxy_gtype_for_fqc_id (YTS_FACTORY (self),
                                                        fqc_id);
  g_return_val_if_fail (proxy_gtype != G_TYPE_INVALID, NULL);

  return g_object_new (proxy_gtype, NULL);
}

