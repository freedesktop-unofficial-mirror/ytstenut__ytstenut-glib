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
  PROP_0,
  PROP_CAPABILITY
};

enum {
  INVOKE_SERVICE_SIGNAL,
  SERVICE_RESPONSE_SIGNAL,
  N_SIGNALS
};

typedef struct {
  char              *capability;
} YtsgProxyPrivate;

static guint _signals[N_SIGNALS] = { 0, };

static void
_constructed (GObject *object)
{
  YtsgProxyPrivate *priv = GET_PRIVATE (object);

  if (G_OBJECT_CLASS (ytsg_proxy_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_proxy_parent_class)->constructed (object);

  g_warn_if_fail (priv->capability);
}

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  YtsgProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CAPABILITY:
      g_value_set_string (value, priv->capability);
      break;
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
  YtsgProxyPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CAPABILITY:
      /* construct-only */
      priv->capability = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsgProxyPrivate *priv = GET_PRIVATE (object);

  if (priv->capability) {
    g_free (priv->capability);
    priv->capability = NULL;
  }

  G_OBJECT_CLASS (ytsg_proxy_parent_class)->dispose (object);
}

static void
ytsg_proxy_class_init (YtsgProxyClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  GParamSpec    *pspec;
  GParamFlags    param_flags;

  g_type_class_add_private (klass, sizeof (YtsgProxyPrivate));

  object_class->constructed = _constructed;
  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  /* Properties */

  param_flags = G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB |
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY;

  pspec = g_param_spec_string ("capability", "", "",
                               NULL,
                               param_flags);
  g_object_class_install_property (object_class, PROP_CAPABILITY, pspec);

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

  _signals[SERVICE_RESPONSE_SIGNAL] =
              g_signal_new ("service-response",
                            YTSG_TYPE_PROXY,
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET (YtsgProxyClass, service_response),
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
  return g_object_new (YTSG_TYPE_PROXY,
                       "capability", capability,
                       NULL);
}

void
ytsg_proxy_invoke (YtsgProxy  *self,
                   char const *invocation_id,
                   char const *aspect,
                   GVariant   *arguments)
{
  g_return_if_fail (YTSG_IS_PROXY (self));

  g_signal_emit (self, _signals[INVOKE_SERVICE_SIGNAL], 0,
                 invocation_id,
                 aspect,
                 arguments);
}

