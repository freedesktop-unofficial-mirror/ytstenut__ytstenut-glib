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

#ifndef YTS_PRIVATE_H
#define YTS_PRIVATE_H

#include <rest/rest-xml-node.h>
#include <telepathy-glib/contact.h>
#include <telepathy-ytstenut-glib/telepathy-ytstenut-glib.h>

#include <ytstenut/yts-client.h>
#include <ytstenut/yts-contact.h>
#include <ytstenut/yts-error.h>
#include <ytstenut/yts-metadata.h>
#include <ytstenut/yts-metadata-service.h>
#include <ytstenut/yts-roster.h>
#include <ytstenut/yts-types.h>

#define I_(str) (g_intern_static_string ((str)))

G_BEGIN_DECLS

YtsContact *_yts_contact_new (YtsClient *client, const char *jid);
void         _yts_contact_add_service (YtsContact *contact,
                                        YtsService *service);
void         _yts_contact_remove_service_by_uid (YtsContact *contact,
                                                  const char *uid);
gboolean     _yts_contact_is_empty (YtsContact *contact);

YtsMetadata *_yts_metadata_new_from_xml (const char *xml);
YtsMetadata *_yts_metadata_new_from_node (RestXmlNode  *node,
                                            const char  **attributes);
GHashTable   *_yts_metadata_extract (YtsMetadata *self, char **body);

YtsService *_yts_metadata_service_new (YtsContact  *contact,
                                         const char   *uid,
                                         const char   *type,
                                         const char  **caps,
                                         GHashTable   *names);

void _yts_metadata_service_received_status (YtsMetadataService *service,
                                             const char          *xml);
void
_yts_metadata_service_received_message (YtsMetadataService *service,
                                         const char          *xml);

YtsRoster   *_yts_roster_new (YtsClient *client);

void          _yts_roster_add_contact (YtsRoster  *roster,
                                        YtsContact *contact);
void          _yts_roster_remove_contact (YtsRoster  *roster,
                                           YtsContact *contact,
                                           gboolean     dispose);
void          _yts_roster_add_service (YtsRoster  *roster,
                                        const char  *jid,
                                        const char  *sid,
                                        const char  *type,
                                        const char **caps,
                                        GHashTable  *names);
void         _yts_roster_remove_service_by_id (YtsRoster *roster,
                                                const char *jid,
                                                const char *uid);

void          _yts_roster_clear (YtsRoster *roster);
YtsContact  *_yts_roster_find_contact_by_handle (YtsRoster *roster,
                                                   guint       handle);
void           _yts_contact_set_ft_channel (YtsContact *item,
                                             TpChannel *channel);

void          _yts_client_reconnect_after (YtsClient *client,
                                            guint after_seconds);
TpConnection *_yts_client_get_connection (YtsClient *client);
TpYtsStatus  *_yts_client_get_tp_status (YtsClient *client);
YtsError     _yts_client_send_message (YtsClient  *client,
                                         YtsContact *contact,
                                         const char  *uid,
                                         YtsMetadata *message);

G_END_DECLS

#endif /* YTS_PRIVATE_H */
