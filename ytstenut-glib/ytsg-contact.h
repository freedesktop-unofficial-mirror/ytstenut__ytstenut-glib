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

#ifndef _YTSG_CONTACT_H
#define _YTSG_CONTACT_H

#include <glib-object.h>
#include <gio/gio.h>
#include <ytstenut-glib/ytsg-service.h>
#include <telepathy-glib/contact.h>

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

  void (*service_added)   (YtsgContact *contact, YtsgService *service);
  void (*service_removed) (YtsgContact *contact, YtsgService *service);
};

struct _YtsgContact
{
  GObject parent;

  /*<private>*/
  YtsgContactPrivate *priv;
};

GType ytsg_contact_get_type (void) G_GNUC_CONST;

const char *ytsg_contact_get_jid            (const YtsgContact  *contact);
const char *ytsg_contact_get_name           (const YtsgContact  *contact);
TpContact  *ytsg_contact_get_tp_contact     (const YtsgContact  *contact);
GFile      *ytsg_contact_get_icon           (const YtsgContact  *contact,
                                             const char        **mime);

G_END_DECLS

#endif /* _YTSG_CONTACT_H */
