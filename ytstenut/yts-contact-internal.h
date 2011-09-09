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

#ifndef YTS_CONTACT_INTERNAL_H
#define YTS_CONTACT_INTERNAL_H

#include <stdbool.h>
#include <ytstenut/yts-contact.h>

YtsContact *
yts_contact_new (YtsClient  *client,
                 char const *jid);

bool
yts_contact_dispatch_event (YtsContact  *self,
                            char const  *capability,
                            char const  *aspect,
                            GVariant    *arguments);

bool
yts_contact_dispatch_response (YtsContact *self,
                               char const *capability,
                               char const *invocation_id,
                               GVariant   *response);

void
yts_contact_add_service (YtsContact *contact,
                         YtsService *service);

void
yts_contact_remove_service_by_uid (YtsContact *contact,
                                   char const *uid);

bool
yts_contact_is_empty (YtsContact *contact);

void
yts_contact_set_ft_channel (YtsContact  *item,
                            TpChannel   *channel);

#endif /* YTS_CONTACT_INTERNAL_H */

