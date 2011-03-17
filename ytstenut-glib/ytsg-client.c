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
#include "ytsg-roster.h"
#include "ytsg-types.h"
#include "ytsg-debug.h"
#include "ytsg-private.h"

#include <string.h>
#include <telepathy-glib/telepathy-glib.h>
#include <telepathy-glib/connection-manager.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/connection.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/channel.h>
#include <telepathy-glib/util.h>
#include <telepathy-glib/contact.h>
#include <telepathy-glib/debug.h>
#include <telepathy-glib/proxy-subclass.h>

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

G_DEFINE_TYPE (YtsgClient, ytsg_client, G_TYPE_OBJECT);

#define YTSG_CLIENT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_CLIENT, YtsgClientPrivate))

struct _YtsgClientPrivate
{
  YtsgRoster *roster;    /* the roster of this client */
  YtsgRoster *unwanted;  /* roster of unwanted items */
  YtsgStatus *status;    /* the nScreen status of this client */
  GArray     *caps;

  char       *jid;

  char       *mgr_name;
  guint       port;
  guint       port_manual;

  YtsgProtocol  protocol;

  char       *icon_token;
  char       *icon_mime_type;
  GArray     *icon_data;

  TpDBusDaemon         *dbus;
  TpConnectionManager  *mgr;
  TpConnection         *connection;
  TpChannel            *mydevices;
  TpChannel            *subscriptions;
  TpProxy              *debug_proxy;

  /* callback ids */
  guint reconnect_id;

  guint authenticated   : 1; /* set once authenticated */
  guint ready           : 1; /* set once TP set up is done */
  guint connect         : 1; /* whether we should connect once we get our conn */
  guint reconnect       : 1; /* whether we should attempt to reconnect */
  guint dialing         : 1; /* whether in process of acquiring connection */
  guint members_pending : 1; /* whether we need to query when ready */
  guint prepared        : 1; /* whether connection features are set up */
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
  PROP_ROSTER,
  PROP_JID,
  PROP_RESOURCE,
  PROP_ICON_TOKEN,
  PROP_PROTOCOL
};

static guint signals[N_SIGNALS] = {0};

static void
ytsg_client_presence_cb (TpContact    *tp_contact,
                         guint         type,
                         gchar        *status,
                         gchar        *message,
                         YtsgContact  *contact)
{
  YTSG_NOTE (CLIENT, "Presence for %s changed: %s [%s]",
             tp_contact_get_identifier (tp_contact), status, message);

  YtsgPresence  presence;

  switch (type)
    {
    case TP_CONNECTION_PRESENCE_TYPE_AVAILABLE:
      presence = YTSG_PRESENCE_AVAILABLE;
      break;
    default:
      presence = YTSG_PRESENCE_UNAVAILABLE;
    }
}

static void
ytsg_client_contact_status_cb (YtsgContact *item,
                               GParamSpec  *pspec,
                               YtsgClient  *client)
{
  YtsgClientPrivate *priv = client->priv;
  gboolean           wanted = TRUE;
  gboolean           in_r;
  gboolean           in_u;
  const YtsgStatus  *status;

  if (!priv->caps)
    return;

  wanted = FALSE;

  /* FIXME -- this is tied to caps being advertised via status, which is not
   *  the case ... hook into new API.
   */
#if 0
  if ((status = ytsg_contact_get_status (item)) && status->caps)
    {
      int j;

      for (j = 0; j < status->caps->len; ++j)
        {
          YtsgCapsTupple *t = g_array_index (status->caps, YtsgCapsTupple*, j);

          if (t && ((t->capability == YTSG_CAPS_CONTROL) ||
                    ytsg_client_has_capability (client, t->capability)))
            {
              wanted = TRUE;
              break;
            }
        }
    }
#else
  wanted = TRUE;
#endif

  in_r  = _ytsg_roster_contains_contact (priv->roster, item);
  in_u  = _ytsg_roster_contains_contact (priv->unwanted, item);

  g_assert ((in_r && !in_u) || (in_u && !in_r));

  if (!wanted)
    {
      if (in_r)
        {
          g_object_ref (item);
          _ytsg_roster_remove_contact (priv->roster, item, FALSE);
        }

      if (!in_u)
        _ytsg_roster_add_contact (priv->unwanted, item);
    }
  else
    {
      if (in_u)
        {
          g_object_ref (item);
          _ytsg_roster_remove_contact (priv->unwanted, item, FALSE);
        }

      if (!in_r)
        _ytsg_roster_add_contact (priv->roster, item);
    }
}

