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

#ifndef _YTSG_PRIVATE_H
#define _YTSG_PRIVATE_H

#include <ytstenut-glib/ytsg-metadata.h>
#include <ytstenut-glib/ytsg-metadata-service.h>
#include <ytstenut-glib/ytsg-contact.h>
#include <ytstenut-glib/ytsg-client.h>
#include <ytstenut-glib/ytsg-roster.h>

#include <rest/rest-xml-node.h>
#include <telepathy-glib/contact.h>

#define I_(str) (g_intern_static_string ((str)))

G_BEGIN_DECLS

YtsgContact *_ytsg_contact_new (YtsgClient *client, TpContact *tp_contact);

YtsgMetadata *_ytsg_metadata_new_from_xml (const char *xml);
YtsgMetadata *_ytsg_metadata_new_from_node (RestXmlNode  *node,
                                            const char  **attributes);

YtsgService  *_ytsg_metadata_service_new (const char *uid);
void _ytsg_metadata_service_received_status (YtsgMetadataService *service,
                                             const char          *xml);
void _ytsg_metadata_service_received_message (YtsgMetadataService *service,
                                              const char          *xml);

YtsgRoster   *_ytsg_roster_new (void);
void          _ytsg_roster_add_contact (YtsgRoster  *roster,
                                        YtsgContact *contact);
void          _ytsg_roster_remove_contact (YtsgRoster  *roster,
                                           YtsgContact *contact,
                                           gboolean     dispose);
void          _ytsg_roster_clear (YtsgRoster *roster);
void          _ytsg_roster_remove_contact_by_handle (YtsgRoster *roster,
                                                     guint       handle);
YtsgContact  *_ytsg_roster_find_contact_by_handle (YtsgRoster *roster,
                                                   guint       handle);
gboolean      _ytsg_roster_contaiytsg_contact (YtsgRoster        *roster,
                                               const YtsgContact *contact);

G_END_DECLS

#endif /* _YTSG_PRIVATE_H */
