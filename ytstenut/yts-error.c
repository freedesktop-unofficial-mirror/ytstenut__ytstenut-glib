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
 * SECTION:yts-error
 * @short_description: An error object.
 * @title: YtsError
 * @section_id: yts-error
 *
 * #YtsError represents an errror in asynchronous operations. All erros consist
 * of a unique identifier and a status code.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "yts-error.h"

/**
 * yts_error_get_code:
 * @error: #YtsError
 *
 * Retrives error code from #YtsError.
 *
 * Return value: the error code represented by this #YtsError.
 */
guint
yts_error_get_code (YtsError error)
{
  return (error & _YTS_ERROR_CODE_MASK);
}

/**
 * yts_error_get_atom:
 * @error: #YtsError
 *
 * Retrieves the atom identifying the origin of this error from #YtsError.
 *
 * Return value: the atom identifying the operation represented by this
 * #YtsError.
 */
guint
yts_error_get_atom (YtsError error)
{
  /* g_debug ("error %d, atom %d", */
  /*          (guint32)(error & _YTS_ERROR_CODE_MASK), */
  /*          ((guint32)error & _YTS_ERROR_ATOM_MASK) >> 16); */

  return ((guint32)error & _YTS_ERROR_ATOM_MASK) >> 16;
}

/**
 * yts_error_new_atom:
 *
 * Obtains a new atom for #YtsError; this function is intended for use by code
 * that generates #YtsError<!-- -->s. NB: the atom is in its canonical shape
 * and will have to be shifted left by 16 bits before it can be ored with an
 * error code.
 *
 * Return value: a new atom for use with #YtsError.
 */
guint32
yts_error_new_atom ()
{
  static guint32 atom = 0;

  if (++atom > _YTS_ERROR_CODE_MASK)
    {
      g_warning ("Atom operation overflow, starting from beginning");
      atom = 1;
    }

  return atom;
}

/**
 * yts_error_make:
 * @atom: the error atom
 * @code: the error code
 *
 * Creates #YtsError from the provided values
 */
YtsError
yts_error_make (guint32 atom, guint32 code)
{
  return ((atom << 16) | code);
}

/**
 * yts_error_new:
 * @code: the error code
 *
 * Creates #YtsError with a new atom from the provide error code
 */
YtsError
yts_error_new (guint32 code)
{
  return ((yts_error_new_atom () << 16) | code);
}
