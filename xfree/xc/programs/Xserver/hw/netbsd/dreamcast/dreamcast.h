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

#ifndef _DREAMCAST_H_
#define _DREAMCAST_H_

#include "Xos.h"
#include "X.h"
#include "Xproto.h"

#include <stdlib.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <memory.h>

#include <sys/param.h>
#include <sys/file.h>
#include <sys/filio.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <dev/wscons/wsconsio.h>
#include <dev/wscons/wsdisplay_usl_io.h>

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

/* Given a struct timeval, convert its time into milliseconds... */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))
#define TSTOMILLI(ts)	(((ts).tv_nsec/1000000)+((ts).tv_sec*1000))

typedef struct wscons_event dreamcastEvent;

typedef struct s_dreamcastPtrPriv * dreamcastPtrPrivPtr;

/*
 * Data private to any dreamcast keyboard.
 */
typedef struct {
    int			fd;		/* fd of device */
    int			type;       	/* type of device */
    int			layout;		/* The type of the layout */
    int			click;		/* kbd click save state */
    Leds		leds;		/* last known LED state */
} dreamcastKbdPrivRec, *dreamcastKbdPrivPtr;
extern dreamcastKbdPrivRec dreamcastKbdPriv;

#define DREAMCAST_KBDTYPE_MAPLE		0

#define DREAMCAST_KBDLAYOUT_JP		0
#define DREAMCAST_KBDLAYOUT_UK		1
#define DREAMCAST_KBDLAYOUT_US		2

#define MIN_KEYCODE	8	/* XXX: necessary to avoid the mouse buttons */
#define MAX_KEYCODE	255	/* XXX */

/*
 * Data private to any dreamcast pointer.
 */
typedef struct s_dreamcastPtrPriv {
    int			fd;		/* fd of device */
    int			bmask;		/* last known button state */
} dreamcastPtrPrivRec;
extern dreamcastPtrPrivRec dreamcastPtrPriv;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} dreamcastModmapRec;

typedef struct {
    unsigned char*  fb;		/* Frame buffer itself */
    int		    fd;		/* frame buffer for ioctl()s, */
    struct wsdisplay_fbinfo info; /* */
    void	    (*EnterLeave)(ScreenPtr, int);/* screen switch */
    char*           devname;	/* device name (e.g. "/dev/ttyE0") */
} dreamcastFbRec, *dreamcastFbPtr;

typedef struct {
    ColormapPtr		installedMap;
    CloseScreenProcPtr	CloseScreen;
    void		(*UpdateColormap)(ScreenPtr, int, int, u_char *, u_char *, u_char *);
    Bool		hasHardwareCursor;
} dreamcastScreenRec, *dreamcastScreenPtr;

#ifdef XKB
extern Bool		noXkbExtension;
#endif

#define dreamcastError(str)	do { \
	int __m; \
	dreamcastSetDisplayMode(fileno(stderr), WSDISPLAYIO_MODE_EMUL, &__m); \
	Error(str); \
	dreamcastSetDisplayMode(fileno(stderr), __m, NULL); \
} while (/*CONSTCOND*/0)

#define dreamcastErrorF(a)	do { \
	int __m; \
	dreamcastSetDisplayMode(fileno(stderr), WSDISPLAYIO_MODE_EMUL, &__m); \
	ErrorF a; \
	dreamcastSetDisplayMode(fileno(stderr), __m, NULL); \
} while (/*CONSTCOND*/0)

#define dreamcastFatalError(a)	do { \
	int __m; \
	dreamcastSetDisplayMode(fileno(stderr), WSDISPLAYIO_MODE_EMUL, &__m); \
	FatalError a; \
} while (/*CONSTCOND*/0)

/*
 * dreamcastColormap.c
 */
void dreamcastColormapInit(ScreenPtr pScreen);

/*
 * dreamcastInit.c
 */
dreamcastFbPtr dreamcastGetScreenFb(ScreenPtr pScreen);

/*
 * dreamcastIo.c
 */
void dreamcastCleanupFd(int);
void dreamcastEnqueueEvents(void);

/*
 * dreamcastKbd.c
 */
int dreamcastKbdProc(DeviceIntPtr pKeyboard, int what);
dreamcastEvent* dreamcastKbdGetEvents(dreamcastKbdPrivPtr, int*, Bool*);
void dreamcastKbdEnqueueEvent(DeviceIntPtr dev, dreamcastEvent* fe);

/*
 * dreamcastMouse.c
 */
int dreamcastMouseProc(DeviceIntPtr pMouse, int what);
dreamcastEvent* dreamcastMouseGetEvents(dreamcastPtrPrivPtr, int*, Bool*);
void dreamcastMouseEnqueueEvent(DeviceIntPtr dev, dreamcastEvent*);

/*
 * dreamcastScreen.c
 */
pointer dreamcastMemoryMap(size_t len, off_t off, int fd);
Bool dreamcastScreenInit(ScreenPtr pScreen);
Bool dreamcastAllocateScreenPrivate(ScreenPtr pScreen);
dreamcastScreenPtr dreamcastGetScreenPrivate(ScreenPtr	pScreen);

/*
 * dreamcastFB.c
 */
Bool dreamcastFBInit(int scrn, ScreenPtr pScrn, int argc, char** argv);
int dreamcastSetDisplayMode(int, int, int *);

#endif
