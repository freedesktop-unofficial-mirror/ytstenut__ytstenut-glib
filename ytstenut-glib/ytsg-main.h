/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2011 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 */

#ifndef _YTSG_UTIL_H
#define _YTSG_UTIL_H

#include <glib.h>

G_BEGIN_DECLS

int           ytsg_init             (int *argc, char ***argv);
GOptionGroup *ytsg_get_option_group (void);

G_END_DECLS

#endif /* _YTSG_UTIL_H */
