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

/**
 * SECTION:yts-types
 * @short_description: Common Ytestenut-glib types
 * @title: Common Types
 * @section_id: yts-types
 *
 * Common Ytstenut types
 */

#ifndef YTS_TYPES_H
#define YTS_TYPES_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * YtsProtocol:
 * @YTS_PROTOCOL_XMPP: Jabber
 * @YTS_PROTOCOL_LOCAL_XMPP: XEP-0174 serverless messaging.
 *
 * YtsProtocol represents the xmpp protocol to use
 */
typedef enum { /*< prefix=YTS_PROTOCOL >*/
  YTS_PROTOCOL_XMPP = 0,
  YTS_PROTOCOL_LOCAL_XMPP
} YtsProtocol;

/**
 * YtsPresence:
 * @YTS_PRESENCE_UNAVAILABLE: Client is not available
 * @YTS_PRESENCE_AVAILABLE: Client is available
 *
 * YtsPresence represents the presence status of #YtsClient.
 */
typedef enum { /*< prefix=YTS_PRESENCE >*/
  YTS_PRESENCE_UNAVAILABLE = 0,
  YTS_PRESENCE_AVAILABLE
} YtsPresence;

/* FIXME maybe create an yts-vs-type.h 
 * so YTS_TYPE_VS_QUERY_RESULT_ORDER can become YTS_VP_TYPE_QUERY_RESULT_ORDER */
typedef enum { /*< prefix=YTS_VP_QUERY >*/
  YTS_VP_QUERY_NONE = 0,
  YTS_VP_QUERY_CHRONOLOGICAL,
  YTS_VP_QUERY_DATE,
  YTS_VP_QUERY_RELEVANCE
} YtsVPQueryResultOrder;

G_END_DECLS

#endif
