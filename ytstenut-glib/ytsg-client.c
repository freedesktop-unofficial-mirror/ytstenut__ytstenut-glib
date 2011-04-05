/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2011 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ytsg-client.h"
#include "ytsg-debug.h"
#include "ytsg-enum-types.h"
#include "ytsg-error.h"
#include "ytsg-marshal.h"
#include "ytsg-metadata.h"
#include "ytsg-private.h"
#include "ytsg-roster.h"
#include "ytsg-types.h"

#include "empathy-tp-file.h"

#include <string.h>
#include <telepathy-glib/telepathy-glib.h>
#include <telepathy-glib/connection-manager.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/connection.h>
#include <telepathy-glib/account.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/util.h>
#include <telepathy-glib/contact.h>
#include <telepathy-glib/debug.h>
#include <telepathy-glib/proxy-subclass.h>
#include <telepathy-ytstenut-glib/telepathy-ytstenut-glib.h>

#define RECONNECT_DELAY 20 /* in seconds */

static void ytsg_client_dispose (GObject *object);
static void ytsg_client_finalize (GObject *object);
static void ytsg_client_constructed (GObject *object);
static void ytsg_client_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec);
static void ytsg_client_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec);
static void ytsg_client_make_connection (YtsgClient *client);

G_DEFINE_TYPE (YtsgClient, ytsg_client, G_TYPE_OBJECT);

#define YTSG_CLIENT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_CLIENT, YtsgClientPrivate))

struct _YtsgClientPrivate
{
  YtsgRoster   *roster;    /* the roster of this client */
  YtsgRoster   *unwanted;  /* roster of unwanted items */
  GArray       *caps;

  /* connection parameters */
  char         *uid;
  char         *mgr_name;
  YtsgProtocol  protocol;

  char         *incoming_dir; /* destination directory for incoming files */

  /* avatar-related stuff */
  char         *icon_token;
  char         *icon_mime_type;
  GArray       *icon_data;

  /* Telepathy bits */
  TpDBusDaemon         *dbus;
  TpYtsAccountManager  *mgr;
  TpAccount            *account;
  TpConnection         *connection;
  TpProxy              *debug_proxy;
  TpYtsStatus          *tp_status;
  YtsgStatus           *status;

  /* callback ids */
  guint reconnect_id;

  guint authenticated   : 1; /* are we authenticated ? */
  guint ready           : 1; /* is TP setup done ? */
  guint connect         : 1; /* connect once we get our connection ? */
  guint reconnect       : 1; /* should we attempt to reconnect ? */
  guint dialing         : 1; /* are we currently acquiring connection ? */
  guint members_pending : 1; /* requery members when TP set up completed ? */
  guint prepared        : 1; /* are connection features set up ? */
  guint disposed        : 1; /* dispose guard */
};

enum
{
  AUTHENTICATED,
  READY,
  DISCONNECTED,
  MESSAGE,
  ERROR,
  INCOMING_FILE,
  INCOMING_FILE_FINISHED,
  N_SIGNALS,
};

enum
{
  PROP_0,
  PROP_UID,
  PROP_PROTOCOL,
  PROP_CAPABILITIES,
  PROP_ICON_TOKEN,
};

static guint signals[N_SIGNALS] = {0};

static gboolean
ytsg_client_channel_requested (TpChannel *proxy)
{
  GHashTable *props;
  gboolean    requested;

  props = tp_channel_borrow_immutable_properties ((TpChannel*)proxy);

  requested = tp_asv_get_boolean (props, TP_PROP_CHANNEL_REQUESTED, NULL);

  return requested;
}

static void
ytsg_client_ft_op_cb (EmpathyTpFile *tp_file,
                      const GError  *error,
                      gpointer       data)
{
  if (error)
    {
      g_warning ("Incoming file transfer failed: %s", error->message);
    }
}

static void
ytsg_client_ft_accept_cb (TpProxy      *proxy,
                          GHashTable   *props,
                          const GError *error,
                          gpointer      data,
                          GObject      *weak_object)
{
  YtsgClient        *client = data;
  YtsgClientPrivate *priv   = client->priv;
  const char        *name;
  const char        *jid;
  guint64            offset;
  guint64            size;
  GHashTable        *iprops;
  YtsgContact       *item;
  guint32            ihandle;

  iprops = tp_channel_borrow_immutable_properties ((TpChannel*)proxy);

  ihandle = tp_asv_get_uint32 (iprops,
                               TP_PROP_CHANNEL_INITIATOR_HANDLE,
                               NULL);

  if ((item = _ytsg_roster_find_contact_by_handle (priv->roster, ihandle)))
    {
      jid = ytsg_contact_get_jid (item);
    }
  else
    {
      g_warning ("Unknown originator with handle %d", ihandle);

      tp_cli_channel_call_close ((TpChannel*)proxy,
                                 -1,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);

      return;
    }

  tp_asv_dump (props);

  name   = tp_asv_get_string (props, "Filename");
  offset = tp_asv_get_uint64 (props, "InitialOffset", NULL);
  size   = tp_asv_get_uint64 (props, "Size", NULL);

  if (!size || size < offset)
    {
      g_warning ("Meaningless file size");

      tp_cli_channel_call_close ((TpChannel*)proxy,
                                 -1,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);

      return;
    }

  g_signal_emit (client, signals[INCOMING_FILE], 0, jid, name, size, offset,
                 proxy);
}

static void
ytsg_client_ft_handle_state (YtsgClient *client, TpChannel *proxy, guint state)
{
  YtsgClientPrivate *priv   = client->priv;
  GHashTable        *props;
  gboolean           requested;

  props = tp_channel_borrow_immutable_properties ((TpChannel*)proxy);
  if (!(requested = tp_asv_get_boolean (props, TP_PROP_CHANNEL_REQUESTED,NULL)))
    {
      YtsgContact *item;
      guint32      ihandle;

      ihandle = tp_asv_get_uint32 (props,
                                   TP_PROP_CHANNEL_INITIATOR_HANDLE,
                                   NULL);
      item = _ytsg_roster_find_contact_by_handle (priv->roster, ihandle);

      switch (state)
        {
        case 1:
          {
            if (item)
              YTSG_NOTE (FILE_TRANSFER,
                         "Got request for FT channel from %s (%s)",
                         ytsg_contact_get_jid (item),
                         tp_proxy_get_bus_name (proxy));
            else
              YTSG_NOTE (FILE_TRANSFER,
                         "Got request for FT channel from handle %d",
                         ihandle);

            tp_cli_dbus_properties_call_get_all (proxy,
                                           -1,
                                           TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,
                                           ytsg_client_ft_accept_cb,
                                           client,
                                           NULL,
                                           (GObject*) client);
          }
          break;
        case 2:
          YTSG_NOTE (FILE_TRANSFER, "Incoming stream state (%s) --> 'accepted'",
                     tp_proxy_get_bus_name (proxy));
          break;
        case 3:
          YTSG_NOTE (FILE_TRANSFER, "Incoming stream state (%s) --> 'open'",
                     tp_proxy_get_bus_name (proxy));
          break;
        case 4:
        case 5:
          YTSG_NOTE (FILE_TRANSFER, "Incoming stream state (%s) --> '%s'",
                     tp_proxy_get_bus_name (proxy),
                     state == 4 ? "completed" : "cancelled");
          {
            const char *name;
            const char *jid;

            if (item)
              {
                jid = ytsg_contact_get_jid (item);

                name   = tp_asv_get_string (props, "Filename");

                g_signal_emit (client, signals[INCOMING_FILE_FINISHED], 0,
                               jid, name, state == 4 ? TRUE : FALSE);
              }
          }
          break;
        default:
          YTSG_NOTE (FILE_TRANSFER, "Invalid value of stream state: %d", state);
        }
    }
  else
    YTSG_NOTE (FILE_TRANSFER, "The FT channel was requested by us ... (%s)",
             tp_proxy_get_bus_name (proxy));
}

