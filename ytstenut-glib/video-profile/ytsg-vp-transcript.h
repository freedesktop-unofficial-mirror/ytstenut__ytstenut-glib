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

#ifndef YTSG_VP_TRANSCRIPT_H
#define YTSG_VP_TRANSCRIPT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_VP_TRANSCRIPT_CAPABILITY \
  "org.freedesktop.ytstenut.VideoProfile.Transcript"

#define YTSG_VP_TYPE_TRANSCRIPT (ytsg_vp_transcript_get_type ())

#define YTSG_VP_TRANSCRIPT(obj)                         \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               YTSG_VP_TYPE_TRANSCRIPT, \
                               YtsgVPTranscript))

#define YTSG_VP_IS_TRANSCRIPT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VP_TYPE_TRANSCRIPT))

#define YTSG_VP_TRANSCRIPT_GET_INTERFACE(obj)                 \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                      \
                                  YTSG_VP_TYPE_TRANSCRIPT,    \
                                  YtsgVPTranscriptInterface))

typedef struct YtsgVPTranscript YtsgVPTranscript;

typedef struct  {

  /*< private >*/
  GTypeInterface parent;

} YtsgVPTranscriptInterface;

GType
ytsg_vp_transcript_get_type (void) G_GNUC_CONST;

GPtrArray *
ytsg_vp_transcript_get_available_locales (YtsgVPTranscript *self);

char *
ytsg_vp_transcript_get_current_text (YtsgVPTranscript *self);

char *
ytsg_vp_transcript_get_locale (YtsgVPTranscript *self);

void
ytsg_vp_transcript_set_locale (YtsgVPTranscript *self,
                               char const       *locale);

G_END_DECLS

#endif /* YTSG_VP_TRANSCRIPT_H */

