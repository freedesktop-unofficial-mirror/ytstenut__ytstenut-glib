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

#include <stdbool.h>
#include <math.h>

#include "yts-vp-playable.h"
#include "yts-vp-playable-proxy.h"
#include "yts-vp-player.h"
#include "yts-vp-player-proxy.h"

static void
_player_interface_init (YtsVPPlayerInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsVPPlayerProxy,
                         yts_vp_player_proxy,
                         YTS_TYPE_PROXY,
                         G_IMPLEMENT_INTERFACE (YTS_VP_TYPE_PLAYER,
                                                _player_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_VP_TYPE_PLAYER_PROXY, YtsVPPlayerProxyPrivate))

typedef struct {

  /* Properties */
  YtsVPPlayableProxy *playable;
  bool                 playing;
  double               volume;
  char                *playable_uri;

  /* Data */
  GHashTable  *invocations;

} YtsVPPlayerProxyPrivate;

static void
player_proxy_set_playing (YtsVPPlayerProxy *self,
                          bool               playing);
static void
player_proxy_set_playable_uri (YtsVPPlayerProxy  *self,
                               char const         *playable_uri);
static void
player_proxy_set_volume (YtsVPPlayerProxy  *self,
                         double              volume);

/*
 * YtsVPPlayer implementation
 */

static void
_player_play (YtsVPPlayer *self)
{
  char *invocation_id;

  invocation_id = yts_proxy_create_invocation_id (YTS_PROXY (self));
  yts_proxy_invoke (YTS_PROXY (self), invocation_id, "play", NULL);
  g_free (invocation_id);
}

static void
_player_pause (YtsVPPlayer *self)
{
  char *invocation_id;

  invocation_id = yts_proxy_create_invocation_id (YTS_PROXY (self));
  yts_proxy_invoke (YTS_PROXY (self), invocation_id, "pause", NULL);
  g_free (invocation_id);
}

static void
_player_next (YtsVPPlayer  *self,
              char const    *invocation_id_)
{
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (self);
  char *invocation_id;

  invocation_id = invocation_id_ ?
                    g_strdup (invocation_id_) :
                    yts_proxy_create_invocation_id (YTS_PROXY (self));
  /* Hash takes invocation_id. */
  g_hash_table_insert (priv->invocations,
                       invocation_id, _player_next);
  // TODO set timeout, well, probably in yts-proxy-service.c

  yts_proxy_invoke (YTS_PROXY (self), invocation_id, "next", NULL);
}

static void
_player_prev (YtsVPPlayer  *self,
              char const    *invocation_id_)
{
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (self);
  char *invocation_id;

  invocation_id = invocation_id_ ?
                    g_strdup (invocation_id_) :
                    yts_proxy_create_invocation_id (YTS_PROXY (self));
  /* Hash takes invocation_id. */
  g_hash_table_insert (priv->invocations, invocation_id, _player_prev);
  // TODO set timeout, well, probably in yts-proxy-service.c

  yts_proxy_invoke (YTS_PROXY (self), invocation_id, "prev", NULL);
}

static void
_player_interface_init (YtsVPPlayerInterface *interface)
{
  interface->play = _player_play;
  interface->pause = _player_pause;
  interface->next = _player_next;
  interface->prev = _player_prev;
}

/*
 * YtsProxy overrides
 */

static void
_proxy_service_event (YtsProxy   *self,
                      char const  *aspect,
                      GVariant    *arguments)
{
//  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (self);

  if (0 == g_strcmp0 ("playable", aspect)) {

    /* TODO */
    g_debug ("%s : 'playable' property not implemented yet", G_STRLOC);

  } else if (0 == g_strcmp0 ("playing", aspect) &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_BOOLEAN)) {

    bool playing = g_variant_get_boolean (arguments);
    player_proxy_set_playing (YTS_VP_PLAYER_PROXY (self), playing);

  } else if (0 == g_strcmp0 ("volume", aspect) &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_DOUBLE)) {

    double volume = g_variant_get_double (arguments);
    player_proxy_set_volume (YTS_VP_PLAYER_PROXY (self), volume);

  } else if (0 == g_strcmp0 ("playable-uri", aspect) &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    char const *playable_uri = g_variant_get_string (arguments, NULL);
    player_proxy_set_playable_uri (YTS_VP_PLAYER_PROXY (self), playable_uri);

  } else {

    g_critical ("%s : Unhandled event '%s' of type '%s'",
                G_STRLOC,
                aspect,
                arguments ?
                  g_variant_get_type_string (arguments) :
                  "NULL");
  }
}

