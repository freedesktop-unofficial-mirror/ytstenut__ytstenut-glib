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

#ifndef YTS_CLIENT_H
#define YTS_CLIENT_H

#include <glib-object.h>
#include <telepathy-glib/channel.h>

#include <ytstenut/yts-capability.h>
#include <ytstenut/yts-error.h>
#include <ytstenut/yts-roster.h>
#include <ytstenut/yts-types.h>

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

typedef struct {

  /*< private >*/
  GObjectClass parent;

  /*< public >*/
  void     
  (*authenticated) (YtsClient *self);

  void     
  (*ready) (YtsClient *self);

  void     
  (*disconnected) (YtsClient *self);

  void     
  (*raw_message) (YtsClient   *self,
                  char const  *xml_payload);

  bool
  (*incoming_file) (YtsClient   *self,
                    const char  *from,
                    const char  *name,
                    guint64      size,
                    guint64      offset,
                    TpChannel   *channel);
} YtsClientClass;

GType
yts_client_get_type (void) G_GNUC_CONST;

YtsClient *
yts_client_new (YtsProtocol  protocol,
                const char  *uid);

void
yts_client_disconnect (YtsClient *self);

void
yts_client_connect (YtsClient *self);

void
yts_client_add_capability (YtsClient  *self,
                           char const *capability);

YtsRoster *
yts_client_get_roster (YtsClient *self);

void
yts_client_emit_error (YtsClient *self,
                       YtsError error);

void
yts_client_set_incoming_file_directory (YtsClient *self,
                                        const char *directory);
                                                     
const char *
yts_client_get_incoming_file_directory (YtsClient *self);

const char *
yts_client_get_jid (const YtsClient *self);

const char *
yts_client_get_uid (const YtsClient *self);

void
yts_client_set_status_by_capability (YtsClient *self,
                                     const char *capability,
                                     const char *activity);

gboolean
yts_client_register_service (YtsClient      *self,
                              YtsCapability  *service);

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

