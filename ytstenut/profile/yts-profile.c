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

#include "yts-capability.h"
#include "yts-marshal.h"
#include "yts-profile.h"
#include "yts-proxy.h"
#include "config.h"

G_DEFINE_INTERFACE (YtsProfile,
                    yts_profile,
                    YTS_TYPE_CAPABILITY)

enum {
  SIG_REGISTER_PROXY_RESPONSE,
  SIG_UNREGISTER_PROXY_RESPONSE,
  N_SIGNALS
};

static unsigned int _signals[N_SIGNALS] = { 0, };

static void
_register_proxy (YtsProfile  *self,
                 char const   *invocation_id,
                 char const   *capability)
{
  g_critical ("%s : Method YtsProfile.register_proxy() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
}

static void
_unregister_proxy (YtsProfile  *self,
                   char const   *invocation_id,
                   char const   *capability)
{
  g_critical ("%s : Method YtsProfile.unregister_proxy() not implemented by %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (self));
}

static void
yts_profile_default_init (YtsProfileInterface *interface)
{
  GParamSpec *pspec;

  interface->register_proxy = _register_proxy;
  interface->unregister_proxy = _unregister_proxy;

  pspec = g_param_spec_boxed ("capabilities", "", "",
                              G_TYPE_PTR_ARRAY,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);
  g_object_interface_install_property (interface, pspec);

  /* Signals */

  _signals[SIG_REGISTER_PROXY_RESPONSE] = g_signal_new (
                                              "register-proxy-response",
                                              YTS_TYPE_PROFILE,
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
                                              yts_marshal_VOID__STRING_BOXED,
                                              G_TYPE_NONE,
                                              2, G_TYPE_STRING, G_TYPE_VARIANT);

  _signals[SIG_UNREGISTER_PROXY_RESPONSE] = g_signal_new (
                                              "unregister-proxy-response",
                                              YTS_TYPE_PROFILE,
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
                                              yts_marshal_VOID__STRING_BOOLEAN,
                                              G_TYPE_NONE,
                                              2, G_TYPE_STRING, G_TYPE_BOOLEAN);
}

GPtrArray *
yts_profile_get_capabilities (YtsProfile  *self)
{
  GPtrArray *capabilities;

  g_return_val_if_fail (YTS_IS_PROFILE (self), NULL);

  capabilities = NULL;
  g_object_get (self, "capabilities", &capabilities, NULL);

  return capabilities;
}

void
yts_profile_register_proxy (YtsProfile  *self,
                             char const   *invocation_id,
                             char const   *capability)
{
  g_return_if_fail (YTS_IS_PROFILE (self));

  YTS_PROFILE_GET_INTERFACE (self)->register_proxy (self,
                                                     invocation_id,
                                                     capability);
}

void
yts_profile_unregister_proxy (YtsProfile  *self,
                               char const   *invocation_id,
                               char const   *capability)
{
  g_return_if_fail (YTS_IS_PROFILE (self));

  YTS_PROFILE_GET_INTERFACE (self)->unregister_proxy (self,
                                                       invocation_id,
                                                       capability);
}

void
yts_profile_register_proxy_return (YtsProfile *self,
                                    char const  *invocation_id,
                                    GVariant    *return_value)
{
  g_return_if_fail (YTS_IS_PROFILE (self));

  g_signal_emit (self, _signals[SIG_REGISTER_PROXY_RESPONSE], 0,
                 invocation_id, return_value);

  if (g_variant_is_floating (return_value)) {
    g_variant_unref (return_value);
  }
}

void
yts_profile_unregister_proxy_return (YtsProfile *self,
                                      char const  *invocation_id,
                                      bool         return_value)
{
  g_return_if_fail (YTS_IS_PROFILE (self));

  g_signal_emit (self, _signals[SIG_UNREGISTER_PROXY_RESPONSE], 0,
                 invocation_id, return_value);
}

