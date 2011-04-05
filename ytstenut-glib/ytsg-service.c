/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2011 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:ytsg-service
 * @short_description: Object representing a service connected to the Ytstenut
 * mesh.
 *
 * #YtsgService represents a known service in the Ytstenut application mesh.
 */

#include "ytsg-service.h"
#include "ytsg-metadata-service.h"
#include "ytsg-client.h"

static void ytsg_service_dispose (GObject *object);
static void ytsg_service_finalize (GObject *object);
static void ytsg_service_constructed (GObject *object);
static void ytsg_service_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec);
static void ytsg_service_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec);

G_DEFINE_ABSTRACT_TYPE (YtsgService, ytsg_service, G_TYPE_OBJECT);

#define YTSG_SERVICE_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_SERVICE, YtsgServicePrivate))

struct _YtsgServicePrivate
{
  const char *uid;
  const char *jid;
  YtsgClient *client;   /* back-reference to the client object that owns us */

  guint disposed : 1;
};

enum
{
  N_SIGNALS,
};

enum
{
  PROP_0,
  PROP_UID,
  PROP_JID,
  PROP_CLIENT,
};

/* static guint signals[N_SIGNALS] = {0}; */

static void
ytsg_service_class_init (YtsgServiceClass *klass)
{
  GParamSpec   *pspec;
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgServicePrivate));

  object_class->dispose      = ytsg_service_dispose;
  object_class->finalize     = ytsg_service_finalize;
  object_class->constructed  = ytsg_service_constructed;
  object_class->get_property = ytsg_service_get_property;
  object_class->set_property = ytsg_service_set_property;

  /**
   * YtsgService:client:
   *
   * #YtsgClient this service belongs to
   */
  pspec = g_param_spec_object ("client",
                               "YtsgClient",
                               "YtsgClient",
                               YTSG_TYPE_CLIENT,
                               G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CLIENT, pspec);

  /**
   * YtsgService:uid:
   *
   * The uid of this service
   */
  pspec = g_param_spec_string ("uid",
                               "uid",
                               "uid",
                               NULL,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_UID, pspec);

  /**
   * YtsgService:jid:
   *
   * The jid of this service
   */
  pspec = g_param_spec_string ("jid",
                               "jid",
                               "jid",
                               NULL,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_JID, pspec);
}

static void
ytsg_service_constructed (GObject *object)
{
  YtsgService        *self = (YtsgService*) object;
  YtsgServicePrivate *priv = self->priv;

  if (G_OBJECT_CLASS (ytsg_service_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_service_parent_class)->constructed (object);

  g_assert (priv->uid);
}

static void
ytsg_service_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  YtsgService        *self = (YtsgService*) object;
  YtsgServicePrivate *priv = self->priv;

  switch (property_id)
    {
    case PROP_UID:
      g_value_set_string (value, priv->uid);
      break;
    case PROP_JID:
      g_value_set_string (value, priv->jid);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_service_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  YtsgService        *self = (YtsgService*) object;
  YtsgServicePrivate *priv = self->priv;

  switch (property_id)
    {
    case PROP_CLIENT:
      priv->client = g_value_get_object (value);
      break;
    case PROP_UID:
      priv->uid = g_intern_string (g_value_get_string (value));
      break;
    case PROP_JID:
      priv->jid = g_intern_string (g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_service_init (YtsgService *self)
{
  self->priv = YTSG_SERVICE_GET_PRIVATE (self);
}

static void
ytsg_service_dispose (GObject *object)
{
  YtsgService        *self = (YtsgService*) object;
  YtsgServicePrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  priv->client = NULL;

  G_OBJECT_CLASS (ytsg_service_parent_class)->dispose (object);
}

static void
ytsg_service_finalize (GObject *object)
{
  G_OBJECT_CLASS (ytsg_service_parent_class)->finalize (object);
}

/**
 * ytsg_service_get_uid:
 * @service: #YtsgService
 *
 * Returns the uid of the the given service. The returned pointer is to a
 * canonical representation created with g_intern_string().
 *
 * Return value: (transfer none): the uid.
 */
const char *
ytsg_service_get_uid (YtsgService *service)
{
  g_return_val_if_fail (YTSG_IS_SERVICE (service), NULL);

  return service->priv->uid;
}

/**
 * ytsg_service_get_jid:
 * @service: #YtsgService
 *
 * Returns the jid of the the client on which th given service is running. The
 * returned pointer is to a canonical representation created with
 * g_intern_string().
 *
 * Return value: (transfer none): the jid.
 */
const char *
ytsg_service_get_jid (YtsgService *service)
{
  g_return_val_if_fail (YTSG_IS_SERVICE (service), NULL);

  return service->priv->jid;
}

/**
 * ytsg_service_get_client:
 * @service: #YtsgService
 *
 * Retrieves the #YtsgClient associated with this service; the client object
 * must not be freed by the caller.
 *
 * Return value (transfer none): #YtsgClient.
 */
YtsgClient*
ytsg_service_get_client (YtsgService *service)
{
  YtsgServicePrivate *priv;

  g_return_val_if_fail (YTSG_IS_SERVICE (service), NULL);

  priv = service->priv;

  return priv->client;
}
