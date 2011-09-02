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

#ifndef YTS_SERVICE_ADAPTER_H
#define YTS_SERVICE_ADAPTER_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut/yts-capability.h>

G_BEGIN_DECLS

#define YTS_TYPE_SERVICE_ADAPTER yts_service_adapter_get_type()

#define YTS_SERVICE_ADAPTER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_SERVICE_ADAPTER, YtsServiceAdapter))

#define YTS_SERVICE_ADAPTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_SERVICE_ADAPTER, YtsServiceAdapterClass))

#define YTS_IS_SERVICE_ADAPTER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_SERVICE_ADAPTER))

#define YTS_IS_SERVICE_ADAPTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_SERVICE_ADAPTER))

#define YTS_SERVICE_ADAPTER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_SERVICE_ADAPTER, YtsServiceAdapterClass))

typedef struct {
  GObject parent;
} YtsServiceAdapter;

typedef struct {
  GObjectClass parent;

  /* Methods */

  GVariant *
  (*collect_properties) (YtsServiceAdapter *self);

  bool
  (*invoke) (YtsServiceAdapter *self,
             char const         *invocation_id,
             char const         *aspect,
             GVariant           *arguments);

  /* Signals */

  void
  (*error) (YtsServiceAdapter  *self,
            char const          *invocation_id,
            GError const        *error);

  void
  (*event) (YtsServiceAdapter  *self,
            char const          *aspect,
            GVariant            *arguments);

  void
  (*response) (YtsServiceAdapter *self,
               char const         *invocation_id,
               GVariant           *return_value);

} YtsServiceAdapterClass;

GType
yts_service_adapter_get_type (void) G_GNUC_CONST;

char *
yts_service_adapter_get_fqc_id (YtsServiceAdapter *self);

YtsCapability *
yts_service_adapter_get_service (YtsServiceAdapter *self);

GVariant *
yts_service_adapter_collect_properties (YtsServiceAdapter *self);

bool
yts_service_adapter_invoke (YtsServiceAdapter *self,
                             char const         *invocation_id,
                             char const         *aspect,
                             GVariant           *arguments);

void
yts_service_adapter_send_error (YtsServiceAdapter *self,
                                 char const         *invocation_id,
                                 GError const       *error);

void
yts_service_adapter_send_event (YtsServiceAdapter *self,
                                 char const         *aspect,
                                 GVariant           *arguments);

void
yts_service_adapter_send_response (YtsServiceAdapter  *self,
                                    char const          *invocation_id,
                                    GVariant            *return_value);

G_END_DECLS

#endif /* YTS_SERVICE_ADAPTER_H */