static void
_proxy_service_response (YtsProxy  *self,
                         char const *invocation_id,
                         GVariant   *response)
{
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (self);
  void *call;

  call = g_hash_table_lookup (priv->invocations, invocation_id);
  // TODO clear timeout, well, probably in yts-proxy-service.c

  if (call == _player_next) {

    yts_vp_player_next_return (YTS_VP_PLAYER (self),
                                invocation_id,
                                g_variant_get_boolean (response));

  } else if (call == _player_prev) {

    yts_vp_player_prev_return (YTS_VP_PLAYER (self),
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
 * YtsVPPlayerProxy
 */

enum {
  PROP_0 = 0,

  PROP_CAPABILITY_FQC_IDS,

  PROP_PLAYER_PLAYABLE,
  PROP_PLAYER_PLAYING,
  PROP_PLAYER_VOLUME,
  PROP_PLAYER_PLAYABLE_URI
};

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CAPABILITY_FQC_IDS: {
      char const *fqc_ids[] = { YTS_VP_PLAYER_FQC_ID, NULL };
      g_value_set_boxed (value, fqc_ids);
    } break;
    case PROP_PLAYER_PLAYABLE:
      g_value_set_object (value, priv->playable);
      break;
    case PROP_PLAYER_PLAYING:
      g_value_set_boolean (value, priv->playing);
      break;
    case PROP_PLAYER_VOLUME:
      g_value_set_double (value, priv->volume);
      break;
    case PROP_PLAYER_PLAYABLE_URI:
      g_value_set_string (value, priv->playable_uri);
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
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (object);
  char *invocation_id;

  switch (property_id) {

    case PROP_PLAYER_PLAYABLE: {

      // FIXME
      g_warning ("%s : Property 'playable' not implemented", G_STRLOC);

    } break;

    case PROP_PLAYER_PLAYING: {
      bool playing = g_value_get_boolean (value);
      if (playing != priv->playing) {
        invocation_id = yts_proxy_create_invocation_id (YTS_PROXY (object));
        yts_proxy_invoke (YTS_PROXY (object), invocation_id,
                           "playing", g_variant_new_boolean (playing));
        g_free (invocation_id);
      }
    } break;

    case PROP_PLAYER_VOLUME: {
      double volume = g_value_get_double (value);
      if (0.01 < fabs (volume - priv->volume)) {
        invocation_id = yts_proxy_create_invocation_id (YTS_PROXY (object));
        yts_proxy_invoke (YTS_PROXY (object), invocation_id,
                           "volume", g_variant_new_double (volume));
        g_free (invocation_id);
      }
    } break;

    case PROP_PLAYER_PLAYABLE_URI: {
      char const *playable_uri = g_value_get_string (value);
      if (0 != g_strcmp0 (playable_uri, priv->playable_uri)) {
        invocation_id = yts_proxy_create_invocation_id (YTS_PROXY (object));
        yts_proxy_invoke (YTS_PROXY (object), invocation_id,
                           "playable-uri", g_variant_new_string (playable_uri));
        g_free (invocation_id);
      }
    } break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (object);

  if (priv->playable) {
    g_object_unref (G_OBJECT (priv->playable));
    priv->playable = NULL;
  }

  if (priv->invocations) {
    g_hash_table_destroy (priv->invocations);
    priv->invocations = NULL;
  }

  G_OBJECT_CLASS (yts_vp_player_proxy_parent_class)->dispose (object);
}

static void
yts_vp_player_proxy_class_init (YtsVPPlayerProxyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  YtsProxyClass  *proxy_class = YTS_PROXY_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsVPPlayerProxyPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  proxy_class->service_event = _proxy_service_event;
  proxy_class->service_response = _proxy_service_response;

  /* YtsCapability */

  g_object_class_override_property (object_class,
                                    PROP_CAPABILITY_FQC_IDS,
                                    "fqc-ids");

  /* YtsVPPlayer */

  g_object_class_override_property (object_class,
                                    PROP_PLAYER_PLAYABLE,
                                    "playable");

  g_object_class_override_property (object_class,
                                    PROP_PLAYER_PLAYING,
                                    "playing");

  g_object_class_override_property (object_class,
                                    PROP_PLAYER_VOLUME,
                                    "volume");

  g_object_class_override_property (object_class,
                                    PROP_PLAYER_PLAYABLE_URI,
                                    "playable-uri");
}

static void
yts_vp_player_proxy_init (YtsVPPlayerProxy *self)
{
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (self);

  priv->invocations = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             NULL);
}

static void
player_proxy_set_playing (YtsVPPlayerProxy *self,
                          bool               playing)
{
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (self);

  if (playing != priv->playing) {
    priv->playing = playing;
    g_object_notify (G_OBJECT (self), "playing");
  }
}

static void
player_proxy_set_playable_uri (YtsVPPlayerProxy  *self,
                               char const         *playable_uri)
{
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (self);

  if (0 != g_strcmp0 (playable_uri, priv->playable_uri)) {

    if (priv->playable_uri) {
      g_free (priv->playable_uri);
      priv->playable_uri = NULL;
    }

    if (playable_uri) {
      priv->playable_uri = g_strdup (playable_uri);
    }

    g_object_notify (G_OBJECT (self), "playable-uri");
  }
}

static void
player_proxy_set_volume (YtsVPPlayerProxy  *self,
                         double              volume)
{
  YtsVPPlayerProxyPrivate *priv = GET_PRIVATE (self);

  if (0.01 < fabs (volume - priv->volume)) {
    priv->volume = volume;
    g_object_notify (G_OBJECT (self), "volume");
  }
}

