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
#include "ytsg-vp-transcript.h"
#include "ytsg-vp-transcript-proxy.h"

static void
_transcript_interface_init (YtsgVPTranscriptInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsgVPTranscriptProxy,
                         ytsg_vp_transcript_proxy,
                         YTSG_TYPE_PROXY,
                         G_IMPLEMENT_INTERFACE (YTSG_VP_TYPE_TRANSCRIPT,
                                                _transcript_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_VP_TYPE_TRANSCRIPT_PROXY, YtsgVPTranscriptProxyPrivate))

typedef struct {

  /* Properties */
  char    **available_locales;
  char     *current_text;
  char     *locale;

  /* Data */
  GHashTable  *invocations;

} YtsgVPTranscriptProxyPrivate;

/*
 * YtsgVPTranscript implementation
 */

static void
_transcript_interface_init (YtsgVPTranscriptInterface *interface)
{
  /* No methods. */
}

/*
 * YtsgProxy overrides
 */

static void
_proxy_service_event (YtsgProxy   *self,
                      char const  *aspect,
                      GVariant    *arguments)
{
//  YtsgVPTranscriptProxyPrivate *priv = GET_PRIVATE (self);

  if (0 == g_strcmp0 ("available-locales", aspect) &&
      g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING_ARRAY)) {

    /* Read-only property, sync behind the scenes. */
    char const **locales = g_variant_get_strv (arguments, NULL);
    vp_transcript_proxy_set_available_locales (YTSG_VP_TRANSCRIPT_PROXY (self),
                                               locales);
    g_free (locales); /* See g_variant_get_strv(). */

  } else if (0 == g_strcmp0 ("current-text", aspect) &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    /* Read-only property, sync behind the scenes. */
    char const *current_text = g_variant_get_string (arguments, NULL);
    vp_transcript_proxy_set_current_text (YTSG_VP_TRANSCRIPT_PROXY (self),
                                          current_text);

  } else if (0 == g_strcmp0 ("locale", aspect) &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    char const *locale = g_variant_get_string (arguments, NULL);
    ytsg_vp_transcript_set_locale (YTSG_VP_TRANSCRIPT (self), locale);

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
_proxy_service_response (YtsgProxy  *self,
                         char const *invocation_id,
                         GVariant   *response)
{
  /* Transcript doesn't have any methods. */

  // TODO emit general response?
  g_critical ("%s : Response not found for invocation %s",
              G_STRLOC,
              invocation_id);
}

/*
 * YtsgVPTranscriptProxy
 */

enum {
  PROP_0 = 0,

  PROP_CAPABILITY_FQC_IDS,

  PROP_TRANSCRIPT_AVAILABLE_LOCALES,
  PROP_TRANSCRIPT_CURRENT_TEXT,
  PROP_TRANSCRIPT_LOCALE
};

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsgVPTranscriptProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CAPABILITY_FQC_IDS: {
      char *fqc_ids[] = { YTSG_VP_TRANSCRIPT_FQC_ID, NULL };
      g_value_set_boxed (value, fqc_ids);
    } break;
    case PROP_TRANSCRIPT_AVAILABLE_LOCALES:
      g_value_set_boxed (value, priv->available_locales);
      break;
    case PROP_TRANSCRIPT_CURRENT_TEXT:
      g_value_set_string (value, priv->current_text);
      break;
    case PROP_TRANSCRIPT_LOCALE:
      g_value_set_string (value, priv->locale);
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
  YtsgVPTranscriptProxyPrivate *priv = GET_PRIVATE (object);
  char *invocation_id;

  switch (property_id) {

    case PROP_TRANSCRIPT_LOCALE: {
      char const *locale = g_value_get_string (value);
      if (0 != g_strcmp0 (locale, priv->locale)) {
        if (priv->locale) {
          g_free (priv->locale);
          priv->locale = NULL;
        }
        if (locale) {
          priv->locale = g_strdup (locale);
        }
        g_object_notify (object, "locale");
        invocation_id = ytsg_proxy_create_invocation_id (YTSG_PROXY (object));
        ytsg_proxy_invoke (YTSG_PROXY (object), invocation_id,
                           "locale", g_variant_new_string (locale));
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
  YtsgVPTranscriptProxyPrivate *priv = GET_PRIVATE (object);

  if (priv->invocations) {
    g_hash_table_destroy (priv->invocations);
    priv->invocations = NULL;
  }

  G_OBJECT_CLASS (ytsg_vp_transcript_proxy_parent_class)->dispose (object);
}

static void
ytsg_vp_transcript_proxy_class_init (YtsgVPTranscriptProxyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  YtsgProxyClass  *proxy_class = YTSG_PROXY_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgVPTranscriptProxyPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  proxy_class->service_event = _proxy_service_event;
  proxy_class->service_response = _proxy_service_response;

  /* YtsgCapability */

  g_object_class_override_property (object_class,
                                    PROP_CAPABILITY_FQC_IDS,
                                    "fqc-ids");

  /* YtsgVPTranscript */

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
ytsg_vp_transcript_proxy_init (YtsgVPTranscriptProxy *self)
{
  YtsgVPTranscriptProxyPrivate *priv = GET_PRIVATE (self);

  priv->invocations = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             NULL);
}

void
vp_transcript_proxy_set_available_locales (YtsgVPTranscriptProxy  *self,
                                           char const *const      *locales)
{
  YtsgVPTranscriptProxyPrivate *priv = GET_PRIVATE (self);

  /* Internal setter for r/o property to sync the proxy. */

  if (priv->available_locales) {
    g_strfreev (priv->available_locales);
    priv->available_locales = NULL;
  }

  if (locales) {
    priv->available_locales = g_strdupv ((char **) locales);
  }

  g_object_notify (G_OBJECT (self), "available-locales");
}

void
vp_transcript_proxy_set_current_text (YtsgVPTranscriptProxy *self,
                                      char const            *current_text)
{
  YtsgVPTranscriptProxyPrivate *priv = GET_PRIVATE (self);

  /* Internal setter for r/o property to sync the proxy. */

  if (priv->current_text) {
    g_free (priv->current_text);
    priv->current_text = NULL;
  }

  if (current_text) {
    priv->current_text = g_strdup (current_text);
  }

  g_object_notify (G_OBJECT (self), "current-text");
}

