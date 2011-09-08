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

#include <ytstenut/yts-message.h>
#include <ytstenut/yts-private.h>
#include <string.h>

int
main (int argc, char **argv)
{
  YtsMessage  *message;
  YtsMetadata *mdata;
  const char   *attrs[] = {"a1", "v1", "a2", "v2", NULL};
  const char   *a1, *a2, *a3;
  RestXmlNode  *child;
  GHashTable   *h;
  char         *body;

  g_thread_init (NULL);
  g_type_init ();

  message = yts_message_new (attrs);
  mdata   = (YtsMetadata*)message;

  yts_metadata_add_attribute (mdata, "a3", "v3");
  a1 = yts_metadata_get_attribute (mdata, "a1");
  a2 = yts_metadata_get_attribute (mdata, "a2");
  a3 = yts_metadata_get_attribute (mdata, "a3");

  g_assert_cmpstr (a1, ==, "v1");
  g_assert_cmpstr (a2, ==, "v2");
  g_assert_cmpstr (a3, ==, "v3");

  child = rest_xml_node_add_child (yts_metadata_get_root_node (mdata),"c1");

  rest_xml_node_add_attr (child, "ca1", "cv1");

  h = _yts_metadata_extract (mdata, &body);
  g_assert (h);
  g_assert_cmpstr (body, ==, "<c1 ca1='cv1'></c1>");

  g_free (body);
  g_hash_table_unref (h);

  return 0;
}
