/*
 * quartzCommon.h
 *
 * Common definitions used internally by all Quartz modes
 *
 * This file should be included before any X11 or IOKit headers
 * so that it can avoid symbol conflicts.
 */
/* $XFree86: xc/programs/Xserver/hw/darwin/bundle/quartzCommon.h,v 1.4.2.1 2002/08/20 21:56:15 torrey Exp $ */

#ifndef _QUARTZCOMMON_H
#define _QUARTZCOMMON_H

// QuickDraw in ApplicationServices has the following conflicts with
// the basic X server headers. Use QD_<name> to use the QuickDraw
// definition of any of these symbols, or the normal name for the
// X11 definition.
#define AllocCursor  QD_AllocCursor
#define InitFonts    QD_InitFonts
#define Cursor       QD_Cursor
#define WindowPtr    QD_WindowPtr
#include <ApplicationServices/ApplicationServices.h>
#undef AllocCursor
#undef InitFonts
#undef Cursor
#undef WindowPtr

#include "quartzShared.h"

// Quartz specific per screen storage structure
typedef struct {
    // List of CoreGraphics displays that this X11 screen covers.
    // This is more than one CG display for video mirroring and
    // rootless PseudoramiX mode.
    // No CG display will be covered by more than one X11 screen.
    int displayCount;
    CGDirectDisplayID *displayIDs;
} QuartzScreenRec, *QuartzScreenPtr;

#define QUARTZ_PRIV(pScreen) \
    ((QuartzScreenPtr)pScreen->devPrivates[quartzScreenIndex].ptr)

// Data stored at startup for Cocoa front end
extern int              quartzEventWriteFD;
extern int              quartzStartClients;

// User preferences used by Quartz modes
extern int              quartzRootless;
extern int              quartzUseSysBeep;

// Other shared data
extern int              quartzServerVisible;
extern int              quartzScreenIndex;
extern int              aquaMenuBarHeight;

void QuartzCapture(void);
void QuartzRelease(void);
void QuartzReadPreferences(void);
void QuartzMessageMainThread(unsigned msg);
int QuartzFSUseQDCursor(int depth);

// Messages that can be sent to the main thread.
enum {
    kQuartzServerHidden,
    kQuartzServerDied,
    kQuartzCursorUpdate
};

#endif	/* _QUARTZCOMMON_H */
