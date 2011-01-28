/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#ifndef _YTSG_SERVICE_H
#define _YTSG_SERVICE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_TYPE_SERVICE                                               \
   (ytsg_service_get_type())
#define YTSG_SERVICE(obj)                                               \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTSG_TYPE_SERVICE,                      \
                                YtsgService))
#define YTSG_SERVICE_CLASS(klass)                                       \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTSG_TYPE_SERVICE,                         \
                             YtsgServiceClass))
#define YTSG_IS_SERVICE(obj)                                            \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTSG_TYPE_SERVICE))
#define YTSG_IS_SERVICE_CLASS(klass)                                    \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTSG_TYPE_SERVICE))
#define YTSG_SERVICE_GET_CLASS(obj)                                     \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTSG_TYPE_SERVICE,                       \
                               YtsgServiceClass))

typedef struct _YtsgService        YtsgService;
typedef struct _YtsgServiceClass   YtsgServiceClass;
typedef struct _YtsgServicePrivate YtsgServicePrivate;

struct _YtsgServiceClass
{
  GObjectClass parent_class;
};

struct _YtsgService
{
  GObject parent;

  /*<private>*/
  YtsgServicePrivate *priv;
};

GType ytsg_service_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _YTSG_SERVICE_H */
