
/* $XConsortium: amigaFbs.c,v 1.7 94/04/17 20:29:37 dpw Exp $ */

/*
Copyright (c) 1990, 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL AMIGA BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/*
 * Copyright (c) 1987 by the Regents of the University of California
 * Copyright (c) 1987 by Adam de Boor, UC Berkeley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/****************************************************************/
/* Modified from  amigaCG4C.c for X11R3 by Tom Jarmolowski	*/
/****************************************************************/

#include "amiga.h"
#include <sys/mman.h>

int amigaScreenIndex;

static unsigned long generation = 0;

pointer amigaMemoryMap (
    size_t	len,
    off_t	off,
    int		fd)
{
    int		pagemask, mapsize;
    caddr_t	addr;
    pointer	mapaddr;

    pagemask = getpagesize() - 1;
    mapsize = ((int) len + pagemask) & ~pagemask;
    addr = 0;

    mapaddr = (pointer) mmap (addr, mapsize,
	PROT_READ | PROT_WRITE, MAP_FILE|MAP_SHARED,
	fd, off);
    if (mapaddr == (pointer) -1) {
	Error ("mapping frame buffer memory");
	(void) close (fd);
	mapaddr = (pointer) NULL;
    }
    return mapaddr;
}

Bool amigaScreenAllocate (
    ScreenPtr	pScreen)
{
    amigaScreenPtr    pPrivate;
    extern int AllocateScreenPrivateIndex();

    if (generation != serverGeneration)
    {
	amigaScreenIndex = AllocateScreenPrivateIndex();
	if (amigaScreenIndex < 0)
	    return FALSE;
	generation = serverGeneration;
    }
    pPrivate = (amigaScreenPtr) xalloc (sizeof (amigaScreenRec));
    if (!pPrivate)
	return FALSE;

    pScreen->devPrivates[amigaScreenIndex].ptr = (pointer) pPrivate;
    return TRUE;
}

Bool amigaSaveScreen (
    ScreenPtr	pScreen,
    int		on)
{
    int		state;

    if (on != SCREEN_SAVER_FORCER)
    {
	if (on == SCREEN_SAVER_ON || on == SCREEN_SAVER_CYCLE)
	    state = 0;
	else
	    state = 1;
	(void) ioctl(amigaFbs[pScreen->myNum].fd, GRFIOCBLANK, &state);
    }
    return( TRUE );
}

static Bool closeScreen (i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    SetupScreen(pScreen);
    Bool    ret;

#ifdef GFX_CARD_SUPPORT
    amigaDisableCursor (pScreen);
#endif
    pScreen->CloseScreen = pPrivate->CloseScreen;
    ret = (*pScreen->CloseScreen) (i, pScreen);
    (void) (*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);
    xfree ((pointer) pPrivate);
    return ret;
}

Bool amigaScreenInit (
    ScreenPtr	pScreen)
{
    SetupScreen(pScreen);
    static ScreenPtr autoRepeatScreen;
    extern miPointerScreenFuncRec   amigaPointerScreenFuncs;

    pPrivate->installedMap = 0;
    pPrivate->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = closeScreen;
    pScreen->SaveScreen = amigaSaveScreen;
#ifdef XKB
    if (noXkbExtension) {
#endif
	/*
	 *	Block/Unblock handlers
	 */
	if (amigaAutoRepeatHandlersInstalled == FALSE) {
	    autoRepeatScreen = pScreen;
	    amigaAutoRepeatHandlersInstalled = TRUE;
	}

	if (pScreen == autoRepeatScreen) {
	    pScreen->BlockHandler = amigaBlockHandler;
	    pScreen->WakeupHandler = amigaWakeupHandler;
	}
#ifdef XKB
    }
#endif

#ifndef GFX_CARD_SUPPORT
    miDCInitialize (pScreen, &amigaPointerScreenFuncs);
#else /* GFX_CARD_SUPPORT */
    if (UseGfxHardwareCursor) {
	if (!amigaCursorInitialize (pScreen))
	    miDCInitialize (pScreen, &amigaPointerScreenFuncs);
    } else {
#ifdef CV64_SUPPORT
	if (!UseCVHardwareCursor)
#endif /* CV64_SUPPORT */
	    miDCInitialize (pScreen, &amigaPointerScreenFuncs);
    }
#endif /* GFX_CARD_SUPPORT */

    return TRUE;
}

Bool amigaInitCommon (
    int		scrn,
    ScreenPtr	pScrn,
    Bool	(*init1)(),
    void	(*init2)(),
    Bool	(*cr_cm)(),
    Bool	(*save)(),
    int		fb_off)
{
    unsigned char*	fb = amigaFbs[scrn].fb;

    if (!amigaScreenAllocate (pScrn))
	return FALSE;
    if (!fb) {
	if ((fb = amigaMemoryMap ((size_t) amigaFbs[scrn].info.gd_fbsize,
			     amigaFbs[scrn].info.gd_regsize,
			     amigaFbs[scrn].fd)) == NULL)
	    return FALSE;
	amigaFbs[scrn].fb = fb;
    }
    /* mfbScreenInit() or cfbScreenInit() */
    if (!(*init1)(pScrn, fb + fb_off,
	    amigaFbs[scrn].info.gd_fbwidth,
	    amigaFbs[scrn].info.gd_fbheight,
	    monitorResolution, monitorResolution,
	    amigaFbs[scrn].info.gd_fbwidth,
	    amigaFbs[scrn].info.gd_planes))
	    return FALSE;
    /* amigaCGScreenInit() if cfb... */
    if (init2)
	(*init2)(pScrn);
    if (!amigaScreenInit(pScrn))
	return FALSE;
    (void) (*save) (pScrn, SCREEN_SAVER_OFF);
    return cr_cm ? (*cr_cm)(pScrn) : 1;
}

