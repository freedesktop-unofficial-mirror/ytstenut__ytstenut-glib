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

#ifndef YTS_VP_TRANSMISSION_H
#define YTS_VP_TRANSMISSION_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTS_VP_TYPE_TRANSMISSION (yts_vp_transmission_get_type ())

#define YTS_VP_TRANSMISSION(obj)                         \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                     \
                               YTS_VP_TYPE_TRANSMISSION, \
                               YtsVPTransmission))

#define YTS_VP_IS_TRANSMISSION(obj)  \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_VP_TYPE_TRANSMISSION))

#define YTS_VP_TRANSMISSION_GET_INTERFACE(obj)                 \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                        \
                                  YTS_VP_TYPE_TRANSMISSION,    \
                                  YtsVPTransmissionInterface))

typedef struct YtsVPTransmission YtsVPTransmission;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

} YtsVPTransmissionInterface;

GType
yts_vp_transmission_get_type (void) G_GNUC_CONST;

char *
yts_vp_transmission_get_local_uri (YtsVPTransmission *self);

unsigned
yts_vp_transmission_get_progress (YtsVPTransmission *self);

char *
yts_vp_transmission_get_remote_uri (YtsVPTransmission *self);

G_END_DECLS

#endif /* YTS_VP_TRANSMISSION_H */

