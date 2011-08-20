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

#include <stdbool.h>
#include "ytsg-marshal.h"
#include "ytsg-proxy.h"

G_DEFINE_TYPE (YtsgProxy, ytsg_proxy, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_PROXY, YtsgProxyPrivate))

enum {
  PROP_0
};

enum {
  INVOKE_SERVICE_SIGNAL,
  SERVICE_EVENT_SIGNAL,
  SERVICE_RESPONSE_SIGNAL,
  N_SIGNALS
};

typedef struct {
  char  *capability;
} YtsgProxyPrivate;

static guint _signals[N_SIGNALS] = { 0, };

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_set_property (GObject      *object,
               unsigned int  property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  G_OBJECT_CLASS (ytsg_proxy_parent_class)->dispose (object);
}

static void
ytsg_proxy_class_init (YtsgProxyClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgProxyPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  /* Signals */

  _signals[INVOKE_SERVICE_SIGNAL] =
                  g_signal_new ("invoke-service",
                                YTSG_TYPE_PROXY,
                                G_SIGNAL_RUN_LAST,
                                0,
                                NULL, NULL,
                                ytsg_marshal_VOID__STRING_STRING_BOXED,
                                G_TYPE_NONE, 3,
                                G_TYPE_STRING, G_TYPE_STRING, G_TYPE_VARIANT);

  _signals[SERVICE_EVENT_SIGNAL] =
                                g_signal_new ("service-event",
                                              YTSG_TYPE_PROXY,
                                              G_SIGNAL_RUN_LAST,
                                              G_STRUCT_OFFSET (YtsgProxyClass,
                                                               service_event),
                                              NULL, NULL,
                                              ytsg_marshal_VOID__STRING_BOXED,
                                              G_TYPE_NONE, 2,
                                              G_TYPE_STRING, G_TYPE_VARIANT);

  _signals[SERVICE_RESPONSE_SIGNAL] =
                              g_signal_new ("service-response",
                                            YTSG_TYPE_PROXY,
                                            G_SIGNAL_RUN_LAST,
                                            G_STRUCT_OFFSET (YtsgProxyClass,
                                                             service_response),
                                            NULL, NULL,
                                            ytsg_marshal_VOID__STRING_BOXED,
                                            G_TYPE_NONE, 2,
                                            G_TYPE_STRING, G_TYPE_VARIANT);
}

static void
ytsg_proxy_init (YtsgProxy *self)
{
}

YtsgProxy *
ytsg_proxy_new (char const *capability)
{
  g_return_val_if_fail (capability, NULL);

  return g_object_new (YTSG_TYPE_PROXY,
                       "capability", capability,
                       NULL);
}

static char *
create_invocation_id (YtsgProxy *self)
{
  static GRand  *_rand = NULL;
  char          *invocation_id;

  if (_rand == NULL) {
    _rand = g_rand_new ();
  }

  invocation_id = g_strdup_printf ("%x", g_rand_int (_rand));

  return invocation_id;
}

void
ytsg_proxy_invoke (YtsgProxy  *self,
                   char const *invocation_id,
                   char const *aspect,
                   GVariant   *arguments)
{
  char *auto_id;

  g_return_if_fail (YTSG_IS_PROXY (self));
  g_return_if_fail (aspect);

  auto_id = invocation_id ? NULL : create_invocation_id (self);

  g_signal_emit (self, _signals[INVOKE_SERVICE_SIGNAL], 0,
                 invocation_id ? invocation_id : auto_id,
                 aspect,
                 arguments);

  if (auto_id)
    g_free (auto_id);

  /* This is a bit hackish, ok, but it allows for creating the variant
   * in the invocation of this function. */
  if (arguments &&
      g_variant_is_floating (arguments)) {
    g_variant_unref (arguments);
  }
}

void
ytsg_proxy_handle_service_event (YtsgProxy  *self,
                                 char const *aspect,
                                 GVariant   *arguments)
{
  g_return_if_fail (YTSG_IS_PROXY (self));
  g_return_if_fail (aspect);

  g_signal_emit (self, _signals[SERVICE_EVENT_SIGNAL], 0,
                 aspect,
                 arguments);

  /* This is a bit hackish, ok, but it allows for creating the variant
   * in the invocation of this function. */
  if (arguments &&
      g_variant_is_floating (arguments)) {
    g_variant_unref (arguments);
  }
}

void
ytsg_proxy_handle_service_response (YtsgProxy   *self,
                                    char const  *invocation_id,
                                    GVariant    *response)
{
  g_return_if_fail (YTSG_IS_PROXY (self));

  g_signal_emit (self, _signals[SERVICE_RESPONSE_SIGNAL], 0,
                 invocation_id,
                 response);

  /* This is a bit hackish, ok, but it allows for creating the variant
   * in the invocation of this function. */
  if (response &&
      g_variant_is_floating (response)) {
    g_variant_unref (response);
  }
}

