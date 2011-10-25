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

#include <telepathy-ytstenut-glib/telepathy-ytstenut-glib.h>

#include "yts-contact-internal.h"
#include "yts-marshal.h"
#include "yts-metadata.h"
#include "yts-roster-internal.h"
#include "yts-service-factory.h"
#include "config.h"

G_DEFINE_TYPE (YtsRoster, yts_roster, G_TYPE_OBJECT);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_ROSTER, YtsRosterPrivate))

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0roster"

/**
 * SECTION: yts-roster
 * @short_description: Represents a roster of devices and services
 * connected to the Ytstenut application mesh.
 *
 * #YtsRoster represents all known devices and services in the Ytstenut
 * application mesh.
 */

enum {
  PROP_0
};

enum {
  SIG_CONTACT_ADDED,
  SIG_CONTACT_REMOVED,

  SIG_SEND_MESSAGE,

  SIG_SERVICE_ADDED,
  SIG_SERVICE_REMOVED,

  N_SIGNALS
};

typedef struct {

  GHashTable *contacts; /* hash of YtsContact this roster holds */

} YtsRosterPrivate;

static unsigned _signals[N_SIGNALS] = { 0, };

static void
_contact_send_message (YtsContact   *contact,
                       YtsService   *service,
                       YtsMetadata  *message,
                       YtsRoster    *self)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);

  g_signal_emit (self, _signals[SIG_SEND_MESSAGE], 0,
                 contact, service, message);
}

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_set_property (GObject      *object,
               unsigned      property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  YtsRosterPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsRosterPrivate *priv = GET_PRIVATE (object);

  if (priv->contacts) {

    GHashTableIter iter;
    char const *contact_id;
    YtsContact *contact;

    g_hash_table_iter_init (&iter, priv->contacts);
    while (g_hash_table_iter_next (&iter,
                                   (void **) &contact_id,
                                   (void **) &contact)) {

      g_signal_handlers_disconnect_by_func (contact,
                                            _contact_send_message,
                                            object);
    }

    g_hash_table_destroy (priv->contacts);
    priv->contacts = NULL;
  }

  G_OBJECT_CLASS (yts_roster_parent_class)->dispose (object);
}

