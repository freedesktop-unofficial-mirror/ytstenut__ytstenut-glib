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

#ifndef YTS_VP_CONTENT_H
#define YTS_VP_CONTENT_H

#include <glib-object.h>
#include <ytstenut/video-profile/yts-vp-query.h>

G_BEGIN_DECLS

#define YTS_VP_CONTENT_FQC_ID "org.freedesktop.ytstenut.VideoProfile.Content"

#define YTS_VP_TYPE_CONTENT  (yts_vp_content_get_type ())

#define YTS_VP_CONTENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_VP_TYPE_CONTENT, YtsVPContent))

#define YTS_VP_IS_CONTENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_VP_TYPE_CONTENT))

#define YTS_VP_CONTENT_GET_INTERFACE(obj)                  \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                    \
                                  YTS_VP_TYPE_CONTENT,     \
                                  YtsVPContentInterface))

typedef struct YtsVPContent YtsVPContent;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

  YtsVPQuery *
  (*retrieve) (YtsVPContent  *self,
               char const     *uri);

  YtsVPQuery *
  (*search) (YtsVPContent           *self,
             char const             **tokens,
             YtsVPQueryResultOrder   order,
             unsigned                 max_results);

} YtsVPContentInterface;

GType
yts_vp_content_get_type (void) G_GNUC_CONST;

YtsVPQuery *
yts_vp_content_retrieve (YtsVPContent *self,
                          char const    *uri);

YtsVPQuery *
yts_vp_content_search (YtsVPContent            *self,
                        char const              **tokens,
                        YtsVPQueryResultOrder    order,
                        unsigned                  max_results);

G_END_DECLS

#endif /* YTS_VP_CONTENT_H */

