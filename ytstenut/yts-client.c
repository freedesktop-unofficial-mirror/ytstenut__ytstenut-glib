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
 *              Rob Staudinger <robsta@linux.intel.com>
 */

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

#include "empathy-tp-file.h"
#include "yts-adapter-factory.h"
#include "yts-caps.h"
#include "yts-client-internal.h"
#include "yts-contact-internal.h"
#include "yts-debug.h"
#include "yts-enum-types.h"
#include "yts-error.h"
#include "yts-error-message.h"
#include "yts-event-message.h"
#include "yts-invocation-message.h"
#include "yts-marshal.h"
#include "yts-metadata-internal.h"
#include "yts-response-message.h"
#include "yts-roster-internal.h"
#include "yts-service.h"
#include "yts-service-adapter.h"
#include "yts-status.h"
#include "yts-types.h"

#include "profile/yts-profile.h"
#include "profile/yts-profile-adapter.h"
#include "profile/yts-profile-impl.h"

#include "config.h"

#define RECONNECT_DELAY 20 /* in seconds */

static void yts_client_make_connection (YtsClient *client);

G_DEFINE_TYPE (YtsClient, yts_client, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_CLIENT, YtsClientPrivate))

/**
 * SECTION: yts-client
 * @title: YtsClient
 * @short_description: Represents a connection to the Ytstenut mesh.
 *
 * #YtsClient is an object that mediates connection between the current
 * application and the Ytstenut application mesh. It provides access to roster
 * of availalble services (#YtsRoster) and means to advertises status within
 * the mesh.
 */

typedef struct {
  YtsRoster   *roster;    /* the roster of this client */
  YtsRoster   *unwanted;  /* roster of unwanted items */
  GArray       *caps;

  /* connection parameters */
  char         *uid;
  char         *mgr_name;
  YtsProtocol  protocol;

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
  YtsStatus           *status;
  TpYtsClient          *tp_client;

  /* Implemented services */
  GHashTable  *services;

  /* Ongoing invocations */
  GHashTable  *invocations;

  /* Registered proxies */
  GHashTable *proxies;

  /* callback ids */
  guint reconnect_id;

  bool authenticated;   /* are we authenticated ? */
  bool ready;           /* is TP setup done ? */
  bool connect;         /* connect once we get our connection ? */
  bool reconnect;       /* should we attempt to reconnect ? */
  bool dialing;         /* are we currently acquiring connection ? */
  bool members_pending; /* requery members when TP set up completed ? */
  bool prepared;        /* are connection features set up ? */
  bool disposed;        /* dispose guard */

} YtsClientPrivate;

enum
{
  AUTHENTICATED,
  READY,
  DISCONNECTED,
  RAW_MESSAGE,
  TEXT_MESSAGE,
  LIST_MESSAGE,
  DICTIONARY_MESSAGE,
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
  PROP_ICON_TOKEN,
};

static guint signals[N_SIGNALS] = {0};

/*
 * ServiceData
 */

typedef struct {
  YtsClient  *client;
  char        *capability;
} ServiceData;

static ServiceData *
service_data_create (YtsClient *client,
                     char const *capability)
{
  ServiceData *self;

  g_return_val_if_fail (YTS_IS_CLIENT (client), NULL);
  g_return_val_if_fail (capability, NULL);

  self = g_new0 (ServiceData, 1);
  self->client = g_object_ref (client);
  self->capability = g_strdup (capability);

  return self;
}

static void
service_data_destroy (ServiceData *self)
{
  g_return_if_fail (self);

  g_object_unref (self->client);
  g_free (self->capability);
  g_free (self);
}

/*
 * InvocationData
 */

/* PONDERING this should probably be configurable. */
#define INVOCATION_RESPONSE_TIMEOUT_S 20

typedef struct {
  YtsClient    *client;            /* free pointer, no ref */
  YtsContact   *contact;           /* free pointer, no ref */
  char          *proxy_id;
  char          *invocation_id;
  unsigned int   timeout_s;
  unsigned int   timeout_id;
} InvocationData;

static void
invocation_data_destroy (InvocationData *self)
{
  g_return_if_fail (self);

  if (self->timeout_id) {
    g_source_remove (self->timeout_id);
    self->timeout_id = 0;
  }

  g_free (self->proxy_id);
  g_free (self->invocation_id);
  g_free (self);
}

static bool
client_conclude_invocation (YtsClient  *self,
                            char const  *invocation_id)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  bool found;

  found = g_hash_table_remove (priv->invocations, invocation_id);
  if (!found) {
    g_warning ("%s : Pending invocation for ID %s not found",
               G_STRLOC,
               invocation_id);
    return false;
  }

  return true;
}

static bool
_invocation_timeout (InvocationData *self)
{
  g_critical ("%s : Invocation %s timed out after %i seconds",
              G_STRLOC,
              self->invocation_id,
              self->timeout_s);

  /* This destroys self */
  client_conclude_invocation (self->client, self->invocation_id);

  // TODO emit timeout / error

  /* Remove timeout */
  return false;
}

static InvocationData *
invocation_data_create (YtsClient    *client,
                        YtsContact   *contact,
                        char const    *proxy_id,
                        char const    *invocation_id,
                        unsigned int   timeout_s)
{
  InvocationData *self;

  self = g_new0 (InvocationData, 1);
  self->client = client;
  self->contact = contact;
  self->proxy_id = g_strdup (proxy_id);
  self->invocation_id = g_strdup (invocation_id);
  self->timeout_s = timeout_s;
  self->timeout_id = g_timeout_add_seconds (timeout_s,
                                            (GSourceFunc) _invocation_timeout,
                                            self);

  return self;
}

static bool
client_establish_invocation (YtsClient   *self,
                             char const   *invocation_id,
                             YtsContact  *contact,
                             char const   *proxy_id)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  InvocationData *invocation_data;

  invocation_data = g_hash_table_lookup (priv->invocations,
                                         invocation_id);
  if (invocation_data) {
    /* Already an invocation running with this ID, bail out. */
    g_critical ("%s: Already have an invocation for ID %s",
                G_STRLOC,
                invocation_id);
    return false;
  }

  invocation_data = invocation_data_create (self,
                                            contact,
                                            proxy_id,
                                            invocation_id,
                                            INVOCATION_RESPONSE_TIMEOUT_S);
  g_hash_table_insert (priv->invocations,
                       g_strdup (invocation_id),
                       invocation_data);

  return true;
}

/*
 * ProxyData
 */

typedef struct {
  YtsContact const *contact;    /* free pointer, no ref. */
  char              *proxy_id;
} ProxyData;

static ProxyData *
proxy_data_create (YtsContact const  *contact,
                   char const         *proxy_id)
{
  ProxyData *self;

  self = g_new0 (ProxyData, 1);
  self->contact = contact;
  self->proxy_id = g_strdup (proxy_id);

  return self;
}

static void
proxy_data_destroy (ProxyData *self)
{
  g_free (self->proxy_id);
  g_free (self);
}

/*
 * ProxyList
 */

typedef struct {
  GList *list;
} ProxyList;

static ProxyList *
proxy_list_create_with_proxy (YtsContact const *contact,
                              char const        *proxy_id)
{
  ProxyList *self;
  ProxyData *data;

  self = g_new0 (ProxyList, 1);

  data = proxy_data_create (contact, proxy_id);

  self->list = g_list_append (NULL, data);

  return self;
}

static bool
proxy_list_ensure_proxy (ProxyList          *self,
                         YtsContact const  *contact,
                         char const         *proxy_id)
{
  GList const *iter;
  ProxyData   *proxy_data;

  g_return_val_if_fail (self, false);
  g_warn_if_fail (self->list);

  for (iter = self->list; iter; iter = iter->next) {
    proxy_data = (ProxyData *) iter->data;
    if (proxy_data->contact == contact &&
        0 == g_strcmp0 (proxy_data->proxy_id, proxy_id)) {
      /* Proxy already in list */
      return false;
    }
  }

  proxy_data = proxy_data_create (contact, proxy_id);
  self->list = g_list_prepend (self->list, proxy_data);

  return true;
}

static void
proxy_list_purge_contact (ProxyList         *self,
                          YtsContact const *contact)
{
  GList *iter;
  bool   found;

  g_return_if_fail (self);
  g_return_if_fail (self->list);

  // FIXME need to do this in a smarter way.
  do {
    found = false;
    for (iter = self->list; iter; iter = iter->next) {

      ProxyData *data = (ProxyData *) iter->data;

      if (data->contact == contact) {
        proxy_data_destroy (data);
        iter->data = NULL;
        self->list = g_list_delete_link (self->list, iter);
        found = true;
        break;
      }
    }
  } while (found);
}

static void
proxy_list_purge_proxy_id (ProxyList  *self,
                           char const *proxy_id)
{
  GList *iter;
  bool   found;

  g_return_if_fail (self);
  g_return_if_fail (self->list);

  // FIXME need to do this in a smarter way.
  do {
    found = false;
    for (iter = self->list; iter; iter = iter->next) {

      ProxyData *data = (ProxyData *) iter->data;

      if (0 == g_strcmp0 (data->proxy_id, proxy_id)) {
        proxy_data_destroy (data);
        iter->data = NULL;
        self->list = g_list_delete_link (self->list, iter);
        found = true;
        break;
      }
    }
  } while (found);
}

static bool
proxy_list_is_empty (ProxyList  *self)
{
  g_return_val_if_fail (self, true);

  return self->list == NULL;
}

