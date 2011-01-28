/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#ifndef _YTSG_CLIENT_H
#define _YTSG_CLIENT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_TYPE_CLIENT                                                \
   (ytsg_client_get_type())
#define YTSG_CLIENT(obj)                                                \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTSG_TYPE_CLIENT,                       \
                                YtsgClient))
#define YTSG_CLIENT_CLASS(klass)                                        \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTSG_TYPE_CLIENT,                          \
                             YtsgClientClass))
#define YTSG_IS_CLIENT(obj)                                             \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTSG_TYPE_CLIENT))
#define YTSG_IS_CLIENT_CLASS(klass)                                     \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTSG_TYPE_CLIENT))
#define YTSG_CLIENT_GET_CLASS(obj)                                      \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTSG_TYPE_CLIENT,                        \
                               YtsgClientClass))

typedef struct _YtsgClient        YtsgClient;
typedef struct _YtsgClientClass   YtsgClientClass;
typedef struct _YtsgClientPrivate YtsgClientPrivate;

struct _YtsgClientClass
{
  GObjectClass parent_class;
};

struct _YtsgClient
{
  GObject parent;

  /*<private>*/
  YtsgClientPrivate *priv;
};

GType ytsg_client_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _YTSG_CLIENT_H */
