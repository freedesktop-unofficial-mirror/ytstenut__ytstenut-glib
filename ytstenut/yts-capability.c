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

#include "yts-capability.h"
#include "config.h"

G_DEFINE_INTERFACE (YtsCapability, yts_capability, G_TYPE_OBJECT)

/**
 * SECTION:yts-capability
 * @short_description: Common interface for service implementations and their
 *                     proxies.
 *
 * #YtsCapability is an iterface to query services and proxies for the fully
 * qualified capability IDs (FQC-IDs) they are supporting.
 */

static void
yts_capability_default_init (YtsCapabilityInterface *interface)
{
  GParamSpec *pspec;

  /**
   * YtsCapability:fqc-ids:
   *
   * Null-terminated array of capability IDs a service or proxy supports.
   * This property is in fact read-only.
   */
  pspec = g_param_spec_boxed ("fqc-ids", "", "",
                              G_TYPE_STRV,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS);
  g_object_interface_install_property (interface, pspec);
}

/**
 * yts_capability_get_fqc_ids:
 * @self: object on which to invoke this method.
 *
 * Get array of supported FQC-IDs.
 *
 * Returns (array zero-terminated=1) (transfer full): Null-terminated array of
 *         FQC-IDs.
 *
 * Since: 0.3
 */
char **
yts_capability_get_fqc_ids (YtsCapability *self)
{
  char **fqc_ids;

  g_return_val_if_fail (YTS_IS_CAPABILITY (self), NULL);

  fqc_ids = NULL;
  g_object_get (self, "fqc-ids", &fqc_ids, NULL);

  return fqc_ids;
}

/**
 * yts_capability_has_fqc_id:
 * @self: object on which to invoke this method.
 * @fqc_id: the capability ID to query for.
 *
 * Query @self whether it supports the capability identified by @fqc_id.
 *
 * Returns: %true if @fqc_id is supported by @self.
 *
 * Since: 0.3
 */
bool
yts_capability_has_fqc_id (YtsCapability  *self,
                           char const     *fqc_id)
{
  char      **fqc_ids;
  unsigned    i;
  bool        ret = false;

  g_return_val_if_fail (YTS_IS_CAPABILITY (self), NULL);

  fqc_ids = NULL;
  g_object_get (self, "fqc-ids", &fqc_ids, NULL);

  for (i = 0; fqc_ids[i]; i++) {
    if (0 == g_strcmp0 (fqc_id, fqc_ids[i])) {
      ret = true;
      break;
    }
  }

  g_strfreev (fqc_ids);

  return ret;
}

