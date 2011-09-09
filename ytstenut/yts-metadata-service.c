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
 * Authored by: Tomas Frydrych <tf@linux.intel.com>
 */

/**
 * SECTION:yts-metadata-service
 * @short_description: Represents a metadata service
 * connected to the Ytstenut mesh.
 *
 * #YtsMetadataService represents a known metadata service in the Ytstenut
 * application mesh.
 */

#include <string.h>
#include <telepathy-glib/util.h>

#include "yts-client-internal.h"
#include "yts-debug.h"
#include "yts-invocation-message.h"
#include "yts-marshal.h"
#include "yts-message.h"
#include "yts-metadata-internal.h"
#include "yts-metadata-service-internal.h"
#include "yts-status.h"
#include "config.h"

static void yts_metadata_service_dispose (GObject *object);
static void yts_metadata_service_finalize (GObject *object);
static void yts_metadata_service_get_property (GObject    *object,
                                                guint       property_id,
                                                GValue     *value,
                                                GParamSpec *pspec);
static void yts_metadata_service_set_property (GObject      *object,
                                                guint         property_id,
                                                const GValue *value,
                                                GParamSpec   *pspec);

G_DEFINE_TYPE (YtsMetadataService, yts_metadata_service, YTS_TYPE_SERVICE)

#define YTS_METADATA_SERVICE_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_METADATA_SERVICE, YtsMetadataServicePrivate))

struct _YtsMetadataServicePrivate
{
  YtsStatus  *status;

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
yts_metadata_service_class_init (YtsMetadataServiceClass *klass)
{
  GParamSpec   *pspec;
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsMetadataServicePrivate));

  object_class->dispose      = yts_metadata_service_dispose;
  object_class->finalize     = yts_metadata_service_finalize;
  object_class->get_property = yts_metadata_service_get_property;
  object_class->set_property = yts_metadata_service_set_property;

  /**
   * YtsMetadataService:test:
   *
   * allows partial construction of YtsMetadataService for compile time
   * test purposes.
   */
  pspec = g_param_spec_boolean ("metadata-service-test",
                                "YtsMetadataService test",
                                "YtsMetadataService test",
                                FALSE,
                                G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_METADATA_SERVICE_TEST,
                                   pspec);

  /**
   * YtsMetadataService::status:
   * @service: the service which received the signal
   * @status: the status
   *
   * The ::status signal is emitted when the status of a given service changes
   *
   * Since: 0.1
   */
  signals[RECEIVED_STATUS] =
    g_signal_new ("status",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsMetadataServiceClass, received_status),
                  NULL, NULL,
                  yts_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTS_TYPE_STATUS);


  /**
   * YtsMetadataService::received-message:
   * @service: the service which received the signal
   * @message: the message
   *
   * The ::received-message signal is emitted when message is received on given service
   *
   * Since: 0.1
   */
  signals[RECEIVED_MESSAGE] =
    g_signal_new ("received-message",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsMetadataServiceClass, received_message),
                  NULL, NULL,
                  yts_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTS_TYPE_MESSAGE);
}

static void
yts_metadata_service_notify_status_xml_cb (YtsMetadataService *self,
                                            GParamSpec          *pspec,
                                            gpointer             data)
{
  YtsMetadataServicePrivate *priv = self->priv;
  char const *xml;

  xml = yts_service_get_status_xml (YTS_SERVICE (self));

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  priv->status = (YtsStatus*) yts_metadata_new_from_xml (xml);

  if (!YTS_IS_STATUS (priv->status))
    g_warning ("Failed to construct YtsStatus object");

  g_signal_emit (self, signals[RECEIVED_STATUS], 0, priv->status);
}

