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

#ifndef _YTSG_CLIENT_H
#define _YTSG_CLIENT_H

#include <glib-object.h>
#include <ytstenut-glib/ytsg-message.h>
#include <ytstenut-glib/ytsg-types.h>
#include <ytstenut-glib/ytsg-caps.h>
#include <telepathy-glib/channel.h>
#include <ytstenut-glib/ytsg-roster.h>

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

  void     (*authenticated) (YtsgClient *client);
  void     (*ready)         (YtsgClient *client);
  void     (*disconnected)  (YtsgClient *client);
  void     (*message)       (YtsgClient *client, YtsgMessage *msg);
  gboolean (*incoming_file) (YtsgClient   *client,
                             const char *from,
                             const char *name,
                             guint64     size,
                             guint64     offset,
                             TpChannel  *channel);

};

struct _YtsgClient
{
  GObject parent;

  /*<private>*/
  YtsgClientPrivate *priv;
};

GType ytsg_client_get_type (void) G_GNUC_CONST;

YtsgClient *ytsg_client_new (YtsgProtocol  protocol,
                             const char   *jid,
                             const char   *resource);

void        ytsg_client_disconnect_from_mesh (YtsgClient *client);
void        ytsg_client_connect_to_mesh (YtsgClient *client);
void        ytsg_client_set_capabilities (YtsgClient *client, YtsgCaps caps);
YtsgRoster *ytsg_client_get_roster (YtsgClient *client);

G_END_DECLS

#endif /* _YTSG_CLIENT_H */
