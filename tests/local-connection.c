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
#include <ytstenut/yts-client.h>
#include <ytstenut/yts-status.h>
#include <ytstenut/yts-main.h>

#include <string.h>

#define TEST_LENGTH 5

static int retval = 1;

static gboolean
timeout_test_cb (gpointer data)
{
  GMainLoop *loop = data;

  g_message ("TIMEOUT: quiting local connection test");

  g_main_loop_quit (loop);

  return FALSE;
}

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
authenticated_cb (YtsClient *client, gpointer data)
{
  g_message ("Client %s authenticated", yts_client_get_jid (client));

  retval = 0;

  g_timeout_add_seconds (15, timeout_presence_cb, client);
}

/*
 * Sample callback for the 'disconnected' signal.
 */
static void
disconnected_cb (YtsClient *client, gpointer data)
{
  g_message ("Client disconnected");
}

/*
 * Sample callback for the YtsContact 'notify::status' signal; this allows the
 * application to monitor status of items in the roster.
 *
 * (As a general rule, items that are not in available state should be hidden
 * in the UI.)
 */
static void
contact_status_cb (YtsContact *item, GParamSpec *spec, gpointer data)
{
  g_message ("Status change on %p", item);
#if 0
  const YtsStatus *status = yts_contact_get_status (item);

  yts_status_dump (status);
#endif
}

/*
 * Demonstrates how to send nScreen command to an item in the roster.
 */
static gboolean
timeout_cb (gpointer data)
{
#if 0
  YtsContact *item = data;
  yts_contact_send_command (item, YTS_CAPS_VIDEO, YTS_ACTIVITY_PLAY,
                               "http://youtube.com/something", 0, NULL);
#endif
  return FALSE;
}

/*
 * Callback for the YtsRoster 'contact-added' signal; the roster UI in the
 * application will need to connect to this.
 */
static void
contact_added_cb (YtsRoster *client, YtsContact *item, gpointer data)
{
  g_message ("Roster: added %p", item);

  /*
   * Connect to the "notify::status" signal so we can monitor status changes.
   */
  g_signal_connect (item, "notify::status",
                    G_CALLBACK (contact_status_cb), NULL);

  g_timeout_add_seconds (10, timeout_cb, item);
}

/*
 * Callback for the YtsRoster 'item-removed' signal; application roster UI will
 * need to connect to this to monitor when items are removed.
 *
 * NB: if the application took reference to the item, it should unref the
 *     object when it receives this signal.
 */
static void
contact_removed_cb (YtsRoster *client, YtsContact *item, gpointer data)
{
  g_message ("Roster: removed %p", item);
}

/*
 * Callback for the YtsClient 'message' signal. Applications should rarely need
 * to connect to this signal (see the 'command' signal instead).
 */
static void
message_cb (YtsClient *client, YtsMessage *msg, gpointer data)
{
  g_debug ("Got message");
}


int
main (int argc, char **argv)
{
  YtsClient *client;
  YtsRoster *roster;
  GMainLoop *loop;

  /*
   * Initialize stuff needed for telepathy and the GObject type system.
   */

  yts_init (0, NULL);

  /*
   * create a main loop for this app
   *
   * (NB: when using a higher level toolkit, the main loop will be created for
   *      us.)
   */
  loop = g_main_loop_new (NULL, FALSE);

  /*
   * Construct the YtsClient object and connect to it's signals that interest
   * us.
   */
  client = yts_client_new (YTS_PROTOCOL_LOCAL_XMPP,
                            "com.meego.ytstenut.LocalConnectionTest");

  g_signal_connect (client, "authenticated",
                    G_CALLBACK (authenticated_cb), NULL);
  g_signal_connect (client, "disconnected",
                    G_CALLBACK (disconnected_cb), NULL);
  g_signal_connect (client, "message",
                    G_CALLBACK (message_cb), NULL);

  roster = yts_client_get_roster (client);

  /*
   * Connect to any YtsRoster object signals that we care about.
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
  yts_client_connect (client);

  g_timeout_add_seconds (TEST_LENGTH, timeout_test_cb, loop);

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

  return retval;
}
