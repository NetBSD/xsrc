/* $Xorg: mdepthinit.c,v 1.3 2000/08/17 19:48:20 cpqbld Exp $ */
/*

Copyright 1992, 1998  The Open Group

All Rights Reserved.

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

/* $XFree86: xc/programs/Xserver/hw/dec/ws/mdepthinit.c,v 1.6 2001/01/17 22:36:46 dawes Exp $ */

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "gcstruct.h"
#include "mibstore.h"

extern int defaultColorVisualClass;

#ifndef SINGLEDEPTH

Bool
mcfbCreateGC(pGC)
    GCPtr   pGC;
{
    switch (BitsPerPixel(pGC->depth)) {
    case 1:
	return mfbCreateGC (pGC);
    case 8:
	return cfbCreateGC (pGC);
    case 16:
	return cfb16CreateGC (pGC);
    case 32:
	return cfb32CreateGC (pGC);
    }
    return FALSE;
}

void
mcfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    char		*pdstStart;	/* where to put the bits */
{
    switch (BitsPerPixel(pDrawable->depth)) {
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

void
mcfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    char	*pdstLine;
{
    switch (BitsPerPixel(pDrawable->depth)) 
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
mcfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
    int depth;			/* depth of root */
    int	bpp;			/* bits per pixel of root */
{
    extern int		cfbWindowPrivateIndex;
    extern int		cfbGCPrivateIndex;

    switch (bpp) {
#ifdef LOWMEMFTPT
    case 1:
#endif
    case 8:
	cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
	break;
    case 16:
	cfb16SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
	break;
    case 32:
	cfb32SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
	break;
    default:
	return FALSE;
    }
    pScreen->CreateGC = mcfbCreateGC;
    pScreen->GetImage = mcfbGetImage;
    pScreen->GetSpans = mcfbGetSpans;
    return TRUE;
}

extern int  cfb16ScreenPrivateIndex, cfb32ScreenPrivateIndex;

Bool
mcfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    int		i;
    pointer	oldDevPrivate;
    VisualPtr	visuals;
    int		nvisuals;
    DepthPtr	depths;
    int		ndepths;
    VisualID	defaultVisual;
    int		rootdepth;
    extern BSFuncRec	cfbBSFuncRec, cfb16BSFuncRec, cfb32BSFuncRec;
    extern Bool		cfbCloseScreen(), cfb16CloseScreen(), cfb32CloseScreen();

    rootdepth = depth;
    if (!cfbInitVisuals(&visuals, &depths, &nvisuals, &ndepths, &rootdepth, &defaultVisual, 1 << (bpp - 1), 8))
	return FALSE;
    rootdepth = depth;
    oldDevPrivate = pScreen->devPrivate;
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals))
	return FALSE;
    switch(bpp)
    {
#ifdef LOWMEMFTPT
    case 1:
#endif
    case 8:
	pScreen->CloseScreen = cfbCloseScreen;
	pScreen->BackingStoreFuncs = cfbBSFuncRec;
	break;
    case 16:
	pScreen->CloseScreen = cfb16CloseScreen;
	pScreen->devPrivates[cfb16ScreenPrivateIndex].ptr = pScreen->devPrivate;
	pScreen->devPrivate = oldDevPrivate;
	pScreen->BackingStoreFuncs = cfb16BSFuncRec;
	break;
    case 32:
	pScreen->CloseScreen = cfb32CloseScreen;
	pScreen->devPrivates[cfb32ScreenPrivateIndex].ptr = pScreen->devPrivate;
	pScreen->devPrivate = oldDevPrivate;
	pScreen->BackingStoreFuncs = cfb32BSFuncRec;
	break;
    }
    return TRUE;
}


/* dts * (inch/dot) * (25.4 mm / inch) = mm */

Bool
mcfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    if (!mcfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth))
	return FALSE;
    return mcfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth);
}

void
mcfbFillInMissingPixmapDepths(bitsPerDepth)
    int *bitsPerDepth;
{
    int i, j;

    j = 0;
    for (i = 1; i <= 32; i++)
    {
	if (bitsPerDepth[i])
	    j |= 1 << (bitsPerDepth[i] - 1);
    }
    if (!(j & (1 << 3)))
	bitsPerDepth[4] = 8;
    if (!(j & (1 << 7)))
	bitsPerDepth[8] = 8;
    if (!(j & (1 << 15)))
	bitsPerDepth[12] = 32;
    if (!(j & (1 << 23)))
	bitsPerDepth[24] = 32;
    if (!(j & (1 << 31)))
	bitsPerDepth[32] = 32;
}

#else /* SINGLEDEPTH */

/* stuff for 8-bit only server */

Bool
mcfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp, depth)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    return cfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy,
			 width, bpp, depth);
}

void
mcfbFillInMissingPixmapDepths(bitsPerDepth)
    int *bitsPerDepth;
{
    /* This does something useful in the multidepth case.  We don't
     * do anything in the single depth case, but we still have to provide
     * the function for linking.
     */
}

#endif /* SINGLEDEPTH */
