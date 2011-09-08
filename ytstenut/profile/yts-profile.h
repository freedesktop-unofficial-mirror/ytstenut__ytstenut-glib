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

#ifndef YTS_PROFILE_H
#define YTS_PROFILE_H

#include <stdbool.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define YTS_PROFILE_FQC_ID "org.freedesktop.Ytstenut"

#define YTS_TYPE_PROFILE \
  (yts_profile_get_type ())

#define YTS_PROFILE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_PROFILE, YtsProfile))

#define YTS_IS_PROFILE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_PROFILE))

#define YTS_PROFILE_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), YTS_TYPE_PROFILE, YtsProfileInterface))

typedef struct YtsProfile YtsProfile;
typedef struct YtsProfileInterface YtsProfileInterface;

struct YtsProfileInterface {

  /*< private >*/
  GTypeInterface parent;

  /* Methods */

  void
  (*register_proxy) (YtsProfile  *self,
                     char const   *invocation_id,
                     char const   *capability);

  void
  (*unregister_proxy) (YtsProfile  *self,
                       char const   *invocation_id,
                       char const   *capability);
};

GType
yts_profile_get_type (void) G_GNUC_CONST;

GPtrArray *
yts_profile_get_capabilities (YtsProfile  *self);

void
yts_profile_register_proxy (YtsProfile  *self,
                             char const   *invocation_id,
                             char const   *capability);

void
yts_profile_unregister_proxy (YtsProfile  *self,
                               char const   *invocation_id,
                               char const   *capability);

/* Protected */

void
yts_profile_register_proxy_return (YtsProfile *self,
                                    char const  *invocation_id,
                                    GVariant    *return_value);

void
yts_profile_unregister_proxy_return (YtsProfile *self,
                                      char const  *invocation_id,
                                      bool         return_value);

G_END_DECLS

#endif /* YTS_PROFILE_H */

