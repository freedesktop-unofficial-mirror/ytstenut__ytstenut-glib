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

#include "ytsg-marshal.h"
#include "ytsg-profile.h"
#include "ytsg-proxy.h"

G_DEFINE_INTERFACE (YtsgProfile,
                    ytsg_profile,
                    G_TYPE_OBJECT)

enum {
  SIG_REGISTER_PROXY_RESPONSE,
  SIG_UNREGISTER_PROXY_RESPONSE,
  N_SIGNALS
};

static unsigned int _signals[N_SIGNALS] = { 0, };

static void
_register_proxy (YtsgProfile  *self,
                 char const   *invocation_id,
                 char const   *capability)
{
  g_critical ("%s : Method YtsgProfile.register_proxy() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
}

static void
_unregister_proxy (YtsgProfile  *self,
                   char const   *invocation_id,
                   char const   *capability)
{
  g_critical ("%s : Method YtsgProfile.unregister_proxy() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
}

static void
ytsg_profile_default_init (YtsgProfileInterface *interface)
{
  GParamSpec *pspec;

  interface->register_proxy = _register_proxy;
  interface->unregister_proxy = _unregister_proxy;

  /* Only to hold the default value */
  g_object_interface_install_property (interface,
                                       g_param_spec_string ("capability", "", "",
                                                            YTSG_PROFILE_CAPABILITY,
                                                            G_PARAM_STATIC_STRINGS));

  pspec = g_param_spec_boxed ("capabilities", "", "",
                              G_TYPE_STRV,
                              G_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS);
  g_object_interface_install_property (interface, pspec);

  /* Signals */

  _signals[SIG_REGISTER_PROXY_RESPONSE] = g_signal_new (
                                              "register-proxy-response",
                                              YTSG_TYPE_PROFILE,
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
                                              ytsg_marshal_VOID__STRING_BOOLEAN,
                                              G_TYPE_NONE,
                                              2, G_TYPE_STRING, G_TYPE_BOOLEAN);

  _signals[SIG_UNREGISTER_PROXY_RESPONSE] = g_signal_new (
                                              "unregister-proxy-response",
                                              YTSG_TYPE_PROFILE,
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
                                              ytsg_marshal_VOID__STRING_BOOLEAN,
                                              G_TYPE_NONE,
                                              2, G_TYPE_STRING, G_TYPE_BOOLEAN);
}

GStrv
ytsg_profile_get_capabilities (YtsgProfile  *self)
{
  GStrv capabilities;

  g_return_val_if_fail (YTSG_IS_PROFILE (self), NULL);

  capabilities = NULL;
  g_object_get (self, "capabilities", &capabilities, NULL);

  return capabilities;
}

void
ytsg_profile_register_proxy (YtsgProfile  *self,
                             char const   *invocation_id,
                             char const   *capability)
{
  g_return_if_fail (YTSG_IS_PROFILE (self));

  YTSG_PROFILE_GET_INTERFACE (self)->register_proxy (self,
                                                     invocation_id,
                                                     capability);
}

void
ytsg_profile_unregister_proxy (YtsgProfile  *self,
                               char const   *invocation_id,
                               char const   *capability)
{
  g_return_if_fail (YTSG_IS_PROFILE (self));

  YTSG_PROFILE_GET_INTERFACE (self)->unregister_proxy (self,
                                                       invocation_id,
                                                       capability);
}

void
ytsg_profile_register_proxy_return (YtsgProfile *self,
                                    char const  *invocation_id,
                                    bool         return_value)
{
  g_return_if_fail (YTSG_IS_PROFILE (self));

  g_signal_emit (self, _signals[SIG_REGISTER_PROXY_RESPONSE], 0,
                 invocation_id, return_value);
}

void
ytsg_profile_unregister_proxy_return (YtsgProfile *self,
                                      char const  *invocation_id,
                                      bool         return_value)
{
  g_return_if_fail (YTSG_IS_PROFILE (self));

  g_signal_emit (self, _signals[SIG_UNREGISTER_PROXY_RESPONSE], 0,
                 invocation_id, return_value);
}

