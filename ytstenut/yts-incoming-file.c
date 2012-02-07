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
#include "yts-incoming-file-internal.h"

#include "config.h"

static void
_initable_interface_init (GInitableIface *interface);

static void
_file_transfer_interface_init (YtsFileTransferInterface *interface);

G_DEFINE_TYPE_WITH_CODE (YtsIncomingFile,
                         yts_incoming_file,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                _initable_interface_init)
                         G_IMPLEMENT_INTERFACE (YTS_TYPE_FILE_TRANSFER,
                                                _file_transfer_interface_init))

/**
 * SECTION: yts-incoming-file
 * @title: YtsIncomingFile
 * @short_description: File download implementation.
 *
 * #YtsIncomingFile represents an incoming file downlod operation from another
 * Ytstenut service.
 *
 * TODO add cancellation in dispose(), and cancel API. Take care not to touch
 * self any more after cancellation.
 */

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_INCOMING_FILE, YtsIncomingFilePrivate))

enum {
  PROP_0,

  /* YtsFileTransfer */
  PROP_FILE_TRANSFER_FILE,
  PROP_FILE_TRANSFER_PROGRESS,

  /* YtsIncomingFile */
  PROP_TP_CHANNEL
};

typedef struct {

  /* Properties */
  GFile     *file;
  float      progress;

  /* Data */
  TpFileTransferChannel *tp_channel;
  uint64_t               size;
} YtsIncomingFilePrivate;

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
 * YtsIncomingFile
 */

