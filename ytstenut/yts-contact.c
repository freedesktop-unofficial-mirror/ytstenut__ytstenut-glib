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

#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/connection.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/channel.h>
#include <telepathy-glib/util.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/channel.h>

#include "empathy-tp-file.h"
#include "yts-capability.h"
#include "yts-contact-impl.h"
#include "yts-contact-internal.h"
#include "yts-enum-types.h"
#include "yts-error.h"
#include "yts-marshal.h"
#include "yts-proxy-service-internal.h"
#include "yts-service-internal.h"
#include "config.h"

G_DEFINE_ABSTRACT_TYPE (YtsContact, yts_contact, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_CONTACT, YtsContactPrivate))

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0contact\0"G_STRLOC

/**
 * SECTION: yts-contact
 * @title: YtsContact
 * @short_description: Represents a device connected to the
 * Ytstenut mesh.
 *
 * #YtsContact represents a known device in the Ytstenut application mesh,
 * and provides access to any services (#YtsService) available on the device.
 */

typedef struct {
  GHashTable   *services;   /* hash of YtsService instances */
  TpContact    *tp_contact; /* TpContact associated with YtsContact */
  GQueue       *pending_files;    /* files dispatched before channel open */
  GHashTable   *ft_cancellables;
} YtsContactPrivate;

enum {
  SIG_SERVICE_ADDED,
  SIG_SERVICE_REMOVED,

  N_SIGNALS
};

enum {
  PROP_0,
  PROP_CONTACT_ID,
  PROP_NAME,
  PROP_TP_CONTACT,

  PROP_LAST
};

static unsigned _signals[N_SIGNALS] = { 0, };

typedef struct {
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
  guint32             atom   = op->atom;

  if (error)
    {
      e = (atom | YTS_ERROR_UNKNOWN);
      g_warning ("File transfer to %s failed: %s",
                 yts_contact_get_contact_id (self), error->message);
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
                         GFile              *file,
                         const char         *name,
                         guint32             atom)
{
  YtsCPendingFile *m = g_slice_new (YtsCPendingFile);

  m->item       = item;
  m->name       = g_strdup (name);
  m->file       = g_object_ref (file);
  m->atom       = atom;
  m->ft_channel = NULL;

  return m;
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
yts_c_pending_file_free (gpointer file)
{
  YtsCPendingFile *m = file;

  g_object_unref (m->file);
  g_free (m->name);
  g_slice_free (YtsCPendingFile, m);
}

static void
_tp_contact_notify_alias (GObject     *tp_contact,
                          GParamSpec  *pspec,
                          YtsContact  *self)
{
  g_object_notify (G_OBJECT (self), "name");
}

static void
_service_send_message (YtsService   *service,
                       YtsMetadata  *message,
                       YtsContact   *self)
{
  /* This is a bit of a hack, we require the non-abstract subclass to
   * implement this interface. */
  yts_contact_impl_send_message (YTS_CONTACT_IMPL (self), service, message);
}

static void
_service_added (YtsContact  *self,
                YtsService  *service,
                void        *data)
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
_service_removed (YtsContact  *self,
                  YtsService  *service,
                  void        *data)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  const char         *uid  = yts_service_get_service_id (service);

  g_return_if_fail (uid && *uid);

  if (!g_hash_table_remove (priv->services, uid))
    g_warning (G_STRLOC ": unknown service with uid %s", uid);

  g_signal_handlers_disconnect_by_func (service,
                                        _service_send_message,
                                        self);
}

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsContactPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_CONTACT_ID:
      g_value_set_string (value,
                          yts_contact_get_contact_id (YTS_CONTACT (object)));
      break;
    case PROP_NAME:
      g_value_set_string (value,
                          tp_contact_get_alias (priv->tp_contact));
      break;
    case PROP_TP_CONTACT:
      g_value_set_object (value, priv->tp_contact);
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
  YtsContactPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_TP_CONTACT: {
      g_return_if_fail (g_value_get_object (value));
      priv->tp_contact = g_value_dup_object (value);
      g_signal_connect (priv->tp_contact, "notify::alias",
                        G_CALLBACK (_tp_contact_notify_alias), object);
    } break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsContactPrivate *priv = GET_PRIVATE (object);

  if (priv->services) {
    g_hash_table_destroy (priv->services);
    priv->services = NULL;
  }

  // FIXME tie to tp_contact lifecycle
  if (priv->tp_contact) {
    g_object_unref (priv->tp_contact);
    priv->tp_contact = NULL;
  }

  G_OBJECT_CLASS (yts_contact_parent_class)->dispose (object);
}

static void
_finalize (GObject *object)
{
  YtsContactPrivate *priv = GET_PRIVATE (object);

  g_queue_foreach (priv->pending_files, (GFunc)yts_c_pending_file_free, NULL);
  g_queue_free (priv->pending_files);

  G_OBJECT_CLASS (yts_contact_parent_class)->finalize (object);
}

