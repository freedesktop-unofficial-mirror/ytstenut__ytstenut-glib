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

#include "ytsg-capability.h"
#include "ytsg-profile.h"
#include "ytsg-profile-proxy.h"

static void
_capability_interface_init (YtsgCapability *interface);

static void
_profile_interface_init (YtsgProfileInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsgProfileProxy,
                         ytsg_profile_proxy,
                         YTSG_TYPE_PROXY,
                         G_IMPLEMENT_INTERFACE (YTSG_TYPE_CAPABILITY,
                                                _capability_interface_init)
                         G_IMPLEMENT_INTERFACE (YTSG_TYPE_PROFILE,
                                                _profile_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_PROFILE_PROXY, YtsgProfileProxyPrivate))

typedef struct {

  /* Properties */
  // FIXME hook this property up
  GPtrArray *capabilities;

  /* Data */
  GHashTable  *invocations;

} YtsgProfileProxyPrivate;

/*
 * YtsgCapability implementation
 */

static void
_capability_interface_init (YtsgCapability *interface)
{
  /* Nothing to do, it's just about overriding the "fqc-id" property */
}

/*
 * YtsgProfile implementation
 */

static void
_register_proxy (YtsgProfile  *self,
                 char const   *invocation_id_,
                 char const   *capability)
{
  YtsgProfileProxyPrivate *priv = GET_PRIVATE (self);
  char *invocation_id;

  invocation_id = invocation_id_ ?
                    g_strdup (invocation_id_) :
                    ytsg_proxy_create_invocation_id (YTSG_PROXY (self));
  /* Hash takes invocation_id. */
  g_hash_table_insert (priv->invocations,
                       invocation_id, _register_proxy);
  // TODO set timeout, well, probably in ytsg-proxy-service.c

  ytsg_proxy_invoke (YTSG_PROXY (self), invocation_id, "register-proxy",
                     g_variant_new_string (capability));
}

static void
_unregister_proxy (YtsgProfile  *self,
                   char const   *invocation_id_,
                   char const   *capability)
{
  YtsgProfileProxyPrivate *priv = GET_PRIVATE (self);
  char *invocation_id;

  invocation_id = invocation_id_ ?
                    g_strdup (invocation_id_) :
                    ytsg_proxy_create_invocation_id (YTSG_PROXY (self));
  /* Hash takes invocation_id. */
  g_hash_table_insert (priv->invocations,
                       invocation_id, _unregister_proxy);
  // TODO set timeout, well, probably in ytsg-proxy-service.c

  ytsg_proxy_invoke (YTSG_PROXY (self), invocation_id, "unregister-proxy",
                     g_variant_new_string (capability));
}

static void
_profile_interface_init (YtsgProfileInterface *interface)
{
  interface->register_proxy = _register_proxy;
  interface->unregister_proxy = _unregister_proxy;
}

/*
 * YtsgProxy overrides
 */

static void
_proxy_service_event (YtsgProxy   *self,
                      char const  *aspect,
                      GVariant    *arguments)
{
  g_warning ("%s : Received unhandled event '%s'",
             G_STRLOC,
             aspect);
}

static void
_proxy_service_response (YtsgProxy  *self,
                         char const *invocation_id,
                         GVariant   *response)
{
  YtsgProfileProxyPrivate *priv = GET_PRIVATE (self);
  void *call;

  call = g_hash_table_lookup (priv->invocations, invocation_id);
  // TODO clear timeout, well, probably in ytsg-proxy-service.c

  if (call == _register_proxy) {

    ytsg_profile_register_proxy_return (YTSG_PROFILE (self),
                                        invocation_id,
                                        response);

  } else if (call == _unregister_proxy) {

    ytsg_profile_unregister_proxy_return (YTSG_PROFILE (self),
                                          invocation_id,
                                          g_variant_get_boolean (response));

  } else if (call == NULL) {

    // TODO emit general response
    g_critical ("%s : Response not found for invocation %s",
                G_STRLOC,
                invocation_id);

  } else {

    g_critical ("%s : Unknown call %p for invocation %s",
                G_STRLOC,
                call,
                invocation_id);
  }

  if (call) {
    g_hash_table_remove (priv->invocations, invocation_id);
  }
}

/*
 * YtsgProfileProxy
 */

enum {
  PROP_0 = 0,

  PROP_CAPABILITY_FQC_ID,

  PROP_PROFILE_CAPABILITIES
};

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  YtsgProfileProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CAPABILITY_FQC_ID:
      g_value_set_string (value, YTSG_PROFILE_CAPABILITY);
      break;
    case PROP_PROFILE_CAPABILITIES:
      g_value_set_boxed (value, priv->capabilities);
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
  // YtsgProfileProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsgProfileProxyPrivate *priv = GET_PRIVATE (object);

  if (priv->invocations) {
    g_hash_table_destroy (priv->invocations);
    priv->invocations = NULL;
  }

  G_OBJECT_CLASS (ytsg_profile_proxy_parent_class)->dispose (object);
}

static void
ytsg_profile_proxy_class_init (YtsgProfileProxyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  YtsgProxyClass  *proxy_class = YTSG_PROXY_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgProfileProxyPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  proxy_class->service_event = _proxy_service_event;
  proxy_class->service_response = _proxy_service_response;

  /* YtsgCapability */

  g_object_class_override_property (object_class,
                                    PROP_CAPABILITY_FQC_ID,
                                    "fqc-id");

  /* YtsgProfile */

  g_object_class_override_property (object_class,
                                    PROP_PROFILE_CAPABILITIES,
                                    "capabilities");
}

static void
ytsg_profile_proxy_init (YtsgProfileProxy *self)
{
  YtsgProfileProxyPrivate *priv = GET_PRIVATE (self);

  priv->invocations = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             NULL);
}

