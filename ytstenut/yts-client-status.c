/*
 * Copyright © 2012 Intel Corp.
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

#include "yts-client-status.h"

#include "config.h"

G_DEFINE_TYPE (YtsClientStatus, yts_client_status, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), YTS_TYPE_CLIENT_STATUS, YtsClientStatusPrivate))

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN PACKAGE"\0client-status\0"G_STRLOC

enum {
  PROP_0,
  PROP_SERVICE_ID
};

typedef struct {
  char *service_id;
} YtsClientStatusPrivate;

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_SERVICE_ID:
      g_value_set_string (value, priv->service_id);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_set_property (GObject      *object,
               unsigned      property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
    case PROP_SERVICE_ID:
      /* Construct-only */
      priv->service_id = g_value_dup_string (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_finalize (GObject *object)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (object);

  g_free (priv->service_id);
  priv->service_id = NULL;

  G_OBJECT_CLASS (yts_client_status_parent_class)->finalize (object);
}

static void
yts_client_status_class_init (YtsClientStatusClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (YtsClientStatusPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->finalize = _finalize;

  pspec = g_param_spec_string ("service-id", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_SERVICE_ID, pspec);
}

static void
yts_client_status_init (YtsClientStatus *self)
{
}

YtsClientStatus *
yts_client_status_new (char const *service_id)
{
  return g_object_new (YTS_TYPE_CLIENT_STATUS,
                       "service-id", service_id,
                       NULL);
}

void
yts_client_status_add_capability (YtsClientStatus *self,
                                  char const      *capability)
{
  g_return_if_fail (YTS_IS_CLIENT_STATUS (self));
  g_return
}
void
yts_client_status_revoke_capability (YtsClientStatus  *self,
                                     char const       *capability);

void
yts_client_status_set (YtsClientStatus    *self,
                       char const         *capability,
                       char const *const  *attribs,
                       char const         *xml_payload);

void
yts_client_status_clear (YtsClientStatus    *self,
                         char const         *capability);

char *
yts_client_status_print (YtsClientStatus    *self,
                         char const         *capability);





          "<status xmlns='urn:ytstenut:status' from-service='service.name' "
          "capability='urn:ytstenut:capabilities:yts-caps-cats' "
          "activity='looking-at-cats-ooooooh'><look>at how cute they "
          "are!</look></status>",

