/*
 * $XFree86: xc/lib/Xrandr/Xrandrint.h,v 1.2 2001/06/07 15:33:43 keithp Exp $
 *
 * Copyright © 2000 Compaq Computer Corporation, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Compaq not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Compaq makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * COMPAQ DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL COMPAQ
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Gettys, Compaq Computer Corporation, Inc.
 */

#ifndef _XRANDRINT_H_
#define _XRANDRINT_H_

#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include "Xext.h"			/* in ../include */
#include "extutil.h"			/* in ../include */
#include "Xrandr.h"
#include "randr.h"
#include "randrproto.h"

extern XExtensionInfo XrandrExtensionInfo;
extern char XrandrExtensionName[];

#define RRCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, XRRExtensionName, val)

#define RRSimpleCheckExtension(dpy,i) \
  XextSimpleCheckExtension (dpy, i, XRRExtensionName)

XExtDisplayInfo *
XRRFindDisplay (Display *dpy);

/* deliberately opaque internal data structure; can be extended, 
   but not reordered */
struct _XRRScreenConfiguration {
  Screen *screen;	/* the root window in GetScreenInfo */
  XRRVisualGroup *visual_group;
  XRRGroupOfVisualGroup *groups_of_visual_groups;
  XRRScreenSize *sizes;
  Rotation rotations;
  Rotation current_rotation;
  int nsizes;
  int current_size;
  int current_visual_group;
  Time timestamp;
  Time config_timestamp;
};

#endif /* _XRANDRINT_H_ */
