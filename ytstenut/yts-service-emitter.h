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

#ifndef YTS_SERVICE_EMITTER_H
#define YTS_SERVICE_EMITTER_H

#include <glib-object.h>
#include <ytstenut/yts-metadata.h>

G_BEGIN_DECLS

#define YTS_TYPE_SERVICE_EMITTER  (yts_service_emitter_get_type ())

#define YTS_SERVICE_EMITTER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_SERVICE_EMITTER, YtsServiceEmitter))

#define YTS_IS_SERVICE_EMITTER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_SERVICE_EMITTER))

#define YTS_SERVICE_EMITTER_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTS_TYPE_SERVICE_EMITTER, YtsServiceEmitterInterface))

typedef struct YtsServiceEmitter YtsServiceEmitter;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

} YtsServiceEmitterInterface;

GType
yts_service_emitter_get_type (void) G_GNUC_CONST;

void
yts_service_emitter_send_message (YtsServiceEmitter *self,
                                  YtsMetadata       *message);

G_END_DECLS

#endif /* YTS_SERVICE_EMITTER_H */