static void
proxy_list_destroy (ProxyList *self)
{
  g_return_if_fail (self);

  if (self->list) {
    do {
      ProxyData *data = (ProxyData *) self->list->data;
      proxy_data_destroy (data);
      self->list->data = NULL;
    } while (NULL != (self->list = g_list_delete_link (self->list, self->list)));
  }

  g_free (self);
}

/*
 * YtsClient
 */

static gboolean
yts_client_channel_requested (TpChannel *proxy)
{
  GHashTable *props;
  gboolean    requested;

  props = tp_channel_borrow_immutable_properties ((TpChannel*)proxy);

  requested = tp_asv_get_boolean (props, TP_PROP_CHANNEL_REQUESTED, NULL);

  return requested;
}

static void
yts_client_ft_op_cb (EmpathyTpFile *tp_file,
                      const GError  *error,
                      gpointer       data)
{
  if (error)
    {
      g_warning ("Incoming file transfer failed: %s", error->message);
    }
}

static void
yts_client_ft_accept_cb (TpProxy      *proxy,
                          GHashTable   *props,
                          const GError *error,
                          gpointer      self,
                          GObject      *weak_object)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  char const        *name;
  char const        *jid;
  uint64_t            offset;
  uint64_t            size;
  GHashTable        *iprops;
  YtsContact       *item;
  guint32            ihandle;

  iprops = tp_channel_borrow_immutable_properties ((TpChannel*)proxy);

  ihandle = tp_asv_get_uint32 (iprops,
                               TP_PROP_CHANNEL_INITIATOR_HANDLE,
                               NULL);

  if ((item = yts_roster_find_contact_by_handle (priv->roster, ihandle)))
    {
      jid = yts_contact_get_id (item);
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

  g_signal_emit (self, signals[INCOMING_FILE], 0,
                 jid, name, size, offset, proxy);
}

static void
yts_client_ft_handle_state (YtsClient *self, TpChannel *proxy, guint state)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  GHashTable        *props;
  gboolean           requested;

  props = tp_channel_borrow_immutable_properties ((TpChannel*)proxy);
  if (!(requested = tp_asv_get_boolean (props, TP_PROP_CHANNEL_REQUESTED,NULL)))
    {
      YtsContact *item;
      guint32      ihandle;

      ihandle = tp_asv_get_uint32 (props,
                                   TP_PROP_CHANNEL_INITIATOR_HANDLE,
                                   NULL);
      item = yts_roster_find_contact_by_handle (priv->roster, ihandle);

      switch (state)
        {
        case 1:
          {
            if (item)
              YTS_NOTE (FILE_TRANSFER,
                         "Got request for FT channel from %s (%s)",
                         yts_contact_get_id (item),
                         tp_proxy_get_bus_name (proxy));
            else
              YTS_NOTE (FILE_TRANSFER,
                         "Got request for FT channel from handle %d",
                         ihandle);

            tp_cli_dbus_properties_call_get_all (proxy,
                                           -1,
                                           TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,
                                           yts_client_ft_accept_cb,
                                           self,
                                           NULL,
                                           (GObject*) self);
          }
          break;
        case 2:
          YTS_NOTE (FILE_TRANSFER, "Incoming stream state (%s) --> 'accepted'",
                     tp_proxy_get_bus_name (proxy));
          break;
        case 3:
          YTS_NOTE (FILE_TRANSFER, "Incoming stream state (%s) --> 'open'",
                     tp_proxy_get_bus_name (proxy));
          break;
        case 4:
        case 5:
          YTS_NOTE (FILE_TRANSFER, "Incoming stream state (%s) --> '%s'",
                     tp_proxy_get_bus_name (proxy),
                     state == 4 ? "completed" : "cancelled");
          {
            char const *name;
            char const *jid;

            if (item)
              {
                jid = yts_contact_get_id (item);

                name   = tp_asv_get_string (props, "Filename");

                g_signal_emit (self, signals[INCOMING_FILE_FINISHED], 0,
                               jid, name, state == 4 ? TRUE : FALSE);
              }
          }
          break;
        default:
          YTS_NOTE (FILE_TRANSFER, "Invalid value of stream state: %d", state);
        }
    }
  else
    YTS_NOTE (FILE_TRANSFER, "The FT channel was requested by us ... (%s)",
             tp_proxy_get_bus_name (proxy));
}

static void
yts_client_ft_state_cb (TpChannel *proxy,
                         guint      state,
                         guint      reason,
                         gpointer   data,
                         GObject   *object)
{
  YtsClient *client = data;

  YTS_NOTE (FILE_TRANSFER,
             "FT channel changed status to %d (reason %d)", state, reason);

  yts_client_ft_handle_state (client, proxy, state);
}

static void
yts_client_ft_core_cb (GObject *proxy, GAsyncResult *res, gpointer data)
{
  YtsClient *client  = data;
  TpChannel  *channel = (TpChannel*) proxy;
  GError     *error   = NULL;

  YTS_NOTE (FILE_TRANSFER, "FT channel ready");

  tp_cli_channel_type_file_transfer_connect_to_file_transfer_state_changed
    (channel,
     yts_client_ft_state_cb,
     client,
     NULL,
     (GObject*)client,
     &error);

  if (!yts_client_channel_requested (channel))
    yts_client_ft_handle_state (client, channel, 1);
}

