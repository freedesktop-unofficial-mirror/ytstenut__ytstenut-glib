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

#include "ytsg-service-adapter.h"
#include "ytsg-vp-transcript.h"
#include "ytsg-vp-transcript-adapter.h"

G_DEFINE_TYPE (YtsgVPTranscriptAdapter,
               ytsg_vp_transcript_adapter,
               YTSG_TYPE_SERVICE_ADAPTER)

#define GET_PRIVATE(o)                                            \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o),                              \
                                YTSG_VP_TYPE_TRANSCRIPT_ADAPTER,  \
                                YtsgVPTranscriptAdapterPrivate))

typedef struct {
  YtsgVPTranscript  *transcript;
} YtsgVPTranscriptAdapterPrivate;

/*
 * YtsgServiceAdapter overrides
 */

static GVariant *
_service_adapter_collect_properties (YtsgServiceAdapter *self)
{
  YtsgVPTranscriptAdapterPrivate *priv = GET_PRIVATE (self);
  GVariantBuilder   builder;
  char            **locales;
  char             *current_text;
  char             *locale;

  g_variant_builder_init (&builder, G_VARIANT_TYPE_ARRAY);

  locales = ytsg_vp_transcript_get_available_locales (priv->transcript);
  if (locales) {
    GVariant *v = g_variant_new_strv ((char const *const *) locales, -1);
    g_variant_builder_add (&builder, "{sv}",
                           "available-locales", v);
    g_strfreev (locales);
  }

  current_text = ytsg_vp_transcript_get_current_text (priv->transcript);
  if (current_text) {
    g_variant_builder_add (&builder, "{sv}",
                           "current-text", g_variant_new_string (current_text));
    g_free (current_text);
  }

  locale = ytsg_vp_transcript_get_locale (priv->transcript);
  if (locale) {
    g_variant_builder_add (&builder, "{sv}",
                           "locale", g_variant_new_string (locale));
    g_free (locale);
  }

  return g_variant_builder_end (&builder);
}

static bool
_service_adapter_invoke (YtsgServiceAdapter *self,
                         char const         *invocation_id,
                         char const         *aspect,
                         GVariant           *arguments)
{
  YtsgVPTranscriptAdapterPrivate *priv = GET_PRIVATE (self);
  bool keep_sae = false;

  /* Properties */

  if (0 == g_strcmp0 ("locale", aspect) &&
      arguments &&
      g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    ytsg_vp_transcript_set_locale (priv->transcript,
                                   g_variant_get_string (arguments, NULL));

  } else {

    char *arg_string = arguments ?
                          g_variant_print (arguments, false) :
                          g_strdup ("NULL");
    g_warning ("%s : Unknown method %s.%s(%s)",
               G_STRLOC,
               G_OBJECT_TYPE_NAME (priv->transcript),
               aspect,
               arg_string);
    g_free (arg_string);
  }

  return keep_sae;
}

/*
 * YtsgVPTranscriptAdapter
 */

enum {
  PROP_0 = 0,
  PROP_SERVICE_ADAPTER_SERVICE
};

static void
_transcript_notify_available_locales (YtsgVPTranscript        *transcript,
                                      GParamSpec              *pspec,
                                      YtsgVPTranscriptAdapter *self)
{
  char **locales;
  GVariant *v;

  locales = ytsg_vp_transcript_get_available_locales (transcript);
  v = g_variant_new_strv ((char const *const *) locales, -1);
  ytsg_service_adapter_send_event (YTSG_SERVICE_ADAPTER (self),
                                   "available-locales",
                                   v);
  g_strfreev (locales);
}

static void
_transcript_notify_current_text (YtsgVPTranscript         *transcript,
                                 GParamSpec               *pspec,
                                 YtsgVPTranscriptAdapter  *self)
{
  char *current_text;

  current_text = ytsg_vp_transcript_get_current_text (transcript);
  ytsg_service_adapter_send_event (YTSG_SERVICE_ADAPTER (self),
                                   "current-text",
                                   g_variant_new_string (current_text));
  g_free (current_text);
}

static void
_transcript_notify_locale (YtsgVPTranscript         *transcript,
                           GParamSpec               *pspec,
                           YtsgVPTranscriptAdapter  *self)
{
  char *locale;

  locale = ytsg_vp_transcript_get_locale (transcript);
  ytsg_service_adapter_send_event (YTSG_SERVICE_ADAPTER (self),
                                   "locale",
                                   g_variant_new_string (locale));
  g_free (locale);
}

static void
_transcript_destroyed (YtsgVPTranscriptAdapter  *self,
                       void                     *stale_transcript_ptr)
{
  YtsgVPTranscriptAdapterPrivate *priv = GET_PRIVATE (self);

  priv->transcript = NULL;
  g_object_unref (self);
}

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsgVPTranscriptAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_SERVICE_ADAPTER_SERVICE:
      g_value_set_object (value, priv->transcript);
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
  YtsgVPTranscriptAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    case PROP_SERVICE_ADAPTER_SERVICE:

      /* Construct-only */

      g_return_if_fail (priv->transcript == NULL);
      g_return_if_fail (YTSG_VP_IS_TRANSCRIPT (g_value_get_object (value)));

      priv->transcript = YTSG_VP_TRANSCRIPT (g_value_get_object (value));
      g_object_weak_ref (G_OBJECT (priv->transcript),
                         (GWeakNotify) _transcript_destroyed,
                         object);

      g_signal_connect (priv->transcript, "notify::available-locales",
                        G_CALLBACK (_transcript_notify_available_locales),
                        object);
      g_signal_connect (priv->transcript, "notify::current-text",
                        G_CALLBACK (_transcript_notify_current_text), object);
      g_signal_connect (priv->transcript, "notify::locale",
                        G_CALLBACK (_transcript_notify_locale), object);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsgVPTranscriptAdapterPrivate *priv = GET_PRIVATE (object);

  if (priv->transcript) {
    g_warning ("%s : Adapter disposed with adaptee still referenced.",
               G_STRLOC);
    g_object_weak_unref (G_OBJECT (priv->transcript),
                         (GWeakNotify) _transcript_destroyed,
                         object);
    priv->transcript = NULL;
  }

  G_OBJECT_CLASS (ytsg_vp_transcript_adapter_parent_class)->dispose (object);
}

static void
ytsg_vp_transcript_adapter_class_init (YtsgVPTranscriptAdapterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  YtsgServiceAdapterClass *adapter_class = YTSG_SERVICE_ADAPTER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgVPTranscriptAdapterPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  adapter_class->collect_properties = _service_adapter_collect_properties;
  adapter_class->invoke = _service_adapter_invoke;

  /* Properties */

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_SERVICE,
                                    "service");
}

static void
ytsg_vp_transcript_adapter_init (YtsgVPTranscriptAdapter *self)
{
}

