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

/**
 * SECTION:yts-roster
 * @short_description: Represents a roster of devices and services
 * connected to the Ytstenut application mesh.
 *
 * #YtsRoster represents all known devices and services in the Ytstenut
 * application mesh.
 */

#include <string.h>
#include <telepathy-ytstenut-glib/telepathy-ytstenut-glib.h>

#include "yts-client-internal.h"
#include "yts-contact-internal.h"
#include "yts-debug.h"
#include "yts-marshal.h"
#include "yts-roster-internal.h"
#include "yts-service-factory.h"
#include "config.h"

static void yts_roster_dispose (GObject *object);
static void yts_roster_finalize (GObject *object);
static void yts_roster_constructed (GObject *object);
static void yts_roster_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec);
static void yts_roster_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec);

G_DEFINE_TYPE (YtsRoster, yts_roster, G_TYPE_OBJECT);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_ROSTER, YtsRosterPrivate))

typedef struct {

  GHashTable *contacts; /* hash of YtsContact this roster holds */
  YtsClient *client;   /* back-reference to the client object that owns us */

  guint disposed : 1;

} YtsRosterPrivate;

enum
{
  CONTACT_ADDED,
  CONTACT_REMOVED,

  SERVICE_ADDED,
  SERVICE_REMOVED,

  N_SIGNALS,
};

enum
{
  PROP_0,
  PROP_CLIENT,
};

static guint signals[N_SIGNALS] = {0};

static void
_contact_send_message (YtsContact   *contact,
                       YtsService   *service,
                       YtsMetadata  *message,
                       YtsRoster    *self)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  char const *service_id = yts_service_get_service_id (service);

  yts_client_send_message (priv->client,
                           contact,
                           service_id,
                           message);
}

static void
yts_roster_class_init (YtsRosterClass *klass)
{
  GParamSpec   *pspec;
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsRosterPrivate));

  object_class->dispose      = yts_roster_dispose;
  object_class->finalize     = yts_roster_finalize;
  object_class->constructed  = yts_roster_constructed;
  object_class->get_property = yts_roster_get_property;
  object_class->set_property = yts_roster_set_property;

  /**
   * YtsRoster:client:
   *
   * #YtsClient this roster represents.
   */
  pspec = g_param_spec_object ("client",
                               "YtsClient",
                               "YtsClient",
                               YTS_TYPE_CLIENT,
                               G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CLIENT, pspec);

  /**
   * YtsRoster::contact-added:
   * @self: object which emitted the signal.
   * @contact: #YtsContact that was added.
   *
   * Emitted when contact is added to the roster.
   *
   * Since: 0.1
   */
  signals[CONTACT_ADDED] =
    g_signal_new ("contact-added",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  yts_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTS_TYPE_CONTACT);

  /**
   * YtsRoster::contact-removed:
   * @self: object which emitted the signal.
   * @contact: #YtsContact that was removed.
   *
   * Emitted when contact is removed from the roster. Applications that
   * connected signal handlers to the contact, should disconnect them when this
   * signal is emitted.
   *
   * Since: 0.1
   */
  signals[CONTACT_REMOVED] =
    g_signal_new ("contact-removed",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  yts_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTS_TYPE_CONTACT);

  /**
   * YtsRoster::service-added:
   * @self: object which emitted the signal.
   * @service: #YtsService that was added.
   *
   * Emitted when service is added to the roster.
   *
   * Since: 0.1
   */
  signals[SERVICE_ADDED] =
    g_signal_new ("service-added",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  yts_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTS_TYPE_SERVICE);

  /**
   * YtsRoster::service-removed:
   * @self: object which emitted the signal.
   * @service: #YtsService that was removed.
   *
   * Emitted when service is removed from the roster. Applications that
   * connected signal handlers to the service, should disconnect them when this
   * signal is emitted.
   *
   * Since: 0.1
   */
  signals[SERVICE_REMOVED] =
    g_signal_new ("service-removed",
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  yts_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTS_TYPE_SERVICE);
}

