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

#include "yts-capability.h"
#include "yts-invocation-message.h"
#include "yts-marshal.h"
#include "yts-message.h"
#include "yts-service-emitter.h"
#include "yts-service-internal.h"
#include "config.h"

static void
_capability_interface_init (YtsCapability *interface);

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (
                           YtsService,
                           yts_service,
                           G_TYPE_OBJECT,
                           G_IMPLEMENT_INTERFACE (YTS_TYPE_CAPABILITY,
                                                  _capability_interface_init))

/**
 * SECTION:yts-service
 * @short_description: Represents a service connected to the Ytstenut
 * application mesh.
 *
 * #YtsService represents a known service in the Ytstenut application mesh.
 */

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_SERVICE, YtsServicePrivate))

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0service\0"G_STRLOC

enum {
  PROP_0,

  /* YtsCapability */
  PROP_CAPABILITY_FQC_IDS,

  /* YtsService */
  PROP_TYPE,
  PROP_NAMES,
  PROP_SERVICE_ID,
  PROP_STATUSES
};

enum {
  SIG_STATUS_CHANGED,
  N_SIGNALS
};

typedef struct {

  /* YtsCapability */
  char  **fqc_ids;

  /* YtsService */
  char const  *type;
  GHashTable  *names;
  char const  *service_id;
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
_constructed (GObject *object)
{
  /* This is a bit of a hack, we require the non-abstract subclass to
   * implement this interface. */
  g_assert (YTS_IS_SERVICE_EMITTER (object));
}

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

    case PROP_SERVICE_ID:
      g_value_set_string (value, priv->service_id);
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

    case PROP_SERVICE_ID:
      priv->service_id = g_intern_string (g_value_get_string (value));
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

  object_class->dispose = _constructed;
  object_class->dispose = _dispose;
  object_class->get_property = _get_property;
  object_class->set_property = _set_property;

  /* YtsCapability, needs to be overridden. */

  g_object_class_override_property (object_class,
                                    PROP_CAPABILITY_FQC_IDS,
                                    "fqc-ids");

  /**
   * YtsService:service-id:
   *
   * The unique identifier of this service.
   */
  pspec = g_param_spec_string ("service-id", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_SERVICE_ID, pspec);

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
   * The names of this service, as they should be shown in a user interface.
   * Keys of the hash-table are locale names like %en_GB, pointing to the
   * respective localised name.
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
   * The current statuses of this service. The hash table contains an FQC-ID
   * to status (XML) text mapping.
   */
  pspec = g_param_spec_boxed ("statuses", "", "",
                              G_TYPE_HASH_TABLE,
                              G_PARAM_READWRITE |
                              G_PARAM_CONSTRUCT_ONLY |
                              G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_STATUSES, pspec);

  /**
   * YtsService::status-changed:
   * @self: object which emitted the signal.
   * @fqc_id: fully qualified capability ID of the changed status.
   * @status_xml: current status xml string.
   *
   * This signal is emitted when a the status of capability @fqc_id
   * of the remote service changed.
   *
   * See also #YtsService:statuses, which holds all statuses of the service.
   *
   * Since: 0.3
   */
  _signals[SIG_STATUS_CHANGED] = g_signal_new ("status-changed",
                                              G_TYPE_FROM_CLASS (object_class),
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
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
 * yts_service_get_service_id:
 * @service: #YtsService
 *
 * Returns the unique id of the the given service.
 *
 * Return value: (transfer none): the service-id.
 */
char const *
yts_service_get_service_id (YtsService *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return priv->service_id;
}

/**
 * yts_service_get_service_type:
 * @self: object on which to invoke this method.
 *
 * Returns: #YtsService:type
 *
 * Since: 0.3
 */
char const *
yts_service_get_service_type (YtsService *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return priv->type;
}

/**
 * yts_service_get_names:
 * @self: object on which to invoke this method.
 *
 * Returns: #YtsService:names
 *
 * Since: 0.3
 */
GHashTable *const
yts_service_get_names (YtsService *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return priv->names;
}

/**
 * yts_service_get_statuses:
 * @self: object on which to invoke this method.
 *
 * Returns: #YtsService:statuses
 *
 * Since: 0.3
 */
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

static YtsMetadata *
create_message (char const  *type,
                GVariant    *payload)
{

  RestXmlNode *node;
  char        *payload_str;
  char        *payload_str_escaped;;

  node = rest_xml_node_add_child (NULL, "message");
  /* PONDERING need those keywords be made reserved */
  rest_xml_node_add_attr (node, "type", type);
  rest_xml_node_add_attr (node, "capability", SERVICE_FQC_ID);

  payload_str = g_variant_print (payload, false);
  /* FIXME this is just a stopgap solution to lacking g_markup_unescape_text()
   * want to move to complex message bodies anywy. */
  payload_str_escaped = g_uri_escape_string (payload_str, NULL, true);
  rest_xml_node_add_attr (node, "payload", payload_str_escaped);
  g_free (payload_str_escaped);
  g_free (payload_str);

  if (g_variant_is_floating (payload))
    g_variant_unref (payload);

  return g_object_new (YTS_TYPE_MESSAGE,
                       "top-level-node", node,
                       NULL);
}

/**
 * yts_service_send_text:
 * @self: object on which to invoke this method.
 * @text: message to send to the remote service.
 *
 * Send a text message to remote service @self.
 *
 * Since: 0.3
 */
void
yts_service_send_text (YtsService *self,
                       char const *text)
{
  YtsMetadata *message;

  g_return_if_fail (YTS_IS_SERVICE (self));

  message = create_message ("text", g_variant_new_string (text));
  yts_service_emitter_send_message (YTS_SERVICE_EMITTER (self), message);
  g_object_unref (message);
}

/**
 * yts_service_send_list:
 * @self: object on which to invoke this method.
 * @texts: messages to send to the remote service.
 * @length: number of elements in @texts, or -1 if @texts is %NULL-terminated.
 *
 * Send a list of text messages to remote service @self.
 *
 * Since: 0.3
 */
void
yts_service_send_list (YtsService         *self,
                       char const *const  *texts,
                       int                 length)
{
  YtsMetadata *message;

  g_return_if_fail (YTS_IS_SERVICE (self));

  message = create_message ("list", g_variant_new_strv (texts, length));
  yts_service_emitter_send_message (YTS_SERVICE_EMITTER (self), message);
  g_object_unref (message);
}

/**
 * yts_service_send_dictionary:
 * @self: object on which to invoke this method.
 * @dictionary: dictionary to send to the remote service.
 * @length: number of elements in @dictionary, or -1 if @dictionary is
 *          %NULL-terminated. This is the number of array elements, not
 *          dictionary entry pairs.
 *
 * Send a list of text messages to remote service @self.
 *
 * Since: 0.3
 */
void
yts_service_send_dictionary (YtsService         *self,
                             char const *const  *dictionary,
                             int                 length)
{
  YtsMetadata     *message;
  GVariantBuilder  builder;
  int              i;

  g_return_if_fail (YTS_IS_SERVICE (self));

  g_variant_builder_init (&builder, G_VARIANT_TYPE_ARRAY);

  for (i = 0;
       length >= 0 ? i < length : dictionary[i] != NULL;
       i++) {

    char const *name = dictionary[i++];
    char const *value = dictionary[i];
    g_variant_builder_add (&builder, "{ss}", name, value);
  }

  message = create_message ("dictionary", g_variant_builder_end (&builder));
  yts_service_emitter_send_message (YTS_SERVICE_EMITTER (self), message);
  g_object_unref (message);
}

