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

#include <stdbool.h>

#include "yts-profile.h"
#include "yts-profile-adapter.h"
#include "config.h"

G_DEFINE_TYPE (YtsProfileAdapter,
               yts_profile_adapter,
               YTS_TYPE_SERVICE_ADAPTER)

#define GET_PRIVATE(o)                                      \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o),                        \
                                YTS_TYPE_PROFILE_ADAPTER,  \
                                YtsProfileAdapterPrivate))

typedef struct {
  YtsProfile *profile;
} YtsProfileAdapterPrivate;

/*
 * YtsServiceAdapter overrides
 */

static bool
_service_adapter_invoke (YtsServiceAdapter *self,
                         char const         *invocation_id,
                         char const         *aspect,
                         GVariant           *arguments)
{
  YtsProfileAdapterPrivate *priv = GET_PRIVATE (self);

  /* Methods */

  if (0 == g_strcmp0 ("register-proxy", aspect) &&
      arguments &&
      g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    char const *capability = g_variant_get_string (arguments, NULL);
    yts_profile_register_proxy (priv->profile,
                                 invocation_id,
                                 capability);

  } else if (0 == g_strcmp0 ("unregister-proxy", aspect) &&
             arguments &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    char const *capability = g_variant_get_string (arguments, NULL);
    yts_profile_unregister_proxy (priv->profile,
                                   invocation_id,
                                   capability);

  } else {

    char *arg_string = arguments ?
                          g_variant_print (arguments, false) :
                          g_strdup ("NULL");
    g_warning ("%s : Unknown method %s.%s(%s)",
               G_STRLOC,
               G_OBJECT_TYPE_NAME (priv->profile),
               aspect,
               arg_string);
    g_free (arg_string);
  }

  /* No need to keep the return SAE. */
  return false;
}

/*
 * YtsProfileAdapter
 */

enum {
  PROP_0 = 0,
  PROP_SERVICE_ADAPTER_FQC_ID,
  PROP_SERVICE_ADAPTER_SERVICE
};

static void
_profile_register_proxy_response (YtsProfile         *profile,
                                  char const          *invocation_id,
                                  GVariant            *return_value,
                                  YtsProfileAdapter  *self)
{
  yts_service_adapter_send_response (YTS_SERVICE_ADAPTER (self),
                                      invocation_id,
                                      return_value);
}

static void
_profile_unregister_proxy_response (YtsProfile         *profile,
                                    char const          *invocation_id,
                                    bool                 return_value,
                                    YtsProfileAdapter  *self)
{
  yts_service_adapter_send_response (YTS_SERVICE_ADAPTER (self),
                                      invocation_id,
                                      g_variant_new_boolean (return_value));
}

static void
_profile_destroyed (YtsProfileAdapter  *self,
                    void                *stale_profile_ptr)
{
  YtsProfileAdapterPrivate *priv = GET_PRIVATE (self);

  priv->profile = NULL;
  g_object_unref (self);
}

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  YtsProfileAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_SERVICE_ADAPTER_FQC_ID:
      g_value_set_string (value, YTS_PROFILE_FQC_ID);
      break;
    case PROP_SERVICE_ADAPTER_SERVICE:
      g_value_set_object (value, priv->profile);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_set_property (GObject      *object,
               unsigned int  property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  YtsProfileAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    case PROP_SERVICE_ADAPTER_SERVICE:

      /* Construct-only */

      g_return_if_fail (priv->profile == NULL);
      g_return_if_fail (YTS_IS_PROFILE (g_value_get_object (value)));

      priv->profile = YTS_PROFILE (g_value_get_object (value));
      g_object_weak_ref (G_OBJECT (priv->profile),
                         (GWeakNotify) _profile_destroyed,
                         object);

      g_signal_connect (priv->profile, "register-proxy-response",
                        G_CALLBACK (_profile_register_proxy_response), object);
      g_signal_connect (priv->profile, "unregister-proxy-response",
                        G_CALLBACK (_profile_unregister_proxy_response), object);

      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsProfileAdapterPrivate *priv = GET_PRIVATE (object);

  if (priv->profile) {
    g_warning ("%s : Adapter disposed with adaptee still referenced.",
               G_STRLOC);
    g_object_weak_unref (G_OBJECT (priv->profile),
                         (GWeakNotify) _profile_destroyed,
                         object);
    priv->profile = NULL;
  }

  G_OBJECT_CLASS (yts_profile_adapter_parent_class)->dispose (object);
}

static void
yts_profile_adapter_class_init (YtsProfileAdapterClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  YtsServiceAdapterClass *adapter_class = YTS_SERVICE_ADAPTER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsProfileAdapterPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  adapter_class->invoke = _service_adapter_invoke;

  /* Properties */

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_FQC_ID,
                                    "fqc-id");

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_SERVICE,
                                    "service");
}

static void
yts_profile_adapter_init (YtsProfileAdapter *self)
{
}