static void
yts_roster_constructed (GObject *object)
{
  if (G_OBJECT_CLASS (yts_roster_parent_class)->constructed)
    G_OBJECT_CLASS (yts_roster_parent_class)->constructed (object);
}

static void
yts_roster_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yts_roster_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsRosterPrivate *priv = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_CLIENT:
      priv->client = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
yts_roster_init (YtsRoster *self)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);

  priv->contacts = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          g_object_unref);
}

static void
yts_roster_dispose (GObject *object)
{
  YtsRosterPrivate *priv = GET_PRIVATE (object);
  GHashTableIter     iter;
  char const        *contact_id;
  YtsContact        *contact;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  priv->client = NULL;

  g_hash_table_iter_init (&iter, priv->contacts);
  while (g_hash_table_iter_next (&iter,
                                 (void **) &contact_id,
                                 (void **) &contact)) {

    g_signal_handlers_disconnect_by_func (contact,
                                          _contact_send_message,
                                          object);
  }

  g_hash_table_destroy (priv->contacts);

  G_OBJECT_CLASS (yts_roster_parent_class)->dispose (object);
}

static void
yts_roster_finalize (GObject *object)
{
  G_OBJECT_CLASS (yts_roster_parent_class)->finalize (object);
}


/**
 * yts_roster_get_contacts:
 * @self: object on which to invoke this method.
 *
 * Returns contacts in this #YtsRoster.
 *
 * Return value: (transfer none): #GHashTable of #YtsContact; the hash table is
 * owned by the roster and must not be modified or freed by the caller.
 */
GHashTable *const
yts_roster_get_contacts (YtsRoster const *self)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_ROSTER (self), NULL);

  return priv->contacts;
}

/*
 * yts_roster_remove_service_by_id:
 * @self: object on which to invoke this method.
 * @jid: JID of the contact that the service is running on
 * @uid: the service UID.
 *
 * Removes service from a roster and emits YtsRoster::service-removed signal.
 *
 * For use by #YtsClient.
 */
void
yts_roster_remove_service_by_id (YtsRoster *self,
                                   char const *jid,
                                   char const *uid)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  YtsContact       *contact;
  gboolean           emit = FALSE;

  g_return_if_fail (YTS_IS_ROSTER (self));

  if (!(contact = (YtsContact*)yts_roster_find_contact_by_jid (self, jid)))
    {
      g_warning ("Contact for service not found");
      return;
    }

  yts_contact_remove_service_by_uid (contact, uid);

  emit = yts_contact_is_empty (contact);

  if (emit)
    {
      g_signal_handlers_disconnect_by_func (contact,
                                            _contact_send_message,
                                            self);
      g_object_ref (contact);
      g_hash_table_remove (priv->contacts, jid);
      g_signal_emit (self, signals[CONTACT_REMOVED], 0, contact);
      g_object_unref (contact);
    }
}

/*
 * yts_roster_find_contact_by_handle:
 * @roster: #YtsRoster
 * @handle: handle of this contact
 *
 * Finds contact in a roster.
 *
 * Return value: (transfer none): #YtsContact if found, or %NULL.
 */
YtsContact *
yts_roster_find_contact_by_handle (YtsRoster *self,
                                   guint handle)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  GHashTableIter     iter;
  gpointer           key, value;

  g_return_val_if_fail (YTS_IS_ROSTER (self), NULL);

  g_hash_table_iter_init (&iter, priv->contacts);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      YtsContact *contact    = value;
      TpContact   *tp_contact = yts_contact_get_tp_contact (contact);
      guint        h          = tp_contact_get_handle (tp_contact);

      if (h == handle)
        {
          return contact;
        }
    }

  return NULL;
}

/**
 * yts_roster_find_contact_by_jid:
 * @self: object on which to invoke this method.
 * @jid: jid of this contact
 *
 * Finds contact in a roster.
 *
 * Return value: (transfer none): #YtsContact if found, or %NULL.
 */
