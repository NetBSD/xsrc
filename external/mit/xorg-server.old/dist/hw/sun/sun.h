
/* $Xorg: sun.h,v 1.3 2000/08/17 19:48:29 cpqbld Exp $ */

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

/* $XFree86: xc/programs/Xserver/hw/sun/sun.h,v 3.13 2003/11/17 22:20:36 dawes Exp $ */

#ifndef _SUN_H_
#define _SUN_H_

/* X headers */
#include <X11/Xos.h>
#undef index /* don't mangle silly Sun structure member names */
#include <X11/X.h>
#include <X11/Xproto.h>

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

#ifdef SVR4
# ifdef X_POSIX_C_SOURCE
#  define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#  include <signal.h>
#  undef _POSIX_C_SOURCE
# else
#  define _POSIX_SOURCE
#  include <signal.h>
#  undef _POSIX_SOURCE
# endif
#endif

#include <fcntl.h>

#ifndef __bsdi__
# ifndef CSRG_BASED
#  ifndef i386
#   include <poll.h>
#  else
#   include <sys/poll.h>
#  endif
# endif
#else
# include <unistd.h>
#endif

#include <errno.h>
#include <memory.h>
#include <signal.h>


/*
 * Sun specific headers Sun moved in Solaris, and are different for NetBSD.
 *
 * Even if only needed by one source file, I have put them here
 * to simplify finding them...
 */
#ifdef SVR4
# include <sys/fbio.h>
# include <sys/kbd.h>
# include <sys/kbio.h>
# include <sys/msio.h>
# include <sys/vuid_event.h>
# include <sys/memreg.h>
# include <stropts.h>
# define usleep(usec) poll((struct pollfd *) 0, (size_t) 0, usec / 1000)
#else
# ifndef CSRG_BASED
#  include <sun/fbio.h>
#  include <sundev/kbd.h>
#  include <sundev/kbio.h>
#  include <sundev/msio.h>
#  include <sundev/vuid_event.h>
#  include <pixrect/pixrect.h>
#  include <pixrect/memreg.h>
extern int ioctl();
extern int getrlimit();
extern int setrlimit();
extern int getpagesize();
# else
#  if defined(CSRG_BASED) && !defined(__bsdi__) && !defined(__NetBSD__)
#   include <machine/fbio.h>
#   include <machine/kbd.h>
#   include <machine/kbio.h>
#   include <machine/vuid_event.h>
#  endif
#  ifdef __bsdi__
#   include <sys/fbio.h>
#   include </sys/sparc/dev/kbd.h>
#   include </sys/sparc/dev/kbio.h>
#   include </sys/sparc/dev/vuid_event.h>
#  endif
#  ifdef __NetBSD__
#   include <dev/sun/fbio.h>
#   include <machine/kbd.h>
#   include <dev/sun/kbio.h>	   /* also <sparc/kbio.h> -wsr */
#   include <dev/sun/vuid_event.h> /* also <sparc/vud_event.h> -wsr */
#  endif
# endif
#endif

/*
 * Sun doesn't see fit to add the TCX to <sys/fbio.h>
 */
#ifndef SVR4
/* On SunOS 4.1.x the TCX pretends to be a CG3 */
#define XFBTYPE_LASTPLUSONE	FBTYPE_LASTPLUSONE
#else
#define XFBTYPE_TCX		21
#define XFBTYPE_LASTPLUSONE	22
#endif

#include <sys/time.h>

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
#include "mipointer.h"

/*
 * ddx specific headers
 */
#ifndef PSZ
#define PSZ 8
#endif

extern int monitorResolution;


