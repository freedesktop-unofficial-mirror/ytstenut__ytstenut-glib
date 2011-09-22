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

#ifndef YTS_PROXY_SERVICE_INTERNAL_H
#define YTS_PROXY_SERVICE_INTERNAL_H

#include <ytstenut/yts-proxy-service.h>

YtsService *
yts_proxy_service_new (char const         *service_id,
                       char const         *type,
                       char const *const  *fqc_ids,
                       GHashTable         *names,
                       GHashTable         *statuses);

bool
yts_proxy_service_dispatch_event (YtsProxyService *self,
                                  char const      *capability,
                                  char const      *aspect,
                                  GVariant        *arguments);

bool
yts_proxy_service_dispatch_response (YtsProxyService  *self,
                                     char const       *capability,
                                     char const       *invocation_id,
                                     GVariant         *response);

#endif /* YTS_PROXY_SERVICE_INTERNAL_H */

