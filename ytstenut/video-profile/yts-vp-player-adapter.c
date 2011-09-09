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

#include "yts-service-adapter.h"
#include "yts-vp-player.h"
#include "yts-vp-player-adapter.h"
#include "config.h"

G_DEFINE_TYPE (YtsVPPlayerAdapter,
               yts_vp_player_adapter,
               YTS_TYPE_SERVICE_ADAPTER)

#define GET_PRIVATE(o)                                        \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o),                          \
                                YTS_VP_TYPE_PLAYER_ADAPTER,  \
                                YtsVPPlayerAdapterPrivate))

typedef struct {
  YtsVPPlayer  *player;
} YtsVPPlayerAdapterPrivate;

/*
 * YtsServiceAdapter overrides
 */

static GVariant *
_service_adapter_collect_properties (YtsServiceAdapter *self)
{
  YtsVPPlayerAdapterPrivate *priv = GET_PRIVATE (self);
  GVariantBuilder  builder;
  bool             playing;
  double           volume;
  char            *playable_uri;

  g_variant_builder_init (&builder, G_VARIANT_TYPE_ARRAY);

  playing = yts_vp_player_get_playing (priv->player);
  g_variant_builder_add (&builder, "{sv}", "playing",
                                           g_variant_new_boolean (playing));

  volume = yts_vp_player_get_volume (priv->player);
  g_variant_builder_add (&builder, "{sv}", "volume",
                                           g_variant_new_double (volume));

  /* PONDERING should NULL properties be synched or assumed default? */
  playable_uri = yts_vp_player_get_playable_uri (priv->player);
  if (playable_uri) {
    g_variant_builder_add (&builder, "{sv}", "playable-uri",
                                             g_variant_new_string (playable_uri));
    g_free (playable_uri);
  }

  return g_variant_builder_end (&builder);
}

static bool
_service_adapter_invoke (YtsServiceAdapter *self,
                         char const         *invocation_id,
                         char const         *aspect,
                         GVariant           *arguments)
{
  YtsVPPlayerAdapterPrivate *priv = GET_PRIVATE (self);
  bool keep_sae = false;

  /* Properties */

  if (0 == g_strcmp0 ("playable", aspect)) {

    /* TODO */
    g_debug ("%s : 'playable' property not implemented yet", G_STRLOC);

  } else if (0 == g_strcmp0 ("playing", aspect) &&
             arguments &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_BOOLEAN)) {

    /* PONDERING at some point we can maybe optimize out sending back the notification */
    yts_vp_player_set_playing (priv->player,
                                g_variant_get_boolean (arguments));

  } else if (0 == g_strcmp0 ("volume", aspect) &&
             arguments &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_DOUBLE)) {

    /* PONDERING at some point we can maybe optimize out sending back the notification */
    yts_vp_player_set_volume (priv->player,
                               g_variant_get_double (arguments));

  } else if (0 == g_strcmp0 ("playable-uri", aspect) &&
             arguments &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    /* PONDERING at some point we can maybe optimize out sending back the notification */
    yts_vp_player_set_playable_uri (priv->player,
                                     g_variant_get_string (arguments, NULL));

  } else

  /* Methods */

  if (0 == g_strcmp0 ("play", aspect)) {

    yts_vp_player_play (priv->player);

  } else if (0 == g_strcmp0 ("pause", aspect)) {

    yts_vp_player_pause (priv->player);

  } else if (0 == g_strcmp0 ("next", aspect)) {

    yts_vp_player_next (priv->player, invocation_id);
    /* This method responds whether it could skip to next,
     * so keep the return envelope. */
    keep_sae = true;

  } else if (0 == g_strcmp0 ("prev", aspect)) {

    yts_vp_player_prev (priv->player, invocation_id);
    /* This method responds whether it could skip to next,
     * so keep the return envelope. */
    keep_sae = true;

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

  return keep_sae;
}

/*
 * YtsVPPlayerAdapter
 */

enum {
  PROP_0 = 0,
  PROP_SERVICE_ADAPTER_FQC_ID,
  PROP_SERVICE_ADAPTER_SERVICE
};

static void
_player_notify_playable (YtsVPPlayer         *player,
                         GParamSpec           *pspec,
                         YtsVPPlayerAdapter  *self)
{
  /* TODO */
  g_debug ("%s : %s() not implemented yet", G_STRLOC, __FUNCTION__);
}

