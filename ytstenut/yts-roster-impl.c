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

#include "yts-roster-impl.h"
#include "yts-marshal.h"
#include "config.h"

G_DEFINE_TYPE (YtsRosterImpl, yts_roster_impl, YTS_TYPE_ROSTER)

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0roster-impl\0"G_STRLOC

enum {
  SIG_SEND_MESSAGE,
  SIG_SEND_FILE,

  N_SIGNALS
};

static unsigned _signals[N_SIGNALS] = { 0, };

static void
yts_roster_impl_class_init (YtsRosterImplClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  /**
   * YtsContact::send-message:
   *
   * <note>Internal signal, should not be considered by users at this time.
   * Maybe in the future when we allow for custom contact subclasses.</note>
   */
  _signals[SIG_SEND_MESSAGE] = g_signal_new ("send-message",
                                         G_TYPE_FROM_CLASS (object_class),
                                         G_SIGNAL_RUN_LAST,
                                         0, NULL, NULL,
                                         yts_marshal_VOID__OBJECT_OBJECT_OBJECT,
                                         G_TYPE_NONE, 3,
                                         YTS_TYPE_CONTACT,
                                         YTS_TYPE_SERVICE,
                                         YTS_TYPE_METADATA);

  _signals[SIG_SEND_FILE] = g_signal_new ("send-file",
                                          G_TYPE_FROM_CLASS (object_class),
                                          G_SIGNAL_RUN_LAST,
                                          0, NULL, NULL,
                                          yts_marshal_OBJECT__OBJECT_OBJECT_OBJECT_STRING_POINTER,
                                          YTS_TYPE_OUTGOING_FILE, 5,
                                          YTS_TYPE_CONTACT,
                                          YTS_TYPE_SERVICE,
                                          G_TYPE_FILE,
                                          G_TYPE_STRING,
                                          G_TYPE_POINTER);
}

static void
yts_roster_impl_init (YtsRosterImpl *self)
{
}

YtsRoster *
yts_roster_impl_new (void)
{
  return g_object_new (YTS_TYPE_ROSTER_IMPL, NULL);
}

void
yts_roster_impl_send_message (YtsRosterImpl *self,
                              YtsContact    *contact,
                              YtsService    *service,
                              YtsMetadata   *message)
{
  g_return_if_fail (YTS_IS_ROSTER_IMPL (self));
  g_return_if_fail (YTS_IS_CONTACT (contact));
  g_return_if_fail (YTS_IS_SERVICE (service));
  g_return_if_fail (YTS_IS_METADATA (message));

  g_signal_emit (self, _signals[SIG_SEND_MESSAGE], 0,
                 contact, service, message);
}

YtsOutgoingFile *
yts_roster_impl_send_file (YtsRosterImpl   *self,
                           YtsContact      *contact,
                           YtsService      *service,
                           GFile           *file,
                           char const      *description,
                           GError         **error_out)
{
  YtsOutgoingFile *transfer;

  transfer = NULL;
  g_signal_emit (self, _signals[SIG_SEND_FILE], 0,
                 contact, service, file, description, error_out,
                 &transfer);

  return transfer;
}

