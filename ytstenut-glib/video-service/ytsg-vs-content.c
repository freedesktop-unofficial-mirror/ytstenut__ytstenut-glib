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

#include "ytsg-vs-content.h"

G_DEFINE_INTERFACE (YtsgVSContent, ytsg_vs_content, G_TYPE_OBJECT)

YtsgVSQuery *
_retrieve (YtsgVSContent  *self,
           char const     *uri)
{
  g_critical ("%s : Method YtsgVSContent.retrieve() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

YtsgVSQuery *
_search (YtsgVSContent           *self,
         char const             **tokens,
         YtsgVSQueryResultOrder   order,
         unsigned int             max_results)
{
  g_critical ("%s : Method YtsgVSContent.search() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

static void
ytsg_vs_content_default_init (YtsgVSContentInterface *interface)
{
  interface->retrieve = _retrieve;
  interface->search = _search;
}

YtsgVSQuery *
ytsg_vs_content_retrieve (YtsgVSContent *self,
                          char const    *uri)
{
  g_return_val_if_fail (YTSG_VS_IS_CONTENT (self), NULL);

  return YTSG_VS_CONTENT_GET_INTERFACE (self)->retrieve (self, uri);
}

YtsgVSQuery *
ytsg_vs_content_search (YtsgVSContent            *self,
                        char const              **tokens,
                        YtsgVSQueryResultOrder    order,
                        unsigned int              max_results)
{
  g_return_val_if_fail (YTSG_VS_IS_CONTENT (self), NULL);

  return YTSG_VS_CONTENT_GET_INTERFACE (self)->search (self,
                                                       tokens,
                                                       order, 
                                                       max_results);
}

