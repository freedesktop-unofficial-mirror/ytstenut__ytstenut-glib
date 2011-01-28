/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#ifndef _YTSG_MESSAGE_H
#define _YTSG_MESSAGE_H

#include <ytstenut-glib/ytsg-metadata.h>

G_BEGIN_DECLS

#define YTSG_TYPE_MESSAGE                                               \
   (ytsg_message_get_type())
#define YTSG_MESSAGE(obj)                                               \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTSG_TYPE_MESSAGE,                      \
                                YtsgMessage))
#define YTSG_MESSAGE_CLASS(klass)                                       \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTSG_TYPE_MESSAGE,                         \
                             YtsgMessageClass))
#define YTSG_IS_MESSAGE(obj)                                            \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTSG_TYPE_MESSAGE))
#define YTSG_IS_MESSAGE_CLASS(klass)                                    \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTSG_TYPE_MESSAGE))
#define YTSG_MESSAGE_GET_CLASS(obj)                                     \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTSG_TYPE_MESSAGE,                       \
                               YtsgMessageClass))

typedef struct _YtsgMessage        YtsgMessage;
typedef struct _YtsgMessageClass   YtsgMessageClass;
typedef struct _YtsgMessagePrivate YtsgMessagePrivate;

struct _YtsgMessageClass
{
  YtsgMetadataClass parent_class;
};

struct _YtsgMessage
{
  YtsgMetadata parent;

  /*<private>*/
  YtsgMessagePrivate *priv;
};

GType ytsg_message_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _YTSG_MESSAGE_H */
