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
 * SECTION:yts-service
 * @short_description: Represents a service connected to the Ytstenut
 * application mesh.
 *
 * #YtsService represents a known service in the Ytstenut application mesh.
 */

#include "yts-capability.h"
#include "yts-debug.h"
#include "yts-marshal.h"
#include "yts-service-internal.h"
#include "config.h"

static void
_capability_interface_init (YtsCapability *interface);

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (YtsService,
                                  yts_service,
                                  G_TYPE_OBJECT,
                                  G_IMPLEMENT_INTERFACE (YTS_TYPE_CAPABILITY,
                                                         _capability_interface_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_SERVICE, YtsServicePrivate))

enum {
  PROP_0,

  /* YtsCapability */
  PROP_CAPABILITY_FQC_IDS,

  /* YtsService */
  PROP_TYPE,
  PROP_NAMES,
  PROP_UID,
  PROP_STATUSES
};

enum {
  SIG_SEND_MESSAGE,
  SIG_STATUS_CHANGED,
  N_SIGNALS
};

typedef struct {

  /* YtsCapability */
  char  **fqc_ids;

  /* YtsService */
  char const  *type;
  GHashTable  *names;
  char const  *uid;
  GHashTable  *statuses;

} YtsServicePrivate;

static unsigned _signals[N_SIGNALS] = { 0, };

/*
 * YtsCapability
 */

static void
_capability_interface_init (YtsCapability *interface)
{
  /* Nothing to do, it's just about overriding the "fqc-ids" property,
   * which has to be done in the concrete subclass of the Proxy. */
}

/*
 * YtsService
 */

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsServicePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    /* YtsCapability */

    case PROP_CAPABILITY_FQC_IDS:
      g_value_set_boxed (value, priv->fqc_ids);
      break;

    /* YtsService */

    case PROP_UID:
      g_value_set_string (value, priv->uid);
      break;
    case PROP_TYPE:
      g_value_set_string (value, priv->type);
      break;
    case PROP_NAMES:
      g_value_set_boxed (value, priv->names);
      break;
    case PROP_STATUSES:
      g_value_set_boxed (value, priv->statuses);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_set_property (GObject      *object,
               unsigned      property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  YtsServicePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    /* YtsCapability */

    case PROP_CAPABILITY_FQC_IDS:
      priv->fqc_ids = g_value_dup_boxed (value);
      break;

    /* YtsService */

    case PROP_UID:
      priv->uid = g_intern_string (g_value_get_string (value));
      break;
    case PROP_STATUSES:
      /* Construct-only */
      /* PONDERING this is a bit quirky, since we rely on the hashtable
       * being set up correctly. */
      g_warn_if_fail (priv->statuses == NULL);
      priv->statuses = g_value_dup_boxed (value);
      break;
    case PROP_TYPE:
      priv->type = g_intern_string (g_value_get_string (value));
      break;
    case PROP_NAMES:
      priv->names = g_value_dup_boxed (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsServicePrivate *priv = GET_PRIVATE (object);

  if (priv->fqc_ids) {
    g_strfreev (priv->fqc_ids);
    priv->fqc_ids = NULL;
  }

  if (priv->names) {
    g_hash_table_unref (priv->names);
    priv->names = NULL;
  }

  if (priv->statuses) {
    g_hash_table_unref (priv->statuses);
    priv->statuses = NULL;
  }

  G_OBJECT_CLASS (yts_service_parent_class)->dispose (object);
}

static void
yts_service_class_init (YtsServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec   *pspec;

  g_type_class_add_private (klass, sizeof (YtsServicePrivate));

  object_class->dispose = _dispose;
  object_class->get_property = _get_property;
  object_class->set_property = _set_property;

  /* YtsCapability, needs to be overridden. */

  g_object_class_override_property (object_class,
                                    PROP_CAPABILITY_FQC_IDS,
                                    "fqc-ids");

  /**
   * YtsService:uid:
   *
   * The uid of this service.
   */
  pspec = g_param_spec_string ("uid", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_UID, pspec);

  /**
   * YtsService:type:
   *
   * The type of this service.
   */
  pspec = g_param_spec_string ("type", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_TYPE, pspec);

  /**
   * YtsService:names:
   *
   * The names of this service.
   */
  pspec = g_param_spec_boxed ("names", "", "",
                              G_TYPE_HASH_TABLE,
                              G_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_NAMES, pspec);

  /**
   * YtsService:statuses:
   *
   * The current statuses of this service.
   */
  pspec = g_param_spec_boxed ("statuses", "", "",
                              G_TYPE_HASH_TABLE,
                              G_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_STATUSES, pspec);

  /**
   * YtsService::message:
   * @service: the service which received the signal
   * @message: the message
   *
   * The ::message signal is emitted when message is received on given service
   *
   * Since: 0.1
   */
  _signals[SIG_SEND_MESSAGE] = g_signal_new ("send-message",
                                              G_TYPE_FROM_CLASS (object_class),
                                              G_SIGNAL_RUN_LAST,
                                              G_STRUCT_OFFSET (YtsServiceClass,
                                                               send_message),
                                              NULL, NULL,
                                              yts_marshal_VOID__OBJECT,
                                              G_TYPE_NONE, 1,
                                              YTS_TYPE_METADATA);

  _signals[SIG_STATUS_CHANGED] = g_signal_new ("status-changed",
                                              G_TYPE_FROM_CLASS (object_class),
                                              G_SIGNAL_RUN_LAST,
                                              G_STRUCT_OFFSET (YtsServiceClass,
                                                               status_changed),
                                              NULL, NULL,
                                              yts_marshal_VOID__STRING_STRING,
                                              G_TYPE_NONE, 2,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING);
}

static void
yts_service_init (YtsService *self)
{
}

/**
 * yts_service_get_uid:
 * @service: #YtsService
 *
 * Returns the uid of the the given service. The returned pointer is to a
 * canonical representation created with g_intern_string().
 *
 * Return value: (transfer none): the uid.
 */
char const *
yts_service_get_uid (YtsService *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return priv->uid;
}

char const *
yts_service_get_service_type (YtsService *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return priv->type;
}

GHashTable *const
yts_service_get_names (YtsService *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return priv->names;
}

GHashTable *const
yts_service_get_statuses (YtsService  *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return priv->statuses;
}

void
yts_service_update_status (YtsService *self,
                           char const *fqc_id,
                           char const *status_xml)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);
  char const *current_status_xml;

  g_return_if_fail (YTS_IS_SERVICE (self));

  current_status_xml = g_hash_table_lookup (priv->statuses, fqc_id);
  if (0 != g_strcmp0 (current_status_xml, status_xml)) {
    g_hash_table_insert (priv->statuses,
                         g_strdup (fqc_id),
                         g_strdup (status_xml));
    g_object_notify (G_OBJECT (self), "statuses");
    g_signal_emit (self, _signals[SIG_STATUS_CHANGED], 0, fqc_id, status_xml);
  }
}

void
yts_service_send_message (YtsService  *self,
                          YtsMetadata *message)
{
  g_signal_emit (self, _signals[SIG_SEND_MESSAGE], 0, message);
}

