/*
 * Copyright (c) 2011 Intel Corp.
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

#ifndef YTSG_RESPONSE_MESSAGE_H
#define YTSG_RESPONSE_MESSAGE_H

#include <glib-object.h>
#include <ytstenut-glib/ytsg-metadata.h>

G_BEGIN_DECLS

#define YTSG_TYPE_RESPONSE_MESSAGE ytsg_response_message_get_type()

#define YTSG_RESPONSE_MESSAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_TYPE_RESPONSE_MESSAGE, YtsgResponseMessage))

#define YTSG_RESPONSE_MESSAGE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTSG_TYPE_RESPONSE_MESSAGE, YtsgResponseMessageClass))

#define YTSG_IS_RESPONSE_MESSAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_TYPE_RESPONSE_MESSAGE))

#define YTSG_IS_RESPONSE_MESSAGE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTSG_TYPE_RESPONSE_MESSAGE))

#define YTSG_RESPONSE_MESSAGE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTSG_TYPE_RESPONSE_MESSAGE, YtsgResponseMessageClass))

typedef struct {
  YtsgMetadata parent;
} YtsgResponseMessage;

typedef struct {
  YtsgMetadataClass parent;
} YtsgResponseMessageClass;

GType
ytsg_response_message_get_type (void) G_GNUC_CONST;

YtsgMetadata *
ytsg_response_message_new (char const *invocation_id,
                           GVariant   *response);

G_END_DECLS

#endif /* YTSG_RESPONSE_MESSAGE_H */
