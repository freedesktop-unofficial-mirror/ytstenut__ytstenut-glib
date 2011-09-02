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

#ifndef _YTS_ROSTER_H
#define _YTS_ROSTER_H

#include <glib-object.h>
#include <ytstenut/yts-caps.h>
#include <ytstenut/yts-contact.h>
#include <ytstenut/yts-types.h>

G_BEGIN_DECLS

#define YTS_TYPE_ROSTER                                                \
   (yts_roster_get_type())
#define YTS_ROSTER(obj)                                                \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                  \
                                YTS_TYPE_ROSTER,                       \
                                YtsRoster))
#define YTS_ROSTER_CLASS(klass)                                        \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             YTS_TYPE_ROSTER,                          \
                             YtsRosterClass))
#define YTS_IS_ROSTER(obj)                                             \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                YTS_TYPE_ROSTER))
#define YTS_IS_ROSTER_CLASS(klass)                                     \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             YTS_TYPE_ROSTER))
#define YTS_ROSTER_GET_CLASS(obj)                                      \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                                   \
                               YTS_TYPE_ROSTER,                        \
                               YtsRosterClass))

typedef struct _YtsRoster        YtsRoster;
typedef struct _YtsRosterClass   YtsRosterClass;
typedef struct _YtsRosterPrivate YtsRosterPrivate;

/**
 * YtsRosterClass:
 *
 * #YtsRoster class.
 */
struct _YtsRosterClass
{
  /*< private >*/
  GObjectClass parent_class;
};

/**
 * YtsRoster:
 *
 * Represents a roster of #YtsContact<!-- -->s known to #YtsClient.
 */
struct _YtsRoster
{
  /*< private >*/
  GObject parent;

  /*< private >*/
  YtsRosterPrivate *priv;
};

GType yts_roster_get_type (void) G_GNUC_CONST;

GHashTable        *yts_roster_get_contacts              (YtsRoster *roster);
YtsContact       *yts_roster_find_contact_by_jid       (YtsRoster *roster,
                                                          const char *jid);
YtsContact       *yts_roster_find_contact_by_capability (YtsRoster *roster,
                                                           YtsCaps    capability);
YtsClient        *yts_roster_get_client (YtsRoster *roster);

G_END_DECLS

#endif /* _YTS_ROSTER_H */
