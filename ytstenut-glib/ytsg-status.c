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
 * SECTION:ytsg-status
 * @short_description: Represent the status of a service connected
 * to the Ytstenut mesh.
 *
 * #YtsgStatus represents status of a known service in the Ytstenut application
 * mesh.
 */

#include <string.h>
#include <rest/rest-xml-node.h>

#include "ytsg-status.h"
#include "ytsg-private.h"

static void ytsg_status_dispose (GObject *object);
static void ytsg_status_finalize (GObject *object);
static void ytsg_status_constructed (GObject *object);
static void ytsg_status_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec);
static void ytsg_status_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgStatus, ytsg_status, YTSG_TYPE_METADATA);

#define YTSG_STATUS_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_STATUS, YtsgStatusPrivate))

struct _YtsgStatusPrivate
{
  guint disposed : 1;
};

enum
{
  N_SIGNALS,
};

enum
{
  PROP_0,
};

/*static guint signals[N_SIGNALS] = {0};*/

static void
ytsg_status_class_init (YtsgStatusClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgStatusPrivate));

  object_class->dispose      = ytsg_status_dispose;
  object_class->finalize     = ytsg_status_finalize;
  object_class->constructed  = ytsg_status_constructed;
  object_class->get_property = ytsg_status_get_property;
  object_class->set_property = ytsg_status_set_property;
}

static void
ytsg_status_constructed (GObject *object)
{
  if (G_OBJECT_CLASS (ytsg_status_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_status_parent_class)->constructed (object);
}

static void
ytsg_status_get_property (GObject    *object,
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
ytsg_status_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_status_init (YtsgStatus *self)
{
  self->priv = YTSG_STATUS_GET_PRIVATE (self);
}

static void
ytsg_status_dispose (GObject *object)
{
  YtsgStatus        *self = (YtsgStatus*) object;
  YtsgStatusPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (ytsg_status_parent_class)->dispose (object);
}

static void
ytsg_status_finalize (GObject *object)
{
  G_OBJECT_CLASS (ytsg_status_parent_class)->finalize (object);
}

/**
 * ytsg_status_new:
 * @attributes: NULL terminated array of name/value pairs; can be %NULL
 *
 * Constructs a new #YtsgStatus object, setting the top level attributes.
 *
 * Return value: (transfer full): newly allocated #YtsgStatus object.
 */
YtsgStatus *
ytsg_status_new (const char ** attributes)
{
  RestXmlNode  *top = rest_xml_node_add_child (NULL, "status");
  YtsgMetadata *mdata;

  g_return_val_if_fail (top, NULL);

  mdata = _ytsg_metadata_new_from_node (top, attributes);

  g_return_val_if_fail (YTSG_IS_STATUS (mdata), NULL);

  return (YtsgStatus*) mdata;
}

