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
status_cb (YtsClient *client, YtsStatus *status, gpointer data)
{
  YtsMetadata *md = (YtsMetadata*)status;

  g_assert (YTS_IS_STATUS (status));

  /* TODO -- check what is in the status ... */
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

  if (client == client1 && strstr (sid, "com.meego.ytstenut.SetStatusTest2"))
    {
      ready1   = TRUE;
      our      = TRUE;
      service2 = service;
    }

  if (client == client2 && strstr (sid, "com.meego.ytstenut.SetStatusTest1"))
    {
      ready2   = TRUE;
      our      = TRUE;

      g_signal_connect (service, "status", G_CALLBACK (status_cb), NULL);
    }

  /*
   * Waiting for both clients to appear ...
   */
  if (our && ready1 && ready2)
    {
      YtsStatus *status;
      const char   *attributes[] =
        {
          "capability", "urn:ytstenut:capabilities:yts-caps-video",
          "activity", "yts-activity-playing",
          "from-service", "com.meego.ytstenut.SetStatusTest1",
          NULL
        };

      g_debug ("Both test services are ready, setting status");

      status = yts_status_new ((const char**)&attributes);

      yts_client_set_status_by_capability (client1,
                                    "urn:ytstenut:capabilities:yts-caps-video",
                                    "yts-activity-playing");
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
                             "com.meego.ytstenut.SetStatusTest1");
  yts_client_set_capabilities (client1, YTS_CAPS_VIDEO);
  g_signal_connect (client1, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster1 = yts_client_get_roster (client1);
  g_signal_connect (roster1, "service-added",
                    G_CALLBACK (service_added_cb), NULL);
  yts_client_connect (client1);

  client2 = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP,
                             "com.meego.ytstenut.SetStatusTest2");
  yts_client_set_capabilities (client2, YTS_CAPS_VIDEO);
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
