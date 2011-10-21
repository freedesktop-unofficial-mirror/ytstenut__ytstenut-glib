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

#include <string.h>
#include <glib.h>

#include "ytstenut-internal.h"
#include "config.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0main"

static const GDebugKey _debug_keys[] = {
  { "client",           YTS_DEBUG_CLIENT },
  { "contact",          YTS_DEBUG_CONTACT },
  { "file-transfer",    YTS_DEBUG_FILE_TRANSFER },
  { "main",             YTS_DEBUG_MAIN },
  { "proxy",            YTS_DEBUG_PROXY },
  { "proxy-service",    YTS_DEBUG_PROXY_SERVICE },
  { "roster",           YTS_DEBUG_ROSTER },
  { "service-adapter",  YTS_DEBUG_SERVICE_ADAPTER },
  { "service",          YTS_DEBUG_SERVICE },
  { "telepathy",        YTS_DEBUG_TELEPATHY },
  { "unspecified",      YTS_DEBUG_UNSPECIFIED }
};

static YtsDebugFlags _debug_flags = 0;

static void
_log (char const      *log_domain,
      GLogLevelFlags   log_level,
      char const      *message,
      gpointer         test_topic)
{
  char const  *topic;
  unsigned     topic_flag = 0;

  /* A bit of a hack, the domain has the format "ytstenut\0topic",
   * so look what's past the \0 for the topic. */
  topic = &log_domain[strlen (log_domain) + 1];

  if (_debug_flags) {

    /* We are in debug mode. */
    for (unsigned i = 0; i < G_N_ELEMENTS (_debug_keys); i++) {
      if (0 == g_strcmp0 (topic, _debug_keys[i].key)) {
        topic_flag = _debug_keys[i].value;
        break;
      }
    }

    if (_debug_flags & topic_flag) {
      /* Prepend topic on the message. */
      char *msg = g_strdup_printf ("%s: %s", topic, message);
      g_log_default_handler (log_domain, log_level, msg, NULL);
      g_free (msg);
    }

  } else {

    /* Not in debug mode, only print debug messages
     * (there should not be any in a tarball release), and exceptions. */
    GLogLevelFlags log_mask = G_LOG_LEVEL_ERROR |
                              G_LOG_LEVEL_CRITICAL |
                              G_LOG_LEVEL_WARNING |
                              G_LOG_LEVEL_DEBUG;
    if (log_level & log_mask) {
      /* Prepend topic on the message. */
      char *msg = g_strdup_printf ("%s: %s", topic, message);
      g_log_default_handler (log_domain, log_level, msg, NULL);
      g_free (msg);
    }
  }
}

bool
ytstenut_init (void)
{
  static bool is_initialized = false;

  char const *yts_debug;

  if (is_initialized) {
    g_warning ("%s: library already initialized.", G_STRLOC);
    return false;
  }

  yts_debug = g_getenv ("YTS_DEBUG");
  if (yts_debug) {
    _debug_flags = g_parse_debug_string (yts_debug,
                                         _debug_keys,
                                         G_N_ELEMENTS (_debug_keys));
  }

  g_log_set_handler (PACKAGE, G_LOG_LEVEL_MASK, _log, NULL);

  is_initialized = true;
  return true;
}

YtsDebugFlags
ytstenut_get_debug_flags (void)
{
  return _debug_flags;
}

