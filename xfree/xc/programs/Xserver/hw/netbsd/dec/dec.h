/*	$NetBSD: dec.h,v 1.2 2011/05/24 22:16:45 jakllsch Exp $	*/

/* XConsortium: sun.h,v 5.39.1.1 95/01/05 19:58:43 kaleb Exp */
/* XFree86: xc/programs/Xserver/hw/sun/sun.h,v 3.2 1995/02/12 02:36:21 dawes Exp */

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

#include <dev/wscons/wsconsio.h>

#define LK_KLL 8 /* from dev/dec/lk201var.h XXX */

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

#include <dev/pci/tgareg.h>
#include <dev/tc/sfbreg.h>

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
 * Data private to any dec keyboard.
 */
typedef struct {
    int		fd;
    int		type;		/* Type of keyboard */
    int		layout;		/* The layout of the keyboard */
    int		click;		/* kbd click save state */
    Leds	leds;		/* last known LED state */
    int		prevClick;	/* click state before server start */
    KeyCode     keys_down[LK_KLL]; /* which keys are down */
} decKbdPrivRec, *decKbdPrivPtr;

extern decKbdPrivRec decKbdPriv;

/*
 * Data private to any dec pointer device.
 */
typedef struct {
    int		fd;
    int		bmask;		/* last known button state */
} decPtrPrivRec, *decPtrPrivPtr;

extern decPtrPrivRec decPtrPriv;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} DecModmapRec;

typedef struct {
    int		    width, height;
    Bool	    has_cursor;
    CursorPtr	    pCursor;		/* current cursor */
} decCursorRec, *decCursorPtr;

typedef struct {
    ColormapPtr	    installedMap;
    CloseScreenProcPtr CloseScreen;
    void	    (*UpdateColormap)();
    decCursorRec    hardwareCursor;
    Bool	    hasHardwareCursor;
} decScreenRec, *decScreenPtr;

#define GetScreenPrivate(s)   ((decScreenPtr) ((s)->devPrivates[decScreenIndex].ptr))
#define SetupScreen(s)	decScreenPtr	pPrivate = GetScreenPrivate(s)

typedef Bool (*decFbInitProc)(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);

typedef struct {
    int			type;		/* WSDISPLAY_TYPE_<xxxxx> */
    int			softCursor;	/* don't use hardware cursor */
    char*		name;		/* textual description */
    decFbInitProc	init;		/* init procedure for this fb */
} decFbDataRec, *decFbDataPtr;

typedef struct {
    unsigned char*  fb;		/* Frame buffer itself */
    int		    fd;		/* frame buffer for ioctl()s, */
    int             width;	/* frame buffer characteristics */
    int             height;
    int             depth;
    int             type;
    int             cmsize;
    size_t          size;
    int		    offset;	/* offset into the fb */
    union {
	    tga_reg_t       *tgaregs[4];  /* Registers, and their aliases */
	    sfb_reg_t       *sfbregs[4];  /* Registers, and their aliases */
    } regs;
#define tgaregs0 regs.tgaregs[0]
#define tgaregs1 regs.tgaregs[1]
#define tgaregs2 regs.tgaregs[2]
#define tgaregs3 regs.tgaregs[3]
    void	    (*EnterLeave)();/* screen switch */
    decFbDataPtr  fbData;
} fbFd;

#ifdef XKB
extern Bool		noXkbExtension;
#endif

#if 0
extern Bool		sunAutoRepeatHandlersInstalled;
extern long		sunAutoRepeatInitiate;
extern long		sunAutoRepeatDelay;
#endif
extern decFbDataRec	decFbData[];
extern fbFd		decFbs[];
extern Bool		decActiveZaphod;
#if 0
extern Bool		sunSwapLkeys;
extern Bool		sunFlipPixels;
extern Bool		sunFbInfo;
extern Bool		sunCG4Frob;
extern Bool		sunNoGX;
#endif
extern int		decScreenIndex;
#if 0
extern int*		sunProtected;
#endif

extern Bool		decSoftCursor;
extern Bool		decHardCursor;
extern Bool		decAccelerate;
extern int		decWantedDepth;
extern char		*decKbdDev;
extern char		*decPtrDev;

extern Bool decCursorInitialize(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

extern void decDisableCursor(
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

extern void decEnqueueEvents(
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

extern Bool decSaveScreen(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    int /* on */
#endif
);

extern Bool decScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

extern Bool decCloseScreen(
#if NeedFunctionPrototypes
    int /* i */,
    ScreenPtr /* pScreen */
#endif
);

extern pointer decMemoryMap(
#if NeedFunctionPrototypes
    size_t /* len */,
    off_t /* off */,
    int /* fd */
#endif
);

extern Bool decScreenAllocate(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */
#endif
);

extern Bool decInitCommon(
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

extern struct wscons_event* decKbdGetEvents(
#if NeedFunctionPrototypes
    int /* fd */,
    int* /* pNumEvents */,
    Bool* /* pAgain */
#endif
);

extern struct wscons_event* decMouseGetEvents(
#if NeedFunctionPrototypes
    int /* fd */,
    int* /* pNumEvents */,
    Bool* /* pAgain */
#endif
);

extern void decKbdEnqueueEvent(
#if NeedFunctionPrototypes
    DeviceIntPtr /* device */,
    struct wscons_event * /* fe */
#endif
);

extern void decMouseEnqueueEvent(
#if NeedFunctionPrototypes
    DeviceIntPtr /* device */,
    struct wscons_event * /* fe */
#endif
);

extern int decKbdProc(
#if NeedFunctionPrototypes
    DeviceIntPtr /* pKeyboard */,
    int /* what */
#endif
);

extern int decMouseProc(
#if NeedFunctionPrototypes
    DeviceIntPtr /* pMouse */,
    int /* what */
#endif
);

extern void decKbdWait(
#if NeedFunctionPrototypes
    void
#endif
);

/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))

/*-
 * TSTOMILLI(ts)
 *	Given a struct timespec, convert its time into milliseconds...
 */
#define TSTOMILLI(ts)	(((ts).tv_nsec/1000000)+((ts).tv_sec*1000))

extern void decColormapScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr /* cmap */
#endif
);

extern void decInstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr /* cmap */
#endif
);

extern void decUninstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr /* cmap */
#endif
);

extern int decListInstalledColormaps(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    Colormap* /* pCmapList */
#endif
);

#endif	/* _ALPHA_H_ */
