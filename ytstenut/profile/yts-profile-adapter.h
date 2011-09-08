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

#ifndef YTS_PROFILE_ADAPTER_H
#define YTS_PROFILE_ADAPTER_H

#include <stdbool.h>
#include <glib-object.h>
#include <ytstenut/yts-service-adapter.h>

G_BEGIN_DECLS

#define YTS_TYPE_PROFILE_ADAPTER yts_profile_adapter_get_type()

#define YTS_PROFILE_ADAPTER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_PROFILE_ADAPTER, YtsProfileAdapter))

#define YTS_PROFILE_ADAPTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_PROFILE_ADAPTER, YtsProfileAdapterClass))

#define YTS_IS_PROFILE_ADAPTER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_PROFILE_ADAPTER))

#define YTS_IS_PROFILE_ADAPTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_PROFILE_ADAPTER))

#define YTS_PROFILE_ADAPTER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_PROFILE_ADAPTER, YtsProfileAdapterClass))

typedef struct {
  YtsServiceAdapter parent;
} YtsProfileAdapter;

typedef struct {
  YtsServiceAdapterClass parent;
} YtsProfileAdapterClass;

GType
yts_profile_adapter_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* YTS_PROFILE_ADAPTER_H */

