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

#include <ytstenut/ytstenut.h>
#include "mock-player.h"

static void
_capability_interface_init (YtsCapability *interface);

static void
_player_interface_init (YtsVPPlayerInterface *interface);

static void
_transcript_interface_init (YtsVPPlayerInterface *interface);

G_DEFINE_TYPE_WITH_CODE (MockPlayer,
                         mock_player,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (YTS_TYPE_CAPABILITY,
                                                _capability_interface_init)
                         G_IMPLEMENT_INTERFACE (YTS_VP_TYPE_PLAYER,
                                                _player_interface_init)
                         G_IMPLEMENT_INTERFACE (YTS_VP_TYPE_TRANSCRIPT,
                                                _transcript_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOCK_TYPE_PLAYER, MockPlayerPrivate))

enum {
  PROP_0,

  /* YtsCapability */
  PROP_CAPABILITY_FQC_IDS,

  /* YtsVPPlayer */
  PROP_PLAYER_PLAYABLE,
  PROP_PLAYER_PLAYING,
  PROP_PLAYER_VOLUME,
  PROP_PLAYER_PLAYABLE_URI,

  /* YtsVPTranscript */
  PROP_TRANSCRIPT_AVAILABLE_LOCALES,
  PROP_TRANSCRIPT_CURRENT_TEXT,
  PROP_TRANSCRIPT_LOCALE
};

typedef struct {

  /* YtsVPPlayer */

  char const *const *playlist;
  unsigned           current;
  bool               playing;
  double             volume;
  char              *playable_uri;

  /* YtsVPTranscript */

  char const *const *available_locales;
  char              *current_text;
  unsigned           locale_idx;

} MockPlayerPrivate;

/*
 * YtsCapability implementation
 */

static void
_capability_interface_init (YtsCapability *interface)
{
  /* Nothing to do, it's just about overriding the "fqc-id" property */
}

/*
 * YtsVPPlayer
 */

static void
_player_play (YtsVPPlayer *self)
{
  MockPlayerPrivate *priv = GET_PRIVATE (self);

  g_debug ("YtsVPPlayer.play() with playing=%s",
           priv->playing ? "true" : "false");

  /* Let the property setter do the work. */
  yts_vp_player_set_playing (self, true);
}

static void
_player_pause (YtsVPPlayer *self)
{
  MockPlayerPrivate *priv = GET_PRIVATE (self);

  g_debug ("YtsVPPlayer.pause() with playing=%s",
           priv->playing ? "true" : "false");

  /* Let the property setter do the work. */
  yts_vp_player_set_playing (self, false);
}

static void
_player_next (YtsVPPlayer  *self,
              char const    *invocation_id)
{
  MockPlayerPrivate *priv = GET_PRIVATE (self);
  char const  *next;

  next = priv->playlist[priv->current + 1];
  if (next) {
    priv->current++;
  }

  g_debug ("YtsVPPlayer.next() -- %s", next);

  /* Return true if we skipped to the next item in the playlist. */
  yts_vp_player_next_return (self, invocation_id, (bool) next);
}

