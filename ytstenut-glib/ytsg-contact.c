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

#include "ytsg-contact.h"
#include "ytsg-private.h"
#include "ytsg-marshal.h"

static void ytsg_contact_dispose (GObject *object);
static void ytsg_contact_finalize (GObject *object);
static void ytsg_contact_constructed (GObject *object);
static void ytsg_contact_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec);
static void ytsg_contact_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgContact, ytsg_contact, G_TYPE_OBJECT);

#define YTSG_CONTACT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_CONTACT, YtsgContactPrivate))

struct _YtsgContactPrivate
{
  GHashTable *services;

  guint disposed : 1;
};

enum
{
  SERVICE_ADDED,
  SERVICE_REMOVED,

  N_SIGNALS,
};

enum
{
  PROP_0,
};

static guint signals[N_SIGNALS] = {0};

static void
ytsg_contact_service_added (YtsgContact *contact, YtsgService *service)
{
  YtsgContactPrivate *priv = contact->priv;
  const char         *uid  = ytsg_service_get_uid (service);

  g_return_if_fail (uid && *uid);
  g_return_if_fail (!g_hash_table_lookup (priv->services, uid));

  g_hash_table_insert (priv->services, (gpointer)uid, g_object_ref (service));
}

static void
ytsg_contact_service_removed (YtsgContact *contact, YtsgService *service)
{
  YtsgContactPrivate *priv = contact->priv;
  const char         *uid  = ytsg_service_get_uid (service);

  g_return_if_fail (uid && *uid);

  if (!g_hash_table_remove (priv->services, uid))
    g_warning (G_STRLOC ": unknown service with uid %s", uid);
}

static void
ytsg_contact_class_init (YtsgContactClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgContactPrivate));

  object_class->dispose      = ytsg_contact_dispose;
  object_class->finalize     = ytsg_contact_finalize;
  object_class->constructed  = ytsg_contact_constructed;
  object_class->get_property = ytsg_contact_get_property;
  object_class->set_property = ytsg_contact_set_property;

  klass->service_added       = ytsg_contact_service_added;
  klass->service_removed     = ytsg_contact_service_removed;

  /**
   * YtsgContact::service-added:
   * @contact: the contact which received the signal
   * @service: the service
   *
   * The ::service-added signal is emitted when a new services is added to
   * the contact.
   *
   * Since: 0.1
   */
  signals[SERVICE_ADDED] =
    g_signal_new (I_("service-added"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (YtsgContactClass, service_added),
                  NULL, NULL,
                  ytsg_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTSG_TYPE_SERVICE);

  /**
   * YtsgContact::service-removed:
   * @contact: the contact which received the signal
   * @service: the service
   *
   * The ::service-removed signal is emitted when a services is removed from
   * the contact.
   *
   * Since: 0.1
   */
  signals[SERVICE_REMOVED] =
    g_signal_new (I_("service-removed"),
                  G_TYPE_FROM_CLASS (object_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (YtsgContactClass, service_removed),
                  NULL, NULL,
                  ytsg_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  YTSG_TYPE_SERVICE);
}

static void
ytsg_contact_constructed (GObject *object)
{
  YtsgContact *self = (YtsgContact*) object;

  if (G_OBJECT_CLASS (ytsg_contact_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_contact_parent_class)->constructed (object);
}

static void
ytsg_contact_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_contact_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_contact_init (YtsgContact *self)
{
  self->priv = YTSG_CONTACT_GET_PRIVATE (self);

  self->priv->services = g_hash_table_new_full (g_direct_hash,
                                                g_direct_equal,
                                                NULL,
                                                g_object_unref);
}

static void
ytsg_contact_dispose (GObject *object)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  g_hash_table_destroy (priv->services);
  priv->services = NULL;

  G_OBJECT_CLASS (ytsg_contact_parent_class)->dispose (object);
}

static void
ytsg_contact_finalize (GObject *object)
{
  YtsgContact        *self = (YtsgContact*) object;
  YtsgContactPrivate *priv = self->priv;

  G_OBJECT_CLASS (ytsg_contact_parent_class)->finalize (object);
}

