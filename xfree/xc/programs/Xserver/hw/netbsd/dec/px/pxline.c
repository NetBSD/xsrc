/*	$NetBSD: pxline.c,v 1.2 2008/04/28 20:57:37 martin Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
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

/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
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
/* from XConsortium: cfbline.c,v 1.19 91/08/13 18:48:42 keith Exp */

#include "px.h"

#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"
#include "miline.h"

#include "cfb.h"

/* single-pixel lines on a color frame buffer

   NON-SLOPED LINES
   horizontal lines are always drawn left to right; we have to
move the endpoints right by one after they're swapped.
   horizontal lines will be confined to a single band of a
region.  the code finds that band (giving up if the lower
bound of the band is above the line we're drawing); then it
finds the first box in that band that contains part of the
line.  we clip the line to subsequent boxes in that band.
   vertical lines are always drawn top to bottom (y-increasing.)
this requires adding one to the y-coordinate of each endpoint
after swapping.

   SLOPED LINES
   when clipping a sloped line, we bring the second point inside
the clipping box, rather than one beyond it, and then add 1 to
the length of the line before drawing it.  this lets us use
the same box for finding the outcodes for both endpoints.  since
the equation for clipping the second endpoint to an edge gives us
1 beyond the edge, we then have to move the point towards the
first point by one step on the major axis.
   eventually, there will be a diagram here to explain what's going
on.  the method uses Cohen-Sutherland outcodes to determine
outsideness, and a method similar to Pike's layers for doing the
actual clipping.

*/

#define SignTimes(sign, n) \
    ( ((sign)<0) ? -(n) : (n) )

#define SWAPPT(i, j) \
{  DDXPointRec _t; \
   _t = i; \
   i = j; \
   j = _t; \
}

void
#ifdef _POLYSEGMENT
pxPolySegment(pDrawable, pGC, nseg, pSeg)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nseg;
    xSegment	*pSeg;
