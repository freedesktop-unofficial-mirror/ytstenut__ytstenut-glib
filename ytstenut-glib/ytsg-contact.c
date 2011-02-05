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

#include <string.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/connection.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/channel.h>
#include <telepathy-glib/util.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/channel.h>

#include "ytsg-contact.h"
#include "ytsg-private.h"
#include "ytsg-marshal.h"
#include "ytsg-client.h"

static void ytsg_contact_dispose (GObject *object);
static void ytsg_contact_finalize (GObject *object);
static void ytsg_contact_constructed (GObject *object);
static void ytsg_contact_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec);
static void ytsg_contact_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgContact, ytsg_contact, G_TYPE_OBJECT);

#define YTSG_CONTACT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_CONTACT, YtsgContactPrivate))

struct _YtsgContactPrivate
{
  GHashTable *services;

  TpContact  *tp_contact;

  char       *icon_token;

  YtsgClient *client; /* client that owns us */

  guint disposed : 1;
};

enum
{
  SERVICE_ADDED,
  SERVICE_REMOVED,

  N_SIGNALS,
};

enum
{
  PROP_0,
  PROP_TP_CONTACT,
  PROP_ICON,
  PROP_CLIENT,
};

static guint signals[N_SIGNALS] = {0};

static void
ytsg_contact_service_added (YtsgContact *contact, YtsgService *service)
{
  YtsgContactPrivate *priv = contact->priv;
  const char         *uid  = ytsg_service_get_uid (service);

  g_return_if_fail (uid && *uid);
  g_return_if_fail (!g_hash_table_lookup (priv->services, uid));

  g_hash_table_insert (priv->services, (gpointer)uid, g_object_ref (service));
}

static void
ytsg_contact_service_removed (YtsgContact *contact, YtsgService *service)
{
  YtsgContactPrivate *priv = contact->priv;
  const char         *uid  = ytsg_service_get_uid (service);

  g_return_if_fail (uid && *uid);

  if (!g_hash_table_remove (priv->services, uid))
    g_warning (G_STRLOC ": unknown service with uid %s", uid);
}

static void
ytsg_contact_class_init (YtsgContactClass *klass)
{
  GParamSpec   *pspec;
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgContactPrivate));

  object_class->dispose      = ytsg_contact_dispose;
  object_class->finalize     = ytsg_contact_finalize;
  object_class->constructed  = ytsg_contact_constructed;
  object_class->get_property = ytsg_contact_get_property;
  object_class->set_property = ytsg_contact_set_property;

  klass->service_added       = ytsg_contact_service_added;
  klass->service_removed     = ytsg_contact_service_removed;

  /**
   * YtsgContact:tp-contact:
   *
   * TpContact of this item.
   */
  pspec = g_param_spec_object ("tp-contact",
                               "TpContact",
                               "TpContact",
                               TP_TYPE_CONTACT,
                               G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_TP_CONTACT, pspec);

  /**
   * YtsgContact:icon:
   *
   * Icon for this item.
   *
   * The property holds a GFile* pointing to the latest
   * cached image.
   *
   * Since: 0.1
   */
  pspec = g_param_spec_object ("icon",
                               "Icon",
                               "Icon",
                               G_TYPE_FILE,
                               G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_ICON, pspec);

  /**
   * YtsgContact:client:
   *
   * #YtsgClient that owns the roster
   */
  pspec = g_param_spec_object ("client",
                               "Client",
                               "Client",
                               YTSG_TYPE_CLIENT,
                               G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CLIENT, pspec);

  /**
   * YtsgContact::service-added:
   * @contact: the contact which received the signal
   * @service: the service
   *
   * The ::service-added signal is emitted when a new services is added to
   * the contact.
   *
   * Since: 0.1
   */
  signals[SERVICE_ADDED] =
    g_signal_new (I_("service-added"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (YtsgContactClass, service_added),
                  NULL, NULL,
                  ytsg_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTSG_TYPE_SERVICE);

  /**
   * YtsgContact::service-removed:
   * @contact: the contact which received the signal
   * @service: the service
   *
   * The ::service-removed signal is emitted when a services is removed from
   * the contact.
   *
   * Since: 0.1
   */
  signals[SERVICE_REMOVED] =
    g_signal_new (I_("service-removed"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsgContactClass, service_removed),
                  NULL, NULL,
                  ytsg_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTSG_TYPE_SERVICE);
}

