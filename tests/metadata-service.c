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

#include <ytstenut/yts-metadata-service.h>
#include <ytstenut/yts-private.h>
#include <ytstenut/yts-status.h>
#include <string.h>

#define MYUID       "com.meego.ytstenut.TestService"
#define STATUS_XML  "<status a1='v1' a2='v2'></status>"
#define MESSAGE_XML "<message a1='v1' a2='v2'></message>"

static gboolean got_status_signal  = FALSE;
static gboolean got_message_signal = FALSE;

static void
status_cb (YtsMetadataService *service, YtsStatus *status, gpointer data)
{
  YtsStatus *s2 = data;

  got_status_signal = TRUE;

  g_assert (yts_metadata_is_equal ((YtsMetadata*)status, (YtsMetadata*)s2));
}

static void
message_cb (YtsMetadataService *service, YtsMessage *message, gpointer data)
{
  YtsMessage *m2 = data;

  got_message_signal = TRUE;

  g_assert (yts_metadata_is_equal ((YtsMetadata*)message, (YtsMetadata*)m2));
}

int
main (int argc, char **argv)
{
  YtsService *service;
  YtsStatus  *status;
  YtsMessage *message;
  const char  *uid;

  g_thread_init (NULL);
  g_type_init ();

  /*
   * The metadata-serivice-test property allows for partial construction of
   * the object, so we can run some rudimentary tests.
   */
  service = g_object_new (YTS_TYPE_METADATA_SERVICE,
                          "metadata-service-test", TRUE,
                          "uid", MYUID,
                          "type", "application",
                          NULL);

  g_assert (service);

  uid = yts_service_get_uid (service);

  g_assert_cmpstr (MYUID, ==, uid);

  status = (YtsStatus*)_yts_metadata_new_from_xml (STATUS_XML);
  g_assert (YTS_IS_STATUS (status));

  g_signal_connect (service, "status",
                    G_CALLBACK (status_cb), status);

  _yts_metadata_service_received_status ((YtsMetadataService*)service,
                                          STATUS_XML);

  g_assert (got_status_signal);

  message = (YtsMessage*)_yts_metadata_new_from_xml (MESSAGE_XML);
  g_assert (YTS_IS_MESSAGE (message));

  g_signal_connect (service, "message",
                    G_CALLBACK (message_cb), message);

  _yts_metadata_service_received_message ((YtsMetadataService*)service,
                                           MESSAGE_XML);

  g_assert (got_message_signal);

  return 0;
}
