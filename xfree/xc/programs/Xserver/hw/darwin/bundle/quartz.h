/*
  quartz.h

  Quartz-specific functions and definitions
*/
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartz.h,v 1.3 2001/04/11 08:34:18 torrey Exp $ */

#ifndef _QUARTZ_H
#define _QUARTZ_H

#include "X11/Xproto.h"
#include "screenint.h"
#include "quartzShared.h"
#include "quartzPasteboard.h"

void QuartzOsVendorInit(void);
Bool QuartzAddScreen(ScreenPtr screen);
void QuartzGiveUp(void);
void QuartzHide(void);
void QuartzShow(int x, int y);

#endif
