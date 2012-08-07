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
#include "config.h"

#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/connection.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/channel.h>
#include <telepathy-glib/util.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/channel.h>

#include "ytstenut-internal.h"
#include "yts-capability.h"
#include "yts-contact-impl.h"
#include "yts-contact-internal.h"
#include "yts-enum-types.h"
#include "yts-error.h"
#include "yts-marshal.h"
#include "yts-proxy-service-internal.h"
#include "yts-service-internal.h"

G_DEFINE_ABSTRACT_TYPE (YtsContact, yts_contact, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_CONTACT, YtsContactPrivate))

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0contact\0"G_STRLOC

/**
 * SECTION: yts-contact
 * @short_description: Represents a device connected to the
 * Ytstenut mesh.
 *
 * #YtsContact represents a known device in the Ytstenut application mesh,
 * and provides access to any services (#YtsService) available on the device.
 */

typedef struct {
  GHashTable   *services;   /* hash of YtsService instances */
  /* string (service ID) => GHashTable<string fqc_id => string status_xml> */
  GHashTable   *deferred_service_statuses;
  TpContact    *tp_contact; /* TpContact associated with YtsContact */
} YtsContactPrivate;

enum {
  SIG_SERVICE_ADDED,
  SIG_SERVICE_REMOVED,

  N_SIGNALS
};

enum {
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_TP_CONTACT,

  PROP_LAST
};

static unsigned _signals[N_SIGNALS] = { 0, };

static void
_tp_contact_notify_alias (GObject     *tp_contact,
                          GParamSpec  *pspec,
                          YtsContact  *self)
{
  g_object_notify (G_OBJECT (self), "name");
}

static void
_service_send_message (YtsService   *service,
                       YtsMetadata  *message,
                       YtsContact   *self)
{
  /* This is a bit of a hack, we require the non-abstract subclass to
   * implement this interface. */
  yts_contact_impl_send_message (YTS_CONTACT_IMPL (self), service, message);
}

static YtsOutgoingFile *
_service_send_file (YtsService   *service,
                    GFile        *file,
                    char const   *description,
                    GError      **error_out,
                    YtsContact   *self)
{
  /* This is a bit of a hack, we require the non-abstract subclass to
   * implement this interface. */
  return yts_contact_impl_send_file (YTS_CONTACT_IMPL (self),
                                     service,
                                     file,
                                     description,
                                     error_out);
}

static void
_service_added (YtsContact  *self,
                YtsService  *service,
                void        *data)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  const char        *service_id  = yts_service_get_id (service);
  GHashTable        *id_status_map;

  DEBUG ("contact=%s service=%s", yts_contact_get_id (self), service_id);

  g_return_if_fail (service_id && *service_id);
  g_return_if_fail (!g_hash_table_lookup (priv->services, service_id));

  g_hash_table_insert (priv->services,
                       g_strdup (service_id),
                       g_object_ref (service));

  g_signal_connect (service, "send-message",
                    G_CALLBACK (_service_send_message), self);
  g_signal_connect (service, "send-file",
                    G_CALLBACK (_service_send_file), self);

  /* Apply deferred status updates */

  id_status_map = g_hash_table_lookup (priv->deferred_service_statuses,
      service_id);

  if (id_status_map != NULL)
    {
      GHashTableIter iter;
      gpointer k, v;

      g_hash_table_iter_init (&iter, id_status_map);

      while (g_hash_table_iter_next (&iter, &k, &v))
        yts_service_update_status (service, k, v);
    }

  g_hash_table_remove (priv->deferred_service_statuses,
      service_id);
}

