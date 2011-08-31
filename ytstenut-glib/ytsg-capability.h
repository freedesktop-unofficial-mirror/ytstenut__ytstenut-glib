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

#ifndef YTSG_CAPABILITY_H
#define YTSG_CAPABILITY_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_TYPE_CAPABILITY (ytsg_capability_get_type ())

#define YTSG_CAPABILITY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_TYPE_CAPABILITY, YtsgCapability))

#define YTSG_IS_CAPABILITY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_TYPE_CAPABILITY))

#define YTSG_CAPABILITY_GET_INTERFACE(obj)                  \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),                    \
                                  YTSG_TYPE_CAPABILITY,     \
                                  YtsgCapabilityInterface))

typedef struct YtsgCapability YtsgCapability;

typedef struct {

  /*< private >*/
  GTypeInterface parent;

} YtsgCapabilityInterface;

GType
ytsg_capability_get_type (void) G_GNUC_CONST;

char *
ytsg_capability_get_fqc_id (YtsgCapability *self);

G_END_DECLS

#endif /* YTSG_CAPABILITY_H */

