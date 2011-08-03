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

#ifndef YTSG_VS_CONTENT_H
#define YTSG_VS_CONTENT_H

#include <glib-object.h>
#include <ytstenut-glib/ytsg-vs-query.h>

G_BEGIN_DECLS

#define YTSG_VS_TYPE_CONTENT \
  (ytsg_vs_content_get_type ())

#define YTSG_VS_CONTENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_VS_TYPE_CONTENT, YtsgVSContent))

#define YTSG_VS_IS_CONTENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_VS_TYPE_CONTENT))

#define YTSG_VS_CONTENT_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTSG_VS_TYPE_CONTENT, YtsgVSContentInterface))

typedef struct YtsgVSContent YtsgVSContent;
typedef struct YtsgVSContentInterface YtsgVSContentInterface;

struct YtsgVSContentInterface {

  /*< private >*/
  GTypeInterface parent;

  YtsgVSQuery * (*retrieve) (YtsgVSContent  *self,
                             char const     *uri);

  YtsgVSQuery * (*search) (YtsgVSContent           *self,
                           char const             **tokens,
                           YtsgVSQueryResultOrder   order,
                           unsigned int             max_results);
};

GType
ytsg_vs_content_get_type (void) G_GNUC_CONST;

YtsgVSQuery *
ytsg_vs_content_retrieve (YtsgVSContent *self,
                          char const    *uri);

YtsgVSQuery *
ytsg_vs_content_search (YtsgVSContent            *self,
                        char const              **tokens,
                        YtsgVSQueryResultOrder    order,
                        unsigned int              max_results);

G_END_DECLS

#endif /* YTSG_VS_CONTENT_H */

