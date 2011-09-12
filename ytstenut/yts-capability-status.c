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

#include <stdbool.h>

#include "yts-capability-status.h"
#include "config.h"

G_DEFINE_TYPE (YtsCapabilityStatus, yts_capability_status, YTS_TYPE_METADATA)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_CAPABILITY_STATUS, YtsCapabilityStatusPrivate))

typedef struct {
  int dummy;
} YtsCapabilityStatusPrivate;

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
  G_OBJECT_CLASS (yts_capability_status_parent_class)->dispose (object);
}

static void
yts_capability_status_class_init (YtsCapabilityStatusClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsCapabilityStatusPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->dispose = _dispose;
}

static void
yts_capability_status_init (YtsCapabilityStatus *self)
{
}

YtsMetadata *
yts_capability_status_new (char const  *capability,
                            char const  *aspect,
                            GVariant    *condition)
{
  RestXmlNode *node;

  node = rest_xml_node_add_child (NULL, "status");
  /* PONDERING need those keywords be made reserved */
  rest_xml_node_add_attr (node, "type", "notification");
  rest_xml_node_add_attr (node, "capability", capability);
  rest_xml_node_add_attr (node, "aspect", aspect);

  if (condition) {
    char *args = g_variant_print (condition, false);
    /* FIXME this is just a stopgap solution to lacking g_markup_unescape_text()
     * want to move to complex message bodies anywy. */
    char *escaped_args = g_uri_escape_string (args, NULL, true);
    rest_xml_node_add_attr (node, "condition", escaped_args);
    g_free (escaped_args);
    g_free (args);
  }

  return g_object_new (YTS_TYPE_CAPABILITY_STATUS,
                       "top-level-node", node,
                       NULL);
}

