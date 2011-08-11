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

#include "ytsg-vs-transcript.h"

G_DEFINE_INTERFACE (YtsgVSTranscript, ytsg_vs_transcript, G_TYPE_OBJECT)

static void
ytsg_vs_transcript_default_init (YtsgVSTranscriptInterface *interface)
{
  /* Only to hold the default value */
  g_object_interface_install_property (interface,
                                       g_param_spec_string ("capability", "", "",
                                                            YTSG_VS_TRANSCRIPT_CAPABILITY,
                                                            G_PARAM_STATIC_NAME |
                                                            G_PARAM_STATIC_NICK |
                                                            G_PARAM_STATIC_BLURB));

  g_object_interface_install_property (interface,
                                       g_param_spec_boxed ("available-locales", "", "",
                                                           G_TYPE_PTR_ARRAY,
                                                           G_PARAM_READABLE));

  g_object_interface_install_property (interface,
                                       g_param_spec_string ("current-text", "", "",
                                                            NULL,
                                                            G_PARAM_READABLE));

  g_object_interface_install_property (interface,
                                       g_param_spec_string ("locale", "", "",
                                                            NULL,
                                                            G_PARAM_READABLE));
}

GPtrArray *
ytsg_vs_transcript_get_available_locales (YtsgVSTranscript *self)
{
  GPtrArray *available_locales;

  g_return_val_if_fail (YTSG_VS_IS_TRANSCRIPT (self), NULL);

  available_locales = NULL;
  g_object_get (G_OBJECT (self), "available-locales", &available_locales, NULL);
  return available_locales;
}

char *
ytsg_vs_transcript_get_current_text (YtsgVSTranscript *self)
{
  char *current_text;

  g_return_val_if_fail (YTSG_VS_IS_TRANSCRIPT (self), NULL);

  current_text = NULL;
  g_object_get (G_OBJECT (self), "current-text", &current_text, NULL);
  return current_text;
}

char *
ytsg_vs_transcript_get_locale (YtsgVSTranscript *self)
{
  char *locale;

  g_return_val_if_fail (YTSG_VS_IS_TRANSCRIPT (self), NULL);

  locale = NULL;
  g_object_get (G_OBJECT (self), "locale", &locale, NULL);
  return locale;
}

void
ytsg_vs_transcript_set_locale (YtsgVSTranscript *self,
                               char const       *locale)
{
  g_return_if_fail (YTSG_VS_IS_TRANSCRIPT (self));

  g_object_set (G_OBJECT (self), "locale", locale, NULL);
}

