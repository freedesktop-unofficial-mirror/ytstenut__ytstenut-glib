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
contact_added_cb (YtsgRoster *roster, YtsgContact *item, gpointer data)
{
  YtsgClient *client = ytsg_roster_get_client (roster);
  const char *cjid   = ytsg_contact_get_jid (item);
  gboolean    our    = FALSE;

  g_debug ("Contact %s", cjid);

  if (client == client1 && strstr (cjid, "ft-testapp2"))
    {
      ready1 = TRUE;
      our    = TRUE;
    }

  if (client == client2 && strstr (cjid, "ft-testapp1@"))
    {
      ready2 = TRUE;
      our    = TRUE;
    }

  /*
   * Waiting for both clients to appear ...
   */
  if (our && ready1 && ready2)
    {
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
                             "com.meego.ytstenut.SendMessageTest1");
  g_signal_connect (client1, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster1 = ytsg_client_get_roster (client1);
  g_signal_connect (roster1, "contact-added",
                    G_CALLBACK (contact_added_cb), NULL);
  ytsg_client_connect_to_mesh (client1);

  client2 = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP,
                             "com.meego.ytstenut.SendMessageTest2");
  g_signal_connect (client2, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  roster2 = ytsg_client_get_roster (client2);
  g_signal_connect (roster2, "contact-added",
                    G_CALLBACK (contact_added_cb), NULL);
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