static void
_service_removed (YtsContact  *self,
                  YtsService  *service,
                  void        *data)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  const char        *service_id  = yts_service_get_id (service);

  DEBUG ("contact=%s service=%s", yts_contact_get_id (self), service_id);

  g_return_if_fail (service_id && *service_id);

  if (!g_hash_table_remove (priv->services, service_id))
    g_warning (G_STRLOC ": unknown service with service-id %s", service_id);

  g_signal_handlers_disconnect_by_func (service,
                                        _service_send_message,
                                        self);
  g_signal_handlers_disconnect_by_func (service,
                                        _service_send_file,
                                        self);
}

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsContactPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_ID:
      g_value_set_string (value,
                          yts_contact_get_id (YTS_CONTACT (object)));
      break;
    case PROP_NAME:
      g_value_set_string (value,
                          tp_contact_get_alias (priv->tp_contact));
      break;
    case PROP_TP_CONTACT:
      g_value_set_object (value, priv->tp_contact);
      break;

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
  YtsContactPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_TP_CONTACT: {
      g_return_if_fail (g_value_get_object (value));
      priv->tp_contact = g_value_dup_object (value);
      g_signal_connect (priv->tp_contact, "notify::alias",
                        G_CALLBACK (_tp_contact_notify_alias), object);
    } break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  YtsContact *self = YTS_CONTACT (object);
  YtsContactPrivate *priv = GET_PRIVATE (object);

  if (priv->services)
    {
      GHashTableIter iter;
      gpointer k, v;

      g_hash_table_iter_init (&iter, priv->services);

      while (g_hash_table_iter_next (&iter, &k, &v))
        {
          g_signal_handlers_disconnect_by_func (v, _service_send_message,
              self);
          g_signal_handlers_disconnect_by_func (v, _service_send_file,
              self);
        }

      g_hash_table_destroy (priv->services);
      priv->services = NULL;
    }

  if (priv->deferred_service_statuses) {
    g_hash_table_destroy (priv->deferred_service_statuses);
    priv->deferred_service_statuses = NULL;
  }

  // FIXME tie to tp_contact lifecycle
  if (priv->tp_contact)
    {
      g_signal_handlers_disconnect_by_func (priv->tp_contact,
          _tp_contact_notify_alias, self);

      g_object_unref (priv->tp_contact);
      priv->tp_contact = NULL;
    }

  G_OBJECT_CLASS (yts_contact_parent_class)->dispose (object);
}

static void
_finalize (GObject *object)
{
  G_OBJECT_CLASS (yts_contact_parent_class)->finalize (object);
}

static void
yts_contact_class_init (YtsContactClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (YtsContactPrivate));

  object_class->dispose      = _dispose;
  object_class->finalize     = _finalize;
  object_class->get_property = _get_property;
  object_class->set_property = _set_property;

  /**
   * YtsContact:id:
   *
   * The JID of this contact.
   */
  pspec = g_param_spec_string ("id", "", "",
                               NULL,
                               G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_ID, pspec);

  /**
   * YtsContact:name:
   *
   * The display name of this contact.
   */
  pspec = g_param_spec_string ("name", "", "",
                               NULL,
                               G_PARAM_READABLE);
  g_object_class_install_property (object_class, PROP_NAME, pspec);

  /**
   * YtsContact:tp-contact:
   *
   * #TpContact of this item.
   *
   * <note>There is no API guarantee for this and other fields that expose telepathy.</note>
   */
  pspec = g_param_spec_object ("tp-contact", "", "",
                               TP_TYPE_CONTACT,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_TP_CONTACT, pspec);

  /**
   * YtsContact::service-added:
   * @self: object which emitted the signal.
   * @service: the service
   *
   * The ::service-added signal is emitted when a new services is added to
   * the contact.
   *
   * Since: 0.1
   */
  _signals[SIG_SERVICE_ADDED] = g_signal_new ("service-added",
                                              G_TYPE_FROM_CLASS (object_class),
                                              G_SIGNAL_RUN_FIRST,
                                              0, NULL, NULL,
                                              yts_marshal_VOID__OBJECT,
                                              G_TYPE_NONE, 1,
                                              YTS_TYPE_SERVICE);

  /**
   * YtsContact::service-removed:
   * @self: object which emitted the signal.
   * @service: the service
   *
   * The ::service-removed signal is emitted when a services is removed from
   * the contact.
   *
   * Since: 0.1
   */
  _signals[SIG_SERVICE_REMOVED] = g_signal_new (
                                              "service-removed",
                                              G_TYPE_FROM_CLASS (object_class),
                                              G_SIGNAL_RUN_LAST,
                                              0, NULL, NULL,
                                              yts_marshal_VOID__OBJECT,
                                              G_TYPE_NONE, 1,
                                              YTS_TYPE_SERVICE);
}

