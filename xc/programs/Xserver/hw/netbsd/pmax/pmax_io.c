/* $NetBSD: pmax_io.c,v 1.1 1999/04/17 17:18:57 ad Exp $ */

/*
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andy Doran <ad@NetBSD.org>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
 
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/proc.h>

#include <machine/tc_machdep.h>
#include <machine/fbio.h>
#include <machine/fbvar.h>
#include <machine/pmioctl.h>
#include <dev/dec/lk201.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "misc.h"
#include "X.h"
#define NEED_EVENTS
#include "Xproto.h"
#include "scrnintstr.h"
#include "pixmap.h"
#include "input.h"
#include "cursorstr.h"
#include "regionstr.h"
#include "resource.h"
#include "dixstruct.h"
#include "colormapst.h"

#include "mi.h"
#include "cfb.h"

static void pmaxQueryBestSize();
static Bool pmaxRealizeCursor();
static Bool pmaxUnrealizeCursor();
static Bool pmaxDisplayCursor();
static void pmaxRecolorCursor();
static Bool pmaxSetCursorPosition();
static void pmaxCursorLimits();
static void pmaxConstrainCursor();
static void pmaxPointerNonInterestBox();
static void pmaxChangeKeyboardControl();
static void pmaxChangePointerControl();
static void pmaxClick();
extern void pmaxStoreColors();
extern void pmaxInstallColormap();
extern void pmaxUninstallColormap();
extern int  pmaxListInstalledColormaps();

#define NOMAPYET        ((ColormapPtr) 1)

static ColormapPtr pInstalledMap = NOMAPYET;

/* these next two are DIX routines */
extern int      TellLostMap();
extern int      TellGainedMap();

static int     fdPM;
static PM_Info *scrInfo;
static pmEventQueue *queue;
static pmBox *mbox;
static pmCursor *mouse;
static int qLimit;
static int lastEventTime;
static DevicePtr pmKeyboard;
static DevicePtr pmPointer;
static int dpix = -1, dpiy = -1, dpi = -1;
static int class = PseudoColor;
extern int defaultColorVisualClass;
int hotX, hotY;

#define MAX_LED 3		/* only 3 LED's can be set by user; Lock LED
				 * is controlled by server */

#define VSYNCFIXED
#ifdef VSYNCFIXED
#define CURRENT_TIME	queue->timestamp_ms
#else
#define CURRENT_TIME	GetTimeInMillis()
#endif

#define NoSuchClass -1

typedef struct {
	BoxRec		cursorRange;
	BoxRec		cursorConstraint;
	ColormapPtr	pInstalledMap;
	Bool		(*CloseScreen)();
} pmaxScreenPrivate;

extern int pmaxScreenPrivateIndex;

static int
ParseClass(className)
	char   *className;
{
	int i;
	static char *names[] = {
		"StaticGray", 
		"GrayScale", 
		"StaticColor",
		"PseudoColor", 
		"TrueColor"
	};
	
	/* 
	 * Only the ones we support and must be in order from X.h, since
	 * return value depends on index into array. 
	 */
	for (i = 0; i < sizeof(names) / sizeof(char *); i++)
		if (strcmp(names[i], className) == 0)
			return i;

	return NoSuchClass;
}

static  Bool
commandLineMatch(argc, argv, pat, pmatch)
	int     argc;		/* may NOT be changed */
	char   *argv[];		/* may NOT be changed */
	char   *pat;
{
	int     ic;

	for (ic = 0; ic < argc; ic++)
		if (strcmp(argv[ic], pat) == 0)
			return TRUE;
	return FALSE;
}

static  Bool
commandLinePairMatch(argc, argv, pat, pmatch)
	int     argc;		/* may NOT be changed */
	char   *argv[];		/* may NOT be changed */
	char   *pat;
	char  **pmatch;		/* RETURN */
{
	int     ic;

	for (ic = 0; ic < argc; ic++)
		if (strcmp(argv[ic], pat) == 0) {
			*pmatch = argv[ic + 1];
			return TRUE;
		}
	return FALSE;
}


