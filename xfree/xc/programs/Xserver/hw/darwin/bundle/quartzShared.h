/*
 * Shared definitions between the Darwin X Server
 * and the Cocoa front end. 
 */
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartzShared.h,v 1.5 2001/05/16 06:10:08 torrey Exp $ */

#ifndef _QUARTZSHARED_H
#define _QUARTZSHARED_H

// Data stored at startup for Cocoa front end
extern int                  quartzEventWriteFD;
extern int                  quartzStartClients;

// User preferences used by X server
extern int                  quartzUseSysBeep;
extern int                  darwinFakeButtons;
extern char                 *darwinKeymapFile;

void QuartzCapture(void);
void QuartzReadPreferences(void);
void QuartzMessageMainThread(unsigned msg);

// NX_APPDEFINED event subtypes for special commands to the X server
// update modifiers: update all modifier keys
// show: vt switch to X server; recapture screen and restore X drawing
// hide: vt switch away from X server; release screen and clip X drawing
// quit: kill the X server and release the display
// read pasteboard: copy Mac OS X pasteboard into X cut buffer
// write pasteboard: copy X cut buffer onto Mac OS X pasteboard

enum {
  kXDarwinUpdateModifiers,
  kXDarwinShow,
  kXDarwinHide,
  kXDarwinQuit, 
  kXDarwinReadPasteboard,
  kXDarwinWritePasteboard
};

// Messages that can be sent to the main thread.
enum {
  kQuartzServerHidden,
  kQuartzServerDied
};

#endif	/* _QUARTZSHARED_H */

