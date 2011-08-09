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

#ifndef YTSG_SERVICE_IMPL_H
#define YTSG_SERVICE_IMPL_H

#include <stdbool.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_TYPE_SERVICE_IMPL \
  (ytsg_service_impl_get_type ())

#define YTSG_SERVICE_IMPL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_TYPE_SERVICE_IMPL, YtsgServiceImpl))

#define YTSG_IS_SERVICE_IMPL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_TYPE_SERVICE_IMPL))

#define YTSG_SERVICE_IMPL_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTSG_TYPE_SERVICE_IMPL, YtsgServiceImplInterface))

typedef struct YtsgServiceImpl YtsgServiceImpl;
typedef struct YtsgServiceImplInterface YtsgServiceImplInterface;

struct YtsgServiceImplInterface {

  /*< private >*/
  GTypeInterface parent;

  /* Signals */

  void (*response) (YtsgServiceImpl *self,
                    char const      *invocation_id,
                    GVariant        *return_value);

  void (*error) (YtsgServiceImpl  *self,
                 char const       *invocation_id,
                 GError const     *error);
};

GType
ytsg_service_impl_get_type (void) G_GNUC_CONST;

void
ytsg_service_impl_invoke_method (char const  *invocation_id,
                                 char const  *capability,
                                 char const  *aspect,
                                 GVariant    *argumets);

G_END_DECLS

#endif /* YTSG_SERVICE_IMPL_H */

