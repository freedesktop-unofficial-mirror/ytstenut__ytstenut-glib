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

#ifndef YTSTENUT_INTERNAL_H
#define YTSTENUT_INTERNAL_H

#include <stdbool.h>

typedef enum {
  YTS_DEBUG_BRIEF             = 1 << 0, /* Brief debug output. */
  YTS_DEBUG_CLIENT            = 1 << 1,
  YTS_DEBUG_CONTACT           = 1 << 2,
  YTS_DEBUG_FILE_TRANSFER     = 1 << 3,
  YTS_DEBUG_MAIN              = 1 << 4,
  YTS_DEBUG_PROXY             = 1 << 5,
  YTS_DEBUG_PROXY_SERVICE     = 1 << 6,
  YTS_DEBUG_ROSTER            = 1 << 7,
  YTS_DEBUG_SERVICE_ADAPTER   = 1 << 8,
  YTS_DEBUG_SERVICE           = 1 << 9,
  YTS_DEBUG_TELEPATHY         = 1 << 10,
  YTS_DEBUG_UNSPECIFIED       = 1 << 11
} YtsDebugFlags;

bool
ytstenut_init (void);

YtsDebugFlags
ytstenut_get_debug_flags (void);

#define ERROR(format, ...) \
  do \
    { \
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, "%s: " format, \
          G_STRFUNC, ##__VA_ARGS__); \
      g_assert_not_reached (); \
    } \
  while (0)
#define CRITICAL(format, ...) \
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "%s: " format, \
      G_STRFUNC, ##__VA_ARGS__)
#define WARNING(format, ...) \
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "%s: " format, \
      G_STRFUNC, ##__VA_ARGS__)
#define MESSAGE(format, ...) \
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, "%s: " format, \
      G_STRFUNC, ##__VA_ARGS__)
#define INFO(format, ...) \
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "%s: " format, \
      G_STRFUNC, ##__VA_ARGS__)
#define DEBUG(format, ...) \
  g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "%s: " format, \
      G_STRFUNC, ##__VA_ARGS__)

#endif /* YTSTENUT_INTERNAL_H */

