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

#include "yts-capability.h"
#include "yts-marshal.h"
#include "yts-service-adapter.h"

G_DEFINE_TYPE (YtsServiceAdapter, yts_service_adapter, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_FQC_ID,
  PROP_SERVICE
};

enum {
  ERROR_SIGNAL,
  EVENT_SIGNAL,
  RESPONSE_SIGNAL,

  N_SIGNALS
};

static unsigned int _signals[N_SIGNALS] = { 0, };

static GVariant *
_collect_properties (YtsServiceAdapter *self)
{
  g_critical ("%s : Method YtsServiceAdapter.collect_properties() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));

  return NULL;
}

static bool
_invoke (YtsServiceAdapter *self,
         char const         *invocation_id,
         char const         *aspect,
         GVariant           *arguments)
{
  g_critical ("%s : Method YtsServiceAdapter.invoke() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));

  return false;
}

static void
_constructed (GObject *object)
{
  YtsCapability   *service;
  char            **fqc_ids;
  char             *fqc_id;
  unsigned          i;
  bool              found = false;

  fqc_id = yts_service_adapter_get_fqc_id (YTS_SERVICE_ADAPTER (object));
  g_return_if_fail (fqc_id);

  service = yts_service_adapter_get_service (YTS_SERVICE_ADAPTER (object));
  g_return_if_fail (service);

  fqc_ids = yts_capability_get_fqc_ids (service);
  g_return_if_fail (fqc_ids);

  for (i = 0; fqc_ids[i] != NULL; i++) {
    if (0 == g_strcmp0 (fqc_id, fqc_ids[i])) {
      found = true;
    }
  }

  if (!found) {
    g_critical ("%s : Service %s does not match the %s capability %s",
                G_STRLOC,
                G_OBJECT_TYPE_NAME (service),
                G_OBJECT_TYPE_NAME (object),
                fqc_id);
  }

  g_free (fqc_id);
  g_strfreev (fqc_ids);
  g_object_unref (service);
}

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  switch (property_id) {
    case PROP_FQC_ID:
      g_value_take_string (value,
                           yts_service_adapter_get_fqc_id (
                              YTS_SERVICE_ADAPTER (object)));
      break;
    /* Other properties need to be implemented by the subclass. */
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

/* Stub, the properties need to be implemented by the subclass. */
static void
_set_property (GObject      *object,
               unsigned int  property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
yts_service_adapter_class_init (YtsServiceAdapterClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  GParamSpec    *pspec;

  object_class->constructed = _constructed;
  object_class->get_property = _get_property;
  object_class->set_property = _set_property;

  klass->collect_properties = _collect_properties;
  klass->invoke = _invoke;

  /* Properties */

  pspec = g_param_spec_string ("fqc-id", "", "",
                               NULL,
                               G_PARAM_READABLE |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_FQC_ID, pspec);

  pspec = g_param_spec_object ("service", "", "",
                               G_TYPE_OBJECT,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_SERVICE, pspec);

  /* Signals */

  _signals[ERROR_SIGNAL] = g_signal_new ("error",
                                         YTS_TYPE_SERVICE_ADAPTER,
                                         G_SIGNAL_RUN_LAST,
                                         G_STRUCT_OFFSET (YtsServiceAdapterClass,
                                                          error),
                                         NULL, NULL,
                                         yts_marshal_VOID__STRING_BOXED,
                                         G_TYPE_NONE, 2,
                                         G_TYPE_STRING, G_TYPE_ERROR);

  _signals[EVENT_SIGNAL] = g_signal_new ("event",
                                         YTS_TYPE_SERVICE_ADAPTER,
                                         G_SIGNAL_RUN_LAST,
                                         G_STRUCT_OFFSET (YtsServiceAdapterClass,
                                                          event),
                                         NULL, NULL,
                                         yts_marshal_VOID__STRING_BOXED,
                                         G_TYPE_NONE, 2,
                                         G_TYPE_STRING, G_TYPE_VARIANT);

  _signals[RESPONSE_SIGNAL] = g_signal_new ("response",
                                            YTS_TYPE_SERVICE_ADAPTER,
                                            G_SIGNAL_RUN_LAST,
                                            G_STRUCT_OFFSET (YtsServiceAdapterClass,
                                                             response),
                                            NULL, NULL,
                                            yts_marshal_VOID__STRING_BOXED,
                                            G_TYPE_NONE, 2,
                                            G_TYPE_STRING, G_TYPE_VARIANT);
}

static void
yts_service_adapter_init (YtsServiceAdapter *self)
{
}

char *
yts_service_adapter_get_fqc_id (YtsServiceAdapter *self)
{
  char *fqc_id;

  g_return_val_if_fail (YTS_IS_SERVICE_ADAPTER (self), NULL);

  /* Get from subclass. */
  fqc_id = NULL;
  g_object_get (self, "fqc-id", &fqc_id, NULL);

  return fqc_id;
}

YtsCapability *
yts_service_adapter_get_service (YtsServiceAdapter *self)
{
  YtsCapability *service;

  g_return_val_if_fail (YTS_IS_SERVICE_ADAPTER (self), NULL);

  service = NULL;
  g_object_get (G_OBJECT (self), "service", &service, NULL);

  return service;
}

GVariant *
yts_service_adapter_collect_properties (YtsServiceAdapter *self)
{
  g_return_val_if_fail (YTS_IS_SERVICE_ADAPTER (self), NULL);

  return YTS_SERVICE_ADAPTER_GET_CLASS (self)->collect_properties (self);
}

bool
yts_service_adapter_invoke (YtsServiceAdapter *self,
                             char const         *invocation_id,
                             char const         *aspect,
                             GVariant           *arguments)
{
  bool keep_sae;

  g_return_val_if_fail (YTS_IS_SERVICE_ADAPTER (self), false);

  keep_sae = YTS_SERVICE_ADAPTER_GET_CLASS (self)->invoke (self,
                                                            invocation_id,
                                                            aspect,
                                                            arguments);

  /* This is a bit hackish, ok, but it allows for creating the variant
   * in the invocation of this function. */
  if (arguments &&
      g_variant_is_floating (arguments)) {
    g_variant_unref (arguments);
  }

  return keep_sae;
}

void
yts_service_adapter_send_error (YtsServiceAdapter *self,
                                 char const         *invocation_id,
                                 GError const       *error)
{
  g_return_if_fail (YTS_IS_SERVICE_ADAPTER (self));

  g_signal_emit (self, _signals[ERROR_SIGNAL], 0,
                 invocation_id, error);
}

void
yts_service_adapter_send_event (YtsServiceAdapter *self,
                                 char const         *aspect,
                                 GVariant           *arguments)
{
  g_return_if_fail (YTS_IS_SERVICE_ADAPTER (self));

  /* This is a bit hackish, ok, but it allows for creating the variant
   * in the invocation of this function. */
  g_signal_emit (self, _signals[EVENT_SIGNAL], 0,
                 aspect, arguments);

  if (arguments &&
      g_variant_is_floating (arguments)) {
    g_variant_unref (arguments);
  }
}

void
yts_service_adapter_send_response (YtsServiceAdapter  *self,
                                    char const          *invocation_id,
                                    GVariant            *return_value)
{
  g_return_if_fail (YTS_IS_SERVICE_ADAPTER (self));

  /* This is a bit hackish, ok, but it allows for creating the variant
   * in the invocation of this function. */
  g_signal_emit (self, _signals[RESPONSE_SIGNAL], 0,
                 invocation_id, return_value);

  if (return_value &&
      g_variant_is_floating (return_value)) {
    g_variant_unref (return_value);
  }
}