static void
yts_client_channel_cb (TpConnection *proxy,
                        char const   *path,
                        char const   *type,
                        guint         handle_type,
                        guint         handle,
                        gboolean      suppress_handle,
                        gpointer      self,
                        GObject      *weak_object)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  if (!path)
    {
      g_warning (G_STRLOC ":%s: no path!", __FUNCTION__);
      return;
    }

  YTS_NOTE (CLIENT, "New channel: %s: %s: h type %d, h %d",
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
          YtsContact *item;

          ch = tp_channel_new (proxy, path, type, handle_type, handle, &error);

          if ((item = yts_roster_find_contact_by_handle (priv->roster,
                                                           handle)))
            {
              yts_contact_set_ft_channel (item, ch);

              tp_proxy_prepare_async (ch, features,
                                      yts_client_ft_core_cb, self);
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
yts_client_authenticated (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  priv->authenticated = true;

  YTS_NOTE (CONNECTION, "Authenticated");
}

static void
yts_client_ready (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  priv->ready = TRUE;

  YTS_NOTE (CLIENT, "YtsClient is ready");

  if (priv->tp_status && priv->status)
    {
      char *xml = yts_metadata_to_string ((YtsMetadata*)priv->status);
      unsigned   i;

      for (i = 0; i < priv->caps->len; ++i)
        {
          char const *c;

          c = g_quark_to_string (g_array_index (priv->caps, YtsCaps, i));

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
yts_client_cleanup_connection_resources (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  /*
   * Clean up items associated with this connection.
   */

  priv->ready    = FALSE;
  priv->prepared = FALSE;

  /*
   * Empty roster
   */
  if (priv->roster)
    yts_roster_clear (priv->roster);

  if (priv->connection)
    {
      g_object_unref (priv->connection);
      priv->connection = NULL;
    }
}

static gboolean
yts_client_reconnect_cb (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  priv->reconnect_id = 0;

  yts_client_connect (self);

  /* one off */
  return FALSE;
}

static void
yts_client_reconnect_after (YtsClient *self, guint after_seconds)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  g_return_if_fail (YTS_IS_CLIENT (self));

  priv->reconnect = TRUE;

  priv->reconnect_id =
    g_timeout_add_seconds (after_seconds,
                           (GSourceFunc) yts_client_reconnect_cb,
                           self);
}

static void
yts_client_disconnected (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  yts_client_cleanup_connection_resources (self);

  if (priv->reconnect)
    yts_client_reconnect_after (self, RECONNECT_DELAY);
}

static void
yts_client_raw_message (YtsClient   *self,
                        char const  *xml_payload)
{
}

static bool
yts_client_incoming_file (YtsClient   *self,
                           char const *from,
                           char const *name,
                           uint64_t     size,
                           uint64_t     offset,
                           TpChannel  *proxy)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  char              *path;
  GFile             *gfile;
  EmpathyTpFile     *tp_file;
  GCancellable      *cancellable;

  YTS_NOTE (FILE_TRANSFER, "Incoming file from %s", from);

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
                          yts_client_ft_op_cb,
                          self);

  g_free (path);
  g_object_unref (gfile);
  g_object_unref (cancellable);

  return TRUE;
}

static gboolean
yts_client_stop_accumulator (GSignalInvocationHint *ihint,
                              GValue                *accumulated,
                              const GValue          *returned,
                              gpointer               data)
{
  gboolean cont = g_value_get_boolean (returned);

  g_value_set_boolean (accumulated, cont);

  return cont;
}

/*
 * Callback for TpProxy::interface-added: we need to add the signals we
 * care for here.
 *
 * TODO -- should we not be able to connect directly to the signal bypassing
 * the unsightly TP machinery ?
 */
static void
yts_client_debug_iface_added_cb (TpProxy    *tproxy,
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

/*
 * Handler for Mgr debug output.
 */
static void
yts_client_debug_msg_cb (TpProxy    *proxy,
                          gdouble     timestamp,
                          char const *domain,
                          guint       level,
                          char const *msg,
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
      YTS_NOTE (MANAGER, "%s: %s", domain, msg);
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
yts_client_debug_msg_collect (DBusGProxy              *proxy,
                               gdouble                  timestamp,
                               char const              *domain,
                               guint                    level,
                               char const              *msg,
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

typedef void (*YtsClientMgrNewDebugMsg)(TpProxy *,
                                         gdouble,
                                         char const *,
                                         guint,
                                         char const *,
                                         gpointer, GObject *);

/*
 * The callback invoker
 */
static void
yts_client_debug_msg_invoke (TpProxy     *proxy,
                              GError      *error,
                              GValueArray *args,
                              GCallback    callback,
                              gpointer     data,
                              GObject     *weak_object)
{
  YtsClientMgrNewDebugMsg cb = (YtsClientMgrNewDebugMsg) callback;

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
yts_client_connect_debug_signals (YtsClient *client, TpProxy *proxy)
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
                                     G_CALLBACK (yts_client_debug_msg_collect),
                                     yts_client_debug_msg_invoke,
                                     G_CALLBACK (yts_client_debug_msg_cb),
                                     client,
                                     NULL,
                                     (GObject*)client,
                                     &error);

  if (error)
    {
      YTS_NOTE (MANAGER, "%s", error->message);
      g_clear_error (&error);
    }

  tp_cli_dbus_properties_call_set (proxy, -1, TP_IFACE_DEBUG,
                                   "Enabled", &v, NULL, NULL, NULL, NULL);
}

static void
yts_client_setup_debug  (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  TpDBusDaemon      *dbus;
  GError            *error = NULL;
  TpProxy           *proxy;
  char              *busname;

  dbus = tp_dbus_daemon_dup (&error);

  if (error != NULL)
    {
      g_warning ("%s", error->message);
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
                    G_CALLBACK (yts_client_debug_iface_added_cb), self);

  tp_proxy_add_interface_by_id (proxy, TP_IFACE_QUARK_DEBUG);

  /*
   * Connecting to the signals triggers the interface-added signal
   */
  yts_client_connect_debug_signals (self, proxy);

  g_object_unref (dbus);
  g_free (busname);
}

/*
 * Callback from the async tp_account_prepare_async() call
 *
 * This function is ready for the New World Order according to Ytstenut ...
 */
static void
yts_client_account_prepared_cb (GObject       *acc,
                                 GAsyncResult *res,
                                 gpointer      self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  GError            *error   = NULL;
  TpAccount         *account = (TpAccount*)acc;

  if (!tp_account_prepare_finish (account, res, &error))
    {
      g_error ("Account unprepared: %s", error->message);
    }

  priv->account = account;

  YTS_NOTE (CONNECTION, "Account successfully opened");

  priv->tp_client = tp_yts_client_new (priv->uid, account);

  if (priv->caps)
    {
      unsigned int i;
      for (i = 0; i < priv->caps->len; i++)
        {
          GQuark cap = g_array_index (priv->caps, GQuark, i);
          tp_yts_client_add_capability (priv->tp_client,
                                        g_quark_to_string (cap));
        }
    }

  /*
   * If connection has been requested already, make one
   */
  if (priv->connect)
    yts_client_make_connection (self);
}

static void
yts_client_account_cb (GObject *object, GAsyncResult *res, gpointer self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  GError            *error      = NULL;
  const GQuark       features[] = { TP_ACCOUNT_FEATURE_CORE, 0 };

  g_assert (TP_IS_YTS_ACCOUNT_MANAGER (object));
  g_assert (G_IS_ASYNC_RESULT (res));

  priv->account =
    tp_yts_account_manager_get_account_finish (TP_YTS_ACCOUNT_MANAGER (object),
                                               res, &error);

  if (error)
    g_error ("Could not access account: %s", error->message);

  YTS_NOTE (CONNECTION, "Got account");

  if (yts_debug_flags & YTS_DEBUG_TP)
    tp_debug_set_flags ("all");

  if (yts_debug_flags & YTS_DEBUG_MANAGER)
    yts_client_setup_debug (self);

  tp_account_prepare_async (priv->account,
                            &features[0],
                            yts_client_account_prepared_cb,
                            self);
}

static void
yts_client_constructed (GObject *object)
{
  YtsClientPrivate *priv = GET_PRIVATE (object);
  GError              *error     = NULL;

  if (G_OBJECT_CLASS (yts_client_parent_class)->constructed)
    G_OBJECT_CLASS (yts_client_parent_class)->constructed (object);

  priv->roster   = yts_roster_new (YTS_CLIENT (object));
  priv->unwanted = yts_roster_new (YTS_CLIENT (object));

  if (!priv->uid || !*priv->uid)
    g_error ("UID must be set at construction time.");

  if (priv->protocol == YTS_PROTOCOL_LOCAL_XMPP)
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
                                            yts_client_account_cb,
                                            object);
}

static void
yts_client_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsClientPrivate *priv = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_UID:
      g_value_set_string (value, priv->uid);
      break;
    case PROP_ICON_TOKEN:
      g_value_set_string (value, priv->icon_token);
      break;
    case PROP_PROTOCOL:
      g_value_set_enum (value, priv->protocol);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yts_client_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsClientPrivate *priv = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_UID:
      {
        g_free (priv->uid);
        priv->uid = g_value_dup_string (value);
      }
      break;
    case PROP_PROTOCOL:
      priv->protocol = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yts_client_dispose (GObject *object)
{
  YtsClientPrivate *priv = GET_PRIVATE (object);

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

  if (priv->invocations)
    {
      g_hash_table_destroy (priv->invocations);
      priv->invocations = NULL;
    }

  if (priv->proxies)
    {
      g_hash_table_destroy (priv->proxies);
      priv->proxies = NULL;
    }

  G_OBJECT_CLASS (yts_client_parent_class)->dispose (object);
}

static void
yts_client_finalize (GObject *object)
{
  YtsClientPrivate *priv = GET_PRIVATE (object);

  g_free (priv->uid);
  g_free (priv->icon_token);
  g_free (priv->icon_mime_type);
  g_free (priv->mgr_name);
  g_free (priv->incoming_dir);

  if (priv->caps)
    g_array_free (priv->caps, TRUE);

  if (priv->icon_data)
    g_array_free (priv->icon_data, TRUE);

  G_OBJECT_CLASS (yts_client_parent_class)->finalize (object);
}

static void
yts_client_class_init (YtsClientClass *klass)
{
  GParamSpec   *pspec;
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsClientPrivate));

  object_class->dispose      = yts_client_dispose;
  object_class->finalize     = yts_client_finalize;
  object_class->constructed  = yts_client_constructed;
  object_class->get_property = yts_client_get_property;
  object_class->set_property = yts_client_set_property;

  klass->authenticated       = yts_client_authenticated;
  klass->ready               = yts_client_ready;
  klass->disconnected        = yts_client_disconnected;
  klass->raw_message         = yts_client_raw_message;
  klass->incoming_file       = yts_client_incoming_file;

  /**
   * YtsClient:uid:
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
   * YtsClient:protocol:
   *
   * XMPP protocol to use for connection.
   *
   * Since: 0.1
   */
  pspec = g_param_spec_enum ("protocol",
                             "Protocol",
                             "Protocol",
                             YTS_TYPE_PROTOCOL,
                             0,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_PROTOCOL, pspec);

  /**
   * YtsClient::authenticated:
   * @self: object which emitted the signal.
   *
   * The authenticated signal is emited when connection to the Ytstenut server
   * is successfully established.
   *
   * Since: 0.1
   */
  signals[AUTHENTICATED] =
    g_signal_new ("authenticated",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (YtsClientClass, authenticated),
                  NULL, NULL,
                  yts_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * YtsClient::ready
   * @self: object which emitted the signal,
   *
   * The ready signal is emited when the initial Telepathy set up is ready.
   * (In practical terms this means the subscription channels are prepared.)
   *
   * Since: 0.1
   */
  signals[READY] =
    g_signal_new ("ready",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (YtsClientClass, ready),
                  NULL, NULL,
                  yts_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * YtsClient::disconnected
   * @self: object which emitted the signal,
   *
   * The disconnected signal is emited when connection to the Ytstenut server
   * is successfully established.
   *
   * Since: 0.1
   */
  signals[DISCONNECTED] =
    g_signal_new ("disconnected",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (YtsClientClass, disconnected),
                  NULL, NULL,
                  yts_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * YtsClient::message
   * @self: object which emitted the signal,
   * @message: #YtsMessage, the message
   *
   * The message signal is emitted when message is received from one of the
   * contacts.
   *
   * Since: 0.1
   */
  signals[RAW_MESSAGE] =
    g_signal_new ("raw-message",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsClientClass, raw_message),
                  NULL, NULL,
                  yts_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  signals[TEXT_MESSAGE] =
    g_signal_new ("text-message",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  yts_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  signals[LIST_MESSAGE] =
    g_signal_new ("list-message",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  yts_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  signals[DICTIONARY_MESSAGE] =
    g_signal_new ("dictionary-message",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  yts_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1,
                  G_TYPE_POINTER);

  /**
   * YtsClient::error
   * @self: object which emitted the signal,
   * @error: #YtsError
   *
   * The error signal is emitted to indicate an error (or eventual success)
   * during the handling of an operation for which the Ytstenut API initially
   * returned %YTS_ERROR_PENDING. The original operation can be determined
   * using the atom part of the #YtsError parameter.
   *
   * Since: 0.1
   */
  signals[ERROR] =
    g_signal_new ("error",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  yts_marshal_VOID__UINT,
                  G_TYPE_NONE, 1,
                  G_TYPE_UINT);

  /**
   * YtsClient::incoming-file
   * @self: object which emitted the signal,
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
    g_signal_new ("incoming-file",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsClientClass, incoming_file),
                  yts_client_stop_accumulator, NULL,
                  yts_marshal_BOOLEAN__STRING_STRING_UINT64_UINT64_OBJECT,
                  G_TYPE_BOOLEAN, 5,
                  G_TYPE_STRING,
                  G_TYPE_STRING,
                  G_TYPE_UINT64,
                  G_TYPE_UINT64,
                  TP_TYPE_CHANNEL);

  /**
   * YtsClient::incoming-file-finished
   * @self: object which emitted the signal,
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
    g_signal_new ("incoming-file-finished",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  yts_marshal_VOID__STRING_STRING_BOOLEAN,
                  G_TYPE_NONE, 3,
                  G_TYPE_STRING,
                  G_TYPE_STRING,
                  G_TYPE_BOOLEAN);
}

static void
yts_client_init (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  yts_client_set_incoming_file_directory (self, NULL);

  priv->services = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          g_object_unref);

  priv->invocations = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             (GDestroyNotify) invocation_data_destroy);

  priv->proxies = g_hash_table_new_full (g_str_hash,
                                         g_str_equal,
                                         g_free,
                                         (GDestroyNotify) proxy_list_destroy);
}

/**
 * yts_client_new:
 * @protocol: #YtsProtocol
 * @service_id: Unique ID for this service; UIDs must follow the dbus
                convention for unique names.
 *
 * Creates a new #YtsClient object.
 *
 * Returns (tranfer full): a #YtsClient object.
 *
 * Since: 0.1
 */
YtsClient *
yts_client_new (YtsProtocol  protocol,
                char const  *service_id)
{
  g_return_val_if_fail (service_id, NULL);

  return g_object_new (YTS_TYPE_CLIENT,
                       "protocol", protocol,
                       "uid",      service_id,
                       NULL);
}

static GVariant *
variant_new_from_escaped_literal (char const *string)
{
  GVariant  *v;
  char      *unescaped;

  unescaped = g_uri_unescape_string (string, NULL);
  v = g_variant_new_parsed (unescaped);
  g_free (unescaped);

  return v;
}

static gboolean
dispatch_to_service (YtsClient  *self,
                     char const *sender_jid,
                     char const *xml)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  RestXmlParser *parser;
  RestXmlNode   *node;
  char const    *proxy_id;
  char const    *capability;
  char const    *type;
  YtsContact   *contact;
  gboolean       dispatched = FALSE;

  parser = rest_xml_parser_new ();
  node = rest_xml_parser_parse_from_data (parser, xml, strlen (xml));
  if (NULL == node) {
    // FIXME report error
    g_critical ("%s : Failed to parse message '%s'", G_STRLOC, xml);
    return false;
  }

  proxy_id = rest_xml_node_get_attr (node, "from-service");
  if (NULL == proxy_id) {
    // FIXME report error
    g_critical ("%s : Malformed message, 'from-service' missing in '%s'",
                G_STRLOC,
                xml);
    return false;
  }

  capability = rest_xml_node_get_attr (node, "capability");
  if (NULL == capability) {
    // FIXME report error
    g_critical ("%s : Malformed message, 'capability' missing in '%s'",
                G_STRLOC,
                xml);
    return false;
  }

  type = rest_xml_node_get_attr (node, "type");
  if (NULL == type) {
    // FIXME report error
    g_critical ("%s : Malformed message, 'type' missing in '%s'",
                G_STRLOC,
                xml);
    return false;
  }

  contact = yts_roster_find_contact_by_jid (priv->roster, sender_jid);
  if (NULL == contact) {
    // FIXME report error
    g_critical ("%s : Contact for '%s' not found",
                G_STRLOC,
                sender_jid);
    return false;
  }

  /*
   * Low-level interface
   */

  if (0 == g_strcmp0 (SERVICE_FQC_ID, capability) &&
      0 == g_strcmp0 ("text", type))
    {
      char const *escaped_payload = rest_xml_node_get_attr (node, "payload");
      GVariant *payload = escaped_payload ?
                            variant_new_from_escaped_literal (escaped_payload) :
                            NULL;
      if (payload)
        {
          char const *text = g_variant_get_string (payload, NULL);
          g_signal_emit (self, signals[TEXT_MESSAGE], 0, text);
          g_variant_unref (payload);
        }
      else
        {
          // FIXME report
          g_warning ("%s : Message empty", G_STRLOC);
        }
    }
  else if (0 == g_strcmp0 (SERVICE_FQC_ID, capability) &&
           0 == g_strcmp0 ("list", type))
    {
      char const *escaped_payload = rest_xml_node_get_attr (node, "payload");
      GVariant *payload = escaped_payload ?
                            variant_new_from_escaped_literal (escaped_payload) :
                            NULL;
      if (payload)
        {
          char const **list = g_variant_get_strv (payload, NULL);
          g_signal_emit (self, signals[LIST_MESSAGE], 0, list);
          g_free (list);
          g_variant_unref (payload);
        }
      else
        {
          // FIXME report
          g_warning ("%s : Message empty", G_STRLOC);
        }
    }
  else if (0 == g_strcmp0 (SERVICE_FQC_ID, capability) &&
           0 == g_strcmp0 ("dictionary", type))
    {
      char const *escaped_payload = rest_xml_node_get_attr (node, "payload");
      GVariant *payload = escaped_payload ?
                            variant_new_from_escaped_literal (escaped_payload) :
                            NULL;
      if (payload)
        {
          GVariantIter iter;
          char const *name;
          char const *value;
          size_t n_entries;
          if (0 < (n_entries = g_variant_iter_init (&iter, payload)))
            {
              char **dictionary = g_new0 (char *, n_entries * 2 + 1);
              unsigned i = 0;
              while (g_variant_iter_loop (&iter, "{ss}", &name, &value))
                {
                  dictionary[i++] = g_strdup (name);
                  dictionary[i++] = g_strdup (value);
                }
              dictionary[i] = NULL;
              g_signal_emit (self, signals[DICTIONARY_MESSAGE], 0, dictionary);
              g_strfreev (dictionary);
            }
          g_variant_unref (payload);
        }
      else
        {
          // FIXME report
          g_warning ("%s : Message empty", G_STRLOC);
        }
    }

  /*
   * High-level interface
   */

  else if (0 == g_strcmp0 ("invocation", type))
    {
      /* Deliver to service */
      YtsServiceAdapter *adapter = g_hash_table_lookup (priv->services,
                                                         capability);
      if (adapter)
        {
          bool keep_sae;
          char const *invocation_id = rest_xml_node_get_attr (node, "invocation");
          char const *aspect = rest_xml_node_get_attr (node, "aspect");
          char const *args = rest_xml_node_get_attr (node, "arguments");
          GVariant *arguments = args ? variant_new_from_escaped_literal (args) : NULL;

          // FIXME check return value
          client_establish_invocation (self,
                                       invocation_id,
                                       contact,
                                       proxy_id);
          keep_sae = yts_service_adapter_invoke (adapter,
                                                  invocation_id,
                                                  aspect,
                                                  arguments);
          if (!keep_sae) {
            client_conclude_invocation (self, invocation_id);
          }

          dispatched = TRUE;
        }
      else
        {
          // FIXME we should probably report back that there's no adapter?
        }
    }
  else if (0 == g_strcmp0 ("event", type))
    {
      char const *aspect = rest_xml_node_get_attr (node, "aspect");
      char const *args = rest_xml_node_get_attr (node, "arguments");
      GVariant *arguments = args ? variant_new_from_escaped_literal (args) : NULL;

      dispatched = yts_contact_dispatch_event (contact,
                                                capability,
                                                aspect,
                                                arguments);
    }
  else if (0 == g_strcmp0 ("response", type))
    {
      char const *invocation_id = rest_xml_node_get_attr (node, "invocation");
      char const *ret = rest_xml_node_get_attr (node, "response");
      GVariant *response = ret ? variant_new_from_escaped_literal (ret) : NULL;

      dispatched = yts_contact_dispatch_response (contact,
                                                   capability,
                                                   invocation_id,
                                                   response);
    }
  else
    {
      // FIXME report error
      g_critical ("%s : Unknown message type '%s'", G_STRLOC, type);
    }

  g_object_unref (parser);
  return dispatched;
}

static void
yts_client_yts_channels_received_cb (TpYtsClient *tp_client,
                                      YtsClient  *client)
{
  TpYtsChannel  *ch;

  while ((ch = tp_yts_client_accept_channel (tp_client)))
    {
      char const      *from;
      GHashTable      *props;
      GHashTableIter   iter;
      gpointer         key, value;

      from = tp_channel_get_initiator_identifier (TP_CHANNEL (ch));

      g_object_get (ch, "channel-properties", &props, NULL);
      g_assert (props);

      g_hash_table_iter_init (&iter, props);
      while (g_hash_table_iter_next (&iter, &key, &value))
        {
          GValue      *v = value;
          char        *k = key;

          if (!strcmp (k, "org.freedesktop.ytstenut.xpmn.Channel.RequestBody"))
            {
              char const *xml_payload = g_value_get_string (v);
              gboolean dispatched = dispatch_to_service (client,
                                                         from,
                                                         xml_payload);

              if (!dispatched)
                {
                  g_signal_emit (client, signals[RAW_MESSAGE], 0, xml_payload);
                }
            }
        }
    }
}

/**
 * yts_client_disconnect:
 * @self: object on which to invoke this method.
 *
 * Disconnects @self.
 *
 * Since: 0.1
 */
void
yts_client_disconnect (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  g_return_if_fail (YTS_IS_CLIENT (self));

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

static void
yts_client_connected_cb (TpConnection   *proxy,
                          const GError  *error,
                          YtsClient     *self,
                          GObject       *weak_object)
{
  if (error)
    {
      g_warning (G_STRLOC ": %s: %s", __FUNCTION__, error->message);

      yts_client_disconnect (self);
      yts_client_reconnect_after (self, RECONNECT_DELAY);
    }
}

static void
yts_client_error_cb (TpConnection *proxy,
                      char const   *arg_Error,
                      GHashTable   *arg_Details,
                      gpointer      user_data,
                      GObject      *weak_object)
{
  YTS_NOTE (CLIENT, "Error: %s", arg_Error);
}

static void
yts_client_status_cb (TpConnection  *proxy,
                       guint         arg_Status,
                       guint         arg_Reason,
                       YtsClient    *self,
                       GObject      *weak_object)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  char const *status[] = {"'connected'   ",
                          "'connecting'  ",
                          "'disconnected'"};
  char const *reason[] =
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

  if (priv->disposed)
    return;

  YTS_NOTE (CONNECTION, "Connection: %s: '%s'",
           status[arg_Status], reason[arg_Reason]);

  if (arg_Status == TP_CONNECTION_STATUS_CONNECTED)
    g_signal_emit (self, signals[AUTHENTICATED], 0);
  else if (arg_Status == TP_CONNECTION_STATUS_DISCONNECTED)
    g_signal_emit (self, signals[DISCONNECTED], 0);
}

static gboolean
yts_client_caps_overlap (GArray *mycaps, char **caps)
{
  unsigned i;

  /* TODO -- this is not nice, maybe YtsClient:caps should also be just a
   *         char**
   */
  for (i = 0; i < mycaps->len; ++i)
    {
      char **p;

      for (p = caps; *p; ++p)
        {
          if (!strcmp (g_quark_to_string (g_array_index (mycaps, YtsCaps, i)),
                       *p))
            {
              return TRUE;
            }
        }
    }

  return FALSE;
}

static gboolean
yts_client_process_one_service (YtsClient         *self,
                                char const        *jid,
                                char const        *sid,
                                const GValueArray *service_info)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  char const        *type;
  GHashTable        *names;
  char             **caps;
  GHashTable        *service_statuses;
  YtsRoster        *roster;

  if (service_info->n_values != 3)
    {
      g_warning ("Missformed service description (nvalues == %d)",
                 service_info->n_values);
      return FALSE;
    }

  YTS_NOTE (CLIENT, "Processing service %s:%s", jid, sid);

  type  = g_value_get_string (&service_info->values[0]);
  names = g_value_get_boxed (&service_info->values[1]);
  caps  = g_value_get_boxed (&service_info->values[2]);

  if (!priv->caps || !caps || !*caps ||
      yts_client_caps_overlap (priv->caps, caps))
    roster = priv->roster;
  else
    roster = priv->unwanted;

  YTS_NOTE (CLIENT, "Using roster %s",
             roster == priv->roster ? "wanted" : "unwanted");

  service_statuses = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            g_free);
  if (priv->tp_status) {
    GHashTable *discovered_statuses = tp_yts_status_get_discovered_statuses (
                                                              priv->tp_status);
    if (discovered_statuses) {
      GHashTable *contact_statuses = g_hash_table_lookup (discovered_statuses,
                                                          jid);
      if (contact_statuses) {
        unsigned i;
        for (i = 0; caps && caps[i]; i++) {
          GHashTable *capability_statuses = g_hash_table_lookup (contact_statuses,
                                                                 caps[i]);
          if (capability_statuses) {
            char const *status_xml = g_hash_table_lookup (capability_statuses,
                                                          sid);
            if (status_xml) {
              g_hash_table_insert (service_statuses,
                                   g_strdup (caps[i]),
                                   g_strdup (status_xml));
            }
          }
        }
      }
    }
  }

  yts_roster_add_service (roster,
                          jid,
                          sid,
                          type,
                          (char const **)caps,
                          names,
                          service_statuses);

  g_hash_table_unref (service_statuses);

  return TRUE;
}

static void
yts_client_service_added_cb (TpYtsStatus        *tp_status,
                             char const         *jid,
                             char const         *sid,
                             const GValueArray  *service_info,
                             YtsClient          *self)
{
  yts_client_process_one_service (self, jid, sid, service_info);
}

static void
yts_client_service_removed_cb (TpYtsStatus  *tp_status,
                               char const   *jid,
                               char const   *sid,
                               YtsClient    *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  yts_roster_remove_service_by_id (priv->roster, jid, sid);
}

static void
yts_client_process_status (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  GHashTable        *services;

  if ((services = tp_yts_status_get_discovered_services (priv->tp_status)))
    {
      char           *jid;
      GHashTable     *service;
      GHashTableIter  iter;

      if (g_hash_table_size (services) <= 0)
        YTS_NOTE (CLIENT, "No services discovered so far");

      g_hash_table_iter_init (&iter, services);
      while (g_hash_table_iter_next (&iter, (void**)&jid, (void**)&service))
        {
          char           *sid;
          GValueArray    *service_info;
          GHashTableIter  iter2;

          g_hash_table_iter_init (&iter2, service);
          while (g_hash_table_iter_next (&iter2, (void**)&sid, (void**)&service_info))
            {
              yts_client_process_one_service (self, jid, sid, service_info);
            }
        }
    }
  else
    YTS_NOTE (CLIENT, "No discovered services");
}

static void
yts_client_advertise_status_cb (GObject      *source_object,
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
      YTS_NOTE (CLIENT, "Advertising of status succeeded");
    }

  g_clear_error (&error);
}

static void
yts_client_dispatch_status (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  char *xml = NULL;
  unsigned i;

  g_return_if_fail (priv->caps && priv->caps->len);

  if (priv->status)
    xml = yts_metadata_to_string ((YtsMetadata*)priv->status);

  YTS_NOTE (CLIENT, "Setting status to\n%s", xml);

  for (i = 0; i < priv->caps->len; ++i)
    {
      char const *c;

      c = g_quark_to_string (g_array_index (priv->caps, YtsCaps, i));

      tp_yts_status_advertise_status_async (priv->tp_status,
                                            c,
                                            priv->uid,
                                            xml,
                                            NULL,
                                            yts_client_advertise_status_cb,
                                            self);
    }

  g_free (xml);
}

static void
_tp_yts_status_changed (TpYtsStatus *tp_status,
                        char const  *contact_id,
                        char const  *fqc_id,
                        char const  *service_id,
                        char const  *status_xml,
                        YtsClient   *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  yts_roster_update_contact_status (priv->roster,
                                    contact_id,
                                    service_id,
                                    fqc_id,
                                    status_xml);
}

static void
yts_client_yts_status_cb (GObject       *obj,
                          GAsyncResult  *res,
                          YtsClient     *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  TpAccount         *acc    = TP_ACCOUNT (obj);
  GError            *error  = NULL;
  TpYtsStatus       *tp_status;

  if (!(tp_status = tp_yts_status_ensure_finish (acc, res,&error)))
    {
      g_error ("Failed to obtain tp_status: %s", error->message);
    }

  YTS_NOTE (CLIENT, "Processing tp_status");

  priv->tp_status = tp_status;

  tp_g_signal_connect_object (tp_status, "service-added",
                              G_CALLBACK (yts_client_service_added_cb),
                              self, 0);
  tp_g_signal_connect_object (tp_status, "service-removed",
                              G_CALLBACK (yts_client_service_removed_cb),
                              self, 0);
  tp_g_signal_connect_object (tp_status, "status-changed",
                              G_CALLBACK (_tp_yts_status_changed),
                              self, 0);


  if (priv->status)
    yts_client_dispatch_status (self);

  yts_client_process_status (self);

  if (!priv->ready)
    {
      YTS_NOTE (CLIENT, "Emitting 'ready' signal");
      g_signal_emit (self, signals[READY], 0);
    }
}

static void
yts_client_connection_ready_cb (TpConnection *conn,
                                GParamSpec   *par,
                                YtsClient   *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  GCancellable      *cancellable;

  if (tp_connection_is_ready (conn))
    {
      YTS_NOTE (CONNECTION, "TP Connection entered ready state");

      cancellable = g_cancellable_new ();

      tp_yts_status_ensure_async (priv->account,
                                  cancellable,
                                  (GAsyncReadyCallback) yts_client_yts_status_cb,
                                  self);

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
yts_client_set_avatar_cb (TpConnection *proxy,
                           char const   *token,
                           const GError *error,
                           YtsClient    *self,
                           GObject      *weak_object)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

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
yts_client_is_avatar_type_supported (YtsClient *self,
                                      char const *mime_type,
                                      guint       bytes)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  char                 **p;
  char const            *alt_type = NULL;
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
yts_client_connection_prepare_cb (GObject       *connection,
                                  GAsyncResult  *res,
                                  YtsClient     *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
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
        YTS_NOTE (CONNECTION, "Registered TpYtsClient");

      tp_g_signal_connect_object (priv->tp_client, "received-channels",
                              G_CALLBACK (yts_client_yts_channels_received_cb),
                              self, 0);

      if (priv->icon_data &&
          yts_client_is_avatar_type_supported (self,
                                              priv->icon_mime_type,
                                              priv->icon_data->len))
        {
          tp_cli_connection_interface_avatars_call_set_avatar (
                                                          priv->connection,
                                                          -1,
                                                          priv->icon_data,
                                                          priv->icon_mime_type,
  (tp_cli_connection_interface_avatars_callback_for_set_avatar) yts_client_set_avatar_cb,
                                                          self,
                                                          NULL,
                                                          (GObject*)self);

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
      if (priv->protocol != YTS_PROTOCOL_LOCAL_XMPP)
        yts_client_setup_caps (client);
#endif
    }
}

/*
 * Sets up required features on the connection, and connects callbacks to
 * signals that we care about.
 */
static void
yts_client_setup_account_connection (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  GError            *error = NULL;
  GQuark             features[] = { TP_CONNECTION_FEATURE_CONTACT_INFO,
                                    TP_CONNECTION_FEATURE_AVATAR_REQUIREMENTS,
                                    TP_CONNECTION_FEATURE_CAPABILITIES,
                                    TP_CONNECTION_FEATURE_CONNECTED,
                                    0 };

  priv->connection = tp_account_get_connection (priv->account);

  g_assert (priv->connection);

  priv->dialing = FALSE;

  YTS_NOTE (CONNECTION, "Connection ready ?: %d",
             tp_connection_is_ready (priv->connection));

  tp_g_signal_connect_object (priv->connection, "notify::connection-ready",
                              G_CALLBACK (yts_client_connection_ready_cb),
                              self, 0);

  tp_cli_connection_connect_to_connection_error (priv->connection,
                                                 yts_client_error_cb,
                                                 self,
                                                 NULL,
                                                 (GObject*)self,
                                                 &error);

  if (error)
    {
      g_critical (G_STRLOC ": %s: %s; no Ytstenut functionality will be "
                  "available", __FUNCTION__, error->message);
      g_clear_error (&error);
      return;
    }

  tp_cli_connection_connect_to_status_changed (priv->connection,
      (tp_cli_connection_signal_callback_status_changed) yts_client_status_cb,
                                               self,
                                               NULL,
                                               (GObject*) self,
                                               &error);

  if (error)
    {
      g_critical (G_STRLOC ": %s: %s; no Ytstenut functionality will be "
                  "available", __FUNCTION__, error->message);
      g_clear_error (&error);
      return;
    }

  tp_cli_connection_connect_to_new_channel (priv->connection,
                                            yts_client_channel_cb,
                                            self,
                                            NULL,
                                            (GObject*)self,
                                            &error);

  if (error)
    {
      g_critical (G_STRLOC ": %s: %s; no Ytstenut functionality will be "
                  "available", __FUNCTION__, error->message);
      g_clear_error (&error);
      return;
    }

  tp_proxy_prepare_async (priv->connection,
                          features,
                          (GAsyncReadyCallback) yts_client_connection_prepare_cb,
                          self);
}

/*
 * Callback for the async request to change presence ... not that we do
 * do anything with it, except when it fails.
 */
static void
yts_client_account_online_cb (GObject      *acc,
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

  YTS_NOTE (CONNECTION,
             "Request to change presence to 'online' succeeded: %d, %s:%s",
             presence, stat, msg);

  g_free (stat);
  g_free (msg);
}

/*
 * One off handler for connection coming online
 */
static void
yts_client_account_connection_notify_cb (TpAccount  *account,
                                          GParamSpec *pspec,
                                          YtsClient *client)
{
  YTS_NOTE (CONNECTION, "We got connection!");

  g_signal_handlers_disconnect_by_func (account,
                                   yts_client_account_connection_notify_cb,
                                   client);

  yts_client_setup_account_connection (client);
}

static void
yts_client_make_connection (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  /*
   * If we don't have an account yet, we do nothing and will make call to this
   * function when the account is ready.
   */
  if (!priv->account)
    {
      YTS_NOTE (CONNECTION, "Account not yet available");
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
      YTS_NOTE (CONNECTION, "Currently off line, changing ...");

      g_signal_connect (priv->account, "notify::connection",
                        G_CALLBACK (yts_client_account_connection_notify_cb),
                        self);

      tp_account_request_presence_async (priv->account,
                                         TP_CONNECTION_PRESENCE_TYPE_AVAILABLE,
                                         "online",
                                         "online",
                                         yts_client_account_online_cb,
                                         self);
    }
  else
    yts_client_setup_account_connection (self);
}

/**
 * yts_client_connect:
 * @self: #YtsClient
 *
 * Initiates connection to the mesh. Once the connection is established,
 * the YtsClient::authenticated signal will be emitted.
 *
 * NB: this function long name is to avoid collision with the GIR singnal
 *     connection method.
 */
void
yts_client_connect (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  YTS_NOTE (CONNECTION, "Connecting ...");

  g_return_if_fail (YTS_IS_CLIENT (self));

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
(tp_cli_connection_callback_for_connect) yts_client_connected_cb,
                                      self,
                                      NULL,
                                      (GObject*)self);
    }
  else if (!priv->dialing)
    yts_client_make_connection (self);
}

static gboolean
yts_client_has_capability (YtsClient *self, YtsCaps cap)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  unsigned i;

  if (!priv->caps)
    return FALSE;

  for (i = 0; i < priv->caps->len; ++i)
    {
      YtsCaps c = g_array_index (priv->caps, YtsCaps, i);

      if (c == cap || c == YTS_CAPS_CONTROL)
        return TRUE;
    }

  return FALSE;
}

static void
yts_client_refresh_roster (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  YTS_NOTE (CLIENT, "Refreshing roster");

  if (!priv->tp_status)
    return;

  yts_roster_clear (priv->roster);
  yts_roster_clear (priv->unwanted);

  yts_client_process_status (self);
}

/**
 * yts_client_add_capability:
 * @self: #YtsClient,
 * @capability: Name of the capability.
 *
 * Adds a capability to the capability set of this client; multiple capabilities
 * can be added by making mulitiple calls to this function.
 *
 * The capability set is used to filter roster items to match.
 */
void
yts_client_add_capability (YtsClient  *self,
                           char const *capability)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  GQuark cap_quark;

  /* FIXME check that there's no collision with service owned capabilities. */

  g_return_if_fail (YTS_IS_CLIENT (self));

  cap_quark = g_quark_from_string (capability);

  if (yts_client_has_capability (self, cap_quark))
    {
      YTS_NOTE (CLIENT, "Capablity '%s' already set", capability);
      return;
    }

  if (priv->tp_client)
    tp_yts_client_add_capability (priv->tp_client, capability);

  if (!priv->caps)
    priv->caps = g_array_sized_new (FALSE, FALSE, sizeof (YtsCaps), 1);

  g_array_append_val (priv->caps, cap_quark);

  yts_client_refresh_roster (self);
}

/**
 * yts_client_get_roster:
 * @self: #YtsClient
 *
 * Gets the #YtsRoster for this client. The object is owned by the client
 * and must not be freed by the caller.
 *
 * Return value (tranfer none): #YtsRoster.
 */
YtsRoster *
yts_client_get_roster (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CLIENT (self), NULL);

  return priv->roster;
}

