/*	$NetBSD: decPX.c,v 1.4 2011/05/24 23:12:36 jakllsch Exp $	*/

/*-
 * Copyright (c) 2001, 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andrew Doran.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

/* from XConsortium: cfbwindow.c,v 5.22 94/04/17 20:29:07 dpw Exp */
/***********************************************************

Copyright (c) 1987  X Consortium

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
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

#include "px.h"

#include "cfb.h"
#include "cfb32.h"
#include "mi.h"

#include <sys/mman.h>
#include <sys/ioctl.h>

#include "../dec.h"

/*
 * XXX
 */
#ifdef cfbSetupScreen
#undef cfbSetupScreen
#undef cfbScreenInit
#undef cfbFinishScreenInit
#undef cfbCreateScreenResources
#endif

/*
 * XXX
 */
extern Bool cfbSetupScreen(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    pointer /*pbits*/,
    int /*xsize*/,
    int /*ysize*/,
    int /*dpix*/,
    int /*dpiy*/,
    int /*width*/
#endif
);

/*
 * XXX
 */
extern Bool cfbFinishScreenInit(
#if NeedFunctionPrototypes
		ScreenPtr /*pScreen*/,
		pointer /*pbits*/,
		int /*xsize*/,
		int /*ysize*/,
		int /*dpix*/,
		int /*dpiy*/,
		int /*width*/
#endif
);

Bool	cfbCreateScreenResources(ScreenPtr);
Bool	cfb32CreateScreenResources(ScreenPtr);

Bool	pxCloseScreen(int, ScreenPtr);

static Bool	pxScreenInit(ScreenPtr pScreen, pointer, int, int, int, int,
			     int, int);
static Bool	pxSetupScreen(ScreenPtr pScreen, pointer, int, int, int, int,
			     int, int);
static Bool	pxFinishScreenInit(ScreenPtr pScreen, pointer, int, int, int,
			     int, int, int);

int	pxGCPrivateIndex;
int	pxWindowPrivateIndex;
int	pxScreenPrivateIndex;

#if 0
extern int defaultColorVisualClass;
#endif

static Bool
pxSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int	bpp;			/* bits per pixel of root */
{
    switch (bpp) {
    case 32:
	if (!cfb32SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy,
	  width))
	    return FALSE;
	if (!cfbSetVisualTypes(24, 1 << TrueColor, 8))
	    return FALSE;
	break;
    case 8:
	if (!cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy,
	  width))
	    return FALSE;
	break;
    default:
	ErrorF("pxSetupScreen:  unsupported bpp = %d\n", bpp);
	return FALSE;
    }
   
    pScreen->CreateGC = pxCreateGC;
    pScreen->CreateWindow = pxCreateWindow;
    pScreen->DestroyWindow = pxDestroyWindow;
    pScreen->PositionWindow = pxPositionWindow;
    pScreen->ChangeWindowAttributes = pxChangeWindowAttributes;
    pScreen->PaintWindowBackground = pxPaintWindowBackground;
    pScreen->PaintWindowBorder = pxPaintWindowBorder;
    pScreen->CopyWindow = pxCopyWindow;
    pScreen->GetSpans = pxGetSpans;
    pScreen->GetImage = pxGetImage;

    return (TRUE);
}

static Bool
pxFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int bpp;			/* bits per pixel of root */
{
#if 0
#ifdef CFB_NEED_SCREEN_PRIVATE
    pointer oldDevPrivate;
#endif
    VisualPtr	visuals;
    DepthPtr	depths;
    int		nvisuals;
    int		ndepths;
    int		rootdepth;
    VisualID	defaultVisual;
    extern Bool cfbCreateScreenResources(ScreenPtr);

    rootdepth = 0;
    if (!cfbInitVisuals (&visuals, &depths, &nvisuals, &ndepths, &rootdepth,
			 &defaultVisual,((unsigned long)1<<(bpp-1)), 8))
	return FALSE;
#ifdef CFB_NEED_SCREEN_PRIVATE
    oldDevPrivate = pScreen->devPrivate;
#endif
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals,
			(miBSFuncPtr) 0))
	return FALSE;
    /* overwrite miCloseScreen with our own */
    pScreen->CloseScreen = cfbCloseScreen;
#ifdef CFB_NEED_SCREEN_PRIVATE
    if (bpp == 32)
        pScreen->CreateScreenResources = cfb32CreateScreenResources;
    else
        pScreen->CreateScreenResources = cfbCreateScreenResources;
    pScreen->devPrivates[cfbScreenPrivateIndex].ptr = pScreen->devPrivate;
    pScreen->devPrivate = oldDevPrivate;
