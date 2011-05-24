/*	$NetBSD: decCursor.c,v 1.6 2011/05/24 21:27:00 jakllsch Exp $	*/

/* XConsortium: sunCursor.c,v 5.19 94/04/17 20:29:35 gildea Exp */
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
#include    "dec.h"
#include    "mfb.h"

#define GetCursorPrivate(s) (&(GetScreenPrivate(s)->hardwareCursor))
#define SetupCursor(s)	    decCursorPtr pCurPriv = GetCursorPrivate(s)

static void decLoadCursor();
static Bool decCursorOffScreen();
static void decCrossScreen();
static void decWarpCursor();

Bool decActiveZaphod = TRUE;

miPointerScreenFuncRec decPointerScreenFuncs = {
    decCursorOffScreen,
    decCrossScreen,
    decWarpCursor,
};

static Bool
decRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupCursor(pScreen);
    int	    x, y;

    /* miRecolorCursor does this */
    if (pCurPriv->pCursor == pCursor)
    {
	miPointerPosition (&x, &y);
	decLoadCursor (pScreen, pCursor, x, y);
    }
    return TRUE;
}

static Bool
decUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    return TRUE;
}

static void
decCursorRepad (pScreen, bits, src_bits, dst_bits, ptSrc, w, h, nw, nh)
    ScreenPtr	    pScreen;
    CursorBitsPtr   bits;
    unsigned char   *src_bits, *dst_bits;
    DDXPointPtr	    ptSrc;
    int		    w, h;
    int             nw, nh;
{
    SetupCursor(pScreen);
    PixmapPtr	src, dst;
    BoxRec	box;
    RegionRec	rgnDst;

    if (!(src = GetScratchPixmapHeader(pScreen, bits->width, bits->height,
				       /*bpp*/ 1, /*depth*/ 1,
				      PixmapBytePad(bits->width,1), src_bits)))
	return;
    if (!(dst = GetScratchPixmapHeader(pScreen, nw, nh, /*bpp*/ 1, /*depth*/ 1,
				       PixmapBytePad(nw,1), dst_bits)))
    {
	FreeScratchPixmapHeader(src);
	return;
    }
    box.x1 = 0;
    box.y1 = 0;
    box.x2 = w;
    box.y2 = h;
    REGION_INIT(pScreen, &rgnDst, &box, 1);
    mfbDoBitblt(&src->drawable, &dst->drawable, GXcopy, &rgnDst, ptSrc);
    REGION_UNINIT(pScreen, &rgnDst);
    FreeScratchPixmapHeader(src);
    FreeScratchPixmapHeader(dst);
}

static void
decLoadCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x, y;
{
    SetupCursor(pScreen);
    struct wsdisplay_cursor wscursor;
    int	w, h;
    unsigned char   r[2], g[2], b[2];
    DDXPointRec	ptSrc;
    unsigned char   source_temp[1024], mask_temp[1024];

    wscursor.which = WSDISPLAY_CURSOR_DOALL;
    wscursor.enable = 1;
    wscursor.pos.x = x;
    wscursor.pos.y = y;
    wscursor.hot.x = pCursor->bits->xhot;
    wscursor.hot.y = pCursor->bits->yhot;
    r[1] = pCursor->backRed >> 8;
    g[1] = pCursor->backGreen >> 8;
    b[1] = pCursor->backBlue >> 8;
    r[0] = pCursor->foreRed >> 8;
    g[0] = pCursor->foreGreen >> 8;
    b[0] = pCursor->foreBlue >> 8;
    wscursor.cmap.index = 0;
    wscursor.cmap.count = 2;
    wscursor.cmap.red = r;
    wscursor.cmap.green = g;
    wscursor.cmap.blue = b;
    wscursor.image = (char *) pCursor->bits->source;
    wscursor.mask = (char *) pCursor->bits->mask;
    w = pCursor->bits->width;
    h = pCursor->bits->height;

    if (w != pCurPriv->width || h != pCurPriv->height) {
	ptSrc.x = 0;
	ptSrc.y = 0;
	if (w > pCurPriv->width)
	    w = pCurPriv->width;
	if (w > pCurPriv->height)
	    h = pCurPriv->height;
	memset(source_temp, 0, sizeof(source_temp));
	memset(mask_temp, 0, sizeof(mask_temp));
	decCursorRepad (pScreen, pCursor->bits, pCursor->bits->source,
			source_temp, &ptSrc, w, h, pCurPriv->width, pCurPriv->height);
	decCursorRepad (pScreen, pCursor->bits, pCursor->bits->mask,
			mask_temp, &ptSrc, w, h, pCurPriv->width, pCurPriv->height);
	wscursor.image = (char *) source_temp;
	wscursor.mask = (char *) mask_temp;
    }
    wscursor.size.x = pCurPriv->width;
    wscursor.size.y = pCurPriv->height;
    (void) ioctl (decFbs[pScreen->myNum].fd, WSDISPLAYIO_SCURSOR, &wscursor);
}

