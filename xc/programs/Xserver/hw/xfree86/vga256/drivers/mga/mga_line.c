/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mga_line.c,v 3.3 1996/10/18 15:03:48 dawes Exp $ */

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

/*
 * Modified for MGA by Dirk H. Hohndel (hohndel@XFree86.Org)
 */

#include "xf86.h"
#include "vga256.h"
#include "vga.h"
#include "miline.h"
#include "compiler.h"
#include "mga.h"
#include "mgareg.h"

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

void
mgaLine (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;		/* number of points */
    DDXPointPtr pptInit;
{
    int nboxInit;
    register int nbox;
    BoxPtr pboxInit;
    register BoxPtr pbox;
    register DDXPointPtr ppt;	/* pointer to list of translated points */

    unsigned int oc1;		/* outcode of point 1 */
    unsigned int oc2;		/* outcode of point 2 */

    unsigned long *addrl;	/* address of destination pixmap */
    int xorg, yorg;		/* origin of window */
    unsigned long pixelCol; 	/* colour of the pixel */

    register int y1, y2;
    register int x1, x2;
    RegionPtr cclip;
    cfbPrivGCPtr    devPriv;
    unsigned long   xor, and;
    int		    alu;

    if (!xf86VTSema || pDrawable->type != DRAWABLE_WINDOW ||
    (pGC->planemask & 0xFF) != 0xFF) 
    {
        vga256LineSS (pDrawable, pGC, mode, npt, pptInit);
	return;
    }

    addrl = 0;

    devPriv = cfbGetGCPrivate(pGC);
    cclip = devPriv->pCompositeClip;
    pboxInit = REGION_RECTS(cclip);
    nboxInit = REGION_NUM_RECTS(cclip);

    alu = devPriv->rop;
    xor = devPriv->xor;
    and = devPriv->and;
    xorg = pDrawable->x;
    yorg = pDrawable->y;

    ppt = pptInit;
    x2 = ppt->x + xorg;
    y2 = ppt->y + yorg;

    /* set the foreground color for future line drawing ops */

    /* grab the entire 32 bit unmasked pixel */	
    pixelCol = pGC->fgPixel;

    switch ( vgaBitsPerPixel)
    {
    case 8:
	pixelCol &= 0xff;
	pixelCol |= (pixelCol << 24) | (pixelCol << 16) | (pixelCol << 8);
    break;

    case 16:
	pixelCol &= 0xffff;
	pixelCol |= pixelCol << 16;
    break;

    case 24:
	pixelCol &= 0xffffff;
    break;

    case 32:
	pixelCol |= 0xff000000; /* highest alpha channel */
    break;
    }

    MGAWAITFIFOSLOTS(1);
    MGAREG(MGAREG_FCOL) = pixelCol;

    while(--npt)
    {
	nbox = nboxInit;
	pbox = pboxInit;

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

	{

	    while(nbox--)
	    {
		oc1 = 0;
		oc2 = 0;
		OUTCODES(oc1, x1, y1, pbox);
		OUTCODES(oc2, x2, y2, pbox);
		if (oc1 & oc2)
		{
		    pbox++;
		}
		else
		{
		    MGAWAITFIFOSLOTS(6);
		    OUTREG(MGAREG_CXBNDRY, (((pbox->x2 - 1) << 16) | pbox->x1));
		    OUTREG(MGAREG_YTOP, MGAScrnWidth * pbox->y1);
		    OUTREG(MGAREG_YBOT, MGAScrnWidth * pbox->y2);
		    OUTREG(MGAREG_XYSTRT, x1 | (y1 << 16));
		    OUTREG(MGAREG_XYEND,  x2 | (y2 << 16));
		    OUTREG(MGAREG_DWGCTL + MGAREG_EXEC, 0x040C4803);
		    pbox++;
		    if ((oc1 | oc2) == 0)
		        break;
		}
	    } /* while (nbox--) */
	} /* sloped line */
    } /* while (nline--) */


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
		MGAWAITFIFOSLOTS(6);
		OUTREG(MGAREG_CXBNDRY, (((pbox->x2 - 1) << 16) | pbox->x1)); 
		OUTREG(MGAREG_YTOP, MGAScrnWidth * pbox->y1);
		OUTREG(MGAREG_YBOT, MGAScrnWidth * pbox->y2);
		OUTREG(MGAREG_XYSTRT, x1 | (y1 << 16));
		OUTREG(MGAREG_XYEND,  x2 | (y2 << 16));
		OUTREG(MGAREG_DWGCTL + MGAREG_EXEC, 0x040c4803);
		break;
	    }
	    else
		pbox++;
	}
    }
    MGAWAITFREE();
}
