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
  YtsgMetadata *mdata;
  const char   *attrs[] = {"a1", "v1", "a2", "v2", NULL};
  const char   *a1, *a2, *a3;
  RestXmlNode  *child;
  char         *xml;
  GHashTable   *h;
  char         *body;

  g_thread_init (NULL);
  g_type_init ();

  message = ytsg_message_new (attrs);
  mdata   = (YtsgMetadata*)message;

  ytsg_metadata_add_attribute (mdata, "a3", "v3");
  a1 = ytsg_metadata_get_attribute (mdata, "a1");
  a2 = ytsg_metadata_get_attribute (mdata, "a2");
  a3 = ytsg_metadata_get_attribute (mdata, "a3");

  g_assert_cmpstr (a1, ==, "v1");
  g_assert_cmpstr (a2, ==, "v2");
  g_assert_cmpstr (a3, ==, "v3");

  child = rest_xml_node_add_child (ytsg_metadata_get_root_node (mdata),"c1");

  rest_xml_node_add_attr (child, "ca1", "cv1");

  h = _ytsg_metadata_extract (mdata, &body);
  g_assert (h);
  g_assert_cmpstr (body, ==, "<c1 ca1='cv1'></c1>");

  g_free (body);
  g_hash_table_unref (h);

  return 0;
}