static void
ytsg_client_contacts_cb (TpConnection      *connection,
                         guint              n_contacts,
                         TpContact * const *contacts,
                         guint              n_failed,
                         const TpHandle    *failed,
                         const GError      *error,
                         gpointer           data,
                         GObject           *weak_object)
{
  YtsgClient        *client = data;
  YtsgClientPrivate *priv = client->priv;
  YtsgRoster        *roster = priv->roster;
  int              i;

  if (error)
    {
      g_warning (G_STRLOC ": %s: %s", __FUNCTION__, error->message);
      return;
    }

  for (i = 0; i < n_contacts; ++i)
    {
      TpContact    *cont = contacts[i];
      YtsgContact *found_r = NULL;
      YtsgContact *found_u = NULL;
      YtsgStatus  *stat = NULL; /* FIXME */

      const char   *msg = tp_contact_get_presence_message (cont);
      TpHandle      h = tp_contact_get_handle (cont);
      YtsgPresence    presence;
      gboolean      wanted = TRUE;

      if (tp_contact_get_presence_type (cont) ==
          TP_CONNECTION_PRESENCE_TYPE_AVAILABLE)
        {
          presence = YTSG_PRESENCE_AVAILABLE;
        }
      else
        {
          presence = YTSG_PRESENCE_UNAVAILABLE;
        }

#if 0
      /* FIXME */
      stat = ytsg_status_new_from_message (presence, msg);

      /* FIXME */

      /*
       * If the client has caps set, we filter the roster by those caps.
       */
      if (priv->caps)
        {
          int j;

          wanted = FALSE;

          if (stat->caps)
            {
              for (j = 0; j < stat->caps->len; ++j)
                {
                  YtsgCapsTupple *t;

                  t = g_array_index (stat->caps, YtsgCapsTupple*, j);

                  if (t && ((t->capability == YTSG_CAPS_CONTROL) ||
                            ytsg_client_has_capability (client, t->capability)))
                    {
                      wanted = TRUE;
                      break;
                    }
                }
            }
        }
#endif

      if (!(found_r = _ytsg_roster_find_contact_by_handle (roster, h)))
        found_u = _ytsg_roster_find_contact_by_handle (priv->unwanted, h);

      if (found_r && !wanted)
        {
          YTSG_NOTE (CLIENT, "Removing unwanted contact %s from roster: %s [%s]",
                   tp_contact_get_identifier (cont),
                   tp_contact_get_presence_status (cont),
                   msg);

          g_object_ref (found_r);
          _ytsg_roster_remove_contact (roster, found_r, FALSE);
          _ytsg_roster_add_contact (priv->unwanted, found_r);
        }
      else if (!found_r && !found_u)
        {
          YtsgContact   *item;
          YtsgSubscription  subs = YTSG_SUBSCRIPTION_NONE;

          YTSG_NOTE (CLIENT, "Adding contact %s to roster %s: %s [%s]",
                   tp_contact_get_identifier (cont),
                   wanted ? "wanted" : "unwanted",
                   tp_contact_get_presence_status (cont),
                   msg);

          /*
           * The subscription status has to be worked out from the membership
           * of the subscription channel.
           */
          if (priv->subscriptions)
            {
              const TpIntSet *m;
              TpHandle        h = tp_contact_get_handle (cont);

              m = tp_channel_group_get_members (priv->subscriptions);

              if (tp_intset_is_member (m, h))
		subs = YTSG_SUBSCRIPTION_APPROVED;
              else
                {
                  m = tp_channel_group_get_remote_pending (priv->subscriptions);

                  if (tp_intset_is_member (m, h))
                    subs |= YTSG_SUBSCRIPTION_PENDING_OUT;
                  else
                    {
                      m = tp_channel_group_get_remote_pending (priv->subscriptions);

                      if (tp_intset_is_member (m, h))
                        subs |= YTSG_SUBSCRIPTION_PENDING_IN;
                    }
                }
            }

          if (priv->protocol == YTSG_PROTOCOL_LOCAL_XMPP)
            subs = YTSG_SUBSCRIPTION_APPROVED;

          item = g_object_new (YTSG_TYPE_CONTACT,
                               "client",       client,
                               "contact",      cont,
                               "group",        "MyDevices",
                               "status",       stat,
                               "subscription", (guint)subs,
                               NULL);

          if (wanted)
            _ytsg_roster_add_contact (roster, item);
          else
            _ytsg_roster_add_contact (priv->unwanted, item);

          /* FIXME -- status was used to advertise caps, that is no longer
           * the case, so replace this ...
           */
          g_signal_connect (item, "notify::status",
                            G_CALLBACK (ytsg_client_contact_status_cb),
                            client);

          tp_g_signal_connect_object (cont, "presence-changed",
                                      G_CALLBACK (ytsg_client_presence_cb),
                                      item, 0);
        }

      if (stat)
        g_object_unref (stat);
    }
}

