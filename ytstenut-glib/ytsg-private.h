/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2011 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _YTSG_PRIVATE_H
#define _YTSG_PRIVATE_H

#include <ytstenut-glib/ytsg-metadata.h>
#include <rest/rest-xml-node.h>

G_BEGIN_DECLS

YtsgMetadata *_ytsg_metadata_new_from_xml (const char *xml);
YtsgMetadata *_ytsg_metadata_new_from_node (RestXmlNode *node);

G_END_DECLS

#endif /* _YTSG_PRIVATE_H */
