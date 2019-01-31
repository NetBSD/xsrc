#ifndef __NV_POINTER__
#define __NV_POINTER__
/*
 * All this mess is needed because the X headers use "pointer" for two
 * different purposes. Clearly this needs to be fixed there, but it is
 * much more intrusive. So we include the headers in the right order to
 * make the necessary magic happen.
 */

/* This defines _XTYPEDEF_POINTER to prevent <Xdefs.h> from doing the typedef */
#include "dix-config.h"

/* This uses "pointer" as a union tag, so include it first */
#include <X11/Xproto.h>

/* Undefine _XTYPEDEF_POINTER, because we need it as a type now */
#undef _XTYPEDEF_POINTER

/* This defines "pointer" as a type, so include it second */
#include <X11/Xdefs.h>

/* not defined in Xdefs.h for xorg-server 1.20. */
#ifndef _XTYPEDEF_POINTER
typedef void * pointer;
#define _XTYPEDEF_POINTER
#endif

#endif
