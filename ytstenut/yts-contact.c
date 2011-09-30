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
 * SECTION:yts-contact
 * @short_description: Represents a device connected to the
 * Ytstenut mesh.
 *
 * #YtsContact represents a known device in the Ytstenut application mesh,
 * and provides access to any services (#YtsService) available on the device.
 */

#include <string.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/connection.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/channel.h>
#include <telepathy-glib/util.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/channel.h>

#include "empathy-tp-file.h"
#include "yts-client-internal.h"
#include "yts-contact-internal.h"
#include "yts-debug.h"
#include "yts-enum-types.h"
#include "yts-error.h"
#include "yts-marshal.h"
#include "yts-proxy-service-internal.h"
#include "yts-service-internal.h"
#include "yts-types.h"
#include "config.h"

static void yts_c_pending_file_free (gpointer file);

static void yts_contact_dispose (GObject *object);
static void yts_contact_finalize (GObject *object);
static void yts_contact_constructed (GObject *object);
static void yts_contact_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec);
static void yts_contact_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec);

static void yts_contact_avatar_file_cb (TpContact   *contact,
                                         GParamSpec  *param,
                                         YtsContact *ycontact);

G_DEFINE_TYPE (YtsContact, yts_contact, G_TYPE_OBJECT);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_CONTACT, YtsContactPrivate))

typedef struct {
  const char   *jid;
  GHashTable   *services;   /* hash of YtsService instances */
  TpContact    *tp_contact; /* TpContact associated with YtsContact */

  char         *icon_token; /* token identifying this contacts avatar */

  YtsClient   *client;     /* back-reference to the client that owns us */
  YtsPresence  presence;   /* presence state of this client */

  GQueue       *pending_files;    /* files dispatched before channel open */
  GHashTable   *ft_cancellables;

  guint disposed : 1;
} YtsContactPrivate;

enum
{
  SEND_MESSAGE,
  SERVICE_ADDED,
  SERVICE_REMOVED,

  N_SIGNALS,
};

enum
{
  PROP_0,
  PROP_JID,
  PROP_ICON,
  PROP_CLIENT,
  PROP_PRESENCE,
  PROP_TP_CONTACT,

  PROP_LAST
};

static guint signals[N_SIGNALS] = {0};
static GParamSpec *properties[PROP_LAST];

static void
_service_send_message (YtsService   *service,
                       YtsMetadata  *message,
                       YtsContact   *self)
{
  g_signal_emit (self, signals[SEND_MESSAGE], 0,
                 service, message);
}

static void
yts_contact_service_added (YtsContact *self,
                           YtsService *service)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  const char         *uid  = yts_service_get_service_id (service);

  g_return_if_fail (uid && *uid);
  g_return_if_fail (!g_hash_table_lookup (priv->services, uid));

  g_hash_table_insert (priv->services,
                       g_strdup (uid),
                       g_object_ref (service));

  g_signal_connect (service, "send-message",
                    G_CALLBACK (_service_send_message), self);
}

static void
yts_contact_service_removed (YtsContact *self,
                             YtsService *service)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  const char         *uid  = yts_service_get_service_id (service);

  g_return_if_fail (uid && *uid);

  yts_client_cleanup_service (priv->client, service);

  if (!g_hash_table_remove (priv->services, uid))
    g_warning (G_STRLOC ": unknown service with uid %s", uid);

  g_signal_handlers_disconnect_by_func (service,
                                        _service_send_message,
                                        self);
}

