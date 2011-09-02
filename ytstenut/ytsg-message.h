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

#ifndef _YTSG_MESSAGE_H
#define _YTSG_MESSAGE_H

#include <ytstenut/ytsg-metadata.h>

G_BEGIN_DECLS

#define YTSG_TYPE_MESSAGE                                               \
   (ytsg_message_get_type())
#define YTSG_MESSAGE(obj)                                               \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTSG_TYPE_MESSAGE,                      \
                                YtsgMessage))
#define YTSG_MESSAGE_CLASS(klass)                                       \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTSG_TYPE_MESSAGE,                         \
                             YtsgMessageClass))
#define YTSG_IS_MESSAGE(obj)                                            \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTSG_TYPE_MESSAGE))
#define YTSG_IS_MESSAGE_CLASS(klass)                                    \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTSG_TYPE_MESSAGE))
#define YTSG_MESSAGE_GET_CLASS(obj)                                     \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTSG_TYPE_MESSAGE,                       \
                               YtsgMessageClass))

typedef struct _YtsgMessage        YtsgMessage;
typedef struct _YtsgMessageClass   YtsgMessageClass;
typedef struct _YtsgMessagePrivate YtsgMessagePrivate;

/**
 * YtsgMessageClass:
 *
 * #YtsgMessage class.
 */
struct _YtsgMessageClass
{
  /*< private >*/
  YtsgMetadataClass parent_class;
};

/**
 * YtsgMessage:
 *
 * Encapsulates a Ytstenut message, either being sent to a given #YtsgService or
 * received by #YtsgClient.
 */
struct _YtsgMessage
{
  /*< private >*/
  YtsgMetadata parent;

  /*< private >*/
  YtsgMessagePrivate *priv;
};

GType ytsg_message_get_type (void) G_GNUC_CONST;

YtsgMessage *ytsg_message_new (const char ** attributes);

G_END_DECLS

#endif /* _YTSG_MESSAGE_H */
