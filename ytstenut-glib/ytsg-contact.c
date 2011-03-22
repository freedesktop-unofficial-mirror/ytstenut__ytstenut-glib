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
#include "ytsg-debug.h"
#include "ytsg-error.h"

#include "empathy-tp-file.h"

static void ytsg_c_pending_file_free (gpointer file);

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
  GHashTable *services;   /* hash of YtsgService instances */

  TpContact  *tp_contact; /* TpContact associated with YtsgContact */

  char       *icon_token; /* token identifying this contacts avatar */

  YtsgClient *client;     /* back-reference to the client that owns us */
  YtsgStatus *status;     /* status of this contact -- FIXME -- status is
                             per-service, not contact */

  GQueue     *pending_files;    /* files dispatched before channel open */
  GHashTable *ft_cancellables;

  YtsgSubscription  subscription; /* subscription state of this item */

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
  PROP_STATUS,
  PROP_SUBSCRIPTION,
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
   * YtsgContact:tp-contact:
   *
   * TpContact of this item.
   */
  pspec = g_param_spec_object ("status",
                               "YtsgStatus",
                               "YtsgStatus",
                               YTSG_TYPE_STATUS,
                               G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_STATUS, pspec);


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
   * YtsgContact:subscription:
   *
   * Subscription state of the item, #YtsgSubscription
   */
  pspec = g_param_spec_uint ("subscription",
                             "Subscription",
                             "Subscription to connect on",
                             0,
                             G_MAXUINT,
                             0,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
  g_object_class_install_property (object_class, PROP_SUBSCRIPTION, pspec);

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
    case PROP_STATUS:
      {
        if (priv->status)
          g_object_unref (priv->status);

        priv->status = g_value_dup_object (value);
      }
      break;
    case PROP_SUBSCRIPTION:
      _ytsg_contact_set_subscription (self, g_value_get_uint (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_contact_init (YtsgContact *self)
{
  YtsgContactPrivate *priv;

  priv = self->priv = YTSG_CONTACT_GET_PRIVATE (self);

  priv->services = g_hash_table_new_full (g_direct_hash,
                                          g_direct_equal,
                                          NULL,
                                          g_object_unref);

  priv->pending_files = g_queue_new ();

  priv->ft_cancellables  = g_hash_table_new_full (g_str_hash,
                                                  g_str_equal,
                                                  g_free,
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

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  G_OBJECT_CLASS (ytsg_contact_parent_class)->dispose (object);
}

static void
ytsg_contact_finalize (GObject *object)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  g_free (priv->icon_token);

  g_queue_foreach (priv->pending_files, (GFunc)ytsg_c_pending_file_free, NULL);
  g_queue_free (priv->pending_files);

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
  YtsgContactPrivate *priv;

  g_return_val_if_fail (YTSG_IS_CONTACT (contact), NULL);

  priv = contact->priv;

  g_return_val_if_fail (!priv->disposed, NULL);

  return priv->tp_contact;
}

/**
 * ytsg_contact_has_capability:
 * @item: #YtsgContact,
 * @cap: #YtsgCaps, capability to check for.
 *
 * Checks whether the contact has the given capability.
 *
 * Return value: returns %TRUE if the item advertises the capability, %FALSE
 * otherwise.
 */
gboolean
ytsg_contact_has_capability (const YtsgContact *item, YtsgCaps cap)
{
  g_critical (G_STRLOC ": NOT IMPLEMENTED!!!");

  return FALSE;
}


/*
 * ytsg_roster_item_set_subscription:
 * @item: #YtsgRosterItem
 * @subscription: #YtsgSubscription
 *
 * Sets #YtsgSubscription state of this item.
 */
void
_ytsg_contact_set_subscription (const YtsgContact *contact,
                                YtsgSubscription   subscription)
{
  YtsgContactPrivate  *priv;
  YtsgSubscription     or_with = subscription;

  g_return_if_fail (YTSG_CONTACT (contact));

  priv = contact->priv;

  g_return_if_fail (!priv->disposed);

  if (priv->subscription & subscription)
    return;

  YTSG_NOTE (ROSTER, "Contact %s: subscription status %d",
           ytsg_contact_get_jid (contact), subscription);

  if (subscription == YTSG_SUBSCRIPTION_APPROVED)
    {
      priv->subscription &= ~YTSG_SUBSCRIPTION_PENDING_IN;
      priv->subscription &= ~YTSG_SUBSCRIPTION_PENDING_OUT;
    }

  priv->subscription |= or_with;

  g_object_notify ((GObject*) contact, "subscription");
}

static gboolean
ytsg_contact_find_cancellable_cb (gpointer key,
                                  gpointer value,
                                  gpointer data)
{
  gpointer *the_thing = data;


  if (value == *the_thing)
    {
      *the_thing = key;
      return TRUE;
    }

  return FALSE;
}

typedef struct
{
  YtsgContact *item;
  guint32      atom;

} YtsgCFtOp;

static YtsgCFtOp *
ytsg_c_ft_op_new (YtsgContact *item, guint32 atom)
{
  YtsgCFtOp *o = g_slice_new (YtsgCFtOp);

  o->item = item;
  o->atom = atom;

  return o;
}

static void
ytsg_c_ft_op_free (gpointer op)
{
  YtsgCFtOp *o = op;

  g_slice_free (YtsgCFtOp, o);
}

static void
ytsg_contact_ft_op_cb (EmpathyTpFile *tp_file,
                       const GError  *error,
                       gpointer       data)
{
  GCancellable       *cancellable;
  YtsgError           e;
  YtsgCFtOp          *op     = data;
  YtsgContact        *item   = op->item;
  YtsgContactPrivate *priv   = item->priv;
  YtsgClient         *client = priv->client;
  guint32             atom   = op->atom;

  if (error)
    {
      e = (atom | YTSG_ERROR_UNKNOWN);
      g_warning ("File transfer to %s failed: %s",
                 ytsg_contact_get_jid (item), error->message);
    }
  else
    e = (atom | YTSG_ERROR_SUCCESS);

  if ((cancellable = empathy_tp_file_get_cancellable (tp_file)))
    {
      gpointer the_thing = cancellable;

      if (g_hash_table_find (priv->ft_cancellables,
                             ytsg_contact_find_cancellable_cb,
                             &the_thing))
        {
          g_hash_table_remove (priv->ft_cancellables, the_thing);
        }
    }

  ytsg_c_ft_op_free (op);
  ytsg_client_emit_error (client, e);
}

typedef struct
{
  const YtsgContact *item;
  TpChannel         *ft_channel;
  GFile             *file;
  char              *name;
  guint32            atom;

} YtsgCPendingFile;

static YtsgCPendingFile *
ytsg_c_pending_file_new (const YtsgContact *item,
                        GFile              *gfile,
                        const char         *name,
                        guint32             atom)
{
  YtsgCPendingFile *m = g_slice_new (YtsgCPendingFile);

  m->item       = item;
  m->name       = g_strdup (name);
  m->file       = g_object_ref (gfile);
  m->atom       = atom;
  m->ft_channel = NULL;

  return m;
}

static void
ytsg_c_pending_file_free (gpointer file)
{
  YtsgCPendingFile *m = file;

  g_object_unref (m->file);
  g_free (m->name);
  g_slice_free (YtsgCPendingFile, m);
}

static YtsgError
ytsg_c_dispatch_file (YtsgCPendingFile *file)
{
  YtsgContactPrivate *priv;
  EmpathyTpFile      *tp_file;
  GCancellable       *cancellable;
  YtsgCFtOp          *op;

  g_return_val_if_fail (file && file->item && file->ft_channel && file->file,
                        YTSG_ERROR_INVALID_PARAMETER);

  priv = file->item->priv;

  op = ytsg_c_ft_op_new ((YtsgContact*)file->item, file->atom);

  tp_file = empathy_tp_file_new (file->ft_channel, FALSE);

  cancellable = g_cancellable_new ();

  empathy_tp_file_offer (tp_file,
                         file->file,
                         cancellable,
                         NULL /*progress_callback*/,
                         NULL /*progress_user_data*/,
                         ytsg_contact_ft_op_cb,
                         (gpointer) op);

  g_hash_table_insert (priv->ft_cancellables,
                       g_file_get_path (file->file),
                       cancellable);

  return (file->atom | YTSG_ERROR_PENDING);
}

static void
ytsg_contact_ft_channel_ready_cb (TpChannel       *channel,
                                  GParamSpec      *pspec,
                                  YtsgCPendingFile *file)
{
  /*
   * NB: we cannot use here tp_is_proxy_prepared(), as the proxy flag is not
   *     yet set when this signal is emitted.
   */
  if (!tp_channel_is_ready (channel))
    return;

  g_debug ("The FT channel is ready");

  ytsg_c_dispatch_file (file);
  ytsg_c_pending_file_free (file);

  /*
   * This is one off.
   */
  g_signal_handlers_disconnect_by_func (channel,
                                    ytsg_contact_ft_channel_ready_cb, file);
}

static int
ytsg_contact_find_file_cb (gconstpointer a, gconstpointer b)
{
  const YtsgCPendingFile *A    = a;
  const char            *name = b;

  return g_strcmp0 (A->name, name);
}

static void
ytsg_contact_do_set_ft_channel (YtsgContact *item,
                                TpChannel    *channel,
                                const char   *name)
{
  YtsgContactPrivate *priv = item->priv;
  YtsgCPendingFile   *file;
  GList              *l;

  g_return_if_fail (!priv->disposed);

  if (!(l = g_queue_find_custom (priv->pending_files,
                                 name,
                                 ytsg_contact_find_file_cb)))
    {
      return;
    }

  file = l->data;

  file->ft_channel = channel;

  g_queue_remove (priv->pending_files, file);

  /*
   * If the channel is ready, push any pending messages.
   */
  if (tp_proxy_is_prepared (channel, TP_CHANNEL_FEATURE_CORE))
    {
      ytsg_c_dispatch_file (file);
      ytsg_c_pending_file_free (file);
    }
  else
    {
      g_signal_connect (channel, "notify::channel-ready",
                        G_CALLBACK (ytsg_contact_ft_channel_ready_cb),
                        file);
    }
}

static void
ytsg_contact_ft_filename_cb (TpProxy      *proxy,
                             const GValue *value,
                             const GError *error,
                             gpointer      data,
                             GObject      *weak_object)
{
  YtsgContact *item = data;
  const char  *name;

  if (error)
    {
      g_warning ("Could not get filename: %s", error->message);
      return;
    }

  name = g_value_get_string (value);

  ytsg_contact_do_set_ft_channel (item, (TpChannel*)proxy, name);
}

/*
 * ytsg_contact_set_ft_channel:
 * @item: #YtsgContact,
 * @channel: #TpChannel
 *
 * Sets the channel file transfer item for this item.
 */
void
_ytsg_contact_set_ft_channel (YtsgContact *item, TpChannel *channel)
{
  YtsgContactPrivate *priv = item->priv;

  g_return_if_fail (!priv->disposed);

  tp_cli_dbus_properties_call_get (channel,
                                   -1,
                                   TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,
                                   "Filename",
                                   ytsg_contact_ft_filename_cb,
                                   item,
                                   NULL,
                                   (GObject*) item);
}

static void
ytsg_contact_create_ft_channel_cb (TpConnection *proxy,
                                   const char   *channel,
                                   GHashTable   *properties,
                                   const GError *error,
                                   gpointer      data,
                                   GObject      *weak_object)
{
  if (error)
    {
      YtsgContact      *item = YTSG_CONTACT (weak_object);
      YtsgCPendingFile *file = data;
      YtsgError         e    = (YTSG_ERROR_NO_ROUTE | file->atom);

      ytsg_client_emit_error (item->priv->client, e);

      g_warning ("Failed to open channel: %s", error->message);
    }
}

/**
 * ytsg_contact_send_file:
 * @item: #YtsgContact,
 * @gfile: #GFile to send
 *
 * Sends file to the contact represented by this item. The caller can safely
 * release reference on the supplied #GFile after calling this function.
 *
 * Return value: Returns %YTSG_ERROR_SUCCESS on success, return value
 * %YTSG_ERROR_NOT_ALLOWED indicates that the current client is not mutually
 * approved to exchange files with the item. %YTSG_ERROR_PENDING is returned if
 * the execution of the command has to be deferred until the communication
 * channel is ready; in this case the file will be automatically send at the
 * appropriate time, and any errors, or eventaul success, will be indicated by
 * emitting the #YtsgClient::error signal at that time.
 */
YtsgError
ytsg_contact_send_file (const YtsgContact *item, GFile *gfile)
{
  YtsgContactPrivate *priv;
  guint32             atom;
  const char         *content_type = "binary";
  GFileInfo          *finfo;
  GError             *error = NULL;
  YtsgCPendingFile   *file;
  GHashTable         *request;
  TpConnection       *conn;
  guint               handle;

  g_return_val_if_fail (YTSG_IS_CONTACT (item) && gfile,
                        YTSG_ERROR_INVALID_PARAMETER);

  priv = item->priv;

  g_return_val_if_fail (!priv->disposed, YTSG_ERROR_OBJECT_DISPOSED);

  if (!(priv->subscription & YTSG_SUBSCRIPTION_APPROVED))
    return YTSG_ERROR_NOT_ALLOWED;

  finfo = g_file_query_info (gfile,
                             "standard::*",
                             0,
                             NULL,
                             &error);

  if (error)
    {
      g_warning ("Unable to query file, %s", error->message);
      g_clear_error (&error);
      return YTSG_ERROR_INVALID_PARAMETER;
    }

  atom = ytsg_error_new_atom ();

  g_debug ("Sending file with atom %d", atom);

  file = ytsg_c_pending_file_new (item, gfile,
                                  g_file_info_get_display_name (finfo), atom);

  g_queue_push_tail (priv->pending_files, file);

  conn = tp_contact_get_connection (priv->tp_contact);
  handle = tp_contact_get_handle (priv->tp_contact);

  request = tp_asv_new (TP_PROP_CHANNEL_CHANNEL_TYPE,
                        G_TYPE_STRING,
                        TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,

                        TP_PROP_CHANNEL_TARGET_HANDLE_TYPE,
                        G_TYPE_UINT,
                        TP_HANDLE_TYPE_CONTACT,

                        TP_PROP_CHANNEL_TARGET_HANDLE,
                        G_TYPE_UINT,
                        handle,

                        TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_CONTENT_TYPE,
                        G_TYPE_STRING,
                        content_type,

                        TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_FILENAME,
                        G_TYPE_STRING,
                        g_file_info_get_display_name (finfo),

                        TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_SIZE,
                        G_TYPE_UINT64,
                        g_file_info_get_size (finfo),
                        NULL);

  tp_cli_connection_interface_requests_call_create_channel (conn,
                                            -1,
                                            request,
                                            ytsg_contact_create_ft_channel_cb,
                                            file,
                                            NULL,
                                            (GObject*)item);

  g_object_unref (finfo);

  return (atom & YTSG_ERROR_PENDING);
}

/**
 * ytsg_contact_cancel_file:
 * @item: #YtsgContact,
 * @gfile: #GFile to cancel
 *
 * Cancels file transfer in progress.
 *
 * Return value: returns %TRUE if the transfer was successfully cancelled; if
 * the tansfer was already completed, returns %FALSE.
 */
gboolean
ytsg_contact_cancel_file (const YtsgContact *item, GFile *gfile)
{
  YtsgContactPrivate *priv;
  GCancellable       *cancellable;
  char               *path;

  g_return_val_if_fail (YTSG_IS_CONTACT (item) && gfile, FALSE);

  priv = item->priv;

  g_return_val_if_fail (!priv->disposed, FALSE);

  if (!(path = g_file_get_path (gfile)))
    return FALSE;

  if (!(cancellable = g_hash_table_lookup (priv->ft_cancellables, path)))
    {
      g_free (path);
      return FALSE;
    }

  g_cancellable_cancel (cancellable);

  g_hash_table_remove (priv->ft_cancellables, path);

  g_free (path);

  return TRUE;
}

