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

#include <ytstenut/ytstenut.h>

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
incoming_file_finished_cb (YtsClient *client,
                           const char *s1,
                           const char *s2,
                           gboolean    r,
                           gpointer    data)
{
  if (r)
    retval = 0;

  g_main_loop_quit (loop);
}

static gboolean
incoming_file_cb (YtsClient *client,
                  const char *from,
                  const char *name,
                  guint64     size,
                  guint64     offset,
                  TpChannel  *proxy,
                  gpointer    data)
{
  g_signal_connect (client, "incoming-file-finished",
                    G_CALLBACK (incoming_file_finished_cb), NULL);

  return TRUE;
}

static void
service_added_cb (YtsRoster *roster, YtsService *service, gpointer data)
{
  YtsClient  *client  = yts_roster_get_client (roster);
  YtsContact *contact = yts_service_get_contact (service);
  const char  *jid     = yts_contact_get_id (contact);
  const char  *sid     = yts_service_get_id (service);

  static YtsService *to = NULL;

  if (client == client1 && strstr (sid, "org.freedesktop.ytstenut.FileTransferTest2"))
    {
      ready1 = TRUE;
      to     = service;
    }

  if (client == client2 && strstr (sid, "org.freedesktop.ytstenut.FileTransferTest1"))
    {
      ready2   = TRUE;
    }

  /*
   * Waiting for both clients to appear ...
   */
  if (ready1 && ready2)
    {
      GFile     *file  = g_file_new_for_path ("file-transfer.c");
      YtsError  e;

      retval = 1;

      if (!file)
        {
          g_message ("Failed to open test file to transfer");
          g_main_loop_quit (loop);
        }

      g_signal_connect (client2, "incoming-file",
                        G_CALLBACK (incoming_file_cb), NULL);

      e = yts_contact_send_file (yts_service_get_contact (to), file);

      g_object_unref (file);

      if (yts_error_get_code (e) != YTS_ERROR_SUCCESS)
        {
          g_message ("Send file status %d:%d",
                     yts_error_get_atom (e),
                     yts_error_get_code (e));
          g_main_loop_quit (loop);
        }
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
                             "org.freedesktop.ytstenut.FileTransferTest1");
  g_signal_connect (client1, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster1 = yts_client_get_roster (client1);
  g_signal_connect (roster1, "service-added",
                    G_CALLBACK (service_added_cb), NULL);
  yts_client_connect (client1);

  client2 = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP,
                             "org.freedesktop.ytstenut.FileTransferTest2");
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
