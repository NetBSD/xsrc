/*-
 * Copyright (C) 1994 Bradley A. Grantham and Lawrence A. Kesteloot
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the Alice Group or any of its members may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ALICE GROUP ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE ALICE GROUP BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <servermd.h>
#include "mac68k.h"
#include "dixstruct.h"
#include "dix.h"
#include "opaque.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>


#define CATCH_BROKEN_MODE 1


void ProcessInputEvents(void);


int mac_adbfd;

Time mac_lasteventtime;

int mac_scrs = 0;
fbinfo_t mac_fbs[MAXSCREENS];

DeviceIntPtr mac68k_mouse;
DeviceIntPtr mac68k_kbd;

int mac68k_mouseproc(
	DeviceIntPtr mouse,
	int what);

int mac68k_kbdproc(
	DeviceIntPtr mouse,
	int what);

Bool mac68kMonoInit(
	int	index,
	ScreenPtr	screen,
	int	argc,
	char	**argv);


void mac68k_enqevents(
	void);


static void handle_sigio(
	int flags)
{
	mac68k_enqevents();
}

void OsVendorPreInit(
#if NeedFunctionPrototypes
    void
#endif
)
{
}

#if MAC68K_IOHACK
void OsVendorInit(
#if NeedFunctionPrototypes
    void
#endif
)
{
}

int initialize_desktop()
#else

void OsVendorInit(
#if NeedFunctionPrototypes
    void
#endif
)

#endif
{
	int				i, j;
	struct imagedata		*id;
	int				totalscreens;
	int				argnum = 1;

	mac_adbfd = open ("/dev/adb", O_RDWR);

	if (mac_adbfd == -1) {
		ErrorF("Couldn't open /dev/adb...");
		FatalError(sys_errlist[errno]);
	}

	mac68k_getmouse();	/* find mouse and set up or error */
	mac68k_getkbd();	/* find keyboard and set up or error */

	signal(SIGIO, handle_sigio);
}

void
choose_best_depths(void)
{
#if 0
	int scr, mode, best;
	struct imagedata *si;
	int umph, bestumph;	/* "umph" is goodness of screen */
		/* Could have a nicer calculation, like depth versus */
		/* resolution versus dimensions with weights... */
	char scrstr[100];

	for(scr = 0; scr < mac_ci.numscreens; scr++){
		best = -1;
		if(mac_si[scr].nummodes > 0){
			best = 0;
			si = &mac_si[scr].modes[0].modeinfo;
			bestumph = si->pixelSize * (si->right - si->left) *
				(si->bottom - si->top);
	
			for(mode = 1; mode < mac_si[scr].nummodes; mode++){
				si = &mac_si[scr].modes[mode].modeinfo;
				umph = si->pixelSize * (si->right - si->left) *
					(si->bottom - si->top);
				if(umph > bestumph){
					bestumph = umph;
					best = mode;
				}
			}
		}else{
			sprintf(scrstr, "Cannot find modes for screen %d."
				"  Ignoring.", scr);
			ErrorF(scrstr);
		}
		mac_bestmode[scr] = best;
	}
#endif
}


Bool mac68k_screeninit(
	ScreenPtr screen)
{
	miDCInitialize(screen, &mac_mousefuncs);

	return TRUE;
}



void
parse_args(
	int argc,
	char **argv)
{
	/* try to match argv params to screens and depths */
	/* BARF I really should do this, but not right now, */
	/* while I am feverishly trying to finish the other X code. */
}


