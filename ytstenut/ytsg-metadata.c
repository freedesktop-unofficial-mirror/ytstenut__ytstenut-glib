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
 * SECTION:ytsg-metadata
 * @short_description: Base class for #YtsgStatus and #YtsgMessage.
 *
 * #YtsgMetadata is a base class for Ytstenut metadata classes.
 */

#include "ytsg-metadata.h"
#include "ytsg-private.h"
#include "ytsg-message.h"
#include "ytsg-status.h"

#include <rest/rest-xml-parser.h>
#include <string.h>

static void ytsg_metadata_dispose (GObject *object);
static void ytsg_metadata_finalize (GObject *object);
static void ytsg_metadata_constructed (GObject *object);
static void ytsg_metadata_get_property (GObject    *object,
                                        guint       property_id,
                                        GValue     *value,
                                        GParamSpec *pspec);
static void ytsg_metadata_set_property (GObject      *object,
                                        guint         property_id,
                                        const GValue *value,
                                        GParamSpec   *pspec);

G_DEFINE_TYPE (YtsgMetadata, ytsg_metadata, G_TYPE_OBJECT);

#define YTSG_METADATA_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_METADATA, YtsgMetadataPrivate))

struct _YtsgMetadataPrivate
{
  RestXmlNode  *top_level_node;
  char         *xml;
  char        **attributes;

  guint disposed : 1;
  guint readonly : 1;
};

enum
{
  N_SIGNALS,
};

enum
{
  PROP_0,
  PROP_XML,
  PROP_TOP_LEVEL_NODE,
  PROP_ATTRIBUTES,
};

static void
ytsg_metadata_class_init (YtsgMetadataClass *klass)
{
  GParamSpec   *pspec;
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsgMetadataPrivate));

  object_class->dispose      = ytsg_metadata_dispose;
  object_class->finalize     = ytsg_metadata_finalize;
  object_class->constructed  = ytsg_metadata_constructed;
  object_class->get_property = ytsg_metadata_get_property;
  object_class->set_property = ytsg_metadata_set_property;

  /**
   * YtsgMetadata:xml:
   *
   * The XML node this #YtsgMetadata object is to represent
   *
   * This property is only valid during construction, as no copies are made.
   */
  pspec = g_param_spec_string ("xml",
                               "Metadata XML",
                               "Metadata XML",
                               NULL,
                               G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_XML, pspec);

  /**
   * YtsgMetadata:top-level-node:
   *
   * The top level node for the metadata
   */
  pspec = g_param_spec_boxed ("top-level-node",
                              "Top-level node",
                              "Top-level RestXmlNode",
                              REST_TYPE_XML_NODE,
                              G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_TOP_LEVEL_NODE, pspec);

  /**
   * YtsgMetadata:attributes:
   *
   * Top level attributes; this property is only valid during the construction
   * of the object, as no copies of the supplied value are made!
   */
  pspec = g_param_spec_boxed ("attributes",
                              "Top-level attributes",
                              "Top-level attributes",
                              G_TYPE_STRV,
                              G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_ATTRIBUTES, pspec);
}

static void
ytsg_metadata_constructed (GObject *object)
{
  YtsgMetadata        *self = (YtsgMetadata*) object;
  YtsgMetadataPrivate *priv = self->priv;

  if (G_OBJECT_CLASS (ytsg_metadata_parent_class)->constructed)
    G_OBJECT_CLASS (ytsg_metadata_parent_class)->constructed (object);

  g_assert (priv->xml || priv->top_level_node);

  if (!priv->top_level_node && priv->xml)
    {

      RestXmlParser *parser = rest_xml_parser_new ();

      priv->top_level_node =
        rest_xml_parser_parse_from_data (parser, priv->xml, strlen (priv->xml));

      g_object_unref (parser);

      g_assert (priv->top_level_node);

      g_free (priv->xml);
      priv->xml = NULL;
    }

  if (priv->attributes)
    {
      char  **p;

      for (p = priv->attributes; *p && *(p + 1); p += 2)
        {
          const char *a = *p;
          const char *v = *(p + 1);

          rest_xml_node_add_attr (priv->top_level_node, a, v);
        }

      /*
       * the stored pointer is only valid during construction during the
       * construction
       */
      g_strfreev (priv->attributes);
      priv->attributes = NULL;
    }
}

