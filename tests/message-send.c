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

#include <ytstenut/yts-private.h>
#include <ytstenut/yts-client.h>
#include <ytstenut/yts-main.h>

#include <string.h>

#define TEST_LENGTH 10

static gboolean    ready1 = FALSE;
static gboolean    ready2 = FALSE;
static int         retval = -2;
static YtsClient *client1 = NULL;
static YtsClient *client2 = NULL;
static GMainLoop  *loop = NULL;

static gboolean
timeout_test_cb (gpointer data)
{
  g_message ("TIMEOUT: quiting local connection test");

  retval = 1;

  g_main_loop_quit (loop);

  return FALSE;
}

static void
authenticated_cb (YtsClient *client, gpointer data)
{
  retval += 1;
}

static void
message_cb (YtsClient *client, YtsMessage *msg, gpointer data)
{
  YtsMetadata *md = (YtsMetadata*)msg;

  g_assert (YTS_IS_MESSAGE (msg));

  g_assert_cmpstr ("v1", ==, yts_metadata_get_attribute (md, "a1"));
  g_assert_cmpstr ("v2", ==, yts_metadata_get_attribute (md, "a2"));

  retval = 0;
  g_main_loop_quit (loop);
}

static void
service_added_cb (YtsRoster *roster, YtsService *service, gpointer data)
{
  YtsClient  *client  = yts_roster_get_client (roster);
  YtsContact *contact = yts_service_get_contact (service);
  const char  *jid     = yts_contact_get_jid (contact);
  const char  *sid     = yts_service_get_uid (service);
  gboolean     our     = FALSE;

  static YtsService *service2 = NULL;

  g_debug ("Service %s:%s", jid, sid);

  if (client == client1 && strstr (sid, "com.meego.ytstenut.SendMessageTest2"))
    {
      ready1   = TRUE;
      our      = TRUE;
      service2 = service;
    }

  if (client == client2 && strstr (sid, "com.meego.ytstenut.SendMessageTest1"))
    {
      ready2   = TRUE;
      our      = TRUE;

      g_signal_connect (client, "message", G_CALLBACK (message_cb), NULL);
    }

  /*
   * Waiting for both clients to appear ...
   */
  if (our && ready1 && ready2)
    {
      YtsError     e;
      YtsMetadata *metadata;
      const char   *attributes[] =
        {
          "a1", "v1",
          "a2", "v2",
          NULL
        };

      metadata = (YtsMetadata*)yts_message_new ((const char**)&attributes);

      g_debug ("Both test services are ready, sending message");

      e = yts_metadata_service_send_metadata ((YtsMetadataService *)service2,
                                               metadata);

      g_assert (yts_error_get_code (e) == YTS_ERROR_PENDING);
    }
}

int
main (int argc, char **argv)
{
  YtsRoster *roster1;
  YtsRoster *roster2;

  yts_init (0, NULL);

  loop = g_main_loop_new (NULL, FALSE);

  client1 = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP,
                             "com.meego.ytstenut.SendMessageTest1");
  g_signal_connect (client1, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster1 = yts_client_get_roster (client1);
  g_signal_connect (roster1, "service-added",
                    G_CALLBACK (service_added_cb), NULL);
  yts_client_connect (client1);

  client2 = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP,
                             "com.meego.ytstenut.SendMessageTest2");
  g_signal_connect (client2, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster2 = yts_client_get_roster (client2);
  g_signal_connect (roster2, "service-added",
                    G_CALLBACK (service_added_cb), NULL);
  yts_client_connect (client2);

  g_timeout_add_seconds (TEST_LENGTH, timeout_test_cb, loop);

  /*
   * Run the main loop.
   */
  g_main_loop_run (loop);

  g_object_unref (client1);
  g_object_unref (client2);

  g_main_loop_unref (loop);

  return retval;
}