static void
yts_contact_class_init (YtsContactClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsContactPrivate));

  object_class->dispose      = yts_contact_dispose;
  object_class->finalize     = yts_contact_finalize;
  object_class->constructed  = yts_contact_constructed;
  object_class->get_property = yts_contact_get_property;
  object_class->set_property = yts_contact_set_property;

  klass->service_added       = yts_contact_service_added;
  klass->service_removed     = yts_contact_service_removed;

  /**
   * YtsContact:icon:
   *
   * Icon for this item.
   *
   * The property holds a GFile* pointing to the latest
   * cached image.
   *
   * Since: 0.1
   */
  properties[PROP_ICON] = g_param_spec_object ("icon",
                                               "Icon",
                                               "Icon",
                                               G_TYPE_FILE,
                                               G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_ICON,
                                   properties[PROP_ICON]);

  /**
   * YtsContact:client:
   *
   * #YtsClient that owns the roster
   */
  properties[PROP_CLIENT] = g_param_spec_object ("client",
                                               "Client",
                                               "Client",
                                               YTS_TYPE_CLIENT,
                               G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CLIENT,
                                   properties[PROP_CLIENT]);

  /**
   * YtsContact:presence:
   *
   * #YtsPresence state for this contact
   */
  properties[PROP_PRESENCE] = g_param_spec_enum ("presence",
                                                 "YtsPresence",
                                                 "YtsPresence",
                                                 YTS_TYPE_PRESENCE,
                                                 YTS_PRESENCE_UNAVAILABLE,
                                                 G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_PRESENCE,
                                   properties[PROP_PRESENCE]);

  /**
   * YtsContact:jid:
   *
   * The jid of this contact
   */
  properties[PROP_JID] = g_param_spec_string ("jid",
                                              "jid",
                                              "jid",
                                              NULL,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_JID,
                                   properties[PROP_JID]);

  /**
   * YtsContact:tp-contact:
   *
   * #TpContact of this item.
   */
  properties[PROP_TP_CONTACT] = g_param_spec_object ("tp-contact",
                                                     "TP Contact",
                                                     "TP Contact",
                                                     TP_TYPE_CONTACT,
                                                     G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_TP_CONTACT,
                                   properties[PROP_TP_CONTACT]);

  /*
   * Internal signal, should not be considered by language bindings at this
   * time. Maybe in the future when we allow for custom contact subclasses.
   */
  signals[SEND_MESSAGE] = g_signal_new ("send-message",
                                        G_TYPE_FROM_CLASS (object_class),
                                        G_SIGNAL_RUN_LAST,
                                        0, NULL, NULL,
                                        yts_marshal_VOID__OBJECT_OBJECT,
                                        G_TYPE_NONE, 2,
                                        YTS_TYPE_SERVICE,
                                        YTS_TYPE_METADATA);

  /**
   * YtsContact::service-added:
   * @contact: the contact which received the signal
   * @service: the service
   *
   * The ::service-added signal is emitted when a new services is added to
   * the contact.
   *
   * Since: 0.1
   */
  signals[SERVICE_ADDED] =
    g_signal_new ("service-added",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (YtsContactClass, service_added),
                  NULL, NULL,
                  yts_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTS_TYPE_SERVICE);

  /**
   * YtsContact::service-removed:
   * @contact: the contact which received the signal
   * @service: the service
   *
   * The ::service-removed signal is emitted when a services is removed from
   * the contact.
   *
   * Since: 0.1
   */
  signals[SERVICE_REMOVED] =
    g_signal_new ("service-removed",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsContactClass, service_removed),
                  NULL, NULL,
                  yts_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTS_TYPE_SERVICE);
}

static void
yts_contact_presence_cb (TpContact    *tp_contact,
                          guint         type,
                          gchar        *status,
                          gchar        *message,
                          YtsContact  *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  YtsPresence        presence;

  YTS_NOTE (CONTACT, "Presence for %s changed: %s [%s]",
             tp_contact_get_identifier (tp_contact), status, message);


  /*
   * The Ytstenut presence differs from the human IM presence; basically, we
   * only care whether the user is online or offline.
   */
  switch (type)
    {
    case TP_CONNECTION_PRESENCE_TYPE_AVAILABLE:
    case TP_CONNECTION_PRESENCE_TYPE_AWAY:
    case TP_CONNECTION_PRESENCE_TYPE_EXTENDED_AWAY:
    case TP_CONNECTION_PRESENCE_TYPE_BUSY:
    case TP_CONNECTION_PRESENCE_TYPE_HIDDEN:
    default:
      presence = YTS_PRESENCE_AVAILABLE;
      break;

    case TP_CONNECTION_PRESENCE_TYPE_UNKNOWN:
    case TP_CONNECTION_PRESENCE_TYPE_OFFLINE:
    case TP_CONNECTION_PRESENCE_TYPE_ERROR:
      presence = YTS_PRESENCE_UNAVAILABLE;
    }

  if (priv->presence != presence)
    {
      priv->presence = presence;
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PRESENCE]);
    }
}