static void
ytsg_client_group_members_cb (TpChannel *self,
                            gchar     *message,
                            GArray    *added,          /* guint */
                            GArray    *removed,        /* guint */
                            GArray    *local_pending,  /* guint */
                            GArray    *remote_pending, /* guint */
                            guint      actor,
                            guint      reason,
                            gpointer   data)
{
  YtsgClient        *client = data;
  YtsgClientPrivate *priv = client->priv;
  YtsgRoster        *roster;
  int              i;
  TpContactFeature features[] = { TP_CONTACT_FEATURE_PRESENCE,
                                  TP_CONTACT_FEATURE_CONTACT_INFO,
                                  TP_CONTACT_FEATURE_AVATAR_DATA,
                                  TP_CONTACT_FEATURE_CAPABILITIES};

  /*
   * NB: this function can be called with no changes at all if the roster is
   *     empty.
   */
  roster = priv->roster;

  if (!priv->connection)
    {
      YTSG_NOTE (CLIENT, "Got members-changed signal after disconnection");
      return;
    }

  YTSG_NOTE (CLIENT, "Members changed on channel %s",
           self == priv->mydevices ? "MyDevices" :
           (self == priv->subscriptions ? "subscriptions" : "unknown"));

  if (priv->protocol == YTSG_PROTOCOL_XMPP)
    {
      if (self == priv->mydevices)
        {
          if (removed)
            for (i = 0; i < removed->len; ++i)
              {
                guint handle = g_array_index (removed, guint, i);

                _ytsg_roster_remove_contact_by_handle (roster, handle);
              }

          if (added && added->len)
            {
              if (!tp_connection_is_ready (priv->connection))
                {
                  priv->members_pending = TRUE;
                  YTSG_NOTE (CLIENT, "Connection not ready, postponing members");
                }
              else
                tp_connection_get_contacts_by_handle (priv->connection,
                                            added->len,
                                            (const TpHandle *)added->data,
                                            G_N_ELEMENTS (features),
                                            (const TpContactFeature *)&features,
                                            ytsg_client_contacts_cb,
                                            client,
                                            NULL,
                                            (GObject*)client);
            }
        }
      else if (self == priv->subscriptions)
        {
          g_debug ("Subscriptions: rm %d, add %d, r_pend %d, l_pend %d",
                   removed ? removed->len : 0,
                   added ? added->len : 0,
                   remote_pending ? remote_pending->len : 0,
                   local_pending ? local_pending->len : 0);

          if (removed)
            for (i = 0; i < removed->len; ++i)
              {
                guint         handle = g_array_index (removed, guint, i);
                YtsgContact *item;

                if ((item = _ytsg_roster_find_contact_by_handle (roster, handle)))
                  _ytsg_contact_set_subscription (item, YTSG_SUBSCRIPTION_NONE);
              }

          if (added)
            for (i = 0; i < added->len; ++i)
              {
                guint         handle = g_array_index (added, guint, i);
                YtsgContact *item;

                if ((item = _ytsg_roster_find_contact_by_handle (roster, handle)))
                  _ytsg_contact_set_subscription (item,
                                                  YTSG_SUBSCRIPTION_APPROVED);
              }

          if (remote_pending)
            for (i = 0; i < remote_pending->len; ++i)
              {
                guint         handle = g_array_index (remote_pending, guint, i);
                YtsgContact *item;

                if ((item = _ytsg_roster_find_contact_by_handle (roster, handle)))
                  _ytsg_contact_set_subscription (item,
                                                  YTSG_SUBSCRIPTION_PENDING_OUT);
              }

          if (local_pending)
            for (i = 0; i < local_pending->len; ++i)
              {
                guint         handle = g_array_index (local_pending, guint, i);
                YtsgContact *item;

                if ((item = _ytsg_roster_find_contact_by_handle (roster, handle)))
                  _ytsg_contact_set_subscription (item,
                                                  YTSG_SUBSCRIPTION_PENDING_IN);
              }
        }
    }
  else if (priv->protocol == YTSG_PROTOCOL_LOCAL_XMPP)
    {
      if (self == priv->subscriptions)
        {
          if (removed)
            for (i = 0; i < removed->len; ++i)
              {
                guint handle = g_array_index (removed, guint, i);

                _ytsg_roster_remove_contact_by_handle (roster, handle);
              }

          if (added && added->len)
            {
              if (!tp_connection_is_ready (priv->connection))
                {
                  priv->members_pending = TRUE;
                  YTSG_NOTE (CLIENT,
                           "Connection not ready, postponing members");
                }
              else
                tp_connection_get_contacts_by_handle (priv->connection,
                                            added->len,
                                            (const TpHandle *)added->data,
                                            G_N_ELEMENTS (features),
                                            (const TpContactFeature *)&features,
                                            ytsg_client_contacts_cb,
                                            client,
                                            NULL,
                                            (GObject*)client);
            }
        }
    }
}