/**
 * yts_client_emit_error:
 * @self: #YtsClient,
 * @error: #YtsError
 *
 * Emits the #YtsClient::error signal with the suplied error parameter.
 *
 * This function is intened primarily for internal use, but can be also used by
 * toolkit libraries that need to generate asynchronous errors. Any function
 * call that returns the %YTS_ERROR_PENDING code to the caller should
 * eventually lead to emission of the ::error signal with either an appropriate
 * error code or %YTS_ERROR_SUCCESS to indicate the operation successfully
 * completed.
 */
void
yts_client_emit_error (YtsClient *self, YtsError error)
{
  g_return_if_fail (YTS_IS_CLIENT (self));

  /*
   * There is no point in throwing an error that has no atom specified.
   */
  g_return_if_fail (yts_error_get_atom (error));

  g_signal_emit (self, signals[ERROR], 0, error);
}

/**
 * yts_client_set_incoming_file_directory:
 * @self: #YtsClient
 * @directory: path to a directory or %NULL.
 *
 * Sets the directory where incoming files will be stored; if the provided path
 * is %NULL, the directory will be reset to the default (~/.Ytstenut/). This
 * function does not do any checks regarding validity of the path provided,
 * though an attempt to create the directory before it is used, with permissions
 * of 0700.
 *
 * To change the directory for a specific file call this function from a
 * callback to the YtsClient::incoming-file signal.
 */
