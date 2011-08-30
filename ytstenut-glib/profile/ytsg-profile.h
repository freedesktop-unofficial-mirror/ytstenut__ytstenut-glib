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

#ifndef YTSG_PROFILE_H
#define YTSG_PROFILE_H

#include <stdbool.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define YTSG_PROFILE_CAPABILITY "org.freedesktop.Ytstenut"

#define YTSG_TYPE_PROFILE \
  (ytsg_profile_get_type ())

#define YTSG_PROFILE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTSG_TYPE_PROFILE, YtsgProfile))

#define YTSG_IS_PROFILE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTSG_TYPE_PROFILE))

#define YTSG_PROFILE_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTSG_TYPE_PROFILE, YtsgProfileInterface))

typedef struct YtsgProfile YtsgProfile;
typedef struct YtsgProfileInterface YtsgProfileInterface;

struct YtsgProfileInterface {

  /*< private >*/
  GTypeInterface parent;

  /* Methods */

  void
  (*register_proxy) (YtsgProfile  *self,
                     char const   *invocation_id,
                     char const   *capability);

  void
  (*unregister_proxy) (YtsgProfile  *self,
                       char const   *invocation_id,
                       char const   *capability);
};

GType
ytsg_profile_get_type (void) G_GNUC_CONST;

GPtrArray *
ytsg_profile_get_capabilities (YtsgProfile  *self);

void
ytsg_profile_register_proxy (YtsgProfile  *self,
                             char const   *invocation_id,
                             char const   *capability);

void
ytsg_profile_unregister_proxy (YtsgProfile  *self,
                               char const   *invocation_id,
                               char const   *capability);

/* Protected */

void
ytsg_profile_register_proxy_return (YtsgProfile *self,
                                    char const  *invocation_id,
                                    GVariant    *return_value);

void
ytsg_profile_unregister_proxy_return (YtsgProfile *self,
                                      char const  *invocation_id,
                                      bool         return_value);

G_END_DECLS

#endif /* YTSG_PROFILE_H */

