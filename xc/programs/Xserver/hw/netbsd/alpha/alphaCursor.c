/* $XConsortium: sunCursor.c,v 5.19 94/04/17 20:29:35 gildea Exp $ */
/*

Copyright (c) 1988  Sun Microsystems, Inc.
Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*-
 * sunCursor.c --
 *	Functions for maintaining the Sun software cursor...
 *
 */

#define NEED_EVENTS
#include    "alpha.h"

#undef FBIOGCURMAX	/* XXX cgd */

#ifdef FBIOGCURMAX  /* has hardware cursor kernel support */

#define GetCursorPrivate(s) (&(GetScreenPrivate(s)->hardwareCursor))
#define SetupCursor(s)	    alphaCursorPtr pCurPriv = GetCursorPrivate(s)

static void alphaLoadCursor();

static Bool
alphaRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupCursor(pScreen);
    int	    x, y;

    /* miRecolorCursor does this */
    if (pCurPriv->pCursor == pCursor)
    {
	miPointerPosition (&x, &y);
	alphaLoadCursor (pScreen, pCursor, x, y);
    }
    return TRUE;
}

static Bool
alphaUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    return TRUE;
}

static void
alphaCursorRepad (pScreen, bits, src_bits, dst_bits, ptSrc, w, h)
    ScreenPtr	    pScreen;
    CursorBitsPtr   bits;
    unsigned char   *src_bits, *dst_bits;
    DDXPointPtr	    ptSrc;
    int		    w, h;
{
    SetupCursor(pScreen);
    PixmapPtr	src, dst;
    BoxRec	box;
    RegionRec	rgnDst;
    extern int mfbDoBitblt();

    if (!(src = GetScratchPixmapHeader(pScreen, bits->width, bits->height,
				       /*bpp*/ 1, /*depth*/ 1,
				      PixmapBytePad(bits->width,1), src_bits)))
	return;
    if (!(dst = GetScratchPixmapHeader(pScreen, w, h, /*bpp*/ 1, /*depth*/ 1,
				       PixmapBytePad(w,1), dst_bits)))
    {
	FreeScratchPixmapHeader(src);
	return;
    }
    box.x1 = 0;
    box.y1 = 0;
    box.x2 = w;
    box.y2 = h;
    REGION_INIT(pScreen, &rgnDst, &box, 1);
    mfbDoBitblt(src, dst, GXcopy, &rgnDst, ptSrc);
    REGION_UNINIT(pScreen, &rgnDst);
    FreeScratchPixmapHeader(src);
    FreeScratchPixmapHeader(dst);
}

static void
alphaLoadCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x, y;
{
    SetupCursor(pScreen);
    struct fbcursor fbcursor;
    int	w, h;
    unsigned char   r[2], g[2], b[2];
    DDXPointRec	ptSrc;
    unsigned char   source_temp[1024], mask_temp[1024];

    fbcursor.set = FB_CUR_SETALL;
    fbcursor.enable = 1;
    fbcursor.pos.x = x;
    fbcursor.pos.y = y;
    fbcursor.hot.x = pCursor->bits->xhot;
    fbcursor.hot.y = pCursor->bits->yhot;
    r[0] = pCursor->backRed >> 8;
    g[0] = pCursor->backGreen >> 8;
    b[0] = pCursor->backBlue >> 8;
    r[1] = pCursor->foreRed >> 8;
    g[1] = pCursor->foreGreen >> 8;
    b[1] = pCursor->foreBlue >> 8;
    fbcursor.cmap.index = 0;
    fbcursor.cmap.count = 2;
    fbcursor.cmap.red = r;
    fbcursor.cmap.green = g;
    fbcursor.cmap.blue = b;
    fbcursor.image = (char *) pCursor->bits->source;
    fbcursor.mask = (char *) pCursor->bits->mask;
    w = pCursor->bits->width;
    h = pCursor->bits->height;
    if (w > pCurPriv->width || h > pCurPriv->height) {
	ptSrc.x = 0;
	ptSrc.y = 0;
	if (w > pCurPriv->width)
	    w = pCurPriv->width;
	if (h > pCurPriv->height)
	    h = pCurPriv->height;
	alphaCursorRepad (pScreen, pCursor->bits, pCursor->bits->source,
			source_temp, &ptSrc, w, h);
	alphaCursorRepad (pScreen, pCursor->bits, pCursor->bits->mask,
			mask_temp, &ptSrc, w, h);
	fbcursor.image = (char *) source_temp;
	fbcursor.mask = (char *) mask_temp;
    }
    fbcursor.size.x = w;
    fbcursor.size.y = h;
    (void) ioctl (alphaFbs[pScreen->myNum].fd, FBIOSCURSOR, &fbcursor);
}