static void
yts_contact_class_init (YtsContactClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (YtsContactPrivate));

  object_class->dispose      = _dispose;
  object_class->finalize     = _finalize;
  object_class->get_property = _get_property;
  object_class->set_property = _set_property;

  /**
   * YtsContact:contact-id:
   *
   * The JID of this contact.
   */
  pspec = g_param_spec_string ("contact-id", "", "",
                               NULL,
                               G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_CONTACT_ID, pspec);

  /**
   * YtsContact:name:
   *
   * The display name of this contact.
   */
  pspec = g_param_spec_string ("name", "", "",
                               NULL,
                               G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_NAME, pspec);

  /**
   * YtsContact:tp-contact:
   *
   * #TpContact of this item.
   */
  pspec = g_param_spec_object ("tp-contact", "", "",
                               TP_TYPE_CONTACT,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_TP_CONTACT, pspec);

  /**
   * YtsContact::service-added:
   * @self: object which emitted the signal.
   * @service: the service
   *
   * The ::service-added signal is emitted when a new services is added to
   * the contact.
   *
   * Since: 0.1
   */
  _signals[SIG_SERVICE_ADDED] = g_signal_new ("service-added",
                                              G_TYPE_FROM_CLASS (object_class),
                                              G_SIGNAL_RUN_FIRST,
                                              0, NULL, NULL,
                                              yts_marshal_VOID__OBJECT,
                                              G_TYPE_NONE, 1,
                                              YTS_TYPE_SERVICE);

  /**
   * YtsContact::service-removed:
   * @self: object which emitted the signal.
   * @service: the service
   *
   * The ::service-removed signal is emitted when a services is removed from
   * the contact.
   *
   * Since: 0.1
   */
  _signals[SIG_SERVICE_REMOVED] = g_signal_new (
                                              "service-removed",
                                              G_TYPE_FROM_CLASS (object_class),
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
                                              yts_marshal_VOID__OBJECT,
                                              G_TYPE_NONE, 1,
                                              YTS_TYPE_SERVICE);
}

static void
yts_contact_init (YtsContact *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_signal_connect (self, "service-added",
                    G_CALLBACK (_service_added), NULL);
  g_signal_connect (self, "service-removed",
                    G_CALLBACK (_service_removed), NULL);

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

/**
 * yts_contact_get_contact_id:
 * @self: object on which to invoke this method.
 *
 * Retrieves the jabber identifier of this contact.
 *
 * Returns: (transfer none): The JID of this contact.
 */
const char *
yts_contact_get_contact_id (YtsContact const *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CONTACT (self), NULL);
  g_return_val_if_fail (priv->tp_contact, NULL);

  return tp_contact_get_identifier (priv->tp_contact);
}

/**
 * yts_contact_get_name:
 * @self: object on which to invoke this method.
 *
 * Retrieves human readable name of this contact. This has undefined semantics
 * with ytstenut, as there can be multiple services running under a single
 * account, and potentially use different names.
 *
 * Returns: (transfer none): The name of this contact.
 */
const char *
yts_contact_get_name (YtsContact const *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CONTACT (self), NULL);
  g_return_val_if_fail (priv->tp_contact, NULL);

  return tp_contact_get_alias (priv->tp_contact);
}

/**
 * yts_contact_get_tp_contact:
 * @self: object on which to invoke this method.
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

  g_message ("The FT channel is ready");

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
 * @self: object on which to invoke this method.
 * @channel: #TpChannel
 *
 * Sets the channel file transfer item for this item.
 */
void
yts_contact_set_ft_channel (YtsContact  *self,
                            TpChannel   *channel)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

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
  if (error)
    {
      YtsCPendingFile *pending_file = data;
      YtsError         e    = (YTS_ERROR_NO_ROUTE | pending_file->atom);

      g_critical ("Failed to open channel: %s", error->message);
    }
}

static YtsError
yts_contact_do_send_file (YtsContact  *self,
                          GFile       *file,
                          guint32      atom)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  const char         *content_type = "binary";
  GFileInfo          *finfo;
  GError             *error = NULL;
  YtsCPendingFile   *pending_file;
  GHashTable         *request;
  TpConnection       *conn;
  guint               handle;

  g_return_val_if_fail (YTS_IS_CONTACT (self) && file,
                        YTS_ERROR_INVALID_PARAMETER);

  g_return_val_if_fail (priv->tp_contact, YTS_ERROR_OBJECT_DISPOSED);

  if (!priv->tp_contact)
    return YTS_ERROR_NO_ROUTE;

  finfo = g_file_query_info (file,
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

  pending_file = yts_c_pending_file_new (self, file,
                                  g_file_info_get_display_name (finfo), atom);

  g_queue_push_tail (priv->pending_files, pending_file);

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
                                            pending_file,
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
  g_message ("Contact ready");
  yts_contact_do_send_file (contact, d->gfile, d->atom);

  g_signal_handlers_disconnect_by_func (contact,
                                        yts_contact_notify_tp_contact_cb,
                                        d);

  g_object_unref (d->gfile);
  g_free (d);
}


