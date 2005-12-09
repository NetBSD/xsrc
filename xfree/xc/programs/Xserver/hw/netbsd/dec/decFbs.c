/*	$NetBSD: decFbs.c,v 1.2 2005/12/09 05:44:50 mhitch Exp $	*/

/* XConsortium: sunFbs.c,v 1.8 94/08/16 13:45:30 dpw Exp */

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

#include "dec.h"
#include <sys/mman.h>
#include <stdio.h>

int decScreenIndex;

static unsigned long generation = 0;

#if NeedFunctionPrototypes
pointer decMemoryMap (
    size_t	len,
    off_t	off,
    int		fd)
#else
pointer decMemoryMap (len, off, fd)
    size_t	len;
    off_t	off;
    int		fd;
#endif
{
    int		pagemask, mapsize;
    caddr_t	addr;
    pointer	mapaddr;
    int		mode = WSDISPLAYIO_MODE_MAPPED;

    pagemask = getpagesize() - 1;
    mapsize = ((int) len + pagemask) & ~pagemask;
    addr = 0;

    if (ioctl(fd, WSDISPLAYIO_SMODE, &mode) == -1)
	Error("WSDISPLAYIO_SMODE");

    mapaddr = (pointer) mmap (addr, mapsize,
		    PROT_READ | PROT_WRITE, MAP_SHARED,
		    fd, off);

    if (mapaddr == (pointer) -1L) {
	Error ("mapping frame buffer memory");
	(void) close (fd);
	mapaddr = (pointer) NULL;
    }

    return mapaddr;
}

#if NeedFunctionPrototypes
Bool decScreenAllocate (
    ScreenPtr	pScreen)
#else
Bool decScreenAllocate (pScreen)
    ScreenPtr	pScreen;
#endif
{
    decScreenPtr    pPrivate;
    extern int AllocateScreenPrivateIndex();

    if (generation != serverGeneration)
    {
	decScreenIndex = AllocateScreenPrivateIndex();
	if (decScreenIndex < 0)
	    return FALSE;
	generation = serverGeneration;
    }
    pPrivate = (decScreenPtr) xalloc (sizeof (decScreenRec));
    if (!pPrivate)
	return FALSE;

    pScreen->devPrivates[decScreenIndex].ptr = (pointer) pPrivate;
    return TRUE;
}

#if NeedFunctionPrototypes
Bool decSaveScreen (
    ScreenPtr	pScreen,
    int		on)
#else
Bool decSaveScreen (pScreen, on)
    ScreenPtr	pScreen;
    int		on;
#endif
{
    int		state;

    if (on != SCREEN_SAVER_FORCER)
    {
	if (on == SCREEN_SAVER_ON || on == SCREEN_SAVER_CYCLE)
	    state = 0;
	else
	    state = 1;
	(void) ioctl(decFbs[pScreen->myNum].fd, WSDISPLAYIO_SVIDEO, &state);
    }
    return( TRUE );
}

Bool
decCloseScreen (i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    SetupScreen(pScreen);
    Bool    ret;
    int mode = WSDISPLAYIO_MODE_EMUL;

    (void) OsSignal (SIGIO, SIG_IGN);
    decDisableCursor (pScreen);
    pScreen->CloseScreen = pPrivate->CloseScreen;
    ret = (*pScreen->CloseScreen) (i, pScreen);
    (void) (*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);
    ioctl(decFbs[pScreen->myNum].fd, WSDISPLAYIO_SMODE, &mode);
    xfree ((pointer) pPrivate);
    return ret;
}

#if NeedFunctionPrototypes
Bool decScreenInit (
    ScreenPtr	pScreen)
#else
Bool decScreenInit (pScreen)
    ScreenPtr	pScreen;
#endif
{
    SetupScreen(pScreen);
    extern void   decBlockHandler();
    extern void   decWakeupHandler();
    static ScreenPtr autoRepeatScreen;
    extern miPointerScreenFuncRec   decPointerScreenFuncs;

    pPrivate->installedMap = 0;
    pPrivate->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = decCloseScreen;
    pScreen->SaveScreen = decSaveScreen;
#ifdef XKB
    if (noXkbExtension) {
#endif
#if 0 /* XXX */
    /*
     *	Block/Unblock handlers
     */
    if (decAutoRepeatHandlersInstalled == FALSE) {
	autoRepeatScreen = pScreen;
	decAutoRepeatHandlersInstalled = TRUE;
    }

    if (pScreen == autoRepeatScreen) {
        pScreen->BlockHandler = decBlockHandler;
        pScreen->WakeupHandler = decWakeupHandler;
    }
#endif /* 0 XXX */
#ifdef XKB
    }
#endif
    if (((decSoftCursor || decFbs[pScreen->myNum].fbData->softCursor) &&
        !decHardCursor) || !decCursorInitialize (pScreen))
	miDCInitialize (pScreen, &decPointerScreenFuncs);
    return TRUE;
}
