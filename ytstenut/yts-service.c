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

// FIXME should not reference contact

#include <telepathy-glib/util.h>
#include <telepathy-ytstenut-glib/telepathy-ytstenut-glib.h>

#include "yts-client-internal.h"
#include "yts-contact.h"
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
  PROP_CONTACT,
  PROP_STATUS_XML
};

enum {
  SIG_MESSAGE,
  N_SIGNALS
};

typedef struct {

  /* YtsCapability */
  char  **fqc_ids;

  /* YtsService */
  char const  *type;
  GHashTable  *names;
  char const  *uid;

  YtsContact *contact;   /* back-reference to the contact object that owns us */

  char        *status_xml;
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
_tp_status_changed (TpYtsStatus *status,
                    char const  *contact_id,
                    char const  *capability,
                    char const  *service_name,
                    char const  *xml,
                    YtsService  *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);
  char const *jid;

  jid = yts_service_get_jid (self);

  YTS_NOTE (STATUS, "Status changed for %s/%s:%s",
             contact_id, service_name, capability);

  if (0 == g_strcmp0 (contact_id, jid) &&
      0 == g_strcmp0 (service_name, priv->uid) &&
      0 != g_strcmp0 (xml, priv->status_xml)) {

    if (priv->status_xml) {
      g_free (priv->status_xml);
      priv->status_xml = NULL;
    }

    if (xml) {
      priv->status_xml = g_strdup (xml);
    }

    g_object_notify (G_OBJECT (self), "status-xml");
  }
}

static void
_constructed (GObject *object)
{
  YtsServicePrivate *priv = GET_PRIVATE (object);
  YtsClient   *client;
  TpYtsStatus *tp_status;
  GHashTable  *stats;

  if (G_OBJECT_CLASS (yts_service_parent_class)->constructed)
    G_OBJECT_CLASS (yts_service_parent_class)->constructed (object);

  g_return_if_fail (priv->contact);

  /*
   * Construct the YtsStatus object from the xml stored in
   * TpYtsStatus:discovered-statuses
   *
   * -- this is bit cumbersome, requiring nested hash table lookup.
   */
  client = yts_contact_get_client (priv->contact);
  tp_status = yts_client_get_tp_status (client);
  g_return_if_fail (tp_status);

  if (priv->fqc_ids && *priv->fqc_ids &&
      (stats = tp_yts_status_get_discovered_statuses (tp_status))) {

    char const *jid = yts_service_get_jid (YTS_SERVICE (object));

    // FIXME, should we do this for every cap possibly?
    char const *cap = *priv->fqc_ids; /*a single capability we have*/
    GHashTable *cinfo;

    if ((cinfo = g_hash_table_lookup (stats, jid))) {

      GHashTable *capinfo;
      if (NULL != (capinfo = g_hash_table_lookup (cinfo, cap))) {

        char *xml;
        if (NULL != (xml = g_hash_table_lookup (capinfo, priv->uid))) {
          priv->status_xml = g_strdup (xml);
        }
      }
    }
  }

  tp_g_signal_connect_object (tp_status, "status-changed",
                              G_CALLBACK (_tp_status_changed),
                              object, 0);
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

    case PROP_UID:
      g_value_set_string (value, priv->uid);
      break;
    case PROP_TYPE:
      g_value_set_string (value, priv->type);
      break;
    case PROP_NAMES:
      g_value_set_boxed (value, priv->names);
      break;
    case PROP_STATUS_XML:
      g_value_set_string (value, priv->status_xml);
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

    case PROP_CONTACT:
      priv->contact = g_value_get_object (value);
      break;
    case PROP_UID:
      priv->uid = g_intern_string (g_value_get_string (value));
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

  /* Free pointer, no ref. */
  priv->contact = NULL;

  if (priv->fqc_ids) {
    g_strfreev (priv->fqc_ids);
    priv->fqc_ids = NULL;
  }

  if (priv->names) {
    g_hash_table_unref (priv->names);
    priv->names = NULL;
  }

  if (priv->status_xml) {
    g_free (priv->status_xml);
    priv->status_xml = NULL;
  }

  G_OBJECT_CLASS (yts_service_parent_class)->dispose (object);
}

static void
yts_service_class_init (YtsServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec   *pspec;

  g_type_class_add_private (klass, sizeof (YtsServicePrivate));

  object_class->constructed = _constructed;
  object_class->dispose = _dispose;
  object_class->get_property = _get_property;
  object_class->set_property = _set_property;

  /* YtsCapability, needs to be overridden. */

  g_object_class_override_property (object_class,
                                    PROP_CAPABILITY_FQC_IDS,
                                    "fqc-ids");

  /**
   * YtsService:contact:
   *
   * #YtsContact this service belongs to.
   */
  pspec = g_param_spec_object ("contact", "", "",
                               YTS_TYPE_CONTACT,
                               G_PARAM_WRITABLE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_CONTACT, pspec);

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
   * YtsService:type:
   *
   * The current status of this service in unparsed form.
   */
  pspec = g_param_spec_string ("status-xml", "", "",
                               NULL,
                               G_PARAM_READABLE |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_STATUS_XML, pspec);

  /**
   * YtsService::message:
   * @service: the service which received the signal
   * @message: the message
   *
   * The ::message signal is emitted when message is received on given service
   *
   * Since: 0.1
   */
  _signals[SIG_MESSAGE] = g_signal_new ("message",
                                        G_TYPE_FROM_CLASS (object_class),
                                        G_SIGNAL_RUN_LAST,
                                        G_STRUCT_OFFSET (YtsServiceClass,
                                                         message),
                                        NULL, NULL,
                                        yts_marshal_VOID__STRING,
                                        G_TYPE_NONE, 1,
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

/**
 * yts_service_get_jid:
 * @service: #YtsService
 *
 * Returns the jid of the the given service. The returned pointer is to a
 * canonical representation created with g_intern_string().
 *
 * Return value: (transfer none): the jid.
 */
char const *
yts_service_get_jid (YtsService *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return yts_contact_get_jid (priv->contact);
}

/**
 * yts_service_get_contact:
 * @service: #YtsService
 *
 * Retrieves the #YtsContact associated with this service; the contact object
 * must not be freed by the caller.
 *
 * Return value (transfer none): #YtsContact.
 */
YtsContact *const
yts_service_get_contact (YtsService *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return priv->contact;
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

char const *
yts_service_get_status_xml (YtsService *self)
{
  YtsServicePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_SERVICE (self), NULL);

  return priv->status_xml;
}