static void
decSetCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x, y;
{
    SetupCursor(pScreen);

    if (pCursor)
    	decLoadCursor (pScreen, pCursor, x, y);
    else
	decDisableCursor (pScreen);
    pCurPriv->pCursor = pCursor;
}

static void
decMoveCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    SetupCursor(pScreen);
    struct wsdisplay_curpos pos;

    pos.x = x;
    pos.y = y;
    ioctl (decFbs[pScreen->myNum].fd, WSDISPLAYIO_SCURPOS, &pos);
}

miPointerSpriteFuncRec decPointerSpriteFuncs = {
    decRealizeCursor,
    decUnrealizeCursor,
    decSetCursor,
    decMoveCursor,
};

static void
decQueryBestSize (class, pwidth, pheight, pScreen)
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

extern miPointerScreenFuncRec	decPointerScreenFuncs;

Bool decCursorInitialize (pScreen)
    ScreenPtr	pScreen;
{

    SetupCursor (pScreen);
    struct wsdisplay_curpos maxsize;

    pCurPriv->has_cursor = FALSE;
    if (ioctl (decFbs[pScreen->myNum].fd, WSDISPLAYIO_GCURMAX, &maxsize) == -1)
	return FALSE;
    pCurPriv->width = maxsize.x;
    pCurPriv->height= maxsize.y;
    pScreen->QueryBestSize = decQueryBestSize;
    miPointerInitialize (pScreen,
			 &decPointerSpriteFuncs,
			 &decPointerScreenFuncs,
			 FALSE);
    pCurPriv->has_cursor = TRUE;
    pCurPriv->pCursor = NULL;
    return TRUE;
}

void decDisableCursor (pScreen)
    ScreenPtr	pScreen;
{
#ifdef notyet
    SetupCursor (pScreen);
    struct wsdisplay_cursor wscursor;

    if (pCurPriv->has_cursor)
    {
    	wscursor.set = FB_CUR_SETCUR;
    	wscursor.enable = 0;
    	(void) ioctl (decFbs[pScreen->myNum].fd, FBIOSCURSOR, &wscursor);
	pCurPriv->pCursor = NULL;
    }
#endif
}

/*ARGSUSED*/
static Bool
decCursorOffScreen (pScreen, x, y)
    ScreenPtr	*pScreen;
    int		*x, *y;
{
    int	    index, ret = FALSE;
    extern Bool PointerConfinedToScreen();

    if (PointerConfinedToScreen()) return TRUE;
    /*
     * Active Zaphod implementation:
     *    increment or decrement the current screen
     *    if the x is to the right or the left of
     *    the current screen.
     */
    if (decActiveZaphod &&
	screenInfo.numScreens > 1 && (*x >= (*pScreen)->width || *x < 0)) {
	index = (*pScreen)->myNum;
	if (*x < 0) {
	    index = (index ? index : screenInfo.numScreens) - 1;
	    *pScreen = screenInfo.screens[index];
	    *x += (*pScreen)->width;
	} else {
	    *x -= (*pScreen)->width;
	    index = (index + 1) % screenInfo.numScreens;
	    *pScreen = screenInfo.screens[index];
	}
	ret = TRUE;
    }
    return ret;
}

static void
decCrossScreen (pScreen, entering)
    ScreenPtr	pScreen;
    Bool	entering;
{
    if (decFbs[pScreen->myNum].EnterLeave)
	(*decFbs[pScreen->myNum].EnterLeave) (pScreen, entering ? 0 : 1);
}

static void
decWarpCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    sigset_t newsigmask;

    (void) sigemptyset (&newsigmask);
    (void) sigaddset (&newsigmask, SIGIO);
    (void) sigprocmask (SIG_BLOCK, &newsigmask, (sigset_t *)NULL);
    miPointerWarpCursor (pScreen, x, y);
    (void) sigprocmask (SIG_UNBLOCK, &newsigmask, (sigset_t *)NULL);
}
