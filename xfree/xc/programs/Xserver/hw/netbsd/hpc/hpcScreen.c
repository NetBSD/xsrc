/* $NetBSD: hpcScreen.c,v 1.2 2005/12/09 05:44:50 mhitch Exp $	*/
/* $XConsortium: sunFbs.c,v 1.8 94/08/16 13:45:30 dpw Exp $ */
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
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
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
/* Modified from  sunCG4C.c for X11R3 by Tom Jarmolowski	*/
/****************************************************************/

#include "hpc.h"
#include <sys/mman.h>
#include <stdio.h>

static int hpcScreenIndex;
static unsigned long generation = 0;

Bool
hpcAllocateScreenPrivate(pScreen)
    ScreenPtr	pScreen;
{
    hpcScreenPtr    pPrivate;
    extern int AllocateScreenPrivateIndex();

    if (generation != serverGeneration) {
	hpcScreenIndex = AllocateScreenPrivateIndex();
	if (hpcScreenIndex < 0)
	    return FALSE;
	generation = serverGeneration;
    }
    pPrivate = (hpcScreenPtr)xalloc(sizeof(hpcScreenRec));
    if (!pPrivate)
	return FALSE;

    pScreen->devPrivates[hpcScreenIndex].ptr = (pointer) pPrivate;
    return TRUE;
}

hpcScreenPtr
hpcGetScreenPrivate(pScreen)
    ScreenPtr	pScreen;
{
    return ((hpcScreenPtr)((pScreen)->devPrivates[hpcScreenIndex].ptr));
}

pointer
hpcMemoryMap (len, off, fd)
    size_t	len;
    off_t	off;
    int		fd;
{
    caddr_t	addr;
    pointer	mapaddr;

    len += off;
    addr = 0;

    /*
     * try and make it private first, that way once we get it, an
     * interloper, e.g. another server, can't get this frame buffer,
     * and if another server already has it, this one won't.
     */
    mapaddr = (pointer) mmap(addr, len,
		    PROT_READ | PROT_WRITE, MAP_SHARED,
		    fd, 0);
    if (mapaddr == (pointer) -1) {
	hpcError ("mapping frame buffer memory");
	(void) close (fd);
	return NULL;
    }
    return mapaddr + off;
}

static Bool
hpcSaveScreen (pScreen, on)
    ScreenPtr	pScreen;
    int		on;
{
    int state;
    hpcFbPtr pFb = hpcGetScreenFb(pScreen);

    if (on != SCREEN_SAVER_FORCER)
    {
	if (on == SCREEN_SAVER_ON || on == SCREEN_SAVER_CYCLE)
	    state = WSDISPLAYIO_VIDEO_OFF;
	else
	    state = WSDISPLAYIO_VIDEO_ON;
	ioctl(pFb->fd, WSDISPLAYIO_SVIDEO, &state);
    }

    return TRUE;
}

static Bool
hpcCloseScreen (i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    Bool    ret;
    hpcScreenPtr pPrivate = hpcGetScreenPrivate(pScreen);
    hpcFbPtr pFb = hpcGetScreenFb(pScreen);
    int mode = WSDISPLAYIO_MODE_EMUL;

    (void) OsSignal (SIGIO, SIG_IGN);
    pScreen->CloseScreen = pPrivate->CloseScreen;
    ret = (*pScreen->CloseScreen) (i, pScreen);
    (void) (*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);
    ioctl(pFb->fd, WSDISPLAYIO_SMODE, &mode);
    xfree ((pointer) pPrivate);
    return ret;
}

Bool
hpcScreenInit (pScreen)
    ScreenPtr	pScreen;
{
    hpcScreenPtr pPrivate;
    extern void   hpcBlockHandler();
    extern void   hpcWakeupHandler();
    static ScreenPtr autoRepeatScreen;
    extern miPointerScreenFuncRec hpcPointerScreenFuncs;

    pPrivate = hpcGetScreenPrivate(pScreen);
    pPrivate->installedMap = 0;
    pPrivate->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = hpcCloseScreen;
    pScreen->SaveScreen = hpcSaveScreen;
#ifdef XKB
    if (noXkbExtension) {
#endif
#if 0 /* XXX */
    /*
     *	Block/Unblock handlers
     */
    if (hpcAutoRepeatHandlersInstalled == FALSE) {
	autoRepeatScreen = pScreen;
	hpcAutoRepeatHandlersInstalled = TRUE;
    }

    if (pScreen == autoRepeatScreen) {
        pScreen->BlockHandler = hpcBlockHandler;
        pScreen->WakeupHandler = hpcWakeupHandler;
    }
#endif /* 0 XXX */
#ifdef XKB
    }
#endif
    miDCInitialize (pScreen, &hpcPointerScreenFuncs);
    return TRUE;
}
