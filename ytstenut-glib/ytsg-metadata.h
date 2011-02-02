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

#ifndef _YTSG_METADATA_H
#define _YTSG_METADATA_H

#include <glib-object.h>
#include <rest/rest-xml-node.h>

G_BEGIN_DECLS

#define YTSG_TYPE_METADATA                                              \
   (ytsg_metadata_get_type())
#define YTSG_METADATA(obj)                                              \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTSG_TYPE_METADATA,                     \
                                YtsgMetadata))
#define YTSG_METADATA_CLASS(klass)                                      \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTSG_TYPE_METADATA,                        \
                             YtsgMetadataClass))
#define YTSG_IS_METADATA(obj)                                           \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTSG_TYPE_METADATA))
#define YTSG_IS_METADATA_CLASS(klass)                                   \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTSG_TYPE_METADATA))
#define YTSG_METADATA_GET_CLASS(obj)                                    \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTSG_TYPE_METADATA,                      \
                               YtsgMetadataClass))

typedef struct _YtsgMetadata        YtsgMetadata;
typedef struct _YtsgMetadataClass   YtsgMetadataClass;
typedef struct _YtsgMetadataPrivate YtsgMetadataPrivate;

struct _YtsgMetadataClass
{
  GObjectClass parent_class;
};

struct _YtsgMetadata
{
  GObject parent;

  /*<private>*/
  YtsgMetadataPrivate *priv;
};

GType ytsg_metadata_get_type (void) G_GNUC_CONST;

const char  *ytsg_metadata_get_attribute (YtsgMetadata *self, const char *name);
void         ytsg_metadata_add_attribute (YtsgMetadata *self,
                                          const char   *name,
                                          const char   *value);
char        *ytsg_metadata_print (YtsgMetadata *self);
RestXmlNode *ytsg_metadata_get_top_node (YtsgMetadata *self);

G_END_DECLS

#endif /* _YTSG_METADATA_H */