static void
alphaSetCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x, y;
{
    SetupCursor(pScreen);

    if (pCursor)
    	alphaLoadCursor (pScreen, pCursor, x, y);
    else
	alphaDisableCursor (pScreen);
    pCurPriv->pCursor = pCursor;
}

static void
alphaMoveCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    SetupCursor(pScreen);
    struct fbcurpos pos;

    pos.x = x;
    pos.y = y;
    ioctl (alphaFbs[pScreen->myNum].fd, FBIOSCURPOS, &pos);
}

miPointerSpriteFuncRec alphaPointerSpriteFuncs = {
    alphaRealizeCursor,
    alphaUnrealizeCursor,
    alphaSetCursor,
    alphaMoveCursor,
};

static void
alphaQueryBestSize (class, pwidth, pheight, pScreen)
    int	class;
    unsigned short   *pwidth, *pheight;
    ScreenPtr	pScreen;
{
    SetupCursor (pScreen);

    switch (class)
    {
    case CursorShape:
	if (*pwidth > pCurPriv->width)
	    *pwidth = pCurPriv->width;
	if (*pheight > pCurPriv->height)
	    *pheight = pCurPriv->height;
	if (*pwidth > pScreen->width)
	    *pwidth = pScreen->width;
	if (*pheight > pScreen->height)
	    *pheight = pScreen->height;
	break;
    default:
	mfbQueryBestSize (class, pwidth, pheight, pScreen);
	break;
    }
}

extern miPointerScreenFuncRec	alphaPointerScreenFuncs;

#endif

#if NeedFunctionPrototypes
Bool alphaCursorInitialize (
    ScreenPtr	pScreen)
#else
Bool alphaCursorInitialize (pScreen)
    ScreenPtr	pScreen;
#endif
{
#ifdef FBIOGCURMAX
    SetupCursor (pScreen);
    struct fbcurpos maxsize;

    pCurPriv->has_cursor = FALSE;
    if (ioctl (alphaFbs[pScreen->myNum].fd, FBIOGCURMAX, &maxsize) == -1)
	return FALSE;
    pCurPriv->width = maxsize.x;
    pCurPriv->height= maxsize.y;
    pScreen->QueryBestSize = alphaQueryBestSize;
    miPointerInitialize (pScreen,
			 &alphaPointerSpriteFuncs,
			 &alphaPointerScreenFuncs,
			 FALSE);
    pCurPriv->has_cursor = TRUE;
    pCurPriv->pCursor = NULL;
    return TRUE;
#else
    return FALSE;
#endif
}

#if NeedFunctionPrototypes
void alphaDisableCursor (
    ScreenPtr	pScreen)
#else
void alphaDisableCursor (pScreen)
    ScreenPtr	pScreen;
#endif
{
#ifdef FBIOGCURMAX
    SetupCursor (pScreen);
    struct fbcursor fbcursor;

    if (pCurPriv->has_cursor)
    {
    	fbcursor.set = FB_CUR_SETCUR;
    	fbcursor.enable = 0;
    	(void) ioctl (alphaFbs[pScreen->myNum].fd, FBIOSCURSOR, &fbcursor);
	pCurPriv->pCursor = NULL;
    }
#endif
}
