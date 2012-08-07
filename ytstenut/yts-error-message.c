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
 * Authored by: Rob Staudinger <robsta@linux.intel.com>
 */
#include "config.h"

#include <stdbool.h>

#include "yts-error-message.h"

G_DEFINE_TYPE (YtsErrorMessage, yts_error_message, YTS_TYPE_METADATA)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_ERROR_MESSAGE, YtsErrorMessagePrivate))

typedef struct {
  int dummy;
} YtsErrorMessagePrivate;

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
  G_OBJECT_CLASS (yts_error_message_parent_class)->dispose (object);
}

static void
yts_error_message_class_init (YtsErrorMessageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsErrorMessagePrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;
}

static void
yts_error_message_init (YtsErrorMessage *self)
{
}

YtsMetadata *
yts_error_message_new (char const  *domain,
                        int          code,
                        char const  *message,
                        char const  *invocation_id)
{
  RestXmlNode *node;

  node = rest_xml_node_add_child (NULL, "message");

  /* PONDERING need those keywords be made reserved */
  rest_xml_node_add_attr (node, "type", "error");

  if (domain) {
    rest_xml_node_add_attr (node, "domain", domain);
  }

  if (code) {
    char *error_code = g_strdup_printf ("%i", code);
    rest_xml_node_add_attr (node, "code", error_code);
    g_free (error_code);
  }

  if (message) {
    rest_xml_node_add_attr (node, "message", message);
  }

  if (invocation_id) {
    rest_xml_node_add_attr (node, "invocation", invocation_id);
  }

  return g_object_new (YTS_TYPE_ERROR_MESSAGE,
                       "top-level-node", node,
                       NULL);
}

