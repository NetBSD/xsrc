/* $XFree86: xc/programs/Xserver/hw/xfree86/shadowfb/shadowfb.h,v 1.2 1999/09/25 14:38:12 dawes Exp $ */

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