static void
yts_contact_tp_contact_cb (TpConnection       *connection,
                            guint               n_contacts,
                            TpContact * const  *contacts,
                            const char * const *requesed_ids,
                            GHashTable         *failed_id_errors,
                            const GError       *error,
                            gpointer            data,
                            GObject            *object)
{
  YtsContact        *self = YTS_CONTACT (object);
  YtsContactPrivate *priv = GET_PRIVATE (self);

  if (error)
    {
      g_warning (G_STRLOC ": %s: %s", __FUNCTION__, error->message);
      return;
    }

  YTS_NOTE (CONTACT, "Got TpContact for %s", priv->jid);

  priv->tp_contact = g_object_ref (contacts[0]);

  /*
   * TODO -- do we need to do this ? I think all services will disappear if
   * if the contact goes off line.
   */
  tp_g_signal_connect_object (priv->tp_contact, "presence-changed",
                              G_CALLBACK (yts_contact_presence_cb),
                              self, 0);

  tp_g_signal_connect_object (priv->tp_contact, "notify::avatar-file",
                              G_CALLBACK (yts_contact_avatar_file_cb),
                              self, 0);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TP_CONTACT]);
}

static void
yts_contact_constructed (GObject *object)
{
  YtsContact        *self = YTS_CONTACT (object);
  YtsContactPrivate *priv = GET_PRIVATE (self);
  TpConnection      *connection;
  TpContactFeature   features[] = { TP_CONTACT_FEATURE_PRESENCE,
                                    TP_CONTACT_FEATURE_CONTACT_INFO,
                                    TP_CONTACT_FEATURE_AVATAR_DATA,
                                    TP_CONTACT_FEATURE_CAPABILITIES};

  if (G_OBJECT_CLASS (yts_contact_parent_class)->constructed)
    G_OBJECT_CLASS (yts_contact_parent_class)->constructed (object);

  connection = yts_client_get_connection (priv->client);

  g_assert (connection);

  YTS_NOTE (CONTACT, "Requesting TpContact for %s", priv->jid);

  tp_connection_get_contacts_by_id (connection,
                                    1,
                                    &priv->jid,
                                    G_N_ELEMENTS (features),
                                    (const TpContactFeature *)&features,
                                    yts_contact_tp_contact_cb,
                                    self,
                                    NULL,
                                    (GObject*) object);
}

static void
yts_contact_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  YtsContact        *self = YTS_CONTACT (object);
  YtsContactPrivate *priv = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_JID:
      g_value_set_string (value, priv->jid);
      break;
    case PROP_ICON:
      {
        GFile *file = yts_contact_get_icon (self, NULL);
        g_value_take_object (value, file);

        g_warning ("Should use ytst_contact_get_icon() instead of querying "
                   "YstgContact:icon");
      }
      break;
    case PROP_PRESENCE:
      g_value_set_enum (value, priv->presence);
      break;
    case PROP_TP_CONTACT:
      g_value_set_object (value, priv->tp_contact);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yts_contact_avatar_file_cb (TpContact   *contact,
                            GParamSpec  *param,
                            YtsContact  *self)
{
  YtsContactPrivate *priv  = GET_PRIVATE (self);
  const char         *token = tp_contact_get_avatar_token (contact);

  if ((priv->icon_token && token && !strcmp (priv->icon_token, token)) ||
      (!priv->icon_token && !token))
    {
      return;
    }

  g_free (priv->icon_token);
  priv->icon_token = g_strdup (token);

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ICON]);
}

static void
yts_contact_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  YtsContactPrivate *priv = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_JID:
      priv->jid = g_intern_string (g_value_get_string (value));
      break;
    case PROP_CLIENT:
      priv->client = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yts_contact_init (YtsContact *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  priv->services = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          g_object_unref);

  priv->pending_files = g_queue_new ();

  priv->ft_cancellables  = g_hash_table_new_full (g_str_hash,
                                                  g_str_equal,
                                                  g_free,
                                                  g_object_unref);
}

static void
yts_contact_dispose (GObject *object)
{
  YtsContactPrivate *priv = GET_PRIVATE (object);

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  if (priv->client) {
    yts_client_cleanup_contact (priv->client, YTS_CONTACT (object));
    priv->client = NULL;
  }

  g_hash_table_destroy (priv->services);
  priv->services = NULL;

  if (priv->tp_contact)
    {
      g_object_unref (priv->tp_contact);
      priv->tp_contact = NULL;
    }

  G_OBJECT_CLASS (yts_contact_parent_class)->dispose (object);
}

