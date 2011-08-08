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
#include "ytsg-invocation-response.h"

G_DEFINE_TYPE (YtsgInvocationResponse, ytsg_invocation_response, YTSG_TYPE_METADATA)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTSG_TYPE_INVOCATION_RESPONSE, YtsgInvocationResponsePrivate))

typedef struct {
  int dummy;
} YtsgInvocationResponsePrivate;

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
  G_OBJECT_CLASS (ytsg_invocation_response_parent_class)->dispose (object);
}

static void
ytsg_invocation_response_class_init (YtsgInvocationResponseClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsgInvocationResponsePrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;
}

static void
ytsg_invocation_response_init (YtsgInvocationResponse *self)
{
}

YtsgMetadata *
ytsg_invocation_response_new (char const  *invocation_id,
                              GVariant    *response)
{
  RestXmlNode *node;

  node = rest_xml_node_add_child (NULL, "message");
  /* TODO need those keywords be made reserved */
  rest_xml_node_add_attr (node, "type", "response");
  rest_xml_node_add_attr (node, "invocation", invocation_id);

  if (response) {
    char *args = g_variant_print (response, false);
    /* FIXME this is just a stopgap solution to lacking g_markup_unescape_text()
     * want to move to complex message bodies anywy. */
    char *escaped_args = g_uri_escape_string (args, NULL, true);
    rest_xml_node_add_attr (node, "response", escaped_args);
    g_free (escaped_args);
    g_free (args);
  }

  return g_object_new (YTSG_TYPE_INVOCATION_RESPONSE,
                       "top-level-node", node,
                       NULL);
}

