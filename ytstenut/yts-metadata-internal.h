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

#ifndef YTS_METADATA_INTERNAL_H
#define YTS_METADATA_INTERNAL_H

#include <glib.h>
#include <rest/rest-xml-node.h>
#include <ytstenut/yts-metadata.h>

YtsMetadata *
yts_metadata_new_from_xml (const char *xml);

YtsMetadata *
yts_metadata_new_from_node (RestXmlNode       *node,
                            char const *const *attributes);

GHashTable *
yts_metadata_extract (YtsMetadata  *self,
                      char        **body);

#endif /* YTS_METADATA_INTERNAL_H */

