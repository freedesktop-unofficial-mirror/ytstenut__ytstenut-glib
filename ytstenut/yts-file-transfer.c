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
 * Authored by: Rob Staudinger <robsta@linux.intel.com>
 */
#include "config.h"

#include <stdbool.h>

#include "yts-file-transfer.h"
#include "yts-marshal.h"

/* HACK, include known implementers headers for type checks. */
#include "yts-incoming-file-internal.h"
#include "yts-outgoing-file-internal.h"

G_DEFINE_INTERFACE (YtsFileTransfer, yts_file_transfer, G_TYPE_OBJECT)

/**
 * SECTION: yts-file-transfer
 * @short_description: Common interface for file transfers between Ytstenut
 * services.
 *
 * #YtsFileTransfer represents an ongoing file transfer operation between
 * Ytstenut services, and offers progress- and status-reporting facilities.
 */

enum {
  SIG_CANCELLED = 0,
  SIG_ERROR,

  LAST_SIGNAL
};

static unsigned _signals[LAST_SIGNAL] = { 0, };

static void
yts_file_transfer_default_init (YtsFileTransferInterface *interface)
{
  static bool _initialized = false;

  if (!_initialized) {

    GParamSpec *pspec;

    /**
     * YtsFileTransfer:file:
     *
     * The #GFile instance backing the transfer.
     *
     * Since: 0.4
     */
    pspec = g_param_spec_object ("file", "", "",
                                 G_TYPE_FILE,
                                 G_PARAM_READWRITE |
                                 G_PARAM_CONSTRUCT_ONLY |
                                 G_PARAM_STATIC_STRINGS);
    g_object_interface_install_property (interface, pspec);

    /**
     * YtsFileTransfer:progress:
     *
     * Read-only property that holds the file transmission progress. Values range
     * from 0.0 at the start of the transfer, to 1.0 upon completion.
     * Error or cancellation leaves the progress with a value smaller than zero.
     */
    pspec = g_param_spec_float ("progress", "", "",
                                -0.1, 1.1, 0.0,
                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
    g_object_interface_install_property (interface, pspec);

    /**
     * YtsFileTransfer::error:
     * @self: object which emitted the signal.
     * @error: the #GError causing the transmission to fail.
     *
     * This signal is emitted when the transmission failed, transporting
     * error details.
     *
     * Since: 0.4
     */
    _signals[SIG_ERROR] = g_signal_new ("error",
                                        G_TYPE_FROM_INTERFACE (interface),
                                        G_SIGNAL_RUN_LAST,
                                        0, NULL, NULL,
                                        yts_marshal_VOID__BOXED,
                                        G_TYPE_NONE, 1,
                                        G_TYPE_ERROR);

    /**
     * YtsFileTransfer::cancelled:
     * @self: object which emitted the signal.
     *
     * This signal is emitted when remote peer cancelled the file transmission.
     *
     * Since: 0.4
     */
    _signals[SIG_CANCELLED] = g_signal_new ("cancelled",
                                            G_TYPE_FROM_INTERFACE (interface),
                                            G_SIGNAL_RUN_LAST,
                                            0, NULL, NULL,
                                            yts_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
  }
}

/**
 * yts_file_transfer_get_file:
 * @self: object on which to invoke this method.
 *
 * See YtsFileTransfer:file property for details.
 *
 * Returns: #GFile instance backing the transfer.
 */
GFile *const
yts_file_transfer_get_file (YtsFileTransfer *self)
{
  /* Known subclasses, so hack it up for const return. */

  if (YTS_IS_INCOMING_FILE (self)) {

    return yts_incoming_file_get_file (YTS_INCOMING_FILE (self));

  } else if (YTS_IS_OUTGOING_FILE (self)) {

    return yts_outgoing_file_get_file (YTS_OUTGOING_FILE (self));

  } else {

    g_warning ("Unhandled YtsFileTransfer instance %s in %s",
               G_OBJECT_TYPE_NAME (self),
               __FUNCTION__);
  }

  return NULL;
}

/**
 * yts_file_transfer_get_progress:
 * @self: object on which to invoke this method.
 *
 * Get progress of file transfer operation, see #YtsFileTransfer:progress for
 * details about the range of values.
 *
 * Returns: file transfer progress.
 */
float
yts_file_transfer_get_progress (YtsFileTransfer *self)
{
  float progress;

  g_return_val_if_fail (YTS_IS_FILE_TRANSFER (self), -1.0);

  g_object_get (self, "progress", &progress, NULL);

  return progress;
}

