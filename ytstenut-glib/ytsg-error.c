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

/*
 * TODO -- this should probably be rewritten using the GAsync machinery.
 */

/**
 * SECTION:ytsg-error
 * @short_description: An error object.
 * @title: YtsgError
 * @section_id: ytsg-error
 *
 * #YtsgError represents an errror in asynchronous operations. All erros consist
 * of a unique identifier and a status code.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ytsg-error.h"

/**
 * ytsg_error_get_code:
 * @error: #YtsgError
 *
 * Retrives error code from #YtsgError.
 *
 * Return value: the error code represented by this #YtsgError.
 */
guint
ytsg_error_get_code (YtsgError error)
{
  return (error & _YTSG_ERROR_CODE_MASK);
}

/**
 * ytsg_error_get_atom:
 * @error: #YtsgError
 *
 * Retrieves the atom identifying the origin of this error from #YtsgError.
 *
 * Return value: the atom identifying the operation represented by this
 * #YtsgError.
 */
guint
ytsg_error_get_atom (YtsgError error)
{
  /* g_debug ("error %d, atom %d", */
  /*          (guint32)(error & _YTSG_ERROR_CODE_MASK), */
  /*          ((guint32)error & _YTSG_ERROR_ATOM_MASK) >> 16); */

  return ((guint32)error & _YTSG_ERROR_ATOM_MASK) >> 16;
}

/**
 * ytsg_error_new_atom:
 *
 * Obtains a new atom for #YtsgError; this function is intended for use by code
 * that generates #YtsgError<!-- -->s. NB: the atom is in its canonical shape
 * and will have to be shifted left by 16 bits before it can be ored with an
 * error code.
 *
 * Return value: a new atom for use with #YtsgError.
 */
guint32
ytsg_error_new_atom ()
{
  static guint32 atom = 0;

  if (++atom > _YTSG_ERROR_CODE_MASK)
    {
      g_warning ("Atom operation overflow, starting from beginning");
      atom = 1;
    }

  return atom;
}

/**
 * ytsg_error_make:
 * @atom: the error atom
 * @code: the error code
 *
 * Creates #YtsgError from the provided values
 */
YtsgError
ytsg_error_make (guint32 atom, guint32 code)
{
  return ((atom << 16) | code);
}

/**
 * ytsg_error_new:
 * @code: the error code
 *
 * Creates #YtsgError with a new atom from the provide error code
 */
YtsgError
ytsg_error_new (guint32 code)
{
  return ((ytsg_error_new_atom () << 16) | code);
}
