/*	$NetBSD: pxzerarc.c,v 1.2 2004/03/12 21:54:00 matt Exp $	*/

/************************************************************

Copyright (c) 1989  X Consortium

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

********************************************************/

/* $XConsortium: cfbzerarc.c /main/24 1995/12/06 16:58:51 dpw $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbzerarc.c,v 3.0 1996/06/29 09:05:57 dawes Exp $ */

/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

#include "X.h"
#include "Xprotostr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "miscstruct.h"
#include "px.h"
#include "cfb.h"
#include "mizerarc.h"
#include "mi.h"

static __inline__ void
DoPixel(pxScreenPrivPtr sp, pxPacketPtr pxp, int x, int y, int Wx, int Wy)
{
	u_int32_t *pb;
	int psy;

	pb = pxPacketAddPrim(sp, pxp);
	x += Wx;
	psy = ((y + Wy) << 3) + (1 << 2) - 1;
	pb[0] = (x << 19) | psy;
	pb[1] = ((x + 1) << 19) | psy;
}

#define Pixelate(xval,yval) \
    DoPixel(sp, pxp, xval, yval, Wx, Wy);

#define DoPix(idx,xval,yval) \
    if (mask & (1 << idx)) DoPixel(sp, pxp, xval, yval, Wx, Wy);

static void
pxZeroArc(pDraw, pGC, sp, pxp, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    pxScreenPrivPtr sp;
    pxPacketPtr pxp;
    xArc *arc;
{
    miZeroArcRec info;
    int x, y, a, b, d, mask;
    int k1, k3, dx, dy, Wx, Wy;
    Bool do360;

    Wx = pDraw->x;
    Wy = pDraw->y;

    do360 = miZeroArcSetup(arc, &info, TRUE);
    MIARCSETUP();
    mask = info.initialMask;
    if (!(arc->width & 1))
    {
	DoPix(1, info.xorgo, info.yorg);
	DoPix(3, info.xorgo, info.yorgo);
    }
    if (!info.end.x || !info.end.y)
    {
	mask = info.end.mask;
	info.end = info.altend;
    }
    if (do360 && (arc->width == arc->height) && !(arc->width & 1))
    {
	int yorgh = info.yorg + info.h;
	int xorghp = info.xorg + info.h;
	int xorghn = info.xorg - info.h;

	while (1)
	{
	    Pixelate(info.xorg + x, info.yorg + y);
	    Pixelate(info.xorg - x, info.yorg + y);
	    Pixelate(info.xorg - x, info.yorgo - y);
	    Pixelate(info.xorg + x, info.yorgo - y);
	    if (a < 0)
		break;
	    Pixelate(xorghp - y, yorgh - x);
	    Pixelate(xorghn + y, yorgh - x);
	    Pixelate(xorghn + y, yorgh + x);
	    Pixelate(xorghp - y, yorgh + x);
	    MIARCCIRCLESTEP(;);
	}
	x = info.w;
	y = info.h;
    }
    else if (do360)
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(;);
	    Pixelate(info.xorg + x, info.yorg + y);
	    Pixelate(info.xorgo - x, info.yorg + y);
	    Pixelate(info.xorgo - x, info.yorgo - y);
	    Pixelate(info.xorg + x, info.yorgo - y);
	    MIARCSTEP(;,;);
	}
    }
    else
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(;);
	    if ((x == info.start.x) || (y == info.start.y))
	    {
		mask = info.start.mask;
		info.start = info.altstart;
	    }
	    DoPix(0, info.xorg + x, info.yorg + y);
	    DoPix(1, info.xorgo - x, info.yorg + y);
	    DoPix(2, info.xorgo - x, info.yorgo - y);
	    DoPix(3, info.xorg + x, info.yorgo - y);
	    if ((x == info.end.x) || (y == info.end.y))
	    {
		mask = info.end.mask;
		info.end = info.altend;
	    }
	    MIARCSTEP(;,;);
	}
    }
    if ((x == info.start.x) || (y == info.start.y))
	mask = info.start.mask;
    DoPix(0, info.xorg + x, info.yorg + y);
    DoPix(2, info.xorgo - x, info.yorgo - y);
    if (arc->height & 1)
    {
	DoPix(1, info.xorgo - x, info.yorg + y);
	DoPix(3, info.xorg + x, info.yorgo - y);
    }
}

void
pxZeroPolyArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    xArc *arc;
    int i;
    BoxRec box;
    int x2, y2;
    RegionPtr cclip;
    pxPacket pxp;
    pxScreenPrivPtr sp;
    pxPrivGCPtr gcPriv;
    u_int32_t *pb;
    int havepb;

    cclip = cfbGetCompositeClip(pGC);
    gcPriv = pxGetGCPrivate(pGC);
    sp = gcPriv->sp;
    havepb = 0;

    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miCanZeroArc(arc))
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
 	    /*
 	     * Because box.x2 and box.y2 get truncated to 16 bits, and the
 	     * RECT_IN_REGION test treats the resulting number as a signed
 	     * integer, the RECT_IN_REGION test alone can go the wrong way.
 	     * This can result in a server crash because the rendering
 	     * routines in this file deal directly with cpu addresses
 	     * of pixels to be stored, and do not clip or otherwise check
 	     * that all such addresses are within their respective pixmaps.
 	     * So we only allow the RECT_IN_REGION test to be used for
 	     * values that can be expressed correctly in a signed short.
 	     */
 	    x2 = box.x1 + (int)arc->width + 1;
 	    box.x2 = x2;
 	    y2 = box.y1 + (int)arc->height + 1;
 	    box.y2 = y2;
 	    if ( (x2 <= MAXSHORT) && (y2 <= MAXSHORT) &&
 		    (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN) ) {
		if (!havepb) {
		        havepb = 1;
	 		pb = pxPacketStart(sp, &pxp, 6, 2);
			pb[0] = STAMP_CMD_LINES | STAMP_RGB_CONST |
			    STAMP_LW_PERPACKET;
			pb[1] = gcPriv->pmask;
			pb[2] = 0;
			pb[3] = gcPriv->umet;
			pb[4] = (1 << 2) - 1;
			pb[5] = gcPriv->fgFill;
		}

		pxZeroArc(pDraw, pGC, sp, &pxp, arc);
	    } else {
		if (havepb) {
    		    havepb = 0;
    		    pxPacketFlush(sp, &pxp);
		}
		miZeroPolyArc(pDraw, pGC, 1, arc);
	    }
	} else {
            if (havepb) {
                havepb = 0;
                pxPacketFlush(sp, &pxp);
            }
	    miPolyArc(pDraw, pGC, 1, arc);
	}
    }

    if (havepb)
        pxPacketFlush(sp, &pxp);
}
