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

#ifndef YTS_ADAPTER_FACTORY_H
#define YTS_ADAPTER_FACTORY_H

#include <glib-object.h>
#include <ytstenut/yts-factory.h>
#include <ytstenut/yts-service-adapter.h>

G_BEGIN_DECLS

#define YTS_TYPE_ADAPTER_FACTORY (yts_adapter_factory_get_type ())

#define YTS_ADAPTER_FACTORY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_ADAPTER_FACTORY, YtsAdapterFactory))

#define YTS_ADAPTER_FACTORY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_ADAPTER_FACTORY, YtsAdapterFactoryClass))

#define YTS_IS_ADAPTER_FACTORY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_ADAPTER_FACTORY))

#define YTS_IS_ADAPTER_FACTORY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_ADAPTER_FACTORY))

#define YTS_ADAPTER_FACTORY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_ADAPTER_FACTORY, YtsAdapterFactoryClass))

typedef struct {
  YtsFactory parent;
} YtsAdapterFactory;

typedef struct {
  YtsFactoryClass parent;
} YtsAdapterFactoryClass;

GType
yts_adapter_factory_get_type (void) G_GNUC_CONST;

YtsAdapterFactory * const
yts_adapter_factory_new (void);

YtsAdapterFactory * const
yts_adapter_factory_get_default (void);

YtsServiceAdapter *
yts_adapter_factory_create_adapter_for_service (YtsAdapterFactory *self,
                                                YtsCapability     *service,
                                                char const        *fqc_id);

G_END_DECLS

#endif /* YTS_ADAPTER_FACTORY_H */