/* 
 * SaveScreen does blanking, so no need to worry about the interval timer 
 */
static Bool
pmaxSaveScreen(pScreen, on)
	ScreenPtr pScreen;
	int     on;
{
	if (on == SCREEN_SAVER_FORCER) {
		lastEventTime = CURRENT_TIME;
	} else {
		if (on == SCREEN_SAVER_ON)
			ioctl(fdPM, QIOVIDEOOFF);
		else
			ioctl(fdPM, QIOVIDEOON);
	}
	
	return (TRUE);
}


Bool
pmaxCloseScreen(index, pScreen)
	int     index;
	ScreenPtr pScreen;
{
    short tmp[32];
    
    pmaxScreenPrivate *sp = (pmaxScreenPrivate *)
		pScreen->devPrivates[pmaxScreenPrivateIndex].ptr;

    /* Remove the cursor image */
    bzero(tmp, sizeof tmp);
    ioctl(fdPM, QIOWCURSOR, tmp);
    
    /* Free our devPrivate stuff, close fdPM and call cfb/mfb CloseScreen */
    pScreen->CloseScreen = sp->CloseScreen;
    Xfree(sp);
    close(fdPM);
    return pScreen->CloseScreen(index, pScreen);
}

Bool
pmaxScreenInit(index, pScreen, argc, argv)
	int     index;
	ScreenPtr pScreen;
	int     argc;
	char  **argv;
{
        pmaxScreenPrivate *sp;
	PixmapPtr pPixmap;
	ColormapPtr pColormap;
	VisualPtr pVisual;
	int     i;
	struct fbtype fb;

	pScreen->devPrivates[pmaxScreenPrivateIndex].ptr = sp = 
	    (pmaxScreenPrivate *)Xalloc(sizeof(pmaxScreenPrivate));

	if ((fdPM = open("/dev/fb0", O_RDWR | O_NDELAY, 0)) < 0) {
		ErrorF("couldn't open /dev/fb0\n");
		return FALSE;
	}

	if (ioctl(fdPM, QIOCGINFO, &scrInfo) < 0) {
		ErrorF("QIOCGINFO ioctl failed: %s\n", strerror(errno));
		exit(1);
	}
		
	/* Damn DEC ioctls() aren't enough. Using the Sun ones too... */
	if (ioctl(fdPM, FBIOGTYPE, &fb) < 0) {
		ErrorF("FBIOGTYPE ioctl failed: %s\n", strerror(errno));
		exit(1);
	}

	ioctl(fdPM, QIOCINIT);
	ioctl(fdPM, QIOKERNLOOP);
	ioctl(fdPM, QIOVIDEOON);

	queue = &scrInfo->qe;
	mouse = &scrInfo->mouse;
	qLimit = queue->eSize - 1;
	lastEventTime = CURRENT_TIME;

	/* discard all the current input events */
	queue->eHead = queue->eTail;

	/*
         * set keyclick, mouse acceleration and threshold
         */
	{
		int     clicklevel;
		char   *clickvolume;
		char   *mouseAcceleration;
		int     ma = 4;
		char   *mouseThreshold;
		int     mt = 4;
		PtrCtrl ctrl;

		if (commandLinePairMatch(argc, argv, "c", &clickvolume)) {
			sscanf(clickvolume, "%d", &clicklevel);
			pmaxClick(clicklevel);
		}
		else if (commandLineMatch(argc, argv, "-c"))
			pmaxClick(0);
		
		/*
	         * calling pmaxChangePointerControl here may be unclean       XXX
	         */
		if (commandLinePairMatch(argc, argv, "-a", &mouseAcceleration))
			sscanf(mouseAcceleration, "%d", &ma);
		if (commandLinePairMatch(argc, argv, "-t", &mouseThreshold))
			sscanf(mouseThreshold, "%d", &mt);
		
		ctrl.num = ma;
		ctrl.den = 1;
		ctrl.threshold = mt;
		pmaxChangePointerControl((DevicePtr) NULL, &ctrl);
	}

	if (dpi == -1) {	/* dpi has not been set */
		if (dpix == -1) {	/* ie dpix has not been set */
			if (dpiy == -1) {
				dpix = 100;
				dpiy = 100;
			} else
				dpix = dpiy;
		} else {
			if (dpiy == -1)
				dpiy = dpix;
		}
	} else {
		dpix = dpi;
		dpiy = dpi;
	}

	pScreen->SaveScreen = pmaxSaveScreen;
	pScreen->RealizeCursor = pmaxRealizeCursor;
	pScreen->UnrealizeCursor = pmaxUnrealizeCursor;
	pScreen->DisplayCursor = pmaxDisplayCursor;
	pScreen->SetCursorPosition = pmaxSetCursorPosition;
	pScreen->CursorLimits = pmaxCursorLimits;
	pScreen->PointerNonInterestBox = pmaxPointerNonInterestBox;
	pScreen->ConstrainCursor = pmaxConstrainCursor;
	pScreen->RecolorCursor = pmaxRecolorCursor;
	pScreen->QueryBestSize = pmaxQueryBestSize;
	pScreen->StoreColors = pmaxStoreColors;
	pScreen->InstallColormap = pmaxInstallColormap;
	pScreen->UninstallColormap = pmaxUninstallColormap;
	pScreen->ListInstalledColormaps = pmaxListInstalledColormaps;

	switch (fb.fb_boardtype) {
	case PMAX_FBTYPE_PM_MONO:
		if (!mfbScreenInit(pScreen, scrInfo->bitmap, 1024, 864, dpix, 
		    dpiy, 2048)) {
		    	close(fdPM);
		    	return (FALSE);
		}

		mfbCreateDefColormap(pScreen);
		break;

	case PMAX_FBTYPE_SFB:
		if (fb.fb_depth != 8) {
			/* XXX */
    		}
    		/* FALLTHROUGH */
		
	case PMAX_FBTYPE_PM_COLOR:
	case PMAX_FBTYPE_XCFB:
	case PMAX_FBTYPE_CFB:
		defaultColorVisualClass = class;

		if (!cfbScreenInit(pScreen, scrInfo->bitmap, fb.fb_width, 
		    fb.fb_height, dpix, dpiy, fb.fb_width)) {
		    	close(fdPM);
		    	return (FALSE);
		}
		
		cfbCreateDefColormap(pScreen);
    		break;
		
	case PMAX_FBTYPE_MFB:
		/*
		 * XXX mfb is mono, but has 1 byte per pixel (only bit 0
		 * is significant). Am I doing this right?
		 */
		defaultColorVisualClass = StaticGray;
		cfbSetVisualTypes(1, StaticGray, 8);
	
		if (!cfbScreenInit(pScreen, scrInfo->bitmap, fb.fb_width, 
		    fb.fb_height, dpix, dpiy, fb.fb_width)) {
		    	close(fdPM);
		    	return (FALSE);
		}

		cfbCreateDefColormap(pScreen);
		break;

	default:
		ErrorF("pmaxScreenInit: unknown fbtype %d\n", fb.fb_boardtype);
		close(fdPM);
		return (FALSE);
	}
	
	sp->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = pmaxCloseScreen;
	sp->cursorRange.x1 = -15;
	sp->cursorRange.x2 = fb.fb_width - 1;
	sp->cursorRange.y1 = -15;
	sp->cursorRange.y2 = fb.fb_height - 1;
	return (TRUE);
}

