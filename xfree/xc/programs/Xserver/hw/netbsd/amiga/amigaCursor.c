/* $XConsortium: amigaCursor.c,v 5.19 94/04/17 20:29:35 gildea Exp $ */
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
 * amigaCursor.c --
 *	Functions for maintaining a kernel-driven hardware cursor...
 *
 */

#ifdef GFX_CARD_SUPPORT /* AmigaCC has no HWC support */

#define NEED_EVENTS
#include "amiga.h"
#ifdef CV64_SUPPORT
#include	"Xmd.h"
#include	"gcstruct.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mistruct.h"
#include	"mifillarc.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"
#include	"s3/amigaCV.h"
#include	"migc.h"
#endif

#define GetCursorPrivate(s) (&(GetScreenPrivate(s)->hardwareCursor))
#define SetupCursor(s)	    amigaCursorPtr pCurPriv = GetCursorPrivate(s)

static void amigaLoadCursor();

static Bool
amigaRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupCursor(pScreen);
    int	    x, y;

    /* miRecolorCursor does this */
    if (pCurPriv->pCursor == pCursor)
    {
	miPointerPosition (&x, &y);
	amigaLoadCursor (pScreen, pCursor, x, y);
    }
    return TRUE;
}

static Bool
amigaUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    return TRUE;
}

static void
amigaCursorRepad (pScreen, bits, src_bits, dst_bits, ptSrc, w, h)
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
    mfbDoBitblt(&src->drawable, &dst->drawable, GXcopy, &rgnDst, ptSrc);
    REGION_UNINIT(pScreen, &rgnDst);
    FreeScratchPixmapHeader(src);
    FreeScratchPixmapHeader(dst);
}

static void
amigaLoadCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x, y;
{
    SetupCursor(pScreen);
    struct grf_spriteinfo spriteinfo;
    int	w, h;
    unsigned char   r[2], g[2], b[2];
    DDXPointRec	ptSrc;
    unsigned char   source_temp[1024], mask_temp[1024];

    spriteinfo.set = GRFSPRSET_ALL;
    spriteinfo.enable = 1;
    spriteinfo.pos.x = x;
    spriteinfo.pos.y = y;
    spriteinfo.hot.x = pCursor->bits->xhot;
    spriteinfo.hot.y = pCursor->bits->yhot;
    r[0] = pCursor->backRed >> 8;
    g[0] = pCursor->backGreen >> 8;
    b[0] = pCursor->backBlue >> 8;
    r[1] = pCursor->foreRed >> 8;
    g[1] = pCursor->foreGreen >> 8;
    b[1] = pCursor->foreBlue >> 8;
    spriteinfo.cmap.index = 0;
    spriteinfo.cmap.count = 2;
    spriteinfo.cmap.red = r;
    spriteinfo.cmap.green = g;
    spriteinfo.cmap.blue = b;
    spriteinfo.image = (char *) pCursor->bits->source;
    spriteinfo.mask = (char *) pCursor->bits->mask;
    w = pCursor->bits->width;
    h = pCursor->bits->height;
    if (w > pCurPriv->width || h > pCurPriv->height) {
	ptSrc.x = 0;
	ptSrc.y = 0;
	if (w > pCurPriv->width)
	    w = pCurPriv->width;
	if (h > pCurPriv->height)
	    h = pCurPriv->height;
	amigaCursorRepad (pScreen, pCursor->bits, pCursor->bits->source,
			source_temp, &ptSrc, w, h);
	amigaCursorRepad (pScreen, pCursor->bits, pCursor->bits->mask,
			mask_temp, &ptSrc, w, h);
	spriteinfo.image = (char *) source_temp;
	spriteinfo.mask = (char *) mask_temp;
    }
    spriteinfo.size.x = w;
    spriteinfo.size.y = h;
    (void) ioctl (amigaFbs[pScreen->myNum].fd, GRFIOCSSPRITEINF, &spriteinfo);
}

static void
amigaSetCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x, y;
{
    SetupCursor(pScreen);

    if (pCursor)
    	amigaLoadCursor (pScreen, pCursor, x, y);
    else
	amigaDisableCursor (pScreen);
    pCurPriv->pCursor = pCursor;
}
#ifdef CV64_SUPPORT

