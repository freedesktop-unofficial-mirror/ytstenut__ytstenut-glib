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

#define _GNU_SOURCE /* for getresuid(), getresgid() */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#if 0
#include <glib/gi18n-lib.h>
#else
#define N_(x) x
#define _(x) x
#endif

#include <locale.h>
#include <glib.h>
#include <glib-object.h>
#include <unistd.h>
#include <string.h>

#include "ytsg-debug.h"
#include "ytsg-main.h"

static void ytsg_base_init (void);
static int  ytsg_init_real (GError **error);

gboolean        ytsg_is_initialized = FALSE;
static gboolean ytsg_fatal_warnings = FALSE;
static gboolean ytsg_args_parsed    = FALSE;

/*
 * Debugging machinery; based on Clutter.
 */
guint ytsg_debug_flags = 0;

#ifdef YTSG_ENABLE_DEBUG
static const GDebugKey ytsg_debug_keys[] = {
  { "client",          YTSG_DEBUG_CLIENT  },
  { "roster",          YTSG_DEBUG_ROSTER  },
  { "status",          YTSG_DEBUG_STATUS  },
  { "tp",              YTSG_DEBUG_TP      },
  { "manager",         YTSG_DEBUG_MANAGER },
  { "message",         YTSG_DEBUG_MESSAGE },
  { "file-transfer",   YTSG_DEBUG_FILE_TRANSFER   },
};
#endif /* YTSG_ENABLE_DEBUG */

#ifdef YTSG_ENABLE_DEBUG
static gboolean
ytsg_arg_debug_cb (const char *key, const char *value, gpointer user_data)
{
  ytsg_debug_flags |=
    g_parse_debug_string (value,
                          ytsg_debug_keys,
                          G_N_ELEMENTS (ytsg_debug_keys));
  return TRUE;
}

static gboolean
ytsg_arg_no_debug_cb (const char *key, const char *value, gpointer user_data)
{
  ytsg_debug_flags &=
    ~g_parse_debug_string (value,
                           ytsg_debug_keys,
                           G_N_ELEMENTS (ytsg_debug_keys));
  return TRUE;
}
#endif

static GOptionEntry ytsg_args[] = {
  { "g-fatal-warnings", 0, 0, G_OPTION_ARG_NONE, &ytsg_fatal_warnings,
    N_("Make all warnings fatal"), NULL },
#ifdef YTSG_ENABLE_DEBUG
  { "ytsg-debug", 0, 0, G_OPTION_ARG_CALLBACK, ytsg_arg_debug_cb,
    N_("Ytstenut-glib debugging flags to set"), "FLAGS" },
  { "ytsg-no-debug", 0, 0, G_OPTION_ARG_CALLBACK, ytsg_arg_no_debug_cb,
    N_("Ytstenut-glib debugging flags to unset"), "FLAGS" },
#endif /* YTSG_ENABLE_DEBUG */
  { NULL, },
};

/* pre_parse_hook: initialise variables depending on environment
 * variables; these variables might be overridden by the command
 * line arguments that are going to be parsed after.
 */
static gboolean
pre_parse_hook (GOptionContext  *context,
                GOptionGroup    *group,
                gpointer         data,
                GError         **error)
{
  const char *env_string;

  if (ytsg_is_initialized)
    return TRUE;

  if (setlocale (LC_ALL, "") == NULL)
    g_warning ("Locale not supported by C library.\n"
               "Using the fallback 'C' locale.");

#ifdef YTSG_ENABLE_DEBUG
  env_string = g_getenv ("YTSG_DEBUG");
  if (env_string != NULL)
    {
      ytsg_debug_flags =
        g_parse_debug_string (env_string,
                              ytsg_debug_keys,
                              G_N_ELEMENTS (ytsg_debug_keys));
      env_string = NULL;
    }
#endif /* YTSG_ENABLE_DEBUG */

  return TRUE;
}

/*
 * post_parse_hook -- initialize things that depend on command line arguments
 */
static gboolean
post_parse_hook (GOptionContext  *context,
                 GOptionGroup    *group,
                 gpointer         data,
                 GError         **error)
{
  if (ytsg_is_initialized)
    return TRUE;

  ytsg_args_parsed = TRUE;

  if (ytsg_fatal_warnings)
    {
      GLogLevelFlags fatal_mask;

      fatal_mask = g_log_set_always_fatal (G_LOG_FATAL_MASK);
      fatal_mask |= G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
      g_log_set_always_fatal (fatal_mask);
    }

  return ytsg_init_real (error);
}