static void
yts_contact_finalize (GObject *object)
{
  YtsContactPrivate *priv = GET_PRIVATE (object);

  g_free (priv->icon_token);

  g_queue_foreach (priv->pending_files, (GFunc)yts_c_pending_file_free, NULL);
  g_queue_free (priv->pending_files);

  G_OBJECT_CLASS (yts_contact_parent_class)->finalize (object);
}

/**
 * yts_contact_get_id:
 * @contact: #YtsContact
 *
 * Retrieves the jabber identifier of this contact.
 *
 * Return value: (transfer none): The jid of this contact.
 */
const char *
yts_contact_get_id (YtsContact const *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CONTACT (self), NULL);

  return priv->jid;
}

/**
 * yts_contact_get_name:
 * @contact: #YtsContact
 *
 * Retrieves human readable name of this client
 *
 * Return value: (transfer none): The name of this contact.
 */
const char *
yts_contact_get_name (YtsContact const *self)
{
  /* FIXME -- */
  g_warning (G_STRLOC ": %s is not implemented", __FUNCTION__);
  return NULL;
}

/**
 * yts_contact_get_icon:
 * @contact: #YtsContact
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
yts_contact_get_icon (YtsContact const   *self,
                      const char        **mime)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  GFile               *file;

  g_return_val_if_fail (YTS_IS_CONTACT (self), NULL);

  g_return_val_if_fail (!priv->disposed, NULL);

  if (!(file = tp_contact_get_avatar_file (priv->tp_contact)))
    return NULL;

  if (mime)
    *mime = tp_contact_get_avatar_mime_type (priv->tp_contact);
  return g_object_ref (file);
}

YtsContact *
yts_contact_new (YtsClient *client, const char *jid)
{
  g_return_val_if_fail (YTS_IS_CLIENT (client), NULL);
  g_return_val_if_fail (jid && *jid, NULL);

  return g_object_new (YTS_TYPE_CONTACT,
                       "client", client,
                       "jid",    jid,
                       NULL);
}

/**
 * yts_contact_get_tp_contact:
 * @contact: #YtsContact
 *
 * Retrieves the #TpContact associated with this #YtsContact object; can be
 * %NULL. When the #TpContact is available, the YtsContact::notify-tp-contact
 * signal will be emitted.
 *
 * Return value (transfer none): The associated #TpContact.
 */
TpContact *const
yts_contact_get_tp_contact (YtsContact const *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CONTACT (self), NULL);

  return priv->tp_contact;
}

static gboolean
yts_contact_find_cancellable_cb (gpointer key,
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
  YtsContact *item;
  guint32      atom;

} YtsCFtOp;

static YtsCFtOp *
yts_c_ft_op_new (YtsContact *item, guint32 atom)
{
  YtsCFtOp *o = g_slice_new (YtsCFtOp);

  o->item = item;
  o->atom = atom;

  return o;
}

static void
yts_c_ft_op_free (gpointer op)
{
  YtsCFtOp *o = op;

  g_slice_free (YtsCFtOp, o);
}

static void
yts_contact_ft_op_cb (EmpathyTpFile *tp_file,
                       const GError  *error,
                       gpointer       data)
{
  GCancellable       *cancellable;
  YtsError           e;
  YtsCFtOp          *op     = data;
  YtsContact        *self   = op->item;
  YtsContactPrivate *priv   = GET_PRIVATE (self);
  YtsClient         *client = priv->client;
  guint32             atom   = op->atom;

  if (error)
    {
      e = (atom | YTS_ERROR_UNKNOWN);
      g_warning ("File transfer to %s failed: %s",
                 yts_contact_get_id (self), error->message);
    }
  else
    e = (atom | YTS_ERROR_SUCCESS);

  if ((cancellable = empathy_tp_file_get_cancellable (tp_file)))
    {
      gpointer the_thing = cancellable;

      if (g_hash_table_find (priv->ft_cancellables,
                             yts_contact_find_cancellable_cb,
                             &the_thing))
        {
          g_hash_table_remove (priv->ft_cancellables, the_thing);
        }
    }

  yts_c_ft_op_free (op);
  yts_client_emit_error (client, e);
}

typedef struct
{
  const YtsContact *item;
  TpChannel         *ft_channel;
  GFile             *file;
  char              *name;
  guint32            atom;

} YtsCPendingFile;

