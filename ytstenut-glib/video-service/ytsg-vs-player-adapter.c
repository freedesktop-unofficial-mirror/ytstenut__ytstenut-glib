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
#include "ytsg-service-adapter.h"
#include "ytsg-vs-player.h"
#include "ytsg-vs-player-adapter.h"

G_DEFINE_TYPE (YtsgVSPlayerAdapter,
               ytsg_vs_player_adapter,
               YTSG_TYPE_SERVICE_ADAPTER)

#define GET_PRIVATE(o)                                        \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o),                          \
                                YTSG_VS_TYPE_PLAYER_ADAPTER,  \
                                YtsgVSPlayerAdapterPrivate))

typedef struct {
  YtsgVSPlayer  *player;

  unsigned int   notify_playable_handler;
  unsigned int   notify_playing_handler;
  unsigned int   notify_volume_handler;
} YtsgVSPlayerAdapterPrivate;

/*
 * YtsgServiceAdapter implementation
 */

static void
_service_adapter_invoke (YtsgServiceAdapter *self,
                         char const         *invocation_id,
                         char const         *aspect,
                         GVariant           *arguments)
{
  YtsgVSPlayerAdapterPrivate *priv = GET_PRIVATE (self);

  /* Properties */

  if (0 == g_strcmp0 ("playable", aspect)) {

    /* TODO */
    g_debug ("%s : 'playable' property not implemented yet", G_STRLOC);

  } else if (0 == g_strcmp0 ("playing", aspect) &&
             arguments &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_BOOLEAN)) {

    /* PONDERING at some point we can maybe optimize out sending back the notification */
    // g_signal_handler_block (priv->player, priv->notify_playing_handler);
    ytsg_vs_player_set_playing (priv->player, g_variant_get_boolean (arguments));
    // g_signal_handler_unblock (priv->player, priv->notify_playing_handler);

  } else if (0 == g_strcmp0 ("volume", aspect) &&
             arguments &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_DOUBLE)) {

    /* PONDERING at some point we can maybe optimize out sending back the notification */
    // g_signal_handler_block (priv->player, priv->notify_volume_handler);
    ytsg_vs_player_set_volume (priv->player, g_variant_get_double (arguments));
    // g_signal_handler_unblock (priv->player, priv->notify_volume_handler);

  } else

  /* Methods */

  if (0 == g_strcmp0 ("play", aspect)) {

    ytsg_vs_player_play (priv->player);

  } else if (0 == g_strcmp0 ("pause", aspect)) {

    ytsg_vs_player_pause (priv->player);

  } else if (0 == g_strcmp0 ("next", aspect)) {

    ytsg_vs_player_next (priv->player);

  } else if (0 == g_strcmp0 ("prev", aspect)) {

    ytsg_vs_player_prev (priv->player);

  } else {

    char *arg_string = arguments ?
                          g_variant_print (arguments, false) :
                          g_strdup ("NULL");
    g_warning ("%s : Unknown method %s.%s(%s)",
               G_STRLOC,
               G_OBJECT_TYPE_NAME (priv->player),
               aspect,
               arg_string);
    g_free (arg_string);
  }
}

/*
 * YtsgVSPlayerAdapter
 */

enum {
  PROP_0 = 0,
  PROP_SERVICE_ADAPTER_SERVICE_GTYPE,
  PROP_SERVICE_ADAPTER_SERVICE
};

static void
_player_notify_playable (YtsgVSPlayer         *player,
                         GParamSpec           *pspec,
                         YtsgVSPlayerAdapter  *self)
{
  /* TODO */
  g_debug ("%s : %s() not implemented yet", G_STRLOC, __FUNCTION__);
}

static void
_player_notify_playing (YtsgVSPlayer         *player,
                        GParamSpec           *pspec,
                        YtsgVSPlayerAdapter  *self)
{
  bool playing;

  playing = ytsg_vs_player_get_playing (player);
  ytsg_service_adapter_send_event (YTSG_SERVICE_ADAPTER (self),
                                   "playing",
                                   g_variant_new_boolean (playing));
}

static void
_player_volume_playable (YtsgVSPlayer         *player,
                         GParamSpec           *pspec,
                         YtsgVSPlayerAdapter  *self)
{
  double volume;

  volume = ytsg_vs_player_get_volume (player);
  ytsg_service_adapter_send_event (YTSG_SERVICE_ADAPTER (self),
                                   "volume",
                                   g_variant_new_double (volume));
}

static void
_player_destroyed (YtsgVSPlayerAdapter  *self,
                   void                 *stale_player_ptr)
{
  g_object_unref (self);
}

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  YtsgVSPlayerAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_SERVICE_ADAPTER_SERVICE_GTYPE:
      g_value_set_gtype (value, YTSG_VS_TYPE_PLAYER);
      break;
    case PROP_SERVICE_ADAPTER_SERVICE:
      g_value_set_object (value, priv->player);
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
  YtsgVSPlayerAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    case PROP_SERVICE_ADAPTER_SERVICE:

      /* Construct-only */

      g_return_if_fail (priv->player == NULL);
      g_return_if_fail (YTSG_VS_IS_PLAYER (g_value_get_object (value)));

      priv->player = YTSG_VS_PLAYER (g_value_get_object (value));

      priv->notify_playable_handler =
              g_signal_connect (priv->player, "notify::playable",
                                G_CALLBACK (_player_notify_playable), object);

      priv->notify_playing_handler =
              g_signal_connect (priv->player, "notify::playing",
                                G_CALLBACK (_player_notify_playing), object);

      priv->notify_volume_handler =
              g_signal_connect (priv->player, "notify::volume",
                                G_CALLBACK (_player_volume_playable), object);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_constructed (GObject *object)
{
  YtsgVSPlayerAdapterPrivate *priv = GET_PRIVATE (object);

  g_return_if_fail (priv->player);

  g_object_weak_ref (G_OBJECT (priv->player),
                     (GWeakNotify) _player_destroyed,
                     object);
}

static void
_dispose (GObject *object)
{
  G_OBJECT_CLASS (ytsg_vs_player_adapter_parent_class)->dispose (object);
}

static void
ytsg_vs_player_adapter_class_init (YtsgVSPlayerAdapterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  YtsgServiceAdapterClass *adapter_class = YTSG_SERVICE_ADAPTER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgVSPlayerAdapterPrivate));

  object_class->constructed = _constructed;
  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  adapter_class->invoke = _service_adapter_invoke;

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_SERVICE,
                                    "service");

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_SERVICE_GTYPE,
                                    "service-gtype");
}

static void
ytsg_vs_player_adapter_init (YtsgVSPlayerAdapter *self)
{
}

