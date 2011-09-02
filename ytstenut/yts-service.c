/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2011 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 *         Rob Staudinger <robsta@linux.intel.com>
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
 * SECTION:yts-service
 * @short_description: Represents a service connected to the Ytstenut
 * application mesh.
 *
 * #YtsService represents a known service in the Ytstenut application mesh.
 */

#include <telepathy-glib/util.h>

#include "yts-contact.h"
#include "yts-debug.h"
#include "yts-marshal.h"
#include "yts-private.h"
#include "yts-service.h"

static void yts_service_dispose (GObject *object);
static void yts_service_finalize (GObject *object);
static void yts_service_constructed (GObject *object);
static void yts_service_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec);
static void yts_service_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec);

G_DEFINE_ABSTRACT_TYPE (YtsService, yts_service, G_TYPE_OBJECT);

#define YTS_SERVICE_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_SERVICE, YtsServicePrivate))

struct _YtsServicePrivate
{
  const char  *type;
  char       **caps;
  GHashTable  *names;
  const char  *uid;

  YtsContact *contact;   /* back-reference to the contact object that owns us */

  char        *status_xml;

  guint disposed : 1;
};

enum
{
  PROP_0,
  PROP_TYPE,
  PROP_NAMES,
  PROP_CAPS,
  PROP_UID,
  PROP_CONTACT,
  PROP_STATUS_XML
};

enum
{
  MESSAGE_SIGNAL,
  N_SIGNALS
};

static guint _signals[N_SIGNALS] = { 0, };

static void
yts_service_status_changed_cb (TpYtsStatus *status,
                                const gchar *contact_id,
                                const gchar *capability,
                                const gchar *service_name,
                                const gchar *xml,
                                YtsService *self)
{
  YtsServicePrivate *priv = self->priv;
  const char *jid = yts_service_get_jid (self);

  g_return_if_fail (contact_id && service_name && jid && priv->uid);

  YTS_NOTE (STATUS, "Status changed for %s/%s:%s",
             contact_id, service_name, capability);

  if (0 == g_strcmp0 (contact_id, jid) &&
      0 == g_strcmp0 (service_name, priv->uid) &&
      0 != g_strcmp0 (xml, priv->status_xml))
    {
      if (priv->status_xml)
        {
          g_free (priv->status_xml);
          priv->status_xml = NULL;
        }

      if (xml)
        {
          priv->status_xml = g_strdup (xml);
        }

      g_object_notify (G_OBJECT (self), "status-xml");
    }
}

static void
yts_service_class_init (YtsServiceClass *klass)
{
  GParamSpec   *pspec;
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsServicePrivate));

  object_class->dispose      = yts_service_dispose;
  object_class->finalize     = yts_service_finalize;
  object_class->constructed  = yts_service_constructed;
  object_class->get_property = yts_service_get_property;
  object_class->set_property = yts_service_set_property;

  /**
   * YtsService:contact:
   *
   * #YtsContact this service belongs to
   */
  pspec = g_param_spec_object ("contact",
                               "YtsContact",
                               "YtsContact",
                               YTS_TYPE_CONTACT,
                               G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CONTACT, pspec);

  /**
   * YtsService:uid:
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
   * YtsService:type:
   *
   * The type of this service
   */
  pspec = g_param_spec_string ("type",
                               "type",
                               "type",
                               NULL,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_TYPE, pspec);

  /**
   * YtsService:names:
   *
   * The names of this service
   */
  pspec = g_param_spec_boxed ("names",
                              "names",
                              "names",
                              G_TYPE_HASH_TABLE,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_NAMES, pspec);

  /**
   * YtsService:caps:
   *
   * The caps of this service
   */
  pspec = g_param_spec_boxed ("caps",
                              "caps",
                              "caps",
                              G_TYPE_STRV,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CAPS, pspec);

  /**
   * YtsService:type:
   *
   * The type of this service
   */
  pspec = g_param_spec_string ("status-xml",
                               "status-xml",
                               "status-xml",
                               NULL,
                               G_PARAM_READABLE);
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
  _signals[MESSAGE_SIGNAL] =
    g_signal_new ("message",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsServiceClass, message),
                  NULL, NULL,
                  yts_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

