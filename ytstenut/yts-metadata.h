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
 * Authored by: Tomas Frydrych <tf@linux.intel.com>
 */

#ifndef YTS_METADATA_H
#define YTS_METADATA_H

#include <glib-object.h>
#include <rest/rest-xml-node.h>

G_BEGIN_DECLS

#define YTS_TYPE_METADATA                                              \
   (yts_metadata_get_type())
#define YTS_METADATA(obj)                                              \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTS_TYPE_METADATA,                     \
                                YtsMetadata))
#define YTS_METADATA_CLASS(klass)                                      \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTS_TYPE_METADATA,                        \
                             YtsMetadataClass))
#define YTS_IS_METADATA(obj)                                           \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTS_TYPE_METADATA))
#define YTS_IS_METADATA_CLASS(klass)                                   \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTS_TYPE_METADATA))
#define YTS_METADATA_GET_CLASS(obj)                                    \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTS_TYPE_METADATA,                      \
                               YtsMetadataClass))

typedef struct _YtsMetadata        YtsMetadata;
typedef struct _YtsMetadataClass   YtsMetadataClass;
typedef struct _YtsMetadataPrivate YtsMetadataPrivate;

/**
 * YtsMetadataClass:
 *
 * #YtsMetadata class.
 */
struct _YtsMetadataClass
{
  /*< private >*/
  GObjectClass parent_class;
};

/**
 * YtsMetadata:
 *
 * Base class for #YtsMessage and #YtsStatus.
 */
struct _YtsMetadata
{
  /*< private >*/
  GObject parent;

  /*< private >*/
  YtsMetadataPrivate *priv;
};

GType yts_metadata_get_type (void) G_GNUC_CONST;

const char  *yts_metadata_get_attribute (YtsMetadata *self, const char *name);
void         yts_metadata_add_attribute (YtsMetadata *self,
                                          const char   *name,
                                          const char   *value);
char        *yts_metadata_to_string (YtsMetadata *self);
gboolean     yts_metadata_is_equal (YtsMetadata *self, YtsMetadata *other);
RestXmlNode *yts_metadata_get_root_node (YtsMetadata *self);

G_END_DECLS

#endif /* YTS_METADATA_H */
