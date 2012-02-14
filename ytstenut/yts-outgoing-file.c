/*
 * Copyright © 2012 Intel Corp.
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
 * Authored by: Rob Staudinger <robsta@linux.intel.com>
 * Using portions from telepathy-ytstenut's server-file-transfer.c, © Intel.
 */

#include <stdbool.h>
#include <stdint.h>
#include <telepathy-glib/telepathy-glib.h>

#include "yts-file-transfer.h"
#include "yts-outgoing-file-internal.h"

#include "config.h"

static void
_initable_interface_init (GInitableIface *interface);

static void
_file_transfer_interface_init (YtsFileTransferInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsOutgoingFile,
                         yts_outgoing_file,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                _initable_interface_init)
                         G_IMPLEMENT_INTERFACE (YTS_TYPE_FILE_TRANSFER,
                                                _file_transfer_interface_init))

/**
 * SECTION: yts-outgoing-file
 * @short_description: File upload implementation.
 *
 * #YtsOutgoingFile represents an ongoing file upload operation to another
 * Ytstenut service.
 *
 * TODO add cancellation in dispose(), and cancel API. Take care not to touch
 * self any more after cancellation.
 */

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_OUTGOING_FILE, YtsOutgoingFilePrivate))

enum {
  PROP_0,

  /* YtsFileTransfer */
  PROP_FILE_TRANSFER_PROGRESS,
  PROP_FILE_TRANSFER_FILE,

  /* YtsOutgoingFile */
  PROP_TP_ACCOUNT,
  PROP_DESCRIPTION,
  PROP_RECIPIENT_CONTACT_ID,
  PROP_RECIPIENT_SERVICE_ID,
  PROP_SENDER_SERVICE_ID
};

typedef struct {
  /* Properties */
  TpAccount *tp_account;
  GFile     *file;
  char      *recipient_contact_id;
  char      *recipient_service_id;
  char      *sender_service_id;
  char      *description;
  float      progress;
  /* Data */
  TpFileTransferChannel *tp_channel;
  uint64_t               size;
} YtsOutgoingFilePrivate;

static gboolean
_initable_init (GInitable      *initable,
			          GCancellable   *cancellable,
			          GError        **error);

/*
 * GInitable interface
 */

static void
_initable_interface_init (GInitableIface *interface)
{
  static bool _is_initialized = false;

  if (!_is_initialized) {
    interface->init = _initable_init;
    _is_initialized = true;
  }
}

/*
 * YtsFileTransfer interface
 */

static void
_file_transfer_interface_init (YtsFileTransferInterface *interface)
{
  /* Nothing to do here, since this interface is only about properties and
   * signals. */
}

/*
 * YtsOutgoingFile
 */

static bool
validate (YtsOutgoingFile  *self,
          GError          **error_out)
{
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (self);

  if (!TP_IS_ACCOUNT (priv->tp_account)) {
    if (error_out) {
      *error_out = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                YTS_OUTGOING_FILE_ERROR_NO_ACCOUNT,
                                "No account to send file from");
    }
    return false;
  }

  if (TP_CONNECTION_STATUS_CONNECTED !=
      tp_account_get_connection_status (priv->tp_account, NULL)) {
    if (error_out) {
      char const *reason = tp_account_get_detailed_error (priv->tp_account,
                                                          NULL);
      *error_out = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                YTS_OUTGOING_FILE_ERROR_NO_CONNECTION,
                                "Account not online (%s)", reason);
    }
    return false;
  }

  if (!G_IS_FILE (priv->file)) {
    if (error_out) {
      *error_out = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                YTS_OUTGOING_FILE_ERROR_NO_FILE,
                                "No file specified for file transfer");
    }
    return false;
  }

  if (NULL == priv->recipient_contact_id) {
    if (error_out) {
      *error_out = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                YTS_OUTGOING_FILE_ERROR_NO_RECIPIENT_CONTACT,
                                "No recipient contact specified for file transfer");
    }
    return false;
  }

  if (NULL == priv->recipient_service_id) {
    if (error_out) {
      *error_out = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                YTS_OUTGOING_FILE_ERROR_NO_RECIPIENT_SERVICE,
                                "No recipient service specified for file transfer");
    }
    return false;
  }

  if (NULL == priv->sender_service_id) {
    if (error_out) {
      *error_out = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                YTS_OUTGOING_FILE_ERROR_NO_SENDER_SERVICE,
                                "No sender service specified for file transfer");
    }
    return false;
  }

  return true;
}

static void
set_and_emit_error (YtsOutgoingFile  *self,
                    GError           *error)
{
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (self);

  g_signal_emit_by_name (self, "error", error);
  priv->progress = -0.1;
  g_object_notify (G_OBJECT (self), "progress");
}