static YtsCPendingFile *
yts_c_pending_file_new (const YtsContact *item,
                         GFile              *gfile,
                         const char         *name,
                         guint32             atom)
{
  YtsCPendingFile *m = g_slice_new (YtsCPendingFile);

  m->item       = item;
  m->name       = g_strdup (name);
  m->file       = g_object_ref (gfile);
  m->atom       = atom;
  m->ft_channel = NULL;

  return m;
}

static void
yts_c_pending_file_free (gpointer file)
{
  YtsCPendingFile *m = file;

  g_object_unref (m->file);
  g_free (m->name);
  g_slice_free (YtsCPendingFile, m);
}

static YtsError
yts_c_dispatch_file (YtsCPendingFile *file)
{
  YtsContactPrivate *priv;
  EmpathyTpFile      *tp_file;
  GCancellable       *cancellable;
  YtsCFtOp          *op;

  g_return_val_if_fail (file && file->item && file->ft_channel && file->file,
                        YTS_ERROR_INVALID_PARAMETER);

  priv = GET_PRIVATE (file->item);

  op = yts_c_ft_op_new ((YtsContact*)file->item, file->atom);

  tp_file = empathy_tp_file_new (file->ft_channel, FALSE);

  cancellable = g_cancellable_new ();

  empathy_tp_file_offer (tp_file,
                         file->file,
                         cancellable,
                         NULL /*progress_callback*/,
                         NULL /*progress_user_data*/,
                         yts_contact_ft_op_cb,
                         (gpointer) op);

  g_hash_table_insert (priv->ft_cancellables,
                       g_file_get_path (file->file),
                       cancellable);

  return (file->atom | YTS_ERROR_PENDING);
}

static void
yts_contact_ft_channel_ready_cb (TpChannel       *channel,
                                  GParamSpec      *pspec,
                                  YtsCPendingFile *file)
{
  /*
   * NB: we cannot use here tp_is_proxy_prepared(), as the proxy flag is not
   *     yet set when this signal is emitted.
   */
  if (!tp_channel_is_ready (channel))
    return;

  YTS_NOTE (FILE_TRANSFER, "The FT channel is ready");

  yts_c_dispatch_file (file);
  yts_c_pending_file_free (file);

  /*
   * This is one off.
   */
  g_signal_handlers_disconnect_by_func (channel,
                                    yts_contact_ft_channel_ready_cb, file);
}

static int
yts_contact_find_file_cb (gconstpointer a, gconstpointer b)
{
  const YtsCPendingFile *A    = a;
  const char            *name = b;

  return g_strcmp0 (A->name, name);
}

static void
yts_contact_do_set_ft_channel (YtsContact *self,
                                TpChannel    *channel,
                                const char   *name)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  YtsCPendingFile   *file;
  GList              *l;

  g_return_if_fail (!priv->disposed);

  if (!(l = g_queue_find_custom (priv->pending_files,
                                 name,
                                 yts_contact_find_file_cb)))
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
      yts_c_dispatch_file (file);
      yts_c_pending_file_free (file);
    }
  else
    {
      g_signal_connect (channel, "notify::channel-ready",
                        G_CALLBACK (yts_contact_ft_channel_ready_cb),
                        file);
    }
}

static void
yts_contact_ft_filename_cb (TpProxy      *proxy,
                             const GValue *value,
                             const GError *error,
                             gpointer      data,
                             GObject      *weak_object)
{
  YtsContact *item = data;
  const char  *name;

  if (error)
    {
      g_warning ("Could not get filename: %s", error->message);
      return;
    }

  name = g_value_get_string (value);

  yts_contact_do_set_ft_channel (item, (TpChannel*)proxy, name);
}

/*
 * yts_contact_set_ft_channel:
 * @item: #YtsContact,
 * @channel: #TpChannel
 *
 * Sets the channel file transfer item for this item.
 */
void
yts_contact_set_ft_channel (YtsContact  *self,
                            TpChannel   *channel)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_return_if_fail (!priv->disposed);

  tp_cli_dbus_properties_call_get (channel,
                                   -1,
                                   TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,
                                   "Filename",
                                   yts_contact_ft_filename_cb,
                                   self,
                                   NULL,
                                   (GObject*) self);
}

