/* $XFree86: xc/programs/Xserver/hw/xfree86/shadowfb/shadowfb.h,v 1.1 1999/01/31 12:22:07 dawes Exp $ */

#ifndef _SHADOWFB_H
#define _SHADOWFB_H

#include "xf86str.h"

typedef void (*RefreshAreaFuncPtr)(ScrnInfoPtr, int, BoxPtr);

Bool
ShadowFBInit (
    ScreenPtr		pScreen,
    RefreshAreaFuncPtr  refreshArea
);

#endif /* _SHADOWFB_H */
