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

#include "yts-capability.h"
#include "yts-vp-content.h"

G_DEFINE_INTERFACE (YtsVPContent,
                    yts_vp_content,
                    YTS_TYPE_CAPABILITY)

YtsVPQuery *
_retrieve (YtsVPContent  *self,
           char const     *uri)
{
  g_critical ("%s : Method YtsVPContent.retrieve() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

YtsVPQuery *
_search (YtsVPContent           *self,
         char const             **tokens,
         YtsVPQueryResultOrder   order,
         unsigned                 max_results)
{
  g_critical ("%s : Method YtsVPContent.search() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
  return NULL;
}

static void
yts_vp_content_default_init (YtsVPContentInterface *interface)
{
  interface->retrieve = _retrieve;
  interface->search = _search;
}

YtsVPQuery *
yts_vp_content_retrieve (YtsVPContent *self,
                          char const    *uri)
{
  g_return_val_if_fail (YTS_VP_IS_CONTENT (self), NULL);

  return YTS_VP_CONTENT_GET_INTERFACE (self)->retrieve (self, uri);
}

YtsVPQuery *
yts_vp_content_search (YtsVPContent            *self,
                        char const              **tokens,
                        YtsVPQueryResultOrder    order,
                        unsigned                  max_results)
{
  g_return_val_if_fail (YTS_VP_IS_CONTENT (self), NULL);

  return YTS_VP_CONTENT_GET_INTERFACE (self)->search (self,
                                                       tokens,
                                                       order, 
                                                       max_results);
}

