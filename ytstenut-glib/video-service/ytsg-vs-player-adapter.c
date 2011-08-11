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
#include "ytsg-service-adapter.h"
#include "ytsg-vs-player.h"
#include "ytsg-vs-player-adapter.h"

static void
_service_adapter_interface_init (YtsgServiceAdapterInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsgVSPlayerAdapter,
                         ytsg_vs_player_adapter,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (YTSG_TYPE_SERVICE_ADAPTER,
                                                _service_adapter_interface_init))

#define GET_PRIVATE(o)                                        \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o),                          \
                                YTSG_VS_TYPE_PLAYER_ADAPTER,  \
                                YtsgVSPlayerAdapterPrivate))

typedef struct {
  YtsgVSPlayer  *player;
} YtsgVSPlayerAdapterPrivate;

/*
 * YtsgServiceAdapter implementation
 */

static void
_service_adapter_invoke (YtsgServiceAdapter *self,
                         char const         *invocation_id,
                         char const         *aspect,
                         GVariant           *argumets)
{
  YtsgVSPlayerAdapterPrivate *priv = GET_PRIVATE (self);

  if (0 == g_strcmp0 ("play", aspect)) {

    ytsg_vs_player_play (priv->player);

  } else if (0 == g_strcmp0 ("pause", aspect)) {

    ytsg_vs_player_pause (priv->player);

  } else if (0 == g_strcmp0 ("next", aspect)) {

    ytsg_vs_player_next (priv->player);

  } else if (0 == g_strcmp0 ("prev", aspect)) {

    ytsg_vs_player_prev (priv->player);

  } else {

    g_warning ("%s : Unknown method %s.%s()",
               G_STRLOC,
               G_OBJECT_TYPE_NAME (priv->player),
               aspect);
  }
}

static void
_service_adapter_interface_init (YtsgServiceAdapterInterface *interface)
{
  /* Methods */
  interface->invoke = _service_adapter_invoke;
}

/*
 * YtsgVSPlayerAdapter
 */

enum {
  PROP_0 = 0,
  PROP_SERVICE_ADAPTER_SERVICE_GTYPE,
  PROP_SERVICE_ADAPTER_SERVICE
};

static void
_player_destroyed (YtsgVSPlayerAdapter  *self,
                   void                 *stale_player_ptr)
{
  g_object_unref (self);
}

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  YtsgVSPlayerAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_SERVICE_ADAPTER_SERVICE_GTYPE:
      g_value_set_gtype (value, YTSG_VS_TYPE_PLAYER);
      break;
    case PROP_SERVICE_ADAPTER_SERVICE:
      g_value_set_object (value, priv->player);
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
  YtsgVSPlayerAdapterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_SERVICE_ADAPTER_SERVICE:
      /* Construct-only */
      g_return_if_fail (priv->player == NULL);
      g_return_if_fail (YTSG_VS_IS_PLAYER (g_value_get_object (value)));
      priv->player = YTSG_VS_PLAYER (g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_constructed (GObject *object)
{
  YtsgVSPlayerAdapterPrivate *priv = GET_PRIVATE (object);

  g_return_if_fail (priv->player);

  g_object_weak_ref (G_OBJECT (priv->player),
                     (GWeakNotify) _player_destroyed,
                     object);
}

static void
_dispose (GObject *object)
{
  G_OBJECT_CLASS (ytsg_vs_player_adapter_parent_class)->dispose (object);
}

static void
ytsg_vs_player_adapter_class_init (YtsgVSPlayerAdapterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgVSPlayerAdapterPrivate));

  object_class->constructed = _constructed;
  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_SERVICE,
                                    "service");

  g_object_class_override_property (object_class,
                                    PROP_SERVICE_ADAPTER_SERVICE_GTYPE,
                                    "service-gtype");
}

static void
ytsg_vs_player_adapter_init (YtsgVSPlayerAdapter *self)
{
}

