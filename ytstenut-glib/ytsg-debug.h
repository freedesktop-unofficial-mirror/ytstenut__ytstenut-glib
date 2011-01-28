/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2011 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 *
 * Based on clutter-debug.h
 */

#ifndef _YTSG_DEBUG_H
#define _YTSG_DEBUG_H

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
  YTSG_DEBUG_CLIENT        = 0x01,
  YTSG_DEBUG_ROSTER        = 0x02,
  YTSG_DEBUG_STATUS        = 0x04,
  YTSG_DEBUG_TP            = 0x08,
  YTSG_DEBUG_MANAGER       = 0x10,
  YTSG_DEBUG_MESSAGE       = 0x20,
  YTSG_DEBUG_FILE_TRANSFER = 0x80
} YtsgDebugFlags;

#ifdef YTSG_ENABLE_DEBUG

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
