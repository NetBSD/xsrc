
/* $XConsortium: amiga.h,v 5.38 94/03/28 14:35:05 kaleb Exp $ */

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

#ifndef _AMIGA_H_ 
#define _AMIGA_H_

#include <errno.h>
#include <signal.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/device.h>
#include <dev/kbdreg.h>
#include <dev/vuid_event.h>
#include <dev/grfabs_reg.h>
#include <dev/viewioctl.h>
#include <dev/grfioctl.h>
#include <dev/grfvar.h>

/* X headers */
#include "Xos.h"
#undef index /* don't mangle silly Amiga structure member names */
#include "X.h"
#include "Xproto.h"

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

#define MAX_COLORS 256

/*
 * MAXEVENTS is the maximum number of events the mouse and keyboard functions
 * will read on a given call to their GetEvents vectors.
 */
#define MAXEVENTS 	32

/*
 * Data private to any amiga keyboard.
 */
typedef struct {
    int		fd;
    int		type;
} amigaKbdPrivRec, *amigaKbdPrivPtr;

extern amigaKbdPrivRec amigaKbdPriv;

/*
 * Data private to any amiga pointer device.
 */
typedef struct {
    int		fd;
#define mseFd fd
    int		bmask;		/* last known button state */
    int		mousetype;	/* -1: FIRM, 0: serial */
    char	*mousename;
    int		dx,dy;
    u_char	fb;

} amigaPtrPrivRec, *amigaPtrPrivPtr;


extern amigaPtrPrivRec amigaPtrPriv;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} AmigaModmapRec;

typedef struct {
    int		    width, height;
    Bool	    has_cursor;
    CursorPtr	    pCursor;		/* current cursor */
} amigaCursorRec, *amigaCursorPtr;

typedef struct {
    ColormapPtr	    installedMap;
    CloseScreenProcPtr CloseScreen;
    void	    (*UpdateColormap)();
    amigaCursorRec    hardwareCursor;
    Bool	    hasHardwareCursor;
} amigaScreenRec, *amigaScreenPtr;

#define GetScreenPrivate(s)   ((amigaScreenPtr) ((s)->devPrivates[amigaScreenIndex].ptr))
#define SetupScreen(s)	amigaScreenPtr	pPrivate = GetScreenPrivate(s)

struct viewinfo {
    bmap_t bm;
    struct view_size vs;
    colormap_t colormap;
    long entry [MAX_COLORS];
};

typedef struct {
    unsigned char*  fb;		/* Frame buffer itself */
    unsigned char*  regs;	/* for accel display, direct access regs */
    int		    fd;		/* frame buffer for ioctl()s, */
    struct grfinfo  info;	/* Frame buffer characteristics */
    struct viewinfo view;	/* Frame buffer characteristics */
    void	    (*EnterLeave)();/* screen switch */
    int		    type;	/* index into the amigaFbData table */
} fbFd;

typedef Bool (*amigaFbInitProc)(
#if NeedFunctionPrototypes
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
#endif
);

typedef struct {
    amigaFbInitProc	init;	 /* init procedure for this fb */
    char*		devname; /* device node that implements this fb */
} amigaFbDataRec;

#ifdef XKB
extern Bool		noXkbExtension;
#endif

extern Bool		amigaAutoRepeatHandlersInstalled;
extern long		amigaAutoRepeatInitiate;
extern long		amigaAutoRepeatDelay;
extern amigaFbDataRec	amigaFbData[];
extern fbFd		amigaFbs[];
extern Bool		amigaSwapLkeys;
extern Bool		amigaFlipPixels;
extern Bool		amigaActiveZaphod;
extern Bool		amigaNoGX;
extern int		amigaScreenIndex;
#ifdef GFX_CARD_SUPPORT
extern Bool		amigaUseHWC;  /* Hardware Cursor */
extern Bool		UseGfxHardwareCursor;  /* Hardware Cursor */
#ifdef CV64_SUPPORT
extern Bool		UseCVHardwareCursor;  /* Hardware Cursor */
extern int		amigaVirtualWidth; /* virtual screen width */
extern int		amigaVirtualHeight;
extern int		amigaRealWidth; /* Real screen width */
extern int		amigaRealHeight;
#endif /* CV64_SUPPORT */
#endif /* GFX_CARD_SUPPORT */
#ifdef AMIGA_CC_COLOR
extern int		amigaCCWidth;
extern int		amigaCCHeight;
extern int 		amigaCCDepth;
extern int		amigaCCXOffset;
extern int 		amigaCCYOffset;
#endif /* AMIGA_CC_COLOR */

extern int 		amigaXdebug;  /* Flag for debugging output to /tmp/xlog */


/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))
#define TRACE(f)	do { ErrorF("[%s #%d] ",__FILE__,__LINE__); ErrorF f; } while(0)

/* amigaC.c */
extern Bool amigaCInit(
    int,
    ScreenPtr,
    int,
    char **
);

extern int xopen_view(
    void
);

extern Bool amigaCProbe(
    ScreenInfo *,
    int,
    int,
    int,
    char **
);

extern Bool amigaCCreate(
    ScreenInfo *,
    int,
    char **
);

/* amigaCfb.c */
extern void amigaInstallColormap(
    ColormapPtr /* cmap */
);

extern void amigaUninstallColormap(
    ColormapPtr /* cmap */
);

extern int amigaListInstalledColormaps(
    ScreenPtr /* pScreen */,
    Colormap* /* pCmapList */
);

