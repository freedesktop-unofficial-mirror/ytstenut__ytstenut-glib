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

#include "ytsg-vp-playable.h"
#include "ytsg-vp-playable-proxy.h"

static void
_playable_interface_init (YtsgVPPlayableInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsgVPPlayableProxy,
                         ytsg_vp_playable_proxy,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (YTSG_VP_TYPE_PLAYABLE,
                                                _playable_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_VP_TYPE_PLAYABLE_PROXY, YtsgVPPlayableProxyPrivate))

typedef struct {
  double       duration;
  GHashTable  *metadata;
  double       position;
  char        *thumbnail;
  char        *title;
  char        *uri;
} YtsgVPPlayableProxyPrivate;

/*
 * YtsgVPPlayable implementation
 */

static void
_playable_interface_init (YtsgVPPlayableInterface *interface)
{
}

/*
 * YtsgVPPlayableProxy
 */

enum {
  PROP_0,
  PROP_PLAYABLE_DURATION,
  PROP_PLAYABLE_METADATA,
  PROP_PLAYABLE_POSITION,
  PROP_PLAYABLE_THUMBNAIL,
  PROP_PLAYABLE_TITLE,
  PROP_PLAYABLE_URI
};

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsgVPPlayableProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_PLAYABLE_DURATION:
      g_value_set_double (value, priv->duration);
      break;
    case PROP_PLAYABLE_METADATA:
      g_value_set_boxed (value, priv->metadata);
      break;
    case PROP_PLAYABLE_POSITION:
      g_value_set_double (value, priv->position);
      break;
    case PROP_PLAYABLE_THUMBNAIL:
      g_value_set_string (value, priv->thumbnail);
      break;
    case PROP_PLAYABLE_TITLE:
      g_value_set_string (value, priv->title);
      break;
    case PROP_PLAYABLE_URI:
      g_value_set_string (value, priv->uri);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_set_property (GObject      *object,
               unsigned      property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  YtsgVPPlayableProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_PLAYABLE_POSITION: {
      double position = g_value_get_double (value);
      if (position != priv->position) {
        priv->position = position;
        g_object_notify (object, "postion");
        /* TODO send home etc */
      }
    } break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsgVPPlayableProxyPrivate *priv = GET_PRIVATE (object);

  if (priv->metadata) {
    g_hash_table_unref (priv->metadata);
    priv->metadata = NULL;
  }

  if (priv->thumbnail) {
    g_free (priv->thumbnail);
    priv->thumbnail = NULL;
  }

  if (priv->uri) {
    g_free (priv->uri);
    priv->uri = NULL;
  }

  G_OBJECT_CLASS (ytsg_vp_playable_proxy_parent_class)->dispose (object);
}

static void
ytsg_vp_playable_proxy_class_init (YtsgVPPlayableProxyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgVPPlayableProxyPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  g_object_class_override_property (object_class,
                                    PROP_PLAYABLE_DURATION,
                                    "duration");

  g_object_class_override_property (object_class,
                                    PROP_PLAYABLE_METADATA,
                                    "metadata");

  g_object_class_override_property (object_class,
                                    PROP_PLAYABLE_POSITION,
                                    "position");

  g_object_class_override_property (object_class,
                                    PROP_PLAYABLE_THUMBNAIL,
                                    "thumbnail");

  g_object_class_override_property (object_class,
                                    PROP_PLAYABLE_TITLE,
                                    "title");

  g_object_class_override_property (object_class,
                                    PROP_PLAYABLE_URI,
                                    "uri");
}

static void
ytsg_vp_playable_proxy_init (YtsgVPPlayableProxy *self)
{
}

YtsgVPPlayableProxy *
ytsg_vp_playable_proxy_new (double       duration,
                            GHashTable  *metadata,
                            double       position,
                            char const  *thumbnail,
                            char const  *title,
                            char const  *uri)
{
  return g_object_new (YTSG_VP_TYPE_PLAYABLE_PROXY,
                       "duration",  duration,
                       "metadata",  metadata,
                       "position",  position,
                       "title",     title,
                       "uri",       uri,
                       NULL);
}

