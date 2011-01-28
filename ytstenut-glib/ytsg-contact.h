/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#ifndef _YTSG_CONTACT_H
#define _YTSG_CONTACT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_TYPE_CONTACT                                               \
   (ytsg_contact_get_type())
#define YTSG_CONTACT(obj)                                               \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTSG_TYPE_CONTACT,                      \
                                YtsgContact))
#define YTSG_CONTACT_CLASS(klass)                                       \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTSG_TYPE_CONTACT,                         \
                             YtsgContactClass))
#define YTSG_IS_CONTACT(obj)                                            \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTSG_TYPE_CONTACT))
#define YTSG_IS_CONTACT_CLASS(klass)                                    \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTSG_TYPE_CONTACT))
#define YTSG_CONTACT_GET_CLASS(obj)                                     \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTSG_TYPE_CONTACT,                       \
                               YtsgContactClass))

typedef struct _YtsgContact        YtsgContact;
typedef struct _YtsgContactClass   YtsgContactClass;
typedef struct _YtsgContactPrivate YtsgContactPrivate;

struct _YtsgContactClass
{
  GObjectClass parent_class;
};

struct _YtsgContact
{
  GObject parent;

  /*<private>*/
  YtsgContactPrivate *priv;
};

GType ytsg_contact_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _YTSG_CONTACT_H */
