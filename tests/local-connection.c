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
#include <ytstenut-glib/ytsg-client.h>
#include <ytstenut-glib/ytsg-status.h>
#include <string.h>

static gboolean
timeout_presence_cb (gpointer data)
{
#if 0
  NsClient         *client = data;
  NsStatus         *status;
  NsCapsTupple     *t = ns_caps_tupple_new (NS_CAPS_VIDEO,
                                            NS_ACTIVITY_PLAY,
                                            "http://youtube.com/something",
                                            NULL,
                                            "Playing Peter Pan trailer");

  status = ns_status_new (NS_PRESENCE_AVAILABLE, "en-GB",
                          NS_CAPS_VIDEO, 1, &t);

  ns_client_set_status (client, status);

  ns_status_free (status);
  ns_caps_tupple_free (t);
#endif
  return FALSE;
}

/*
 * Sample callback for the 'authenticated' signal; this should be rarely useful
 * to the client applications.
 */
static void
authenticated_cb (YtsgClient *client, gpointer data)
{
  g_message ("Client authenticated");

  g_timeout_add_seconds (15, timeout_presence_cb, client);
}

/*
 * Sample callback for the 'disconnected' signal.
 */
static void
disconnected_cb (YtsgClient *client, gpointer data)
{
  g_message ("Client disconnected");
}

/*
 * Sample callback for the YtsgContact 'notify::status' signal; this allows the
 * application to monitor status of items in the roster.
 *
 * (As a general rule, items that are not in available state should be hidden
 * in the UI.)
 */
static void
contact_status_cb (YtsgContact *item, GParamSpec *spec, gpointer data)
{
  g_message ("Status change on %p", item);
#if 0
  const YtsgStatus *status = ytsg_contact_get_status (item);

  ytsg_status_dump (status);
#endif
}

/*
 * Demonstrates how to send nScreen command to an item in the roster.
 */
static gboolean
timeout_cb (gpointer data)
{
#if 0
  YtsgContact *item = data;
  ytsg_contact_send_command (item, YTSG_CAPS_VIDEO, YTSG_ACTIVITY_PLAY,
                               "http://youtube.com/something", 0, NULL);
#endif
  return FALSE;
}

/*
 * Callback for the YtsgRoster 'item-added' signal; the roster UI in the
 * application will need to connect to this.
 */
static void
contact_added_cb (YtsgRoster *client, YtsgContact *item, gpointer data)
{
  const YtsgStatus *status;

  g_message ("Roster: added %p", item);

#if 0
  /*
   * Get the initial status of the item.
   */
  status = ytsg_contact_get_status (item);

  /*
   * Print to console for debugging purposes.
   */
  ytsg_status_dump (status);
#endif

  /*
   * Connect to the "notify::status" signal so we can monitor status changes.
   */
  g_signal_connect (item, "notify::status",
                    G_CALLBACK (contact_status_cb), NULL);

  g_timeout_add_seconds (10, timeout_cb, item);
}

/*
 * Callback for the YtsgRoster 'item-removed' signal; application roster UI will
 * need to connect to this to monitor when items are removed.
 *
 * NB: if the application took reference to the item, it should unref the
 *     object when it receives this signal.
 */
static void
contact_removed_cb (YtsgRoster *client, YtsgContact *item, gpointer data)
{
  g_message ("Roster: removed %p", item);
}

/*
 * Callback for the YtsgClient 'message' signal. Applications should rarely need
 * to connect to this signal (see the 'command' signal instead).
 */
static void
message_cb (YtsgClient *client, YtsgMessage *msg, gpointer data)
{
  g_debug ("Got message");
}


int
main (int argc, char **argv)
{
  YtsgClient *client;
  YtsgRoster *roster;
  GMainLoop *loop;

  /*
   * Initialize stuff needed for telepathy and the GObject type system.
   */

  ytsg_init (0, NULL);

  /*
   * create a main loop for this app
   *
   * (NB: when using a higher level toolkit, the main loop will be created for
   *      us.)
   */
  loop = g_main_loop_new (NULL, FALSE);

  /*
   * Construct the YtsgClient object and connect to it's signals that interest
   * us.
   */
  client = ytsg_client_new (YTSG_PROTOCOL_LOCAL_XMPP,
                            "testapp@local",
                            "testapp");

  g_signal_connect (client, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (disconnected_cb), NULL);
  g_signal_connect (client, "message",
                    G_CALLBACK (message_cb), NULL);

  roster = ytsg_client_get_roster (client);

  /*
   * Connect to any YtsgRoster object signals that we care about.
   *
   * NB: at this point the roster is empty, i.e., you cannot start populating
   *     the UI here.
   */
  g_signal_connect (roster, "contact-added",
                    G_CALLBACK (contact_added_cb), NULL);
  g_signal_connect (roster, "contact-removed",
                    G_CALLBACK (contact_removed_cb), NULL);

  /*
   * Initiate network connection.
   */
  ytsg_client_connect_to_mesh (client);

  /*
   * Run the main loop.
   */
  g_main_loop_run (loop);

  /*
   * Now dispose of the client.
   *
   * NB: this is important; if you do not do this, the Telepathy connection
   *     will remain open, and you will not be able to reconnect.
   */
  g_object_unref (client);

  g_main_loop_unref (loop);

  return 0;
}
