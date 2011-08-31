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

#include <ytstenut-glib/ytstenut-glib.h>
#include "mock-player.h"

static void
_capability_interface_init (YtsgCapability *interface);

static void
_player_interface_init (YtsgVPPlayerInterface *interface);

G_DEFINE_TYPE_WITH_CODE (MockPlayer,
                         mock_player,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (YTSG_TYPE_CAPABILITY,
                                                _capability_interface_init)
                         G_IMPLEMENT_INTERFACE (YTSG_VP_TYPE_PLAYER,
                                                _player_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOCK_TYPE_PLAYER, MockPlayerPrivate))

enum {
  PROP_0,

  PROP_CAPABILITY_FQC_ID,

  PROP_PLAYER_PLAYABLE,
  PROP_PLAYER_PLAYING,
  PROP_PLAYER_VOLUME,
  PROP_PLAYER_PLAYABLE_URI
};

typedef struct {
  char const  **playlist;
  unsigned      current;
  bool          playing;
  double        volume;
  char         *playable_uri;
} MockPlayerPrivate;

/*
 * YtsgCapability implementation
 */

static void
_capability_interface_init (YtsgCapability *interface)
{
  /* Nothing to do, it's just about overriding the "fqc-id" property */
}

/*
 * YtsgVPPlayer
 */

static void
_player_play (YtsgVPPlayer *self)
{
  MockPlayerPrivate *priv = GET_PRIVATE (self);

  g_debug ("YtsgVPPlayer.play() with playing=%s",
           priv->playing ? "true" : "false");

  /* Let the property setter do the work. */
  ytsg_vp_player_set_playing (self, true);
}

static void
_player_pause (YtsgVPPlayer *self)
{
  MockPlayerPrivate *priv = GET_PRIVATE (self);

  g_debug ("YtsgVPPlayer.pause() with playing=%s",
           priv->playing ? "true" : "false");

  /* Let the property setter do the work. */
  ytsg_vp_player_set_playing (self, false);
}

static void
_player_next (YtsgVPPlayer  *self,
              char const    *invocation_id)
{
  MockPlayerPrivate *priv = GET_PRIVATE (self);
  char const  *next;

  next = priv->playlist[priv->current + 1];
  if (next) {
    priv->current++;
  }

  g_debug ("YtsgVPPlayer.next() -- %s", next);

  /* Return true if we skipped to the next item in the playlist. */
  ytsg_vp_player_next_return (self, invocation_id, (bool) next);
}

static void
_player_prev (YtsgVPPlayer  *self,
              char const    *invocation_id)
{
  MockPlayerPrivate *priv = GET_PRIVATE (self);
  char const  *prev;

  prev = priv->current > 0 ?
            priv->playlist[priv->current - 1] :
            NULL;
  if (prev) {
    priv->current--;
  }

  g_debug ("YtsgVPPlayer.prev() -- %s", prev);

  /* Return true if we skipped to the previous item in the playlist. */
  ytsg_vp_player_prev_return (self, invocation_id, (bool) prev);
}

static void
_player_interface_init (YtsgVPPlayerInterface *interface)
{
  interface->play = _player_play;
  interface->pause = _player_pause;
  interface->next = _player_next;
  interface->prev = _player_prev;
}

/*
 * MockPlayer
 */

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  MockPlayerPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CAPABILITY_FQC_ID:
      g_value_set_string (value, YTSG_VP_PLAYER_FQC_ID);
      break;
    case PROP_PLAYER_PLAYABLE:
      /* TODO */
      g_critical ("%s: property MockPlayer.playable not implemented", G_STRLOC);
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
  MockPlayerPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    case PROP_PLAYER_PLAYABLE:
      /* TODO */
      g_critical ("%s: property MockPlayer.playable not implemented", G_STRLOC);
      break;

    case PROP_PLAYER_PLAYING: {
      bool playing = g_value_get_boolean (value);
      if (playing != priv->playing) {
        g_debug ("YtsgVPPlayer.playing = %s", playing ? "true" : "false");
        priv->playing = playing;
        g_object_notify (object, "playing");
      }
    } break;

    case PROP_PLAYER_VOLUME: {
      double volume = g_value_get_double (value);
      if (volume != priv->volume) {
        g_debug ("YtsgVPPlayer.volume = %.2f", volume);
        priv->volume = volume;
        g_object_notify (object, "volume");
      }
    } break;

    case PROP_PLAYER_PLAYABLE_URI: {
      char const *playable_uri = g_value_get_string (value);
      if (0 != g_strcmp0 (playable_uri, priv->playable_uri)) {
        if (priv->playable_uri) {
          g_free (priv->playable_uri);
          priv->playable_uri = NULL;
        }
        if (playable_uri) {
          priv->playable_uri = g_strdup (playable_uri);
        }
        g_debug ("YtsgVPPlayer.playable-uri = %s", priv->playable_uri);
        g_object_notify (object, "playable-uri");
      }
    } break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  MockPlayerPrivate *priv = GET_PRIVATE (object);

  if (priv->playable_uri) {
    g_free (priv->playable_uri);
    priv->playable_uri = NULL;
  }

  G_OBJECT_CLASS (mock_player_parent_class)->dispose (object);
}

static void
mock_player_class_init (MockPlayerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MockPlayerPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  /* YtsgCapability */

  g_object_class_override_property (object_class,
                                    PROP_CAPABILITY_FQC_ID,
                                    "fqc-id");

  /* YtsgVPPlayer */

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
mock_player_init (MockPlayer *self)
{
  MockPlayerPrivate *priv = GET_PRIVATE (self);
  static char const *_playlist[] = {
    "#1 foo",
    "#2 bar",
    "#3 baz",
    NULL
  };

  priv->playlist = _playlist;
}

MockPlayer *
mock_player_new (void)
{
  return g_object_new (MOCK_TYPE_PLAYER, NULL);
}