static void
ChangeLED(led, on)
	int     led;
	Bool    on;
{
	struct pm_kpcmd ioc;

	switch (led) {
	case 1:
		ioc.par[0] = LED_1;
		break;
	case 2:
		ioc.par[0] = LED_2;
		break;
	case 3:
		/* 
		 * The keyboard's LED_3 is the Lock LED, which the server
		 * owns. So the user's LED #3 maps to the keyboard's LED_4. 
		 */
		ioc.par[0] = LED_4;
		break;
	default:
		return;		/* out-of-range LED value */
	}

	ioc.cmd = on ? LK_LED_ENABLE : LK_LED_DISABLE;
	ioc.par[1] = 0;
	ioc.nbytes = 1;
	ioctl(fdPM, QIOCKPCMD, &ioc);
}


static void
pmaxClick(click)
	int     click;
{
	struct pm_kpcmd ioc;

	if (click == 0) {
		ioc.nbytes = 0;
		ioc.cmd = 0x99;
	} else {
		ioc.nbytes = 1;
		ioc.cmd = LK_CL_ENABLE;
		ioc.par[0] = 7 - ((click / 14) & 7);
	}

	ioctl(fdPM, QIOCKPCMD, &ioc);
}

static void
pmaxChangeKeyboardControl(device, ctrl)
	DevicePtr device;
	KeybdCtrl *ctrl;
{
	int     i;

	pmaxClick(ctrl->click);

	for (i = 1; i <= MAX_LED; i++)
		ChangeLED(i, ctrl->leds & (1 << (i - 1)));

	SetLKAutoRepeat(ctrl->autoRepeat);
}

