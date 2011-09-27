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

#ifndef YTS_CAPABILITY_H
#define YTS_CAPABILITY_H

#include <stdbool.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define YTS_TYPE_CAPABILITY \
  (yts_capability_get_type ())

#define YTS_CAPABILITY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_CAPABILITY, YtsCapability))

#define YTS_IS_CAPABILITY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_CAPABILITY))

#define YTS_CAPABILITY_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTS_TYPE_CAPABILITY, YtsCapabilityInterface))

typedef struct YtsCapability YtsCapability;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

} YtsCapabilityInterface;

GType
yts_capability_get_type (void) G_GNUC_CONST;

char **
yts_capability_get_fqc_ids (YtsCapability *self);

bool
yts_capability_has_fqc_id (YtsCapability  *self,
                           char const     *fqc_id);

G_END_DECLS

#endif /* YTS_CAPABILITY_H */

