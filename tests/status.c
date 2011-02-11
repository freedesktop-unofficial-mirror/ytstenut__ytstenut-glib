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

#include <ytstenut-glib/ytsg-status.h>
#include <ytstenut-glib/ytsg-message.h>
#include <string.h>

int
main (int argc, char **argv)
{
  YtsgStatus  *status1, *status2;
  YtsgMessage *message;
  const char  *attrs[] = {"a1", "v1", "a2", "v2", NULL};
  const char  *a1, *a2, *a3;
  RestXmlNode *top1, *top2, *child11, *child12, *child21, *child22;

  g_thread_init (NULL);
  g_type_init ();

  status1 = ytsg_status_new (attrs);

  ytsg_metadata_add_attribute ((YtsgMetadata*)status1, "a3", "v3");
  a1 = ytsg_metadata_get_attribute ((YtsgMetadata*)status1, "a1");
  a2 = ytsg_metadata_get_attribute ((YtsgMetadata*)status1, "a2");
  a3 = ytsg_metadata_get_attribute ((YtsgMetadata*)status1, "a3");

  g_assert_cmpstr (a1, ==, "v1");
  g_assert_cmpstr (a2, ==, "v2");
  g_assert_cmpstr (a3, ==, "v3");

  status2 = ytsg_status_new (attrs);

  g_assert (!ytsg_metadata_is_equal ((YtsgMetadata*)status1,
                                     (YtsgMetadata*)status2));

  ytsg_metadata_add_attribute ((YtsgMetadata*)status2, "a3", "v3");

  g_assert (ytsg_metadata_is_equal ((YtsgMetadata*)status1,
                                    (YtsgMetadata*)status2));

  top1 = ytsg_metadata_get_root_node ((YtsgMetadata*)status1);
  top2 = ytsg_metadata_get_root_node ((YtsgMetadata*)status2);

  child11 = rest_xml_node_add_child (top1, "t");
  child12 = rest_xml_node_add_child (top1, "t");

  child21 = rest_xml_node_add_child (top2, "t");
  child22 = rest_xml_node_add_child (top2, "t");

  g_assert (ytsg_metadata_is_equal ((YtsgMetadata*)status1,
                                    (YtsgMetadata*)status2));

  rest_xml_node_add_attr (child11, "a4", "v4");
  rest_xml_node_add_attr (child21, "a4", "v4");

  g_assert (ytsg_metadata_is_equal ((YtsgMetadata*)status1,
                                    (YtsgMetadata*)status2));

  rest_xml_node_add_attr (child21, "a5", "v5");

  g_assert (!ytsg_metadata_is_equal ((YtsgMetadata*)status1,
                                     (YtsgMetadata*)status2));

  message = ytsg_message_new (NULL);

  g_assert (!ytsg_metadata_is_equal ((YtsgMetadata*)status1,
                                     (YtsgMetadata*)message));

  return 0;
}
