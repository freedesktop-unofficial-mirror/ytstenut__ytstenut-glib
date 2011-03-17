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

#ifndef _YTSG_TYPES_H
#define _YTSG_TYPES_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * YtsgProtocol:
 * @YTSG_PROTOCOL_XMPP: Jabber
 * @YTSG_PROTOCOL_LOCAL_XMPP: Bonjour
 *
 * YtsgProtocol represents the xmpp protocol to use
 */
typedef enum { /*< prefix=YTSG_PROTOCOL >*/
  YTSG_PROTOCOL_XMPP = 0,
  YTSG_PROTOCOL_LOCAL_XMPP
} YtsgProtocol;

/**
 * YtsgPresence:
 * @YTSG_PRESENCE_UNAVAILABLE: Client is not available
 * @YTSG_PRESENCE_AVAILABLE: Client is available
 *
 * YtsgPresence represents the presence status of #YtsgClient.
 */
typedef enum { /*< prefix=YTSG_PRESENCE >*/
  YTSG_PRESENCE_UNAVAILABLE = 0,
  YTSG_PRESENCE_AVAILABLE,

  /* <private> */
  /* Must be last */
  _YTSG_PRESENCE_LAST_
} YtsgPresence;

/**
 * YtsgSubscription:
 * @YTSG_SUBSCRIPTION_NONE: No subscription
 * @YTSG_SUBSCRIPTION_PENDING_OUT: Subscription pending approval by remote
 * @YTSG_SUBSCRIPTION_PENDING_IN: Subscription pending approval locally,
 * @YTSG_SUBSCRIPTION_APPROVED: Subscription is approved for two way communication
 *
 * YtsgSubscription represents the subscription status of #YtsgClient.
 */
typedef enum { /*< prefix=YTSG_SUBSCRIPTION >*/
  YTSG_SUBSCRIPTION_NONE          = 0,
  YTSG_SUBSCRIPTION_PENDING_OUT   = 0x1,
  YTSG_SUBSCRIPTION_PENDING_IN    = 0x2,
  YTSG_SUBSCRIPTION_APPROVED      = 0x4,

  /* <private>*/
  /* Must be last */
  _YTSG_SUBSCRIPTION_LAST_
} YtsgSubscription;

G_END_DECLS

#endif
