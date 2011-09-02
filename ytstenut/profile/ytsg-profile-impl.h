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

#ifndef YTSG_PROFILE_IMPL_H
#define YTSG_PROFILE_IMPL_H

#include <glib-object.h>
#include <ytstenut/ytsg-client.h>

G_BEGIN_DECLS

#define YTSG_TYPE_PROFILE_IMPL ytsg_profile_impl_get_type()

#define YTSG_PROFILE_IMPL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_TYPE_PROFILE_IMPL, YtsgProfileImpl))

#define YTSG_PROFILE_IMPL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTSG_TYPE_PROFILE_IMPL, YtsgProfileImplClass))

#define YTSG_IS_PROFILE_IMPL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_TYPE_PROFILE_IMPL))

#define YTSG_IS_PROFILE_IMPL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTSG_TYPE_PROFILE_IMPL))

#define YTSG_PROFILE_IMPL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTSG_TYPE_PROFILE_IMPL, YtsgProfileImplClass))

typedef struct {
  GObject parent;
} YtsgProfileImpl;

typedef struct {
  GObjectClass parent;
} YtsgProfileImplClass;

GType
ytsg_profile_impl_get_type (void) G_GNUC_CONST;

YtsgProfileImpl *
ytsg_profile_impl_new (YtsgClient *client);

bool
ytsg_profile_impl_add_capability (YtsgProfileImpl *self,
                                  char const      *capability);

bool
ytsg_profile_impl_remove_capability (YtsgProfileImpl  *self,
                                     char const       *capability);

G_END_DECLS

#endif /* YTSG_PROFILE_IMPL_H */