/* Frame buffer devices */
#ifdef SVR4
# define CGTWO0DEV	"/dev/fbs/cgtwo0"
# define CGTWO1DEV	"/dev/fbs/cgtwo1"
# define CGTWO2DEV	"/dev/fbs/cgtwo2"
# define CGTWO3DEV	"/dev/fbs/cgtwo3"
# define CGTHREE0DEV	"/dev/fbs/cgthree0"
# define CGTHREE1DEV	"/dev/fbs/cgthree1"
# define CGTHREE2DEV	"/dev/fbs/cgthree2"
# define CGTHREE3DEV	"/dev/fbs/cgthree3"
# define CGFOUR0DEV	"/dev/fbs/cgfour0"
# define CGSIX0DEV	"/dev/fbs/cgsix0"
# define CGSIX1DEV	"/dev/fbs/cgsix1"
# define CGSIX2DEV	"/dev/fbs/cgsix2"
# define CGSIX3DEV	"/dev/fbs/cgsix3"
# define BWTWO0DEV	"/dev/fbs/bwtwo0"
# define BWTWO1DEV	"/dev/fbs/bwtwo1"
# define BWTWO2DEV	"/dev/fbs/bwtwo2"
# define BWTWO3DEV	"/dev/fbs/bwtwo3"
# define CGEIGHT0DEV	"/dev/fbs/cgeight0"
# define TCX0DEV	"/dev/fbs/tcx0"
#else
# define CGTWO0DEV	"/dev/cgtwo0"
# define CGTWO1DEV	"/dev/cgtwo1"
# define CGTWO2DEV	"/dev/cgtwo2"
# define CGTWO3DEV	"/dev/cgtwo3"
# define CGTHREE0DEV	"/dev/cgthree0"
# define CGTHREE1DEV	"/dev/cgthree1"
# define CGTHREE2DEV	"/dev/cgthree2"
# define CGTHREE3DEV	"/dev/cgthree3"
# define CGFOUR0DEV	"/dev/cgfour0"
# define CGSIX0DEV	"/dev/cgsix0"
# define CGSIX1DEV	"/dev/cgsix1"
# define CGSIX2DEV	"/dev/cgsix2"
# define CGSIX3DEV	"/dev/cgsix3"
# define BWTWO0DEV	"/dev/bwtwo0"
# define BWTWO1DEV	"/dev/bwtwo1"
# define BWTWO2DEV	"/dev/bwtwo2"
# define BWTWO3DEV	"/dev/bwtwo3"
# define CGEIGHT0DEV	"/dev/cgeight0"
#endif

/*
 * MAXEVENTS is the maximum number of events the mouse and keyboard functions
 * will read on a given call to their GetEvents vectors.
 */
#define MAXEVENTS 	32

/*
 * Data private to any sun keyboard.
 */
typedef struct {
    int		fd;
    int		type;		/* Type of keyboard */
    int		layout;		/* The layout of the keyboard */
    int		click;		/* kbd click save state */
    Leds	leds;		/* last known LED state */
} sunKbdPrivRec, *sunKbdPrivPtr;

extern sunKbdPrivRec sunKbdPriv;

/*
 * Data private to any sun pointer device.
 */
typedef struct {
    int		fd;
    int		bmask;		/* last known button state */
} sunPtrPrivRec, *sunPtrPrivPtr;

extern sunPtrPrivRec sunPtrPriv;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} SunModmapRec;

typedef struct {
    int		    width, height;
    Bool	    has_cursor;
    CursorPtr	    pCursor;		/* current cursor */
} sunCursorRec, *sunCursorPtr;

#define NCMAP	256
typedef struct {
    u_char	    origRed[NCMAP];
    u_char	    origGreen[NCMAP];
    u_char	    origBlue[NCMAP];
} sunCmapRec, *sunCmapPtr;

typedef struct {
    ColormapPtr	    installedMap;
    CloseScreenProcPtr CloseScreen;
    void	    (*UpdateColormap)(ScreenPtr, int, int, u_char *, u_char *, u_char *);
    void	    (*GetColormap)(ScreenPtr, int, int, u_char *, u_char *, u_char *);
    Bool	    origColormapValid;
    sunCmapRec	    origColormap;
    void	    (*RestoreColormap)(ScreenPtr);
    sunCursorRec    hardwareCursor;
    Bool	    hasHardwareCursor;
} sunScreenRec, *sunScreenPtr;

extern DevPrivateKeyRec sunScreenPrivateKeyRec;
#define sunScreenPrivateKey (&sunScreenPrivateKeyRec)
#define sunSetScreenPrivate(pScreen, v) \
    dixSetPrivate(&(pScreen)->devPrivates, sunScreenPrivateKey, (v))
#define sunGetScreenPrivate(pScreen) ((sunScreenRec *) \
    dixLookupPrivate(&(pScreen)->devPrivates, sunScreenPrivateKey))

typedef struct {
    unsigned char*  fb;		/* Frame buffer itself */
    int		    fd;		/* frame buffer for ioctl()s, */
    struct fbtype   info;	/* Frame buffer characteristics */
    void	    (*EnterLeave)(ScreenPtr, int);/* screen switch */
    unsigned char*  fbPriv;	/* fbattr stuff, for the real type */
} fbFd;

typedef Bool (*sunFbInitProc)(
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
);

typedef struct {
    sunFbInitProc	init;	/* init procedure for this fb */
    const char		*name;	/* /usr/include/fbio names */
} sunFbDataRec;

/* sunInit.c */
extern EventList	*sunEvents;
extern sunFbDataRec	sunFbData[];
extern fbFd		sunFbs[];
extern Bool		sunSwapLkeys;
extern Bool		sunForceMono;
extern Bool		sunDebug;
extern char		*sunDeviceList;
extern Bool		sunFlipPixels;
extern Bool		sunFbInfo;
extern Bool		sunCG4Frob;
extern Bool		sunNoGX;

