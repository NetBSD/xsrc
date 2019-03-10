/*
 * Copyright © 2007,2008 Red Hat, Inc.
 * Copyright © 2010 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Soft-
 * ware"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, provided that the above copyright
 * notice(s) and this permission notice appear in all copies of the Soft-
 * ware and that both the above copyright notice(s) and this permission
 * notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
 * ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY
 * RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN
 * THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSE-
 * QUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFOR-
 * MANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization of
 * the copyright holder.
 *
 * Authors:
 *   Kristian Høgsberg (krh@redhat.com)
 *   Modified for VDPAU by Aaron Plattner (aplattner@nvidia.com)
 */

#ifndef _VDP_DRI2_H_
#define _VDP_DRI2_H_

#include <X11/extensions/dri2tokens.h>

#if (defined(__GNUC__) && __GNUC__ >= 4)
  #define PRIVATE __attribute__ ((visibility("hidden")))
#else
  #define PRIVATE
#endif


PRIVATE extern Bool
_vdp_DRI2QueryExtension(Display * display, int *eventBase, int *errorBase);

PRIVATE extern Bool
_vdp_DRI2QueryVersion(Display * display, int *major, int *minor);

PRIVATE extern Bool
_vdp_DRI2Connect(Display * display, XID window, char **driverName,
                 char **deviceName);

PRIVATE extern void
_vdp_DRI2RemoveExtension(Display * display);

#endif