static unsigned short xpan;
static unsigned short ypan;

void
amigaCVSetPanning (inf, xoff, yoff)
	fbFd *inf;
	unsigned short xoff, yoff;
{
	volatile unsigned char *ba = inf->regs;
	int depth = inf->info.gd_planes;
        unsigned long off;

	xpan = xoff;
	ypan = yoff;
	if (depth > 8 && depth <= 16)
		xoff *= 2;
	else if (depth > 16)
		xoff *= 4;

#if 0
	vgar(ba, ACT_ADDRESS_RESET);
	WAttr(ba, ACT_ID_HOR_PEL_PANNING, (unsigned char)((xoff << 1) & 0x07));
	/* have the color lookup function normally again */
	vgaw(ba,  ACT_ADDRESS_W, 0x20);
#endif

	if (depth == 8)
		off = ((yoff * amigaVirtualWidth)/ 4) + (xoff >> 2);
	else if (depth == 16)
		off = ((yoff * amigaVirtualWidth * 2)/ 4) + (xoff >> 2);
	else
		off = ((yoff * amigaVirtualWidth * 4)/ 4) + (xoff >> 2);

	WCrt(ba, CRT_ID_START_ADDR_LOW, ((unsigned char)off));
	off >>= 8;
	WCrt(ba, CRT_ID_START_ADDR_HIGH, ((unsigned char)off));
	off >>= 8;
	WCrt(ba, CRT_ID_EXT_SYS_CNTL_3 , (off & 0x0f));
}
#endif /* CV64_SUPPORT */

static void
amigaMoveCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    SetupCursor(pScreen);
    struct grf_position pos;

#ifdef CV64_SUPPORT
    if (pScreen == amigaCVsavepScreen) /* only on Cybervision */ {
	fbFd *inf = amigaInfo(pScreen);

	if (x < xpan)
		amigaCVSetPanning (inf, x, ypan);
	if (x >= (xpan + amigaRealWidth))
		amigaCVSetPanning (inf, (1 + x - amigaRealWidth), ypan);
	if (y < ypan)
		amigaCVSetPanning(inf,  xpan, y);
	if (y >= (ypan + amigaRealHeight))
		amigaCVSetPanning(inf,  xpan, (1 + y - amigaRealHeight));
	x -= xpan;
	y -= ypan;
    }
#endif /* CV64_SUPPORT */

    pos.x = x;
    pos.y = y;

    ioctl (amigaFbs[pScreen->myNum].fd, GRFIOCSSPRITEPOS, &pos);
}

miPointerSpriteFuncRec amigaPointerSpriteFuncs = {
    amigaRealizeCursor,
    amigaUnrealizeCursor,
    amigaSetCursor,
    amigaMoveCursor,
};

static void
amigaQueryBestSize (class, pwidth, pheight, pScreen)
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

extern miPointerScreenFuncRec	amigaPointerScreenFuncs;

Bool amigaCursorInitialize (
    ScreenPtr	pScreen)
{
    SetupCursor (pScreen);
    struct grf_position maxsize;

    pCurPriv->has_cursor = FALSE;
    if (ioctl (amigaFbs[pScreen->myNum].fd, GRFIOCGSPRITEMAX, &maxsize) == -1)
	return FALSE;
    pCurPriv->width = maxsize.x;
    pCurPriv->height= maxsize.y;
    pScreen->QueryBestSize = amigaQueryBestSize;
    miPointerInitialize (pScreen,
			 &amigaPointerSpriteFuncs,
			 &amigaPointerScreenFuncs,
			 FALSE);
    pCurPriv->has_cursor = TRUE;
    pCurPriv->pCursor = NULL;
    return TRUE;
}

void amigaDisableCursor (
    ScreenPtr	pScreen)
{
    SetupCursor (pScreen);
    struct grf_spriteinfo spriteinfo;

    if (pCurPriv->has_cursor)
    {
    	spriteinfo.set = GRFSPRSET_ENABLE;
    	spriteinfo.enable = 0;
    	(void) ioctl (amigaFbs[pScreen->myNum].fd, GRFIOCSSPRITEINF, &spriteinfo);
	pCurPriv->pCursor = NULL;
    }
}
#endif /*GFX_CARD_SUPPORT*/