static void
yts_contact_create_ft_channel_cb (TpConnection *proxy,
                                   const char   *channel,
                                   GHashTable   *properties,
                                   const GError *error,
                                   gpointer      data,
                                   GObject      *weak_object)
{
  YtsContact        *self = YTS_CONTACT (weak_object);
  YtsContactPrivate *priv = GET_PRIVATE (self);

  if (error)
    {
      YtsCPendingFile *file = data;
      YtsError         e    = (YTS_ERROR_NO_ROUTE | file->atom);

      yts_client_emit_error (priv->client, e);

      g_warning ("Failed to open channel: %s", error->message);
    }
}

static YtsError
yts_contact_do_send_file (YtsContact  *self,
                          GFile       *gfile,
                          guint32      atom)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  const char         *content_type = "binary";
  GFileInfo          *finfo;
  GError             *error = NULL;
  YtsCPendingFile   *file;
  GHashTable         *request;
  TpConnection       *conn;
  guint               handle;

  g_return_val_if_fail (YTS_IS_CONTACT (self) && gfile,
                        YTS_ERROR_INVALID_PARAMETER);

  g_return_val_if_fail (!priv->disposed, YTS_ERROR_OBJECT_DISPOSED);

  if (!priv->tp_contact)
    return YTS_ERROR_NO_ROUTE;

  finfo = g_file_query_info (gfile,
                             "standard::*",
                             0,
                             NULL,
                             &error);

  if (error)
    {
      g_warning ("Unable to query file, %s", error->message);
      g_clear_error (&error);
      return YTS_ERROR_INVALID_PARAMETER;
    }

  file = yts_c_pending_file_new (self, gfile,
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
                                            yts_contact_create_ft_channel_cb,
                                            file,
                                            NULL,
                                            (GObject*)self);

  g_object_unref (finfo);

  return (atom & YTS_ERROR_PENDING);
}

struct YtsContactFTData
{
  GFile   *gfile;
  guint32  atom;
};

static void
yts_contact_notify_tp_contact_cb (YtsContact              *contact,
                                   GParamSpec               *pspec,
                                   struct YtsContactFTData *d)
{
  YTS_NOTE (FILE_TRANSFER, "Contact ready");
  yts_contact_do_send_file (contact, d->gfile, d->atom);

  g_signal_handlers_disconnect_by_func (contact,
                                        yts_contact_notify_tp_contact_cb,
                                        d);

  g_object_unref (d->gfile);
  g_free (d);
}


/**
 * yts_contact_send_file:
 * @item: #YtsContact,
 * @gfile: #GFile to send
 *
 * Sends file to the contact represented by this item. The caller can safely
 * release reference on the supplied #GFile after calling this function.
 *
 * Return value: Returns %YTS_ERROR_SUCCESS on success, return value
 * %YTS_ERROR_NOT_ALLOWED indicates that the current client is not mutually
 * approved to exchange files with the item. %YTS_ERROR_PENDING is returned if
 * the execution of the command has to be deferred until the communication
 * channel is ready; in this case the file will be automatically send at the
 * appropriate time, and any errors, or eventaul success, will be indicated by
 * emitting the #YtsClient::error signal at that time.
 */
YtsError
yts_contact_send_file (YtsContact *self,
                       GFile      *gfile)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  GFileInfo          *finfo;
  GError             *error = NULL;
  guint32             atom;

  g_return_val_if_fail (YTS_IS_CONTACT (self) && gfile,
                        YTS_ERROR_INVALID_PARAMETER);

  g_return_val_if_fail (!priv->disposed, YTS_ERROR_OBJECT_DISPOSED);

  finfo = g_file_query_info (gfile,
                             "standard::*",
                             0,
                             NULL,
                             &error);

  if (error)
    {
      g_warning ("Unable to query file, %s", error->message);
      g_clear_error (&error);
      return YTS_ERROR_INVALID_PARAMETER;
    }

  g_object_unref (finfo);

  /*
   * NB: the atom through this file is used in its shifted, rather than
   * canonical form, so it can be just ored with an error code.
   */
  atom = (yts_error_new_atom () << 16);

  YTS_NOTE (FILE_TRANSFER, "Sending file with atom %d", atom);

  if (priv->tp_contact)
    {
      yts_contact_do_send_file (self, gfile, atom);
    }
  else
    {
      struct YtsContactFTData *d = g_new (struct YtsContactFTData, 1);

      d->gfile = g_object_ref (gfile);
      d->atom  = atom;

      YTS_NOTE (FILE_TRANSFER,
                 "Contact not ready, postponing message file transfer");

      g_signal_connect (self, "notify::tp-contact",
                        G_CALLBACK (yts_contact_notify_tp_contact_cb),
                        d);
    }

  return (atom & YTS_ERROR_PENDING);
}