static void
pmaxBell(loud, pDevice)
	int     loud;
	DevicePtr pDevice;
{
	struct pm_kpcmd ioc;

	/* the lk201 volume is between 7 (quiet but audible) and 0 (loud) */
	loud = 7 - ((loud / 14) & 7);
	ioc.nbytes = 1;
	ioc.cmd = LK_BELL_ENABLE;
	ioc.par[0] = loud;
	ioctl(fdPM, QIOCKPCMD, &ioc);

	ioc.nbytes = 0;
	ioc.cmd = LK_RING_BELL;
	ioctl(fdPM, QIOCKPCMD, &ioc);
}


/*
 * These serve protocol requests, setting/getting acceleration and threshold.
 * X10 analog is "SetMouseCharacteristics".
 */
static void
pmaxChangePointerControl(device, ctrl)
	DevicePtr device;
	PtrCtrl *ctrl;
{
	scrInfo->mthreshold = ctrl->threshold;

	if (ctrl->num < ctrl->den)
		scrInfo->mscale = 1;
	else
		scrInfo->mscale = ctrl->num / ctrl->den;
}

static int
pmaxGetMotionEvents(pDevice, coords, start, stop, pScreen)
	DevicePtr pDevice;
	xTimecoord *coords;
	unsigned long start, stop;
	ScreenPtr pScreen;
{
	return (0);
}

int
pmaxMouseProc(pDev, onoff, argc, argv)
	DevicePtr pDev;
	int     onoff, argc;
	char   *argv[];
{
	int     i;
	BYTE    map[4];

	switch (onoff) {
	case DEVICE_INIT:
		pmPointer = pDev;
		pDev->devicePrivate = (pointer) & queue;
		map[1] = 1;
		map[2] = 2;
		map[3] = 3;

		InitPointerDeviceStruct(
		    pmPointer, map, 3, pmaxGetMotionEvents,
		    pmaxChangePointerControl, MOTION_BUFFER_SIZE);
		SetInputCheck((long *)&queue->eHead, (long *)&queue->eTail);
		hotX = hotY = 0;
		break;

	case DEVICE_ON:
		pDev->on = TRUE;
		AddEnabledDevice(fdPM);
		break;

	case DEVICE_OFF:
		pDev->on = FALSE;
		RemoveEnabledDevice(fdPM);
		break;

	case DEVICE_CLOSE:
		break;
	}
	return Success;
}

