/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2011 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _YTSG_STATUS_H
#define _YTSG_STATUS_H

#include <ytstenut-glib/ytsg-metadata.h>

G_BEGIN_DECLS

#define YTSG_TYPE_STATUS                                                \
   (ytsg_status_get_type())
#define YTSG_STATUS(obj)                                                \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTSG_TYPE_STATUS,                       \
                                YtsgStatus))
#define YTSG_STATUS_CLASS(klass)                                        \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTSG_TYPE_STATUS,                          \
                             YtsgStatusClass))
#define YTSG_IS_STATUS(obj)                                             \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTSG_TYPE_STATUS))
#define YTSG_IS_STATUS_CLASS(klass)                                     \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTSG_TYPE_STATUS))
#define YTSG_STATUS_GET_CLASS(obj)                                      \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTSG_TYPE_STATUS,                        \
                               YtsgStatusClass))

typedef struct _YtsgStatus        YtsgStatus;
typedef struct _YtsgStatusClass   YtsgStatusClass;
typedef struct _YtsgStatusPrivate YtsgStatusPrivate;

struct _YtsgStatusClass
{
  YtsgMetadataClass parent_class;
};

struct _YtsgStatus
{
  YtsgMetadata parent;

  /*<private>*/
  YtsgStatusPrivate *priv;
};

GType ytsg_status_get_type (void) G_GNUC_CONST;

YtsgStatus *ytsg_status_new (const char **attributes);

gboolean    ytsg_status_equal (YtsgStatus *self, YtsgStatus *other);

G_END_DECLS

#endif /* _YTSG_STATUS_H */
