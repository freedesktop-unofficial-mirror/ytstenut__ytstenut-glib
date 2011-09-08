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

#ifndef MOCK_PLAYER_H
#define MOCK_PLAYER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MOCK_TYPE_PLAYER mock_player_get_type()

#define MOCK_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOCK_TYPE_PLAYER, MockPlayer))

#define MOCK_PLAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MOCK_TYPE_PLAYER, MockPlayerClass))

#define MOCK_IS_PLAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOCK_TYPE_PLAYER))

#define MOCK_IS_PLAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOCK_TYPE_PLAYER))

#define MOCK_PLAYER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOCK_TYPE_PLAYER, MockPlayerClass))

typedef struct {
  GObject parent;
} MockPlayer;

typedef struct {
  GObjectClass parent;
} MockPlayerClass;

GType
mock_player_get_type (void) G_GNUC_CONST;

MockPlayer *
mock_player_new (void);

G_END_DECLS

#endif /* MOCK_PLAYER_H */

