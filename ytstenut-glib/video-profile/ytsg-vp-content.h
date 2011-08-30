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

#ifndef YTSG_VP_CONTENT_H
#define YTSG_VP_CONTENT_H

#include <glib-object.h>
#include <ytstenut-glib/video-profile/ytsg-vp-query.h>

G_BEGIN_DECLS

#define YTSG_VP_CONTENT_CAPABILITY  \
  "org.freedesktop.ytstenut.VideoProfile.Content"

#define YTSG_VP_TYPE_CONTENT  (ytsg_vp_content_get_type ())

#define YTSG_VP_CONTENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VP_TYPE_CONTENT, YtsgVPContent))

#define YTSG_VP_IS_CONTENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VP_TYPE_CONTENT))

#define YTSG_VP_CONTENT_GET_INTERFACE(obj)                  \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                    \
                                  YTSG_VP_TYPE_CONTENT,     \
                                  YtsgVPContentInterface))

typedef struct YtsgVPContent YtsgVPContent;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

  YtsgVPQuery *
  (*retrieve) (YtsgVPContent  *self,
               char const     *uri);

  YtsgVPQuery *
  (*search) (YtsgVPContent           *self,
             char const             **tokens,
             YtsgVPQueryResultOrder   order,
             unsigned                 max_results);

} YtsgVPContentInterface;

GType
ytsg_vp_content_get_type (void) G_GNUC_CONST;

YtsgVPQuery *
ytsg_vp_content_retrieve (YtsgVPContent *self,
                          char const    *uri);

YtsgVPQuery *
ytsg_vp_content_search (YtsgVPContent            *self,
                        char const              **tokens,
                        YtsgVPQueryResultOrder    order,
                        unsigned                  max_results);

G_END_DECLS

#endif /* YTSG_VP_CONTENT_H */