#define LK_REPEAT_ON  0xe3
#define LK_REPEAT_OFF 0xe1

int
SetLKAutoRepeat(onoff)
	Bool    onoff;
{
	extern char *AutoRepeatLKMode();
	extern char *UpDownLKMode();

	struct pm_kpcmd ioc;
	register char *divsets;
	divsets = onoff ? (char *) AutoRepeatLKMode() : (char *) UpDownLKMode();
	ioc.nbytes = 0;
	while (ioc.cmd = *divsets++)
		ioctl(fdPM, QIOCKPCMD, &ioc);
	ioc.cmd = ((onoff > 0) ? LK_REPEAT_ON : LK_REPEAT_OFF);
	return (ioctl(fdPM, QIOCKPCMD, &ioc));
}

int
pmaxKeybdProc(pDev, onoff, argc, argv)
	DevicePtr pDev;
	int     onoff, argc;
	char   *argv[];
{
	KeySymsRec keySyms;
	CARD8   modMap[MAP_LENGTH];

	switch (onoff) {
	case DEVICE_INIT:
		pmKeyboard = pDev;
		GetLK201Mappings(&keySyms, modMap);
		InitKeyboardDeviceStruct(pmKeyboard, &keySyms, modMap, 
			pmaxBell, pmaxChangeKeyboardControl);
		/* pmaxClick(20); */	
		Xfree(keySyms.map);
		break;
	case DEVICE_ON:
		pDev->on = TRUE;
		AddEnabledDevice(fdPM);
		break;
	case DEVICE_OFF:
		pDev->on = FALSE;
		RemoveEnabledDevice(fdPM);
		break;
	case DEVICE_CLOSE:
		break;
	}

	return Success;
}


/*
 * The driver has been set up to put events in the queue that are identical
 * in shape to the events that the DDX layer has to deliver to ProcessInput
 * in DIX.
 */
extern int screenIsSaved;

void
ProcessInputEvents(void)
{
#define DEVICE_KEYBOARD 2
	xEvent  x;
	pmEvent e;
	register int i;

	i = queue->eHead;
	while (i != queue->eTail) {
		e = *((pmEvent *) & queue->events[i]);
		if (screenIsSaved == SCREEN_SAVER_ON)
			SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);
		x.u.keyButtonPointer.rootX = e.x + hotX;
		x.u.keyButtonPointer.rootY = e.y + hotY;
		x.u.keyButtonPointer.time = lastEventTime = e.time;
		x.u.u.detail = e.key;

		if (e.device == DEVICE_KEYBOARD) {
			switch (e.type) {
			case BUTTON_DOWN_TYPE:
				x.u.u.type = KeyPress;
				(*pmKeyboard->processInputProc) (&x, (DeviceIntPtr)pmKeyboard, 1);
				break;
			case BUTTON_UP_TYPE:
				x.u.u.type = KeyRelease;
				(*pmKeyboard->processInputProc) (&x, (DeviceIntPtr)pmKeyboard, 1);
				break;
			default:	/* hopefully BUTTON_RAW_TYPE */
				ProcessLK201Input(&x, pmKeyboard);
			}
		} else {
			switch (e.type) {
			case BUTTON_DOWN_TYPE:
				x.u.u.type = ButtonPress;
				break;
			case BUTTON_UP_TYPE:
				x.u.u.type = ButtonRelease;
				break;
			case MOTION_TYPE:
				x.u.u.type = MotionNotify;
				break;
			default:
				ErrorF("Unknown input event = %d\n", e.type);
				continue;
			}
			(*pmPointer->processInputProc) (&x, (DeviceIntPtr)pmPointer, 1);
		}
		if (i == qLimit)
			i = queue->eHead = 0;
		else
			i = ++queue->eHead;
	}
#undef DEVICE_KEYBOARD
}


TimeSinceLastInputEvent()
{
	if (lastEventTime == 0)
		lastEventTime = CURRENT_TIME;
	return CURRENT_TIME - lastEventTime;
}