/**
 * ytsg_get_option_group:
 *
 * Returns a #GOptionGroup for the command line arguments recognized
 * by Ytstenut-Glib. You should add this group to your #GOptionContext with
 * g_option_context_add_group(), if you are using g_option_context_parse()
 * to parse your commandline arguments.
 *
 * Calling g_option_context_parse() with Ytstenut-Glib's #GOptionGroup will
 * result in Ytstenut-Glib's initialization. That is, the following code:
 *
 * |[
 *   g_option_context_set_main_group (context, ytsg_get_option_group ());
 *   res = g_option_context_parse (context, &amp;argc, &amp;argc, NULL);
 * ]|
 *
 * is functionally equivalent to:
 *
 * |[
 *   ytsg_init (&amp;argc, &amp;argv);
 * ]|
 *
 * After g_option_context_parse() on a #GOptionContext containing the
 * Ytstenut-Glib #GOptionGroup has returned %TRUE, Ytstenut-Glib is guaranteed
 * to be initialized.
 *
 * Return value: (transfer full): a #GOptionGroup for the commandline arguments
 *   recognized by Ytstenut-Glib
 *
 * Since: 0.1
 */
GOptionGroup *
ytsg_get_option_group (void)
{
  GOptionGroup *group;

  ytsg_base_init ();

  group = g_option_group_new ("Ytstenut-glib",
                              _("Ytstenut-glib Options"),
                              _("Show Ytstenut-glib Options"),
                              NULL,
                              NULL);

  g_option_group_set_parse_hooks (group, pre_parse_hook, post_parse_hook);
  g_option_group_add_entries (group, ytsg_args);
#if 0
  g_option_group_set_translation_domain (group, GETTEXT_PACKAGE);
#endif
  return group;
}

/*
 * The standard GLib log handler is far too verbose.
 *
 * Keep it simple, send everything we log to stderr.
 */
static void
ytsg_loger (const gchar    *domain,
            GLogLevelFlags  level,
            const gchar    *message,
            gpointer        data)
{
  char *s = NULL;

  if (level & G_LOG_LEVEL_ERROR)
    s = g_strdup_printf ("Ytsg-Error: %s\n", message);
  else if (level & G_LOG_LEVEL_CRITICAL)
    s = g_strdup_printf ("Ytsg-Critical: %s\n", message);
  else if (level & G_LOG_LEVEL_WARNING)
    s = g_strdup_printf ("Ytsg-Warning: %s\n", message);
  else if (level & G_LOG_LEVEL_DEBUG)
    s = g_strdup_printf ("Ytsg-Debug: %s\n", message);
  else
    s = g_strdup_printf ("Ytsg: %s\n", message);

  write (STDERR_FILENO, s, strlen (s));

  g_free (s);
}

static void
ytsg_base_init (void)
{
  static gboolean initialised = FALSE;

  if (!initialised)
    {
      uid_t ruid;
      uid_t euid;
      uid_t suid;
      gid_t rgid;
      gid_t egid;
      gid_t sgid;
      gboolean privs_ok = TRUE;

      if (getresuid (&ruid, &euid, &suid) != 0)
        {
          if (getuid () != geteuid ())
            privs_ok = FALSE;
        }
      else if (ruid == 0 || ruid != euid || ruid != suid)
        privs_ok = FALSE;

      if (getresgid (&rgid, &egid, &sgid) != 0)
        {
          if (getgid () != getegid ())
            privs_ok = FALSE;
        }
      else if (rgid == 0 || rgid != egid || rgid != sgid)
        privs_ok = FALSE;

      if (!privs_ok)
        {
          g_error ("Yetstenut applications can only run as regular user; they "
                   "they cannot be run with escalated privilidges or as root");
        }

      initialised = TRUE;

#if 0
      bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
      bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
      /* initialise GLib type system */
      g_type_init ();

      g_log_set_handler ("YtstenutGlib",
                         G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL,
                         ytsg_loger, NULL);
    }
}

static int
ytsg_init_real (GError **error)
{
  ytsg_is_initialized = TRUE;

  return 1;
}

static gboolean
ytsg_parse_args (int *argc, char ***argv)
{
  GOptionContext *option_context;
  GOptionGroup   *ytsg_group;
  GError         *error = NULL;
  gboolean        ret = TRUE;

  if (ytsg_is_initialized)
    return TRUE;

  option_context = g_option_context_new (NULL);
  g_option_context_set_ignore_unknown_options (option_context, TRUE);
  g_option_context_set_help_enabled (option_context, FALSE);

  ytsg_group = ytsg_get_option_group ();
  g_option_context_set_main_group (option_context, ytsg_group);

  if (!g_option_context_parse (option_context, argc, argv, &error))
    {
      if (error)
	{
	  g_warning ("%s", error->message);
	  g_error_free (error);
	}

      ret = FALSE;
    }

  g_option_context_free (option_context);

  return ret;
}

/**
 * ytsg_init:
 * @argc: (inout): The number of arguments in @argv
 * @argv: (array length=argc) (inout) (allow-none): A pointer to an array
 *   of arguments.
 *
 * Initialises Ytstenut-glib library.
 *
 * Return value: 1 on success, < 0 on failure.
 */
int
ytsg_init (int *argc, char ***argv)
{
  GError *error = NULL;

  if (ytsg_is_initialized)
    return 1;

  ytsg_base_init ();

  if (!ytsg_parse_args (argc, argv))
    {
      g_warning ("Failed to parse arguments.");
      return 0;
    }

  return ytsg_init_real (&error);
}