void
yts_client_set_incoming_file_directory (YtsClient *self,
                                         char const *directory)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  g_return_if_fail (YTS_IS_CLIENT (self));

  if (!directory || !*directory)
    priv->incoming_dir =
      g_build_filename (g_get_home_dir (), ".ytstenut", NULL);
  else
    priv->incoming_dir = g_strdup (directory);
}

/**
 * yts_client_get_incoming_file_directory:
 * @self: #YtsClient
 *
 * Returns the directory into which any files from incoming file transfers will
 * be placed.
 *
 * Return value: (tranfer none): directory where incoming files are stored.
 */
char const *
yts_client_get_incoming_file_directory (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CLIENT (self), NULL);

  return priv->incoming_dir;
}

/**
 * yts_client_get_jid:
 * @self: #YtsClient
 *
 * Returns the jabber id associated with the current client.
 *
 * Return value: the jabber id.
 */
char const *
yts_client_get_jid (const YtsClient *self)
{
  g_warning (G_STRLOC ": NOT IMPLEMENTED !!!");

  return NULL;
}

/**
 * yts_client_get_uid:
 * @self: #YtsClient
 *
 * Returns uid of the service this client represents.
 *
 * Return value: the service uid.
 */
char const *
yts_client_get_uid (const YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CLIENT (self), NULL);

  return priv->uid;
}

