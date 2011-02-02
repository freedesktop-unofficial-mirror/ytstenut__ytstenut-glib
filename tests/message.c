/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2011, Intel Corporation.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <ytstenut-glib/ytsg-message.h>
#include <string.h>

int
main (int argc, char **argv)
{
  YtsgMessage  *message;
  const char  *attrs[] = {"a1", "v1", "a2", "v2", NULL};
  const char  *a1, *a2, *a3;

  g_thread_init (NULL);
  g_type_init ();

  message = ytsg_message_new (attrs);

  ytsg_metadata_add_attribute ((YtsgMetadata*)message, "a3", "v3");
  a1 = ytsg_metadata_get_attribute ((YtsgMetadata*)message, "a1");
  a2 = ytsg_metadata_get_attribute ((YtsgMetadata*)message, "a2");
  a3 = ytsg_metadata_get_attribute ((YtsgMetadata*)message, "a3");

  g_assert_cmpstr (a1, ==, "v1");
  g_assert_cmpstr (a2, ==, "v2");
  g_assert_cmpstr (a3, ==, "v3");

  return 0;
}
