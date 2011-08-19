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
#include "ytsg-private.h"
#include "ytsg-proxy-service.h"

G_DEFINE_TYPE (YtsgProxyService, ytsg_proxy_service, YTSG_TYPE_SERVICE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_PROXY_SERVICE, YtsgProxyServicePrivate))

typedef struct {
  GHashTable  *proxies;
} YtsgProxyServicePrivate;


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
    g_hash_table_unref (priv->proxies);
    priv->proxies = NULL;
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
}

static void
ytsg_proxy_service_init (YtsgProxyService *self)
{
  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);

  priv->proxies = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         NULL);
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
_proxy_invoke_service (YtsgProxy        *proxy,
                       char const       *invocation_id,
                       char const       *aspect,
                       GVariant         *arguments,
                       YtsgProxyService *self)
{
  YtsgContact   *contact;
  YtsgClient    *client;
  YtsgMetadata  *message;
  char const    *uid;
  char const    *capability;

  contact = ytsg_service_get_contact (YTSG_SERVICE (self));
  client = ytsg_contact_get_client (contact);
  uid = ytsg_service_get_uid (YTSG_SERVICE (self));

  capability = ytsg_proxy_get_capability (proxy);
  message = ytsg_invocation_message_new (invocation_id,
                                         capability,
                                         aspect,
                                         arguments);

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
ytsg_vs_player_proxy_get_type (void);

YtsgProxy *
ytsg_proxy_service_create_proxy (YtsgProxyService *self,
                                 char const       *capability)
{
  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);
  YtsgProxy *proxy;

  struct {
    char const *capability;
    GType       gtype;
  } proxies[] = {
//    { "org.freedesktop.ytstenut.VideoService.Content",
//      ytsg_vs_content_proxy_get_type () },
    { "org.freedesktop.ytstenut.VideoService.Player",
      ytsg_vs_player_proxy_get_type () },
//    { "org.freedesktop.ytstenut.VideoService.Transcript",
//      ytsg_vs_transcript_proxy_get_type () },
//    { "org.freedesktop.ytstenut.VideoService.Transfer",
//      ytsg_vs_transfer_proxy_get_type () },
    { NULL }
  };

  unsigned int i;

  g_return_val_if_fail (YTSG_IS_PROXY_SERVICE (self), NULL);

  proxy = NULL;
  for (i = 0; proxies[i].capability != NULL; i++) {
    if (0 == g_strcmp0 (capability, proxies[i].capability)) {
      proxy = g_object_new (proxies[i].gtype,
                            "capability", capability,
                            NULL);
      break;
    }
  }

  if (proxy) {
    g_hash_table_insert (priv->proxies,
                         g_strdup (capability),
                         proxy);
    g_signal_connect (proxy, "invoke-service",
                      G_CALLBACK (_proxy_invoke_service), self);
    g_object_weak_ref (G_OBJECT (proxy),
                       (GWeakNotify) _proxy_destroyed,
                       self);
  }

  return proxy;
}

bool
ytsg_proxy_service_dispatch_event (YtsgProxyService *self,
                                   char const       *capability,
                                   char const       *aspect,
                                   GVariant         *arguments)
{
  YtsgProxy *proxy;

  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);

  proxy = g_hash_table_lookup (priv->proxies, capability);
  if (proxy) {
    ytsg_proxy_handle_service_event (proxy, aspect, arguments);
    return true;
  }

  return false;
}

bool
ytsg_proxy_service_dispatch_response (YtsgProxyService  *self,
                                      char const        *capability,
                                      char const        *invocation_id,
                                      GVariant          *response)
{
  YtsgProxy *proxy;

  YtsgProxyServicePrivate *priv = GET_PRIVATE (self);

  proxy = g_hash_table_lookup (priv->proxies, capability);
  if (proxy) {
    ytsg_proxy_handle_service_response (proxy, invocation_id, response);
    return true;
  }

  return false;
}

