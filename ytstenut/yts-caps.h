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
 * SECTION:yts-caps
 * @short_description: represents application capability.
 *
 * #YtsCaps represents application capability.
 */

#ifndef YTS_CAPS_H
#define YTS_CAPS_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * YtsCaps:
 *
 * Represents client capablilites.
 */
typedef GQuark YtsCaps;

/**
 * YTS_CAPS_S_UNDEFINED:
 *
 * Ytstenut string representing unknown capabilities
 */
#define YTS_CAPS_S_UNDEFINED NULL

/**
 * YTS_CAPS_UI_UNDEFINED:
 *
 * Ytstenut UI string representing unknown capabilities
 */
#define YTS_CAPS_UI_UNDEFINED "Unknown"

/**
 * YTS_CAPS_UNDEFINED:
 *
 * Ytstenut GQuark representing unknown capabilties
 */
#define YTS_CAPS_UNDEFINED 0

/**
 * YTS_CAPS_S_AUDIO:
 *
 * Ytstenut string representing audio capabilities
 */
#define YTS_CAPS_S_AUDIO "yts-caps-audio"

/**
 * YTS_CAPS_UI_AUDIO:
 *
 * Ytstenut UI string representing audio capabilities
 */
#define YTS_CAPS_UI_AUDIO "Audio"

/**
 * YTS_CAPS_AUDIO:
 *
 * Ytstenut GQuark representing audio capabilties
 */
#define YTS_CAPS_AUDIO g_quark_from_static_string (YTS_CAPS_S_AUDIO)

/**
 * YTS_CAPS_S_VIDEO:
 *
 * Ytstenut string representing video capabilities
 */
#define YTS_CAPS_S_VIDEO "yts-caps-video"

/**
 * YTS_CAPS_UI_VIDEO:
 *
 * Ytstenut UI string representing video capabilities
 */
#define YTS_CAPS_UI_VIDEO "Video"


/**
 * YTS_CAPS_VIDEO:
 *
 * Ytstenut GQuark representing video capabilties
 */
#define YTS_CAPS_VIDEO g_quark_from_static_string (YTS_CAPS_S_VIDEO)

/**
 * YTS_CAPS_S_IMAGE:
 *
 * Ytstenut string representing image capabilities
 */
#define YTS_CAPS_S_IMAGE "yts-caps-image"

/**
 * YTS_CAPS_UI_IMAGE:
 *
 * Ytstenut UI string representing unknown capabilities
 */
#define YTS_CAPS_UI_IMAGE "Image"

/**
 * YTS_CAPS_IMAGE:
 *
 * Ytstenut GQuark representing image capabilties
 */
#define YTS_CAPS_IMAGE g_quark_from_static_string (YTS_CAPS_S_IMAGE)

/**
 * YTS_CAPS_S_HTML:
 *
 * Ytstenut string representing html capabilities
 */
#define YTS_CAPS_S_HTML "yts-caps-html"

/**
 * YTS_CAPS_UI_HTML:
 *
 * Ytstenut UI string representing unknown capabilities
 */
#define YTS_CAPS_UI_HTML "Web page"

/**
 * YTS_CAPS_HTML:
 *
 * Ytstenut GQuark representing html capabilties
 */
#define YTS_CAPS_HTML g_quark_from_static_string (YTS_CAPS_S_HTML)

/**
 * YTS_CAPS_S_CONTROL:
 *
 * Ytstenut string representing control capabilities
 */
#define YTS_CAPS_S_CONTROL "yts-caps-control"

/**
 * YTS_CAPS_UI_CONTROL:
 *
 * Ytstenut UI string representing unknown capabilities
 */
#define YTS_CAPS_UI_CONTROL "Control"


/**
 * YTS_CAPS_CONTROL:
 *
 * Ytstenut GQuark representing control capabilties. This is a special value for
 * applications the sole purpose of which is to be Ytstenut remote
 * control. Clients that set their #YtsClient status using this value will
 * receive unfiltered roster and in turn will be added to the roster of every
 * other connected client. Clients that implement regular capabilities should
 * fitler out clients with YTS_CAPS_CONTROL capability from their UI (though
 * they need to respond to commands received from them).
 */
#define YTS_CAPS_CONTROL g_quark_from_static_string (YTS_CAPS_S_CONTROL)

G_END_DECLS

#endif /* YTS_CAPS_H */
