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
 *              Rob Staudinger <robsta@linux.intel.com>
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

#include "yts-debug.h"
#include "yts-marshal.h"
#include "yts-metadata-internal.h"
#include "yts-metadata-service-internal.h"
#include "yts-service-internal.h"
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
}

static void
yts_metadata_service_status_changed_cb (YtsMetadataService *self,
                                        char const          *fqc_id,
                                        char const          *status_xml,
                                        gpointer             data)
{
  // PONDERING this fails when the status has multiple capabilities
  // but it's legacy anyway.

  YtsMetadataServicePrivate *priv = self->priv;

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  priv->status = (YtsStatus*) yts_metadata_new_from_xml (status_xml);

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

  g_signal_connect (self, "status-changed",
                    G_CALLBACK (yts_metadata_service_status_changed_cb), NULL);
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

YtsService *
yts_metadata_service_new (char const        *uid,
                          char const        *type,
                          char const *const *fqc_ids,
                          GHashTable        *names,
                          GHashTable        *statuses)
{
  g_return_val_if_fail (uid && *uid, NULL);

  return g_object_new (YTS_TYPE_METADATA_SERVICE,
                       "fqc-ids",   fqc_ids,
                       "uid",       uid,
                       "type",      type,
                       "names",     names,
                       "statuses",  statuses,
                       NULL);
}

