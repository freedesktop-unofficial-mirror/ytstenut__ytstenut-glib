/*
 * Copyright (c) 2011 Intel Corp.
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
 * Authored by: Rob Staudinger <robsta@linux.intel.com>
 */

#include <stdbool.h>
#include "ytsg-invocation-message.h"

G_DEFINE_TYPE (YtsgInvocationMessage, ytsg_invocation_message, YTSG_TYPE_METADATA)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_INVOCATION_MESSAGE, YtsgInvocationMessagePrivate))

typedef struct {
  int dummy;
} YtsgInvocationMessagePrivate;

static void
_get_property (GObject      *object,
               unsigned int  property_id,
               GValue       *value,
               GParamSpec   *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_set_property (GObject      *object,
               unsigned int  property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_dispose (GObject *object)
{
  G_OBJECT_CLASS (ytsg_invocation_message_parent_class)->dispose (object);
}

static void
ytsg_invocation_message_class_init (YtsgInvocationMessageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgInvocationMessagePrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;
}

static void
ytsg_invocation_message_init (YtsgInvocationMessage *self)
{
}

YtsgMetadata *
ytsg_invocation_message_new (char const *capability,
                             char const *aspect,
                             GVariant   *arguments)
{
  RestXmlNode *node;

  node = rest_xml_node_add_child (NULL, "message");
  rest_xml_node_add_attr (node, "capability", capability);
  rest_xml_node_add_attr (node, "aspect", aspect);

  if (arguments) {
    char *args = g_variant_print (arguments, false);
    /* FIXME this is just a stopgap solution to lacking g_markup_unescape_text()
     * want to move to complex message bodies anywy. */
    char *escaped_args = g_uri_escape_string (args, NULL, true);
    rest_xml_node_add_attr (node, "arguments", escaped_args);
    g_free (escaped_args);
    g_free (args);
  }

  return g_object_new (YTSG_TYPE_INVOCATION_MESSAGE,
                       "top-level-node", node,
                       NULL);
}

