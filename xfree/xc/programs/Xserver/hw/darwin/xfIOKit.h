/*
  xfIOKit.h

  IOKit specific functions and definitions
*/
/* $XFree86: xc/programs/Xserver/hw/darwin/xfIOKit.h,v 1.3 2001/04/01 07:12:13 torrey Exp $ */

#ifndef _XFIOKIT_H
#define _XFIOKIT_H

#include "X11/Xproto.h"
#include "screenint.h"

Bool XFIOKitAddScreen(ScreenPtr pScreen);
Bool XFIOKitInitCursor(ScreenPtr pScreen);
void XFIOKitOsVendorInit(void);
void XFIOKitGiveUp(void);
void XFIOKitBell(int volume, DeviceIntPtr pDevice, pointer ctrl, int class);

#endif