static void
_channel_close (GObject       *source,
                GAsyncResult  *result,
                void          *data)
{
  /* Object might be in dispose() already, do not touch self/priv any more. */
  GError *error_in = NULL;

  tp_channel_close_finish (TP_CHANNEL (source), result, &error_in);
  if (error_in) {
    g_critical ("Failed to close the file-transfer channel");
    g_clear_error (&error_in);
  }
}

static void
_channel_provide_file (GObject          *object,
                       GAsyncResult     *result,
                       gpointer          data)
{
  YtsOutgoingFile         *self = YTS_OUTGOING_FILE (data);
  YtsOutgoingFilePrivate  *priv = GET_PRIVATE (self);
  GError                  *error_in = NULL;

  tp_file_transfer_channel_provide_file_finish (priv->tp_channel,
                                                result,
                                                &error_in);
  if (error_in) {
    GError *error_out = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                     YTS_OUTGOING_FILE_ERROR_TRANSFER_FAILED,
                                     "Failed to transfer file "
                                     "(%s)",
                                     error_in->message);
    set_and_emit_error (self, error_out);
    g_error_free (error_out);
    g_clear_error (&error_in);
  }
}

static void
_channel_notify_state (TpFileTransferChannel  *channel,
                       GParamSpec             *pspec,
                       YtsOutgoingFile        *self)
{
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (self);
  TpFileTransferState             state;
  TpFileTransferStateChangeReason reason;
  bool                            close_channel = false;

  state = tp_file_transfer_channel_get_state (priv->tp_channel, &reason);

  if (state == TP_FILE_TRANSFER_STATE_ACCEPTED
      && tp_channel_get_requested (TP_CHANNEL (channel))) {

    tp_file_transfer_channel_provide_file_async (priv->tp_channel,
                                                 priv->file,
                                                 _channel_provide_file,
                                                 self);

  } else if (state == TP_FILE_TRANSFER_STATE_COMPLETED) {

    priv->progress = 1.1;
    g_object_notify (G_OBJECT (self), "progress");
    close_channel = true;

  } else if (reason == TP_FILE_TRANSFER_STATE_CHANGE_REASON_REMOTE_STOPPED) {

    g_signal_emit_by_name (self, "cancelled");
    priv->progress = -0.1;
    g_object_notify (G_OBJECT (self), "progress");
    close_channel = true;

  } else if (reason == TP_FILE_TRANSFER_STATE_CHANGE_REASON_LOCAL_ERROR) {

    GError *error = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                 YTS_OUTGOING_FILE_ERROR_LOCAL,
                                 "Transmission failed because of a local error");
    set_and_emit_error (self, error);
    g_error_free (error);
    close_channel = true;

  } else if (reason == TP_FILE_TRANSFER_STATE_CHANGE_REASON_REMOTE_ERROR) {

    GError *error = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                 YTS_OUTGOING_FILE_ERROR_REMOTE,
                                 "Transmission failed because of a remote error");
    set_and_emit_error (self, error);
    g_error_free (error);
    close_channel = true;
  }

  if (close_channel && priv->tp_channel) {
    tp_channel_close_async (TP_CHANNEL (priv->tp_channel),
                            _channel_close,
                            self);
    g_object_unref (priv->tp_channel);
    priv->tp_channel = NULL;
  }
}

static void
_channel_notify_transferred_bytes (TpFileTransferChannel  *channel,
                                   GParamSpec             *pspec,
                                   YtsOutgoingFile        *self)
{
  YtsOutgoingFilePrivate  *priv = GET_PRIVATE (self);
  float transferred_bytes;

  transferred_bytes = tp_file_transfer_channel_get_transferred_bytes (channel);
  priv->progress = transferred_bytes / priv->size;
  g_object_notify (G_OBJECT (self), "progress");
}

static void
_account_channel_request_create (GObject      *source,
                                 GAsyncResult *result,
                                 gpointer      data)
{
  YtsOutgoingFile         *self = YTS_OUTGOING_FILE (data);
  YtsOutgoingFilePrivate  *priv = GET_PRIVATE (self);
  TpAccountChannelRequest *channel_request = TP_ACCOUNT_CHANNEL_REQUEST (source);
  GError                  *error_in = NULL;

  priv->tp_channel = TP_FILE_TRANSFER_CHANNEL (
      tp_account_channel_request_create_and_handle_channel_finish (
                                                              channel_request,
                                                              result,
                                                              NULL,
                                                              &error_in));
  if (error_in) {
    GError *error_out = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                     YTS_OUTGOING_FILE_ERROR_CHANNEL_FAILED,
                                     "Failed to create the file transfer channel "
                                     "(%s)",
                                     error_in->message);
    set_and_emit_error (self, error_out);
    g_error_free (error_out);
    g_clear_error (&error_in);
    return;
  }

  g_signal_connect (priv->tp_channel, "notify::state",
                    G_CALLBACK (_channel_notify_state), self);
  g_signal_connect (priv->tp_channel, "notify::transferred-bytes",
                    G_CALLBACK (_channel_notify_transferred_bytes), self);
}

