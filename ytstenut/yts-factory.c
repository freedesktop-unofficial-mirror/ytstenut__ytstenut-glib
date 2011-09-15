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

#include <stdlib.h>
#include <string.h>

#include "yts-factory.h"

#include "video-profile/yts-vp-player-adapter.h"
#include "video-profile/yts-vp-player-proxy.h"
#include "video-profile/yts-vp-transcript-adapter.h"
#include "video-profile/yts-vp-transcript-proxy.h"
#include "config.h"

G_DEFINE_ABSTRACT_TYPE (YtsFactory, yts_factory, G_TYPE_OBJECT)

typedef struct {
  char const *fqc_id;
  GType       adapter_gtype;
  GType       proxy_gtype;
} FactoryEntry;

static GArray *_table = NULL;

static int
_factory_entry_compare (const void *fe1,
                        const void *fe2)
{
  return strcmp (((FactoryEntry *) fe1)->fqc_id,
                 ((FactoryEntry *) fe2)->fqc_id);
}

static void
_dispose (GObject *object)
{
  G_OBJECT_CLASS (yts_factory_parent_class)->dispose (object);
}

static void
yts_factory_class_init (YtsFactoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  FactoryEntry table[] = {
    { "org.freedesktop.ytstenut.VideoProfile.Player",
       yts_vp_player_adapter_get_type (),
       yts_vp_player_proxy_get_type () },
    { "org.freedesktop.ytstenut.VideoProfile.Transcript",
       yts_vp_transcript_adapter_get_type (),
       yts_vp_transcript_proxy_get_type () }
  };

  object_class->dispose = _dispose;

  if (NULL == _table) {
    _table = g_array_sized_new (false,
                                false,
                                sizeof (FactoryEntry),
                                G_N_ELEMENTS (table));
    memcpy (_table->data, table, sizeof (table));
    _table->len = G_N_ELEMENTS (table);
    g_array_sort (_table, _factory_entry_compare);
  }
}

static void
yts_factory_init (YtsFactory *self)
{
}

bool
yts_factory_has_fqc_id (YtsFactory const  *self,
                        char const        *fqc_id)
{
  FactoryEntry const *entry;
  FactoryEntry        needle;

  needle.fqc_id = fqc_id;
  entry = bsearch (&needle,
                   _table->data,
                   _table->len,
                   sizeof (FactoryEntry),
                   _factory_entry_compare);

  return (bool) entry;
}

GType
yts_factory_get_proxy_gtype_for_fqc_id (YtsFactory const  *self,
                                        char const        *fqc_id)
{
  FactoryEntry const *entry;
  FactoryEntry        needle;

  needle.fqc_id = fqc_id;
  entry = bsearch (&needle,
                   _table->data,
                   _table->len,
                   sizeof (FactoryEntry),
                   _factory_entry_compare);

  if (entry) {
    return entry->proxy_gtype;
  }

  return G_TYPE_INVALID;
}

GType
yts_factory_get_adapter_gtype_for_fqc_id (YtsFactory const  *self,
                                          char const        *fqc_id)
{
  FactoryEntry const *entry;
  FactoryEntry        needle;

  needle.fqc_id = fqc_id;
  entry = bsearch (&needle,
                   _table->data,
                   _table->len,
                   sizeof (FactoryEntry),
                   _factory_entry_compare);

  if (entry) {
    return entry->adapter_gtype;
  }

  return G_TYPE_INVALID;
}