YtsContact *const
yts_roster_find_contact_by_jid (YtsRoster const *self,
                                char const      *jid)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  GHashTableIter     iter;
  gpointer           key, value;

  g_return_val_if_fail (YTS_IS_ROSTER (self) && jid, NULL);

  g_hash_table_iter_init (&iter, priv->contacts);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      YtsContact *contact = value;
      char const *j       = key;

      if (j && !strcmp (j, jid))
        {
          return contact;
        }
    }

  return NULL;
}

/*
 * yts_roster_clear:
 * @self: object on which to invoke this method.
 *
 * Removes all contacts from the roster; for each contact removed, the
 * contact-removed signal is emitted and the contact's dispose method is
 * forecefully run.
 */
void
yts_roster_clear (YtsRoster *self)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  GHashTableIter     iter;
  gpointer           key, value;

  g_return_if_fail (YTS_IS_ROSTER (self));

  // FIXME changing the hash while iterating seems not safe?!
  // also we can probably get rid of that run_dispose() once the
  // client referencing frenzy is no more.

  g_hash_table_iter_init (&iter, priv->contacts);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      YtsContact *contact = value;

      g_object_ref (contact);

      g_hash_table_iter_remove (&iter);

      g_signal_emit (self, signals[CONTACT_REMOVED], 0, contact);
      g_object_run_dispose ((GObject*)contact);

      g_object_unref (contact);
    }
}

YtsRoster*
yts_roster_new (YtsClient *client)
{
  return g_object_new (YTS_TYPE_ROSTER,
                       "client", client,
                       NULL);
}

static void
yts_roster_contact_service_removed_cb (YtsContact *contact,
                                        YtsService *service,
                                        YtsRoster  *roster)
{
  g_signal_emit (roster, signals[SERVICE_REMOVED], 0, service);
}

static void
yts_roster_contact_service_added_cb (YtsContact *contact,
                                      YtsService *service,
                                      YtsRoster  *roster)
{
  g_signal_emit (roster, signals[SERVICE_ADDED], 0, service);
}

void
yts_roster_add_service (YtsRoster           *self,
                          char const        *jid,
                          char const        *sid,
                          char const        *type,
                          char const *const *caps,
                          GHashTable        *names,
                          GHashTable        *statuses)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  YtsContact        *contact;
  YtsService        *service;
  YtsServiceFactory *factory = yts_service_factory_get_default ();

  g_return_if_fail (YTS_IS_ROSTER (self));

  if (!(contact = (YtsContact*)yts_roster_find_contact_by_jid (self, jid)))
    {
      YTS_NOTE (ROSTER, "Creating new contact for %s", jid);

      contact = yts_contact_new (priv->client, jid);

      g_signal_connect (contact, "service-added",
                        G_CALLBACK (yts_roster_contact_service_added_cb),
                        self);
      g_signal_connect (contact, "service-removed",
                        G_CALLBACK (yts_roster_contact_service_removed_cb),
                        self);

      g_hash_table_insert (priv->contacts, g_strdup (jid), contact);

      YTS_NOTE (ROSTER, "Emitting contact-added for new contact %s", jid);
      g_signal_emit (self, signals[CONTACT_ADDED], 0, contact);

      g_signal_connect (contact, "send-message",
                        G_CALLBACK (_contact_send_message), self);
    }

  YTS_NOTE (ROSTER, "Adding service %s:%s", jid, sid);

  service = yts_service_factory_create_service (factory,
                                                caps,
                                                sid,
                                                type,
                                                names,
                                                statuses);

  yts_contact_add_service (contact, service);

  g_object_unref (service);
}

void
yts_roster_update_contact_status (YtsRoster   *self,
                                  char const  *contact_id,
                                  char const  *service_id,
                                  char const  *fqc_id,
                                  char const  *status_xml)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  YtsContact *contact;

  contact = g_hash_table_lookup (priv->contacts, contact_id);
  g_return_if_fail (contact);

  yts_contact_update_service_status (contact, service_id, fqc_id, status_xml);
}

