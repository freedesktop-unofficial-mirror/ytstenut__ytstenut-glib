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
#include "ytsg-proxy-service.h"

G_DEFINE_TYPE (YtsgProxyService, ytsg_proxy_service, YTSG_TYPE_SERVICE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_VS_TYPE_PROXY_SERVICE, YtsgProxyServicePrivate))

typedef struct {
  int dummy;
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
  // YtsgProxyServicePrivate *priv = GET_PRIVATE (object);

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

// FIXME need factory foo

extern GType
ytsg_vs_player_proxy_get_type (void);

GObject *
ytsg_proxy_service_create_proxy (YtsgProxyService *self,
                                 char const       *capability)
{
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

  for (i = 0; proxies[i].capability != NULL; i++) {
    if (0 == g_strcmp0 (capability, proxies[i].capability)) {
      return g_object_new (proxies[i].gtype,
                           "",
                           NULL);
    }
  }

  return NULL;

}

