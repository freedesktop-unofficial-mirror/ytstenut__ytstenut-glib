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

#include "yts-marshal.h"
#include "yts-service.h"
#include "yts-service-emitter.h"

G_DEFINE_INTERFACE (YtsServiceEmitter, yts_service_emitter, YTS_TYPE_SERVICE)

enum {
  SIG_SEND_MESSAGE,
  SIG_SEND_FILE,

  N_SIGNALS
};

static unsigned _signals[N_SIGNALS] = { 0, };

static void
yts_service_emitter_default_init (YtsServiceEmitterInterface *interface)
{
  static bool _initialized = false;

  if (!_initialized) {

    _signals[SIG_SEND_MESSAGE] = g_signal_new ("send-message",
                                               G_TYPE_FROM_INTERFACE (interface),
                                               G_SIGNAL_RUN_LAST,
                                               0, NULL, NULL,
                                               yts_marshal_VOID__OBJECT,
                                               G_TYPE_NONE, 1,
                                               YTS_TYPE_METADATA);

    _signals[SIG_SEND_FILE] = g_signal_new ("send-file",
                                            G_TYPE_FROM_INTERFACE (interface),
                                            G_SIGNAL_RUN_LAST,
                                            0, NULL, NULL,
                                            yts_marshal_OBJECT__OBJECT_STRING_POINTER,
                                            YTS_TYPE_OUTGOING_FILE, 3,
                                            G_TYPE_FILE,
                                            G_TYPE_STRING,
                                            G_TYPE_POINTER);
    _initialized = true;
  }
}

void
yts_service_emitter_send_message (YtsServiceEmitter *self,
                                  YtsMetadata       *message)
{
  g_signal_emit (self, _signals[SIG_SEND_MESSAGE], 0, message);
}

YtsOutgoingFile *
yts_service_emitter_send_file (YtsServiceEmitter   *self,
                               GFile               *file,
                               char const          *description,
                               GError             **error_out)
{
  YtsOutgoingFile *transfer;

  transfer = NULL;
  g_signal_emit (self, _signals[SIG_SEND_FILE], 0,
                 file, description, error_out,
                 &transfer);

  return transfer;
}