static void
ytsg_client_ft_state_cb (TpChannel *proxy,
                         guint      state,
                         guint      reason,
                         gpointer   data,
                         GObject   *object)
{
  YtsgClient *client = data;

  YTSG_NOTE (FILE_TRANSFER,
             "FT channel changed status to %d (reason %d)", state, reason);

  ytsg_client_ft_handle_state (client, proxy, state);
}

static void
ytsg_client_ft_core_cb (GObject *proxy, GAsyncResult *res, gpointer data)
{
  YtsgClient *client  = data;
  TpChannel  *channel = (TpChannel*) proxy;
  GError     *error   = NULL;

  YTSG_NOTE (FILE_TRANSFER, "FT channel ready");

  tp_cli_channel_type_file_transfer_connect_to_file_transfer_state_changed
    (channel,
     ytsg_client_ft_state_cb,
     client,
     NULL,
     (GObject*)client,
     &error);

  if (!ytsg_client_channel_requested (channel))
    ytsg_client_ft_handle_state (client, channel, 1);
}

static void
ytsg_client_channel_cb (TpConnection *proxy,
                        const gchar  *path,
                        const gchar  *type,
                        guint         handle_type,
                        guint         handle,
                        gboolean      suppress_handle,
                        gpointer      data,
                        GObject      *weak_object)
{
  YtsgClient        *client = data;
  YtsgClientPrivate *priv   = client->priv;

  if (!path)
    {
      g_warning (G_STRLOC ":%s: no path!", __FUNCTION__);
      return;
    }

  YTSG_NOTE (CLIENT, "New channel: %s: %s: h type %d, h %d",
             path, type, handle_type, handle);

  switch (handle_type)
    {
    case TP_HANDLE_TYPE_CONTACT:
      /* FIXME -- this is where the messaging channel will go */
      if (!strcmp (type, TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER))
        {
          GError      *error = NULL;
          TpChannel   *ch;
          GQuark       features[] = { TP_CHANNEL_FEATURE_CORE, 0};
          YtsgContact *item;

          ch = tp_channel_new (proxy, path, type, handle_type, handle, &error);

          if ((item = _ytsg_roster_find_contact_by_handle (priv->roster,
                                                           handle)))
            {
              _ytsg_contact_set_ft_channel (item, ch);

              tp_proxy_prepare_async (ch, features,
                                      ytsg_client_ft_core_cb, client);
            }
          else
            {
              g_warning (G_STRLOC ": orphaned channel ?");
              g_object_unref (ch);
            }
        }
      break;
    case TP_HANDLE_TYPE_LIST:
      break;
    case TP_HANDLE_TYPE_GROUP:
      break;
    default:;
    }
}

static void
ytsg_client_authenticated (YtsgClient *client)
{
  YtsgClientPrivate *priv = client->priv;

  priv->authenticated = TRUE;

  YTSG_NOTE (CLIENT, "Authenticated");
}

static void
ytsg_client_ready (YtsgClient *client)
{
  YtsgClientPrivate *priv = client->priv;

  priv->ready = TRUE;

  YTSG_NOTE (CLIENT, "TP Channel is ready");

  if (priv->tp_status && priv->status)
    {
      char *xml = ytsg_metadata_to_string ((YtsgMetadata*)priv->status);
      int   i;

      for (i = 0; i < priv->caps->len; ++i)
        {
          const char *c;

          c = g_quark_to_string (g_array_index (priv->caps, YtsgCaps, i));

          tp_yts_status_advertise_status_async (priv->tp_status,
                                                c,
                                                priv->uid,
                                                xml,
                                                NULL,
                                                NULL,
                                                NULL);
        }

      g_free (xml);
    }
}

static void
ytsg_client_cleanup_connection_resources (YtsgClient *client)
{
  YtsgClientPrivate *priv = client->priv;

  /*
   * Clean up items associated with this connection.
   */

  priv->ready    = FALSE;
  priv->prepared = FALSE;

  /*
   * Empty roster
   */
  if (priv->roster)
    _ytsg_roster_clear (priv->roster);

  if (priv->connection)
    {
      g_object_unref (priv->connection);
      priv->connection = NULL;
    }
}

static void
ytsg_client_disconnected (YtsgClient *client)
{
  YtsgClientPrivate *priv = client->priv;

  ytsg_client_cleanup_connection_resources (client);

  if (priv->reconnect)
    _ytsg_client_reconnect_after (client, RECONNECT_DELAY);
}

static void
ytsg_client_message (YtsgClient *client, YtsgMessage *msg)
{
  g_signal_emit (client, signals[MESSAGE], 0, msg);
}

static gboolean
ytsg_client_incoming_file (YtsgClient *client,
                           const char *from,
                           const char *name,
                           guint64     size,
                           guint64     offset,
                           TpChannel  *proxy)
{
  YtsgClientPrivate *priv = client->priv;
  char              *path;
  GFile             *gfile;
  EmpathyTpFile     *tp_file;
  GCancellable      *cancellable;

  YTSG_NOTE (FILE_TRANSFER, "Incoming file from %s", from);

  if (g_mkdir_with_parents (priv->incoming_dir, 0700))
    {
      g_warning ("Unable to create directory %s", priv->incoming_dir);

      tp_cli_channel_call_close (proxy,
                                 -1,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);

      return FALSE;
    }

  path = g_build_filename (priv->incoming_dir, name, NULL);

  gfile = g_file_new_for_path (path);

  tp_file = empathy_tp_file_new ((TpChannel*)proxy, TRUE);

  cancellable = g_cancellable_new ();

  empathy_tp_file_accept (tp_file, offset, gfile,
                          cancellable,
                          NULL /*progress_callback*/,
                          NULL /*progress_user_data*/,
                          ytsg_client_ft_op_cb,
                          client);

  g_free (path);
  g_object_unref (gfile);
  g_object_unref (cancellable);

  return TRUE;
}