/* sunKeyMap.c */
extern KeySymsRec	sunKeySyms[];
extern const int	sunMaxLayout;
extern KeySym		*sunType4KeyMaps[];

/* sunKbd.c */
extern long		sunAutoRepeatInitiate;
extern long		sunAutoRepeatDelay;
extern DeviceIntPtr	sunKeyboardDevice;

/* sunMouse.c */
extern Bool		sunActiveZaphod;
extern DeviceIntPtr	sunPointerDevice;
extern miPointerScreenFuncRec sunPointerScreenFuncs;

/* sunFbs.c */
extern int		sunScreenIndex;

/* sunCursor.c */
extern Bool sunCursorInitialize(ScreenPtr);
extern void sunDisableCursor(ScreenPtr);

/* sunInit.c */
extern void sunNonBlockConsoleOff(
#if defined(SVR4) || defined(CSRG_BASED)
    void
#else
    char* /* arg */
#endif
);

/* sunIo.c */
extern void sunEnqueueEvents(void);

/* sunGX.c */
extern int sunGXInit(ScreenPtr, fbFd *);

/* sunFbs.c */
extern Bool sunSaveScreen(ScreenPtr, int);
extern Bool sunScreenInit(ScreenPtr);
extern pointer sunMemoryMap(size_t, off_t, int);
extern Bool sunScreenAllocate(ScreenPtr);
extern Bool sunInitCommon(int, ScreenPtr, off_t,
    Bool (* /* init1 */)(ScreenPtr, pointer, int, int, int, int, int, int),
    void (* /* init2 */)(ScreenPtr),
    Bool (* /* cr_cm */)(ScreenPtr),
    Bool (* /* save */)(ScreenPtr, int),
    int);

/* sunKbd.c */
extern int sunChangeKbdTranslation(int, Bool);
extern Firm_event* sunKbdGetEvents(int, Bool, int *, Bool *);
extern void sunKbdEnqueueEvent(DeviceIntPtr, Firm_event *);
extern int sunKbdProc(DeviceIntPtr, int);
extern void sunKbdWait(void);

/* sunMouse.c */
extern Firm_event* sunMouseGetEvents(int, Bool, int *, Bool *);
extern void sunMouseEnqueueEvent(DeviceIntPtr, Firm_event *);
extern int sunMouseProc(DeviceIntPtr, int);

/* sunCfb.c */
Bool sunCG3Init(int, ScreenPtr, int, char **);
Bool sunTCXInit(int, ScreenPtr, int, char **);
Bool sunCG2Init(int, ScreenPtr, int, char **);
Bool sunCG4Init(int, ScreenPtr, int, char **);
Bool sunCG6Init(int, ScreenPtr, int, char **);

/* sunCfb24.c */
Bool sunCG8Init(int, ScreenPtr, int, char **);

/* sunMfb.c */
Bool sunBW2Init(int, ScreenPtr, int, char **);

/* XXX */
extern void mfbDoBitblt(DrawablePtr, DrawablePtr, int, RegionPtr, DDXPointPtr);

/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))

extern Bool sunCfbSetupScreen(
    ScreenPtr /* pScreen */,
    pointer /* pbits */,	/* pointer to screen bitmap */
    int /* xsize */,		/* in pixels */
    int /* ysize */,
    int /* dpix */,		/* dots per inch */
    int /* dpiy */,		/* dots per inch */
    int /* width */,		/* pixel width of frame buffer */
    int	/* bpp */		/* bits per pixel of root */
);

extern Bool sunCfbFinishScreenInit(
    ScreenPtr /* pScreen */,
    pointer /* pbits */,	/* pointer to screen bitmap */
    int /* xsize */,		/* in pixels */
    int /* ysize */,
    int /* dpix */,		/* dots per inch */
    int /* dpiy */,		/* dots per inch */
    int /* width */,		/* pixel width of frame buffer */
    int	/* bpp */		/* bits per pixel of root */
);

extern Bool sunCfbScreenInit(
    ScreenPtr /* pScreen */,
    pointer /* pbits */,	/* pointer to screen bitmap */
    int /* xsize */,		/* in pixels */
    int /* ysize */,
    int /* dpix */,		/* dots per inch */
    int /* dpiy */,		/* dots per inch */
    int /* width */,		/* pixel width of frame buffer */
    int	/* bpp */		/* bits per pixel of root */
);

extern void sunInstallColormap(
    ColormapPtr /* cmap */
);

extern void sunUninstallColormap(
    ColormapPtr /* cmap */
);

extern int sunListInstalledColormaps(
    ScreenPtr /* pScreen */,
    Colormap* /* pCmapList */
);

#endif
