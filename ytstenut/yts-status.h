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

#ifndef _YTS_STATUS_H
#define _YTS_STATUS_H

#include <ytstenut/yts-metadata.h>

G_BEGIN_DECLS

#define YTS_TYPE_STATUS                                                \
   (yts_status_get_type())
#define YTS_STATUS(obj)                                                \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTS_TYPE_STATUS,                       \
                                YtsStatus))
#define YTS_STATUS_CLASS(klass)                                        \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTS_TYPE_STATUS,                          \
                             YtsStatusClass))
#define YTS_IS_STATUS(obj)                                             \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTS_TYPE_STATUS))
#define YTS_IS_STATUS_CLASS(klass)                                     \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTS_TYPE_STATUS))
#define YTS_STATUS_GET_CLASS(obj)                                      \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTS_TYPE_STATUS,                        \
                               YtsStatusClass))

typedef struct _YtsStatus        YtsStatus;
typedef struct _YtsStatusClass   YtsStatusClass;
typedef struct _YtsStatusPrivate YtsStatusPrivate;

/**
 * YtsStatusClass:
 *
 * #YtsStatus class.
 */
struct _YtsStatusClass
{
  /*< private >*/
  YtsMetadataClass parent_class;
};

/**
 * YtsStatus:
 *
 * Status of a #YtsService or #YtsClient.
 */
struct _YtsStatus
{
  /*< private >*/
  YtsMetadata parent;

  /*< private >*/
  YtsStatusPrivate *priv;
};

GType yts_status_get_type (void) G_GNUC_CONST;

YtsStatus *yts_status_new (const char **attributes);

G_END_DECLS

#endif /* _YTS_STATUS_H */