static gboolean
ytsg_client_stop_accumulator (GSignalInvocationHint *ihint,
                              GValue                *accumulated,
                              const GValue          *returned,
                              gpointer               data)
{
  gboolean cont = g_value_get_boolean (returned);

  g_value_set_boolean (accumulated, cont);

  return cont;
}

static void
ytsg_client_class_init (YtsgClientClass *klass)
{
  GParamSpec   *pspec;
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgClientPrivate));

  object_class->dispose      = ytsg_client_dispose;
  object_class->finalize     = ytsg_client_finalize;
  object_class->constructed  = ytsg_client_constructed;
  object_class->get_property = ytsg_client_get_property;
  object_class->set_property = ytsg_client_set_property;

  klass->authenticated       = ytsg_client_authenticated;
  klass->ready               = ytsg_client_ready;
  klass->disconnected        = ytsg_client_disconnected;
  klass->message             = ytsg_client_message;
  klass->incoming_file       = ytsg_client_incoming_file;

  /**
   * YtsgClient:uid:
   *
   * The uid of this service
   *
   * Since: 0.1
   */
  pspec = g_param_spec_string ("uid",
                               "Service UID",
                               "Service UID",
                               NULL,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_UID, pspec);

  pspec = g_param_spec_enum ("protocol",
                             "Protocol",
                             "Protocol",
                             YTSG_TYPE_PROTOCOL,
                             0,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_PROTOCOL, pspec);

  /**
   * YtsgClient:capabilites:
   *
   * Capabilities of this client, used to filter roster.
   */
  pspec = g_param_spec_boxed ("capabilities",
                              "Capabilities",
                              "Capabilities of this client",
                              G_TYPE_ARRAY,
                              G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CAPABILITIES, pspec);


  /**
   * YtsgClient::authenticated
   * @client: object which emitted the signal,
   *
   * The authenticated signal is emited when connection to the nScreen server
   * is successfully established.
   *
   * Since: 0.1
   */
  signals[AUTHENTICATED] =
    g_signal_new (I_("authenticated"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (YtsgClientClass, authenticated),
                  NULL, NULL,
                  ytsg_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * YtsgClient::ready
   * @client: object which emitted the signal,
   *
   * The ready signal is emited when the initial Telepathy set up is ready.
   * (In practical terms this means the subscription channels are prepared.)
   *
   * Since: 0.1
   */
  signals[READY] =
    g_signal_new (I_("ready"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (YtsgClientClass, ready),
                  NULL, NULL,
                  ytsg_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * YtsgClient::disconnected
   * @client: object which emitted the signal,
   *
   * The disconnected signal is emited when connection to the nScreen server
   * is successfully established.
   *
   * Since: 0.1
   */
  signals[DISCONNECTED] =
    g_signal_new (I_("disconnected"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (YtsgClientClass, disconnected),
                  NULL, NULL,
                  ytsg_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * YtsgClient::message
   * @client: object which emitted the signal,
   * @message: #YtsgMessage, the message
   *
   * The message signal is emitted when message is received from one of the
   * contacts.
   *
   * Since: 0.1
   */
  signals[MESSAGE] =
    g_signal_new (I_("message"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsgClientClass, message),
                  NULL, NULL,
                  ytsg_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTSG_TYPE_MESSAGE);

  /**
   * YtsgClient::error
   * @client: object which emitted the signal,
   * @error: #YtsgError
   *
   * The error signal is emitted to indicate an error (or eventual success)
   * during the handling of an operation for which the nScreen API initially
   * returned %YTSG_ERROR_PENDING. The original operation can be determined
   * using the atom part of the #YtsgError parameter.
   *
   * Since: 0.1
   */
  signals[ERROR] =
    g_signal_new (I_("error"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  ytsg_marshal_VOID__UINT,
                  G_TYPE_NONE, 1,
                  G_TYPE_UINT);

  /**
   * YtsgClient::incoming-file
   * @client: object which emitted the signal,
   * @from: jid of the originator
   * @name: name of the file
   * @size: size of the file
   * @offset: offset into the file,
   * @channel: #TpChannel
   *
   * The ::incoming-file signal is emitted when the client receives
   * incoming request for a file transfer. The signal closure will
   * kickstart the transfer -- this can be prevented by a connected handler
   * returning %FALSE.
   *
   * Since: 0.1
   */
  signals[INCOMING_FILE] =
    g_signal_new (I_("incoming-file"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsgClientClass, incoming_file),
                  ytsg_client_stop_accumulator, NULL,
                  ytsg_marshal_BOOLEAN__STRING_STRING_UINT64_UINT64_OBJECT,
                  G_TYPE_BOOLEAN, 5,
                  G_TYPE_STRING,
                  G_TYPE_STRING,
                  G_TYPE_UINT64,
                  G_TYPE_UINT64,
                  TP_TYPE_CHANNEL);

  /**
   * YtsgClient::incoming-file-finished
   * @client: object which emitted the signal,
   * @from: jid of the originator
   * @name: name of the file
   * @success: %TRUE if the transfer was completed successfully.
   *
   * The ::incoming-file-finished signal is emitted when a file tranfers is
   * completed.
   *
   * Since: 0.1
   */
  signals[INCOMING_FILE_FINISHED] =
    g_signal_new (I_("incoming-file-finished"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  ytsg_marshal_VOID__STRING_STRING_BOOLEAN,
                  G_TYPE_NONE, 3,
                  G_TYPE_STRING,
                  G_TYPE_STRING,
                  G_TYPE_BOOLEAN);
}

/*
 * Handler for Mgr debug output.
 */
static void
ytsg_client_debug_msg_cb (TpProxy    *proxy,
                          gdouble     timestamp,
                          const char *domain,
                          guint       level,
                          const char *msg,
                          gpointer    data,
                          GObject    *weak_object)
{
  switch (level)
    {
    case 0:
      g_error ("%s: %s", domain, msg);
      break;
    case 1:
      g_critical ("%s: %s", domain, msg);
      break;
    case 2:
      g_warning ("%s: %s", domain, msg);
      break;
    default:
    case 3:
    case 4:
      g_message ("%s: %s", domain, msg);
      break;
    case 5:
      YTSG_NOTE (MANAGER, "%s: %s", domain, msg);
    }
}

/*
 * The machinery for adding the NewDebugMessage signal; this is PITA, and can
 * probably be autogenerated from somewhere, but no documentation.
 *
 * TODO - check we cannot connect directly to the dbus proxy avoiding all
 * this unsightly marshaling.
 *
 * First, the collect function
 */
static void
ytsg_client_debug_msg_collect (DBusGProxy              *proxy,
                               gdouble                  timestamp,
                               const char              *domain,
                               guint                    level,
                               const char              *msg,
                               TpProxySignalConnection *signal)
{
  GValueArray *args = g_value_array_new (4);
  GValue t = { 0 };

  g_value_init (&t, G_TYPE_DOUBLE);
  g_value_set_double (&t, timestamp);
  g_value_array_append (args, &t);
  g_value_unset (&t);

  g_value_init (&t, G_TYPE_STRING);
  g_value_set_string (&t, domain);
  g_value_array_append (args, &t);
  g_value_unset (&t);

  g_value_init (&t, G_TYPE_UINT);
  g_value_set_uint (&t, level);
  g_value_array_append (args, &t);
  g_value_unset (&t);

  g_value_init (&t, G_TYPE_STRING);
  g_value_set_string (&t, msg);
  g_value_array_append (args, &t);

  tp_proxy_signal_connection_v0_take_results (signal, args);
}

typedef void (*YtsgClientMgrNewDebugMsg)(TpProxy *,
                                         gdouble,
                                         const char *,
                                         guint,
                                         const char *,
                                         gpointer, GObject *);

/*
 * The callback invoker
 */
static void
ytsg_client_debug_msg_invoke (TpProxy     *proxy,
                              GError      *error,
                              GValueArray *args,
                              GCallback    callback,
                              gpointer     data,
                              GObject     *weak_object)
{
  YtsgClientMgrNewDebugMsg cb = (YtsgClientMgrNewDebugMsg) callback;

  if (cb)
    {
      cb (g_object_ref (proxy),
          g_value_get_double (args->values),
          g_value_get_string (args->values + 1),
          g_value_get_uint (args->values + 2),
          g_value_get_string (args->values + 3),
          data,
          weak_object);

      g_object_unref (proxy);
    }

  g_value_array_free (args);
}

/*
 * Connects to the signal(s) and enable debugging output.
 */
static void
ytsg_client_connect_debug_signals (YtsgClient *client, TpProxy *proxy)
{
  GError   *error = NULL;
  GValue    v = {0};
  GType     expected[] =
    {
      G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING,
      G_TYPE_INVALID
    };

  g_value_init (&v, G_TYPE_BOOLEAN);
  g_value_set_boolean (&v, TRUE);

  tp_proxy_signal_connection_v0_new (proxy,
                                     TP_IFACE_QUARK_DEBUG,
                                     "NewDebugMessage",
                                     &expected[0],
                                     G_CALLBACK (ytsg_client_debug_msg_collect),
                                     ytsg_client_debug_msg_invoke,
                                     G_CALLBACK (ytsg_client_debug_msg_cb),
                                     client,
                                     NULL,
                                     (GObject*)client,
                                     &error);

  if (error)
    {
      YTSG_NOTE (CLIENT, "%s", error->message);
      g_clear_error (&error);
    }

  tp_cli_dbus_properties_call_set (proxy, -1, TP_IFACE_DEBUG,
                                   "Enabled", &v, NULL, NULL, NULL, NULL);
}

/*
 * Callback for TpProxy::interface-added: we need to add the signals we
 * care for here.
 *
 * TODO -- should we not be able to connect directly to the signal bypassing
 * the unsightly TP machinery ?
 */
static void
ytsg_client_debug_iface_added_cb (TpProxy    *tproxy,
                                  guint       id,
                                  DBusGProxy *proxy,
                                  gpointer    data)
{
  if (id != TP_IFACE_QUARK_DEBUG)
    return;

  dbus_g_proxy_add_signal (proxy, "NewDebugMessage",
                           G_TYPE_DOUBLE,
                           G_TYPE_STRING,
                           G_TYPE_UINT,
                           G_TYPE_STRING,
                           G_TYPE_INVALID);
}

static void
ytsg_client_setup_debug  (YtsgClient *client)
{
  YtsgClientPrivate *priv = client->priv;
  TpDBusDaemon      *dbus;
  GError            *error = NULL;
  TpProxy           *proxy;
  char              *busname;

  dbus = tp_dbus_daemon_dup (&error);

  if (error != NULL)
    {
      g_warning (error->message);
      g_clear_error (&error);
      return;
    }

  busname = g_strdup_printf ("org.freedesktop.Telepathy.ConnectionManager.%s",
                             priv->mgr_name);
  proxy =
    g_object_new (TP_TYPE_PROXY,
                  "bus-name", busname,
                  "dbus-daemon", dbus,
                  "object-path", "/org/freedesktop/Telepathy/debug",
                  NULL);

  priv->debug_proxy = proxy;

  g_signal_connect (proxy, "interface-added",
                    G_CALLBACK (ytsg_client_debug_iface_added_cb), client);

  tp_proxy_add_interface_by_id (proxy, TP_IFACE_QUARK_DEBUG);

  /*
   * Connecting to the signals triggers the interface-added signal
   */
  ytsg_client_connect_debug_signals (client, proxy);

  g_object_unref (dbus);
  g_free (busname);
}

/*
 * Callback from the async tp_account_prepare_async() call
 *
 * This function is ready for the New World Order according to Ytstenut ...
 */
static void
ytsg_client_account_prepared_cb (GObject      *acc,
                                 GAsyncResult *res,
                                 gpointer      data)
{
  YtsgClient        *client  = data;
  YtsgClientPrivate *priv    = client->priv;
  GError            *error   = NULL;
  TpAccount         *account = (TpAccount*)acc;

  if (!tp_account_prepare_finish (account, res, &error))
    {
      g_error ("Account unprepared: %s", error->message);
    }

  priv->account = account;

  YTSG_NOTE (CLIENT, "Account successfully opened");

  /*
   * If connection has been requested already, make one
   */
  if (priv->connect)
    ytsg_client_make_connection (client);
}

static void
ytsg_client_account_cb (GObject *object, GAsyncResult *res, gpointer data)
{
  YtsgClient        *client     = (YtsgClient*) data;
  YtsgClientPrivate *priv       = client->priv;
  GError            *error      = NULL;
  const GQuark       features[] = { TP_ACCOUNT_FEATURE_CORE, 0 };

  g_assert (TP_IS_YTS_ACCOUNT_MANAGER (object));
  g_assert (G_IS_ASYNC_RESULT (res));

  priv->account =
    tp_yts_account_manager_get_account_finish (TP_YTS_ACCOUNT_MANAGER (object),
                                               res, &error);

  if (error)
    g_error ("Could not access account: %s", error->message);

  if (ytsg_debug_flags & YTSG_DEBUG_TP)
    tp_debug_set_flags ("all");

  if (ytsg_debug_flags & YTSG_DEBUG_MANAGER)
    ytsg_client_setup_debug (client);

  tp_account_prepare_async (priv->account,
                            &features[0],
                            ytsg_client_account_prepared_cb,
                            client);
}

static void
ytsg_client_constructed (GObject *object)
{
  YtsgClient          *client    = (YtsgClient*) object;
  YtsgClientPrivate   *priv      = client->priv;
  GError              *error     = NULL;

  if (G_OBJECT_CLASS (ytsg_client_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_client_parent_class)->constructed (object);

  priv->roster   = _ytsg_roster_new (client);
  priv->unwanted = _ytsg_roster_new (client);

  if (!priv->uid || !*priv->uid)
    g_error ("UID must be set at construction time.");

  if (priv->protocol == YTSG_PROTOCOL_LOCAL_XMPP)
    priv->mgr_name = g_strdup ("salut");
  else
    g_error ("Unknown protocol requested");

  priv->dbus = tp_dbus_daemon_dup (&error);

  if (error)
    {
      g_error ("Can't connect to dbus: %s", error->message);
      return;
    }

  priv->mgr = tp_yts_account_manager_dup ();


  if (!TP_IS_YTS_ACCOUNT_MANAGER (priv->mgr))
    g_error ("Missing Account Manager");

  tp_yts_account_manager_hold (priv->mgr);

  tp_yts_account_manager_get_account_async (priv->mgr, NULL,
                                            ytsg_client_account_cb,
                                            client);
}

static void
ytsg_client_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsgClient        *client = (YtsgClient*) object;
  YtsgClientPrivate *priv   = client->priv;

  switch (property_id)
    {
    case PROP_UID:
      g_value_set_string (value, priv->uid);
      break;
    case PROP_ICON_TOKEN:
      g_value_set_string (value, priv->icon_token);
      break;
    case PROP_CAPABILITIES:
      g_value_set_boxed (value, priv->caps);
      break;
    case PROP_PROTOCOL:
      g_value_set_enum (value, priv->protocol);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_client_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsgClient        *client = (YtsgClient*) object;
  YtsgClientPrivate *priv   = client->priv;

  switch (property_id)
    {
    case PROP_UID:
      {
        g_free (priv->uid);
        priv->uid = g_value_dup_string (value);
      }
      break;
    case PROP_CAPABILITIES:
      ytsg_client_set_capabilities (client, g_value_get_uint (value));
      break;
    case PROP_PROTOCOL:
      priv->protocol = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_client_init (YtsgClient *client)
{
  client->priv = YTSG_CLIENT_GET_PRIVATE (client);

  ytsg_client_set_incoming_file_directory (client, NULL);
}

static void
ytsg_client_dispose (GObject *object)
{
  YtsgClient        *client = (YtsgClient*) object;
  YtsgClientPrivate *priv   = client->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  if (priv->roster)
    {
      g_object_unref (priv->roster);
      priv->roster = NULL;
    }

  if (priv->unwanted)
    {
      g_object_unref (priv->unwanted);
      priv->unwanted = NULL;
    }

  if (priv->connection)
    {
      tp_cli_connection_call_disconnect  (priv->connection,
                                          -1,
                                          NULL, NULL, NULL, NULL);
      g_object_unref (priv->connection);
      priv->connection = NULL;
    }

  if (priv->mgr)
    {
      tp_yts_account_manager_release (priv->mgr);

      g_object_unref (priv->mgr);
      priv->mgr = NULL;
    }

  if (priv->debug_proxy)
    {
      g_object_unref (priv->debug_proxy);
      priv->debug_proxy = NULL;
    }

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  G_OBJECT_CLASS (ytsg_client_parent_class)->dispose (object);
}

static void
ytsg_client_finalize (GObject *object)
{
  YtsgClient        *client = (YtsgClient*) object;
  YtsgClientPrivate *priv   = client->priv;

  g_free (priv->uid);
  g_free (priv->icon_token);
  g_free (priv->icon_mime_type);
  g_free (priv->mgr_name);
  g_free (priv->incoming_dir);

  if (priv->caps)
    g_array_free (priv->caps, TRUE);

  if (priv->icon_data)
    g_array_free (priv->icon_data, TRUE);

  G_OBJECT_CLASS (ytsg_client_parent_class)->finalize (object);
}

/**
 * ytsg_client_disconnect_from_mesh:
 * @client: #YtsgClient
 *
 * Disconnects #YtsgClient from Ytstenut mesh.
 */
void
ytsg_client_disconnect_from_mesh (YtsgClient *client)
{
  YtsgClientPrivate *priv;

  g_return_if_fail (YTSG_IS_CLIENT (client));

  priv = client->priv;

  /* cancel any pending reconnect timeout */
  if (priv->reconnect_id)
    {
      g_source_remove (priv->reconnect_id);
      priv->reconnect_id = 0;
    }

  /* clear flag indicating pending connect */
  priv->connect = FALSE;

  /*
   * Since this was a disconnect at our end, clear the reconnect flag,
   * to avoid the signal closure from installing a reconnect callback.
   */
  priv->reconnect = FALSE;

  if (priv->connection)
    tp_cli_connection_call_disconnect  (priv->connection,
                                        -1, NULL, NULL, NULL, NULL);
}

static gboolean
ytsg_client_reconnect_cb (gpointer data)
{
  YtsgClient *client = data;

  client->priv->reconnect_id = 0;

  ytsg_client_connect_to_mesh (client);

  /* one off */
  return FALSE;
}


/*
 * _ytsg_client_reconnect_after:
 * @client: #YtsgClient
 * @after_seconds: #guint
 *
 * Attempst to reconnect to server after given number of seconds.
 */
void
_ytsg_client_reconnect_after (YtsgClient *client, guint after_seconds)
{
  YtsgClientPrivate *priv;

  g_return_if_fail (YTSG_IS_CLIENT (client));

  priv = client->priv;

  priv->reconnect = TRUE;

  priv->reconnect_id =
    g_timeout_add_seconds (after_seconds, ytsg_client_reconnect_cb, client);
}

static void
ytsg_client_connected_cb (TpConnection *proxy,
                          const GError *error,
                          gpointer      data,
                          GObject      *weak_object)
{
  YtsgClient *client = data;

  if (error)
    {
      g_warning (G_STRLOC ": %s: %s", __FUNCTION__, error->message);

      ytsg_client_disconnect_from_mesh (client);
      _ytsg_client_reconnect_after (client, RECONNECT_DELAY);
    }
}

static void
ytsg_client_error_cb (TpConnection *proxy,
                      const gchar  *arg_Error,
                      GHashTable   *arg_Details,
                      gpointer      user_data,
                      GObject      *weak_object)
{
  YTSG_NOTE (CLIENT, "Error: %s", arg_Error);
}

static void
ytsg_client_status_cb (TpConnection *proxy,
                       guint         arg_Status,
                       guint         arg_Reason,
                       gpointer      data,
                       GObject      *weak_object)
{
  YtsgClient   *client = data;
  const char *status[] = {"'connected'   ",
                          "'connecting'  ",
                          "'disconnected'"};
  const char *reason[] =
    {
      "NONE_SPECIFIED",
      "REQUESTED",
      "NETWORK_ERROR",
      "AUTHENTICATION_FAILED",
      "ENCRYPTION_ERROR",
      "NAME_IN_USE",
      "CERT_NOT_PROVIDED",
      "CERT_UNTRUSTED",
      "CERT_EXPIRED",
      "CERT_NOT_ACTIVATED",
      "CERT_HOSTNAME_MISMATCH",
      "CERT_FINGERPRINT_MISMATCH",
      "CERT_SELF_SIGNED",
      "CERT_OTHER_ERROR"
    };

  if (client->priv->disposed)
    return;

  YTSG_NOTE (CLIENT, "Connection: %s: '%s'",
           status[arg_Status], reason[arg_Reason]);

  if (arg_Status == TP_CONNECTION_STATUS_CONNECTED)
    g_signal_emit (client, signals[AUTHENTICATED], 0);
  else if (arg_Status == TP_CONNECTION_STATUS_DISCONNECTED)
    g_signal_emit (client, signals[DISCONNECTED], 0);
}

static gboolean
yts_client_caps_overlap (GArray *mycaps, char **caps)
{
  int i;

  /* TODO -- this is not nice, maybe YtsgClient:caps should also be just a
   *         char**
   */
  for (i = 0; i < mycaps->len; ++i)
    {
      char **p;

      for (p = caps; *p; ++p)
        {
          if (!strcmp (g_quark_to_string (g_array_index (mycaps, YtsgCaps, i)),
                       *p))
            {
              return TRUE;
            }
        }
    }

  return FALSE;
}

static gboolean
ytsg_client_process_one_service (YtsgClient        *client,
                                 const char        *jid,
                                 const char        *sid,
                                 const GValueArray *service)
{
  YtsgClientPrivate *priv = client->priv;
  const char        *type;
  GHashTable        *names;
  char             **caps;
  YtsgRoster        *roster;
  GValueArray       *sinfo = (GValueArray*) service;

  if (sinfo->n_values == 3)
    {
      g_warning ("Missformed service description");
      return FALSE;
    }

  type  = g_value_get_string (g_value_array_get_nth (sinfo, 0));
  names = g_value_get_boxed (g_value_array_get_nth (sinfo, 1));
  caps  = g_value_get_boxed (g_value_array_get_nth (sinfo, 2));

  if (yts_client_caps_overlap (priv->caps, caps))
    roster = priv->roster;
  else
    roster = priv->unwanted;

  _ytsg_roster_add_service (roster, jid, sid, type,
                            (const char**)caps, names);

  return TRUE;
}

static void
ytsg_client_service_added_cb (TpYtsStatus       *self,
                              const gchar       *jid,
                              const gchar       *sid,
                              const GValueArray *sinfo,
                              YtsgClient        *client)
{
  ytsg_client_process_one_service (client, jid, sid, sinfo);
}

static void
ytsg_client_process_status (YtsgClient *client)
{
  YtsgClientPrivate *priv = client->priv;
  GHashTable        *services;

  if ((services = tp_yts_status_get_discovered_services (priv->tp_status)))
    {
      char           *jid;
      GHashTable     *service;
      GHashTableIter  iter;

      g_hash_table_iter_init (&iter, services);
      while (g_hash_table_iter_next (&iter, (void**)&jid, (void**)&service))
        {
          char           *sid;
          GValueArray    *sinfo;
          GHashTableIter  iter2;

          g_hash_table_iter_init (&iter2, service);
          while (g_hash_table_iter_next (&iter2, (void**)&sid, (void**)&sinfo))
            {
              ytsg_client_process_one_service (client, jid, sid, sinfo);
            }
        }
    }
}

static void
ytsg_client_yts_status_cb (GObject      *obj,
                           GAsyncResult *res,
                           gpointer      data)
{
  TpConnection      *conn   = TP_CONNECTION (obj);
  YtsgClient        *client = data;
  YtsgClientPrivate *priv   = client->priv;
  GError            *error  = NULL;
  TpYtsStatus       *status;

  if (!(status = tp_yts_status_ensure_for_connection_finish (conn, res,&error)))
    {
      g_error ("Failed to obtain status: %s", error->message);
    }

  priv->tp_status = status;

  tp_g_signal_connect_object (status, "service-added",
                              G_CALLBACK (ytsg_client_service_added_cb),
                              client, 0);

  ytsg_client_process_status (client);

  if (!priv->ready)
    g_signal_emit (client, signals[READY], 0);
}

static void
ytsg_client_connection_ready_cb (TpConnection *conn,
                                 GParamSpec   *par,
                                 YtsgClient   *client)
{
  GCancellable *cancellable;

  if (tp_connection_is_ready (conn))
    {
      YTSG_NOTE (CLIENT, "TP Connection entered ready state");

      cancellable = g_cancellable_new ();

      tp_yts_status_ensure_for_connection_async (conn,
                                                 cancellable,
                                                 ytsg_client_yts_status_cb,
                                                 client);

      /*
       * TODO -- this should be stored, so we can clean up in dispose any
       * pending op, ???
       *
       * But the TpYtsStatus docs say it's not used ...
       */
      g_object_unref (cancellable);
    }
}

static void
ytsg_client_set_avatar_cb (TpConnection *proxy,
                           const gchar  *token,
                           const GError *error,
                           gpointer      data,
                           GObject      *weak_object)
{
  YtsgClient        *client = data;
  YtsgClientPrivate *priv   = client->priv;

  if (error)
    {
      g_warning ("Failed to set avatar: %s", error->message);
      return;
    }

  g_free (priv->icon_token);
  priv->icon_token = g_strdup (token);
}

/*
 * check whether the given avatar mime type is supported.
 */
static gboolean
ytsg_client_is_avatar_type_supported (YtsgClient *client,
                                      const char *mime_type,
                                      guint       bytes)
{
  YtsgClientPrivate     *priv = client->priv;
  char                 **p;
  const char            *alt_type = NULL;
  TpAvatarRequirements  *req;

  req = tp_connection_get_avatar_requirements (priv->connection);

  if (!req || !req->supported_mime_types)
    {
      g_warning ("Icon functionality is not supported by backend");
      return FALSE;
    }

  if (bytes > req->maximum_bytes)
    {
      g_warning ("Icon can be at most %d bytes in size (requested %d)",
                 req->maximum_bytes, bytes);
      return FALSE;
    }

  if (!mime_type)
    {
      g_warning ("Icon mime type not specified, ignoring icon");
      return FALSE;
    }

  /*
   * This is really annoying, but TP internally uses both image/jpg and
   * image/jpeg (we get the former when querying existing avatars, and the
   * latter when quering avatar requirements), so we need to handle both.
   */
  if (!strcmp (mime_type, "image/jpg"))
    alt_type = "image/jpeg";
  else if (!strcmp (mime_type, "image/jpeg"))
    alt_type = "image/jpg";

  for (p = req->supported_mime_types; *p; ++p)
    {
      if (!strcmp (*p, mime_type) || (alt_type && !strcmp (*p, alt_type)))
        {
          return TRUE;
        }
    }

  g_warning ("Icon uses unsupported mime type %s", mime_type);

  return FALSE;
}

static void
ytsg_client_connection_prepare_cb (GObject      *connection,
                                   GAsyncResult *res,
                                   gpointer      data)
{
  YtsgClient        *client = data;
  YtsgClientPrivate *priv   = client->priv;
  GError            *error  = NULL;

  if (!tp_proxy_prepare_finish (connection, res, &error))
    {
      g_critical ("Failed to prepare info: %s", error->message);
    }
  else
    {
      if (priv->icon_data &&
          ytsg_client_is_avatar_type_supported (client,
                                              priv->icon_mime_type,
                                              priv->icon_data->len))
        {
          tp_cli_connection_interface_avatars_call_set_avatar (priv->connection,
                                                        -1,
                                                        priv->icon_data,
                                                        priv->icon_mime_type,
                                                        ytsg_client_set_avatar_cb,
                                                        client,
                                                        NULL,
                                                        (GObject*)client);

          g_array_free (priv->icon_data, TRUE);
          priv->icon_data = NULL;
          g_free (priv->icon_mime_type);
          priv->icon_mime_type = NULL;
        }

#if 0
      /* TODO -- */
      /*
       * local-xmpp / salut does not support the ContactCapabilities interface,
       * but file transfer is enabled by default, so it does not matter to us.
       */
      if (priv->protocol != YTSG_PROTOCOL_LOCAL_XMPP)
        ytsg_client_setup_caps (client);
#endif
    }
}

/*
 * Sets up required features on the connection, and connects callbacks to
 * signals that we care about.
 */
static void
ytsg_client_setup_account_connection (YtsgClient *client)
{
  YtsgClientPrivate *priv  = client->priv;
  GError            *error = NULL;
  GQuark             features[] = { TP_CONNECTION_FEATURE_CONTACT_INFO,
                                    TP_CONNECTION_FEATURE_AVATAR_REQUIREMENTS,
                                    TP_CONNECTION_FEATURE_CAPABILITIES,
                                    0 };

  priv->connection = tp_account_get_connection (priv->account);

  g_assert (priv->connection);

  priv->dialing = FALSE;

  YTSG_NOTE (CLIENT, "Connection ready ?: %d",
             tp_connection_is_ready (priv->connection));

  tp_g_signal_connect_object (priv->connection, "notify::connection-ready",
                              G_CALLBACK (ytsg_client_connection_ready_cb),
                              client, 0);

  tp_cli_connection_connect_to_connection_error (priv->connection,
                                                 ytsg_client_error_cb,
                                                 client,
                                                 NULL,
                                                 (GObject*)client,
                                                 &error);

  if (error)
    {
      g_critical (G_STRLOC ": %s: %s; no nScreen functionality will be "
                  "available", __FUNCTION__, error->message);
      g_clear_error (&error);
      return;
    }

  tp_cli_connection_connect_to_status_changed (priv->connection,
                                               ytsg_client_status_cb,
                                               client,
                                               NULL,
                                               (GObject*) client,
                                               &error);

  if (error)
    {
      g_critical (G_STRLOC ": %s: %s; no nScreen functionality will be "
                  "available", __FUNCTION__, error->message);
      g_clear_error (&error);
      return;
    }

  tp_cli_connection_connect_to_new_channel (priv->connection,
                                            ytsg_client_channel_cb,
                                            client,
                                            NULL,
                                            (GObject*)client,
                                            &error);

  if (error)
    {
      g_critical (G_STRLOC ": %s: %s; no nScreen functionality will be "
                  "available", __FUNCTION__, error->message);
      g_clear_error (&error);
      return;
    }

  tp_proxy_prepare_async (priv->connection,
                          features,
                          ytsg_client_connection_prepare_cb,
                          client);
}

/*
 * Callback for the async request to change presence ... not that we do
 * do anything with it, except when it fails.
 */
static void
ytsg_client_account_online_cb (GObject      *acc,
                               GAsyncResult *res,
                               gpointer      data)
{
  GError    *error   = NULL;
  TpAccount *account = (TpAccount*)acc;

  if (!tp_account_request_presence_finish (account, res, &error))
    {
      g_error ("Failed to change presence to online");
    }

  YTSG_NOTE (CLIENT, "Request to change presence to 'online' succeeded");
}

/*
 * One off handler for connection coming online
 */
static void
ytsg_client_account_connection_notify_cb (TpAccount  *account,
                                          GParamSpec *pspec,
                                          YtsgClient *client)
{
  YTSG_NOTE (CLIENT, "We got connection!");

  g_signal_handlers_disconnect_by_func (account,
                                   ytsg_client_account_connection_notify_cb,
                                   client);

  ytsg_client_setup_account_connection (client);
}

static void
ytsg_client_make_connection (YtsgClient *client)
{
  YtsgClientPrivate *priv  = client->priv;

  /*
   * If we don't have an account yet, we do nothing and will make call to this
   * function when the account is ready.
   */
  if (!priv->account)
    return;

  /*
   * At this point the account is prepared, but that does not mean we have a
   * connection (i.e., the current presence could 'off line' -- if we do not
   * have a connection, we request that the presence changes to 'on line' and
   * listen for when the :connection property changes.
   */
  if (!tp_account_get_connection (priv->account))
    {
      YTSG_NOTE (CLIENT, "Currently off line, changing ...");

      g_signal_connect (priv->account, "notify::connection",
                        G_CALLBACK (ytsg_client_account_connection_notify_cb),
                        client);

      tp_account_request_presence_async (priv->account,
                                         TP_CONNECTION_PRESENCE_TYPE_AVAILABLE,
                                         "online",
                                         "online",
                                         ytsg_client_account_online_cb,
                                         client);
    }
  else
    ytsg_client_setup_account_connection (client);
}

/**
 * ytsg_client_connect_to_mesh:
 * @client: #YtsgClient
 *
 * Initiates connection to the mesh. Once the connection is established,
 * the YtsgClient::authenticated signal will be emitted.
 *
 * NB: this function long name is to avoid collision with the GIR singnal
 *     connection method.
 */
void
ytsg_client_connect_to_mesh (YtsgClient *client)
{
  YtsgClientPrivate *priv;

  YTSG_NOTE (CLIENT, "Connecting ...");

  g_return_if_fail (YTSG_IS_CLIENT (client));

  priv = client->priv;

  /* cancel any pending reconnect timeout */
  if (priv->reconnect_id)
    {
      g_source_remove (priv->reconnect_id);
      priv->reconnect_id = 0;
    }

  priv->connect = TRUE;

  if (priv->connection)
    {
      /*
       * We already have the connection, so just connect.
       */
      tp_cli_connection_call_connect (priv->connection,
                                      -1,
                                      ytsg_client_connected_cb,
                                      client,
                                      NULL,
                                      (GObject*)client);
    }
  else if (!priv->dialing)
    ytsg_client_make_connection (client);
}

/**
 * ytsg_client_new:
 * @protocol: #YtsgProtocol
 * @uid: UID for this service; UIDs must follow the dbus convetion for unique
 * names.
 *
 * Creates a new #YtsgClient object connected to the provided roster
 *
 * Return value: #YtsgClient object.
 */
YtsgClient *
ytsg_client_new (YtsgProtocol protocol, const char *uid)
{
  if (!uid)
    g_error ("UID must be specified at construction time.");

  return g_object_new (YTSG_TYPE_CLIENT,
                       "protocol", protocol,
                       "uid",      uid,
                       NULL);
}

static gboolean
ytsg_client_has_capability (YtsgClient *client, YtsgCaps cap)
{
  YtsgClientPrivate *priv = client->priv;
  int                i;

  if (!priv->caps)
    return FALSE;

  for (i = 0; i < priv->caps->len; ++i)
    {
      YtsgCaps c = g_array_index (priv->caps, YtsgCaps, i);

      if (c == cap || c == YTSG_CAPS_CONTROL)
        return TRUE;
    }

  return FALSE;
}

static void
ytsg_client_refresh_roster (YtsgClient *client)
{
  YtsgClientPrivate *priv = client->priv;

  if (!priv->tp_status)
    return;

  _ytsg_roster_clear (priv->roster);
  _ytsg_roster_clear (priv->unwanted);

  ytsg_client_process_status (client);
}

/**
 * ytsg_client_set_capabilities:
 * @client: #YtsgClient,
 * @caps: #YtsgCaps
 *
 * Adds a capability to the capability set of this client; multiple capabilities
 * can be added by making mulitiple calls to this function.
 *
 * The capability set is used to filter roster items to match.
 */
void
ytsg_client_set_capabilities (YtsgClient *client, YtsgCaps caps)
{
  YtsgClientPrivate *priv;

  g_return_if_fail (YTSG_IS_CLIENT (client));

  priv = client->priv;

  if (ytsg_client_has_capability (client, caps))
    return;

  if (!priv->caps)
    priv->caps = g_array_sized_new (FALSE, FALSE, sizeof (YtsgCaps), 1);

  g_array_append_val (priv->caps, caps);

  ytsg_client_refresh_roster (client);

  g_object_notify ((GObject*)client, "capabilities");
}

YtsgRoster *
ytsg_client_get_roster (YtsgClient *client)
{
  YtsgClientPrivate *priv;

  g_return_val_if_fail (YTSG_IS_CLIENT (client), NULL);

  priv = client->priv;

  return priv->roster;
}

/**
 * ytsg_client_emit_error:
 * @client: #YtsgClient,
 * @error: #YtsgError
 *
 * Emits the #YtsgClient::error signal with the suplied error parameter.
 *
 * This function is intened primarily for internal use, but can be also used by
 * toolkit libraries that need to generate asynchronous errors. Any function
 * call that returns the %YTSG_ERROR_PENDING code to the caller should
 * eventually lead to emission of the ::error signal with either an appropriate
 * error code or %YTSG_ERROR_SUCCESS to indicate the operation successfully
 * completed.
 */
void
ytsg_client_emit_error (YtsgClient *client, YtsgError error)
{
  g_return_if_fail (YTSG_IS_CLIENT (client));

  /*
   * There is no point in throwing an error that has no atom specified.
   */
  g_return_if_fail (ytsg_error_get_atom (error));

  g_signal_emit (client, signals[ERROR], 0, error);
}

/**
 * ytsg_client_set_incoming_file_directory:
 * @client: #YtsgClient
 * @directory: path to a directory or %NULL.
 *
 * Sets the directory where incoming files will be stored; if the provided path
 * is %NULL, the directory will be reset to the default (~/.nscreen/). This
 * function does not do any checks regarding validity of the path provided,
 * though an attempt to create the directory before it is used, with permissions
 * of 0700.
 *
 * To change the directory for a specific file call this function from a
 * callback to the YtsgClient::incoming-file signal.
 */
void
ytsg_client_set_incoming_file_directory (YtsgClient *client,
                                         const char *directory)
{
  YtsgClientPrivate *priv;

  g_return_if_fail (YTSG_IS_CLIENT (client));

  priv = client->priv;

  if (!directory || !*directory)
    priv->incoming_dir =
      g_build_filename (g_get_home_dir (), ".ytstenut", NULL);
  else
    priv->incoming_dir = g_strdup (directory);
}

/**
 * ytsg_client_get_incoming_file_directory:
 * @client: #YtsgClient
 *
 * Return value: (tranfer none): directory where incoming files are stored.
 */
const char *
ytsg_client_get_incoming_file_directory (YtsgClient *client)
{
  YtsgClientPrivate *priv;

  g_return_val_if_fail (YTSG_IS_CLIENT (client), NULL);

  priv = client->priv;

  return priv->incoming_dir;
}

const char *
ytsg_client_get_jid (const YtsgClient *client)
{
  YtsgClientPrivate *priv;

  g_return_val_if_fail (YTSG_IS_CLIENT (client), NULL);

  priv = client->priv;

  g_warning ("NOT IMPLEMENTED !!!");

  return NULL;
}

const char *
ytsg_client_get_uid (const YtsgClient *client)
{
  YtsgClientPrivate *priv;

  g_return_val_if_fail (YTSG_IS_CLIENT (client), NULL);

  priv = client->priv;

  return priv->uid;
}

TpConnection *
_ytsg_client_get_connection (YtsgClient *client)
{
  YtsgClientPrivate *priv;

  g_return_val_if_fail (YTSG_IS_CLIENT (client), NULL);

  priv = client->priv;

  return priv->connection;
}

TpYtsStatus *
_ytsg_client_get_tp_status (YtsgClient *client)
{
  YtsgClientPrivate *priv;

  g_return_val_if_fail (YTSG_IS_CLIENT (client), NULL);

  priv = client->priv;

  return priv->tp_status;
}

void
ytsg_client_set_status (YtsgClient *client, YtsgStatus *status)
{
  YtsgClientPrivate *priv;

  g_return_if_fail (YTSG_IS_CLIENT (client) &&
                    (!status || YTSG_IS_STATUS (status)));

  priv = client->priv;

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  if (status)
    {
      char *xml;
      int   i;

      priv->status = status;

      xml = ytsg_metadata_to_string ((YtsgMetadata*)priv->status);

      for (i = 0; i < priv->caps->len; ++i)
        {
          const char *c;

          c = g_quark_to_string (g_array_index (priv->caps, YtsgCaps, i));

          tp_yts_status_advertise_status_async (priv->tp_status,
                                                c,
                                                priv->uid,
                                                xml,
                                                NULL,
                                                NULL,
                                                NULL);
        }
    }
}