static void
ytsg_client_channel_prepare_cb (GObject      *channel,
                                GAsyncResult *res,
                                gpointer      data)
{
  YtsgClient        *client = data;
  YtsgClientPrivate *priv   = client->priv;
  GError            *error  = NULL;

  if (!tp_proxy_prepare_finish (channel, res, &error))
    {
      g_critical ("Failed to prepare channel %s: %s",
                  (void*)channel == priv->subscriptions ? "subscriptions" :
                  "MyDevices",
                  error && error->message ? error->message : "unknown error");
    }
  else
    {
      /*
       * If we are not in ready state, emit the ready signal.
       */
      if (!priv->ready)
        g_signal_emit (client, signals[READY], 0);
    }
}

static void
ytsg_client_channel_cb (TpConnection *proxy,
                        const gchar *path,
                        const gchar *type,
                        guint handle_type,
                        guint handle,
                        gboolean suppress_handle,
                        gpointer data,
                        GObject *weak_object)
{
  YtsgClient        *client = data;
  YtsgClientPrivate *priv = client->priv;

  if (!path)
    {
      g_warning (G_STRLOC ":%s: no path!", __FUNCTION__);
      return;
    }

  /* YTSG_NOTE (CLIENT, "New channel: %s: %s: h type %d, h %d", */
  /*          path, type, handle_type, handle); */

  switch (handle_type)
    {
    case TP_HANDLE_TYPE_CONTACT:
      /* FIXME -- this is where the messaging channel will go */
      break;
    case TP_HANDLE_TYPE_LIST:
      /*
       * decide if we care about the subscription channel, or the MyDevices
       * group only
       */

      /*
       * There is probably a better way, but querrying the channel props
       * requires yet another async call, so we just check the path ends in
       * 'subscribe'
       */
      if (!strcmp (path + (strlen (path) - strlen ("subscribe")), "subscribe"))
        {
          GError    *error = NULL;
          TpChannel *ch;
          GQuark     features[] = { TP_CHANNEL_FEATURE_GROUP, 0};

          ch = tp_channel_new (proxy, path, type, handle_type, handle, &error);

          priv->subscriptions = ch;

          if (error)
            {
              g_warning (G_STRLOC ": %s: %s", __FUNCTION__, error->message);
              g_clear_error (&error);

              /*
               * This is pretty bad, the subscription channel is essential.
               */
              ytsg_client_disconnect_from_mesh (client);
              _ytsg_client_reconnect_after (client, RECONNECT_DELAY);
              return;
            }

          tp_g_signal_connect_object (ch, "group-members-changed",
                                      G_CALLBACK (ytsg_client_group_members_cb),
                                      client,
                                      0);

          tp_proxy_prepare_async (ch, features,
                                  ytsg_client_channel_prepare_cb, client);
        }
      break;
    case TP_HANDLE_TYPE_GROUP:
      if (!strcmp (path + (strlen (path) - strlen ("MyDevices")), "MyDevices"))
        {
          GError    *error = NULL;
          TpChannel *ch;
          GQuark     features[] = { TP_CHANNEL_FEATURE_GROUP, 0};

          YTSG_NOTE (CLIENT, "MyDevices channel");

          ch = tp_channel_new (proxy, path, type, handle_type, handle, &error);

          if (error)
            {
              g_warning (G_STRLOC ": %s: %s", __FUNCTION__, error->message);
              g_clear_error (&error);

              /*
               * This is pretty bad, the MyDevices channel is where we get the
               * rosters from.
               */
              ytsg_client_disconnect_from_mesh (client);
              _ytsg_client_reconnect_after (client, RECONNECT_DELAY);
              return;
            }

          priv->mydevices = ch;

          tp_g_signal_connect_object (ch, "group-members-changed",
                                      G_CALLBACK (ytsg_client_group_members_cb),
                                      client,
                                      0);

          tp_proxy_prepare_async (ch, features,
                                  ytsg_client_channel_prepare_cb, client);
        }
      break;
    default:;
    }
}