static void
yts_metadata_service_get_property (GObject    *object,
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
yts_metadata_service_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  YtsMetadataService        *self = (YtsMetadataService*) object;
  YtsMetadataServicePrivate *priv = self->priv;

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
yts_metadata_service_init (YtsMetadataService *self)
{
  self->priv = YTS_METADATA_SERVICE_GET_PRIVATE (self);

  g_signal_connect (self, "notify::status-xml",
                    G_CALLBACK (yts_metadata_service_notify_status_xml_cb), NULL);
}

static void
yts_metadata_service_dispose (GObject *object)
{
  YtsMetadataService        *self = (YtsMetadataService*) object;
  YtsMetadataServicePrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  G_OBJECT_CLASS (yts_metadata_service_parent_class)->dispose (object);
}

static void
yts_metadata_service_finalize (GObject *object)
{
  G_OBJECT_CLASS (yts_metadata_service_parent_class)->finalize (object);
}

void
yts_metadata_service_received_status (YtsMetadataService *service,
                                        const char          *xml)
{
  YtsMetadataServicePrivate *priv = service->priv;
  YtsStatus                 *status;

  status = (YtsStatus*) yts_metadata_new_from_xml (xml);

  if (priv->status)
    g_object_unref (priv->status);

  priv->status = status;

  g_return_if_fail (YTS_IS_STATUS (status));

  g_signal_emit (service, signals[RECEIVED_STATUS], 0, status);
}

void
yts_metadata_service_received_message (YtsMetadataService *service,
                                         const char          *xml)
{
  YtsMessage *message;

  message = (YtsMessage*) yts_metadata_new_from_xml (xml);

  g_return_if_fail (YTS_IS_MESSAGE (message));

  g_signal_emit (service, signals[RECEIVED_MESSAGE], 0, message);
}

static YtsError
yts_service_metadata_send_message (YtsMetadataService *service,
                                    YtsMessage         *message)
{
  YtsService *s       = (YtsService*) service;
  YtsContact *contact = yts_service_get_contact (s);
  YtsClient  *client  = yts_contact_get_client (contact);
  const char  *uid     = yts_service_get_uid (s);

  return yts_client_send_message (client, contact, uid, YTS_METADATA (message));
}

/**
 * yts_metadata_service_send_metadata:
 * @service: #YtsMetadataService
 * @metadata: #YtsMetadata that was received
 *
 * Sends the provided metadata via the service.
 *
 * Return value: #YtsError indicating status of the operation.
 */
YtsError
yts_metadata_service_send_metadata (YtsMetadataService *service,
                                     YtsMetadata        *metadata)
{
  g_return_val_if_fail (YTS_IS_METADATA_SERVICE (service),
                        yts_error_new (YTS_ERROR_INVALID_PARAMETER));
  g_return_val_if_fail (YTS_IS_METADATA (metadata),
                        yts_error_new (YTS_ERROR_INVALID_PARAMETER));

  if (YTS_IS_MESSAGE (metadata) ||
      YTS_IS_INVOCATION_MESSAGE (metadata))
    {
      return yts_service_metadata_send_message (service,
                                                 (YtsMessage*)metadata);
    }
  else if (YTS_IS_STATUS (metadata))
    {
      g_warning ("Cannot send YtsStatus, only YtsMessage !!!");
    }
  else
    g_warning ("Unknown metadata type %s",  G_OBJECT_TYPE_NAME (metadata));

  return yts_error_new (YTS_ERROR_INVALID_PARAMETER);
}

YtsService *
yts_metadata_service_new (YtsContact        *contact,
                          char const        *uid,
                          char const        *type,
                          char const *const *caps,
                          GHashTable        *names)
{
  g_return_val_if_fail (uid && *uid, NULL);

  return g_object_new (YTS_TYPE_METADATA_SERVICE,
                       "contact", contact,
                       "uid",     uid,
                       "type",    type,
                       "caps",    caps,
                       "names",   names,
                       NULL);
}

YtsStatus *
yts_metadata_service_get_status (YtsMetadataService *service)
{
  YtsMetadataServicePrivate *priv;

  g_return_val_if_fail (YTS_IS_METADATA_SERVICE (service), NULL);

  priv = service->priv;

  return priv->status;
}
