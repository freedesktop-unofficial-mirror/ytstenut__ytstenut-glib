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
 * SECTION:ytsg-metadata-service
 * @short_description: Represents a metadata service
 * connected to the Ytstenut mesh.
 *
 * #YtsgMetadataService represents a known metadata service in the Ytstenut
 * application mesh.
 */

#include <string.h>
#include <telepathy-glib/util.h>

#include "ytsg-metadata-service.h"

#include "ytsg-client.h"
#include "ytsg-debug.h"
#include "ytsg-invocation-message.h"
#include "ytsg-marshal.h"
#include "ytsg-message.h"
#include "ytsg-private.h"
#include "ytsg-status.h"

static void ytsg_metadata_service_dispose (GObject *object);
static void ytsg_metadata_service_finalize (GObject *object);
static void ytsg_metadata_service_get_property (GObject    *object,
                                                guint       property_id,
                                                GValue     *value,
                                                GParamSpec *pspec);
static void ytsg_metadata_service_set_property (GObject      *object,
                                                guint         property_id,
                                                const GValue *value,
                                                GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgMetadataService, ytsg_metadata_service, YTSG_TYPE_SERVICE)

#define YTSG_METADATA_SERVICE_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_METADATA_SERVICE, YtsgMetadataServicePrivate))

struct _YtsgMetadataServicePrivate
{
  YtsgStatus  *status;

  guint disposed : 1;
  guint test     : 1;
};

enum
{
  RECEIVED_STATUS,
  RECEIVED_MESSAGE,

  N_SIGNALS
};

enum
{
  PROP_0,
  PROP_METADATA_SERVICE_TEST,
};

static guint signals[N_SIGNALS] = {0};

static void
ytsg_metadata_service_class_init (YtsgMetadataServiceClass *klass)
{
  GParamSpec   *pspec;
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgMetadataServicePrivate));

  object_class->dispose      = ytsg_metadata_service_dispose;
  object_class->finalize     = ytsg_metadata_service_finalize;
  object_class->get_property = ytsg_metadata_service_get_property;
  object_class->set_property = ytsg_metadata_service_set_property;

  /**
   * YtsgMetadataService:test:
   *
   * allows partial construction of YtsgMetadataService for compile time
   * test purposes.
   */
  pspec = g_param_spec_boolean ("metadata-service-test",
                                "YtsgMetadataService test",
                                "YtsgMetadataService test",
                                FALSE,
                                G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_METADATA_SERVICE_TEST,
                                   pspec);

  /**
   * YtsgMetadataService::status:
   * @service: the service which received the signal
   * @status: the status
   *
   * The ::status signal is emitted when the status of a given service changes
   *
   * Since: 0.1
   */
  signals[RECEIVED_STATUS] =
    g_signal_new (I_("status"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsgMetadataServiceClass, received_status),
                  NULL, NULL,
                  ytsg_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTSG_TYPE_STATUS);


  /**
   * YtsgMetadataService::received-message:
   * @service: the service which received the signal
   * @message: the message
   *
   * The ::received-message signal is emitted when message is received on given service
   *
   * Since: 0.1
   */
  signals[RECEIVED_MESSAGE] =
    g_signal_new (I_("received-message"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsgMetadataServiceClass, received_message),
                  NULL, NULL,
                  ytsg_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTSG_TYPE_MESSAGE);
}

static void
ytsg_metadata_service_notify_status_xml_cb (YtsgMetadataService *self,
                                            GParamSpec          *pspec,
                                            gpointer             data)
{
  YtsgMetadataServicePrivate *priv = self->priv;
  char const *xml;

  xml = ytsg_service_get_status_xml (YTSG_SERVICE (self));

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  priv->status = (YtsgStatus*) _ytsg_metadata_new_from_xml (xml);

  if (!YTSG_IS_STATUS (priv->status))
    g_warning ("Failed to construct YtsgStatus object");

  g_signal_emit (self, signals[RECEIVED_STATUS], 0, priv->status);
}