static void
ytsg_client_class_init (YtsgClientClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgClientPrivate));

  object_class->dispose      = ytsg_client_dispose;
  object_class->finalize     = ytsg_client_finalize;
  object_class->constructed  = ytsg_client_constructed;
  object_class->get_property = ytsg_client_get_property;
  object_class->set_property = ytsg_client_set_property;
}

static void
ytsg_client_constructed (GObject *object)
{
  YtsgClient *self = (YtsgClient*) object;

  if (G_OBJECT_CLASS (ytsg_client_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_client_parent_class)->constructed (object);
}

static void
ytsg_client_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsgClient        *self = (YtsgClient*) object;
  YtsgClientPrivate *priv = self->priv;

  switch (property_id)
    {
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
  YtsgClient        *self = (YtsgClient*) object;
  YtsgClientPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_client_init (YtsgClient *self)
{
  self->priv = YTSG_CLIENT_GET_PRIVATE (self);
}

static void
ytsg_client_dispose (GObject *object)
{
  YtsgClient        *self = (YtsgClient*) object;
  YtsgClientPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (ytsg_client_parent_class)->dispose (object);
}

static void
ytsg_client_finalize (GObject *object)
{
  YtsgClient        *self = (YtsgClient*) object;
  YtsgClientPrivate *priv = self->priv;

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
                      const gchar *arg_Error,
                      GHashTable *arg_Details,
                      gpointer user_data,
                      GObject *weak_object)
{
  YTSG_NOTE (CLIENT, "Error: %s", arg_Error);
}

static void
ytsg_client_status_cb (TpConnection *proxy,
                       guint arg_Status,
                       guint arg_Reason,
                       gpointer data,
                       GObject *weak_object)
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

static void
ytsg_client_connection_ready_cb (TpConnection *conn,
                                 GParamSpec   *par,
                                 YtsgClient     *client)
{
  YtsgClientPrivate *priv = client->priv;

  if (tp_connection_is_ready (conn))
    {
      YTSG_NOTE (CLIENT, "TP Connection entered ready state");

      if (priv->members_pending)
        {
          /* get members */
          TpContactFeature features[] = { TP_CONTACT_FEATURE_PRESENCE,
                                          TP_CONTACT_FEATURE_CONTACT_INFO,
                                          TP_CONTACT_FEATURE_AVATAR_DATA,
                                          TP_CONTACT_FEATURE_CAPABILITIES};
          const TpIntSet *set = tp_channel_group_get_members (priv->mydevices);

          if (set)
            {
              GArray *added = tp_intset_to_array (set);

              tp_connection_get_contacts_by_handle (conn,
                                          added->len,
                                          (const TpHandle *)added->data,
                                          G_N_ELEMENTS (features),
                                          (const TpContactFeature *)&features,
                                          ytsg_client_contacts_cb,
                                          client,
                                          NULL,
                                          (GObject*)client);


              g_array_free (added, TRUE);
            }

          priv->members_pending = FALSE;
        }
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
  YtsgClientPrivate *priv = client->priv;

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
ytsg_client_is_avatar_type_supported (YtsgClient   *client,
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
  GError          *error  = NULL;

  if (!tp_proxy_prepare_finish (connection, res, &error))
    {
      g_critical ("Failed to prepare info: %s", error->message);
    }
  else
    {
      TpContactInfoFlags flags;

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
      /* FIXME -- */
      /*
       * local-xmpp / salut does not support the ContactCapabilities interface,
       * but file transfer is enabled by default, so it does not matter to us.
       */
      if (priv->protocol != YTSG_PROTOCOL_LOCAL_XMPP)
        ytsg_client_setup_caps (client);
#endif
    }
}

static void
ytsg_client_connection_cb (TpConnectionManager *proxy,
                           const gchar         *bus_name,
                           const gchar         *object_path,
                           const GError        *error,
                           gpointer             data,
                           GObject             *weak_object)
{
  YtsgClient        *client  = data;
  YtsgClientPrivate *priv    = client->priv;
  GError          *myerror = NULL;
  GQuark           features[] = { TP_CONNECTION_FEATURE_CONTACT_INFO,
                                  TP_CONNECTION_FEATURE_AVATAR_REQUIREMENTS,
                                  TP_CONNECTION_FEATURE_CAPABILITIES,
                                  0 };

  priv->dialing = FALSE;

  if (error)
    {
      /*
       * We can't function without a connection, but we probably do not want
       * to abort the application.
       */
      g_warning (G_STRLOC ": %s: %s; will attempt to reconnect after %ds",
                 __FUNCTION__, error->message, RECONNECT_DELAY);

      _ytsg_client_reconnect_after (client, RECONNECT_DELAY);
      return;
    }

  priv->connection = tp_connection_new (priv->dbus,
                                        bus_name,
                                        object_path,
                                        &myerror);

  if (myerror)
    {
      g_critical (G_STRLOC ": %s: %s; no nScreen functionality will be "
                  "available", __FUNCTION__, myerror->message);
      g_clear_error (&myerror);
      return;
    }

  tp_g_signal_connect_object (priv->connection, "notify::connection-ready",
                              G_CALLBACK (ytsg_client_connection_ready_cb),
                              client, 0);

  tp_cli_connection_connect_to_connection_error (priv->connection,
                                                 ytsg_client_error_cb,
                                                 client,
                                                 NULL,
                                                 (GObject*)client,
                                                 &myerror);

  if (myerror)
    {
      g_critical (G_STRLOC ": %s: %s; no nScreen functionality will be "
                  "available", __FUNCTION__, myerror->message);
      g_clear_error (&myerror);
      return;
    }

  tp_cli_connection_connect_to_status_changed (priv->connection,
                                               ytsg_client_status_cb,
                                               client,
                                               NULL,
                                               (GObject*) client,
                                               &myerror);

  if (myerror)
    {
      g_critical (G_STRLOC ": %s: %s; no nScreen functionality will be "
                  "available", __FUNCTION__, myerror->message);
      g_clear_error (&myerror);
      return;
    }

  tp_cli_connection_connect_to_new_channel (priv->connection,
                                            ytsg_client_channel_cb,
                                            client,
                                            NULL,
                                            (GObject*)client,
                                            &myerror);

  if (myerror)
    {
      g_critical (G_STRLOC ": %s: %s; no nScreen functionality will be "
                  "available", __FUNCTION__, myerror->message);
      g_clear_error (&myerror);
      return;
    }

  tp_proxy_prepare_async (priv->connection,
                          features,
                          ytsg_client_connection_prepare_cb,
                          client);

  if (priv->connect)
    tp_cli_connection_call_connect (priv->connection,
                                    -1,
                                    ytsg_client_connected_cb,
                                    client,
                                    NULL,
                                    (GObject*)client);
}

static void
ytsg_client_make_connection (YtsgClient *client)
{
  YtsgClientPrivate *priv  = client->priv;
  GHashTable      *hash;
  const char      *proto = "jabber";

  /* FIXME */
  priv->jid =
    g_strdup_printf ("random-jid");

  if (priv->protocol == YTSG_PROTOCOL_LOCAL_XMPP)
    {
      proto = "local-xmpp";

      /* FIXME */
      hash = tp_asv_new ("jid",                G_TYPE_STRING, priv->jid,
                         "first-name",         G_TYPE_STRING, priv->jid,
                         "last-name",          G_TYPE_STRING, priv->jid,
                         "published-name",     G_TYPE_STRING, priv->jid,
                         NULL);
    }
#if 0
 else if (priv->protocol == YTSG_PROTOCOL_XMPP)
    {
      hash = tp_asv_new ("account",            G_TYPE_STRING, priv->jid,
                         "server",             G_TYPE_STRING, priv->server,
                         "port",               G_TYPE_UINT,    priv->port,
                         "password",           G_TYPE_STRING, priv->password,
                         "ignore-ssl-errors",  G_TYPE_BOOLEAN, TRUE,
                         "require-encryption", G_TYPE_BOOLEAN, TRUE,
                         "keepalive-interval", G_TYPE_UINT,     20,
                         NULL);

      if (priv->resource)
        tp_asv_set_string (hash, "resource", priv->resource);
    }
#endif
  else
    g_error ("Unknown protocol.");

  priv->dialing = TRUE;

  tp_cli_connection_manager_call_request_connection (priv->mgr,
                                                     -1,
                                                     proto,
                                                     hash,
                                                     ytsg_client_connection_cb,
                                                     client,
                                                     NULL,
                                                     (GObject*)client);

  g_hash_table_destroy (hash);
}


/**
 * ytsg_client_connect_to_mesh:
 * @client: #YtsgClient
 *
 * Initiates connection to nScreen server. Once the connection is established,
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
