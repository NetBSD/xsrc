/*
 * quartzShared.h
 *
 * Shared definitions between the Darwin X Server and the Cocoa front end
 *
 * This file is included in all parts of the Darwin X Server and must not
 * include any types defined in X11 or Mac OS X specific headers.
 * Definitions that are internal to the Quartz modes or use Mac OS X
 * specific types should be in quartzCommon.h instead of here.
 */
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartzShared.h,v 1.11 2001/10/14 03:02:18 torrey Exp $ */

#ifndef _QUARTZSHARED_H
#define _QUARTZSHARED_H

// User preferences used by generic Darwin X server code
extern int                  quartzMouseAccelChange;
extern int                  darwinFakeButtons;
extern int                  darwinFakeMouse2Mask;
extern int                  darwinFakeMouse3Mask;
extern char                 *darwinKeymapFile;
extern unsigned int         darwinDesiredWidth, darwinDesiredHeight;
extern int                  darwinDesiredDepth;
extern int                  darwinDesiredRefresh;

// location of X11's (0,0) point in global screen coordinates
extern int                  darwinMainScreenX;
extern int                  darwinMainScreenY;

// NX_APPDEFINED event subtypes for special commands to the X server
enum {
  kXDarwinUpdateModifiers,  // update all modifier keys
  kXDarwinShow,             // vt switch to X server;
                            // recapture screen and restore X drawing
  kXDarwinHide,             // vt switch away from X server;
                            // release screen and clip X drawing
  kXDarwinQuit,             // kill the X server and release the display
  kXDarwinReadPasteboard,   // copy Mac OS X pasteboard into X cut buffer
  kXDarwinWritePasteboard   // copy X cut buffer onto Mac OS X pasteboard
};

#endif	/* _QUARTZSHARED_H */

