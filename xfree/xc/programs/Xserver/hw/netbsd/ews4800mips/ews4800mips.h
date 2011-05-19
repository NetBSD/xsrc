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

#ifndef _EWS4800MIPS_H_
#define	_EWS4800MIPS_H_

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

#include <dev/wscons/wsconsio.h>
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

typedef struct wscons_event ews4800mipsEvent;

typedef struct s_ews4800mipsPtrPriv *ews4800mipsPtrPrivPtr;

/*
 * Data private to any ews4800mips keyboard.
 */
typedef struct {
	int fd;		/* fd of device */
	int type;	/* type of device */
	int layout;	/* The type of the layout */
	int click;	/* kbd click save state */
	Leds leds;	/* last known LED state */
} ews4800mipsKbdPrivRec, *ews4800mipsKbdPrivPtr;
extern ews4800mipsKbdPrivRec ews4800mipsKbdPriv;

/*
 * Data private to any ews4800mips pointer.
 */
typedef struct s_ews4800mipsPtrPriv {
	int fd;		/* fd of device */
	int bmask;		/* last known button state */
} ews4800mipsPtrPrivRec;
extern ews4800mipsPtrPrivRec ews4800mipsPtrPriv;

typedef struct {
	BYTE key;
	CARD8 modifiers;
} ews4800mipsModmapRec;

typedef struct {
	unsigned char *fb;	/* Frame buffer itself */
	int fd;			/* frame buffer for ioctl()s, */
	struct wsdisplay_fbinfo info;
	void (*EnterLeave)();	/* screen switch */
	char *devname;		/* device name (e.g. "/dev/ttyE0") */
} ews4800mipsFbRec, *ews4800mipsFbPtr;

typedef struct {
	ColormapPtr installedMap;
	CloseScreenProcPtr CloseScreen;
	void (*UpdateColormap)();
	Bool hasHardwareCursor;
} ews4800mipsScreenRec, *ews4800mipsScreenPtr;

#ifdef XKB
extern Bool noXkbExtension;
#endif

#define	ews4800mipsError(str) {						\
	int mode;							\
	ews4800mipsSetDisplayMode(fileno(stderr),			\
	    WSDISPLAYIO_MODE_EMUL, &mode);				\
	Error(str);							\
	ews4800mipsSetDisplayMode(fileno(stderr), mode, NULL);		\
}

#define	ews4800mipsErrorF(a) {						\
	int mode;							\
	ews4800mipsSetDisplayMode(fileno(stderr),			\
	    WSDISPLAYIO_MODE_EMUL, &mode);				\
	ErrorF a;							\
	ews4800mipsSetDisplayMode(fileno(stderr), mode, NULL);		\
}

#define	ews4800mipsFatalError(a) {					\
	int mode;							\
	ews4800mipsSetDisplayMode(fileno(stderr),			\
	    WSDISPLAYIO_MODE_EMUL, &mode);				\
	FatalError a;							\
}

/*
 * ews4800mipsColormap.c
 */
void ews4800mipsColormapInit(ScreenPtr);

/*
 * ews4800mipsInit.c
 */
ews4800mipsFbPtr ews4800mipsGetScreenFb(ScreenPtr);

/*
 * ews4800mipsIo.c
 */
void ews4800mipsCleanupFd(int);
void ews4800mipsEnqueueEvents(void);

/*
 * ews4800mipsKbd.c
 */
int ews4800mipsKbdProc(DeviceIntPtr, int);
ews4800mipsEvent* ews4800mipsKbdGetEvents(ews4800mipsKbdPrivPtr, int *, Bool *);
void ews4800mipsKbdEnqueueEvent(DeviceIntPtr, ews4800mipsEvent *);

/*
 * ews4800mipsMouse.c
 */
int ews4800mipsMouseProc(DeviceIntPtr, int);
ews4800mipsEvent* ews4800mipsMouseGetEvents(ews4800mipsPtrPrivPtr, int *,
    Bool *);
void ews4800mipsMouseEnqueueEvent(DeviceIntPtr, ews4800mipsEvent *);

/*
 * ews4800mipsScreen.c
 */
pointer ews4800mipsMemoryMap(size_t, off_t, int);
Bool ews4800mipsScreenInit(ScreenPtr);
ews4800mipsScreenPtr ews4800mipsGetScreenPrivate(ScreenPtr);
Bool ews4800mipsAllocateScreenPrivate(ScreenPtr);

/*
 * ews4800mipsFB.c
 */
Bool ews4800mipsFBInit(int, ScreenPtr, int, char **);
int ews4800mipsSetDisplayMode(int, int, int *);

#endif
