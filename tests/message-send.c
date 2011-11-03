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

#include <string.h>
#include <ytstenut/ytstenut.h>
#include "ytstenut/yts-message.h"

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
_dictionary_message (YtsClient          *client,
                     char const *const  *dictionary,
                     gpointer            data)
{
  unsigned    length;
  unsigned    i;
  char const *a1 = NULL;
  char const *a2 = NULL;

  length = g_strv_length ((char **) dictionary);

  g_assert (length == 4);

  for (i = 0; i < length && !(a1 && a2); i++) {
    if (0 == g_strcmp0 ("a1", dictionary[2*i])) {
      a1 = dictionary[2*i + 1];
    } else if (0 == g_strcmp0 ("a2", dictionary[2*i])) {
      a2 = dictionary[2*i + 1];
    }
  }

  g_assert_cmpstr ("v1", ==, a1);
  g_assert_cmpstr ("v2", ==, a2);

  retval = 0;
  g_main_loop_quit (loop);
}

static void
service_added_cb (YtsRoster *roster, YtsService *service, YtsClient *client)
{
  const char  *sid     = yts_service_get_id (service);
  gboolean     our     = FALSE;

  static YtsService *service2 = NULL;

  g_debug ("Service %s", sid);

  if (client == client1 && strstr (sid, "org.freedesktop.ytstenut.SendMessageTest2"))
    {
      ready1   = TRUE;
      our      = TRUE;
      service2 = service;
    }

  if (client == client2 && strstr (sid, "org.freedesktop.ytstenut.SendMessageTest1"))
    {
      ready2   = TRUE;
      our      = TRUE;

      g_signal_connect (client, "dictionary-message",
                        G_CALLBACK (_dictionary_message), NULL);
    }

  /*
   * Waiting for both clients to appear ...
   */
  if (our && ready1 && ready2)
    {
      const char   *attributes[] =
        {
          "a1", "v1",
          "a2", "v2",
          NULL
        };

      g_debug ("Both test services are ready, sending message");

      yts_service_send_dictionary (service2, attributes, -1);
    }
}

int
main (int argc, char **argv)
{
  YtsRoster *roster1;
  YtsRoster *roster2;

  g_type_init ();

  loop = g_main_loop_new (NULL, FALSE);

  client1 = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP,
                             "org.freedesktop.ytstenut.SendMessageTest1");
  g_signal_connect (client1, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster1 = yts_client_get_roster (client1);
  g_signal_connect (roster1, "service-added",
                    G_CALLBACK (service_added_cb), client1);
  yts_client_connect (client1);

  client2 = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP,
                             "org.freedesktop.ytstenut.SendMessageTest2");
  g_signal_connect (client2, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster2 = yts_client_get_roster (client2);
  g_signal_connect (roster2, "service-added",
                    G_CALLBACK (service_added_cb), client2);
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