static void
set_and_emit_error (YtsIncomingFile  *self,
                    GError           *error)
{
  YtsIncomingFilePrivate *priv = GET_PRIVATE (self);

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
_channel_notify_state (TpFileTransferChannel  *channel,
                       GParamSpec             *pspec,
                       YtsIncomingFile        *self)
{
  YtsIncomingFilePrivate *priv = GET_PRIVATE (self);
  TpFileTransferState             state;
  TpFileTransferStateChangeReason reason;
  bool                            close_channel = false;

  state = tp_file_transfer_channel_get_state (channel, &reason);

  if (state == TP_FILE_TRANSFER_STATE_COMPLETED) {

    priv->progress = 1.1;
    g_object_notify (G_OBJECT (self), "progress");
    close_channel = true;

  } else if (reason == TP_FILE_TRANSFER_STATE_CHANGE_REASON_REMOTE_STOPPED) {

    g_signal_emit_by_name (self, "cancelled");
    priv->progress = -0.1;
    g_object_notify (G_OBJECT (self), "progress");
    close_channel = true;

  } else if (reason == TP_FILE_TRANSFER_STATE_CHANGE_REASON_LOCAL_ERROR) {

    GError *error = g_error_new (YTS_INCOMING_FILE_ERROR,
                                 YTS_INCOMING_FILE_ERROR_LOCAL,
                                 "Transmission failed because of a local error");
    set_and_emit_error (self, error);
    g_error_free (error);
    close_channel = true;

  } else if (reason == TP_FILE_TRANSFER_STATE_CHANGE_REASON_REMOTE_ERROR) {

    GError *error = g_error_new (YTS_INCOMING_FILE_ERROR,
                                 YTS_INCOMING_FILE_ERROR_REMOTE,
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
                                   YtsIncomingFile        *self)
{
  YtsIncomingFilePrivate  *priv = GET_PRIVATE (self);
  float transferred_bytes;

  transferred_bytes = tp_file_transfer_channel_get_transferred_bytes (channel);
  priv->progress = transferred_bytes / priv->size;
  g_object_notify (G_OBJECT (self), "progress");
}

static gboolean
_initable_init (GInitable      *initable,
			          GCancellable   *cancellable,
			          GError        **error_out)
{
  YtsIncomingFilePrivate *priv = GET_PRIVATE (initable);

  if (!TP_IS_FILE_TRANSFER_CHANNEL (priv->tp_channel)) {
    if (error_out) {
      *error_out = g_error_new (YTS_INCOMING_FILE_ERROR,
                                YTS_INCOMING_FILE_ERROR_NO_CHANNEL,
                                "No channel to receive file");
    }
    return false;
  }

  return true;
}

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsIncomingFilePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    /* YtsFileTransfer */

    case PROP_FILE_TRANSFER_FILE:
      g_value_set_object (value, priv->file);
      break;
    case PROP_FILE_TRANSFER_PROGRESS:
      g_value_set_float (value, priv->progress);
      break;

    /* YtsIncomingFile */

    case PROP_TP_CHANNEL:
      g_value_set_object (value, priv->tp_channel);
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
  YtsIncomingFilePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {

    /* YtsFileTransfer */

    case PROP_FILE_TRANSFER_FILE:
      /* Construct-only, but optional, because it's set behind the scenes
       * in _accept(). */
      if (g_value_get_object (value))
        priv->file = g_value_dup_object (value);
      break;

    /* YtsIncomingFile */

    case PROP_TP_CHANNEL: {
      /* Construct-only */
      priv->tp_channel = g_value_dup_object (value);
      priv->size = tp_file_transfer_channel_get_size (priv->tp_channel);
      } break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsIncomingFilePrivate *priv = GET_PRIVATE (object);

  if (priv->tp_channel) {
    tp_channel_close_async (TP_CHANNEL (priv->tp_channel),
                            _channel_close,
                            object);
    g_object_unref (priv->tp_channel);
    priv->tp_channel = NULL;
  }

  G_OBJECT_CLASS (yts_incoming_file_parent_class)->finalize (object);
}

static void
_finalize (GObject *object)
{
  YtsIncomingFilePrivate *priv = GET_PRIVATE (object);

  if (priv->file) {
    g_object_unref (priv->file);
    priv->file = NULL;
  }

  G_OBJECT_CLASS (yts_incoming_file_parent_class)->finalize (object);
}

static void
yts_incoming_file_class_init (YtsIncomingFileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (YtsIncomingFilePrivate));

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

  /* YtsIncomingFile properties */

  pspec = g_param_spec_object ("tp-channel", "", "",
                               TP_TYPE_FILE_TRANSFER_CHANNEL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_TP_CHANNEL, pspec);
}

static void
yts_incoming_file_init (YtsIncomingFile *self)
{
}

YtsIncomingFile *
yts_incoming_file_new (TpFileTransferChannel *tp_channel)
{
  return g_object_new (YTS_TYPE_INCOMING_FILE,
                       "tp-channel", tp_channel,
                       NULL);
}

GFile *const
yts_incoming_file_get_file (YtsIncomingFile *self)
{
  YtsIncomingFilePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_INCOMING_FILE (self), NULL);

  return priv->file;
}

static void
_channel_accept_file (GObject       *source,
                      GAsyncResult  *result,
                      gpointer       data)
{
  YtsIncomingFile         *self = YTS_INCOMING_FILE (data);
  YtsIncomingFilePrivate  *priv = GET_PRIVATE (self);
  GError *error_in = NULL;

  tp_file_transfer_channel_accept_file_finish (priv->tp_channel,
                                               result,
                                               &error_in);
  if (error_in) {
    GError *error_out = g_error_new (YTS_INCOMING_FILE_ERROR,
                                     YTS_INCOMING_FILE_ERROR_ACCEPT_FAILED,
                                     "Failed to start file transfer "
                                     "(%s)",
                                     error_in->message);
    set_and_emit_error (self, error_out);
    g_error_free (error_out);
    g_clear_error (&error_in);
  }
}

bool
yts_incoming_file_accept (YtsIncomingFile  *self,
                          GFile            *file,
                          GError          **error_out)
{
  YtsIncomingFilePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_INCOMING_FILE (self), false);
  g_return_val_if_fail (G_IS_FILE (file), false);

  if (G_IS_FILE (priv->file)) {
    if (error_out) {
      *error_out = g_error_new (YTS_INCOMING_FILE_ERROR,
                                YTS_INCOMING_FILE_ERROR_ALREADY_ACCEPTED,
                                "Incoming file has already been accepted");
    }
    return false;
  }

  priv->file = g_object_ref (file);
  g_object_notify (G_OBJECT (self), "file");

  g_signal_connect (priv->tp_channel, "notify::state",
                    G_CALLBACK (_channel_notify_state), self);

  g_signal_connect (priv->tp_channel, "notify::transferred-bytes",
                    G_CALLBACK (_channel_notify_transferred_bytes), self);

  tp_file_transfer_channel_accept_file_async (priv->tp_channel,
                                              priv->file,
                                              0,
                                              _channel_accept_file,
                                              self);

  return true;
}

bool
yts_incoming_file_reject (YtsIncomingFile  *self,
                          GError          **error)
{
  YtsIncomingFilePrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_INCOMING_FILE (self), false);
  g_return_val_if_fail (TP_IS_FILE_TRANSFER_CHANNEL (priv->tp_channel), false);

  tp_channel_close_async (TP_CHANNEL (priv->tp_channel),
                          _channel_close,
                          self);
  g_object_unref (priv->tp_channel);
  priv->tp_channel = NULL;

  return true;
}

