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

#ifndef YTSG_VS_TRANSCRIPT_H
#define YTSG_VS_TRANSCRIPT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_VS_TYPE_TRANSCRIPT \
  (ytsg_vs_transcript_get_type ())

#define YTSG_VS_TRANSCRIPT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VS_TYPE_TRANSCRIPT, YtsgVSTranscript))

#define YTSG_VS_IS_TRANSCRIPT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VS_TYPE_TRANSCRIPT))

#define YTSG_VS_TRANSCRIPT_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTSG_VS_TYPE_TRANSCRIPT, YtsgVSTranscriptInterface))

typedef struct YtsgVSTranscript YtsgVSTranscript;
typedef struct YtsgVSTranscriptInterface YtsgVSTranscriptInterface;

struct YtsgVSTranscriptInterface {

  /*< private >*/
  GTypeInterface parent;
};

GType
ytsg_vs_transcript_get_type (void) G_GNUC_CONST;

GPtrArray *
ytsg_vs_transcript_get_available_locales (YtsgVSTranscript *self);

char *
ytsg_vs_transcript_get_current_text (YtsgVSTranscript *self);

char *
ytsg_vs_transcript_get_locale (YtsgVSTranscript *self);

void
ytsg_vs_transcript_set_locale (YtsgVSTranscript *self,
                               char const       *locale);

G_END_DECLS

#endif /* YTSG_VS_TRANSCRIPT_H */