#else
pxPolylines(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;		/* number of points */
    DDXPointPtr pptInit;
#endif
{
    pxScreenPrivPtr sp;
    pxPrivGCPtr gcPriv;
    int nboxInit;
    int nbox;
    BoxPtr pboxInit;
    BoxPtr pbox;
#ifndef _POLYSEGMENT
    DDXPointPtr ppt;	/* pointer to list of translated points */
#endif
    pxPacket pxp;

    unsigned int oc1;		/* outcode of point 1 */
    unsigned int oc2;		/* outcode of point 2 */

    int xorg, yorg;		/* origin of window */

    int adx;		/* abs values of dx and dy */
    int ady;
    int signdx;		/* sign of dx and dy */
    int signdy;
    int e, e1, e2;		/* bresenham error and increments */
    int axis;			/* major axis */
    int octant;
    unsigned int bias = miGetZeroLineBias(pDrawable->pScreen);
    int tmp;
    int y1, y2;
    int x1, x2;
    RegionPtr cclip;
    cfbPrivGCPtr    devPriv;
    u_int32_t	*pb;

#ifdef _POLYSEGMENT
    PX_TRACE("pxPolySegment");
#else
    PX_TRACE("pxPolyLines");
#endif

    gcPriv = pxGetGCPrivate(pGC);
    devPriv = cfbGetGCPrivate(pGC);
    cclip = cfbGetCompositeClip(pGC);
    sp = gcPriv->sp;

    pboxInit = REGION_RECTS(cclip);
    nboxInit = REGION_NUM_RECTS(cclip);

    pb = pxPacketStart(sp, &pxp, 6, 2);
    pb[0] = STAMP_CMD_LINES | STAMP_RGB_CONST | STAMP_LW_PERPACKET;
    pb[1] = gcPriv->pmask;
    pb[2] = 0;
    pb[3] = gcPriv->umet;
    pb[4] = 4;
    pb[5] = gcPriv->fgFill;

    xorg = pDrawable->x;
    yorg = pDrawable->y;
#ifdef _POLYSEGMENT
    while (nseg--)
#else
    ppt = pptInit;
    x2 = ppt->x + xorg;
    y2 = ppt->y + yorg;
    while(--npt)
#endif
    {
	nbox = nboxInit;
	pbox = pboxInit;

#ifdef _POLYSEGMENT
	x1 = pSeg->x1 + xorg;
	y1 = pSeg->y1 + yorg;
	x2 = pSeg->x2 + xorg;
	y2 = pSeg->y2 + yorg;
	pSeg++;
#else
	x1 = x2;
	y1 = y2;
	++ppt;
	if (mode == CoordModePrevious)
	{
	    xorg = x1;
	    yorg = y1;
	}
	x2 = ppt->x + xorg;
	y2 = ppt->y + yorg;
#endif

	if (x1 == x2)  /* vertical line */
	{
	    /* make the line go top to bottom of screen, keeping
	       endpoint semantics
	    */
	    if (y1 > y2)
	    {
		int tmp;

		tmp = y2;
		y2 = y1 + 1;
		y1 = tmp + 1;
#ifdef _POLYSEGMENT
		if (pGC->capStyle != CapNotLast)
		    y1--;
#endif
	    }
#ifdef _POLYSEGMENT
	    else if (pGC->capStyle != CapNotLast)
		y2++;
#endif
	    /* get to first band that might contain part of line */
	    while ((nbox) && (pbox->y2 <= y1))
	    {
		pbox++;
		nbox--;
	    }

	    if (nbox)
	    {
		/* stop when lower edge of box is beyond end of line */
		while((nbox) && (y2 >= pbox->y1))
		{
		    if ((x1 >= pbox->x1) && (x1 < pbox->x2))
		    {
			int y1t, y2t;
			/* this box has part of the line in it */
			y1t = max(y1, pbox->y1);
			y2t = min(y2, pbox->y2);
			if (y1t != y2t)
			{
			    pxAddLine(sp, &pxp, x1, y1t, x1, y2t);
			}
		    }
		    nbox--;
		    pbox++;
		}
	    }
#ifndef _POLYSEGMENT
	    y2 = ppt->y + yorg;
#endif
	}
	else if (y1 == y2)  /* horizontal line */
	{
	    /* force line from left to right, keeping
	       endpoint semantics
	    */
	    if (x1 > x2)
	    {
		int tmp;

		tmp = x2;
		x2 = x1 + 1;
		x1 = tmp + 1;
#ifdef _POLYSEGMENT
		if (pGC->capStyle != CapNotLast)
		    x1--;
#endif
	    }
#ifdef _POLYSEGMENT
	    else if (pGC->capStyle != CapNotLast)
		x2++;
#endif

	    /* find the correct band */
	    while( (nbox) && (pbox->y2 <= y1))
	    {
		pbox++;
		nbox--;
	    }

	    /* try to draw the line, if we haven't gone beyond it */
	    if ((nbox) && (pbox->y1 <= y1))
	    {
		/* when we leave this band, we're done */
		tmp = pbox->y1;
		while((nbox) && (pbox->y1 == tmp))
		{
		    int	x1t, x2t;

		    if (pbox->x2 <= x1)
		    {
			/* skip boxes until one might contain start point */
			nbox--;
			pbox++;
			continue;
		    }

		    /* stop if left of box is beyond right of line */
		    if (pbox->x1 >= x2)
		    {
			nbox = 0;
			break;
		    }

		    x1t = max(x1, pbox->x1);
		    x2t = min(x2, pbox->x2);
		    if (x1t != x2t)
		    {
			pxAddLine(sp, &pxp, x1t, y1, x2t, y1);
		    }
		    nbox--;
		    pbox++;
		}
	    }
#ifndef _POLYSEGMENT
	    x2 = ppt->x + xorg;
#endif
	}
	else	/* sloped line */
	{
	    CalcLineDeltas(x1, y1, x2, y2, adx, ady, signdx, signdy,
			   1, 1, octant);

	    if (adx > ady)
	    {
		axis = X_AXIS;
		e1 = ady << 1;
		e2 = e1 - (adx << 1);
		e = e1 - adx;
 	    }
	    else
	    {
		axis = Y_AXIS;
		e1 = adx << 1;
		e2 = e1 - (ady << 1);
		e = e1 - ady;
		SetYMajorOctant(octant);
	    }

	    FIXUP_ERROR(e, octant, bias);

	    /* we have bresenham parameters and two points.
	       all we have to do now is clip and draw.
	    */

	    while(nbox--)
	    {
		oc1 = 0;
		oc2 = 0;
		OUTCODES(oc1, x1, y1, pbox);
		OUTCODES(oc2, x2, y2, pbox);
		if ((oc1 | oc2) == 0)
		{
#ifdef _POLYSEGMENT
		    if (pGC->capStyle != CapNotLast)
		    {
			if (axis == X_AXIS)
			    x2++;
			else
		  	    y2++;
		    }
#endif
		    pxAddLine(sp, &pxp, x1, y1, x2, y2);
		    break;
		}
		else if (oc1 & oc2)
		{
		    pbox++;
		}
		else
		{
		    int new_x1 = x1, new_y1 = y1, new_x2 = x2, new_y2 = y2;
		    int clip1 = 0, clip2 = 0;
		    
		    if (miZeroClipLine(pbox->x1, pbox->y1, pbox->x2-1,
				       pbox->y2-1,
				       &new_x1, &new_y1, &new_x2, &new_y2,
				       adx, ady, &clip1, &clip2,
				       octant, bias, oc1, oc2) == -1)
		    {
			pbox++;
			continue;
		    }

#ifdef _POLYSEGMENT
		    	if (clip2 != 0 || pGC->capStyle != CapNotLast)
			{
			    if (axis == X_AXIS)
			        new_x2++;
			    else
			        new_y2++;
			}
#else
			if (axis == X_AXIS)
			    new_x2 += (clip2 != 0);
			else
			    new_y2 += (clip2 != 0);
#endif
	                pxAddLine(sp, &pxp, new_x1, new_y1, new_x2, new_y2);

		    pbox++;
		}
	    } /* while (nbox--) */
	} /* sloped line */
    } /* while (nline--) */

#ifndef _POLYSEGMENT
    /* paint the last point if the end style isn't CapNotLast.
       (Assume that a projecting, butt, or round cap that is one
        pixel wide is the same as the single pixel of the endpoint.)
    */

    if ((pGC->capStyle != CapNotLast) &&
	((ppt->x + xorg != pptInit->x + pDrawable->x) ||
	 (ppt->y + yorg != pptInit->y + pDrawable->y) ||
	 (ppt == pptInit + 1)))
    {
	nbox = nboxInit;
	pbox = pboxInit;
	while (nbox--)
	{
	    if ((x2 >= pbox->x1) &&
		(y2 >= pbox->y1) &&
		(x2 <  pbox->x2) &&
		(y2 <  pbox->y2))
	    {
	        pxAddLine(sp, &pxp, x2, y2, x2, y2);
		break;
	    }
	    else
		pbox++;
	}
    }
#endif

    pxPacketFlush(sp, &pxp);
}

/*
 * Draw dashed 1-pixel lines.
 */

void
#ifdef _POLYSEGMENT
pxPolySegmentD (pDrawable, pGC, nseg, pSeg)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nseg;
    xSegment	*pSeg;
#else
pxPolylinesD( pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr pGC;
    int mode;		/* Origin or Previous */
    int npt;		/* number of points */
    DDXPointPtr pptInit;
#endif
{
    int nboxInit;
    int nbox;
    BoxPtr pboxInit;
    BoxPtr pbox;
#ifndef _POLYSEGMENT
    DDXPointPtr ppt;	/* pointer to list of translated points */
#endif
    pxPacket pxp;

    unsigned int oc1;		/* outcode of point 1 */
    unsigned int oc2;		/* outcode of point 2 */

    int xorg, yorg;		/* origin of window */

    int adx;		/* abs values of dx and dy */
    int ady;
    int signdx;		/* sign of dx and dy */
    int signdy;
    int e, e1, e2;		/* bresenham error and increments */
    int len;			/* length of segment */
    int axis;			/* major axis */
    int octant;
    unsigned int bias = miGetZeroLineBias(pDrawable->pScreen);
    int x1, x2, y1, y2;
    RegionPtr cclip;
    unsigned char   *pDash;
    int		    dashOffset;
    int		    numInDashList;
    int		    dashIndex;
    int		    isDoubleDash;
    int		    unclippedlen;
    pxPrivGCPtr     gcPriv;
    pxScreenPrivPtr sp;
    u_int32_t       *pb;
#ifdef _POLYSEGMENT
    int dashIndexTmp;
    int dashOffsetTmp;
#endif

#ifdef _POLYSEGMENT
    PX_TRACE("pxPolySegmentD");
#else
    PX_TRACE("pxPolyLinesD");
#endif

    gcPriv = pxGetGCPrivate(pGC);
    sp = gcPriv->sp;

    pb = pxPacketStart(sp, &pxp, 5, 3);
    pb[0] = STAMP_CMD_LINES | STAMP_RGB_FLAT | STAMP_LW_PERPACKET;
    pb[1] = gcPriv->pmask;
    pb[2] = 0;
    pb[3] = gcPriv->umet;
    pb[4] = 4;

    cclip = cfbGetCompositeClip(pGC);
    pboxInit = REGION_RECTS(cclip);
    nboxInit = REGION_NUM_RECTS(cclip);

    /* compute initial dash values */
     
    pDash = (unsigned char *) pGC->dash;
    numInDashList = pGC->numInDashList;
    isDoubleDash = (pGC->lineStyle == LineDoubleDash);
    dashIndex = 0;
    dashOffset = 0;
    miStepDash ((int)pGC->dashOffset, &dashIndex, pDash,
		numInDashList, &dashOffset);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
#ifdef _POLYSEGMENT
    while (nseg--)
#else
    ppt = pptInit;
    x2 = ppt->x + xorg;
    y2 = ppt->y + yorg;
    while(--npt)
#endif
    {
	nbox = nboxInit;
	pbox = pboxInit;

#ifdef _POLYSEGMENT
	x1 = pSeg->x1 + xorg;
	y1 = pSeg->y1 + yorg;
	x2 = pSeg->x2 + xorg;
	y2 = pSeg->y2 + yorg;
	pSeg++;
#else
	x1 = x2;
	y1 = y2;
	++ppt;
	if (mode == CoordModePrevious)
	{
	    xorg = x1;
	    yorg = y1;
	}
	x2 = ppt->x + xorg;
	y2 = ppt->y + yorg;
#endif

	CalcLineDeltas(x1, y1, x2, y2, adx, ady, signdx, signdy, 1, 1, octant);

	if (adx > ady)
	{
	    axis = X_AXIS;
	    e1 = ady << 1;
	    e2 = e1 - (adx << 1);
	    e = e1 - adx;
	    unclippedlen = adx;
	}
	else
	{
	    axis = Y_AXIS;
	    e1 = adx << 1;
	    e2 = e1 - (ady << 1);
	    e = e1 - ady;
	    unclippedlen = ady;
	    SetYMajorOctant(octant);
	}

	FIXUP_ERROR(e, octant, bias);

	/* we have bresenham parameters and two points.
	   all we have to do now is clip and draw.
	*/

	while(nbox--)
	{
	    oc1 = 0;
	    oc2 = 0;
	    OUTCODES(oc1, x1, y1, pbox);
	    OUTCODES(oc2, x2, y2, pbox);
	    if ((oc1 | oc2) == 0)
	    {
#ifdef _POLYSEGMENT
		if (pGC->capStyle != CapNotLast)
		    unclippedlen++;
		dashIndexTmp = dashIndex;
		dashOffsetTmp = dashOffset;
		pxBresD(sp, &pxp, gcPriv,
		      &dashIndexTmp, pDash, numInDashList,
		      &dashOffsetTmp, isDoubleDash,
		      signdx, signdy, axis, x1, y1,
		      e, e1, e2, unclippedlen);
		break;
#else
		pxBresD(sp, &pxp, gcPriv,
		      &dashIndex, pDash, numInDashList,
		      &dashOffset, isDoubleDash,
		      signdx, signdy, axis, x1, y1,
		      e, e1, e2, unclippedlen);
		goto dontStep;
#endif
	    }
	    else if (oc1 & oc2)
	    {
		pbox++;
	    }
	    else /* have to clip */
	    {
		int new_x1 = x1, new_y1 = y1, new_x2 = x2, new_y2 = y2;
		int clip1 = 0, clip2 = 0;
		int clipdx, clipdy;
		int err;
		int dashIndexTmp, dashOffsetTmp;
		
		if (miZeroClipLine(pbox->x1, pbox->y1, pbox->x2-1,
				   pbox->y2-1,
				   &new_x1, &new_y1, &new_x2, &new_y2,
				   adx, ady, &clip1, &clip2,
				   octant, bias, oc1, oc2) == -1)
		{
		    pbox++;
		    continue;
		}

		dashIndexTmp = dashIndex;
		dashOffsetTmp = dashOffset;

		if (clip1)
		{
		    int dlen;
    
		    if (axis == X_AXIS)
			dlen = abs(new_x1 - x1);
		    else
			dlen = abs(new_y1 - y1);
		    miStepDash (dlen, &dashIndexTmp, pDash,
				numInDashList, &dashOffsetTmp);
		}
		
		if (axis == X_AXIS)
		    len = abs(new_x2 - new_x1);
		else
		    len = abs(new_y2 - new_y1);
#ifdef _POLYSEGMENT
		if (clip2 != 0 || pGC->capStyle != CapNotLast)
		    len++;
#else
		len += (clip2 != 0);
#endif
		if (len)
		{
		    /* unwind bresenham error term to first point */
		    if (clip1)
		    {
			clipdx = abs(new_x1 - x1);
			clipdy = abs(new_y1 - y1);
			if (axis == X_AXIS)
			    err = e+((clipdy*e2) + ((clipdx-clipdy)*e1));
			else
			    err = e+((clipdx*e2) + ((clipdy-clipdx)*e1));
		    }
		    else
			err = e;
		    pxBresD(sp, &pxp, gcPriv,
		        &dashIndexTmp, pDash, numInDashList,
			&dashOffsetTmp, isDoubleDash,
			signdx, signdy, axis, new_x1, new_y1,
			err, e1, e2, len);
		}
		pbox++;
	    }
	} /* while (nbox--) */
#ifndef _POLYSEGMENT
	/*
	 * walk the dash list around to the next line
	 */
	miStepDash (unclippedlen, &dashIndex, pDash,
		    numInDashList, &dashOffset);
dontStep:	;
#endif
    } /* while (nline--) */

#ifndef _POLYSEGMENT
    /* paint the last point if the end style isn't CapNotLast.
       (Assume that a projecting, butt, or round cap that is one
        pixel wide is the same as the single pixel of the endpoint.)
    */

    if ((pGC->capStyle != CapNotLast) &&
        ((dashIndex & 1) == 0 || isDoubleDash) &&
	((ppt->x + xorg != pptInit->x + pDrawable->x) ||
	 (ppt->y + yorg != pptInit->y + pDrawable->y) ||
	 (ppt == pptInit + 1)))
    {
	nbox = nboxInit;
	pbox = pboxInit;
	while (nbox--)
	{
	    if ((x2 >= pbox->x1) &&
		(y2 >= pbox->y1) &&
		(x2 <  pbox->x2) &&
		(y2 <  pbox->y2))
	    {
		if (dashIndex & 1)
		    pxAddLineC(sp, &pxp, x1, y2, x1, y1, gcPriv->fgFill);
		else
		    pxAddLineC(sp, &pxp, x2, y2, x2, y2, gcPriv->bgPixel);
		break;
	    }
	    else
		pbox++;
	}
    }
#endif

    pxPacketFlush(sp, &pxp);
}