/*
 * set the bounds in the device for this particular cursor
 */
static void
pmaxConstrainCursor(pScr, pBox)
	ScreenPtr pScr;
	BoxPtr  pBox;
{
        pmaxScreenPrivate *sp = (pmaxScreenPrivate *)
		pScr->devPrivates[pmaxScreenPrivateIndex].ptr;
	
	sp->cursorConstraint = *pBox;

	scrInfo->max_cur_x = pBox->x2 - hotX - 1;
	scrInfo->min_cur_x = pBox->x1 - hotX;
	scrInfo->max_cur_y = pBox->y2 - hotY - 1;
	scrInfo->min_cur_y = pBox->y1 - hotY;
}


Bool
pmaxSetCursorPosition(pScr, newx, newy, generateEvent)
	ScreenPtr pScr;
	unsigned int newx;
	unsigned int newy;
	Bool    generateEvent;
{
	pmCursor pmCPos;
	xEvent  motion;

	mouse->x = pmCPos.x = newx - hotX;
	mouse->y = pmCPos.y = newy - hotY;
	
	if (ioctl(fdPM, QIOCPMSTATE, &pmCPos) < 0) {
		ErrorF("error warping cursor\n");
		return FALSE;
	}

	if (generateEvent) {
		if (queue->eHead != queue->eTail)
			ProcessInputEvents();
		motion.u.keyButtonPointer.rootX = newx;
		motion.u.keyButtonPointer.rootY = newy;
		motion.u.keyButtonPointer.time = currentTime.milliseconds;
		motion.u.u.type = MotionNotify;
		pmPointer->processInputProc(&motion, (DeviceIntPtr)pmPointer, 1);
	}

	return TRUE;
}


static  Bool
pmaxDisplayCursor(pScr, pCurs)
	ScreenPtr pScr;
	CursorPtr pCurs;
{
        pmaxScreenPrivate *sp = (pmaxScreenPrivate *)
		pScr->devPrivates[pmaxScreenPrivateIndex].ptr;
	int i, x, y;

	/*
         * load the cursor
         */
	if ((hotX != pCurs->bits->xhot) || (hotY != pCurs->bits->yhot)) {
		x = mouse->x + hotX;
		y = mouse->y + hotY;
		hotX = pCurs->bits->xhot;
		hotY = pCurs->bits->yhot;
		pmaxSetCursorPosition(pScr, x, y, FALSE);
		pmaxConstrainCursor(pScr, &sp->cursorConstraint);
		/* to update constraints in driver */
	}

	ioctl(fdPM, QIOWCURSOR, pCurs->devPriv[pScr->myNum]);
	pmaxRecolorCursor(pScr, pCurs, TRUE);
	return (TRUE);
}

static void
pmaxRecolorCursor(pScr, pCurs, displayed)
	ScreenPtr pScr;
	CursorPtr pCurs;
	Bool    displayed;
{
	u_int data[6];

	if (displayed) {
		data[0] = pCurs->backRed & 0xff00;
		data[1] = pCurs->backGreen & 0xff00;
		data[2] = pCurs->backBlue & 0xff00;
		data[3] = pCurs->foreRed & 0xff00;
		data[4] = pCurs->foreGreen & 0xff00;
		data[5] = pCurs->foreBlue & 0xff00;
		ioctl(fdPM, QIOWCURSORCOLOR, data);
	}
}

static  Bool
pmaxRealizeCursor(pScr, pCur)
	ScreenPtr pScr;
	CursorPtr pCur;
{
	int i, mr;
	
	pCur->devPriv[pScr->myNum] = (pointer)Xalloc(64); 
	bzero(pCur->devPriv[pScr->myNum], 64);
	
	mr = (pCur->bits->height > 16 ? 16 : pCur->bits->height);

	for (i = 0; i < mr; i++) {
		((u_short *)pCur->devPriv[pScr->myNum])[i] = 
			((u_int *)pCur->bits->source)[i];

		((u_short *)pCur->devPriv[pScr->myNum])[i+16] = 
			((u_int *)pCur->bits->mask)[i];
	}

	return (TRUE);
}