void
setup_screens(
	ScreenInfo *xsi,
	int argc,
	char **argv)
{
	char	fname[128];
	int	done = 0;
	caddr_t vaddr;
	int	scr;
	
	do {
		sprintf(fname, "/dev/grf%d", mac_scrs);
#if DEBUG
		ErrorF("attempting %s...\n", fname);
#endif
		if ((mac_fbs[mac_scrs].fd = open(fname, O_RDWR)) != -1) {
#if DEBUG
			ErrorF("opened successfully\n");
#endif
			if (ioctl (mac_fbs[mac_scrs].fd, GRFIOCGMODE,
				&mac_fbs[mac_scrs].idata) != 0)
			{
				ErrorF("Couldn't get info for %d.\n",mac_scrs);
				FatalError(sys_errlist[errno]);
			}
			vaddr = mmap(0, mac_fbs[mac_scrs].idata.fbsize,
				(PROT_READ|PROT_WRITE),MAP_SHARED,
				mac_fbs[mac_scrs].fd,
				mac_fbs[mac_scrs].idata.fboff);
			if (vaddr == (caddr_t) -1) {
				ErrorF("Couldn't map SCREEN %d.\n", mac_scrs);
				FatalError(sys_errlist[errno]);
			}
			mac_fbs[mac_scrs].modenum = 0;
			mac_fbs[mac_scrs].vaddr = vaddr;
			mac_fbs[mac_scrs].added = FALSE;
			mac_scrs++;
		} else {
			if (errno != ENXIO) {
				ErrorF("Couldn't open %s.\n", fname);
				FatalError(sys_errlist[errno]);
			}
			done = 1;
		}
	} while(!done);

	if(mac_scrs == 0)
		FatalError("Can't run X server with no screens!");

	xsi->imageByteOrder = IMAGE_BYTE_ORDER;
	xsi->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
	xsi->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
	xsi->bitmapBitOrder = BITMAP_BIT_ORDER;

	xsi->formats[0].depth = 1;
	xsi->formats[0].bitsPerPixel = 1;
	xsi->formats[0].scanlinePad = BITMAP_SCANLINE_PAD;

	xsi->formats[1].depth = 8;
	xsi->formats[1].bitsPerPixel = 8;
	xsi->formats[1].scanlinePad = BITMAP_SCANLINE_PAD;

	xsi->formats[2].depth = 16;
	xsi->formats[2].bitsPerPixel = 16;
	xsi->formats[2].scanlinePad = BITMAP_SCANLINE_PAD;

	xsi->formats[3].depth = 24;
	xsi->formats[3].bitsPerPixel = 32;
	xsi->formats[3].scanlinePad = BITMAP_SCANLINE_PAD;

	xsi->numPixmapFormats = 4;

#if DEBUG
	ErrorF("# of screens: %d\n", mac_scrs);
#endif
	for(scr = 0; scr < mac_scrs; scr++){
#if DEBUG
		ErrorF("adding screen %d\n", scr);
#endif
#if CATCH_BROKEN_MODE
		if(mac_fbs[scr].idata.psize == 0)
			mac_fbs[scr].idata.psize = 1;
#endif /* CATCH_BROKEN_MODE */
		if(mac_fbs[scr].idata.psize == 1)
			AddScreen(mac68kMonoInit, argc, argv);
		else
#if COLOR_SUPPORT	/* Damn, I'm lazy! */
			AddScreen(mac68kColorInit, argc, argv);
#else
			FatalError("I don't support color screens!\n");
#endif
	}

	xsi->numScreens = scr;
}


void InitOutput(
	ScreenInfo *xsi,
	int argc,
	char **argv)
{
	static int dejavu = 0;

	if(dejavu)
		return;
	else
		dejavu = 1;

	/* set up all screens */
#if MAC68K_IOHACK
	initialize_desktop();
#endif
	choose_best_depths();
	parse_args(argc, argv);
	setup_screens(xsi, argc, argv);
}


void InitInput(
	int argc,
	char **argv)
{
	DeviceIntPtr mouse;
	DeviceIntPtr kbd;
	static int dejavu = 0;

	if(dejavu)
		return;
	else
		dejavu = 1;

	mac68k_mouse = mouse = AddInputDevice((DeviceProc)mac68k_mouseproc,
		TRUE);
	RegisterPointerDevice(mouse);
	miRegisterPointerDevice(screenInfo.screens[0], mouse);

	mac68k_kbd = kbd = AddInputDevice((DeviceProc)mac68k_kbdproc, TRUE);
	RegisterKeyboardDevice(kbd);

	if (!mieqInit ((DevicePtr)kbd, (DevicePtr)mouse))
		FatalError ("could not initialize event queue");

	mac_lasteventtime = time(0) * 1000;
}


void AbortDDX(void)
{
	close(mac_adbfd);
}


void ddxGiveUp(void)
{
}


int ddxProcessArgument(
	int argc,
	char **argv,
	int i)
{
	if(strcmp(argv[i], "-screen") == 0)
		return(3);
	return(0);
}


void ddxUseMsg(void)
{
	ErrorF("-screen S D\t\t\tuse depth <D> for screen <S>");
	ErrorF("\t(use the 'console' program for screen information)");
}


void MessageF(
	char *s)
{
	ErrorF(s);
}

void OsVendorFatalError(void)
{
}

#ifdef DPMSExtension
/*
 * DPMS stubs
 */
void DPMSSet(int level)
{
}

int DPMSGet(int level)
{
	return -1;
}

Bool DPMSSupported()
{
	return FALSE;
}
#endif /* DPMSExtension */