static void
yts_contact_init (YtsContact *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_signal_connect (self, "service-added",
                    G_CALLBACK (_service_added), NULL);
  g_signal_connect (self, "service-removed",
                    G_CALLBACK (_service_removed), NULL);

  priv->services = g_hash_table_new_full (g_str_hash,
                                          g_str_equal,
                                          g_free,
                                          g_object_unref);

  priv->deferred_service_statuses = g_hash_table_new_full (g_str_hash,
      g_str_equal, g_free, (GDestroyNotify) g_hash_table_unref);
}

/**
 * yts_contact_get_id:
 * @self: object on which to invoke this method.
 *
 * Retrieves the jabber identifier of this contact.
 *
 * Returns: (transfer none): The JID of this contact.
 */
const char *
yts_contact_get_id (YtsContact const *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CONTACT (self), NULL);
  g_return_val_if_fail (priv->tp_contact, NULL);

  return tp_contact_get_identifier (priv->tp_contact);
}

/**
 * yts_contact_get_name:
 * @self: object on which to invoke this method.
 *
 * Retrieves human readable name of this contact. This has undefined semantics
 * with ytstenut, as there can be multiple services running under a single
 * account, and potentially use different names.
 *
 * Returns: (transfer none): The name of this contact.
 */
const char *
yts_contact_get_name (YtsContact const *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CONTACT (self), NULL);
  g_return_val_if_fail (priv->tp_contact, NULL);

  return tp_contact_get_alias (priv->tp_contact);
}

/**
 * yts_contact_get_tp_contact:
 * @self: object on which to invoke this method.
 *
 * Retrieves the #TpContact associated with this #YtsContact object; can be
 * %NULL. When the #TpContact is available, the YtsContact::notify-tp-contact
 * signal will be emitted.
 *
 * Returns: (transfer none): The associated #TpContact.
 */
TpContact *const
yts_contact_get_tp_contact (YtsContact const *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CONTACT (self), NULL);

  return priv->tp_contact;
}

void
yts_contact_add_service (YtsContact *self,
                         YtsService *service)
{
  /*
   * Emit the signal; the run-first signal closure will do the rest
   */
  g_message ("New service %s on %s",
             yts_service_get_id (service),
             yts_contact_get_id (self));

  g_signal_emit (self, _signals[SIG_SERVICE_ADDED], 0, service);
}

YtsService *const
yts_contact_find_service_by_id (YtsContact  *self,
                                char const  *service_id)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  return g_hash_table_lookup (priv->services, service_id);
}

void
yts_contact_remove_service_by_id (YtsContact  *self,
                                  const char  *service_id)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  YtsService        *service;

  DEBUG ("contact=%s service=%s", yts_contact_get_id (self), service_id);

  g_return_if_fail (service_id && *service_id);

  /*
   * Look up the service and emit the service-removed signal; the signal closure
   *  will take care of the rest.
   */
  service = g_hash_table_lookup (priv->services, service_id);
  if (service)
    {
      g_signal_emit (self, _signals[SIG_SERVICE_REMOVED], 0, service);
    }
  else
    {
      g_warning ("%s : Trying to remove service %s but not found?!",
                 G_STRLOC,
                 service_id);
    }
}

bool
yts_contact_is_empty (YtsContact *self)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);

  return (g_hash_table_size (priv->services) == 0);
}