static Bool
pmaxUnrealizeCursor(pScr, pCurs)
	ScreenPtr pScr;
	CursorPtr pCurs;
{
	Xfree(pCurs->devPriv[pScr->myNum]);
	return (TRUE);
}


/*
 * pm cursor top-left corner can now go to negative coordinates
 */
static void
pmaxCursorLimits(pScr, pCurs, pHotBox, pPhysBox)
	ScreenPtr pScr;
	CursorPtr pCurs;
	BoxPtr  pHotBox;
	BoxPtr  pPhysBox;	/* return value */
{
        pmaxScreenPrivate *sp = (pmaxScreenPrivate *)
		pScr->devPrivates[pmaxScreenPrivateIndex].ptr;
	BoxPtr crange = &sp->cursorRange;

	pPhysBox->x1 = max(pHotBox->x1, crange->x1 + (int) pCurs->bits->xhot);
	pPhysBox->y1 = max(pHotBox->y1, crange->y1 + (int) pCurs->bits->yhot);
	pPhysBox->x2 = min(pHotBox->x2, crange->x2 + 1);
	pPhysBox->y2 = min(pHotBox->y2, crange->y2 + 1);
}

static void
pmaxPointerNonInterestBox(pScr, pBox)
	ScreenPtr pScr;
	BoxPtr  pBox;
{
	scrInfo->mbox.bottom = pBox->y2;
	scrInfo->mbox.top = pBox->y1;
	scrInfo->mbox.left = pBox->x1;
	scrInfo->mbox.right = pBox->x2;
}

static void
pmaxQueryBestSize(class, pwidth, pheight, pScreen)
	int     class;
	short  *pwidth;
	short  *pheight;
	ScreenPtr pScreen;
{
	if (class == CursorShape) {
		*pwidth = 16;
		*pheight = 16;
	} else
		mfbQueryBestSize(class, pwidth, pheight, pScreen);
}

void
SetLockLED(on)
	Bool    on;
{
	struct pm_kpcmd ioc;

	ioc.cmd = on ? LK_LED_ENABLE : LK_LED_DISABLE;
	ioc.par[0] = LED_3;
	ioc.par[1] = 0;
	ioc.nbytes = 1;
	ioctl(fdPM, QIOCKPCMD, &ioc);
}


/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void
AbortDDX()
{
}
/* Called by GiveUp(). */
void
ddxGiveUp()
{
}

int
ddxProcessArgument(argc, argv, i)
	int     argc;
	char   *argv[];
	int     i;
{
	int     argind = i;
	int     skip;
	static int Once = 0;
	void    ddxUseMsg();

	skip = 0;
	if (!Once)
		Once = 1;

	if (strcmp(argv[argind], "-dpix") == 0) {
		if (++argind < argc) {
			dpix = atoi(argv[argind]);
			skip = 2;
		} else
			return 0;	/* failed to parse */
	} else
		if (strcmp(argv[argind], "-dpiy") == 0) {
			if (++argind < argc) {
				dpiy = atoi(argv[argind]);
				skip = 2;
			} else
				return 0;
		} else
			if (strcmp(argv[argind], "-dpi") == 0) {
				if (++argind < argc) {
					dpi = atoi(argv[argind]);
					dpix = dpi;
					dpiy = dpi;
					skip = 2;
				} else
					return 0;
			} else
				if (strcmp(argv[argind], "-class") == 0) {
					if (++argind < argc) {
						class = ParseClass(argv[argind]);
					
					if (class == NoSuchClass) {
						ErrorF("Bad argument to -class");
						return 0;
					}
					
					skip = 2;
				} else
					return 0;
			}
	return skip;
}

