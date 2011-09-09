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

#ifndef YTS_METADATA_SERVICE_INTERNAL_H
#define YTS_METADATA_SERVICE_INTERNAL_H

#include <glib.h>
#include <ytstenut/yts-contact.h>
#include <ytstenut/yts-service.h>
#include <ytstenut/yts-metadata-service.h>

YtsService *
yts_metadata_service_new (YtsContact        *contact,
                          char const        *uid,
                          char const        *type,
                          char const *const *caps,
                          GHashTable        *names);

void
yts_metadata_service_received_status (YtsMetadataService  *service,
                                      char const          *xml);
void
yts_metadata_service_received_message (YtsMetadataService *service,
                                       char const         *xml);

#endif /* YTS_METADATA_SERVICE_INTERNAL_H */

