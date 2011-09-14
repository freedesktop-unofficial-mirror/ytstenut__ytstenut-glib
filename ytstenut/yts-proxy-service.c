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

#include "yts-capability.h"
#include "yts-client-internal.h"
#include "yts-invocation-message.h"
#include "yts-marshal.h"
#include "yts-proxy-factory.h"
#include "yts-proxy-internal.h"
#include "yts-proxy-service-internal.h"

#include "profile/yts-profile.h"
#include "profile/yts-profile-proxy.h"
#include "config.h"

G_DEFINE_TYPE (YtsProxyService, yts_proxy_service, YTS_TYPE_SERVICE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_PROXY_SERVICE, YtsProxyServicePrivate))

enum {
  SIG_PROXY_CREATED,
  N_SIGNALS
};

typedef struct {
  YtsProfile *profile;
  GHashTable  *pending_proxies;
  GHashTable  *proxies;
} YtsProxyServicePrivate;

static unsigned _signals[N_SIGNALS] = { 0, };

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  // YtsProxyServicePrivate *priv = GET_PRIVATE (object);

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
  // YtsProxyServicePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsProxyServicePrivate *priv = GET_PRIVATE (object);

  if (priv->proxies) {

    GHashTableIter iter;
    char const *capability;
    YtsProxy *proxy;

    g_hash_table_iter_init (&iter, priv->proxies);
    while (g_hash_table_iter_next (&iter,
                                   (void **) &capability,
                                   (void **) &proxy)) {

      char *invocation_id = yts_proxy_create_invocation_id (YTS_PROXY (proxy));
      yts_profile_unregister_proxy (priv->profile,
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

  if (priv->pending_proxies) {
    g_hash_table_destroy (priv->pending_proxies);
    priv->pending_proxies = NULL;
  }

  G_OBJECT_CLASS (yts_proxy_service_parent_class)->dispose (object);
}

static void
yts_proxy_service_class_init (YtsProxyServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsProxyServicePrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  /* Signals */

  _signals[SIG_PROXY_CREATED] = g_signal_new ("proxy-created",
                                              YTS_TYPE_PROXY_SERVICE,
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
                                              yts_marshal_VOID__OBJECT,
                                              G_TYPE_NONE, 1,
                                              YTS_TYPE_PROXY);
}

static void
yts_proxy_service_init (YtsProxyService *self)
{
  YtsProxyServicePrivate *priv = GET_PRIVATE (self);

  priv->proxies = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);

  priv->pending_proxies = g_hash_table_new_full (g_str_hash,
                                                 g_str_equal,
                                                 g_free,
                                                 g_free);
}

YtsService *
yts_proxy_service_new (YtsContact         *contact,
                        char const        *service_uid,
                        char const        *type,
                        char const *const *fqc_ids,
                        GHashTable        *names)
{
  return g_object_new (YTS_TYPE_PROXY_SERVICE,
                       "fqc-ids", fqc_ids,
                       "contact", contact,
                       "uid",     service_uid,
                       "type",    type,
                       "names",   names,
                       NULL);
}

static void
_profile_invoke_service (YtsProfile      *profile,
                         char const       *invocation_id,
                         char const       *aspect,
                         GVariant         *arguments,
                         YtsProxyService *self)
{
  YtsContact   *contact;
  YtsClient    *client;
  YtsMetadata  *message;
  char const    *uid;
  char          *fqc_id;

  fqc_id = yts_proxy_get_fqc_id (YTS_PROXY (profile));
  contact = yts_service_get_contact (YTS_SERVICE (self));
  client = yts_contact_get_client (contact);
  uid = yts_service_get_uid (YTS_SERVICE (self));

  message = yts_invocation_message_new (invocation_id,
                                         fqc_id,
                                         aspect,
                                         arguments);

  // TODO maybe we should attach the invocation-id to the contact
  // and handle the timeout here, so handling the response is simpler.

  yts_client_send_message (client, contact, uid, message);

  g_object_unref (message);
  g_free (fqc_id);
}

static void
_proxy_invoke_service (YtsProxy        *proxy,
                       char const       *invocation_id,
                       char const       *aspect,
                       GVariant         *arguments,
                       YtsProxyService *self)
{
  YtsProxyServicePrivate *priv = GET_PRIVATE (self);
  YtsContact     *contact;
  YtsClient      *client;
  YtsMetadata    *message;
  char const      *uid;
  GHashTableIter   iter;
  char const      *capability;
  YtsProxy const *p;

  contact = yts_service_get_contact (YTS_SERVICE (self));
  client = yts_contact_get_client (contact);
  uid = yts_service_get_uid (YTS_SERVICE (self));

  /* FIXME not very nice, but a proxy doesn't know its capability, otherwise
   * it conflicts with the capability property of YtsVSPlayer et al. */
  // TODO get capability from proxy parameter instead.
  g_hash_table_iter_init (&iter, priv->proxies);
  while (g_hash_table_iter_next (&iter, (void **) &capability, (void **) &p)) {
    if (p == proxy) {
      /* This is the capability we're looking for */
      break;
    }
  }

  message = yts_invocation_message_new (invocation_id,
                                         capability,
                                         aspect,
                                         arguments);

  // TODO maybe we should attach the invocation-id to the contact
  // and handle the timeout here, so handling the response is simpler.

  yts_client_send_message (client, contact, uid, message);

  g_object_unref (message);
}

static void
_proxy_destroyed (YtsProxyService  *self,
                  void              *stale_proxy_ptr)
{
  YtsProxyServicePrivate *priv = GET_PRIVATE (self);
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

bool
yts_proxy_service_create_proxy (YtsProxyService *self,
                                 char const       *capability)
{
  YtsProxyServicePrivate *priv = GET_PRIVATE (self);
  bool   has_fqc_id;
  char  *invocation_id;

  has_fqc_id = yts_capability_has_fqc_id (YTS_CAPABILITY (self), capability);
  if (!has_fqc_id) {
    // FIXME GError
    g_critical ("%s : Service does not support capability %s",
                G_STRLOC,
                capability);
    return false;
  }

  if (NULL == priv->profile) {
    /* Lazily create the profile proxy. */
    priv->profile = g_object_new (YTS_TYPE_PROFILE_PROXY, NULL);
    g_signal_connect (priv->profile, "invoke-service",
                      G_CALLBACK (_profile_invoke_service), self);
  }

  /* Register new proxy with the service.
   * For now just remember its type, and create it when the server responds. */

  invocation_id = yts_proxy_create_invocation_id (YTS_PROXY (priv->profile));
  g_hash_table_insert (priv->pending_proxies,
                       invocation_id,
                       g_strdup (capability));

  // TODO timeout
  yts_profile_register_proxy (priv->profile, invocation_id, capability);

  return true;
}

bool
yts_proxy_service_dispatch_event (YtsProxyService *self,
                                   char const       *capability,
                                   char const       *aspect,
                                   GVariant         *arguments)
{
  YtsProxyServicePrivate *priv = GET_PRIVATE (self);

  if (0 == g_strcmp0 (YTS_PROFILE_FQC_ID, capability)) {

    /* This one comes from the Profile / Meta interface */
    yts_proxy_handle_service_event (YTS_PROXY (priv->profile),
                                     aspect,
                                     arguments);
    return true;

  } else {

    YtsProxy *proxy = g_hash_table_lookup (priv->proxies, capability);
    if (proxy) {
      yts_proxy_handle_service_event (proxy, aspect, arguments);
      return true;
    }
  }

  return false;
}

bool
yts_proxy_service_dispatch_response (YtsProxyService  *self,
                                      char const        *capability,
                                      char const        *invocation_id,
                                      GVariant          *response)
{
  YtsProxyServicePrivate *priv = GET_PRIVATE (self);
  char const *new_proxy_fqc_id;

  /* PONDERING this reply should really go to the profile proxy
   * and be handled there. */
  new_proxy_fqc_id = (char const *) g_hash_table_lookup (priv->pending_proxies,
                                                         invocation_id);
  if (new_proxy_fqc_id) {

    YtsProxyFactory * const factory = yts_proxy_factory_get_default ();
    YtsProxy *proxy;
    /* Initial properties for the proxy */
    GVariantIter iter;
    char *name;
    GVariant *value;

    if (!g_variant_is_of_type (response, G_VARIANT_TYPE_DICTIONARY)) {
      g_critical ("%s : Registering proxy for capability %s failed",
                  G_STRLOC,
                  new_proxy_fqc_id);
      return false;
    }

    /* Create proxy object */
    proxy = yts_proxy_factory_create_proxy (factory, new_proxy_fqc_id);
    if (!proxy) {
      g_critical ("%s : Creating proxy for capability %s failed",
                  G_STRLOC,
                  new_proxy_fqc_id);
      return false;
    }

    g_hash_table_insert (priv->proxies,
                         g_strdup (new_proxy_fqc_id),
                         proxy);
    g_signal_connect (proxy, "invoke-service",
                      G_CALLBACK (_proxy_invoke_service), self);
    g_object_weak_ref (G_OBJECT (proxy),
                       (GWeakNotify) _proxy_destroyed,
                       self);

    g_variant_iter_init (&iter, response);
    while (g_variant_iter_next (&iter, "{sv}", &name, &value)) {
      /* Pass the properties to the proxy through the standard mechanism. */
      yts_proxy_handle_service_event (proxy, name, value);
      g_free (name);
      g_variant_unref (value);
    }

    g_signal_emit (self, _signals[SIG_PROXY_CREATED], 0, proxy);
    g_object_unref (proxy);
    g_hash_table_remove (priv->pending_proxies, invocation_id);
    return true;
  }

  if (0 == g_strcmp0 (YTS_PROFILE_FQC_ID, capability)) {

    /* This one comes from the Profile / Meta interface */
    yts_proxy_handle_service_response (YTS_PROXY (priv->profile),
                                        invocation_id,
                                        response);
    return true;

  } else {

    YtsProxy *proxy = g_hash_table_lookup (priv->proxies, capability);
    if (proxy) {
      yts_proxy_handle_service_response (proxy, invocation_id, response);
      return true;
    }
  }

  return false;
}