static void
ytsg_contact_constructed (GObject *object)
{
  YtsgContact *self = (YtsgContact*) object;

  if (G_OBJECT_CLASS (ytsg_contact_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_contact_parent_class)->constructed (object);
}

static void
ytsg_contact_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  switch (property_id)
    {
    case PROP_ICON:
      {
        GFile *file = ytsg_contact_get_icon (self, NULL);
        g_value_take_object (value, file);

        g_warning ("Should use ytst_contact_get_icon() instead of querying "
                   "YstgContact:icon");
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_contact_avatar_file_cb (TpContact   *contact,
                             GParamSpec  *param,
                             YtsgContact *ycontact)
{
  YtsgContactPrivate *priv  = ycontact->priv;
  const char         *token = tp_contact_get_avatar_token (contact);

  if ((priv->icon_token && token && !strcmp (priv->icon_token, token)) ||
      (!priv->icon_token && !token))
    {
      return;
    }

  g_free (priv->icon_token);
  priv->icon_token = g_strdup (token);

  g_object_notify ((GObject*)ycontact, "icon");
}

static void
ytsg_contact_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  switch (property_id)
    {
    case PROP_TP_CONTACT:
      {
        priv->tp_contact = g_value_dup_object (value);

        g_signal_connect (priv->tp_contact, "notify::avatar-file",
                          G_CALLBACK (ytsg_contact_avatar_file_cb),
                          self);
      }
      break;
    case PROP_CLIENT:
      priv->client = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_contact_init (YtsgContact *self)
{
  self->priv = YTSG_CONTACT_GET_PRIVATE (self);

  self->priv->services = g_hash_table_new_full (g_direct_hash,
                                                g_direct_equal,
                                                NULL,
                                                g_object_unref);
}

static void
ytsg_contact_dispose (GObject *object)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  priv->client = NULL;

  g_hash_table_destroy (priv->services);
  priv->services = NULL;

  if (priv->tp_contact)
    {
      g_object_unref (priv->tp_contact);
      priv->tp_contact = NULL;
    }

  G_OBJECT_CLASS (ytsg_contact_parent_class)->dispose (object);
}

static void
ytsg_contact_finalize (GObject *object)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  g_free (priv->icon_token);

  G_OBJECT_CLASS (ytsg_contact_parent_class)->finalize (object);
}

/**
 * ytsg_contact_get_jid:
 * @contact: #YtsgContact
 *
 * Retrieves the jabber identifier of this contact.
 *
 * Return value: (transfer none): The jid of this contact.
 */
const char *
ytsg_contact_get_jid (const YtsgContact *contact)
{
  YtsgContactPrivate *priv;

  g_return_val_if_fail (YTSG_IS_CONTACT (contact), NULL);

  priv = contact->priv;

  g_return_val_if_fail (!priv->disposed, NULL);
  g_return_val_if_fail (priv->tp_contact, NULL);

  return tp_contact_get_identifier (priv->tp_contact);
}

/**
 * ytsg_contact_get_name:
 * @contact: #YtsgContact
 *
 * Retrieves human readable name of this client
 *
 * Return value: (transfer none): The name of this contact.
 */
const char *
ytsg_contact_get_name (const YtsgContact *contact)
{
  g_warning (G_STRLOC ": %s is not implemented", __FUNCTION__);
  return NULL;
}

/**
 * ytsg_contact_get_icon:
 * @contact: #YtsgContact
 * @mime: (transfer none): location to store a pointer to the icon mime type
 *
 * Retrieves icon of this contact. If the mime parameter is provided, on return
 * it will contain the mime type of the icon, this pointer must not be modified
 * or freed by the caller.
 *
 * Return value: (transfer full): #GFile pointing to the icon image, can be
 * %NULL. The caller owns a reference on the returned object, and must release
 * it when no longer needed with g_object_unref().
 */
GFile *
ytsg_contact_get_icon (const YtsgContact  *contact, const char **mime)
{
  YtsgContactPrivate  *priv;
  GFile               *file;

  g_return_val_if_fail (YTSG_IS_CONTACT (contact), NULL);

  priv = contact->priv;

  g_return_val_if_fail (!priv->disposed, NULL);

  if (!(file = tp_contact_get_avatar_file (priv->tp_contact)))
    return NULL;

  if (mime)
    *mime = tp_contact_get_avatar_mime_type (priv->tp_contact);
  return g_object_ref (file);
}

YtsgContact *
_ytsg_contact_new (YtsgClient *client, TpContact *tp_contact)
{
  g_return_val_if_fail (YTSG_IS_CLIENT (client), NULL);
  g_return_val_if_fail ( TP_IS_CONTACT (tp_contact), NULL);

  return g_object_new (YTSG_TYPE_CONTACT,
                       "client",       client,
                       "tp-contact",   tp_contact,
                       NULL);
}

TpContact *
ytsg_contact_get_tp_contact (const YtsgContact  *contact)
{
  YtsgContactPrivate  *priv;

  g_return_val_if_fail (YTSG_IS_CONTACT (contact), NULL);

  priv = contact->priv;

  g_return_val_if_fail (!priv->disposed, NULL);

  return priv->tp_contact;
}

