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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "ytstenut-internal.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0main\0"G_STRLOC

static const GDebugKey _debug_keys[] = {
  { "brief",            YTS_DEBUG_BRIEF },            /* Brief debug output. */
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
log_brief (GLogLevelFlags  log_level,
           char const     *context,
           char const     *message)
{
  char const *level;

  if (G_LOG_LEVEL_ERROR & log_level)
    level = "error";
  else if (G_LOG_LEVEL_CRITICAL & log_level)
    level = "critical";
  else if (G_LOG_LEVEL_WARNING & log_level)
    level = "warning";
  else if (G_LOG_LEVEL_MESSAGE & log_level)
    level = "message";
  else if (G_LOG_LEVEL_INFO & log_level)
    level = "info";
  else if (G_LOG_LEVEL_DEBUG & log_level)
    level = "debug";
  else
    level = "unknown";

  fprintf (stderr, "%s: %s: %s\n", context, level, message);
}

static void
log_default (char const     *log_domain,
             GLogLevelFlags  log_level,
             char const     *topic,
             char const     *location,
             char const     *message_)
{
  char *message;

  /* Prepend context to message. */
  if (location) {
    message = g_strdup_printf ("%s: %s", location, message_);
  } else {
    message = g_strdup_printf ("%s: %s", topic, message_);
  }

  g_log_default_handler (log_domain, log_level, message, NULL);
  g_free (message);
}

static void
_log_handler (char const      *log_domain,
              GLogLevelFlags   log_level,
              char const      *message,
              gpointer         user_data)
{
  char const  *topic;
  char const  *location = NULL;
  unsigned     topic_flag = 0;

  /* A bit of a hack, the domain has the format "ytstenut\0topic",
   * so look what's past the \0 for the topic. */
  topic = &log_domain[strlen (log_domain) + 1];

  if (0 != g_strcmp0 ("unspecified", topic)) {
    location = &topic[strlen (topic) + 1];
    if (location[0] == '.' && location[1] == '/')
      location += 2;
  }

  if (_debug_flags) {

    /* == We are in debug mode. == */

    for (unsigned i = 1; i < G_N_ELEMENTS (_debug_keys); i++) {
      if (0 == g_strcmp0 (topic, _debug_keys[i].key)) {
        topic_flag = _debug_keys[i].value;
        break;
      }
    }

    if (_debug_flags & topic_flag) {

      if (YTS_DEBUG_BRIEF & _debug_flags) {

        log_brief (log_level,
                   location ? location : topic,
                   message);

      } else {

        log_default (log_domain, log_level, topic, location, message);
      }
    }

  } else {

    /* == Not in debug mode ==
     * Only print debug messages (there should not be any in a tarball release),
     * and exceptions. */
    GLogLevelFlags log_mask = G_LOG_LEVEL_ERROR |
                              G_LOG_LEVEL_CRITICAL |
                              G_LOG_LEVEL_WARNING |
                              G_LOG_LEVEL_DEBUG;

    if (log_level & log_mask) {
      log_default (log_domain, log_level, topic, location, message);
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

  g_log_set_handler (PACKAGE, G_LOG_LEVEL_MASK, _log_handler, NULL);

  is_initialized = true;
  return true;
}

YtsDebugFlags
ytstenut_get_debug_flags (void)
{
  return _debug_flags;
}

#if 0

/* This is just for testing the above code standalone. */

int
main (int     argc,
      char  **argv)
{
  ytstenut_init ();

  g_warning ("foo");

  printf ("=== %s\n", FOO_LOG_DOMAIN);

  return 0;
}
#endif

