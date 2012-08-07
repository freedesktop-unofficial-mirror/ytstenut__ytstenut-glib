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

#include "yts-capability.h"
#include "yts-client-internal.h"
#include "yts-profile.h"
#include "yts-profile-impl.h"
#include "yts-response-message.h"

static void
_capability_interface_init (YtsCapability *interface);

static void
_profile_interface_init (YtsProfileInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsProfileImpl,
                         yts_profile_impl,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (YTS_TYPE_CAPABILITY,
                                                _capability_interface_init)
                         G_IMPLEMENT_INTERFACE (YTS_TYPE_PROFILE,
                                                _profile_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_PROFILE_IMPL, YtsProfileImplPrivate))

typedef struct {
  GPtrArray   *capabilities;
  YtsClient  *client;        /* free pointer */
} YtsProfileImplPrivate;

/*
 * YtsCapability implementation
 */

static void
_capability_interface_init (YtsCapability *interface)
{
  /* Nothing to do, it's just about overriding the "fqc-id" property */
}

/*
 * YtsProfile
 */

static void
_register_proxy (YtsProfile  *self,
                 char const   *invocation_id,
                 char const   *capability)
{
  YtsProfileImplPrivate *priv = GET_PRIVATE (self);
  YtsContact   *contact;
  char const    *proxy_id;
  bool           found;
  unsigned       i;
  GVariant      *return_value = NULL;
  YtsMetadata  *message;

  found = false;
  for (i = 0; i < priv->capabilities->len; i++) {
    if (0 == g_strcmp0 (priv->capabilities->pdata[i], capability)) {
      found = true;
      break;
    }
  }

  if (found) {

    bool have_proxy = yts_client_get_invocation_proxy (priv->client,
                                                        invocation_id,
                                                        &contact,
                                                        &proxy_id);
    if (have_proxy) {

      return_value = yts_client_register_proxy (priv->client,
                                                 capability,
                                                 contact,
                                                 proxy_id);

      if (NULL == return_value) {
        g_critical ("%s : Failed to register proxy %s:%s for %s",
                    G_STRLOC,
                    yts_contact_get_id (contact),
                    proxy_id,
                    capability);
        return_value = g_variant_new_boolean (false);
      }
    } else {
      g_critical ("%s : Failed to get proxy info for %s",
                  G_STRLOC,
                  capability);
      return_value = g_variant_new_boolean (false);
    }

  } else {
    g_critical ("%s : Capability %s not available in profile",
                G_STRLOC,
                capability);
    return_value = g_variant_new_boolean (false);
  }

  /* This is one big HACK. The request was made to org.freedesktop.Ytstenut
   * but the response goes to the actual capability that was registered,
   * so it ends up in the right place. */
  message = yts_response_message_new (capability,
                                       invocation_id,
                                       return_value);
  yts_client_send_message (priv->client, contact, proxy_id, message);
  g_object_unref (message);
  g_variant_unref (return_value);
}

static void
_unregister_proxy (YtsProfile  *self,
                   char const   *invocation_id,
                   char const   *capability)
{
  YtsProfileImplPrivate *priv = GET_PRIVATE (self);
  YtsContact *contact;
  char const  *proxy_id;
  bool         found;
  bool         ret;
  unsigned     i;

  found = false;
  for (i = 0; i < priv->capabilities->len; i++) {
    if (0 == g_strcmp0 (priv->capabilities->pdata[i], capability)) {
      found = true;
      break;
    }
  }

  if (!found) {
    g_critical ("%s : Capability %s not available in profile",
                G_STRLOC,
                capability);
    yts_profile_unregister_proxy_return (self, invocation_id, false);
    return;
  }

  ret = yts_client_get_invocation_proxy (priv->client,
                                          invocation_id,
                                          &contact,
                                          &proxy_id);
  if (!ret) {
    g_critical ("%s : Failed to get proxy info for %s",
                G_STRLOC,
                capability);
    yts_profile_unregister_proxy_return (self, invocation_id, false);
    return;
  }

  ret = yts_client_unregister_proxy (priv->client,
                                      capability,
                                      proxy_id);
  if (!ret) {
    g_critical ("%s : Failed to unregister proxy %s:%s for %s",
                G_STRLOC,
                yts_contact_get_id (contact),
                proxy_id,
                capability);
    yts_profile_unregister_proxy_return (self, invocation_id, false);
    return;
  }

  yts_profile_unregister_proxy_return (self, invocation_id, true);
}

static void
_profile_interface_init (YtsProfileInterface *interface)
{
  interface->register_proxy = _register_proxy;
  interface->unregister_proxy = _unregister_proxy;
}

/*
 * YtsProfileImpl
 */

enum {
  PROP_0 = 0,

  PROP_CAPABILITY_FQC_IDS,

  PROP_PROFILE_CAPABILITIES,

  PROP_CLIENT
};

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  YtsProfileImplPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case   PROP_CAPABILITY_FQC_IDS: {
      char const *fqc_ids[] = { YTS_PROFILE_FQC_ID, NULL };
      g_value_set_boxed (value, fqc_ids);
    } break;
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
  YtsProfileImplPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CLIENT:
      /* Construct-only */
      priv->client = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsProfileImplPrivate *priv = GET_PRIVATE (object);

  if (priv->capabilities) {
    g_ptr_array_free (priv->capabilities, TRUE);
    priv->capabilities = NULL;
  }

  G_OBJECT_CLASS (yts_profile_impl_parent_class)->dispose (object);
}

static void
yts_profile_impl_class_init (YtsProfileImplClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (YtsProfileImplPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  /* YtsCapability */

  g_object_class_override_property (object_class,
                                    PROP_CAPABILITY_FQC_IDS,
                                    "fqc-ids");

  /* YtsProfile interface */

  g_object_class_override_property (object_class,
                                    PROP_PROFILE_CAPABILITIES,
                                    "capabilities");

  /* Properties */

  pspec = g_param_spec_object ("client", "", "",
                               YTS_TYPE_CLIENT,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
                                   PROP_CLIENT,
                                   pspec);
}

static void
yts_profile_impl_init (YtsProfileImpl *self)
{
  YtsProfileImplPrivate *priv = GET_PRIVATE (self);

  priv->capabilities = g_ptr_array_new_with_free_func (g_free);
}

YtsProfileImpl *
yts_profile_impl_new (YtsClient   *client)
{
  return g_object_new (YTS_TYPE_PROFILE_IMPL,
                       "client",        client,
                       NULL);
}

bool
yts_profile_impl_add_capability (YtsProfileImpl *self,
                                  char const      *capability)
{
  YtsProfileImplPrivate *priv = GET_PRIVATE (self);
  unsigned int i;

  g_return_val_if_fail (YTS_IS_PROFILE_IMPL (self), false);

  for (i = 0; i < priv->capabilities->len; i++) {
    if (0 == g_strcmp0 (capability, priv->capabilities->pdata[i])) {
      return false;
    }
  }

  g_ptr_array_add (priv->capabilities, g_strdup (capability));

  return true;
}

bool
yts_profile_impl_remove_capability (YtsProfileImpl  *self,
                                     char const       *capability)
{
  YtsProfileImplPrivate *priv = GET_PRIVATE (self);
  unsigned int i;

  g_return_val_if_fail (YTS_IS_PROFILE_IMPL (self), false);

  for (i = 0; i < priv->capabilities->len; i++) {
    if (0 == g_strcmp0 (capability, priv->capabilities->pdata[i])) {
      g_ptr_array_remove_index (priv->capabilities, i);
      return true;
    }
  }

  return false;
}

