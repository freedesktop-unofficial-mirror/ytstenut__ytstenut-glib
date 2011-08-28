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

#include <stdbool.h>
#include <glib-object.h>
#include <telepathy-glib/channel.h>

#include <ytstenut-glib/ytsg-caps.h>
#include <ytstenut-glib/ytsg-error.h>
#include <ytstenut-glib/ytsg-message.h>
#include <ytstenut-glib/ytsg-roster.h>
#include <ytstenut-glib/ytsg-types.h>
#include <ytstenut-glib/ytsg-status.h>


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

typedef struct _YtsgClientClass   YtsgClientClass;
typedef struct _YtsgClientPrivate YtsgClientPrivate;

/**
 * YtsgClientClass:
 * @authenticated: signal handler for #YtsgClient::authenticated
 * @ready: signal handler for #YtsgClient::ready
 * @disconnected: signal handler for #YtsgClient::disconnected
 * @message: signal handler for #YtsgClient::message
 * @incoming_file: signal handler for #YtsgClient::incoming-file
 *
 * Class for #YtsgClient
 */
struct _YtsgClientClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
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

/**
 * YtsgClient:
 *
 * Class representing an application connection to the Ytstenut mesh.
 */
struct _YtsgClient
{
  /*< private >*/
  GObject parent;

  YtsgClientPrivate *priv;
};

GType ytsg_client_get_type (void) G_GNUC_CONST;

YtsgClient *ytsg_client_new (YtsgProtocol protocol, const char *uid);

void        ytsg_client_disconnect (YtsgClient *client);
void        ytsg_client_connect (YtsgClient *client);
void        ytsg_client_set_capabilities (YtsgClient *client, YtsgCaps caps);
YtsgRoster *ytsg_client_get_roster (YtsgClient *client);
void        ytsg_client_emit_error (YtsgClient *client, YtsgError error);
void        ytsg_client_set_incoming_file_directory (YtsgClient *client,
                                                     const char *directory);
const char *ytsg_client_get_incoming_file_directory (YtsgClient *client);
const char *ytsg_client_get_jid (const YtsgClient *client);
const char *ytsg_client_get_uid (const YtsgClient *client);
void        ytsg_client_set_status (YtsgClient *client, YtsgStatus *status);
void        ytsg_client_set_status_by_capability (YtsgClient *client,
                                                  const char *capability,
                                                  const char *activity);

gboolean
ytsg_client_register_service (YtsgClient  *self,
                              GObject     *service);

/* Protected */

void
ytsg_client_cleanup_contact (YtsgClient         *self,
                             YtsgContact const  *contact);

bool
ytsg_client_get_invocation_proxy (YtsgClient         *self,
                                  char const         *invocation_id,
                                  YtsgContact const **contact,
                                  char const        **proxy_id);

bool
ytsg_client_register_proxy (YtsgClient        *self,
                            char const        *capability,
                            YtsgContact const *contact,
                            char const        *proxy_id);

bool
ytsg_client_unregister_proxy (YtsgClient        *self,
                              char const        *capability,
                              YtsgContact const *contact,
                              char const        *proxy_id);

G_END_DECLS

#endif /* _YTSG_CLIENT_H */

