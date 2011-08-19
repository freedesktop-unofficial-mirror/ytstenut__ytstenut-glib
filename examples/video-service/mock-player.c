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
_player_interface_init (YtsgVSPlayerInterface *interface);

G_DEFINE_TYPE_WITH_CODE (MockPlayer,
                         mock_player,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (YTSG_VS_TYPE_PLAYER,
                                                _player_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOCK_TYPE_PLAYER, MockPlayerPrivate))

enum {
  PROP_0,
  PROP_PLAYER_CAPABILITY,
  PROP_PLAYER_PLAYABLE,
  PROP_PLAYER_PLAYING,
  PROP_PLAYER_VOLUME
};

typedef struct {
  bool    playing;
  double  volume;
} MockPlayerPrivate;

/*
 * YtsgVSPlayer
 */

static void
_player_play (YtsgVSPlayer *self)
{
  g_debug ("YtsgVSPlayer.play()");
  ytsg_vs_player_set_playing (self, true);
}

static void
_player_pause (YtsgVSPlayer *self)
{
  g_debug ("YtsgVSPlayer.pause()");
  ytsg_vs_player_set_playing (self, false);
}

static void
_player_next (YtsgVSPlayer *self)
{
  g_debug ("YtsgVSPlayer.next()");
}

static void
_player_prev (YtsgVSPlayer *self)
{
  g_debug ("YtsgVSPlayer.prev()");
}

static void
_player_interface_init (YtsgVSPlayerInterface *interface)
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
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  MockPlayerPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
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
  MockPlayerPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    case PROP_PLAYER_PLAYABLE:
      /* TODO */
      g_critical ("%s: property MockPlayer.playable not implemented", G_STRLOC);
      break;

    case PROP_PLAYER_PLAYING: {
      bool playing = g_value_get_boolean (value);
      if (playing != priv->playing) {
        g_debug ("YtsgVSPlayer.playing = %s", playing ? "true" : "false");
        priv->playing = playing;
        g_object_notify (object, "playing");
      }
    } break;

    case PROP_PLAYER_VOLUME: {
      double volume = g_value_get_double (value);
      if (volume != priv->volume) {
        g_debug ("YtsgVSPlayer.volume = %.2f", volume);
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

  /* YtsgVSPlayer interface */

  /* Just for default value, no need to handle get/set. */
  g_object_class_override_property (object_class,
                                    PROP_PLAYER_CAPABILITY,
                                    "capability");

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
mock_player_init (MockPlayer *self)
{
}

MockPlayer *
mock_player_new (void)
{
  return g_object_new (MOCK_TYPE_PLAYER, NULL);
}

