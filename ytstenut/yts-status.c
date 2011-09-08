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
 */

/**
 * SECTION:yts-status
 * @short_description: Represent the status of a service connected
 * to the Ytstenut mesh.
 *
 * #YtsStatus represents status of a known service in the Ytstenut application
 * mesh.
 */

#include <string.h>
#include <rest/rest-xml-node.h>

#include "yts-status.h"
#include "yts-private.h"

static void yts_status_dispose (GObject *object);
static void yts_status_finalize (GObject *object);
static void yts_status_constructed (GObject *object);
static void yts_status_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec);
static void yts_status_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec);

G_DEFINE_TYPE (YtsStatus, yts_status, YTS_TYPE_METADATA);

#define YTS_STATUS_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_STATUS, YtsStatusPrivate))

struct _YtsStatusPrivate
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
yts_status_class_init (YtsStatusClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsStatusPrivate));

  object_class->dispose      = yts_status_dispose;
  object_class->finalize     = yts_status_finalize;
  object_class->constructed  = yts_status_constructed;
  object_class->get_property = yts_status_get_property;
  object_class->set_property = yts_status_set_property;
}

static void
yts_status_constructed (GObject *object)
{
  if (G_OBJECT_CLASS (yts_status_parent_class)->constructed)
    G_OBJECT_CLASS (yts_status_parent_class)->constructed (object);
}

static void
yts_status_get_property (GObject    *object,
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
yts_status_set_property (GObject      *object,
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
yts_status_init (YtsStatus *self)
{
  self->priv = YTS_STATUS_GET_PRIVATE (self);
}

static void
yts_status_dispose (GObject *object)
{
  YtsStatus        *self = (YtsStatus*) object;
  YtsStatusPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (yts_status_parent_class)->dispose (object);
}

static void
yts_status_finalize (GObject *object)
{
  G_OBJECT_CLASS (yts_status_parent_class)->finalize (object);
}

/**
 * yts_status_new:
 * @attributes: NULL terminated array of name/value pairs; can be %NULL
 *
 * Constructs a new #YtsStatus object, setting the top level attributes.
 *
 * Return value: (transfer full): newly allocated #YtsStatus object.
 */
YtsStatus *
yts_status_new (const char ** attributes)
{
  RestXmlNode  *top = rest_xml_node_add_child (NULL, "status");
  YtsMetadata *mdata;

  g_return_val_if_fail (top, NULL);

  mdata = _yts_metadata_new_from_node (top, attributes);

  g_return_val_if_fail (YTS_IS_STATUS (mdata), NULL);

  return (YtsStatus*) mdata;
}

