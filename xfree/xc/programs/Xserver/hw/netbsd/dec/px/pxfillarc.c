/*	$NetBSD: pxfillarc.c,v 1.2 2004/03/12 21:54:00 matt Exp $	*/

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

/* $XConsortium: cfbfillarc.c /main/17 1995/12/06 16:57:18 dpw $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbfillarc.c,v 3.1 1996/08/13 11:27:33 dawes Exp $ */

#include "X.h"
#include "Xprotostr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "miscstruct.h"
#include "mifillarc.h"
#include "mi.h"
#include "px.h"

#include "cfb.h"

#define	INLINE	__inline__

static INLINE void
AddSpan(pxScreenPrivPtr sp, pxPacketPtr pxp, int y, int x1, int x2)
{
	u_int32_t *pb;
	int psy;

	pb = pxPacketAddPrim(sp, pxp);
	psy = (y << 3) + (1 << 2) - 1;
	pb[0] = (x1 << 19) | psy;
	pb[1] = (x2 << 19) | psy;
}

static void
pxFillEllipseI(pDraw, pGC, sp, pxp, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    pxScreenPrivPtr sp;
    pxPacketPtr pxp;
    xArc *arc;
{
    int x, y, e;
    int yk, xk, ym, xm, dx, dy, xorg, yorg;
    int slw;
    miFillArcRec info;

    miFillArcSetup(arc, &info);
    MIFILLARCSETUP();
    xorg += pDraw->x;
    yorg += pDraw->y;

    while (y > 0)
    {
	MIFILLARCSTEP(slw);
	AddSpan(sp, pxp, yorg - y, xorg - x, xorg - x + slw);
	if (miFillArcLower(slw))
    	    AddSpan(sp, pxp, yorg + y + dy, xorg - x, xorg - x + slw);
    }
}

static void
pxFillEllipseD(pDraw, pGC, sp, pxp, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    pxScreenPrivPtr sp;
    pxPacketPtr pxp;
    xArc *arc;
{
    int x, y;
    int xorg, yorg, dx, dy, slw;
    double e, yk, xk, ym, xm;
    miFillArcDRec info;

    miFillArcDSetup(arc, &info);
    MIFILLARCSETUP();
    xorg += pDraw->x;
    yorg += pDraw->y;

    while (y > 0)
    {
	MIFILLARCSTEP(slw);
	AddSpan(sp, pxp, yorg - y, xorg - x, xorg - x + slw);
	if (miFillArcLower(slw))
    	    AddSpan(sp, pxp, yorg + y + dy, xorg - x, xorg - x + slw);
    }
}

#define ADDSPAN(l,r) \
    if (r >= l) \
        AddSpan(sp, pxp, ya, l, r + 1);

#define ADDSLICESPANS(flip) \
    if (!flip) \
    { \
	ADDSPAN(xl, xr); \
    } \
    else \
    { \
	xc = xorg - x; \
	ADDSPAN(xc, xr); \
	xc += slw - 1; \
	ADDSPAN(xl, xc); \
    }

static void
pxFillArcSliceI(pDraw, pGC, sp, pxp, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    pxScreenPrivPtr sp;
    pxPacketPtr pxp;
    xArc *arc;
{
    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
    int x, y, e;
    miFillArcRec info;
    miArcSliceRec slice;
    int ya, xl, xr, xc;

    miFillArcSetup(arc, &info);
    miFillArcSliceSetup(arc, &slice, pGC);
    MIFILLARCSETUP();
    slw = arc->height;
    if (slice.flip_top || slice.flip_bot)
	slw += (arc->height >> 1) + 1;

    xorg += pDraw->x;
    yorg += pDraw->y;
    slice.edge1.x += pDraw->x;
    slice.edge2.x += pDraw->x;

    while (y > 0)
    {
	MIFILLARCSTEP(slw);
	MIARCSLICESTEP(slice.edge1);
	MIARCSLICESTEP(slice.edge2);
	if (miFillSliceUpper(slice))
	{
	    ya = yorg - y;
	    MIARCSLICEUPPER(xl, xr, slice, slw);
	    ADDSLICESPANS(slice.flip_top);
	}
	if (miFillSliceLower(slice))
	{
	    ya = yorg + y + dy;
	    MIARCSLICELOWER(xl, xr, slice, slw);
	    ADDSLICESPANS(slice.flip_bot);
	}
    }
}

static void
pxFillArcSliceD(pDraw, pGC, sp, pxp, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    pxScreenPrivPtr sp;
    pxPacketPtr pxp;
    xArc *arc;
{
    int x, y;
    int dx, dy, xorg, yorg, slw;
    double e, yk, xk, ym, xm;
    miFillArcDRec info;
    miArcSliceRec slice;
    int ya, xl, xr, xc;

    miFillArcDSetup(arc, &info);
    miFillArcSliceSetup(arc, &slice, pGC);
    MIFILLARCSETUP();
    slw = arc->height;
    if (slice.flip_top || slice.flip_bot)
	slw += (arc->height >> 1) + 1;

    xorg += pDraw->x;
    yorg += pDraw->y;
    slice.edge1.x += pDraw->x;
    slice.edge2.x += pDraw->x;

    while (y > 0)
    {
	MIFILLARCSTEP(slw);
	MIARCSLICESTEP(slice.edge1);
	MIARCSLICESTEP(slice.edge2);
	if (miFillSliceUpper(slice))
	{
	    ya = yorg - y;
	    MIARCSLICEUPPER(xl, xr, slice, slw);
	    ADDSLICESPANS(slice.flip_top);
	}
	if (miFillSliceLower(slice))
	{
	    ya = yorg + y + dy;
	    MIARCSLICELOWER(xl, xr, slice, slw);
	    ADDSLICESPANS(slice.flip_bot);
	}
    }
}

void
pxPolyFillArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    xArc *arc;
    int i, can;
    int x2, y2;
    BoxRec box;
    RegionPtr cclip;
    pxPacket pxp;
    pxScreenPrivPtr sp;
    pxPrivGCPtr gcPriv;
    u_int32_t *pb;
    int havepb;

    cclip = cfbGetCompositeClip(pGC);
    havepb = 0;
    gcPriv = pxGetGCPrivate(pGC);
    sp = gcPriv->sp;

    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miFillArcEmpty(arc))
	    continue;

	/*
	 * Tall arcs are faster using mi, due to an optimization in the fill
	 * spans routine.  Emulating it here would kill our performance on
	 * smaller arcs.
	 */
	if (arc->height >= 250) {
	    if (havepb) {
	        havepb = 0;
	        pxPacketFlush(sp, &pxp);
	    }
	    miPolyFillArc(pDraw, pGC, 1, arc);
	    continue;
	}

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
 	    (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN) )
	{
	    can = miCanFillArc(arc);

 	    if (!havepb) {
 	        havepb = 1;
 	        pb = pxPacketStart(sp, &pxp, 6, 2);
	        pb[0] = STAMP_CMD_LINES | STAMP_RGB_CONST | STAMP_LW_PERPACKET;
	        pb[1] = gcPriv->pmask;
	        pb[2] = 0;
	        pb[3] = gcPriv->umet;
	        pb[4] = (1 << 2) - 1;
	        pb[5] = gcPriv->fgFill;
	    }

	    if ((arc->angle2 >= FULLCIRCLE) ||
		(arc->angle2 <= -FULLCIRCLE)) {
		if (can)
		    pxFillEllipseI(pDraw, pGC, sp, &pxp, arc);
		else
		    pxFillEllipseD(pDraw, pGC, sp, &pxp, arc);
	    } else {
	        if (can)
		    pxFillArcSliceI(pDraw, pGC, sp, &pxp, arc);
		else
		    pxFillArcSliceD(pDraw, pGC, sp, &pxp, arc);
	    }
	} else {
	    if (havepb) {
	        havepb = 0;
	        pxPacketFlush(sp, &pxp);
	    }
	    miPolyFillArc(pDraw, pGC, 1, arc);
	}
    }

    if (havepb)
	pxPacketFlush(sp, &pxp);
}