void
ddxUseMsg()
{
	ErrorF("\n");
	ErrorF("Device Dependent Usage\n");
	ErrorF("\n");
	ErrorF("-dpix <n>          Dots per inch, x coordinate\n");
	ErrorF("-dpiy <n>          Dots per inch, y coordinate\n");
	ErrorF("-dpi <n>           Dots per inch, x and y coordinates\n");
	ErrorF("                   (overrides -dpix and -dpiy above)\n");
}


void
pmaxStoreColors(pmap, ndef, pdefs)
    ColormapPtr pmap;
    int         ndef;
    xColorItem  *pdefs;
{
    int		idef;
    xColorItem	directDefs[256];
    u_char	*p;
    ColorMap	map;

    if (pmap != pInstalledMap)
	return;

    if ((pmap->pVisual->class | DynamicClass) == DirectColor)
    {
	ndef = cfbExpandDirectColors (pmap, ndef, pdefs, directDefs);
	pdefs = directDefs;
    }

    for(idef = 0; idef < ndef; idef++)
    {
	map.index = pdefs[idef].pixel;
	map.Entry.red = pdefs[idef].red >> 8;
	map.Entry.green = pdefs[idef].green >> 8;
	map.Entry.blue = pdefs[idef].blue >> 8;
	ioctl(fdPM, QIOSETCMAP, &map);
    }
}

void
pmaxInstallColormap(pcmap)
	ColormapPtr	pcmap;
{
    int         entries;
    Pixel *     ppix;
    xrgb *      prgb;
    xColorItem *defs;
    int         i;

    if (pcmap == pInstalledMap)
        return;

    if ((pcmap->pVisual->class | DynamicClass) == DirectColor)
	entries = (pcmap->pVisual->redMask |
		   pcmap->pVisual->greenMask |
		   pcmap->pVisual->blueMask) + 1;
    else
	entries = pcmap->pVisual->ColormapEntries;

    ppix = (Pixel *)ALLOCATE_LOCAL(entries * sizeof(Pixel));
    prgb = (xrgb *)ALLOCATE_LOCAL(entries * sizeof(xrgb));
    defs = (xColorItem *)ALLOCATE_LOCAL(entries * sizeof(xColorItem));

    if (pInstalledMap != NOMAPYET)
        WalkTree(pcmap->pScreen, TellLostMap, &pInstalledMap->mid);
    pInstalledMap = pcmap;
    for ( i=0; i<entries; i++)
        ppix[i] = i;
    QueryColors(pcmap, entries, ppix, prgb);
    for ( i=0; i<entries; i++) /* convert xrgbs to xColorItems */
    {
        defs[i].pixel = ppix[i];
        defs[i].red = prgb[i].red;
        defs[i].green = prgb[i].green;
        defs[i].blue = prgb[i].blue;
        defs[i].flags =  DoRed|DoGreen|DoBlue;
    }
    pmaxStoreColors(pcmap, entries, defs);
    WalkTree(pcmap->pScreen, TellGainedMap, &pcmap->mid);

    DEALLOCATE_LOCAL(ppix);
    DEALLOCATE_LOCAL(prgb);
    DEALLOCATE_LOCAL(defs);
}


void
pmaxUninstallColormap(pcmap)
    ColormapPtr pcmap;
{
    /*  Replace installed colormap with default colormap */

    ColormapPtr defColormap;

    if (pcmap != pInstalledMap)
        return;

    defColormap = (ColormapPtr) LookupIDByType( pcmap->pScreen->defColormap,
                        RT_COLORMAP);

    if (defColormap == pInstalledMap)
        return;

    (*pcmap->pScreen->InstallColormap) (defColormap);
}

int
pmaxListInstalledColormaps( pscr, pcmaps)
    ScreenPtr   pscr;
    Colormap *  pcmaps;
{
    *pcmaps = pInstalledMap->mid;
    return 1;
}