TpConnection *
yts_client_get_connection (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CLIENT (self), NULL);

  return priv->connection;
}

TpYtsStatus *
yts_client_get_tp_status (YtsClient *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CLIENT (self), NULL);

  return priv->tp_status;
}

/**
 * yts_client_set_status:
 * @self: #YtsClient
 * @status: new #YtsStatus
 *
 * Changes the status of the service represented by this client to status;
 */
static void
yts_client_set_status (YtsClient *self, YtsStatus *status)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);

  g_return_if_fail (YTS_IS_CLIENT (self));
  g_return_if_fail (YTS_IS_STATUS (status));

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
      yts_client_dispatch_status (self);
    }
}

/*
 * FIXME -- bad API, constructing the YtsStatus is hard, this should be a
 * private API, with a better public API wrapping it.
 */

/**
 * yts_client_set_status_by_capability:
 * @self: #YtsClient
 * @capability: the capability to set status for
 * @activity: the activity to set the status to.
 *
 * Changes the status of the service represented by this client to status.
 */
void
yts_client_set_status_by_capability (YtsClient *self,
                                      char const *capability,
                                      char const *activity)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  YtsStatus        *status = NULL;

  g_return_if_fail (YTS_IS_CLIENT (self) && capability);

  g_return_if_fail (priv->caps && priv->caps->len);

  if (activity)
    {
      char const   *attributes[] =
        {
          "capability",   capability,
          "activity",     activity,
          "from-service", priv->uid,
          NULL
        };

      g_debug ("Constructing status for %s, %s, %s",
               capability, activity, priv->uid);

      status = yts_status_new ((char const**)&attributes);
    }

  yts_client_set_status (self, status);
}

