
/* $XConsortium: sun.h,v 5.39.1.1 95/01/05 19:58:43 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/sun/sun.h,v 3.2 1995/02/12 02:36:21 dawes Exp $ */

/*-
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef _MACPPC_H_
#define _MACPPC_H_

/* X headers */
#include "Xos.h"
#include "X.h"
#include "Xproto.h"

/* general system headers */
#ifndef NOSTDHDRS
# include <stdlib.h>
#else
# include <malloc.h>
extern char *getenv();
#endif

/* system headers common to both SunOS and Solaris */
#include <sys/param.h>
#include <sys/file.h>
#include <sys/filio.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#define _POSIX_SOURCE
#include <signal.h>
#undef _POSIX_SOURCE
#include <fcntl.h>
#include <errno.h>
#include <memory.h>

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

#include <dev/wscons/wsconsio.h>

typedef struct wscons_event Firm_event;

extern int gettimeofday();

/*
 * Server specific headers
 */
#include "misc.h"
#undef abs /* don't munge function prototypes in headers, sigh */
#include "scrnintstr.h"
#ifdef NEED_EVENTS
# include "inputstr.h"
#endif
#include "input.h"
#include "colormapst.h"
#include "colormap.h"
#include "cursorstr.h"
#include "cursor.h"
#include "dixstruct.h"
#include "dix.h"
#include "opaque.h"
#include "resource.h"
#include "servermd.h"
#include "windowstr.h"

/*
 * ddx specific headers
 */
#ifndef PSZ
#define PSZ 8
#endif

#include "mipointer.h"

extern int monitorResolution;

/*
 * MAXEVENTS is the maximum number of events the mouse and keyboard functions
 * will read on a given call to their GetEvents vectors.
 */
#define MAXEVENTS 	32

/*
 * Data private to any macppc keyboard.
 */
typedef struct {
    int		fd;
    int		type;		/* Type of keyboard */
    int		layout;		/* The layout of the keyboard */
    int		click;		/* kbd click save state */
    Leds	leds;		/* last known LED state */
} macppcKbdPrivRec, *macppcKbdPrivPtr;

extern macppcKbdPrivRec macppcKbdPriv;

/*
 * Data private to any macppc pointer device.
 */
typedef struct {
    int		fd;
    int		bmask;		/* last known button state */
} macppcPtrPrivRec, *macppcPtrPrivPtr;

extern macppcPtrPrivRec macppcPtrPriv;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} macppcModmapRec;

typedef struct {
    ColormapPtr	    installedMap;
    CloseScreenProcPtr CloseScreen;
    void	    (*UpdateColormap)();
    Bool	    hasHardwareCursor;
} macppcScreenRec, *macppcScreenPtr;

#define GetScreenPrivate(s)   ((macppcScreenPtr) ((s)->devPrivates[macppcScreenIndex].ptr))
#define SetupScreen(s)	macppcScreenPtr	pPrivate = GetScreenPrivate(s)

typedef struct {
    unsigned char*  fb;		/* Frame buffer itself */
    int		    fd;		/* frame buffer for ioctl()s, */
    struct wsdisplay_fbinfo   info;	/* Frame buffer characteristics */
    void	    (*EnterLeave)();/* screen switch */
} fbFd;

typedef Bool (*macppcFbInitProc)(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);

typedef struct {
    macppcFbInitProc	init;	/* init procedure for this fb */
    char*		name;	/* /usr/include/fbio names */
} macppcFbDataRec;

#ifdef XKB
extern Bool		noXkbExtension;
#endif

#if 0
extern Bool		sunAutoRepeatHandlersInstalled;
extern long		sunAutoRepeatInitiate;
extern long		sunAutoRepeatDelay;
#endif
extern macppcFbDataRec	macppcFbData[];
extern fbFd		macppcFbs[];
#if 0
extern Bool		sunFlipPixels;
extern Bool		sunActiveZaphod;
extern Bool		sunFbInfo;
#endif
extern int		macppcScreenIndex;

extern void macppcDisableCursor(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

extern void macppcEnqueueEvents(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool macppcSaveScreen(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    int /* on */
#endif
);

extern Bool macppcScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

extern pointer macppcMemoryMap(
#if NeedFunctionPrototypes
    size_t /* len */,
    off_t /* off */,
    int /* fd */
#endif
);

extern Bool macppcScreenAllocate(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

extern Bool macppcInitCommon(
#if NeedFunctionPrototypes
    int /* scrn */,
    ScreenPtr /* pScrn */,
    off_t /* offset */,
    Bool (* /* init1 */)(),
    void (* /* init2 */)(),
    Bool (* /* cr_cm */)(),
    Bool (* /* save */)(),
    int /* fb_off */
#endif
);

extern Firm_event* macppcKbdGetEvents(
#if NeedFunctionPrototypes
    int /* fd */,
    int* /* pNumEvents */,
    Bool* /* pAgain */
#endif
);

extern Firm_event* macppcMouseGetEvents(
#if NeedFunctionPrototypes
    int /* fd */,
    int* /* pNumEvents */,
    Bool* /* pAgain */
#endif
);

extern void macppcKbdEnqueueEvent(
#if NeedFunctionPrototypes
    DeviceIntPtr /* device */,
    Firm_event* /* fe */
#endif
);

extern void macppcMouseEnqueueEvent(
#if NeedFunctionPrototypes
    DeviceIntPtr /* device */,
    Firm_event* /* fe */
#endif
);

extern int macppcKbdProc(
#if NeedFunctionPrototypes
    DeviceIntPtr /* pKeyboard */,
    int /* what */
#endif
);

extern int macppcMouseProc(
#if NeedFunctionPrototypes
    DeviceIntPtr /* pMouse */,
    int /* what */
#endif
);

extern void macppcKbdWait(
#if NeedFunctionPrototypes
    void
#endif
);

/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))
#define TSTOMILLI(ts)	(((ts).tv_nsec/1000000)+((ts).tv_sec*1000))

extern void macppcInstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr /* cmap */
#endif
);

extern void macppcUninstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr /* cmap */
#endif
);

extern int macppcListInstalledColormaps(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    Colormap* /* pCmapList */
#endif
);

#endif
