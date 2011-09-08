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
 * Authored by: Tomas Frydrych <tf@linux.intel.com>
 */

#include <ytstenut/yts-status.h>
#include <ytstenut/yts-message.h>
#include <string.h>

int
main (int argc, char **argv)
{
  YtsStatus  *status1, *status2;
  YtsMessage *message;
  const char  *attrs[] = {
    "capability", "urn:ytstenut:capabilities:yts-caps-video",
    "activity", "yts-activity-playing",
    "from-service", "com.meego.ytstenut.TestStatus",
    "a1", "v1", "a2", "v2", NULL};
  const char  *a1, *a2, *a3;
  RestXmlNode *top1, *top2, *child11, *child12, *child21, *child22;

  g_thread_init (NULL);
  g_type_init ();

  status1 = yts_status_new (attrs);

  yts_metadata_add_attribute ((YtsMetadata*)status1, "a3", "v3");
  a1 = yts_metadata_get_attribute ((YtsMetadata*)status1, "a1");
  a2 = yts_metadata_get_attribute ((YtsMetadata*)status1, "a2");
  a3 = yts_metadata_get_attribute ((YtsMetadata*)status1, "a3");

  g_assert_cmpstr (a1, ==, "v1");
  g_assert_cmpstr (a2, ==, "v2");
  g_assert_cmpstr (a3, ==, "v3");

  status2 = yts_status_new (attrs);

  g_assert (!yts_metadata_is_equal ((YtsMetadata*)status1,
                                     (YtsMetadata*)status2));

  yts_metadata_add_attribute ((YtsMetadata*)status2, "a3", "v3");

  g_assert (yts_metadata_is_equal ((YtsMetadata*)status1,
                                    (YtsMetadata*)status2));

  top1 = yts_metadata_get_root_node ((YtsMetadata*)status1);
  top2 = yts_metadata_get_root_node ((YtsMetadata*)status2);

  child11 = rest_xml_node_add_child (top1, "t");
  child12 = rest_xml_node_add_child (top1, "t");

  child21 = rest_xml_node_add_child (top2, "t");
  child22 = rest_xml_node_add_child (top2, "t");

  g_assert (yts_metadata_is_equal ((YtsMetadata*)status1,
                                    (YtsMetadata*)status2));

  rest_xml_node_add_attr (child11, "a4", "v4");
  rest_xml_node_add_attr (child21, "a4", "v4");

  g_assert (yts_metadata_is_equal ((YtsMetadata*)status1,
                                    (YtsMetadata*)status2));

  rest_xml_node_add_attr (child21, "a5", "v5");

  g_assert (!yts_metadata_is_equal ((YtsMetadata*)status1,
                                     (YtsMetadata*)status2));

  message = yts_message_new (NULL);

  g_assert (!yts_metadata_is_equal ((YtsMetadata*)status1,
                                     (YtsMetadata*)message));

  return 0;
}