/**
 * yts_contact_cancel_file:
 * @item: #YtsContact,
 * @gfile: #GFile to cancel
 *
 * Cancels file transfer in progress.
 *
 * Return value: returns %TRUE if the transfer was successfully cancelled; if
 * the tansfer was already completed, returns %FALSE.
 */
bool
yts_contact_cancel_file (YtsContact *self,
                         GFile      *gfile)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  GCancellable       *cancellable;
  char               *path;

  g_return_val_if_fail (YTS_IS_CONTACT (self) && gfile, FALSE);

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

void
yts_contact_add_service (YtsContact *self,
                         YtsService *service)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  /*
   * Emit the signal; the run-first signal closure will do the rest
   */
  YTS_NOTE (CONTACT, "New service %s on %s",
             yts_service_get_id (service),
             priv->jid);

  g_signal_emit (self, signals[SERVICE_ADDED], 0, service);
}

void
yts_contact_remove_service_by_uid (YtsContact *self,
                                   const char *uid)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  YtsService        *service;

  g_return_if_fail (uid && *uid);

  /*
   * Look up the service and emit the service-removed signal; the signal closure
   *  will take care of the rest.
   */
  service = g_hash_table_lookup (priv->services, uid);
  if (service)
    {
      g_signal_emit (self, signals[SERVICE_REMOVED], 0, service);
    }
  else
    {
      g_warning ("%s : Trying to remove service %s but not found?!",
                 G_STRLOC,
                 uid);
    }
}

bool
yts_contact_is_empty (YtsContact *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  return (g_hash_table_size (priv->services) == 0);
}

bool
yts_contact_dispatch_event (YtsContact  *self,
                             char const   *capability,
                             char const   *aspect,
                             GVariant     *arguments)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  char const      *service_uid;
  YtsService     *service;
  GHashTableIter   iter;
  gboolean         dispatched = FALSE;

  g_return_val_if_fail (YTS_IS_CONTACT (self), FALSE);

  g_hash_table_iter_init (&iter, priv->services);
  while (g_hash_table_iter_next (&iter,
                                 (void **) &service_uid,
                                 (void **) &service)) {
    if (YTS_IS_PROXY_SERVICE (service) &&
        yts_capability_has_fqc_id (YTS_CAPABILITY (service), capability)) {

      /* Dispatch to all matching services, be happy if one of them accepts. */
      dispatched = dispatched ||
              yts_proxy_service_dispatch_event (YTS_PROXY_SERVICE (service),
                                                 capability,
                                                 aspect,
                                                 arguments);
    }
  }
  return dispatched;
}

bool
yts_contact_dispatch_response (YtsContact *self,
                                char const  *capability,
                                char const  *invocation_id,
                                GVariant    *response)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  char const      *service_uid;
  YtsService     *service;
  GHashTableIter   iter;
  gboolean         dispatched = FALSE;

  g_return_val_if_fail (YTS_IS_CONTACT (self), FALSE);

  g_hash_table_iter_init (&iter, priv->services);
  while (g_hash_table_iter_next (&iter,
                                 (void **) &service_uid,
                                 (void **) &service)) {
    if (YTS_IS_PROXY_SERVICE (service) &&
        yts_capability_has_fqc_id (YTS_CAPABILITY (service), capability)) {

      /* Invocations are unique, so just go home after delivery. */
      dispatched =
            yts_proxy_service_dispatch_response (YTS_PROXY_SERVICE (service),
                                                  capability,
                                                  invocation_id,
                                                  response);
      if (dispatched)
        break;
    }
  }
  return dispatched;
}

void
yts_contact_update_service_status (YtsContact *self,
                                   char const *service_id,
                                   char const *fqc_id,
                                   char const *status_xml)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  YtsService *service;

  service = g_hash_table_lookup (priv->services, service_id);
  g_return_if_fail (service);

  yts_service_update_status (service, fqc_id, status_xml);
}

