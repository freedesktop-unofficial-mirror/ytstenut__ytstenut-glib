/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2011, Intel Corporation.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <ytstenut-glib/ytsg-metadata-service.h>
#include <ytstenut-glib/ytsg-private.h>
#include <ytstenut-glib/ytsg-status.h>
#include <string.h>

#define MYUID       "com.meego.ytstenut.TestService"
#define STATUS_XML  "<status a1='v1' a2='v2'></status>"
#define MESSAGE_XML "<message a1='v1' a2='v2'></message>"

static gboolean got_status_signal  = FALSE;
static gboolean got_message_signal = FALSE;

static void
status_cb (YtsgMetadataService *service, YtsgStatus *status, gpointer data)
{
  YtsgStatus *s2 = data;

  got_status_signal = TRUE;

  g_assert (ytsg_metadata_is_equal ((YtsgMetadata*)status, (YtsgMetadata*)s2));
}

static void
message_cb (YtsgMetadataService *service, YtsgMessage *message, gpointer data)
{
  YtsgMessage *m2 = data;

  got_message_signal = TRUE;

  g_assert (ytsg_metadata_is_equal ((YtsgMetadata*)message, (YtsgMetadata*)m2));
}

int
main (int argc, char **argv)
{
  YtsgService *service;
  YtsgStatus  *status;
  YtsgMessage *message;
  const char  *uid;

  g_thread_init (NULL);
  g_type_init ();

  /*
   * The metadata-serivice-test property allows for partial construction of
   * the object, so we can run some rudimentary tests.
   */
  service = g_object_new (YTSG_TYPE_METADATA_SERVICE,
                          "metadata-service-test", TRUE,
                          "uid", MYUID,
                          "type", "application",
                          NULL);

  g_assert (service);

  uid = ytsg_service_get_uid (service);

  g_assert_cmpstr (MYUID, ==, uid);

  status = (YtsgStatus*)_ytsg_metadata_new_from_xml (STATUS_XML);
  g_assert (YTSG_IS_STATUS (status));

  g_signal_connect (service, "status",
                    G_CALLBACK (status_cb), status);

  _ytsg_metadata_service_received_status ((YtsgMetadataService*)service,
                                          STATUS_XML);

  g_assert (got_status_signal);

  message = (YtsgMessage*)_ytsg_metadata_new_from_xml (MESSAGE_XML);
  g_assert (YTSG_IS_MESSAGE (message));

  g_signal_connect (service, "message",
                    G_CALLBACK (message_cb), message);

  _ytsg_metadata_service_received_message ((YtsgMetadataService*)service,
                                           MESSAGE_XML);

  g_assert (got_message_signal);

  return 0;
}
