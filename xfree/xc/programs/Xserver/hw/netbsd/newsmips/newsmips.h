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

#ifndef _NEWSMIPS_H_
#define	_NEWSMIPS_H_

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

#include <machine/wsconsio.h>
#include <dev/wscons/wsdisplay_usl_io.h>

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
#define	PSZ 8
#endif

#include "mipointer.h"

extern int monitorResolution;

/*
 * MAXEVENTS is the maximum number of events the mouse and keyboard functions
 * will read on a given call to their GetEvents vectors.
 */
#define	MAXEVENTS 	32

/* Given a struct timeval, convert its time into milliseconds... */
#define	TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))
#define	TSTOMILLI(ts)	(((ts).tv_nsec/1000000)+((ts).tv_sec*1000))

typedef struct wscons_event newsmipsEvent;

typedef struct s_newsmipsPtrPriv *newsmipsPtrPrivPtr;

/*
 * Data private to any newsmips keyboard.
 */
typedef struct {
	int fd;		/* fd of device */
	int type;	/* type of device */
	int layout;	/* The type of the layout */
	int click;	/* kbd click save state */
	Leds leds;	/* last known LED state */
} newsmipsKbdPrivRec, *newsmipsKbdPrivPtr;
extern newsmipsKbdPrivRec newsmipsKbdPriv;

/*
 * Data private to any newsmips pointer.
 */
typedef struct s_newsmipsPtrPriv {
	int fd;		/* fd of device */
	int bmask;		/* last known button state */
} newsmipsPtrPrivRec;
extern newsmipsPtrPrivRec newsmipsPtrPriv;

typedef struct {
	BYTE key;
	CARD8 modifiers;
} newsmipsModmapRec;

typedef struct {
	unsigned char *fb;	/* Frame buffer itself */
	int fd;			/* frame buffer for ioctl()s, */
	struct newsmips_wsdisplay_fbinfo info;
	void (*EnterLeave)();	/* screen switch */
	char *devname;		/* device name (e.g. "/dev/ttyE0") */
} newsmipsFbRec, *newsmipsFbPtr;

typedef struct {
	ColormapPtr installedMap;
	CloseScreenProcPtr CloseScreen;
	void (*UpdateColormap)();
	Bool hasHardwareCursor;
} newsmipsScreenRec, *newsmipsScreenPtr;

#ifdef XKB
extern Bool noXkbExtension;
#endif

#define	newsmipsError(str) {						\
	int mode;							\
	newsmipsSetDisplayMode(fileno(stderr),			\
	    WSDISPLAYIO_MODE_EMUL, &mode);				\
	Error(str);							\
	newsmipsSetDisplayMode(fileno(stderr), mode, NULL);		\
}

#define	newsmipsErrorF(a) {						\
	int mode;							\
	newsmipsSetDisplayMode(fileno(stderr),			\
	    WSDISPLAYIO_MODE_EMUL, &mode);				\
	ErrorF a;							\
	newsmipsSetDisplayMode(fileno(stderr), mode, NULL);		\
}

#define	newsmipsFatalError(a) {					\
	int mode;							\
	newsmipsSetDisplayMode(fileno(stderr),			\
	    WSDISPLAYIO_MODE_EMUL, &mode);				\
	FatalError a;							\
}

void newsmipsColormapInit(ScreenPtr);

/*
 * newsmipsColormap.c
 */
/*
 * newsmipsInit.c
 */
newsmipsFbPtr newsmipsGetScreenFb(ScreenPtr);

/*
 * newsmipsIo.c
 */
void newsmipsCleanupFd(int);
void newsmipsEnqueueEvents(void);

/*
 * newsmipsKbd.c
 */
int newsmipsKbdProc(DeviceIntPtr, int);
newsmipsEvent* newsmipsKbdGetEvents(newsmipsKbdPrivPtr, int *, Bool *);
void newsmipsKbdEnqueueEvent(DeviceIntPtr, newsmipsEvent *);

/*
 * newsmipsMouse.c
 */
int newsmipsMouseProc(DeviceIntPtr, int);
newsmipsEvent* newsmipsMouseGetEvents(newsmipsPtrPrivPtr, int *,
    Bool *);
void newsmipsMouseEnqueueEvent(DeviceIntPtr, newsmipsEvent *);

/*
 * newsmipsScreen.c
 */
pointer newsmipsMemoryMap(size_t, off_t, int);
Bool newsmipsScreenInit(ScreenPtr);
newsmipsScreenPtr newsmipsGetScreenPrivate(ScreenPtr);
Bool newsmipsAllocateScreenPrivate(ScreenPtr);

/*
 * newsmipsFB.c
 */
Bool newsmipsFBInit(int, ScreenPtr, int, char **);
int newsmipsSetDisplayMode(int, int, int *);

#endif
