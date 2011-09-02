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

/**
 * SECTION:ytsg-caps
 * @short_description: represents application capability.
 *
 * #YtsgCaps represents application capability.
 */

#ifndef _YTSG_CAPS_H
#define _YTSG_CAPS_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * YtsgCaps:
 *
 * Represents client capablilites.
 */
typedef GQuark YtsgCaps;

/**
 * YTSG_CAPS_S_UNDEFINED:
 *
 * Ytstenut string representing unknown capabilities
 */
#define YTSG_CAPS_S_UNDEFINED NULL

/**
 * YTSG_CAPS_UI_UNDEFINED:
 *
 * Ytstenut UI string representing unknown capabilities
 */
#define YTSG_CAPS_UI_UNDEFINED "Unknown"

/**
 * YTSG_CAPS_UNDEFINED:
 *
 * Ytstenut GQuark representing unknown capabilties
 */
#define YTSG_CAPS_UNDEFINED 0

/**
 * YTSG_CAPS_S_AUDIO:
 *
 * Ytstenut string representing audio capabilities
 */
#define YTSG_CAPS_S_AUDIO "ytsg-caps-audio"

/**
 * YTSG_CAPS_UI_AUDIO:
 *
 * Ytstenut UI string representing audio capabilities
 */
#define YTSG_CAPS_UI_AUDIO "Audio"

/**
 * YTSG_CAPS_AUDIO:
 *
 * Ytstenut GQuark representing audio capabilties
 */
#define YTSG_CAPS_AUDIO g_quark_from_static_string (YTSG_CAPS_S_AUDIO)

/**
 * YTSG_CAPS_S_VIDEO:
 *
 * Ytstenut string representing video capabilities
 */
#define YTSG_CAPS_S_VIDEO "ytsg-caps-video"

/**
 * YTSG_CAPS_UI_VIDEO:
 *
 * Ytstenut UI string representing video capabilities
 */
#define YTSG_CAPS_UI_VIDEO "Video"


/**
 * YTSG_CAPS_VIDEO:
 *
 * Ytstenut GQuark representing video capabilties
 */
#define YTSG_CAPS_VIDEO g_quark_from_static_string (YTSG_CAPS_S_VIDEO)

/**
 * YTSG_CAPS_S_IMAGE:
 *
 * Ytstenut string representing image capabilities
 */
#define YTSG_CAPS_S_IMAGE "ytsg-caps-image"

/**
 * YTSG_CAPS_UI_IMAGE:
 *
 * Ytstenut UI string representing unknown capabilities
 */
#define YTSG_CAPS_UI_IMAGE "Image"

/**
 * YTSG_CAPS_IMAGE:
 *
 * Ytstenut GQuark representing image capabilties
 */
#define YTSG_CAPS_IMAGE g_quark_from_static_string (YTSG_CAPS_S_IMAGE)

/**
 * YTSG_CAPS_S_HTML:
 *
 * Ytstenut string representing html capabilities
 */
#define YTSG_CAPS_S_HTML "ytsg-caps-html"

/**
 * YTSG_CAPS_UI_HTML:
 *
 * Ytstenut UI string representing unknown capabilities
 */
#define YTSG_CAPS_UI_HTML "Web page"

/**
 * YTSG_CAPS_HTML:
 *
 * Ytstenut GQuark representing html capabilties
 */
#define YTSG_CAPS_HTML g_quark_from_static_string (YTSG_CAPS_S_HTML)

/**
 * YTSG_CAPS_S_CONTROL:
 *
 * Ytstenut string representing control capabilities
 */
#define YTSG_CAPS_S_CONTROL "ytsg-caps-control"

/**
 * YTSG_CAPS_UI_CONTROL:
 *
 * Ytstenut UI string representing unknown capabilities
 */
#define YTSG_CAPS_UI_CONTROL "Control"


/**
 * YTSG_CAPS_CONTROL:
 *
 * Ytstenut GQuark representing control capabilties. This is a special value for
 * applications the sole purpose of which is to be Ytstenut remote
 * control. Clients that set their #YtsgClient status using this value will
 * receive unfiltered roster and in turn will be added to the roster of every
 * other connected client. Clients that implement regular capabilities should
 * fitler out clients with YTSG_CAPS_CONTROL capability from their UI (though
 * they need to respond to commands received from them).
 */
#define YTSG_CAPS_CONTROL g_quark_from_static_string (YTSG_CAPS_S_CONTROL)

G_END_DECLS

#endif /* _YTSG_CAPS_H */
