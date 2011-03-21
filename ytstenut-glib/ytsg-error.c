/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2010 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ytsg-error.h"

/**
 * ytsg_error_get_code:
 * @error: #YtsgError
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
 * that generates #YtsgError's
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

  return (atom << 16);
}

