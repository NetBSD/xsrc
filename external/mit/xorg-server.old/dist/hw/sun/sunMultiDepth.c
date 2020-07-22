/* $Xorg: sunMultiDepth.c,v 1.4 2001/02/09 02:04:44 xorgcvs Exp $ */
/*

Copyright 1992, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

/* $XFree86: xc/programs/Xserver/hw/sun/sunMultiDepth.c,v 1.6 2001/12/14 19:59:43 dawes Exp $ */

#include "sun.h"
#include <X11/Xmd.h>
#include "pixmapstr.h"
#include "mi.h"
#include "mistruct.h"
#include "gcstruct.h"
#include "fb.h"

#ifndef SINGLEDEPTH

static Bool
sunCfbCreateGC(GCPtr pGC)
{
    if (pGC->depth == 1)
    {
	return mfbCreateGC (pGC);
    }
    else if (pGC->depth <= 8)
    {
	return cfbCreateGC (pGC);
    }
    else if (pGC->depth <= 16)
    {
	return cfb16CreateGC (pGC);
    }
    else if (pGC->depth <= 32)
    {
	return cfb32CreateGC (pGC);
    }
    return FALSE;
}

static void
sunCfbGetSpans(
    DrawablePtr		pDrawable,	/* drawable from which to get bits */
    int			wMax,		/* largest value of all *pwidths */
    DDXPointPtr 	ppt,		/* points to start copying from */
    int			*pwidth,	/* list of number of bits to copy */
    int			nspans,		/* number of scanlines to copy */
    char		*pdstStart	/* where to put the bits */
)
{
    switch (pDrawable->bitsPerPixel) {
    case 1:
	mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 8:
	cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 16:
	cfb16GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    case 32:
	cfb32GetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
	break;
    }
    return;
}

static void
sunCfbGetImage(DrawablePtr pDrawable, int sx, int sy, int w, int h, unsigned int format, unsigned long planeMask, char *pdstLine)
{
    switch (pDrawable->bitsPerPixel)
    {
    case 1:
	mfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 8:
	cfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 16:
	cfb16GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    case 32:
	cfb32GetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	break;
    }
}

Bool
sunCfbSetupScreen(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize,			/* in pixels */
    int ysize,			/* in pixels */
    int dpix,			/* dots per inch */
    int dpiy,			/* dots per inch */
    int width,			/* pixel width of frame buffer */
    int	bpp			/* bits per pixel of root */
)
{
    int ret;

    sunRegisterPixmapFormat( /* depth */ 1,  /* bits per pixel */ 1);
    sunRegisterPixmapFormat( /* depth */ 8,  /* bits per pixel */ 8);
    sunRegisterPixmapFormat( /* depth */ 12,  /* bits per pixel */ 16);
    sunRegisterPixmapFormat( /* depth */ 24,  /* bits per pixel */ 32);

    ret = fbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp);
    pScreen->CreateGC = sunCfbCreateGC;
    pScreen->GetImage = sunCfbGetImage;
    pScreen->GetSpans = sunCfbGetSpans;
    return ret;
}

Bool
sunCfbFinishScreenInit(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize,			/* in pixels */
    int ysize,			/* in pixels */
    int dpix,			/* dots per inch */
    int dpiy,			/* dots per inch */
    int width,			/* pixel width of frame buffer */
    int bpp
)
{
    int		i;
    VisualPtr	visuals;
    int		nvisuals;
    DepthPtr	depths;
    int		ndepths;
    VisualID	defaultVisual;
    int		rootdepth;

    if (!fbInitVisuals(&visuals, &depths, &nvisuals, &ndepths,
			&rootdepth, &defaultVisual, 1 << (bpp - 1), 8))
	return FALSE;
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals))
	return FALSE;
    pScreen->CloseScreen = fbCloseScreen;
    return TRUE;
}


Bool
sunCfbScreenInit(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize,			/* in pixels */
    int ysize,			/* in pixels */
    int dpix,			/* dots per inch */
    int dpiy,			/* dots per inch */
    int width,			/* pixel width of frame buffer */
    int bpp
)
{
    if (!sunCfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy,
			   width, bpp))
	return FALSE;
    return sunCfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix,
				  dpiy, width, bpp);
}


#else /* SINGLEDEPTH */

/* stuff for 8-bit only server */

Bool
sunCfbSetupScreen(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize,			/* in pixels */
    int ysize,			/* in pixels */
    int dpix,			/* dots per inch */
    int dpiy,			/* dots per inch */
    int width,			/* pixel width of frame buffer */
    int	bpp			/* bits per pixel of root */
)
{
    sunRegisterPixmapFormat( /* depth */ 1, /* bits per pixel */ 1);
    sunRegisterPixmapFormat( /* depth */ 8,  /* bits per pixel */ 8);
    return fbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp);
}

Bool
sunCfbFinishScreenInit(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize,			/* in pixels */
    int ysize,			/* in pixels */
    int dpix,			/* dots per inch */
    int dpiy,			/* dots per inch */
    int width,			/* pixel width of frame buffer */
    int bpp
)
{
    return fbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy,
			      width, bpp);
}

Bool
sunCfbScreenInit(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize,			/* in pixels */
    int ysize,			/* in pixels */
    int dpix,			/* dots per inch */
    int dpiy,			/* dots per inch */
    int width,			/* pixel width of frame buffer */
    int bpp
)
{
    sunRegisterPixmapFormat( /* depth */ 1, /* bits per pixel */ 1);
    sunRegisterPixmapFormat( /* depth */ 8,  /* bits per pixel */ 8);
    return fbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp);
}

#endif /* SINGLEDEPTH */