static void
yts_roster_class_init (YtsRosterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec   *pspec;

  g_type_class_add_private (klass, sizeof (YtsRosterPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;

  /**
   * YtsRoster::contact-added:
   * @self: object which emitted the signal.
   * @contact: #YtsContact that was added.
   *
   * Emitted when contact is added to the roster.
   *
   * Since: 0.1
   */
  _signals[SIG_CONTACT_ADDED] = g_signal_new ("contact-added",
                                              G_TYPE_FROM_CLASS (object_class),
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
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
  _signals[SIG_CONTACT_REMOVED] = g_signal_new ("contact-removed",
                                                G_TYPE_FROM_CLASS (object_class),
                                                G_SIGNAL_RUN_LAST,
                                                0, NULL, NULL,
                                                yts_marshal_VOID__OBJECT,
                                                G_TYPE_NONE, 1,
                                                YTS_TYPE_CONTACT);

  /**
   * YtsContact::send-message:
   *
   * <note>Internal signal, should not be considered by users at this time.
   * Maybe in the future when we allow for custom contact subclasses.</note>
   */
  _signals[SIG_SEND_MESSAGE] = g_signal_new ("send-message",
                                         G_TYPE_FROM_CLASS (object_class),
                                         G_SIGNAL_RUN_LAST,
                                         0, NULL, NULL,
                                         yts_marshal_VOID__OBJECT_OBJECT_OBJECT,
                                         G_TYPE_NONE, 3,
                                         YTS_TYPE_CONTACT,
                                         YTS_TYPE_SERVICE,
                                         YTS_TYPE_METADATA);

  /**
   * YtsRoster::service-added:
   * @self: object which emitted the signal.
   * @service: #YtsService that was added.
   *
   * Emitted when service is added to the roster.
   *
   * Since: 0.1
   */
  _signals[SIG_SERVICE_ADDED] = g_signal_new ("service-added",
                                              G_TYPE_FROM_CLASS (object_class),
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
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
  _signals[SIG_SERVICE_REMOVED] = g_signal_new ("service-removed",
                                                G_TYPE_FROM_CLASS (object_class),
                                                G_SIGNAL_RUN_LAST,
                                                0, NULL, NULL,
                                                yts_marshal_VOID__OBJECT,
                                                G_TYPE_NONE, 1,
                                                YTS_TYPE_SERVICE);
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

/*
 * yts_roster_remove_service_by_id:
 * @self: object on which to invoke this method.
 * @contact_id: JID of the contact that the service is running on
 * @service_id: the service UID.
 *
 * Removes service from a roster and emits YtsRoster::service-removed signal.
 */
void
yts_roster_remove_service_by_id (YtsRoster  *self,
                                 char const *contact_id,
                                 char const *service_id)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  YtsContact       *contact;
  gboolean           emit = FALSE;

  g_return_if_fail (YTS_IS_ROSTER (self));

  contact = yts_roster_find_contact_by_id (self, contact_id);
  if (!contact) {
    g_critical ("Contact for service not found");
    return;
  }

  yts_contact_remove_service_by_id (contact, service_id);

  emit = yts_contact_is_empty (contact);
  if (emit) {
    g_signal_handlers_disconnect_by_func (contact,
                                          _contact_send_message,
                                          self);
    g_object_ref (contact);
    g_hash_table_remove (priv->contacts, contact_id);
    g_signal_emit (self, _signals[SIG_CONTACT_REMOVED], 0, contact);
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
yts_roster_find_contact_by_handle (YtsRoster  *self,
                                   guint       handle)
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
 * yts_roster_find_contact_by_id:
 * @self: object on which to invoke this method.
 * @contact_id: JID of this contact
 *
 * Finds contact in a roster.
 *
 * Return value: (transfer none): #YtsContact if found, or %NULL.
 */
YtsContact *const
yts_roster_find_contact_by_id (YtsRoster const  *self,
                               char const       *contact_id)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_ROSTER (self), NULL);
  g_return_val_if_fail (contact_id, NULL);

  return g_hash_table_lookup (priv->contacts, contact_id);
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

      g_signal_handlers_disconnect_by_func (contact,
                                            _contact_send_message,
                                            self);

      g_object_ref (contact);

      g_hash_table_iter_remove (&iter);

      g_signal_emit (self, _signals[SIG_CONTACT_REMOVED], 0, contact);
      g_object_run_dispose ((GObject*)contact);

      g_object_unref (contact);
    }
}

YtsRoster *
yts_roster_new (void)
{
  return g_object_new (YTS_TYPE_ROSTER, NULL);
}

static void
yts_roster_contact_service_removed_cb (YtsContact *contact,
                                        YtsService *service,
                                        YtsRoster  *roster)
{
  g_signal_emit (roster, _signals[SIG_SERVICE_REMOVED], 0, service);
}

static void
yts_roster_contact_service_added_cb (YtsContact *contact,
                                      YtsService *service,
                                      YtsRoster  *roster)
{
  g_signal_emit (roster, _signals[SIG_SERVICE_ADDED], 0, service);
}

static void
_connection_get_contacts (TpConnection        *connection,
                            guint              n_contacts,
                            TpContact *const  *contacts,
                            const char *const *requested_ids,
                            GHashTable        *failed_id_errors,
                            const GError      *error,
                            gpointer           self_,
                            GObject           *service_)
{
  YtsRoster *self = YTS_ROSTER (self_);
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  YtsService *service = YTS_SERVICE (service_);

  if (n_contacts == 0) {

    GError const *error;

    g_return_if_fail (requested_ids && requested_ids[0]);
    g_return_if_fail (failed_id_errors);

    error = g_hash_table_lookup (failed_id_errors, requested_ids[0]);
    g_critical ("%s : %s", G_STRLOC, error->message);

  } else {

    YtsContact *contact;
    char const *contact_id = requested_ids[0];
    TpContact *tp_contact = TP_CONTACT (contacts[0]);

    g_message ("Creating new contact for %s", contact_id);

    contact = yts_contact_new (tp_contact);

    g_signal_connect (contact, "service-added",
                      G_CALLBACK (yts_roster_contact_service_added_cb),
                      self);
    g_signal_connect (contact, "service-removed",
                      G_CALLBACK (yts_roster_contact_service_removed_cb),
                      self);

    g_hash_table_insert (priv->contacts, g_strdup (contact_id), contact);

    g_message ("Emitting contact-added for new contact %s", contact_id);
    g_signal_emit (self, _signals[SIG_CONTACT_ADDED], 0, contact);

    g_signal_connect (contact, "send-message",
                      G_CALLBACK (_contact_send_message), self);

    yts_contact_add_service (contact, service);
    g_object_unref (service);
  }
}

void
yts_roster_add_service (YtsRoster         *self,
                        TpConnection      *tp_connection,
                        char const        *contact_id,
                        char const        *service_id,
                        char const        *type,
                        char const *const *caps,
                        GHashTable        *names,
                        GHashTable        *statuses)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  YtsContact        *contact;
  YtsService        *service;
  YtsServiceFactory *factory = yts_service_factory_get_default ();

  service = yts_service_factory_create_service (factory,
                                                caps,
                                                service_id,
                                                type,
                                                names,
                                                statuses);

  contact = yts_roster_find_contact_by_id (self, contact_id);
  if (contact) {

    yts_contact_add_service (contact, service);

  } else {

    TpContactFeature const features[] = { TP_CONTACT_FEATURE_PRESENCE,
                                          TP_CONTACT_FEATURE_CONTACT_INFO,
                                          TP_CONTACT_FEATURE_AVATAR_DATA,
                                          TP_CONTACT_FEATURE_CAPABILITIES };

    tp_connection_get_contacts_by_id (tp_connection,
                                      1,
                                      &contact_id,
                                      G_N_ELEMENTS (features),
                                      features,
                                      _connection_get_contacts,
                                      self,
                                      NULL,
                                      G_OBJECT (service));
  }
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

/**
 * yts_roster_foreach_contact:
 * @self: object on which to invoke this method.
 * @iterator: iterator function.
 * @user_data: context to pass to the iterator function.
 *
 * Iterate over @self's contacts.
 *
 * Returns: %true if all the contacts have been iterated.
 *
 * Since: 0.4
 */
bool
yts_roster_foreach_contact (YtsRoster                 *self,
                            YtsRosterContactIterator   iterator,
                            void                      *user_data)
{
  YtsRosterPrivate *priv = GET_PRIVATE (self);
  GHashTableIter   iter;
  char const      *contact_id;
  YtsContact      *contact;
  bool             ret = true;

  g_return_val_if_fail (YTS_IS_ROSTER (self), false);
  g_return_val_if_fail (iterator, false);

  g_hash_table_iter_init (&iter, priv->contacts);
  while (ret &&
         g_hash_table_iter_next (&iter,
                                 (void **) &contact_id,
                                 (void **) &contact)) {
    ret = iterator (self, contact_id, contact, user_data);
  }

  return ret;
}

