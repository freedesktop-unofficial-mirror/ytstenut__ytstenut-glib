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

#include <stdbool.h>
#include "ytsg-invocation-message.h"
#include "ytsg-marshal.h"
#include "ytsg-private.h"
#include "ytsg-proxy-service.h"

#include "profile/ytsg-profile.h"
#include "profile/ytsg-profile-proxy.h"

G_DEFINE_TYPE (YtsgProxyService, ytsg_proxy_service, YTSG_TYPE_SERVICE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_PROXY_SERVICE, YtsgProxyServicePrivate))

enum {
  SIG_PROXY_CREATED,
  N_SIGNALS
};

typedef struct {
  YtsgProfile *profile;
  GHashTable  *invocations;
  GHashTable  *proxies;
} YtsgProxyServicePrivate;

static unsigned _signals[N_SIGNALS] = { 0, };

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  // YtsgProxyServicePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
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
  // YtsgProxyServicePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsgProxyServicePrivate *priv = GET_PRIVATE (object);

  if (priv->proxies) {

    GHashTableIter iter;
    char const *capability;
    YtsgProxy *proxy;

    g_hash_table_iter_init (&iter, priv->proxies);
    while (g_hash_table_iter_next (&iter,
                                   (void **) &capability,
                                   (void **) &proxy)) {

      char *invocation_id = ytsg_proxy_create_invocation_id (YTSG_PROXY (proxy));
      ytsg_profile_unregister_proxy (priv->profile,
                                     invocation_id,
                                     capability);
      g_free (invocation_id);
    }

    g_hash_table_unref (priv->proxies);
    priv->proxies = NULL;
  }

  if (priv->profile) {
    g_object_unref (priv->profile);
    priv->profile = NULL;
  }

  if (priv->invocations) {
    g_hash_table_destroy (priv->invocations);
    priv->invocations = NULL;
  }

  G_OBJECT_CLASS (ytsg_proxy_service_parent_class)->dispose (object);
}

static void
ytsg_proxy_service_class_init (YtsgProxyServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgProxyServicePrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  /* Signals */

  _signals[SIG_PROXY_CREATED] = g_signal_new ("proxy-created",
                                              YTSG_TYPE_PROXY_SERVICE,
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
                                              ytsg_marshal_VOID__OBJECT,
                                              G_TYPE_NONE, 1,
                                              YTSG_TYPE_PROXY);
}

static void
ytsg_proxy_service_init (YtsgProxyService *self)
{
  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);

  priv->proxies = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);

  priv->invocations = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             g_object_unref);
}

YtsgService *
ytsg_proxy_service_new (YtsgContact  *contact,
                        char const   *service_uid,
                        char const   *type,
                        char const  **capabilities,
                        GHashTable   *names)
{
  return g_object_new (YTSG_TYPE_PROXY_SERVICE,
                       "contact", contact,
                       "uid",     service_uid,
                       "type",    type,
                       "caps",    capabilities,
                       "names",   names,
                       NULL);
}

static void
_profile_invoke_service (YtsgProfile      *profile,
                         char const       *invocation_id,
                         char const       *aspect,
                         GVariant         *arguments,
                         YtsgProxyService *self)
{
  YtsgContact     *contact;
  YtsgClient      *client;
  YtsgMetadata    *message;
  char const      *uid;
  GParamSpec      *pspec;
  char const      *capability;

  contact = ytsg_service_get_contact (YTSG_SERVICE (self));
  client = ytsg_contact_get_client (contact);
  uid = ytsg_service_get_uid (YTSG_SERVICE (self));

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (profile),
                                        "capability");
  if (pspec &&
      G_IS_PARAM_SPEC_STRING (pspec)) {

    capability = G_PARAM_SPEC_STRING (pspec)->default_value;

  } else {

    g_critical ("%s : Could not determine capability", G_STRLOC);
    return;
  }

  message = ytsg_invocation_message_new (invocation_id,
                                         capability,
                                         aspect,
                                         arguments);

  // TODO maybe we should attach the invocation-id to the contact
  // and handle the timeout here, so handling the response is simpler.

  _ytsg_client_send_message (client, contact, uid, message);

  g_object_unref (message);
}

static void
_proxy_invoke_service (YtsgProxy        *proxy,
                       char const       *invocation_id,
                       char const       *aspect,
                       GVariant         *arguments,
                       YtsgProxyService *self)
{
  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);
  YtsgContact     *contact;
  YtsgClient      *client;
  YtsgMetadata    *message;
  char const      *uid;
  GHashTableIter   iter;
  char const      *capability;
  YtsgProxy const *p;

  contact = ytsg_service_get_contact (YTSG_SERVICE (self));
  client = ytsg_contact_get_client (contact);
  uid = ytsg_service_get_uid (YTSG_SERVICE (self));

  /* FIXME not very nice, but a proxy doesn't know its capability, otherwise
   * it conflicts with the capability property of YtsgVSPlayer et al. */
  // TODO get capability from proxy parameter instead.
  g_hash_table_iter_init (&iter, priv->proxies);
  while (g_hash_table_iter_next (&iter, (void **) &capability, (void **) &p)) {
    if (p == proxy) {
      /* This is the capability we're looking for */
      break;
    }
  }

  message = ytsg_invocation_message_new (invocation_id,
                                         capability,
                                         aspect,
                                         arguments);

  // TODO maybe we should attach the invocation-id to the contact
  // and handle the timeout here, so handling the response is simpler.

  _ytsg_client_send_message (client, contact, uid, message);

  g_object_unref (message);
}