struct YtsCLChannelData
{
  YtsClient  *client;
  YtsContact *contact;
  GHashTable  *attrs;
  char        *xml;
  char        *uid;
  YtsError    error;
  gboolean     status_done;
  int          ref_count;
};

static void
yts_cl_channel_data_unref (struct YtsCLChannelData *d)
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

static struct YtsCLChannelData *
yts_cl_channel_data_ref (struct YtsCLChannelData *d)
{
  d->ref_count++;
  return d;
}

static void
yts_client_msg_replied_cb (TpYtsChannel *proxy,
                            GHashTable   *attributes,
                            char const   *body,
                            gpointer      data,
                            GObject      *weak_object)
{
  GHashTableIter            iter;
  gpointer                  key, value;
  struct YtsCLChannelData *d = data;

  YTS_NOTE (MESSAGE, "Got reply with attributes:");

  g_hash_table_iter_init (&iter, attributes);

  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      YTS_NOTE (MESSAGE, "    %s = %s\n",
                 (char const *) key, (char const  *) value);
    }

  YTS_NOTE (MESSAGE, "    body: %s\n", body);

  if (!d->status_done)
    {
      guint32   a;
      YtsError e;

      a = yts_error_get_atom (d->error);
      e = yts_error_make (a, YTS_ERROR_SUCCESS);

      yts_client_emit_error (d->client, e);

      d->status_done = TRUE;
    }

  yts_cl_channel_data_unref (d);
}

static void
yts_client_msg_failed_cb (TpYtsChannel *proxy,
                           guint         error_type,
                           char const   *stanza_error_name,
                           char const   *ytstenut_error_name,
                           char const   *text,
                           gpointer      data,
                           GObject      *weak_object)
{
  guint32                   a;
  YtsError                 e;
  struct YtsCLChannelData *d = data;

  a = yts_error_get_atom (d->error);

  g_warning ("Sending of message failed: type %u, %s, %s, %s",
             error_type, stanza_error_name, ytstenut_error_name, text);

  e = yts_error_make (a, YTS_ERROR_NO_MSG_CHANNEL);

  yts_client_emit_error (d->client, e);

  d->status_done = TRUE;

  yts_cl_channel_data_unref (d);
}

static void
yts_client_msg_closed_cb (TpChannel *channel,
                           gpointer   data,
                           GObject   *weak_object)
{
  struct YtsCLChannelData *d = data;

  YTS_NOTE (MESSAGE, "Channel closed");

  if (!d->status_done)
    {
      guint32   a;
      YtsError e;

      a = yts_error_get_atom (d->error);
      e = yts_error_make (a, YTS_ERROR_SUCCESS);

      yts_client_emit_error (d->client, e);

      d->status_done = TRUE;
    }

  yts_cl_channel_data_unref (d);
}

static void
yts_client_msg_request_cb (GObject      *source_object,
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
      YTS_NOTE (MESSAGE, "Channel requested");
    }

  g_clear_error (&error);
}

static void
yts_client_outgoing_channel_cb (GObject      *obj,
                                 GAsyncResult *res,
                                 gpointer      data)
{
  TpYtsChannel             *ch;
  TpYtsClient              *client = TP_YTS_CLIENT (obj);
  GError                   *error  = NULL;
  struct YtsCLChannelData *d      = data;

  if (!(ch = tp_yts_client_request_channel_finish (client, res, &error)))
    {
      guint32   a;
      YtsError e;

      a = yts_error_get_atom (d->error);

      g_warning ("Failed to open outgoing channel: %s", error->message);
      g_clear_error (&error);

      e = yts_error_make (a, YTS_ERROR_NO_MSG_CHANNEL);

      yts_client_emit_error (d->client, e);
    }
  else
    {
      YTS_NOTE (MESSAGE, "Got message channel, sending request");

      tp_yts_channel_connect_to_replied (ch, yts_client_msg_replied_cb,
                                         yts_cl_channel_data_ref (d),
                                         NULL, NULL, NULL);
      tp_yts_channel_connect_to_failed (ch, yts_client_msg_failed_cb,
                                        yts_cl_channel_data_ref (d),
                                        NULL, NULL, NULL);
      tp_cli_channel_connect_to_closed (TP_CHANNEL (ch),
                                        yts_client_msg_closed_cb,
                                        yts_cl_channel_data_ref (d),
                                        NULL, NULL, NULL);

      tp_yts_channel_request_async (ch, NULL, yts_client_msg_request_cb, NULL);
    }

  yts_cl_channel_data_unref (d);
}

static YtsError
yts_client_dispatch_message (struct YtsCLChannelData *d)
{
  TpContact         *tp_contact;
  YtsClientPrivate *priv = GET_PRIVATE (d->client);

  YTS_NOTE (CLIENT, "Dispatching delayed message to %s", d->uid);

  tp_contact = yts_contact_get_tp_contact (d->contact);
  g_assert (tp_contact);

  tp_yts_client_request_channel_async (priv->tp_client,
                                       tp_contact,
                                       d->uid,
                                       TP_YTS_REQUEST_TYPE_GET,
                                       d->attrs,
                                       d->xml,
                                       NULL,
                                       yts_client_outgoing_channel_cb,
                                       d);

  return d->error;
}

static void
yts_client_notify_tp_contact_cb (YtsContact              *contact,
                                  GParamSpec               *pspec,
                                  struct YtsCLChannelData *d)
{
  YTS_NOTE (CLIENT, "Contact ready");
  yts_client_dispatch_message (d);
  g_signal_handlers_disconnect_by_func (contact,
                                        yts_client_notify_tp_contact_cb,
                                        d);
}

YtsError
yts_client_send_message (YtsClient   *client,
                           YtsContact  *contact,
                           char const   *uid,
                           YtsMetadata *message)
{
  GHashTable               *attrs;
  struct YtsCLChannelData *d;
  YtsError                 e;
  char                     *xml = NULL;

  if (!(attrs = yts_metadata_extract (message, &xml)))
    {
      g_warning ("Failed to extract content from YtsMessage object");

      e = yts_error_new (YTS_ERROR_INVALID_PARAMETER);
      g_free (xml);
      return e;
    }

  e = yts_error_new (YTS_ERROR_PENDING);

  d              = g_new (struct YtsCLChannelData, 1);
  d->error       = e;
  d->client      = client;
  d->contact     = contact;
  d->status_done = FALSE;
  d->ref_count   = 1;
  d->attrs       = attrs;
  d->xml         = xml;
  d->uid         = g_strdup (uid);

  if (yts_contact_get_tp_contact (contact))
    {
      yts_client_dispatch_message (d);
    }
  else
    {
      YTS_NOTE (CLIENT, "Contact not ready, postponing message dispatch");

      g_signal_connect (contact, "notify::tp-contact",
                        G_CALLBACK (yts_client_notify_tp_contact_cb),
                        d);
    }

  return e;
}

static void
_adapter_error (YtsServiceAdapter  *adapter,
                char const          *invocation_id,
                GError const        *error,
                YtsClient          *self)
{
  YtsMetadata *message;

  message = yts_error_message_new (g_quark_to_string (error->domain),
                                    error->code,
                                    error->message,
                                    invocation_id);

  // TODO
  g_debug ("%s() not implemented at %s", __FUNCTION__, G_STRLOC);

  g_object_unref (message);
}

static void
_adapter_event (YtsServiceAdapter  *adapter,
                char const          *aspect,
                GVariant            *arguments,
                YtsClient          *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  YtsMetadata *message;
  ProxyList   *proxy_list;
  char        *fqc_id;

  fqc_id = yts_service_adapter_get_fqc_id (adapter);
  message = yts_event_message_new (fqc_id, aspect, arguments);

  /* Dispatch to all registered proxies. */
  proxy_list = g_hash_table_lookup (priv->proxies, fqc_id);
  if (proxy_list) {
    GList const *iter;
    for (iter = proxy_list->list; iter; iter = iter->next) {
      ProxyData const *proxy_data = (ProxyData const *) iter->data;
      yts_client_send_message (self,
                                 YTS_CONTACT (proxy_data->contact),
                                 proxy_data->proxy_id,
                                 message);
    }
  }
  g_free (fqc_id);
  g_object_unref (message);
}

static void
_adapter_response (YtsServiceAdapter *adapter,
                   char const         *invocation_id,
                   GVariant           *return_value,
                   YtsClient         *self)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  InvocationData  *invocation;
  YtsMetadata    *message;
  char            *fqc_id;

  invocation = g_hash_table_lookup (priv->invocations, invocation_id);
  if (NULL == invocation) {
    // FIXME report error
    g_critical ("%s : Data not found to respond to invocation %s",
                G_STRLOC,
                invocation_id);
  }

  fqc_id = yts_service_adapter_get_fqc_id (adapter);
  message = yts_response_message_new (fqc_id,
                                       invocation_id,
                                       return_value);
  yts_client_send_message (self,
                             invocation->contact,
                             invocation->proxy_id,
                             message);
  g_object_unref (message);
  g_free (fqc_id);

  client_conclude_invocation (self, invocation_id);
}

