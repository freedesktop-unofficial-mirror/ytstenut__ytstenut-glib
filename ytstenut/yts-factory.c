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

static FactoryEntry *_table = NULL;
static unsigned      _table_size = 0;

static int
_factory_entry_compare (FactoryEntry const *fe1,
                        FactoryEntry const *fe2)
{
  return strcmp (fe1->fqc_id, fe2->fqc_id);
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

  struct {
    char const *fqc_id;       /* Needs to be first so we can use strcmp. */
    GType       adapter_type;
    GType       proxy_type;
  } table[] = {
    { "org.freedesktop.ytstenut.VideoProfile.Player",
       yts_vp_player_adapter_get_type (),
       yts_vp_player_proxy_get_type () },
    { "org.freedesktop.ytstenut.VideoProfile.Transcript",
       yts_vp_transcript_adapter_get_type (),
       yts_vp_transcript_proxy_get_type () }
  };

  object_class->dispose = _dispose;

  if (NULL == _table) {
    _table = g_memdup (table, sizeof (table));
    _table_size = G_N_ELEMENTS (table);
    qsort (_table,
           _table_size,
           sizeof (FactoryEntry),
           (__compar_fn_t) _factory_entry_compare);
  }
}

static void
yts_factory_init (YtsFactory *self)
{
}

bool
yts_factory_handles_fqc_id (YtsFactory const  *self,
                            char const        *fqc_id)
{
  FactoryEntry const *entry;
  FactoryEntry        needle;

  needle.fqc_id = fqc_id;
  entry = bsearch (&needle,
                   _table,
                   _table_size,
                   sizeof (FactoryEntry),
                   (__compar_fn_t) strcmp);

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
                   _table,
                   _table_size,
                   sizeof (FactoryEntry),
                   (__compar_fn_t) _factory_entry_compare);

  g_return_val_if_fail (entry, G_TYPE_INVALID);

  return entry->proxy_gtype;
}

GType
yts_factory_get_adapter_gtype_for_fqc_id (YtsFactory const  *self,
                                          char const        *fqc_id)
{
  FactoryEntry const *entry;
  FactoryEntry        needle;

  needle.fqc_id = fqc_id;
  entry = bsearch (&needle,
                   _table,
                   _table_size,
                   sizeof (FactoryEntry),
                   (__compar_fn_t) _factory_entry_compare);

  g_return_val_if_fail (entry, G_TYPE_INVALID);

  return entry->adapter_gtype;
}