static void
_proxy_destroyed (YtsgProxyService  *self,
                  void              *stale_proxy_ptr)
{
  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);
  GHashTableIter   iter;
  char const      *key;
  gpointer         value;

  g_hash_table_iter_init (&iter, priv->proxies);
  while (g_hash_table_iter_next (&iter, (gpointer *) &key, (gpointer *) &value)) {
    if (value == stale_proxy_ptr) {
      g_hash_table_remove (priv->proxies, key);
      break;
    }
  }
}

// FIXME need factory foo

extern GType
ytsg_vp_player_proxy_get_type (void);

// TODO instantiate proxy only on response
//  an move from invocations hash to proxies
bool
ytsg_proxy_service_create_proxy (YtsgProxyService *self,
                                 char const       *capability)
{
  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);
  YtsgProxy *proxy;
  char      *invocation_id;

  struct {
    char const *capability;
    GType       gtype;
  } proxies[] = {
//    { "org.freedesktop.ytstenut.VideoProfile.Content",
//      ytsg_vp_content_proxy_get_type () },
    { "org.freedesktop.ytstenut.VideoProfile.Player",
      ytsg_vp_player_proxy_get_type () },
//    { "org.freedesktop.ytstenut.VideoProfile.Transcript",
//      ytsg_vp_transcript_proxy_get_type () },
//    { "org.freedesktop.ytstenut.VideoProfile.Transfer",
//      ytsg_vp_transfer_proxy_get_type () },
    { NULL }
  };

  unsigned int i;

  g_return_val_if_fail (YTSG_IS_PROXY_SERVICE (self), NULL);

  proxy = NULL;
  for (i = 0; proxies[i].capability != NULL; i++) {
    if (0 == g_strcmp0 (capability, proxies[i].capability)) {
      proxy = g_object_new (proxies[i].gtype, NULL);
      break;
    }
  }

  if (NULL == proxy) {
    // FIXME GError
    return false;
  }

  g_hash_table_insert (priv->proxies,
                       g_strdup (capability),
                       proxy);
  g_signal_connect (proxy, "invoke-service",
                    G_CALLBACK (_proxy_invoke_service), self);
  g_object_weak_ref (G_OBJECT (proxy),
                     (GWeakNotify) _proxy_destroyed,
                     self);

  if (NULL == priv->profile) {
    /* Lazily create the profile proxy. */
    priv->profile = g_object_new (YTSG_TYPE_PROFILE_PROXY, NULL);
    g_signal_connect (priv->profile, "invoke-service",
                      G_CALLBACK (_profile_invoke_service), self);
  }

  invocation_id = ytsg_proxy_create_invocation_id (YTSG_PROXY (priv->profile));

  /* Register new proxy with the service. */
  // TODO timeout
  ytsg_profile_register_proxy (priv->profile,
                               invocation_id,
                               capability);

  g_hash_table_insert (priv->invocations, invocation_id, proxy);

  return true;
}

// todo break here

bool
ytsg_proxy_service_dispatch_event (YtsgProxyService *self,
                                   char const       *capability,
                                   char const       *aspect,
                                   GVariant         *arguments)
{
  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);

  if (0 == g_strcmp0 (YTSG_PROFILE_CAPABILITY, capability)) {

    /* This one comes from the Profile / Meta interface */
    ytsg_proxy_handle_service_event (YTSG_PROXY (priv->profile),
                                     aspect,
                                     arguments);
    return true;

  } else {

    YtsgProxy *proxy = g_hash_table_lookup (priv->proxies, capability);
    if (proxy) {
      ytsg_proxy_handle_service_event (proxy, aspect, arguments);
      return true;
    }
  }

  return false;
}

bool
ytsg_proxy_service_dispatch_response (YtsgProxyService  *self,
                                      char const        *capability,
                                      char const        *invocation_id,
                                      GVariant          *response)
{
  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);
  YtsgProxy *proxy;

  if (priv->invocations &&
      g_hash_table_size (priv->invocations) > 0 &&
      NULL != (proxy = g_hash_table_lookup (priv->invocations, invocation_id))) {

    /* Initial properties for the proxy */
    GVariantIter iter;
    char *name;
    GVariant *value;

    if (!g_variant_is_of_type (response, G_VARIANT_TYPE_DICTIONARY)) {
      g_critical ("%s : Registering proxy for capability %s failed",
                  G_STRLOC,
                  capability);
      return false;
    }

    g_variant_iter_init (&iter, response);
    while (g_variant_iter_next (&iter, "{sv}", &name, &value)) {
      /* Pass the properties to the proxy through the standard mechanism. */
      ytsg_proxy_handle_service_event (proxy, name, value);
      g_free (name);
      g_variant_unref (value);
    }

    g_signal_emit (self, _signals[SIG_PROXY_CREATED], 0,
                   proxy);
    g_hash_table_remove (priv->invocations, invocation_id);
    return true;
  }

  if (0 == g_strcmp0 (YTSG_PROFILE_CAPABILITY, capability)) {

    /* This one comes from the Profile / Meta interface */
    ytsg_proxy_handle_service_response (YTSG_PROXY (priv->profile),
                                        invocation_id,
                                        response);
    return true;

  } else {

    YtsgProxy *proxy = g_hash_table_lookup (priv->proxies, capability);
    if (proxy) {
      ytsg_proxy_handle_service_response (proxy, invocation_id, response);
      return true;
    }
  }

  return false;
}

