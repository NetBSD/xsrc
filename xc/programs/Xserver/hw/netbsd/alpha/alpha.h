
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

#ifndef _ALPHA_H_ 
#define _ALPHA_H_

/* X headers */
#include "Xos.h"
#undef index /* don't mangle silly Sun structure member names */
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

#include <machine/fbio.h>
#include <machine/wsconsio.h>
#if 0 /* XXX */
#include <machine/kbd.h>
#include <machine/kbio.h>
#endif
#include <machine/vuid_event.h>
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
 * Data private to any alpha keyboard.
 */
typedef struct {
    int		fd;
    int		type;		/* Type of keyboard */
    int		layout;		/* The layout of the keyboard */
    int		click;		/* kbd click save state */
    Leds	leds;		/* last known LED state */
} alphaKbdPrivRec, *alphaKbdPrivPtr;

extern alphaKbdPrivRec alphaKbdPriv;

/*
 * Data private to any alpha pointer device.
 */
typedef struct {
    int		fd;
    int		bmask;		/* last known button state */
} alphaPtrPrivRec, *alphaPtrPrivPtr;

extern alphaPtrPrivRec alphaPtrPriv;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} AlphaModmapRec;

typedef struct {
    int		    width, height;
    Bool	    has_cursor;
    CursorPtr	    pCursor;		/* current cursor */
} alphaCursorRec, *alphaCursorPtr;

typedef struct {
    ColormapPtr	    installedMap;
    CloseScreenProcPtr CloseScreen;
    void	    (*UpdateColormap)();
    alphaCursorRec    hardwareCursor;
    Bool	    hasHardwareCursor;
} alphaScreenRec, *alphaScreenPtr;

#define GetScreenPrivate(s)   ((alphaScreenPtr) ((s)->devPrivates[alphaScreenIndex].ptr))
#define SetupScreen(s)	alphaScreenPtr	pPrivate = GetScreenPrivate(s)

typedef struct {
    unsigned char*  fb;		/* Frame buffer itself */
    int		    fd;		/* frame buffer for ioctl()s, */
    struct fbtype   info;	/* Frame buffer characteristics */
    void	    (*EnterLeave)();/* screen switch */
} fbFd;

typedef Bool (*alphaFbInitProc)(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);

typedef struct {
    alphaFbInitProc	init;	/* init procedure for this fb */
    char*		name;	/* /usr/include/fbio names */
} alphaFbDataRec;

#ifdef XKB
extern Bool		noXkbExtension;
#endif

#if 0
extern Bool		sunAutoRepeatHandlersInstalled;
extern long		sunAutoRepeatInitiate;
extern long		sunAutoRepeatDelay;
#endif
extern alphaFbDataRec	alphaFbData[];
extern fbFd		alphaFbs[];
#if 0
extern Bool		sunSwapLkeys;
extern Bool		sunFlipPixels;
extern Bool		sunActiveZaphod;
extern Bool		sunFbInfo;
extern Bool		sunCG4Frob;
extern Bool		sunNoGX;
#endif
extern int		alphaScreenIndex;
#if 0
extern int*		sunProtected;
#endif

extern Bool alphaCursorInitialize(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

extern void alphaDisableCursor(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

#if 0 /* XXX */
extern int sunChangeKbdTranslation(
#if NeedFunctionPrototypes
    int /* fd */,
    Bool /* makeTranslated */
#endif
);

extern void sunNonBlockConsoleOff(
#if NeedFunctionPrototypes
#if defined(SVR4) || defined(__NetBSD__)
    void
#else
    char* /* arg */
#endif
#endif
);
#endif /* 0 XXX */

extern void alphaEnqueueEvents(
#if NeedFunctionPrototypes
    void
#endif
);

#if 0 /* XXX */
extern int sunGXInit(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    fbFd* /* fb */
#endif
);
#endif /* 0 XXX */

extern Bool alphaSaveScreen(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    int /* on */
#endif
);

extern Bool alphaScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

extern pointer alphaMemoryMap(
#if NeedFunctionPrototypes
    size_t /* len */,
    off_t /* off */,
    int /* fd */
#endif
);

extern Bool alphaScreenAllocate(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

extern Bool alphaInitCommon(
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

extern Firm_event* alphaKbdGetEvents(
#if NeedFunctionPrototypes
    int /* fd */,
    int* /* pNumEvents */,
    Bool* /* pAgain */
#endif
);

extern Firm_event* alphaMouseGetEvents(
#if NeedFunctionPrototypes
    int /* fd */,
    int* /* pNumEvents */,
    Bool* /* pAgain */
#endif
);

extern void alphaKbdEnqueueEvent(
#if NeedFunctionPrototypes
    DeviceIntPtr /* device */,
    Firm_event* /* fe */
#endif
);

extern void alphaMouseEnqueueEvent(
#if NeedFunctionPrototypes
    DeviceIntPtr /* device */,
    Firm_event* /* fe */
#endif
);

extern int alphaKbdProc(
#if NeedFunctionPrototypes
    DeviceIntPtr /* pKeyboard */,
    int /* what */
#endif
);

extern int alphaMouseProc(
#if NeedFunctionPrototypes
    DeviceIntPtr /* pMouse */,
    int /* what */
#endif
);

extern void alphaKbdWait(
#if NeedFunctionPrototypes
    void
#endif
);

/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))

extern Bool alphaCfbSetupScreen(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    pointer /* pbits */,	/* pointer to screen bitmap */
    int /* xsize */,		/* in pixels */
    int /* ysize */,
    int /* dpix */,		/* dots per inch */
    int /* dpiy */,		/* dots per inch */
    int /* width */,		/* pixel width of frame buffer */
    int	/* bpp */		/* bits per pixel of root */
#endif
);

extern Bool alphaCfbFinishScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    pointer /* pbits */,	/* pointer to screen bitmap */
    int /* xsize */,		/* in pixels */
    int /* ysize */,
    int /* dpix */,		/* dots per inch */
    int /* dpiy */,		/* dots per inch */
    int /* width */,		/* pixel width of frame buffer */
    int	/* bpp */		/* bits per pixel of root */
#endif
);

extern Bool alphaCfbScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    pointer /* pbits */,	/* pointer to screen bitmap */
    int /* xsize */,		/* in pixels */
    int /* ysize */,
    int /* dpix */,		/* dots per inch */
    int /* dpiy */,		/* dots per inch */
    int /* width */,		/* pixel width of frame buffer */
    int	/* bpp */		/* bits per pixel of root */
#endif
);

extern void alphaInstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr /* cmap */
#endif
);

extern void alphaUninstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr /* cmap */
#endif
);

extern int alphaListInstalledColormaps(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    Colormap* /* pCmapList */
#endif
);

#endif
