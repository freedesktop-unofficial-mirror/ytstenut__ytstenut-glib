/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2010 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
 */

#ifndef _YTS_ERROR_H
#define _YTS_ERROR_H

#include <ytstenut/yts-types.h>

G_BEGIN_DECLS

/**
 * YtsError:
 * @YTS_ERROR_SUCCESS: Operation succeeded,
 * @YTS_ERROR_PENDING: The operation will be completed at a later stage,
 * subject to completing intermediate asynchronous operations (implies success
 * up to the point the asynchronous operation began). The operation to which
 * it pertains can be retrieved from the #YtsError value using
 * yts_error_get_atom().  Operations that return this error code must emit
 * #YtsClient::error signal at a later stage to indicate either subsequent
 * errors or eventual success.
 * @YTS_ERROR_OBJECT_DISPOSED: the object is in process of being destroyed
 * @YTS_ERROR_INVALID_PARAMETER: Invalid parameter supplied to function
 * @YTS_ERROR_NOT_ALLOWED: the operation is not allowed.
 * @YTS_ERROR_NO_ROUTE: no route to complete the operation
 * @YTS_ERROR_UNKNOWN: some other,unspecified, error condition.
 * @YTS_ERROR_CUSTOM_START: custom error codes can start at this value
 * @YTS_ERROR_CUSTOM_END: custom error code must not exceed this value
 *
 * YtsError represents common errors for nScreen operation; YtsError combines
 * an error code defined by the above enumeration and an atom value that
 * uniquely identifies the operation associated with the value. Use
 * yts_error_get_code() and yts_error_get_atom() to retrieve these components.
 */

typedef enum { /*< prefix=YTS_ERROR >*/
  YTS_ERROR_SUCCESS = 0,
  YTS_ERROR_PENDING,
  YTS_ERROR_OBJECT_DISPOSED,
  YTS_ERROR_INVALID_PARAMETER,
  YTS_ERROR_NOT_ALLOWED,
  YTS_ERROR_NO_ROUTE,
  YTS_ERROR_NO_MSG_CHANNEL,

  /* Last predefined error code */
  YTS_ERROR_UNKNOWN      = 0x00007fff,

  YTS_ERROR_CUSTOM_START = 0x00008000,
  /* Custom error code range is in between here */
  YTS_ERROR_CUSTOM_END   = 0x0000ffff,

  /* <private> */
  _YTS_ERROR_CODE_MASK = 0x0000ffff,
  _YTS_ERROR_ATOM_MASK = 0xffff0000,

} YtsError;

YtsError yts_error_new (guint32 code);
guint32   yts_error_new_atom ();
YtsError yts_error_make (guint32 atom, guint32 code);
guint     yts_error_get_code (YtsError error);
guint     yts_error_get_atom (YtsError error);

G_END_DECLS

#endif /* _YTS_ERROR_H */
