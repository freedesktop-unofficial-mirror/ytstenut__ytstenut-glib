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

#include "config.h"

#include "yts-message.h"
#include "yts-metadata-internal.h"

static void yts_message_dispose (GObject *object);
static void yts_message_finalize (GObject *object);
static void yts_message_constructed (GObject *object);
static void yts_message_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec);
static void yts_message_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec);

G_DEFINE_TYPE (YtsMessage, yts_message, YTS_TYPE_METADATA);

#define YTS_MESSAGE_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_MESSAGE, YtsMessagePrivate))

struct _YtsMessagePrivate
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
yts_message_class_init (YtsMessageClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  g_type_class_add_private (klass, sizeof (YtsMessagePrivate));

  object_class->dispose      = yts_message_dispose;
  object_class->finalize     = yts_message_finalize;
  object_class->constructed  = yts_message_constructed;
  object_class->get_property = yts_message_get_property;
  object_class->set_property = yts_message_set_property;
}

static void
yts_message_constructed (GObject *object)
{
  if (G_OBJECT_CLASS (yts_message_parent_class)->constructed)
    G_OBJECT_CLASS (yts_message_parent_class)->constructed (object);
}

static void
yts_message_get_property (GObject    *object,
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
yts_message_set_property (GObject      *object,
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
yts_message_init (YtsMessage *self)
{
  self->priv = YTS_MESSAGE_GET_PRIVATE (self);
}

static void
yts_message_dispose (GObject *object)
{
  YtsMessage        *self = (YtsMessage*) object;
  YtsMessagePrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  G_OBJECT_CLASS (yts_message_parent_class)->dispose (object);
}

static void
yts_message_finalize (GObject *object)
{
  G_OBJECT_CLASS (yts_message_parent_class)->finalize (object);
}

/**
 * yts_message_new:
 * @attributes: NULL terminated array of name/value pairs; can be %NULL
 *
 * Constructs a new #YtsMessage object, setting the top level attributes.
 *
 * Returns: (transfer full): newly allocated #YtsMessage object.
 */
YtsMessage *
yts_message_new (const char ** attributes)
{
  RestXmlNode  *top = rest_xml_node_add_child (NULL, "message");
  YtsMetadata *mdata;

  g_return_val_if_fail (top, NULL);

  mdata = yts_metadata_new_from_node (top, attributes);

  g_return_val_if_fail (YTS_IS_MESSAGE (mdata), NULL);

  return (YtsMessage*) mdata;
}

