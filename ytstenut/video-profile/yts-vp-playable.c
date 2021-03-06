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

#include "yts-vp-playable.h"

G_DEFINE_INTERFACE (YtsVPPlayable,
                    yts_vp_playable,
                    G_TYPE_OBJECT)

static void
yts_vp_playable_default_init (YtsVPPlayableInterface *interface)
{
  GParamFlags param_flags;

  param_flags = G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
                G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB;

  g_object_interface_install_property (interface,
                                       g_param_spec_double ("duration", "", "",
                                                            0.0, G_MAXDOUBLE, 0.0,
                                                            param_flags));

  g_object_interface_install_property (interface,
                                       g_param_spec_boxed ("metadata", "", "",
                                                            G_TYPE_HASH_TABLE,
                                                            param_flags));

  g_object_interface_install_property (interface,
                                       g_param_spec_double ("position", "", "",
                                                            0.0, G_MAXDOUBLE, 0.0,
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_STATIC_NAME |
                                                            G_PARAM_STATIC_NICK |
                                                            G_PARAM_STATIC_BLURB));

  g_object_interface_install_property (interface,
                                       g_param_spec_string ("thumbnail", "", "",
                                                            NULL,
                                                            param_flags));

  g_object_interface_install_property (interface,
                                       g_param_spec_string ("title", "", "",
                                                            NULL,
                                                            param_flags));

  g_object_interface_install_property (interface,
                                       g_param_spec_string ("uri", "", "",
                                                            NULL,
                                                            param_flags));
}

double
yts_vp_playable_get_duration (YtsVPPlayable *self)
{
  double duration;

  g_return_val_if_fail (YTS_VP_IS_PLAYABLE (self), 0.0);

  duration = 0.0;
  g_object_get (G_OBJECT (self), "duration", &duration, NULL);
  return duration;
}

GHashTable *
yts_vp_playable_get_metadata (YtsVPPlayable *self)
{
  GHashTable *metadata;

  g_return_val_if_fail (YTS_VP_IS_PLAYABLE (self), NULL);

  metadata = NULL;
  g_object_get (G_OBJECT (self), "metadata", &metadata, NULL);
  return metadata;
}

char const *
yts_vp_playable_get_metadata_attribute (YtsVPPlayable *self,
                                         char const     *attribute)
{
  GHashTable  *metadata;
  char const  *value;

  g_return_val_if_fail (YTS_VP_IS_PLAYABLE (self), NULL);
  g_return_val_if_fail (YTS_VP_IS_PLAYABLE (attribute), NULL);

  metadata = yts_vp_playable_get_metadata (self);
  g_return_val_if_fail (metadata, NULL);

  value = (char const *) g_hash_table_lookup (metadata, attribute);
  g_boxed_free (G_TYPE_HASH_TABLE, metadata);

  return value;
}

double
yts_vp_playable_get_position (YtsVPPlayable *self)
{
  double position;

  g_return_val_if_fail (YTS_VP_IS_PLAYABLE (self), 0.0);

  position = 0.0;
  g_object_get (G_OBJECT (self), "position", &position, NULL);
  return position;
}

void
yts_vp_playable_set_position (YtsVPPlayable *self,
                               double          position)
{
  g_return_if_fail (YTS_VP_IS_PLAYABLE (self));

  g_object_set (G_OBJECT (self), "position", position, NULL);
}

char *
yts_vp_playable_get_thumbnail (YtsVPPlayable *self)
{
  char *thumbnail;

  g_return_val_if_fail (YTS_VP_IS_PLAYABLE (self), NULL);

  thumbnail = NULL;
  g_object_get (G_OBJECT (self), "thumbnail", &thumbnail, NULL);
  return thumbnail;
}

char *
yts_vp_playable_get_title (YtsVPPlayable *self)
{
  char *title;

  g_return_val_if_fail (YTS_VP_IS_PLAYABLE (self), NULL);

  title = NULL;
  g_object_get (G_OBJECT (self), "title", &title, NULL);
  return title;
}

char *
yts_vp_playable_get_uri (YtsVPPlayable *self)
{
  char *uri;

  g_return_val_if_fail (YTS_VP_IS_PLAYABLE (self), NULL);

  uri = NULL;
  g_object_get (G_OBJECT (self), "uri", &uri, NULL);
  return uri;
}

