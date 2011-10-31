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

#include "yts-contact-impl.h"
#include "yts-marshal.h"
#include "config.h"

G_DEFINE_TYPE (YtsContactImpl, yts_contact_impl, YTS_TYPE_CONTACT)

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0contact-impl\0"G_STRLOC

enum {
  SIG_SEND_MESSAGE,

  N_SIGNALS
};

static unsigned _signals[N_SIGNALS] = { 0, };

static void
yts_contact_impl_class_init (YtsContactImplClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  /**
   * YtsImplContact::send-message:
   *
   * <note>Internal signal, should not be considered by users at this time.
   * Maybe in the future when we allow for custom contact subclasses.</note>
   */
  _signals[SIG_SEND_MESSAGE] = g_signal_new ("send-message",
                                             G_TYPE_FROM_CLASS (object_class),
                                             G_SIGNAL_RUN_LAST,
                                             0, NULL, NULL,
                                             yts_marshal_VOID__OBJECT_OBJECT,
                                             G_TYPE_NONE, 2,
                                             YTS_TYPE_SERVICE,
                                             YTS_TYPE_METADATA);
}

static void
yts_contact_impl_init (YtsContactImpl *self)
{
}

YtsContact *
yts_contact_impl_new (TpContact *tp_contact)
{
  return g_object_new (YTS_TYPE_CONTACT_IMPL,
                       "tp-contact", tp_contact,
                       NULL);
}

void
yts_contact_impl_send_message (YtsContactImpl *self,
                               YtsService     *service,
                               YtsMetadata    *message)
{
  g_return_if_fail (YTS_IS_CONTACT_IMPL (self));
  g_return_if_fail (YTS_IS_SERVICE (service));
  g_return_if_fail (YTS_IS_METADATA (message));

  g_signal_emit (self, _signals[SIG_SEND_MESSAGE], 0,
                 service, message);
}