static void
ytsg_metadata_service_get_property (GObject    *object,
                                    guint       property_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_metadata_service_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  YtsgMetadataService        *self = (YtsgMetadataService*) object;
  YtsgMetadataServicePrivate *priv = self->priv;

  switch (property_id)
    {
    case PROP_METADATA_SERVICE_TEST:
      priv->test = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_metadata_service_init (YtsgMetadataService *self)
{
  self->priv = YTSG_METADATA_SERVICE_GET_PRIVATE (self);

  g_signal_connect (self, "notify::status-xml",
                    G_CALLBACK (ytsg_metadata_service_notify_status_xml_cb), NULL);
}

static void
ytsg_metadata_service_dispose (GObject *object)
{
  YtsgMetadataService        *self = (YtsgMetadataService*) object;
  YtsgMetadataServicePrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  G_OBJECT_CLASS (ytsg_metadata_service_parent_class)->dispose (object);
}

static void
ytsg_metadata_service_finalize (GObject *object)
{
  G_OBJECT_CLASS (ytsg_metadata_service_parent_class)->finalize (object);
}

void
_ytsg_metadata_service_received_status (YtsgMetadataService *service,
                                        const char          *xml)
{
  YtsgMetadataServicePrivate *priv = service->priv;
  YtsgStatus                 *status;

  status = (YtsgStatus*) _ytsg_metadata_new_from_xml (xml);

  if (priv->status)
    g_object_unref (priv->status);

  priv->status = status;

  g_return_if_fail (YTSG_IS_STATUS (status));

  g_signal_emit (service, signals[RECEIVED_STATUS], 0, status);
}

void
_ytsg_metadata_service_received_message (YtsgMetadataService *service,
                                         const char          *xml)
{
  YtsgMessage *message;

  message = (YtsgMessage*) _ytsg_metadata_new_from_xml (xml);

  g_return_if_fail (YTSG_IS_MESSAGE (message));

  g_signal_emit (service, signals[RECEIVED_MESSAGE], 0, message);
}

static YtsgError
ytsg_service_metadata_send_message (YtsgMetadataService *service,
                                    YtsgMessage         *message)
{
  YtsgService *s       = (YtsgService*) service;
  YtsgContact *contact = ytsg_service_get_contact (s);
  YtsgClient  *client  = ytsg_contact_get_client (contact);
  const char  *uid     = ytsg_service_get_uid (s);

  return _ytsg_client_send_message (client, contact, uid, message);
}

/**
 * ytsg_metadata_service_send_metadata:
 * @service: #YtsgMetadataService
 * @metadata: #YtsgMetadata that was received
 *
 * Sends the provided metadata via the service.
 *
 * Return value: #YtsgError indicating status of the operation.
 */
YtsgError
ytsg_metadata_service_send_metadata (YtsgMetadataService *service,
                                     YtsgMetadata        *metadata)
{
  g_return_val_if_fail (YTSG_IS_METADATA_SERVICE (service),
                        ytsg_error_new (YTSG_ERROR_INVALID_PARAMETER));
  g_return_val_if_fail (YTSG_IS_METADATA (metadata),
                        ytsg_error_new (YTSG_ERROR_INVALID_PARAMETER));

  if (YTSG_IS_MESSAGE (metadata) ||
      YTSG_IS_INVOCATION_MESSAGE (metadata))
    {
      return ytsg_service_metadata_send_message (service,
                                                 (YtsgMessage*)metadata);
    }
  else if (YTSG_IS_STATUS (metadata))
    {
      g_warning ("Cannot send YtsgStatus, only YtsgMessage !!!");
    }
  else
    g_warning ("Unknown metadata type %s",  G_OBJECT_TYPE_NAME (metadata));

  return ytsg_error_new (YTSG_ERROR_INVALID_PARAMETER);
}

YtsgService *
_ytsg_metadata_service_new (YtsgContact *contact,
                            const char  *uid,
                            const char  *type,
                            const char **caps,
                            GHashTable  *names)
{
  g_return_val_if_fail (uid && *uid, NULL);

  return g_object_new (YTSG_TYPE_METADATA_SERVICE,
                       "contact", contact,
                       "uid",     uid,
                       "type",    type,
                       "caps",    caps,
                       "names",   names,
                       NULL);
}

YtsgStatus *
ytsg_metadata_service_get_status (YtsgMetadataService *service)
{
  YtsgMetadataServicePrivate *priv;

  g_return_val_if_fail (YTSG_IS_METADATA_SERVICE (service), NULL);

  priv = service->priv;

  return priv->status;
}
