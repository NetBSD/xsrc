
/* $Xorg: sunFbs.c,v 1.4 2001/02/09 02:04:43 xorgcvs Exp $ */

/*
Copyright 1990, 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or The Open Group
not be used in advertising or publicity pertaining to
distribution  of  the software  without specific prior
written permission. Sun and The Open Group make no
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
 * Copyright 1987 by the Regents of the University of California
 * Copyright 1987 by Adam de Boor, UC Berkeley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/* $XFree86: xc/programs/Xserver/hw/sun/sunFbs.c,v 1.8 2003/11/17 22:20:36 dawes Exp $ */

/****************************************************************/
/* Modified from  sunCG4C.c for X11R3 by Tom Jarmolowski	*/
/****************************************************************/

#include "sun.h"
#include <sys/mman.h>

static Bool closeScreen(ScreenPtr pScreen);

int sunScreenIndex;

DevPrivateKeyRec sunScreenPrivateKeyRec;

void *
sunMemoryMap(size_t len, off_t off, int fd)
{
    int		pagemask, mapsize;
    caddr_t	addr;
    void	*mapaddr;

#ifdef SVR4
    pagemask = sysconf(_SC_PAGESIZE) - 1;
#else
    pagemask = getpagesize() - 1;
#endif
    mapsize = ((int) len + pagemask) & ~pagemask;
    addr = 0;

#if !defined(__bsdi__) && !defined(_MAP_NEW) && !defined(__NetBSD__) && !defined(__OpenBSD__)
    if ((addr = (caddr_t) valloc (mapsize)) == NULL) {
	ErrorF("Couldn't allocate frame buffer memory\n");
	(void) close (fd);
	return NULL;
    }
#endif

#if !defined(__NetBSD__) && !defined(__OpenBSD__)
    /*
     * try and make it private first, that way once we get it, an
     * interloper, e.g. another server, can't get this frame buffer,
     * and if another server already has it, this one won't.
     */
    if ((int)(mapaddr = (void *) mmap (addr,
		mapsize,
		PROT_READ | PROT_WRITE, MAP_PRIVATE,
		fd, off)) == -1)
#endif
	mapaddr = mmap (addr,
		    mapsize,
		    PROT_READ | PROT_WRITE, MAP_SHARED,
		    fd, off);
    if (mapaddr == (void *) -1) {
	ErrorF("mapping frame buffer memory\n");
	(void) close (fd);
	mapaddr = NULL;
    }
    return mapaddr;
}

Bool
sunScreenAllocate(ScreenPtr pScreen)
{
    sunScreenPtr    pPrivate;

    if (!dixRegisterPrivateKey(&sunScreenPrivateKeyRec, PRIVATE_SCREEN, 0)) {
	ErrorF("dixRegisterPrivateKey failed\n");
	return FALSE;
    }
    pPrivate = calloc(1, sizeof (sunScreenRec));
    if (!pPrivate)
	return FALSE;

    pPrivate->origColormapValid = FALSE;
    sunSetScreenPrivate(pScreen, pPrivate);
    return TRUE;
}

Bool
sunSaveScreen(ScreenPtr pScreen, int on)
{
    int		state;

    if (on != SCREEN_SAVER_FORCER)
    {
	if (on == SCREEN_SAVER_ON)
	    state = 0;
	else
	    state = 1;
	(void) ioctl(sunFbs[pScreen->myNum].fd, FBIOSVIDEO, &state);
    }
    return( TRUE );
}

static Bool
closeScreen(ScreenPtr pScreen)
{
    sunScreenPtr pPrivate = sunGetScreenPrivate(pScreen);
    Bool    ret;

    (void) OsSignal (SIGIO, SIG_IGN);
#if 0	/* XXX GX is disabled for now */
    sunDisableCursor (pScreen);
#endif
    if (pPrivate->origColormapValid)
	(*pPrivate->RestoreColormap)(pScreen);
    pScreen->CloseScreen = pPrivate->CloseScreen;
    ret = (*pScreen->CloseScreen) (pScreen);
    (void) (*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);
    free (pPrivate);
    return ret;
}

Bool
sunScreenInit(ScreenPtr pScreen)
{
    sunScreenPtr pPrivate = sunGetScreenPrivate(pScreen);

    pPrivate->installedMap = 0;
    pPrivate->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = closeScreen;
    pScreen->SaveScreen = sunSaveScreen;
#if 0	/* XXX GX is disabled for now */
    if (!sunCursorInitialize (pScreen))
#endif
	miDCInitialize (pScreen, &sunPointerScreenFuncs);
    return TRUE;
}

Bool
sunInitCommon(
    int		scrn,
    ScreenPtr	pScrn,
    off_t	offset,
    Bool	(*init1)(ScreenPtr, void *, int, int, int, int, int, int),
    void	(*init2)(ScreenPtr),
    Bool	(*cr_cm)(ScreenPtr),
    Bool	(*save)(ScreenPtr, int),
    int		fb_off
)
{
    unsigned char*	fb = sunFbs[scrn].fb;

    if (!sunScreenAllocate (pScrn))
	return FALSE;
    if (!fb) {
	if ((fb = sunMemoryMap ((size_t) sunFbs[scrn].info.fb_size,
			     offset,
			     sunFbs[scrn].fd)) == NULL)
	    return FALSE;
	sunFbs[scrn].fb = fb;
    }
    /* mfbScreenInit() or cfbScreenInit() */
    if (!(*init1)(pScrn, fb + fb_off,
	    sunFbs[scrn].info.fb_width,
	    sunFbs[scrn].info.fb_height,
	    monitorResolution, monitorResolution,
	    sunFbs[scrn].info.fb_width,
	    sunFbs[scrn].info.fb_depth))
	    return FALSE;
    /* sunCGScreenInit() if cfb... */
    if (init2)
	(*init2)(pScrn);
    if (!sunScreenInit(pScrn))
	return FALSE;
    (void) (*save) (pScrn, SCREEN_SAVER_OFF);
    return (*cr_cm)(pScrn);
}