#endif

    if (bpp == 32) {
	/* XXXNJW cfb doesn't provide a way to tweak these, so cheat. */
	pScreen->visuals[0].redMask = 0xff0000;
	pScreen->visuals[0].greenMask = 0xff00;
	pScreen->visuals[0].blueMask = 0xff;

	pScreen->visuals[0].offsetRed = 16;
	pScreen->visuals[0].offsetGreen = 8;
	pScreen->visuals[0].offsetBlue = 0;
    }

    return TRUE;
#endif
    Bool retval;

    switch (bpp) {
    case 32:
	retval = cfb32FinishScreenInit(pScreen, pbits, xsize, ysize,
		    dpix, dpiy, width);

	/* XXXNJW cfb doesn't provide a way to tweak these, so cheat. */
	pScreen->visuals[0].redMask = 0xff0000;
	pScreen->visuals[0].greenMask = 0xff00;
	pScreen->visuals[0].blueMask = 0xff;

	pScreen->visuals[0].offsetRed = 16;
	pScreen->visuals[0].offsetGreen = 8;
	pScreen->visuals[0].offsetBlue = 0;

	break;
    case 8:
	retval = cfbFinishScreenInit(pScreen, pbits, xsize, ysize,
		    dpix, dpiy, width);
	break;
    default:
	retval = FALSE;
    }

    return retval;

}

static Bool
pxScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp)
    ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int bpp;			/* bits per pixel of root */
{

    if (!pxSetupScreen(pScreen, pbits, xsize, ysize, dpix,
	dpiy, width, bpp))
	    return FALSE;

    return pxFinishScreenInit(pScreen, pbits, xsize, ysize, dpix,
	  dpiy, width, bpp);
}

Bool
pxCloseScreen(int i, ScreenPtr pScreen)
{
	pxScreenPrivPtr sp;

	sp = pScreen->devPrivates[pxScreenPrivateIndex].ptr;

	if (sp != NULL && sp->sxc != NULL) {
		munmap((void *)sp->sxc, sp->maplen);
		close(sp->mapfd);
		sp->sxc = NULL;
	}

	return ((sp->CloseScreen)(i, pScreen));
}