static void
ytsg_metadata_get_property (GObject    *object,
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
ytsg_metadata_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  YtsgMetadata        *self = (YtsgMetadata*) object;
  YtsgMetadataPrivate *priv = self->priv;

  switch (property_id)
    {
    case PROP_XML:
      priv->xml = g_value_dup_string (value);
      break;
    case PROP_TOP_LEVEL_NODE:
      if (priv->top_level_node)
        rest_xml_node_unref (priv->top_level_node);

      priv->top_level_node = g_value_get_boxed (value);
      break;
    case PROP_ATTRIBUTES:
      priv->attributes = g_value_dup_boxed (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
ytsg_metadata_init (YtsgMetadata *self)
{
  self->priv = YTSG_METADATA_GET_PRIVATE (self);
}

static void
ytsg_metadata_dispose (GObject *object)
{
  YtsgMetadata        *self = (YtsgMetadata*) object;
  YtsgMetadataPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  if (priv->top_level_node)
    rest_xml_node_unref (priv->top_level_node);

  G_OBJECT_CLASS (ytsg_metadata_parent_class)->dispose (object);
}

static void
ytsg_metadata_finalize (GObject *object)
{
  G_OBJECT_CLASS (ytsg_metadata_parent_class)->finalize (object);
}

/**
 * ytsg_metadata_get_root_node:
 * @self: #YtsgMetadata
 *
 * Returns pointer to the top-level node of the metadata, that can be used
 * with the #RestXmlNode API.
 *
 * NB: Any strings set directly through librest API must be in utf-8 encoding.
 *
 * Return value: (transfer none): #RestXmlNode representing the top-level node
 * of the metadata xml.
 */
RestXmlNode *
ytsg_metadata_get_root_node (YtsgMetadata *self)
{
  YtsgMetadataPrivate *priv;

  g_return_val_if_fail (YTSG_IS_METADATA (self), NULL);

  priv = self->priv;

  return priv->top_level_node;
}

/*
 * _ytsg_metadata_new_from_xml:
 * @xml: the xml the metatdata object is to represent
 *
 * Constructs a new #YtsgMetadata object from the xml snippet; depending on the
 * xml, this is either #YtsgMessage or #YtsgStatus.
 */
YtsgMetadata *
_ytsg_metadata_new_from_xml (const char *xml)
{
  YtsgMetadata  *mdata;
  RestXmlParser *parser;
  RestXmlNode   *node;

  g_return_val_if_fail (xml && *xml, NULL);

  parser = rest_xml_parser_new ();

  node = rest_xml_parser_parse_from_data (parser, xml, strlen (xml));

  g_object_unref (parser);

  /* We do not unref the node, the object takes over the reference */

  mdata = _ytsg_metadata_new_from_node (node, NULL);
  mdata->priv->readonly = TRUE;

  return mdata;
}

/*
 * _ytsg_metadata_new_from_node:
 * @node: #RestXmlNode
 * @attributes: %NULL terminated array of name/value pairs for additional
 * attributes, can be %NULL
 *
 * Private constructor.
 *
 * Return value: (transfer full): newly allocated #YtsgMetadata subclass, either
 * #YtsgMessage or #YtsgStatus, depending on the top level node.
 */
YtsgMetadata *
_ytsg_metadata_new_from_node (RestXmlNode *node, const char **attributes)
{
  YtsgMetadata  *mdata = NULL;

  g_return_val_if_fail (node && node->name, NULL);

  if (!strcmp (node->name, "message"))
    {
      if (attributes)
        mdata = g_object_new (YTSG_TYPE_MESSAGE,
                              "top-level-node", node,
                              "attributes",     attributes,
                              NULL);
      else
        mdata = g_object_new (YTSG_TYPE_MESSAGE, "top-level-node", node, NULL);
    }
  else if (!strcmp (node->name, "status"))
    {
      if (attributes)
        {
          const char **p;
          gboolean     have_caps     = FALSE;
          gboolean     have_activity = FALSE;
          gboolean     have_xmlns    = FALSE;
          gboolean     have_from     = FALSE;

          for (p = attributes; *p; ++p)
            {
              if (!strcmp (*p, "capability"))
                have_caps = TRUE;
              else if (!strcmp (*p, "activity"))
                have_activity = TRUE;
              else if (!strcmp (*p, "xmlns"))
                have_xmlns = TRUE;
              else if (!strcmp (*p, "from-service"))
                have_from = TRUE;
            }

          if (!have_caps || !have_activity || !have_from)
            {
              g_warning ("Cannot construct YtsgStatus without capability, "
                         "activity and from-service attributes.");
              return NULL;
            }

          if (have_xmlns)
            {
              g_warning ("xmlns attribute is not allowed when constructing "
                         "YtsgStatus");
              return NULL;
            }

          mdata = g_object_new (YTSG_TYPE_STATUS,
                                "top-level-node", node,
                                "attributes",     attributes,
                                NULL);

          ytsg_metadata_add_attribute (mdata, "xmlns", "urn:ytstenut:status");
        }
      else
        mdata = g_object_new (YTSG_TYPE_STATUS, "top-level-node", node, NULL);
    }
  else
    g_warning ("Unknown top level node '%s'", node->name);

  /* We do not unref the node, the object takes over the reference */

  return mdata;
}

/**
 * ytsg_metadata_get_attribute:
 * @self: #YtsgMetadata
 * @name: name of the attribute to look up
 *
 * Retrieves the value of an attribute of the given name on the top level node
 * of the #YtsgMetadata object (to query attributes on children of the top level
 * node, you need to use ytsg_metadata_get_root_node() and the librest API to
 * locate and query the appropriate node).
 *
 * Return value: (transfer none): the attribute value or %NULL if attribute
 * does not exist.
 */
const char *
ytsg_metadata_get_attribute (YtsgMetadata *self, const char *name)
{
  YtsgMetadataPrivate *priv;

  g_return_val_if_fail (YTSG_IS_METADATA (self), NULL);

  priv = self->priv;

  g_return_val_if_fail (priv->top_level_node, NULL);

  return rest_xml_node_get_attr (priv->top_level_node, name);
}

/**
 * ytsg_metadata_add_attribute:
 * @self: #YtsgMetadata
 * @name: name of the attribute to add
 * @value: value of the attribute to add
 *
 * Adds an attribute of the given name on the top level node
 * of the #YtsgMetadata object (to add attributes to children of the top level
 * node, you need to use ytsg_metadata_get_root_node() and the librest API to
 * construct the metadata tree).
 *
 * NB: Both attribute names and values must be in utf-8 encoding.
 */
void
ytsg_metadata_add_attribute (YtsgMetadata *self,
                             const char   *name,
                             const char   *value)
{
  YtsgMetadataPrivate *priv;

  g_return_if_fail (YTSG_IS_METADATA (self) && name && *name && value);

  priv = self->priv;

  g_return_if_fail (priv->top_level_node);
  g_return_if_fail (!priv->readonly);

  rest_xml_node_add_attr (priv->top_level_node, name, value);
}

/**
 * ytsg_metadata_to_string:
 * @self: #YtsgMetadata
 *
 * Converts the #YtsgMetada object in XML representation.
 *
 * Return value: (transfer full): xml string; the caller must free the string
 * with g_free() when no longer needed.
 */
char *
ytsg_metadata_to_string (YtsgMetadata *self)
{
  YtsgMetadataPrivate *priv;

  g_return_val_if_fail (YTSG_IS_METADATA (self), NULL);

  priv = self->priv;

  g_return_val_if_fail (priv->top_level_node, NULL);

  return rest_xml_node_print (priv->top_level_node);
}

/*
 * _ytsg_metadata_extract:
 * @self: #YtsgMetadata
 * @body: location to store the body of the metadata; free with g_free() when no
 * longer needed.
 *
 * Extracts the top level attributes into a hash table, and converts the
 * content of any child nodes to xml string. This is intended for use by
 * #YtsgClient when sending messages.
 *
 * Return value: (transfer full): top level attributes, the caller holds a
 * reference on the returned hash table, which it needs to release with
 * g_hash_table_unref() when no longer needed.
 */
GHashTable *
_ytsg_metadata_extract (YtsgMetadata *self, char **body)
{
  YtsgMetadataPrivate *priv;
  RestXmlNode         *n0;
  GHashTableIter       iter;
  gpointer             key, value;
  char                *b;

  g_return_val_if_fail (YTSG_IS_METADATA (self) && body, NULL);

  priv = self->priv;
  n0 = priv->top_level_node;

  b = g_strdup (n0->content);

  /*
   * This is necessary for the g_strconcat() below to work */
  if (!b)
    b = g_strdup ("");

  g_hash_table_iter_init (&iter, n0->children);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      char *child = rest_xml_node_print ((RestXmlNode *) value);

      b = g_strconcat (b, child, NULL);
      g_free (child);
    }

  *body = b;

  return g_hash_table_ref (n0->attrs);
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
 * ytsg_metadata_is_equal:
 * @self: #YtsgMetadata,
 * @other: #YtsgMetadata
 *
 * Compares two metadata instances and returns %TRUE if they are equal.
 * NB: equality implies identity of type, i.e., #YtsgMessage and #YtsgStatus
 * can be compared, but will always be unequal.
 *
 * Return value: %TRUE if equal, %FALSE otherwise.
 */
gboolean
ytsg_metadata_is_equal (YtsgMetadata *self, YtsgMetadata *other)
{
  RestXmlNode *node0;
  RestXmlNode *node1;

  g_return_val_if_fail (YTSG_IS_METADATA (self) && YTSG_IS_METADATA (other),
                        FALSE);

  if (G_OBJECT_TYPE (self) != G_OBJECT_TYPE (other))
    return FALSE;

  node0 = ytsg_metadata_get_root_node ((YtsgMetadata*) self);
  node1 = ytsg_metadata_get_root_node ((YtsgMetadata*) other);

  if (!ytsg_rest_xml_node_check_attrs (node0, node1))
    return FALSE;

  if (!ytsg_rest_xml_node_check_children (node0, node1))
    return FALSE;

  return TRUE;
}