extern void CGScreenInit(
    ScreenPtr
);

extern Bool amigaGRFInit(
    int,
    ScreenPtr,
    int,
    char **
);

extern Bool amigaRZ3Init(
    int,
    ScreenPtr,
    int,
    char **
);

extern Bool amigaCLInit(
    int,
    ScreenPtr,
    int,
    char **
);

extern int amigaGXInit(
    ScreenPtr /* pScreen */,
    fbFd* /* fb */
);

/* amigaCursor.c */
extern void amigaCVSetPanning(
    fbFd *,
    unsigned short,
    unsigned short
);

extern Bool amigaCursorInitialize(
    ScreenPtr /* pScreen */
);

extern void amigaDisableCursor(
    ScreenPtr /* pScreen */
);

/* amigaFbs.c */
extern pointer amigaMemoryMap(
    size_t /* len */,
    off_t /* off */,
    int /* fd */
);

extern Bool amigaScreenAllocate(
    ScreenPtr /* pScreen */
);

extern Bool amigaSaveScreen(
    ScreenPtr /* pScreen */,
    int /* on */
);

extern Bool amigaScreenInit(
    ScreenPtr /* pScreen */
);

extern Bool amigaInitCommon(
    int /* scrn */,
    ScreenPtr /* pScrn */,
    Bool (* /* init1 */)(),
    void (* /* init2 */)(),
    Bool (* /* cr_cm */)(),
    Bool (* /* save */)(),
    int /* fb_off */
);

/* amigaInit.c */
extern void amigaNonBlockConsoleOff(
    void /* no args */
);

extern void OsVendorPreInit(
    void
);

extern void OsVendorInit(
    void
);

extern void InitOutput(
    ScreenInfo *,
    int,
    char **
);

extern void InitInput(
    int,
    char **
);

extern Bool amigaCfbSetupScreen(
    ScreenPtr /* pScreen */,
    pointer /* pbits */,	/* pointer to screen bitmap */
    int /* xsize */,		/* in pixels */
    int /* ysize */,
    int /* dpix */,		/* dots per inch */
    int /* dpiy */,		/* dots per inch */
    int /* width */,		/* pixel width of frame buffer */
    int	/* bpp */		/* bits per pixel of root */
);

extern Bool amigaCfbFinishScreenInit(
    ScreenPtr /* pScreen */,
    pointer /* pbits */,	/* pointer to screen bitmap */
    int /* xsize */,		/* in pixels */
    int /* ysize */,
    int /* dpix */,		/* dots per inch */
    int /* dpiy */,		/* dots per inch */
    int /* width */,		/* pixel width of frame buffer */
    int	/* bpp */		/* bits per pixel of root */
);

extern Bool amigaCfbScreenInit(
    ScreenPtr /* pScreen */,
    pointer /* pbits */,	/* pointer to screen bitmap */
    int /* xsize */,		/* in pixels */
    int /* ysize */,
    int /* dpix */,		/* dots per inch */
    int /* dpiy */,		/* dots per inch */
    int /* width */,		/* pixel width of frame buffer */
    int	/* bpp */		/* bits per pixel of root */
);

extern Bool amigaCfbCreateGC(
    GCPtr
);

extern void OsVendorFatalError(
    void
);

extern void DPMSSet(
    int
);

extern int DPMSGet(
    int *
);

extern Bool DPMSSupported(
    void
);

/* amigaIo.c */
extern void ProcessInputEvents(
    void
);

extern void amigaEnqueueEvents(
    void
);

extern void AbortDDX(
    void
);

extern void ddxGiveUp(
    void
);

extern int ddxProcessArgument(
    int,
    char *[],
    int
);

extern void ddxUseMsg(
    void
);


/* amigaKbd.c */
extern int amigaCVChangeMode(
    int
);

extern void amigaKbdWait(
    void
);

extern int amigaKbdProc(
    DeviceIntPtr /* pKeyboard */,
    int /* what */
);

extern Firm_event* amigaKbdGetEvents(
    int /* fd */,
    Bool /* on */,
    int* /* pNumEvents */,
    Bool* /* pAgain */
);

extern void amigaKbdEnqueueEvent(
    DeviceIntPtr /* device */,
    Firm_event* /* fe */
);

extern void amigaEnqueueAutoRepeat(
    void
);

extern int amigaChangeKbdTranslation(
    int /* fd */,
    Bool /* makeTranslated */
);

extern Bool LegalModifier(
    unsigned int,
    DevicePtr
);

extern void amigaBlockHandler(
    int,
    pointer,
    pointer,
    pointer
);

extern void amigaWakeupHandler(
    int,
    pointer,
    unsigned long,
    pointer
);


/* amigaMfb.c */
extern Bool amigaCCInit(
    int,
    ScreenPtr,
    int,
    char **
);

/* amigaMouse.c */
extern int amigaMouseProc(
    DeviceIntPtr /* pMouse */,
    int /* what */
);

extern Firm_event* amigaMouseGetEvents(
    int /* fd */,
    Bool /* on */,
    int* /* pNumEvents */,
    Bool* /* pAgain */
);

extern Firm_event* amigaSerGetEvents(
    int /* fd */,
    Bool /* on */,
    int* /* pNumEvents */,
    Bool* /* pAgain */
);

extern void amigaMouseEnqueueEvent(
    DeviceIntPtr /* device */,
    Firm_event* /* fe */,
    Firm_event* /* fe_next */
);


#endif