static gboolean
_initable_init (GInitable      *initable,
			          GCancellable   *cancellable,
			          GError        **error_out)
{
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (initable);
  GFileInfo               *info;
  char const              *name;
  char const              *mimetype;
  GTimeVal                 mtime;
  GHashTable              *metadata;
  char                   **values;
  GHashTable              *request;
  TpAccountChannelRequest *channel_request;
  GError                  *error = NULL;

  if (!validate (YTS_OUTGOING_FILE (initable), error_out)) {
    return false;
  }

  info = g_file_query_info (priv->file,
                            "*",
                            G_FILE_QUERY_INFO_NONE,
                            NULL,
                            &error);
  if (NULL == info) {
    if (error_out) {
      char *uri = g_file_get_uri (priv->file);
      *error_out = g_error_new (YTS_OUTGOING_FILE_ERROR,
                                YTS_OUTGOING_FILE_ERROR_READ_FAILED,
                                "Failed to read file %s",
                                uri);
      g_free (uri);
    }
    return false;
  }

  name = g_file_info_get_name (info);
  mimetype = g_file_info_get_content_type (info);
  g_file_info_get_modification_time (info, &mtime);
  priv->size = g_file_info_get_size (info);

  metadata = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  values = g_new0 (char *, 2);
  values[0] = priv->sender_service_id;
  g_hash_table_insert (metadata,
                       g_strdup ("FromService"),
                       values);

  /* Now we have everything prepared to continue, let's create the
   * Ytstenut channel handler with service name specified. */
  request = tp_asv_new (
      TP_PROP_CHANNEL_CHANNEL_TYPE,
      G_TYPE_STRING,
      TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,

      TP_PROP_CHANNEL_TARGET_HANDLE_TYPE,
      G_TYPE_UINT,
      TP_HANDLE_TYPE_CONTACT,

      TP_PROP_CHANNEL_TARGET_ID,
      G_TYPE_STRING,
      priv->recipient_contact_id,

      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_CONTENT_TYPE,
      G_TYPE_STRING,
      mimetype,

      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_DATE,
      G_TYPE_INT64,
      (gint64) mtime.tv_sec,

      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_DESCRIPTION,
      G_TYPE_STRING,
      priv->description,

      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_FILENAME,
      G_TYPE_STRING,
      name,

      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_INITIAL_OFFSET,
      G_TYPE_UINT64,
      (guint64) 0,

      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_SIZE,
      G_TYPE_UINT64,
      (guint64) priv->size,

      /* Here is the remote service */
      TP_PROP_CHANNEL_INTERFACE_FILE_TRANSFER_METADATA_SERVICE_NAME,
      G_TYPE_STRING,
      priv->recipient_service_id,

      /* And include our own service name. FIXME this should be a define. */
      TP_PROP_CHANNEL_INTERFACE_FILE_TRANSFER_METADATA_METADATA,
      TP_HASH_TYPE_METADATA,
      metadata,

      NULL);

  channel_request = tp_account_channel_request_new (
                                            priv->tp_account,
                                            request,
                                            TP_USER_ACTION_TIME_CURRENT_TIME);

  tp_account_channel_request_create_and_handle_channel_async (
                                              channel_request,
                                              NULL,
                                              _account_channel_request_create,
                                              initable);

  g_hash_table_unref (request);
  g_object_unref (info);

  return true;
}

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    /* YtsFileTransfer */

    case PROP_FILE_TRANSFER_FILE:
      g_value_set_object (value, priv->file);
      break;
    case PROP_FILE_TRANSFER_PROGRESS:
      g_value_set_float (value, priv->progress);
      break;

    /* YtsOutgoingFile */

    case PROP_DESCRIPTION:
      g_value_set_string (value, priv->description);
      break;
    case PROP_RECIPIENT_CONTACT_ID:
      g_value_set_string (value, priv->recipient_contact_id);
      break;
    case PROP_RECIPIENT_SERVICE_ID:
      g_value_set_string (value, priv->recipient_service_id);
      break;
    case PROP_SENDER_SERVICE_ID:
      g_value_set_string (value, priv->sender_service_id);
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
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    /* YtsFileTransfer */

    case PROP_FILE_TRANSFER_FILE:
      /* Construct-only */
      priv->file = g_value_dup_object (value);
      break;

    /* YtsOutgoingFile */

    case PROP_TP_ACCOUNT:
      /* Construct-only */
      priv->tp_account = g_value_dup_object (value);
      break;
    case PROP_DESCRIPTION:
      /* Construct-only */
      priv->description = g_value_dup_string (value);
      break;
    case PROP_RECIPIENT_CONTACT_ID:
      /* Construct-only */
      priv->recipient_contact_id = g_value_dup_string (value);
      break;
    case PROP_RECIPIENT_SERVICE_ID:
      /* Construct-only */
      priv->recipient_service_id = g_value_dup_string (value);
    break;
    case PROP_SENDER_SERVICE_ID:
      /* Construct-only */
      priv->sender_service_id = g_value_dup_string (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (object);

  if (priv->tp_channel) {
    tp_channel_close_async (TP_CHANNEL (priv->tp_channel),
                            _channel_close,
                            object);
    priv->tp_channel = NULL;
  }

  G_OBJECT_CLASS (yts_outgoing_file_parent_class)->finalize (object);
}

