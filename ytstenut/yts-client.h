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
 *              Rob Staudinger <robsta@linux.intel.com>
 */

#ifndef YTS_CLIENT_H
#define YTS_CLIENT_H

#include <stdint.h>
#include <glib-object.h>
#include <ytstenut/yts-capability.h>
#include <ytstenut/yts-roster.h>

G_BEGIN_DECLS

#define YTS_TYPE_CLIENT (yts_client_get_type())

#define YTS_CLIENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_CLIENT, YtsClient))

#define YTS_CLIENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_CLIENT, YtsClientClass))

#define YTS_IS_CLIENT(obj) \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_CLIENT))

#define YTS_IS_CLIENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_CLIENT))

#define YTS_CLIENT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_CLIENT, YtsClientClass))

typedef struct {

  /*< private >*/
  GObject parent;

} YtsClient;

/**
 * YtsClientClass:
 * @authenticated: virtual function for the #YtsClient::authenticated signal.
 * @disconnected: virtual function for the #YtsClient::disconnected signal.
 * @incoming_file: virtual function for the #YtsClient::incoming-file signal.
 * @raw_message: virtual function for the #YtsClient::raw-message signal.
 * @ready: virtual function for the #YtsClient::ready signal.
 *
 * Deprecated: the class handlers for signals are deprecated and will be
 *             removed in 0.4.
 * Since: 0.1
 */
typedef struct {

  /*< private >*/
  GObjectClass parent;

  /*< public >*/

  void
  (*authenticated) (YtsClient *self);

  void
  (*disconnected) (YtsClient *self);

  void
  (*raw_message) (YtsClient   *self,
                  char const  *xml_payload);

  void
  (*ready) (YtsClient *self);

} YtsClientClass;

GType
yts_client_get_type (void) G_GNUC_CONST;

/**
 * YtsProtocol:
 * @YTS_PROTOCOL_XMPP: Jabber
 * @YTS_PROTOCOL_LOCAL_XMPP: XEP-0174 serverless messaging.
 *
 * YtsProtocol represents the xmpp protocol to use
 */
typedef enum { /*< prefix=YTS_PROTOCOL >*/
  YTS_PROTOCOL_XMPP = 0,
  YTS_PROTOCOL_LOCAL_XMPP
} YtsProtocol;

YtsClient *
yts_client_new (YtsProtocol  protocol,
                char const  *service_id);

void
yts_client_disconnect (YtsClient *self);

void
yts_client_connect (YtsClient *self);

void
yts_client_add_capability (YtsClient  *self,
                           char const *capability);

YtsRoster *const
yts_client_get_roster (YtsClient const *self);

void
yts_client_set_incoming_file_directory (YtsClient   *self,
                                        char const  *directory);

char const *
yts_client_get_incoming_file_directory (YtsClient const *self);

char const *
yts_client_get_jid (YtsClient const *self);

char const *
yts_client_get_uid (YtsClient const *self);

void
yts_client_set_status_by_capability (YtsClient *self,
                                     char const *capability,
                                     char const *activity);

bool
yts_client_register_service (YtsClient      *self,
                             YtsCapability  *service);

/**
 * YtsClientServiceIterator:
 * @self: object owning @service.
 * @fqc_id: capability ID.
 * @service: service implementation.
 * @user_data: data passed to yts_client_foreach_service().
 *
 * Callback signature for iterating a an #YtsClient's published services.
 *
 * Since: 0.3
 */
typedef void
(*YtsClientServiceIterator) (YtsClient      *self,
                             char const     *fqc_id,
                             YtsCapability  *service,
                             void           *user_data);

void
yts_client_foreach_service (YtsClient                 *self,
                            YtsClientServiceIterator   callback,
                            void                      *user_data);

G_END_DECLS

#endif /* YTS_CLIENT_H */

