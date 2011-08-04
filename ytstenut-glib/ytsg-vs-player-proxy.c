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
#include "ytsg-vs-playable.h"
#include "ytsg-vs-playable-proxy.h"
#include "ytsg-vs-player.h"
#include "ytsg-vs-player-proxy.h"

static void
_player_interface_init (YtsgVSPlayerInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsgVSPlayerProxy, ytsg_vs_player_proxy, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (YTSG_VS_TYPE_PLAYER, _player_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_VS_TYPE_PLAYER_PROXY, YtsgVSPlayerProxyPrivate))

typedef struct {
  YtsgVSPlayableProxy *playable;
  bool                 playing;
  double               volume;
} YtsgVSPlayerProxyPrivate;

/*
 * YtsgVSPlayer implementation
 */

static void
_player_interface_init (YtsgVSPlayerInterface *interface)
{
}

/*
 * YtsgVSPlayerProxy
 */

enum {
  PROP_0 = 0,
  PROP_PLAYER_PLAYABLE,
  PROP_PLAYER_PLAYING,
  PROP_PLAYER_VOLUME
};

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  YtsgVSPlayerProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_PLAYER_PLAYABLE:
      g_value_set_object (value, priv->playable);
      break;
    case PROP_PLAYER_PLAYING:
      g_value_set_boolean (value, priv->playing);
      break;
    case PROP_PLAYER_VOLUME:
      g_value_set_double (value, priv->volume);
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
  YtsgVSPlayerProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    case PROP_PLAYER_PLAYABLE: {
      GObject *playable = g_value_get_object (value);
      if (playable != G_OBJECT (priv->playable)) {
        if (priv->playable) {
          g_object_unref (priv->playable);
          priv->playable = NULL;
        }
        if (playable) {
          priv->playable = g_object_ref (playable);
        }
        g_object_notify (object, "playable");
      }
    } break;

    case PROP_PLAYER_PLAYING: {
      bool playing = g_value_get_boolean (value);
      if (playing != priv->playing) {
        priv->playing = playing;
        g_object_notify (object, "playing");
      }
    } break;

    case PROP_PLAYER_VOLUME: {
      double volume = g_value_get_double (value);
      if (volume != priv->volume) {
        priv->volume = volume;
        g_object_notify (object, "volume");
      }
    } break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsgVSPlayerProxyPrivate *priv = GET_PRIVATE (object);

  if (priv->playable) {
    g_object_unref (G_OBJECT (priv->playable));
    priv->playable = NULL;
  }

  G_OBJECT_CLASS (ytsg_vs_player_proxy_parent_class)->dispose (object);
}

static void
ytsg_vs_player_proxy_class_init (YtsgVSPlayerProxyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgVSPlayerProxyPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  g_object_class_override_property (object_class,
                                    PROP_PLAYER_PLAYABLE,
                                    "playable");

  g_object_class_override_property (object_class,
                                    PROP_PLAYER_PLAYING,
                                    "playing");

  g_object_class_override_property (object_class,
                                    PROP_PLAYER_VOLUME,
                                    "volume");
}

static void
ytsg_vs_player_proxy_init (YtsgVSPlayerProxy *self)
{
}

YtsgVSPlayerProxy *
ytsg_vs_player_proxy_new (double       duration,
                            GHashTable  *metadata,
                            double       position,
                            char const  *thumbnail,
                            char const  *title,
                            char const  *uri)
{
  return g_object_new (YTSG_VS_TYPE_PLAYER_PROXY,
                       "duration",  duration,
                       "metadata",  metadata,
                       "position",  position,
                       "title",     title,
                       "uri",       uri,
                       NULL);
}