static void
yts_service_constructed (GObject *object)
{
  YtsServicePrivate *priv = YTS_SERVICE (object)->priv;
  YtsClient                 *client;
  TpYtsStatus                *status;
  GHashTable                 *stats;

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
  status = _yts_client_get_tp_status (client);
  g_return_if_fail (status);

  if (priv->caps && *priv->caps &&
      (stats = tp_yts_status_get_discovered_statuses (status)))
    {
      const char *jid = yts_service_get_jid ((YtsService*)object);
      const char *uid = yts_service_get_uid ((YtsService*)object);

      // FIXME, should we do this for every cap possibly?
      const char *cap = *priv->caps; /*a single capability we have*/
      GHashTable *cinfo;

      if ((cinfo = g_hash_table_lookup (stats, jid)))
        {
          GHashTable *capinfo;

          if ((capinfo = g_hash_table_lookup (cinfo, cap)))
            {
              char *xml;

              if ((xml = g_hash_table_lookup (capinfo, uid)))
                {
                  priv->status_xml = g_strdup (xml);
                }
            }
        }
    }

  tp_g_signal_connect_object (status, "status-changed",
                              G_CALLBACK (yts_service_status_changed_cb),
                              object, 0);
}

static void
yts_service_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  YtsService        *self = (YtsService*) object;
  YtsServicePrivate *priv = self->priv;

  switch (property_id)
    {
    case PROP_UID:
      g_value_set_string (value, priv->uid);
      break;
    case PROP_TYPE:
      g_value_set_string (value, priv->type);
      break;
    case PROP_NAMES:
      g_value_set_boxed (value, priv->names);
      break;
    case PROP_CAPS:
      g_value_set_boxed (value, priv->caps);
      break;
    case PROP_STATUS_XML:
      g_value_set_string (value, priv->status_xml);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yts_service_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  YtsService        *self = (YtsService*) object;
  YtsServicePrivate *priv = self->priv;

  switch (property_id)
    {
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
    case PROP_CAPS:
      priv->caps = g_value_dup_boxed (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yts_service_init (YtsService *self)
{
  self->priv = YTS_SERVICE_GET_PRIVATE (self);
}

static void
yts_service_dispose (GObject *object)
{
  YtsService        *self = (YtsService*) object;
  YtsServicePrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  priv->contact = NULL;

  if (priv->names)
    {
      g_hash_table_unref (priv->names);
      priv->names = NULL;
    }

  if (priv->status_xml)
    {
      g_free (priv->status_xml);
      priv->status_xml = NULL;
    }

  G_OBJECT_CLASS (yts_service_parent_class)->dispose (object);
}

static void
yts_service_finalize (GObject *object)
{
  G_OBJECT_CLASS (yts_service_parent_class)->finalize (object);
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
const char *
yts_service_get_uid (YtsService *service)
{
  g_return_val_if_fail (YTS_IS_SERVICE (service), NULL);

  return service->priv->uid;
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
const char *
yts_service_get_jid (YtsService *service)
{
  g_return_val_if_fail (YTS_IS_SERVICE (service), NULL);

  return yts_contact_get_jid (service->priv->contact);
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
YtsContact*
yts_service_get_contact (YtsService *service)
{
  YtsServicePrivate *priv;

  g_return_val_if_fail (YTS_IS_SERVICE (service), NULL);

  priv = service->priv;

  return priv->contact;
}

const char *
yts_service_get_service_type (YtsService *service)
{
  YtsServicePrivate *priv = service->priv;

  g_return_val_if_fail (YTS_IS_SERVICE (service), NULL);

  return priv->type;
}

const char **
yts_service_get_caps (YtsService *service)
{
  YtsServicePrivate *priv = service->priv;

  g_return_val_if_fail (YTS_IS_SERVICE (service), NULL);

  return (const char **) priv->caps;
}

GHashTable *
yts_service_get_names (YtsService *service)
{
  YtsServicePrivate *priv = service->priv;

  g_return_val_if_fail (YTS_IS_SERVICE (service), NULL);

  return priv->names;
}

const char *
yts_service_get_status_xml (YtsService *service)
{
  YtsServicePrivate *priv = service->priv;

  g_return_val_if_fail (YTS_IS_SERVICE (service), NULL);

  return priv->status_xml;
}

gboolean
yts_service_has_capability (YtsService *self,
                             char const  *capability)
{
  YtsServicePrivate *priv = self->priv;
  guint i;

  g_return_val_if_fail (YTS_IS_SERVICE (self), FALSE);
  g_return_val_if_fail (priv->caps, FALSE);

  for (i = 0; priv->caps[i] != NULL; i++) {
    if (0 == g_strcmp0 (capability, priv->caps[i])) {
      return TRUE;
    }
  }

  return FALSE;
}

