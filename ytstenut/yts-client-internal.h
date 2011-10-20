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
 * Authored by: Rob Staudinger <robsta@linux.intel.com>
 */

#ifndef YTS_CLIENT_INTERNAL_H
#define YTS_CLIENT_INTERNAL_H

#include <telepathy-ytstenut-glib/telepathy-ytstenut-glib.h>
#include <ytstenut/yts-client.h>
#include <ytstenut/yts-error.h>
#include <ytstenut/yts-metadata.h>

TpConnection *
yts_client_get_connection (YtsClient *client);

TpYtsStatus *
yts_client_get_tp_status (YtsClient *client);

YtsError
yts_client_send_message (YtsClient    *client,
                         YtsContact   *contact,
                         char const   *uid,
                         YtsMetadata  *message);


void
yts_client_cleanup_contact (YtsClient         *self,
                             YtsContact const  *contact);

void
yts_client_emit_error (YtsClient  *self,
                       YtsError    error);

bool
yts_client_get_invocation_proxy (YtsClient   *self,
                                  char const   *invocation_id,
                                  YtsContact **contact,
                                  char const  **proxy_id);

GVariant *
yts_client_register_proxy (YtsClient  *self,
                            char const  *capability,
                            YtsContact *contact,
                            char const  *proxy_id);

bool
yts_client_unregister_proxy (YtsClient  *self,
                              char const  *capability,
                              char const  *proxy_id);

#endif /* YTS_CLIENT_INTERNAL_H */

