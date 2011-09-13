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

#ifndef YTS_PROXY_H
#define YTS_PROXY_H

#include <glib-object.h>

G_BEGIN_DECLS

#define YTS_TYPE_PROXY yts_proxy_get_type()

#define YTS_PROXY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), YTS_TYPE_PROXY, YtsProxy))

#define YTS_PROXY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), YTS_TYPE_PROXY, YtsProxyClass))

#define YTS_IS_PROXY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YTS_TYPE_PROXY))

#define YTS_IS_PROXY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), YTS_TYPE_PROXY))

#define YTS_PROXY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), YTS_TYPE_PROXY, YtsProxyClass))

typedef struct {
  GObject parent;
} YtsProxy;

typedef struct {
  GObjectClass parent;

  /*< private >*/

  /* Signals */

  void
  (*service_event) (YtsProxy   *self,
                    char const  *aspect,
                    GVariant    *arguments);
  void
  (*service_response) (YtsProxy  *self,
                       char const *invocation_id,
                       GVariant   *response);

} YtsProxyClass;

GType
yts_proxy_get_type (void) G_GNUC_CONST;

char *
yts_proxy_get_fqc_id (YtsProxy *self);

char *
yts_proxy_create_invocation_id (YtsProxy *self);

void
yts_proxy_invoke (YtsProxy  *self,
                   char const *invocation_id,
                   char const *aspect,
                   GVariant   *arguments);

G_END_DECLS

#endif /* YTS_PROXY_H */