/**
 * yts_contact_send_file:
 * @self: object on which to invoke this method.
 * @file: #GFile to send
 *
 * Sends file to the contact represented by this item. The caller can safely
 * release reference on the supplied #GFile after calling this function.
 *
 * Returns: %true on success.
 */
bool
yts_contact_send_file (YtsContact  *self,
                       GFile       *file,
                       GError     **error_out)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  GFileInfo          *finfo;
  GError             *error = NULL;
  guint32             atom;

  g_return_val_if_fail (YTS_IS_CONTACT (self) && file, false);
  g_return_val_if_fail (priv->tp_contact, false);

  finfo = g_file_query_info (file,
                             "standard::*",
                             0,
                             NULL,
                             error_out);

  if (NULL == finfo)
    {
      char *path = g_file_get_path (file);
      if (error_out && *error_out)
        g_critical ("%s", (*error_out)->message);
      g_critical ("%s : Unable to query file, %s", G_STRLOC, path);
      g_free (path);
      return false;
    }

  g_object_unref (finfo);

  /*
   * NB: the atom through this file is used in its shifted, rather than
   * canonical form, so it can be just ored with an error code.
   */
  atom = (yts_error_new_atom () << 16);

  g_message ("Sending file with atom %d", atom);

  if (priv->tp_contact)
    {
      yts_contact_do_send_file (self, file, atom);
    }
  else
    {
      struct YtsContactFTData *d = g_new (struct YtsContactFTData, 1);

      d->gfile = g_object_ref (file);
      d->atom  = atom;

      g_message ("Contact not ready, postponing message file transfer");

      g_signal_connect (self, "notify::tp-contact",
                        G_CALLBACK (yts_contact_notify_tp_contact_cb),
                        d);
    }

  return true;
}

/**
 * yts_contact_cancel_file:
 * @self: object on which to invoke this method.
 * @file: #GFile to cancel
 *
 * Cancels file transfer in progress.
 *
 * Returns: %TRUE if the transfer was successfully cancelled; if
 * the tansfer was already completed, returns %FALSE.
 */
bool
yts_contact_cancel_file (YtsContact *self,
                         GFile      *file)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  GCancellable       *cancellable;
  char               *path;

  g_return_val_if_fail (YTS_IS_CONTACT (self) && file, FALSE);
  g_return_val_if_fail (priv->tp_contact, FALSE);

  if (!(path = g_file_get_path (file)))
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
  /*
   * Emit the signal; the run-first signal closure will do the rest
   */
  g_message ("New service %s on %s",
             yts_service_get_service_id (service),
             yts_contact_get_contact_id (self));

  g_signal_emit (self, _signals[SIG_SERVICE_ADDED], 0, service);
}

void
yts_contact_remove_service_by_id (YtsContact  *self,
                                  const char  *service_id)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  YtsService        *service;

  g_return_if_fail (service_id && *service_id);

  /*
   * Look up the service and emit the service-removed signal; the signal closure
   *  will take care of the rest.
   */
  service = g_hash_table_lookup (priv->services, service_id);
  if (service)
    {
      g_signal_emit (self, _signals[SIG_SERVICE_REMOVED], 0, service);
    }
  else
    {
      g_warning ("%s : Trying to remove service %s but not found?!",
                 G_STRLOC,
                 service_id);
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
  char const      *service_id;
  YtsService      *service;
  GHashTableIter   iter;
  bool             dispatched = FALSE;

  g_return_val_if_fail (YTS_IS_CONTACT (self), FALSE);

  g_hash_table_iter_init (&iter, priv->services);
  while (g_hash_table_iter_next (&iter,
                                 (void **) &service_id,
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
  char const      *service_id;
  YtsService      *service;
  GHashTableIter   iter;
  bool             dispatched = FALSE;

  g_return_val_if_fail (YTS_IS_CONTACT (self), FALSE);

  g_hash_table_iter_init (&iter, priv->services);
  while (g_hash_table_iter_next (&iter,
                                 (void **) &service_id,
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

/**
 * yts_contact_foreach_service:
 * @self: object on which to invoke this method.
 * @iterator: iterator function.
 * @user_data: context to pass to the iterator function.
 *
 * Iterate over @self's services.
 *
 * Returns: %true if all the services have been iterated.
 *
 * Since: 0.4
 */
bool
yts_contact_foreach_service (YtsContact                 *self,
                             YtsContactServiceIterator   iterator,
                             void                       *user_data)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  GHashTableIter   iter;
  char const      *service_id;
  YtsService      *service;
  bool             ret = true;

  g_return_val_if_fail (YTS_IS_CONTACT (self), false);
  g_return_val_if_fail (iterator, false);

  g_hash_table_iter_init (&iter, priv->services);
  while (ret &&
         g_hash_table_iter_next (&iter,
                                 (void **) &service_id,
                                 (void **) &service)) {
    ret = iterator (self, service_id, service, user_data);
  }

  return ret;
}

