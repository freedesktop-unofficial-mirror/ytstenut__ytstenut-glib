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
#include "yts-xml.h"

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
  char        *service_id;
  GHashTable  *status;
  GList       *interests;
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

  g_hash_table_destroy (priv->status);
  priv->status = NULL;

  G_OBJECT_CLASS (yts_client_status_parent_class)->finalize (object);
}

static void
yts_client_status_class_init (YtsClientStatusClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (YtsClientStatusPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->finalize = _finalize;

  pspec = g_param_spec_string ("service-id", "", "",
                               NULL,
                               G_PARAM_READWRITE |
                               G_PARAM_CONSTRUCT_ONLY |
                               G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_SERVICE_ID, pspec);
}

static void
yts_client_status_init (YtsClientStatus *self)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (self);

  priv->status = g_hash_table_new_full (g_str_hash,
                                        g_str_equal,
                                        g_free,
                                        g_free);
}

YtsClientStatus *
yts_client_status_new (char const *service_id)
{
  return g_object_new (YTS_TYPE_CLIENT_STATUS,
                       "service-id", service_id,
                       NULL);
}

bool
yts_client_status_add_capability (YtsClientStatus *self,
                                  char const      *capability)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (self);
  bool have_capability;

  g_return_val_if_fail (YTS_IS_CLIENT_STATUS (self), false);
  g_return_val_if_fail (capability, false);
  g_return_val_if_fail (g_str_has_prefix (capability,
                                          YTS_XML_CAPABILITY_NAMESPACE),
                        false);

  have_capability = g_hash_table_lookup_extended (priv->status,
                                                  capability,
                                                  NULL, NULL);
  if (!have_capability) {
    g_hash_table_insert (priv->status, g_strdup (capability), NULL);
    return true;
  }

  return false;
}

bool
yts_client_status_revoke_capability (YtsClientStatus  *self,
                                     char const       *capability)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CLIENT_STATUS (self), false);
  g_return_val_if_fail (capability, false);
  g_return_val_if_fail (g_str_has_prefix (capability,
                                          YTS_XML_CAPABILITY_NAMESPACE),
                        false);

  return g_hash_table_remove (priv->status, capability);
}

bool
yts_client_status_add_interest (YtsClientStatus *self,
                                char const      *interest)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (self);
  GList *element;

  g_return_val_if_fail (YTS_IS_CLIENT_STATUS (self), false);
  g_return_val_if_fail (interest, false);
  g_return_val_if_fail (g_str_has_prefix (interest,
                                          YTS_XML_CAPABILITY_NAMESPACE),
                        false);

  /* We could do better with something else but a list, but list is small. */
  element = g_list_find_custom (priv->interests,
                                interest,
                                (GCompareFunc) g_strcmp0);
  if (!element) {
    priv->interests = g_list_prepend (priv->interests, g_strdup (interest));
    return true;
  }

  return false;
}

bool
yts_client_status_revoke_interest (YtsClientStatus  *self,
                                   char const       *interest)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (self);
  GList *element;

  g_return_val_if_fail (YTS_IS_CLIENT_STATUS (self), false);
  g_return_val_if_fail (interest, false);
  g_return_val_if_fail (g_str_has_prefix (interest,
                                          YTS_XML_CAPABILITY_NAMESPACE),
                        false);

  /* We could do better with something else but a list, but list is small. */
  element = g_list_find_custom (priv->interests,
                                interest,
                                (GCompareFunc) g_strcmp0);
  if (element) {
    g_free (element->data);
    priv->interests = g_list_delete_link (priv->interests, element);
    return true;
  } else {
    g_warning ("Interest '%s' not set in the first place, can not be revoked.",
               interest);
  }

  return false;
}

bool
yts_client_status_foreach_interest (YtsClientStatus                 *self,
                                    YtsClientStatusInterestIterator  iterator,
                                    void                            *user_data)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (self);
  GList *iter;
  bool   ret = true;

  g_return_val_if_fail (YTS_IS_CLIENT_STATUS (self), false);

  for (iter = priv->interests;
       iter && ret;
       iter = iter->next) {
    ret = iterator (self, (char const *) iter->data, user_data);
  }

  return ret;
}

char const *
yts_client_status_set (YtsClientStatus    *self,
                       char const         *capability,
                       char const *const  *attribs,
                       char const         *xml_payload)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (self);
  GString *attribs_str;
  char *status_xml;

  g_return_val_if_fail (YTS_IS_CLIENT_STATUS (self), NULL);
  g_return_val_if_fail (capability, NULL);
  g_return_val_if_fail (g_str_has_prefix (capability,
                                          YTS_XML_CAPABILITY_NAMESPACE),
                        NULL);

  attribs_str = g_string_new ("");
  for (unsigned i = 0; attribs && attribs[i] && attribs[i+1]; i += 2) {
    g_string_append_printf (attribs_str, "%s='%s' ", attribs[i], attribs[i+1]);
  }

  status_xml = g_strdup_printf ("<status xmlns='%s' "
                                "from-service='%s' "
                                "capability='%s' "
                                "%s>"
                                "%s"
                                "</status>",
                                YTS_XML_STATUS_NAMESPACE,
                                priv->service_id,
                                capability,
                                attribs_str->str,
                                xml_payload ? xml_payload : "");

  g_string_free (attribs_str, true);

  g_hash_table_replace (priv->status, g_strdup (capability), status_xml);

  return status_xml;
}

bool
yts_client_status_clear (YtsClientStatus  *self,
                         char const       *capability)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (YTS_IS_CLIENT_STATUS (self), false);
  g_return_val_if_fail (capability, false);
  g_return_val_if_fail (g_str_has_prefix (capability,
                                          YTS_XML_CAPABILITY_NAMESPACE),
                        false);

  return g_hash_table_remove (priv->status, capability);
}

bool
yts_client_status_foreach_capability (YtsClientStatus                   *self,
                                      YtsClientStatusCapabilityIterator  iterator,
                                      void                              *user_data)
{
  YtsClientStatusPrivate *priv = GET_PRIVATE (self);
  GHashTableIter iter;
  char const      *capability;
  char const      *status_xml;
  bool             ret = true;

  g_return_val_if_fail (YTS_IS_CLIENT_STATUS (self), false);
  g_return_val_if_fail (iterator, false);

  g_hash_table_iter_init (&iter, priv->status);
  while (ret &&
         g_hash_table_iter_next (&iter,
                                 (gpointer *) &capability,
                                 (gpointer *) &status_xml)) {

    ret = iterator (self, capability, status_xml, user_data);
  }

  return ret;
}

