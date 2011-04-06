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
 * SECTION:ytsg-debug
 * @short_description: Debugging API
 *
 * This section list available debugging API
 */

#ifndef _YTSG_DEBUG_H
#define _YTSG_DEBUG_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * YtsgProtocol:
 * @YTSG_DEBUG_CLIENT: debug client
 * @YTSG_DEBUG_ROSTER: debug roster
 * @YTSG_DEBUG_STATUS: debug status
 * @YTSG_DEBUG_TP: debug telepathy
 * @YTSG_DEBUG_MANAGER: debug manager
 * @YTSG_DEBUG_MESSAGE: debug messages
 * @YTSG_DEBUG_FILE_TRANSFER: debug file transfer
 * @YTSG_DEBUG_CONTACT: debug contact
 *
 */
typedef enum {
  YTSG_DEBUG_CLIENT        = 0x001,
  YTSG_DEBUG_ROSTER        = 0x002,
  YTSG_DEBUG_STATUS        = 0x004,
  YTSG_DEBUG_TP            = 0x008,
  YTSG_DEBUG_MANAGER       = 0x010,
  YTSG_DEBUG_MESSAGE       = 0x020,
  YTSG_DEBUG_FILE_TRANSFER = 0x080,
  YTSG_DEBUG_CONTACT       = 0x100
} YtsgDebugFlags;

#ifdef YTSG_ENABLE_DEBUG

/**
 * YTSG_NOTE:
 * @type: the note type
 * @x: the note text, printf-like formatting allowed
 * @a...: parmaters, if any
 *
 * Prints a note to stdout
 */

/**
 * YTSG_TIMESTAMP:
 * @type: the note type
 * @x: the note text, printf-like formatting allowed
 * @a...: parmaters, if any
 *
 * Prints a note with a timestamp to stdout.
 */

/**
 * YTSG_MARK:
 *
 * Prints a mark to stdout.
 */

#ifdef __GNUC__
#define YTSG_NOTE(type,x,a...)               G_STMT_START {  \
    if (ytsg_debug_flags & YTSG_DEBUG_##type)                \
      { g_message (#type "(" G_STRLOC "): " x, ##a); }       \
                                                } G_STMT_END

#define YTSG_TIMESTAMP(type,x,a...)             G_STMT_START {  \
    if (ytsg_debug_flags & YTSG_DEBUG_##type)                   \
      { g_message ( #type " %li("  G_STRLOC "): "               \
                       x, ytsg_get_timestamp(), ##a); }         \
                                                   } G_STMT_END
#else
/* Try the C99 version; unfortunately, this does not allow us to pass
 * empty arguments to the macro, which means we have to
 * do an intemediate printf.
 */
#define YTSG_NOTE(type,...)               G_STMT_START {  \
    if (ytsg_debug_flags & YTSG_DEBUG_##type)                \
      {                                                      \
	  gchar * _fmt = g_strdup_printf (__VA_ARGS__);      \
          g_message ( #type  "(" G_STRLOC "): %s",_fmt);     \
          g_free (_fmt);                                     \
	}                                                    \
                                                } G_STMT_END

#define YTSG_TIMESTAMP(type,...)             G_STMT_START {     \
    if (ytsg_debug_flags & YTSG_DEBUG_##type)                   \
	{                                                       \
	  gchar * _fmt = g_strdup_printf (__VA_ARGS__);         \
          g_message ( #type  " %li("  G_STRLOC "): %s",         \
                      ytsg_get_timestamp(), _fmt);              \
          g_free (_fmt);                                        \
	}                                                       \
                                                   } G_STMT_END
#endif

#define YTSG_MARK()      YTSG_NOTE(MISC, "== mark ==")

#else /* !YTSG_ENABLE_DEBUG */

#define YTSG_NOTE(type,...)         G_STMT_START { } G_STMT_END
#define YTSG_MARK()                 G_STMT_START { } G_STMT_END
#define YTSG_TIMESTAMP(type,...)    G_STMT_START { } G_STMT_END

#endif /* YTSG_ENABLE_DEBUG */

extern guint ytsg_debug_flags;

G_END_DECLS

#endif /* _YTSG_DEBUG_H */