static void
_service_destroyed (ServiceData *data,
                    void        *stale_service_ptr)
{
  YtsClientPrivate *priv = GET_PRIVATE (data->client);

  g_hash_table_remove (priv->services, data->capability);
  service_data_destroy (data);
}

/*
 * TODO add GError reporting
 * The client does not take ownership of the service, it will be
 * unregistered upon destruction.
 */
bool
yts_client_register_service (YtsClient      *self,
                              YtsCapability  *service)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  YtsServiceAdapter   *adapter;
  YtsProfileImpl      *profile_impl;
  ServiceData          *service_data;
  char                **fqc_ids;
  unsigned              i;
  YtsAdapterFactory   *const factory = yts_adapter_factory_get_default ();

  g_return_val_if_fail (YTS_IS_CLIENT (self), FALSE);
  g_return_val_if_fail (YTS_IS_CAPABILITY (service), FALSE);

  fqc_ids = yts_capability_get_fqc_ids (service);

  /* Check that capabilities are not implemented yet. */
  for (i = 0; fqc_ids[i] != NULL; i++) {

    adapter = g_hash_table_lookup (priv->services, fqc_ids[i]);
    if (adapter)
      {
        g_critical ("%s : Service for capability %s already registered",
                    G_STRLOC,
                    fqc_ids[i]);
        g_strfreev (fqc_ids);
        return FALSE;
      }
  }

  /* Hook up the service */
  for (i = 0; fqc_ids[i] != NULL; i++) {

    adapter = yts_adapter_factory_create_adapter (factory, service, fqc_ids[i]);
    g_return_val_if_fail (adapter, FALSE);

    service_data = service_data_create (self, fqc_ids[i]);
    g_object_weak_ref (G_OBJECT (service),
                       (GWeakNotify) _service_destroyed,
                       service_data);

    g_signal_connect (adapter, "error",
                      G_CALLBACK (_adapter_error), self);
    g_signal_connect (adapter, "event",
                      G_CALLBACK (_adapter_event), self);
    g_signal_connect (adapter, "response",
                      G_CALLBACK (_adapter_response), self);

    /* Hash table takes adapter reference */
    g_hash_table_insert (priv->services,
                         g_strdup (fqc_ids[i]),
                         adapter);
    yts_client_add_capability (self, fqc_ids[i]);

    /* Keep the proxy management service up to date. */
    adapter = g_hash_table_lookup (priv->services, YTS_PROFILE_FQC_ID);
    if (NULL == adapter) {
      profile_impl = yts_profile_impl_new (self);
      adapter = g_object_new (YTS_TYPE_PROFILE_ADAPTER,
                              "service", profile_impl,
                              NULL);
      g_hash_table_insert (priv->services,
                           g_strdup (YTS_PROFILE_FQC_ID),
                           adapter);

      g_signal_connect (adapter, "error",
                        G_CALLBACK (_adapter_error), self);
      g_signal_connect (adapter, "event",
                        G_CALLBACK (_adapter_event), self);
      g_signal_connect (adapter, "response",
                        G_CALLBACK (_adapter_response), self);

    } else {
      profile_impl = YTS_PROFILE_IMPL (
                        yts_service_adapter_get_service (adapter));
      /* Not nice, but it's still referenced by the adapter. */
      g_object_unref (profile_impl);
    }

    yts_profile_impl_add_capability (profile_impl, fqc_ids[i]);
  }

  g_strfreev (fqc_ids);

  return TRUE;
}

void
yts_client_cleanup_contact (YtsClient         *self,
                             YtsContact const  *contact)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  GHashTableIter   iter;
  bool             start_over;

  /*
   * Clear pending responses.
   */

  // FIXME this would be better solved using g_hash_table_foreach_remove().
  do {
    char const *invocation_id;
    InvocationData *data;
    start_over = false;
    g_hash_table_iter_init (&iter, priv->invocations);
    while (g_hash_table_iter_next (&iter,
                                   (void **) &invocation_id,
                                   (void **) &data)) {

      if (data->contact == contact) {
        g_hash_table_remove (priv->invocations, invocation_id);
        start_over = true;
        break;
      }
    }
  } while (start_over);

  /*
   * Unregister proxies
   */

  // FIXME this would be better solved using g_hash_table_foreach_remove().
  do {
    char const *capability;
    ProxyList *proxy_list;
    start_over = false;
    g_hash_table_iter_init (&iter, priv->proxies);
    while (g_hash_table_iter_next (&iter,
                                   (void **) &capability,
                                   (void **) &proxy_list)) {

      proxy_list_purge_contact (proxy_list, contact);
      if (proxy_list_is_empty (proxy_list)) {
        g_hash_table_remove (priv->proxies, capability);
        start_over = true;
        break;
      }
    }
  } while (start_over);
}

void
yts_client_cleanup_service (YtsClient   *self,
                             YtsService  *service)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  char const      *service_id;
  GHashTableIter   iter;
  bool             start_over;

  service_id = yts_service_get_service_id (service);

  /*
   * Clear pending responses.
   */

  // FIXME this would be better solved using g_hash_table_foreach_remove().
  do {
    char const *invocation_id;
    InvocationData *data;
    start_over = false;
    g_hash_table_iter_init (&iter, priv->invocations);
    while (g_hash_table_iter_next (&iter,
                                   (void **) &invocation_id,
                                   (void **) &data)) {

      if (0 == g_strcmp0 (data->proxy_id, service_id)) {
        g_hash_table_remove (priv->invocations, invocation_id);
        start_over = true;
        break;
      }
    }
  } while (start_over);

  /*
   * Unregister proxies
   */

  // FIXME this would be better solved using g_hash_table_foreach_remove().
  do {
    char const *capability;
    ProxyList *proxy_list;
    start_over = false;
    g_hash_table_iter_init (&iter, priv->proxies);
    while (g_hash_table_iter_next (&iter,
                                   (void **) &capability,
                                   (void **) &proxy_list)) {

      proxy_list_purge_proxy_id (proxy_list, service_id);
      if (proxy_list_is_empty (proxy_list)) {
        g_hash_table_remove (priv->proxies, capability);
        start_over = true;
        break;
      }
    }
  } while (start_over);
}

bool
yts_client_get_invocation_proxy (YtsClient   *self,
                                  char const   *invocation_id,
                                  YtsContact **contact,
                                  char const  **proxy_id)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  InvocationData const *invocation;

  g_return_val_if_fail (YTS_IS_CLIENT (self), false);
  g_return_val_if_fail (contact, false);
  g_return_val_if_fail (proxy_id, false);

  invocation = g_hash_table_lookup (priv->invocations, invocation_id);
  g_return_val_if_fail (invocation, false);

  *contact = invocation->contact;
  *proxy_id = invocation->proxy_id;

  return true;
}

GVariant *
yts_client_register_proxy (YtsClient  *self,
                            char const  *capability,
                            YtsContact *contact,
                            char const  *proxy_id)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  ProxyList           *proxy_list;
  YtsServiceAdapter  *adapter = NULL;
  GVariant            *properties = NULL;

  g_return_val_if_fail (YTS_IS_CLIENT (self), false);

  proxy_list = g_hash_table_lookup (priv->proxies, capability);
  if (NULL == proxy_list) {
    proxy_list = proxy_list_create_with_proxy (contact, proxy_id);
    g_hash_table_insert (priv->proxies,
                         g_strdup (capability),
                         proxy_list);
  } else {
    proxy_list_ensure_proxy (proxy_list, contact, proxy_id);
  }


  /* This is a bit of a hack but we're returning the collected
   * object properties as response to the register-proxy invocation. */
  adapter = g_hash_table_lookup (priv->services, capability);
  if (adapter) {
    properties = yts_service_adapter_collect_properties (adapter);

  } else {

    g_critical ("%s : Could not find adapter for capability %s",
                G_STRLOC,
                capability);
  }

  return properties;
}

bool
yts_client_unregister_proxy (YtsClient  *self,
                              char const  *capability,
                              char const  *proxy_id)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  ProxyList *proxy_list;

  g_return_val_if_fail (YTS_IS_CLIENT (self), false);

  proxy_list = g_hash_table_lookup (priv->proxies, capability);
  if (NULL == proxy_list) {
    g_warning ("%s : No proxy for %s:%s",
               G_STRLOC,
               proxy_id,
               capability);
    return false;
  }

  proxy_list_purge_proxy_id (proxy_list, proxy_id);
  if (proxy_list_is_empty (proxy_list)) {
    g_hash_table_remove (priv->proxies, capability);
  }

  return true;
}

void
yts_client_foreach_service (YtsClient                 *self,
                            YtsClientServiceIterator   callback,
                            void                      *user_data)
{
  YtsClientPrivate *priv = GET_PRIVATE (self);
  GHashTableIter     iter;
  char const        *fqc_id;
  YtsServiceAdapter *adapter;

  g_return_if_fail (YTS_IS_CLIENT (self));
  g_return_if_fail (callback);

  g_hash_table_iter_init (&iter, priv->services);
  while (g_hash_table_iter_next (&iter,
                                 (void **) &fqc_id,
                                 (void **) &adapter)) {
    YtsCapability *capability = yts_service_adapter_get_service (adapter);
    callback (self, fqc_id, capability, user_data);
  }
}