static void
_player_notify_playing (YtsVPPlayer         *player,
                        GParamSpec           *pspec,
                        YtsVPPlayerAdapter  *self)
{
  bool playing;

  playing = yts_vp_player_get_playing (player);
  yts_service_adapter_send_event (YTS_SERVICE_ADAPTER (self),
                                   "playing",
                                   g_variant_new_boolean (playing));
}

static void
_player_notify_volume (YtsVPPlayer         *player,
                       GParamSpec           *pspec,
                       YtsVPPlayerAdapter  *self)
{
  double volume;

  volume = yts_vp_player_get_volume (player);
  yts_service_adapter_send_event (YTS_SERVICE_ADAPTER (self),
                                   "volume",
                                   g_variant_new_double (volume));
}

static void
_player_notify_playable_uri (YtsVPPlayer         *player,
                             GParamSpec           *pspec,
                             YtsVPPlayerAdapter  *self)
{
  char *playable_uri;

  playable_uri = yts_vp_player_get_playable_uri (player);
  yts_service_adapter_send_event (YTS_SERVICE_ADAPTER (self),
                                   "playable-uri",
                                   g_variant_new_string (playable_uri));
  g_free (playable_uri);
}

static void
_player_next_response (YtsVPPlayer         *player,
                       char const           *invocation_id,
                       bool                  return_value,
                       YtsVPPlayerAdapter  *self)
{
  yts_service_adapter_send_response (YTS_SERVICE_ADAPTER (self),
                                      invocation_id,
                                      g_variant_new_boolean (return_value));
}

static void
_player_prev_response (YtsVPPlayer         *player,
                       char const           *invocation_id,
                       bool                  return_value,
                       YtsVPPlayerAdapter  *self)
{
  yts_service_adapter_send_response (YTS_SERVICE_ADAPTER (self),
                                      invocation_id,
                                      g_variant_new_boolean (return_value));
}

static void
_player_destroyed (YtsVPPlayerAdapter  *self,
                   void                 *stale_player_ptr)
{
  YtsVPPlayerAdapterPrivate *priv = GET_PRIVATE (self);

  priv->player = NULL;
  g_object_unref (self);
}

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsVPPlayerAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_SERVICE_ADAPTER_FQC_ID:
      g_value_set_string (value, YTS_VP_PLAYER_FQC_ID);
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
               unsigned      property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  YtsVPPlayerAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    case PROP_SERVICE_ADAPTER_SERVICE:

      /* Construct-only */

      g_return_if_fail (priv->player == NULL);
      g_return_if_fail (YTS_VP_IS_PLAYER (g_value_get_object (value)));

      priv->player = YTS_VP_PLAYER (g_value_get_object (value));
      g_object_weak_ref (G_OBJECT (priv->player),
                         (GWeakNotify) _player_destroyed,
                         object);

      g_signal_connect (priv->player, "notify::playable",
                        G_CALLBACK (_player_notify_playable), object);
      g_signal_connect (priv->player, "notify::playing",
                        G_CALLBACK (_player_notify_playing), object);
      g_signal_connect (priv->player, "notify::volume",
                        G_CALLBACK (_player_notify_volume), object);
      g_signal_connect (priv->player, "notify::playable-uri",
                        G_CALLBACK (_player_notify_playable_uri), object);

      g_signal_connect (priv->player, "next-response",
                        G_CALLBACK (_player_next_response), object);
      g_signal_connect (priv->player, "prev-response",
                        G_CALLBACK (_player_prev_response), object);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsVPPlayerAdapterPrivate *priv = GET_PRIVATE (object);

  if (priv->player) {
    g_warning ("%s : Adapter disposed with adaptee still referenced.",
               G_STRLOC);
    g_object_weak_unref (G_OBJECT (priv->player),
                         (GWeakNotify) _player_destroyed,
                         object);
    priv->player = NULL;
  }

  G_OBJECT_CLASS (yts_vp_player_adapter_parent_class)->dispose (object);
}

static void
yts_vp_player_adapter_class_init (YtsVPPlayerAdapterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  YtsServiceAdapterClass *adapter_class = YTS_SERVICE_ADAPTER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsVPPlayerAdapterPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  adapter_class->collect_properties = _service_adapter_collect_properties;
  adapter_class->invoke = _service_adapter_invoke;

  /* Properties */

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_FQC_ID,
                                    "fqc-id");

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_SERVICE,
                                    "service");
}

static void
yts_vp_player_adapter_init (YtsVPPlayerAdapter *self)
{
}

