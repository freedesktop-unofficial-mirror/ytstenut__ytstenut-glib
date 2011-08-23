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

#include "ytsg-profile.h"
#include "ytsg-profile-adapter.h"

G_DEFINE_TYPE (YtsgProfileAdapter,
               ytsg_profile_adapter,
               YTSG_TYPE_PROFILE_ADAPTER)

#define GET_PRIVATE(o)                                      \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o),                        \
                                YTSG_TYPE_PROFILE_ADAPTER,  \
                                YtsgProfileAdapterPrivate))

typedef struct {
  YtsgProfile *profile;
} YtsgProfileAdapterPrivate;

/*
 * YtsgServiceAdapter overrides
 */

static void
_service_adapter_invoke (YtsgServiceAdapter *self,
                         char const         *invocation_id,
                         char const         *aspect,
                         GVariant           *arguments)
{
  YtsgProfileAdapterPrivate *priv = GET_PRIVATE (self);

  /* Methods */

  if (0 == g_strcmp0 ("register-proxy", aspect) &&
      arguments &&
      g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    char const *capability = g_variant_get_string (arguments, NULL);
    ytsg_profile_register_proxy (priv->profile, capability);

  } else if (0 == g_strcmp0 ("unregister-proxy", aspect) &&
             arguments &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    char const *capability = g_variant_get_string (arguments, NULL);
    ytsg_profile_unregister_proxy (priv->profile, capability);

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
}

/*
 * YtsgProfileAdapter
 */

enum {
  PROP_0 = 0,
  PROP_SERVICE_ADAPTER_SERVICE,
  PROP_SERVICE_ADAPTER_SERVICE_GTYPE
};

static void
_profile_destroyed (YtsgProfileAdapter  *self,
                    void                *stale_profile_ptr)
{
  YtsgProfileAdapterPrivate *priv = GET_PRIVATE (self);

  priv->profile = NULL;
  g_object_unref (self);
}

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  YtsgProfileAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_SERVICE_ADAPTER_SERVICE:
      g_value_set_gtype (value, YTSG_TYPE_PROFILE);
      break;
    case PROP_SERVICE_ADAPTER_SERVICE_GTYPE:
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
  YtsgProfileAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    case PROP_SERVICE_ADAPTER_SERVICE:

      /* Construct-only */

      g_return_if_fail (priv->profile == NULL);
      g_return_if_fail (YTSG_IS_PROFILE (g_value_get_object (value)));

      priv->profile = YTSG_PROFILE (g_value_get_object (value));
      g_object_weak_ref (G_OBJECT (priv->profile),
                         (GWeakNotify) _profile_destroyed,
                         object);

      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsgProfileAdapterPrivate *priv = GET_PRIVATE (object);

  if (priv->profile) {
    g_warning ("%s : Adapter disposed with adaptee still referenced.",
               G_STRLOC);
    g_object_weak_unref (G_OBJECT (priv->profile),
                         (GWeakNotify) _profile_destroyed,
                         object);
    priv->profile = NULL;
  }

  G_OBJECT_CLASS (ytsg_profile_adapter_parent_class)->dispose (object);
}

static void
ytsg_profile_adapter_class_init (YtsgProfileAdapterClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  YtsgServiceAdapterClass *adapter_class = YTSG_SERVICE_ADAPTER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgProfileAdapterPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  adapter_class->invoke = _service_adapter_invoke;

  /* Properties */

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_SERVICE,
                                    "service");

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_SERVICE_GTYPE,
                                    "service-gtype");
}

static void
ytsg_profile_adapter_init (YtsgProfileAdapter *self)
{
}