static void
_player_prev (YtsVPPlayer  *self,
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

  g_debug ("YtsVPPlayer.prev() -- %s", prev);

  /* Return true if we skipped to the previous item in the playlist. */
  yts_vp_player_prev_return (self, invocation_id, (bool) prev);
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
 * YtsVPTranscript
 */

static bool
_transcript_emit_text (MockPlayer *self)
{
  static unsigned _counter = 0;

  MockPlayerPrivate *priv = GET_PRIVATE (self);

  if (priv->playing == false) {
    /* Stop timer */
    return false;
  }

  if (priv->current_text) {
    g_free (priv->current_text);
    priv->current_text = NULL;
  }

  priv->current_text = g_strdup_printf ("text %i", _counter);
  g_object_notify (G_OBJECT (self), "current-text");

  _counter++;

  /* Keep running. */
  return true;
}

static void
_transcript_interface_init (YtsVPPlayerInterface *interface)
{
  /* No methods to override. */
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

    /*
     * YtsCapability
     */

    case PROP_CAPABILITY_FQC_IDS: {
      char *fcq_ids[] = {
        YTS_VP_PLAYER_FQC_ID,
        YTS_VP_TRANSCRIPT_FQC_ID,
        NULL };
      g_value_set_boxed (value, fcq_ids);
    } break;

    /*
     * YtsVPPlayer
     */

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

    /*
     * YtsVPTranscript
     */

    case PROP_TRANSCRIPT_AVAILABLE_LOCALES:
      g_value_set_boxed (value, (char **) priv->available_locales);
      break;
    case PROP_TRANSCRIPT_CURRENT_TEXT:
      g_value_set_string (value, priv->current_text);
      break;
    case PROP_TRANSCRIPT_LOCALE:
      g_value_set_string (value, priv->available_locales[priv->locale_idx]);
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

    /*
     * YtsVPPlayer
     */

    case PROP_PLAYER_PLAYABLE:
      /* TODO */
      g_critical ("%s: property MockPlayer.playable not implemented", G_STRLOC);
      break;

    case PROP_PLAYER_PLAYING: {
      bool playing = g_value_get_boolean (value);
      if (playing != priv->playing) {
        g_debug ("YtsVPPlayer.playing = %s", playing ? "true" : "false");
        priv->playing = playing;
        g_object_notify (object, "playing");

        if (priv->playing) {
          /* Timer to fake a stream of subtitles. */
          g_timeout_add_seconds (3,
                                 (GSourceFunc) _transcript_emit_text,
                                 object);
        }
      }
    } break;

    case PROP_PLAYER_VOLUME: {
      double volume = g_value_get_double (value);
      if (volume != priv->volume) {
        g_debug ("YtsVPPlayer.volume = %.2f", volume);
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
        g_debug ("YtsVPPlayer.playable-uri = %s", priv->playable_uri);
        g_object_notify (object, "playable-uri");
      }
    } break;

    /*
     * YtsVPTranscript
     */

    case PROP_TRANSCRIPT_LOCALE: {

      char const *locale = g_value_get_string (value);
      unsigned i;
      int locale_idx = -1;
      for (i = 0; priv->available_locales[i]; i++) {
        if (0 == g_strcmp0 (locale, priv->available_locales[i])) {
          locale_idx = i;
          break;
        }
      }

      if (locale_idx < 0) {
        g_warning ("%s : Locale %s not available.",
                   G_STRLOC,
                   locale);
        /* TODO emit error. */

      } else if (locale_idx != priv->locale_idx) {

        priv->locale_idx = locale_idx;
        g_debug ("YtsVPTranscript.locale = %s",
                 priv->available_locales[priv->locale_idx]);
        g_object_notify (object, "locale");
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

  /* YtsVPTranscript */

  g_object_class_override_property (object_class,
                                    PROP_TRANSCRIPT_AVAILABLE_LOCALES,
                                    "available-locales");

  g_object_class_override_property (object_class,
                                    PROP_TRANSCRIPT_CURRENT_TEXT,
                                    "current-text");

  g_object_class_override_property (object_class,
                                    PROP_TRANSCRIPT_LOCALE,
                                    "locale");
}

static void
mock_player_init (MockPlayer *self)
{
  MockPlayerPrivate *priv = GET_PRIVATE (self);
  static char const *const _playlist[] = {
    "#1 foo",
    "#2 bar",
    "#3 baz",
    NULL
  };

  static char const *const _locales[] = {
    "en_GB",
    "de_AT",
    NULL
  };

  priv->playlist = _playlist;
  priv->available_locales = _locales;
}

MockPlayer *
mock_player_new (void)
{
  return g_object_new (MOCK_TYPE_PLAYER, NULL);
}

