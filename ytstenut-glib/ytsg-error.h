/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2010 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 */

#ifndef _YTSG_ERROR_H
#define _YTSG_ERROR_H

#include <ytstenut-glib/ytsg-types.h>

G_BEGIN_DECLS

/**
 * YtsgError:
 * @YTSG_ERROR_SUCCESS: Operation succeeded,
 * @YTSG_ERROR_PENDING: The operation will be completed at a later stage,
 * subject to completing intermediate asynchronous operations (implies success
 * up to the point the asynchronous operation began). The operation to which
 * it pertains can be retrieved from the #YtsgError value using
 * ytsg_error_get_atom().  Operations that return this error code must emit
 * #YtsgClient::error signal at a later stage to indicate either subsequent
 * errors or eventual success.
 * @YTSG_ERROR_OBJECT_DISPOSED: the object is in process of being destroyed
 * @YTSG_ERROR_INVALID_PARAMETER: Invalid parameter supplied to function
 * @YTSG_ERROR_NOT_ALLOWED: the operation is not allowed.
 * @YTSG_ERROR_NO_ROUTE: no route to complete the operation
 * @YTSG_ERROR_UNKNOWN: some other,unspecified, error condition.
 * @YTSG_ERROR_CUSTOM_START: custom error codes can start at this value
 * @YTSG_ERROR_CUSTOM_END: custom error code must not exceed this value
 *
 * YtsgError represents common errors for nScreen operation; YtsgError combines
 * an error code defined by the above enumeration and an atom value that
 * uniquely identifies the operation associated with the value. Use
 * ytsg_error_get_code() and ytsg_error_get_atom() to retrieve these components.
 */

typedef enum { /*< prefix=YTSG_ERROR >*/
  YTSG_ERROR_SUCCESS = 0,
  YTSG_ERROR_PENDING,
  YTSG_ERROR_OBJECT_DISPOSED,
  YTSG_ERROR_INVALID_PARAMETER,
  YTSG_ERROR_NOT_ALLOWED,
  YTSG_ERROR_NO_ROUTE,
  YTSG_ERROR_NO_MSG_CHANNEL,

  /* Last predefined error code */
  YTSG_ERROR_UNKNOWN      = 0x00007fff,

  YTSG_ERROR_CUSTOM_START = 0x00008000,
  /* Custom error code range is in between here */
  YTSG_ERROR_CUSTOM_END   = 0x0000ffff,

  /* <private> */
  _YTSG_ERROR_CODE_MASK = 0x0000ffff,
  _YTSG_ERROR_ATOM_MASK = 0xffff0000,

} YtsgError;

YtsgError ytsg_error_new (guint32 code);
guint32   ytsg_error_new_atom ();
YtsgError ytsg_error_make (guint32 atom, guint32 code);
guint     ytsg_error_get_code (YtsgError error);
guint     ytsg_error_get_atom (YtsgError error);

G_END_DECLS

#endif /* _YTSG_ERROR_H */
