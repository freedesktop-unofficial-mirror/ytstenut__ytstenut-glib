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

/*
 * Based on clutter-debug.h
 */

/**
 * SECTION:yts-debug
 * @short_description: Debugging API
 *
 * This section list available debugging API
 */

#ifndef YTS_DEBUG_H
#define YTS_DEBUG_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * YtsProtocol:
 * @YTS_DEBUG_CLIENT: debug client
 * @YTS_DEBUG_ROSTER: debug roster
 * @YTS_DEBUG_STATUS: debug status
 * @YTS_DEBUG_TP: debug telepathy
 * @YTS_DEBUG_MANAGER: debug manager
 * @YTS_DEBUG_MESSAGE: debug messages
 * @YTS_DEBUG_FILE_TRANSFER: debug file transfer
 * @YTS_DEBUG_CONTACT: debug contact
 *
 */
typedef enum {
  YTS_DEBUG_CLIENT        = 0x001,
  YTS_DEBUG_ROSTER        = 0x002,
  YTS_DEBUG_STATUS        = 0x004,
  YTS_DEBUG_TP            = 0x008,
  YTS_DEBUG_MANAGER       = 0x010,
  YTS_DEBUG_MESSAGE       = 0x020,
  YTS_DEBUG_FILE_TRANSFER = 0x080,
  YTS_DEBUG_CONTACT       = 0x100,
  YTS_DEBUG_CONNECTION    = 0x200
} YtsDebugFlags;

#ifdef YTS_ENABLE_DEBUG

/**
 * YTS_NOTE:
 * @type: the note type
 * @x: the note text, printf-like formatting allowed
 * @a...: parmaters, if any
 *
 * Prints a note to stdout
 */

/**
 * YTS_TIMESTAMP:
 * @type: the note type
 * @x: the note text, printf-like formatting allowed
 * @a...: parmaters, if any
 *
 * Prints a note with a timestamp to stdout.
 */

/**
 * YTS_MARK:
 *
 * Prints a mark to stdout.
 */

#ifdef __GNUC__
#define YTS_NOTE(type,x,a...)               G_STMT_START {  \
    if (yts_debug_flags & YTS_DEBUG_##type)                \
      { g_message (#type "(" G_STRLOC "): " x, ##a); }       \
                                                } G_STMT_END

#define YTS_TIMESTAMP(type,x,a...)             G_STMT_START {  \
    if (yts_debug_flags & YTS_DEBUG_##type)                   \
      { g_message ( #type " %li("  G_STRLOC "): "               \
                       x, yts_get_timestamp(), ##a); }         \
                                                   } G_STMT_END
#else
/* Try the C99 version; unfortunately, this does not allow us to pass
 * empty arguments to the macro, which means we have to
 * do an intemediate printf.
 */
#define YTS_NOTE(type,...)               G_STMT_START {  \
    if (yts_debug_flags & YTS_DEBUG_##type)                \
      {                                                      \
	  gchar * _fmt = g_strdup_printf (__VA_ARGS__);      \
          g_message ( #type  "(" G_STRLOC "): %s",_fmt);     \
          g_free (_fmt);                                     \
	}                                                    \
                                                } G_STMT_END

#define YTS_TIMESTAMP(type,...)             G_STMT_START {     \
    if (yts_debug_flags & YTS_DEBUG_##type)                   \
	{                                                       \
	  gchar * _fmt = g_strdup_printf (__VA_ARGS__);         \
          g_message ( #type  " %li("  G_STRLOC "): %s",         \
                      yts_get_timestamp(), _fmt);              \
          g_free (_fmt);                                        \
	}                                                       \
                                                   } G_STMT_END
#endif

#define YTS_MARK()      YTS_NOTE(MISC, "== mark ==")

#else /* !YTS_ENABLE_DEBUG */

#define YTS_NOTE(type,...)         G_STMT_START { } G_STMT_END
#define YTS_MARK()                 G_STMT_START { } G_STMT_END
#define YTS_TIMESTAMP(type,...)    G_STMT_START { } G_STMT_END

#endif /* YTS_ENABLE_DEBUG */

extern guint yts_debug_flags;

G_END_DECLS

#endif /* YTS_DEBUG_H */
