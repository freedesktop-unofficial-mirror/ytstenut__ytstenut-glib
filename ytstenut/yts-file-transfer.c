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

#include <stdbool.h>

#include "yts-file-transfer.h"
#include "yts-marshal.h"

#include "config.h"

G_DEFINE_INTERFACE (YtsFileTransfer, yts_file_transfer, G_TYPE_OBJECT)

/**
 * SECTION:yts-file-transfer
 * @short_description: Common interface for file transfers between Ytstenut
 * services.
 *
 * #YtsFileTransfer represents an ongoing file transfer operation between
 * Ytstenut services, and offers progress- and status-reporting facilities.
 */

enum {
  SIG_ERROR = 0,

  LAST_SIGNAL
};

static unsigned _signals[LAST_SIGNAL] = { 0, };

/**
 * SECTION:yts-file_transfer
 * @short_description: Common interface for file up- and downloads.
 *
 * #YtsFileTransfer is an common interface for the #YtsIncomingFile and
 * #YtsOutgoingFile file transmission classes. A value smaller than 0.0
 * denotes error state.
 */

static void
yts_file_transfer_default_init (YtsFileTransferInterface *interface)
{
  static bool _initialized = false;

  if (!_initialized) {

    GParamSpec *pspec;

    /**
     * YtsFileTransfer:progress:
     *
     * Read-only property that holds the file transmission progress. Values range
     * from 0.0 at the start of the transfer, to 1.0 upon completion.
     */
    pspec = g_param_spec_float ("progress", "", "",
                                -0.1, 1.1, 0.0,
                                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
    g_object_interface_install_property (interface, pspec);

    /*
     * Signals
     */

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
  }
}

/**
 * yts_file_transfer_get_progress:
 * @self: object on which to invoke this method.
 *
 * Get progress of file transfer operation, see #YtsFileTransfer.progress for
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

