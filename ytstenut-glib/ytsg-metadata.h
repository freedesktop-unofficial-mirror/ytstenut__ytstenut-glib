/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#ifndef _YTSG_METADATA_H
#define _YTSG_METADATA_H

#include <glib-object.h>

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

G_END_DECLS

#endif /* _YTSG_METADATA_H */