int
decPXInit(int screen, ScreenPtr pScreen, int argc, char **argv)
{
	pxScreenPrivPtr sp;
	static int generation;
	struct stic_xmap *sxm;
	struct stic_xinfo sxi;
	int bpp, realbpp, mode;
	char buf[MAXPATHLEN];
	size_t sz;
	fbFd *pFb;

	pFb = &decFbs[screen];
	realbpp = pFb->depth > 8 ? 32 : 8;

	if (ioctl(pFb->fd, STICIO_RESET)) {
		ErrorF("decPXInit: ioctl(STICIO_RESET) failed\n");
		return (FALSE);
	}

	if (ioctl(pFb->fd, STICIO_GXINFO, &sxi)) {
		ErrorF("decPXInit: ioctl(STICIO_GXINFO) failed\n");
		return (FALSE);
	}

	if (sxi.sxi_unit < 0) {
		ErrorF("decPXInit: no auxiallary control device for STIC\n");
		return (FALSE);
	}

	pFb->EnterLeave = (void (*)())NoopDDA;

    	if (!decScreenAllocate(pScreen))
		return (FALSE);

	switch (decWantedDepth) {
	case 8:
		bpp = 8;
		break;
	case 32:
		if (realbpp == 32)
			bpp = 32;
		else {
			ErrorF("pxFBInit: 32-bit color not available\n");
			bpp = realbpp;
		}
		break;
	default:
		bpp = realbpp;
		break;
	}

	if (!pxScreenInit(pScreen, NULL, pFb->width, pFb->height,
	    monitorResolution, monitorResolution, pFb->width, bpp))
		return (FALSE);

	pScreen->backingStoreSupport = NotUseful;
	pScreen->saveUnderSupport = NotUseful;

	if (generation != serverGeneration) {
		pxGCPrivateIndex = AllocateGCPrivateIndex();
		pxWindowPrivateIndex = AllocateWindowPrivateIndex();
		pxScreenPrivateIndex = AllocateScreenPrivateIndex();

		if (pxGCPrivateIndex < 0 ||
		    pxWindowPrivateIndex < 0 ||
		    pxScreenPrivateIndex < 0)
			return (FALSE);

		generation = serverGeneration;
		sp = (pxScreenPrivPtr)xalloc(sizeof(*sp));
		if (!sp)
			return (FALSE);
		memset(sp, 0, sizeof(*sp));

		pScreen->devPrivates[pxScreenPrivateIndex].ptr =
		    (pointer)sp;

	 	if (!AllocateGCPrivate(pScreen, pxGCPrivateIndex,
		    sizeof(pxPrivGCRec)) ||
		    !AllocateWindowPrivate(pScreen, pxWindowPrivateIndex,
		    sizeof(pxPrivWinRec)))
			return FALSE;
	}

	sp = pScreen->devPrivates[pxScreenPrivateIndex].ptr;
	if (pScreen->CloseScreen != pxCloseScreen) {
		sp->CloseScreen = pScreen->CloseScreen;
		pScreen->CloseScreen = pxCloseScreen;
		sp->mapfd = -1;
	}

	if (sp->sxc == NULL) {
		snprintf(buf, sizeof(buf) - 1, "/dev/stic%d", sxi.sxi_unit);

		if ((sp->mapfd = open(buf, O_RDWR)) < 0)
			FatalError("decPXInit: unable to open %s: %s\n",
			    buf, strerror(errno));

		sz = (size_t)(sizeof(*sxm) - sizeof(sxm->sxm_xcomm) +
		    sxi.sxi_buf_size);

		mode = WSDISPLAYIO_MODE_MAPPED;
		if (ioctl(pFb->fd, WSDISPLAYIO_SMODE, &mode) == -1)
			FatalError("WSDISPLAYIO_SMODE");
		sxm = mmap(NULL, sz, PROT_READ | PROT_WRITE,
		    MAP_SHARED /* | MAP_NODUMP */, sp->mapfd, 0);
		if (sxm == MAP_FAILED) {
			FatalError("unable to map buffers");
			return (FALSE);
		}

		sp->fd = pFb->fd;
		sp->bpp = bpp;
		sp->realbpp = realbpp;
		sp->stampw = sxi.sxi_stampw;
		sp->stamph = sxi.sxi_stamph;
		sp->stamphm = sp->stamph - 1;
		sp->buf_pa = sxi.sxi_buf_phys;
		sp->nbuf = sxi.sxi_buf_pktcnt;
#ifdef notyet
		sp->queueing = (ioctl(pFb->fd, STICIO_STARTQ) == 0);
#else
		sp->queueing = 0;
#endif

		sp->stic = (volatile struct stic_regs *)sxm->sxm_stic;
		sp->poll = (volatile u_int32_t *)sxm->sxm_poll;
		sp->sxc = (volatile struct stic_xcomm *)sxm->sxm_xcomm;
		sp->buf = (u_int32_t *)((caddr_t)sp->sxc + sxi.sxi_buf_pktoff);

		sp->ib[0].ptr =
		    (u_int32_t *)((caddr_t)sp->sxc + sxi.sxi_buf_imgoff); 
		sp->ib[1].ptr =
		    sp->ib[0].ptr + (STIC_IMGBUF_SIZE / sizeof(u_int32_t));

		sp->ib[0].paddr = pxVirtToStic(sp, sp->ib[0].ptr);
		sp->ib[1].paddr = pxVirtToStic(sp, sp->ib[1].ptr);
		sp->ib[0].bufnum = 0;
		sp->ib[1].bufnum = 0;
		sp->ib[0].ident = 2;		/* an impossible ID */
		sp->ib[1].ident = 2;		/* an impossible ID */

		if (sp->bpp == 8) {
			sp->compressBuf = pxCompressBuf24to8;
			if (sp->realbpp == 8) {
				sp->expandBuf = pxExpandBuf8to8;
				sp->tileBuf = pxTileBuf8r8;
			} else {
				sp->expandBuf = pxExpandBuf8to24;
				sp->tileBuf = pxTileBuf8r24;
			}
		} else {
			sp->compressBuf = pxCompressBuf24to24;
			sp->expandBuf = pxExpandBuf24to24;
			sp->tileBuf = pxTileBuf24r24;
		}
	}

	if (bpp > 8) {
		struct wsdisplay_cmap cm;
		u_char intensity[256];
		int i;

		for (i = 0; i < 256; i++)
			intensity[i] = i;

		cm.index = 0;
		cm.count = 256;
		cm.red = intensity;
		cm.green = intensity;
		cm.blue = intensity;

		if (ioctl(pFb->fd, WSDISPLAYIO_PUTCMAP, &cm)) {
			ErrorF("pxFBInit: unable to set colormap\n");
			return (FALSE);
		}
	}

	decColormapScreenInit(pScreen);

	if (!decScreenInit(pScreen)) {
                ErrorF("decScreenInit failed\n");
		return FALSE;
	}

	decSaveScreen(pScreen, SCREEN_SAVER_OFF);
	return (cfbCreateDefColormap(pScreen));
}
