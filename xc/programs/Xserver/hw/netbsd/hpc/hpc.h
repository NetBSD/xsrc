/* $NetBSD: hpc.h,v 1.1 2000/05/06 06:01:49 takemura Exp $	*/
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

#ifndef _HPC_H_
#define _HPC_H_

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
#include <dev/hpc/hpcfbio.h>

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

#undef __P
#if NeedFunctionPrototypes
#   define __P(p) p
#else
#   define __P(p) ()
#endif

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

typedef struct wscons_event hpcEvent;

/*
 * Data private to any hpc keyboard.
 */
typedef struct {
    int		fd;
    int		devtype;       	/* type of device */
    int		type;		/* Type of keyboard */
    int		click;		/* kbd click save state */
    Leds	leds;		/* last known LED state */
    int		xlatestat;     	/* state machine for key code xlation */
    struct termios kbdtty;	/* previous tty settings */
} hpcKbdPrivRec, *hpcKbdPrivPtr;
extern hpcKbdPrivRec hpcKbdPriv;

#define HPC_KBDDEV_RAW		0
#define HPC_KBDDEV_WSKBD	1
#define HPC_KBDDEV_WSMUX	2
#define HPC_KBDXSTAT_INIT	0
#define HPC_KBDXSTAT_EXT0	1
#define HPC_KBDXSTAT_EXT1	2
#define HPC_KBDXSTAT_EXT1_1D	3
#define HPC_KBDXSTAT_EXT1_9D	4

/*
 * Data private to any hpc device.
 */
typedef struct {
    int		fd;
    int		bmask;		/* last known button state */
} hpcPtrPrivRec, *hpcPtrPrivPtr;
extern hpcPtrPrivRec hpcPtrPriv;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} hpcModmapRec;

typedef struct {
    unsigned char*  fb;		/* Frame buffer itself */
    int		    fd;		/* frame buffer for ioctl()s, */
    struct hpcfb_fbconf info;	/* Frame buffer characteristics */
    void	    (*EnterLeave)();/* screen switch */
} hpcFbRec, *hpcFbPtr;

typedef struct {
    ColormapPtr		installedMap;
    CloseScreenProcPtr	CloseScreen;
    void		(*UpdateColormap)();
    Bool		hasHardwareCursor;
} hpcScreenRec, *hpcScreenPtr;

#ifdef XKB
extern Bool		noXkbExtension;
#endif

/*
 * hpcInit.c
 */
hpcFbPtr hpcGetScreenFb __P((ScreenPtr	pScreen));

/*
 * hpcIo.c
 */
void hpcCleanupFd __P((int));
void hpcEnqueueEvents __P((void));

/*
 * hpcKbd.c
 */
int hpcKbdProc __P((DeviceIntPtr pKeyboard, int what));
hpcEvent* hpcKbdGetEvents __P((hpcKbdPrivPtr, int*, Bool*));
void hpcKbdEnqueueEvent __P((DeviceIntPtr dev, hpcEvent* fe));

/*
 * hpcMouse.c
 */
int hpcMouseProc __P((DeviceIntPtr pMouse, int what));
hpcEvent* hpcMouseGetEvents __P((hpcPtrPrivPtr, int*, Bool*));
void hpcMouseEnqueueEvent __P((DeviceIntPtr dev, hpcEvent*));

/*
 * hpcScreen.c
 */
pointer hpcMemoryMap __P((size_t len, off_t off, int fd));
Bool hpcScreenInit __P((ScreenPtr pScreen));
hpcScreenPtr hpcGetScreenPrivate __P((ScreenPtr	pScreen));

/*
 * hpcFB.c
 */
Bool hpcFBInit __P((int scrn, ScreenPtr pScrn, int argc, char** argv));

#endif
