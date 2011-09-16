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

#include "yts-vp-transcript.h"
#include "yts-vp-transcript-proxy.h"
#include "config.h"

static void
_transcript_interface_init (YtsVPTranscriptInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsVPTranscriptProxy,
                         yts_vp_transcript_proxy,
                         YTS_TYPE_PROXY,
                         G_IMPLEMENT_INTERFACE (YTS_VP_TYPE_TRANSCRIPT,
                                                _transcript_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_VP_TYPE_TRANSCRIPT_PROXY, YtsVPTranscriptProxyPrivate))

typedef struct {

  /* Properties */
  char    **available_locales;
  char     *current_text;
  char     *locale;

  /* Data */
  GHashTable  *invocations;

} YtsVPTranscriptProxyPrivate;

static void
transcript_proxy_set_available_locales (YtsVPTranscriptProxy *self,
                                        char const *const     *locales);
static void
transcript_proxy_set_current_text (YtsVPTranscriptProxy  *self,
                                   char const             *current_text);
static void
transcript_proxy_set_locale (YtsVPTranscriptProxy  *self,
                             char const             *locale);

/*
 * YtsVPTranscript implementation
 */

static void
_transcript_interface_init (YtsVPTranscriptInterface *interface)
{
  /* No methods. */
}

/*
 * YtsProxy overrides
 */

static void
_proxy_service_event (YtsProxy   *self,
                      char const  *aspect,
                      GVariant    *arguments)
{
//  YtsVPTranscriptProxyPrivate *priv = GET_PRIVATE (self);

  if (0 == g_strcmp0 ("available-locales", aspect) &&
      g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING_ARRAY)) {

    /* Read-only property, sync behind the scenes. */
    char const **locales = g_variant_get_strv (arguments, NULL);
    transcript_proxy_set_available_locales (YTS_VP_TRANSCRIPT_PROXY (self),
                                            locales);
    g_free (locales); /* See g_variant_get_strv(). */

  } else if (0 == g_strcmp0 ("current-text", aspect) &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    /* Read-only property, sync behind the scenes. */
    char const *current_text = g_variant_get_string (arguments, NULL);
    transcript_proxy_set_current_text (YTS_VP_TRANSCRIPT_PROXY (self),
                                       current_text);

  } else if (0 == g_strcmp0 ("locale", aspect) &&
             g_variant_is_of_type (arguments, G_VARIANT_TYPE_STRING)) {

    char const *locale = g_variant_get_string (arguments, NULL);
    transcript_proxy_set_locale (YTS_VP_TRANSCRIPT_PROXY (self), locale);

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
  /* Transcript doesn't have any methods. */

  // TODO emit general response?
  g_critical ("%s : Response not found for invocation %s",
              G_STRLOC,
              invocation_id);
}

/*
 * YtsVPTranscriptProxy
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
  YtsVPTranscriptProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CAPABILITY_FQC_IDS: {
      char const *fqc_ids[] = { YTS_VP_TRANSCRIPT_FQC_ID, NULL };
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
  YtsVPTranscriptProxyPrivate *priv = GET_PRIVATE (object);
  char *invocation_id;

  switch (property_id) {

    case PROP_TRANSCRIPT_LOCALE: {

      char const *locale = g_value_get_string (value);
      if (0 != g_strcmp0 (locale, priv->locale)) {
        invocation_id = yts_proxy_create_invocation_id (YTS_PROXY (object));
        yts_proxy_invoke (YTS_PROXY (object), invocation_id,
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
  YtsVPTranscriptProxyPrivate *priv = GET_PRIVATE (object);

  if (priv->invocations) {
    g_hash_table_destroy (priv->invocations);
    priv->invocations = NULL;
  }

  G_OBJECT_CLASS (yts_vp_transcript_proxy_parent_class)->dispose (object);
}

static void
yts_vp_transcript_proxy_class_init (YtsVPTranscriptProxyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  YtsProxyClass  *proxy_class = YTS_PROXY_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsVPTranscriptProxyPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  proxy_class->service_event = _proxy_service_event;
  proxy_class->service_response = _proxy_service_response;

  /* YtsCapability */

  g_object_class_override_property (object_class,
                                    PROP_CAPABILITY_FQC_IDS,
                                    "fqc-ids");

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
yts_vp_transcript_proxy_init (YtsVPTranscriptProxy *self)
{
  YtsVPTranscriptProxyPrivate *priv = GET_PRIVATE (self);

  priv->invocations = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             NULL);
}

static void
transcript_proxy_set_available_locales (YtsVPTranscriptProxy *self,
                                        char const *const     *locales)
{
  YtsVPTranscriptProxyPrivate *priv = GET_PRIVATE (self);

  /* Assume the value has been validated by the service. */

  if (priv->available_locales) {
    g_strfreev (priv->available_locales);
    priv->available_locales = NULL;
  }

  if (locales) {
    priv->available_locales = g_strdupv ((char **) locales);
  }

  g_object_notify (G_OBJECT (self), "available-locales");
}

static void
transcript_proxy_set_current_text (YtsVPTranscriptProxy  *self,
                                   char const             *current_text)
{
  YtsVPTranscriptProxyPrivate *priv = GET_PRIVATE (self);

  /* Assume the value has been validated by the service. */

  if (0 != g_strcmp0 (current_text, priv->current_text)) {

    if (priv->current_text) {
      g_free (priv->current_text);
      priv->current_text = NULL;
    }

    if (current_text) {
      priv->current_text = g_strdup (current_text);
    }

    g_object_notify (G_OBJECT (self), "current-text");
  }
}

static void
transcript_proxy_set_locale (YtsVPTranscriptProxy  *self,
                             char const             *locale)
{
  YtsVPTranscriptProxyPrivate *priv = GET_PRIVATE (self);

  /* Assume the value has been validated by the service. */

  if (0 != g_strcmp0 (locale, priv->locale)) {

    if (priv->locale) {
      g_free (priv->locale);
      priv->locale = NULL;
    }

    if (locale) {
      priv->locale = g_strdup (locale);
    }

    g_object_notify (G_OBJECT (self), "locale");
  }
}