static void
_finalize (GObject *object)
{
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (object);

  if (priv->tp_account) {
    g_object_unref (priv->tp_account);
    priv->tp_account = NULL;
  }

  if (priv->file) {
    g_object_unref (priv->file);
    priv->file = NULL;
  }

  if (priv->recipient_contact_id) {
    g_free (priv->recipient_contact_id);
    priv->recipient_contact_id = NULL;
  }

  if (priv->recipient_service_id) {
    g_free (priv->recipient_service_id);
    priv->recipient_service_id = NULL;
  }

  if (priv->sender_service_id) {
    g_free (priv->sender_service_id);
    priv->sender_service_id = NULL;
  }

  if (priv->description) {
    g_free (priv->description);
    priv->description = NULL;
  }

  G_OBJECT_CLASS (yts_outgoing_file_parent_class)->finalize (object);
}

static void
yts_outgoing_file_class_init (YtsOutgoingFileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (YtsOutgoingFilePrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;
  object_class->finalize = _finalize;

  /* YtsFileTransfer properties */

  g_object_class_override_property (object_class,
                                    PROP_FILE_TRANSFER_FILE,
                                    "file");
  g_object_class_override_property (object_class,
                                    PROP_FILE_TRANSFER_PROGRESS,
                                    "progress");

  /* YtsOutgoingFile properties */

  /**
   * YtsOutgoingFile:description:
   *
   * Describes the purpose of the file transfer. May be %NULL.
   *
   * Since: 0.4
   */
  pspec = g_param_spec_string ("description", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_DESCRIPTION, pspec);

  /**
   * YtsOutgoingFile:tp-account:
   *
   * Internal use only.
   */
  pspec = g_param_spec_object ("tp-account", "", "",
                               TP_TYPE_ACCOUNT,
                               G_PARAM_WRITABLE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_TP_ACCOUNT, pspec);

  /**
   * YtsOutgoingFile:recipient-contact-id:
   *
   * Contact ID of the file recipient.
   *
   * Since: 0.4
   */
  pspec = g_param_spec_string ("recipient-contact-id", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
                                   PROP_RECIPIENT_CONTACT_ID,
                                   pspec);

  /**
   * YtsOutgoingFile:recipient-service-id:
   *
   * Service ID of the file recipient.
   *
   * Since: 0.4
   */
  pspec = g_param_spec_string ("recipient-service-id", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
                                   PROP_RECIPIENT_SERVICE_ID,
                                   pspec);

  /**
   * YtsOutgoingFile:sender-service-id:
   *
   * Service ID of the file sender.
   *
   * Since: 0.4
   */
  pspec = g_param_spec_string ("sender-service-id", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
                                   PROP_SENDER_SERVICE_ID,
                                   pspec);
}

static void
yts_outgoing_file_init (YtsOutgoingFile *self)
{
}

YtsOutgoingFile *
yts_outgoing_file_new (TpAccount  *tp_account,
                       GFile      *file,
                       char const *sender_service_id,
                       char const *recipient_contact_id,
                       char const *recipient_service_id,
                       char const *description)
{
  return g_object_new (YTS_TYPE_OUTGOING_FILE,
                       "tp-account", tp_account,
                       "file", file,
                       "sender-service-id", sender_service_id,
                       "recipient-contact-id", recipient_contact_id,
                       "recipient-service-id", recipient_service_id,
                       "description", description,
                       NULL);
}

char const *
yts_outgoing_file_get_description (YtsOutgoingFile *self)
{
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_OUTGOING_FILE (self), NULL);

  return priv->description;
}

GFile *const
yts_outgoing_file_get_file (YtsOutgoingFile *self)
{
  YtsOutgoingFilePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_OUTGOING_FILE (self), NULL);

  return priv->file;
}