bool
yts_contact_dispatch_event (YtsContact  *self,
                             char const   *capability,
                             char const   *aspect,
                             GVariant     *arguments)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  char const      *service_id;
  YtsService      *service;
  GHashTableIter   iter;
  bool             dispatched = FALSE;

  g_return_val_if_fail (YTS_IS_CONTACT (self), FALSE);

  g_hash_table_iter_init (&iter, priv->services);
  while (g_hash_table_iter_next (&iter,
                                 (void **) &service_id,
                                 (void **) &service)) {
    if (YTS_IS_PROXY_SERVICE (service) &&
        yts_capability_has_fqc_id (YTS_CAPABILITY (service), capability)) {

      /* Dispatch to all matching services, be happy if one of them accepts. */
      dispatched = dispatched ||
              yts_proxy_service_dispatch_event (YTS_PROXY_SERVICE (service),
                                                 capability,
                                                 aspect,
                                                 arguments);
    }
  }
  return dispatched;
}

bool
yts_contact_dispatch_response (YtsContact *self,
                                char const  *capability,
                                char const  *invocation_id,
                                GVariant    *response)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  char const      *service_id;
  YtsService      *service;
  GHashTableIter   iter;
  bool             dispatched = FALSE;

  g_return_val_if_fail (YTS_IS_CONTACT (self), FALSE);

  g_hash_table_iter_init (&iter, priv->services);
  while (g_hash_table_iter_next (&iter,
                                 (void **) &service_id,
                                 (void **) &service)) {
    if (YTS_IS_PROXY_SERVICE (service) &&
        yts_capability_has_fqc_id (YTS_CAPABILITY (service), capability)) {

      /* Invocations are unique, so just go home after delivery. */
      dispatched =
            yts_proxy_service_dispatch_response (YTS_PROXY_SERVICE (service),
                                                  capability,
                                                  invocation_id,
                                                  response);
      if (dispatched)
        break;
    }
  }
  return dispatched;
}

void
yts_contact_update_service_status (YtsContact *self,
                                   char const *service_id,
                                   char const *fqc_id,
                                   char const *status_xml)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  YtsService *service;

  DEBUG ("contact=%s service=%s fqc=%s", yts_contact_get_id (self),
      service_id, fqc_id);
  service = g_hash_table_lookup (priv->services, service_id);

  if (service != NULL)
    {
      DEBUG ("Service already exists, updating status now");
      yts_service_update_status (service, fqc_id, status_xml);
    }
  else
    {
      /* We've hit a race condition between the service's status being
       * discovered, and the service's capabilities being discovered.
       * Save the status and apply it when we get the service's
       * capabilities. */
      GHashTable *id_status_map = g_hash_table_lookup (
          priv->deferred_service_statuses, service_id);

      DEBUG ("Service does not already exist, saving its status for later");

      if (id_status_map == NULL)
        {
          id_status_map = g_hash_table_new_full (g_str_hash, g_str_equal,
              g_free, g_free);
          g_hash_table_insert (priv->deferred_service_statuses,
              g_strdup (service_id), id_status_map);
        }

      g_hash_table_insert (id_status_map, g_strdup (fqc_id),
          g_strdup (status_xml));
    }
}

/**
 * yts_contact_foreach_service:
 * @self: object on which to invoke this method.
 * @iterator: iterator function.
 * @user_data: context to pass to the iterator function.
 *
 * Iterate over @self's services.
 *
 * Returns: <literal>true</literal> if all the services have been iterated.
 *
 * Since: 0.4
 */
bool
yts_contact_foreach_service (YtsContact                 *self,
                             YtsContactServiceIterator   iterator,
                             void                       *user_data)
{
  YtsContactPrivate *priv = GET_PRIVATE (self);
  GHashTableIter   iter;
  char const      *service_id;
  YtsService      *service;
  bool             ret = true;

  g_return_val_if_fail (YTS_IS_CONTACT (self), false);
  g_return_val_if_fail (iterator, false);

  g_hash_table_iter_init (&iter, priv->services);
  while (ret &&
         g_hash_table_iter_next (&iter,
                                 (void **) &service_id,
                                 (void **) &service)) {
    ret = iterator (self, service_id, service, user_data);
  }

  return ret;
}

