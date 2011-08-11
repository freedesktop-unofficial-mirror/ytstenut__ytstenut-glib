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

/**
 * SECTION:ytsg-client
 * @short_description: Represents a connection to the Ytstenut mesh.
 *
 * #YtsgClient is an object that mediates connection between the current
 * application and the Ytstenut application mesh. It provides access to roster
 * of availalble services (#YtsgRoster) and means to advertises status within
 * the mesh.
 */

#include "ytsg-client.h"
#include "ytsg-debug.h"
#include "ytsg-enum-types.h"
#include "ytsg-error.h"
#include "ytsg-marshal.h"
#include "ytsg-metadata.h"
#include "ytsg-private.h"
#include "ytsg-roster.h"
#include "ytsg-service-adapter.h"
#include "ytsg-types.h"

#include "empathy-tp-file.h"

#include <string.h>
#include <rest/rest-xml-parser.h>
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
  TpYtsClient          *tp_client;

  /* Implemented services */
  GHashTable  *services;

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

  YTSG_NOTE (CONNECTION, "Authenticated");
}

static void
ytsg_client_ready (YtsgClient *client)
{
  YtsgClientPrivate *priv = client->priv;

  priv->ready = TRUE;

  YTSG_NOTE (CLIENT, "YtsgClient is ready");

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

  /**
   * YtsgClient:protocol:
   *
   * XMPP protocol to use for connection.
   *
   * Since: 0.1
   */
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
      YTSG_NOTE (MANAGER, "%s", error->message);
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

static gboolean
deliver_to_service (YtsgClient  *self,
                    char const  *xml)
{
  YtsgClientPrivate *priv = self->priv;
  RestXmlParser *parser;
  RestXmlNode   *node;
  gboolean       delivered = FALSE;

  parser = rest_xml_parser_new ();

  node = rest_xml_parser_parse_from_data (parser, xml, -1);
  if (node)
    {
      char const *type = rest_xml_node_get_attr (node, "type");
      if (0 == g_strcmp0 ("invocation", type))
        {
          char const *capability = rest_xml_node_get_attr (node, "capability");
          YtsgServiceAdapter *adapter = g_hash_table_lookup (priv->services,
                                                             capability);
          if (adapter)
            {
              char const *invocation_id = rest_xml_node_get_attr (node, "invocation");
              char const *aspect = rest_xml_node_get_attr (node, "aspect");
              char const *args = rest_xml_node_get_attr (node, "arguments");
              GVariant *arguments = NULL;
              
              if (args)
                arguments = g_variant_new_parsed (args);
              
              ytsg_service_adapter_invoke (adapter,
                                           invocation_id,
                                           aspect,
                                           arguments);
              delivered = TRUE;
            }
        }
    }

  g_object_unref (parser);
  return delivered;
}

static void
ytsg_client_yts_channels_received_cb (TpYtsClient *tp_client,
                                      YtsgClient  *client)
{
  TpYtsChannel *ch;

  while ((ch = tp_yts_client_accept_channel (tp_client)))
    {
      GHashTable     *props;
      GHashTableIter  iter;
      gpointer        key, value;

      g_object_get (ch, "channel-properties", &props, NULL);
      g_assert (props);

      g_hash_table_iter_init (&iter, props);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          GValue      *v = value;
          char        *k = key;

          if (!strcmp (k, "com.meego.xpmn.ytstenut.Channel.RequestBody"))
            {
              const char *xml = g_value_get_string (v);
              gboolean delivered = deliver_to_service (client, xml);

              if (!delivered)
                {
                  YtsgMetadata *msg = _ytsg_metadata_new_from_xml (xml);
                  g_signal_emit (client, signals[MESSAGE], 0, msg);
                  g_object_unref (msg);
                }
            }
        }
    }
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

  YTSG_NOTE (CONNECTION, "Account successfully opened");

  priv->tp_client = tp_yts_client_new (priv->uid, account);

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

  YTSG_NOTE (CONNECTION, "Got account");

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

  client->priv->services = g_hash_table_new (g_str_hash, g_str_equal);
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

  if (priv->services)
    {
      g_hash_table_destroy (priv->services);
      priv->services = NULL;
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

  YTSG_NOTE (CONNECTION, "Connection: %s: '%s'",
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

  if (sinfo->n_values != 3)
    {
      g_warning ("Missformed service description (nvalues == %d)",
                 sinfo->n_values);
      return FALSE;
    }

  YTSG_NOTE (CLIENT, "Processing service %s:%s", jid, sid);

  type  = g_value_get_string (g_value_array_get_nth (sinfo, 0));
  names = g_value_get_boxed (g_value_array_get_nth (sinfo, 1));
  caps  = g_value_get_boxed (g_value_array_get_nth (sinfo, 2));

  if (!priv->caps || !caps || !*caps ||
      yts_client_caps_overlap (priv->caps, caps))
    roster = priv->roster;
  else
    roster = priv->unwanted;

  YTSG_NOTE (CLIENT, "Using roster %s",
             roster == priv->roster ? "wanted" : "unwanted");

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

      if (g_hash_table_size (services) <= 0)
        YTSG_NOTE (CLIENT, "No services discovered so far");

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
  else
    YTSG_NOTE (CLIENT, "No discovered services");
}

static void
ytsg_client_advertise_status_cb (GObject      *source_object,
                                 GAsyncResult *result,
                                 gpointer      data)
{
  TpYtsStatus *status = TP_YTS_STATUS (source_object);
  GError      *error = NULL;

  if (!tp_yts_status_advertise_status_finish (status, result, &error))
    {
      g_critical ("Failed to advertise status: %s", error->message);
    }
  else
    {
      YTSG_NOTE (CLIENT, "Advertising of status succeeded");
    }

  g_clear_error (&error);
}

static void
ytsg_client_dispatch_status (YtsgClient *client)
{
  YtsgClientPrivate *priv;
  char *xml = NULL;
  int   i;

  g_return_if_fail (YTSG_IS_CLIENT (client));

  priv = client->priv;

  g_return_if_fail (priv->caps && priv->caps->len);

  if (priv->status)
    xml = ytsg_metadata_to_string ((YtsgMetadata*)priv->status);

  YTSG_NOTE (CLIENT, "Setting status to\n%s", xml);

  for (i = 0; i < priv->caps->len; ++i)
    {
      const char *c;

      c = g_quark_to_string (g_array_index (priv->caps, YtsgCaps, i));

      tp_yts_status_advertise_status_async (priv->tp_status,
                                            c,
                                            priv->uid,
                                            xml,
                                            NULL,
                                            ytsg_client_advertise_status_cb,
                                            client);
    }

  g_free (xml);
}

static void
ytsg_client_yts_status_cb (GObject      *obj,
                           GAsyncResult *res,
                           gpointer      data)
{
  TpAccount         *acc    = TP_ACCOUNT (obj);
  YtsgClient        *client = data;
  YtsgClientPrivate *priv   = client->priv;
  GError            *error  = NULL;
  TpYtsStatus       *status;

  if (!(status = tp_yts_status_ensure_finish (acc, res,&error)))
    {
      g_error ("Failed to obtain status: %s", error->message);
    }

  YTSG_NOTE (CLIENT, "Processing status");

  priv->tp_status = status;

  tp_g_signal_connect_object (status, "service-added",
                              G_CALLBACK (ytsg_client_service_added_cb),
                              client, 0);

  if (priv->status)
    ytsg_client_dispatch_status (client);

  ytsg_client_process_status (client);

  if (!priv->ready)
    {
      YTSG_NOTE (CLIENT, "Emitting 'ready' signal");
      g_signal_emit (client, signals[READY], 0);
    }
}

static void
ytsg_client_connection_ready_cb (TpConnection *conn,
                                 GParamSpec   *par,
                                 YtsgClient   *client)
{
  GCancellable      *cancellable;
  YtsgClientPrivate *priv = client->priv;

  if (tp_connection_is_ready (conn))
    {
      YTSG_NOTE (CONNECTION, "TP Connection entered ready state");

      cancellable = g_cancellable_new ();

      tp_yts_status_ensure_async (priv->account,
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
      if (!tp_yts_client_register (priv->tp_client, &error))
        {
          g_error ("Failed to register account: %s", error->message);
        }
      else
        YTSG_NOTE (CONNECTION, "Registered TpYtsClient");

      tp_g_signal_connect_object (priv->tp_client, "received-channels",
                              G_CALLBACK (ytsg_client_yts_channels_received_cb),
                              client, 0);

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
                                    TP_CONNECTION_FEATURE_CONNECTED,
                                    0 };

  priv->connection = tp_account_get_connection (priv->account);

  g_assert (priv->connection);

  priv->dialing = FALSE;

  YTSG_NOTE (CONNECTION, "Connection ready ?: %d",
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
  char      *stat;
  char      *msg;
  TpConnectionPresenceType presence;

  if (!tp_account_request_presence_finish (account, res, &error))
    {
      g_error ("Failed to change presence to online");
    }

  presence = tp_account_get_current_presence (account, &stat, &msg);

  YTSG_NOTE (CONNECTION,
             "Request to change presence to 'online' succeeded: %d, %s:%s",
             presence, stat, msg);

  g_free (stat);
  g_free (msg);
}

/*
 * One off handler for connection coming online
 */
static void
ytsg_client_account_connection_notify_cb (TpAccount  *account,
                                          GParamSpec *pspec,
                                          YtsgClient *client)
{
  YTSG_NOTE (CONNECTION, "We got connection!");

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
    {
      YTSG_NOTE (CONNECTION, "Account not yet available");
      return;
    }

  /*
   * At this point the account is prepared, but that does not mean we have a
   * connection (i.e., the current presence could 'off line' -- if we do not
   * have a connection, we request that the presence changes to 'on line' and
   * listen for when the :connection property changes.
   */
  if (!tp_account_get_connection (priv->account))
    {
      YTSG_NOTE (CONNECTION, "Currently off line, changing ...");

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

  YTSG_NOTE (CONNECTION, "Connecting ...");

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

      /* FIXME Huh why would say having control caps imply video or whatever? -Rob */
      if (c == cap || c == YTSG_CAPS_CONTROL)
        return TRUE;
    }

  return FALSE;
}

static void
ytsg_client_refresh_roster (YtsgClient *client)
{
  YtsgClientPrivate *priv = client->priv;

  YTSG_NOTE (CLIENT, "Refreshing roster");

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
    {
      YTSG_NOTE (CLIENT, "Capablity '%s' already set",
                 g_quark_to_string (caps));

      return;
    }

  if (!priv->caps)
    priv->caps = g_array_sized_new (FALSE, FALSE, sizeof (YtsgCaps), 1);

  g_array_append_val (priv->caps, caps);

  ytsg_client_refresh_roster (client);

  g_object_notify ((GObject*)client, "capabilities");
}

/**
 * ytsg_client_get_roster:
 * @client: #YtsgClient
 *
 * Gets the #YtsgRoster for this client. The object is owned by the client
 * and must not be freed by the caller.
 *
 * Return value (tranfer none): #YtsgRoster.
 */
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
 * Returns the directory into which any files from incoming file transfers will
 * be placed.
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

/**
 * ytsg_client_get_jid:
 * @client: #YtsgClient
 *
 * Returns the jabber id associated with the current client.
 *
 * Return value: the jabber id.
 */
const char *
ytsg_client_get_jid (const YtsgClient *client)
{
/*
  YtsgClientPrivate *priv;

  g_return_val_if_fail (YTSG_IS_CLIENT (client), NULL);

  priv = client->priv;
*/
  g_warning (G_STRLOC ": NOT IMPLEMENTED !!!");

  return NULL;
}

/**
 * ytsg_client_get_uid:
 * @client: #YtsgClient
 *
 * Returns uid of the service this client represents.
 *
 * Return value: the service uid.
 */
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

/*
 * FIXME -- bad API, constructing the YtsgStatus is hard, this should be a
 * private API, with a better public API wrapping it.
 */

/**
 * ytsg_client_set_status_by_capability:
 * @client: #YtsgClient
 * @capability: the capability to set status for
 * @activity: the activity to set the status to.
 *
 * Changes the status of the service represented by this client to status.
 */
void
ytsg_client_set_status_by_capability (YtsgClient *client,
                                      const char *capability,
                                      const char *activity)
{
  YtsgClientPrivate *priv;
  YtsgStatus        *status = NULL;

  g_return_if_fail (YTSG_IS_CLIENT (client) && capability);

  priv = client->priv;

  g_return_if_fail (priv->caps && priv->caps->len);

  if (activity)
    {
      const char   *attributes[] =
        {
          "capability",   capability,
          "activity",     activity,
          "from-service", priv->uid,
          NULL
        };

      g_debug ("Constructing status for %s, %s, %s",
               capability, activity, priv->uid);

      status = ytsg_status_new ((const char**)&attributes);
    }

  ytsg_client_set_status (client, status);
}

/**
 * ytsg_client_set_status:
 * @client: #YtsgClient
 * @status: new #YtsgStatus
 *
 * Changes the status of the service represented by this client to status;
 */
void
ytsg_client_set_status (YtsgClient *client, YtsgStatus *status)
{
  YtsgClientPrivate *priv;

  g_return_if_fail (YTSG_IS_CLIENT (client) && YTSG_IS_STATUS (status));

  priv = client->priv;

  g_return_if_fail (priv->caps && priv->caps->len);

  if (status)
    g_object_ref (status);

  if (priv->status)
    {
      g_object_unref (priv->status);
      priv->status = NULL;
    }

  priv->status = status;

  if (priv->tp_status)
    {
      ytsg_client_dispatch_status (client);
    }
}

struct YtsgCLChannelData
{
  YtsgClient  *client;
  YtsgContact *contact;
  GHashTable  *attrs;
  char        *xml;
  char        *uid;
  YtsgError    error;
  gboolean     status_done;
  int          ref_count;
};

static void
ytsg_cl_channel_data_unref (struct YtsgCLChannelData *d)
{
  d->ref_count--;

  if (d->ref_count <= 0)
    {
      g_hash_table_unref (d->attrs);
      g_free (d->xml);
      g_free (d->uid);
      g_free (d);
    }
}

static struct YtsgCLChannelData *
ytsg_cl_channel_data_ref (struct YtsgCLChannelData *d)
{
  d->ref_count++;
  return d;
}

static void
ytsg_client_msg_replied_cb (TpYtsChannel *proxy,
                            GHashTable   *attributes,
                            const gchar  *body,
                            gpointer      data,
                            GObject      *weak_object)
{
  GHashTableIter            iter;
  gpointer                  key, value;
  struct YtsgCLChannelData *d = data;

  YTSG_NOTE (MESSAGE, "Got reply with attributes:");

  g_hash_table_iter_init (&iter, attributes);

  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      YTSG_NOTE (MESSAGE, "    %s = %s\n",
                 (const gchar *) key, (const gchar *) value);
    }

  YTSG_NOTE (MESSAGE, "    body: %s\n", body);

  if (!d->status_done)
    {
      guint32   a;
      YtsgError e;

      a = ytsg_error_get_atom (d->error);
      e = ytsg_error_make (a, YTSG_ERROR_SUCCESS);

      ytsg_client_emit_error (d->client, e);

      d->status_done = TRUE;
    }

  ytsg_cl_channel_data_unref (d);
}

static void
ytsg_client_msg_failed_cb (TpYtsChannel *proxy,
                           guint         error_type,
                           const gchar  *stanza_error_name,
                           const gchar  *ytstenut_error_name,
                           const gchar  *text,
                           gpointer      data,
                           GObject      *weak_object)
{
  guint32                   a;
  YtsgError                 e;
  struct YtsgCLChannelData *d = data;

  a = ytsg_error_get_atom (d->error);

  g_warning ("Sending of message failed: type %u, %s, %s, %s",
             error_type, stanza_error_name, ytstenut_error_name, text);

  e = ytsg_error_make (a, YTSG_ERROR_NO_MSG_CHANNEL);

  ytsg_client_emit_error (d->client, e);

  d->status_done = TRUE;

  ytsg_cl_channel_data_unref (d);
}

static void
ytsg_client_msg_closed_cb (TpChannel *channel,
                           gpointer   data,
                           GObject   *weak_object)
{
  struct YtsgCLChannelData *d = data;

  YTSG_NOTE (MESSAGE, "Channel closed");

  if (!d->status_done)
    {
      guint32   a;
      YtsgError e;

      a = ytsg_error_get_atom (d->error);
      e = ytsg_error_make (a, YTSG_ERROR_SUCCESS);

      ytsg_client_emit_error (d->client, e);

      d->status_done = TRUE;
    }

  ytsg_cl_channel_data_unref (d);
}

static void
ytsg_client_msg_request_cb (GObject      *source_object,
                            GAsyncResult *result,
                            gpointer      data)
{
  GError *error = NULL;

  if (!tp_yts_channel_request_finish (
          TP_YTS_CHANNEL (source_object), result, &error))
    {
      g_warning ("Failed to Request on channel: %s\n", error->message);
    }
  else
    {
      YTSG_NOTE (MESSAGE, "Channel requested");
    }

  g_clear_error (&error);
}

static void
ytsg_client_outgoing_channel_cb (GObject      *obj,
                                 GAsyncResult *res,
                                 gpointer      data)
{
  TpYtsChannel             *ch;
  TpYtsClient              *client = TP_YTS_CLIENT (obj);
  GError                   *error  = NULL;
  struct YtsgCLChannelData *d      = data;

  if (!(ch = tp_yts_client_request_channel_finish (client, res, &error)))
    {
      guint32   a;
      YtsgError e;

      a = ytsg_error_get_atom (d->error);

      g_warning ("Failed to open outgoing channel: %s", error->message);
      g_clear_error (&error);

      e = ytsg_error_make (a, YTSG_ERROR_NO_MSG_CHANNEL);

      ytsg_client_emit_error (d->client, e);
    }
  else
    {
      YTSG_NOTE (MESSAGE, "Got message channel, sending request");

      tp_yts_channel_connect_to_replied (ch, ytsg_client_msg_replied_cb,
                                         ytsg_cl_channel_data_ref (d),
                                         NULL, NULL, NULL);
      tp_yts_channel_connect_to_failed (ch, ytsg_client_msg_failed_cb,
                                        ytsg_cl_channel_data_ref (d),
                                        NULL, NULL, NULL);
      tp_cli_channel_connect_to_closed (TP_CHANNEL (ch),
                                        ytsg_client_msg_closed_cb,
                                        ytsg_cl_channel_data_ref (d),
                                        NULL, NULL, NULL);

      tp_yts_channel_request_async (ch, NULL, ytsg_client_msg_request_cb, NULL);
    }

  ytsg_cl_channel_data_unref (d);
}

static YtsgError
ytsg_client_dispatch_message (struct YtsgCLChannelData *d)
{
  TpContact         *tp_contact;
  YtsgClientPrivate *priv = d->client->priv;

  YTSG_NOTE (CLIENT, "Dispatching delayed message to %s", d->uid);

  tp_contact = ytsg_contact_get_tp_contact (d->contact);
  g_assert (tp_contact);

  tp_yts_client_request_channel_async (priv->tp_client,
                                       tp_contact,
                                       d->uid,
                                       TP_YTS_REQUEST_TYPE_GET,
                                       d->attrs,
                                       d->xml,
                                       NULL,
                                       ytsg_client_outgoing_channel_cb,
                                       d);

  return d->error;
}

static void
ytsg_client_notify_tp_contact_cb (YtsgContact              *contact,
                                  GParamSpec               *pspec,
                                  struct YtsgCLChannelData *d)
{
  YTSG_NOTE (CLIENT, "Contact ready");
  ytsg_client_dispatch_message (d);
  g_signal_handlers_disconnect_by_func (contact,
                                        ytsg_client_notify_tp_contact_cb,
                                        d);
}

YtsgError
_ytsg_client_send_message (YtsgClient  *client,
                           YtsgContact *contact,
                           const char  *uid,
                           YtsgMessage *message)
{
  GHashTable               *attrs;
  struct YtsgCLChannelData *d;
  YtsgError                 e;
  char                     *xml = NULL;

  if (!(attrs = _ytsg_metadata_extract ((YtsgMetadata*)message, &xml)))
    {
      g_warning ("Failed to extract content from YtsgMessage object");

      e = ytsg_error_new (YTSG_ERROR_INVALID_PARAMETER);
      g_free (xml);
      return e;
    }

  e = ytsg_error_new (YTSG_ERROR_PENDING);

  d              = g_new (struct YtsgCLChannelData, 1);
  d->error       = e;
  d->client      = client;
  d->contact     = contact;
  d->status_done = FALSE;
  d->ref_count   = 1;
  d->attrs       = attrs;
  d->xml         = xml;
  d->uid         = g_strdup (uid);

  if (ytsg_contact_get_tp_contact (contact))
    {
      ytsg_client_dispatch_message (d);
    }
  else
    {
      YTSG_NOTE (CLIENT, "Contact not ready, postponing message dispatch");

      g_signal_connect (contact, "notify::tp-contact",
                        G_CALLBACK (ytsg_client_notify_tp_contact_cb),
                        d);
    }

  return e;
}

/* FIXME this should probably go into some sort of factory.
 * A bit hacky for now, so we don't need to include video-service headers here. */

extern GType
ytsg_vs_content_adapter_get_type (void);

extern GType
ytsg_vs_player_adapter_get_type (void);

extern GType
ytsg_vs_query_get_adapter_type (void);

extern GType
ytsg_vs_transfer_get_adapter_type (void);

static YtsgServiceAdapter *
create_adapter_for_service (YtsgClient  *self,
                            GObject     *service)
{
  GType interface_type;

  interface_type = g_type_from_name ("YtsgVSPlayer");
  if (interface_type &&
      g_type_is_a (G_OBJECT_TYPE (service), interface_type)) {

    return g_object_new (ytsg_vs_player_adapter_get_type (),
                         "service", service,
                         "service-gtype", interface_type,
                         NULL);
  }

  g_critical ("%s : Failed to find built-in adapter class for %s",
              G_STRLOC,
              G_OBJECT_TYPE_NAME (service));

  return NULL;
}

static void
_adapter_destroyed (YtsgClient  *self,
                    void        *stale_adapter_ptr)
{
  YtsgClientPrivate *priv = self->priv;
  GHashTableIter     iter;
  char const        *capability;
  gpointer           value;

  g_hash_table_iter_init (&iter, priv->services);
  while (g_hash_table_iter_next (&iter, (gpointer *) &capability, &value))
    {
      if (value == stale_adapter_ptr)
        {
          YTSG_NOTE (CLIENT, "unregistering capability %s", capability);
          g_hash_table_remove (priv->services, capability);
          // FIXME also no longer advertise this capability
        }
    }
}

/*
 * TODO add GError reporting
 * The client does not take ownership of the service, it will be
 * unregistered upon destruction.
 */
gboolean
ytsg_client_register_service (YtsgClient  *self,
                              GObject     *service)
{
  YtsgClientPrivate   *priv = self->priv;
  YtsgServiceAdapter  *adapter;
  GParamSpec          *pspec;
  char const          *capability;

  g_return_val_if_fail (YTSG_IS_CLIENT (self), FALSE);

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (service),
                                        "capability");
  if (NULL == pspec ||
      !G_IS_PARAM_SPEC_STRING (pspec))
    {
      g_critical ("%s : Service of type %s does not implement 'capability' "
                  "property of type string",
                  G_STRLOC,
                  G_OBJECT_TYPE_NAME (service));
      return FALSE;
    }

  capability = G_PARAM_SPEC_STRING (pspec)->default_value;
  adapter = g_hash_table_lookup (priv->services, capability);
  if (adapter)
    {
      g_critical ("%s : Service for capability %s already registered",
                  G_STRLOC,
                  capability);
      return FALSE;
    }

  adapter = create_adapter_for_service (self, service);
  g_return_val_if_fail (adapter, FALSE);

  g_object_weak_ref (G_OBJECT (adapter),
                     (GWeakNotify) _adapter_destroyed,
                     self);

  g_hash_table_insert (priv->services, (char *) capability, adapter);
  ytsg_client_set_capabilities (self, g_quark_from_static_string (capability));

  return TRUE;
}

