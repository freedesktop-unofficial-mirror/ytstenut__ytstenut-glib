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

static gboolean
ytsg_rest_xml_node_check_attrs (RestXmlNode *node0, RestXmlNode *node1)
{
  GHashTableIter iter;
  gpointer       key, value;

  if (g_hash_table_size (node0->attrs) != g_hash_table_size (node1->attrs))
    return FALSE;

  g_hash_table_iter_init (&iter, node0->attrs);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      const char *at1 = rest_xml_node_get_attr (node1, key);

      if (!value && !at1)
        continue;

      if ((!at1 && value) || (at1 && !value) || strcmp (value, at1))
        return FALSE;
    }

  return TRUE;
}

static gboolean ytsg_rest_xml_node_check_children (RestXmlNode*, RestXmlNode*);

/*
 * NB: this function is somewhat simplistic; it assumes that if two nodes have
 *     siblings, the corresponding siblings must be in identical order. But
 *     can we define equaly in any other way?
 */
static gboolean
ytsg_rest_xml_node_check_siblings (RestXmlNode *node0, RestXmlNode *node1)
{
  RestXmlNode *sib0 = node0->next;
  RestXmlNode *sib1 = node1->next;

  if (!sib0 && !sib1)
    return TRUE;

  do
    {
      if ((!sib0 && sib1) || (sib0 && !sib1))
        return FALSE;

      if (!ytsg_rest_xml_node_check_attrs (sib0, sib1))
        return FALSE;

      if (!ytsg_rest_xml_node_check_children (sib0, sib1))
        return FALSE;

      sib0 = sib0->next;
      sib1 = sib1->next;
    } while (sib0 || sib1);

  return TRUE;
}

static gboolean
ytsg_rest_xml_node_check_children (RestXmlNode *node0, RestXmlNode *node1)
{
  GHashTableIter iter;
  gpointer       key, value;

  if (g_hash_table_size (node0->attrs) != g_hash_table_size (node1->attrs))
    return FALSE;

  g_hash_table_iter_init (&iter, node0->children);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      const char  *name0  = key;
      RestXmlNode *child0 = value;
      RestXmlNode *child1 = rest_xml_node_find (node1, name0);

      if (!child1 ||
          (child0->next && !child1->next) || (!child0->next && child1->next))
        return FALSE;

      if (!ytsg_rest_xml_node_check_attrs (child0, child1))
        return FALSE;

      if (!ytsg_rest_xml_node_check_siblings (child0, child1))
        return FALSE;

      if (!ytsg_rest_xml_node_check_children (child0, child1))
        return FALSE;
    }

  return TRUE;
}

/**
 * ytsg_status_equal:
 * @self: #YtsgStatus,
 * @other: #YtsgStatus
 *
 * Compares two statuses and returns %TRUE if they are equal.
 *
 * Return value: %TRUE if equal, %FALSE otherwise.
 */
gboolean
ytsg_status_equal (YtsgStatus *self, YtsgStatus *other)
{
  RestXmlNode *node0;
  RestXmlNode *node1;

  g_return_val_if_fail (YTSG_IS_STATUS (self) && YTSG_IS_STATUS (other), FALSE);

  node0 = ytsg_metadata_get_top_node ((YtsgMetadata*) self);
  node1 = ytsg_metadata_get_top_node ((YtsgMetadata*) other);

  if (!ytsg_rest_xml_node_check_attrs (node0, node1))
    return FALSE;

  if (!ytsg_rest_xml_node_check_children (node0, node1))
    return FALSE;

  return TRUE;
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

