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

#include <ytstenut-glib/ytsg-private.h>
#include <ytstenut-glib/ytsg-client.h>
#include <ytstenut-glib/ytsg-main.h>

#include <string.h>

#define TEST_LENGTH 10

static gboolean    ready1 = FALSE;
static gboolean    ready2 = FALSE;
static int         retval = -2;
static YtsgClient *client1 = NULL;
static YtsgClient *client2 = NULL;
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
authenticated_cb (YtsgClient *client, gpointer data)
{
  retval += 1;
}

static void
incoming_file_finished_cb (YtsgClient *client,
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
incoming_file_cb (YtsgClient *client,
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
service_added_cb (YtsgRoster *roster, YtsgService *service, gpointer data)
{
  YtsgClient  *client  = ytsg_roster_get_client (roster);
  YtsgContact *contact = ytsg_service_get_contact (service);
  const char  *jid     = ytsg_contact_get_jid (contact);
  const char  *sid     = ytsg_service_get_uid (service);

  static YtsgService *to = NULL;

  if (client == client1 && strstr (sid, "com.meego.ytstenut.FileTransferTest2"))
    {
      ready1 = TRUE;
      to     = service;
    }

  if (client == client2 && strstr (sid, "com.meego.ytstenut.FileTransferTest1"))
    {
      ready2   = TRUE;
    }

  /*
   * Waiting for both clients to appear ...
   */
  if (ready1 && ready2)
    {
      GFile     *file  = g_file_new_for_path ("file-transfer.c");
      YtsgError  e;

      retval = 1;

      if (!file)
        {
          g_message ("Failed to open test file to transfer");
          g_main_loop_quit (loop);
        }

      g_signal_connect (client2, "incoming-file",
                        G_CALLBACK (incoming_file_cb), NULL);

      e = ytsg_contact_send_file (ytsg_service_get_contact (to), file);

      g_object_unref (file);

      if (ytsg_error_get_code (e) != YTSG_ERROR_SUCCESS)
        {
          g_message ("Send file status %d:%d",
                     ytsg_error_get_atom (e),
                     ytsg_error_get_code (e));
          g_main_loop_quit (loop);
        }
    }
}

int
main (int argc, char **argv)
{
  YtsgRoster *roster1;
  YtsgRoster *roster2;

  ytsg_init (0, NULL);

  loop = g_main_loop_new (NULL, FALSE);

  client1 = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP,
                             "com.meego.ytstenut.FileTransferTest1");
  g_signal_connect (client1, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster1 = ytsg_client_get_roster (client1);
  g_signal_connect (roster1, "service-added",
                    G_CALLBACK (service_added_cb), NULL);
  ytsg_client_connect_to_mesh (client1);

  client2 = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP,
                             "com.meego.ytstenut.FileTransferTest2");
  g_signal_connect (client2, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster2 = ytsg_client_get_roster (client2);
  g_signal_connect (roster2, "service-added",
                    G_CALLBACK (service_added_cb), NULL);
  ytsg_client_connect_to_mesh (client2);

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
