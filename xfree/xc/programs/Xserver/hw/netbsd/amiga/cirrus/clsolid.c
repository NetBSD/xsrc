/*
 * $XConsortium: cfbsolid.c,v 1.9 94/04/17 20:29:02 dpw Exp $
 *
Copyright (c) 1990  X Consortium

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include "amiga.h"

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "cfbrrop.h"

#include "mi.h"
#include "mispans.h"

#include <stdio.h>
#include "clstuff.h"
#if defined(FAST_CONSTANT_OFFSET_MODE) && (RROP != GXcopy)
# define Expand(left,right,leftAdjust) {\
    int part = nmiddle & 3; \
    int widthStep; \
    widthStep = widthDst - nmiddle - leftAdjust; \
    nmiddle >>= 2; \
    pdst = pdstRect; \
    while (h--) { \
	left \
	pdst += part; \
	switch (part) { \
	    RROP_UNROLL_CASE3(pdst) \
	} \
	m = nmiddle; \
	while (m) { \
	    pdst += 4; \
	    RROP_UNROLL_LOOP4(pdst,-4) \
	    m--; \
	} \
	right \
	pdst += widthStep; \
    } \
}
#else
# ifdef RROP_UNROLL
#  define Expand(left,right,leftAdjust) {\
    int part = nmiddle & RROP_UNROLL_MASK; \
    int widthStep; \
    widthStep = widthDst - nmiddle - leftAdjust; \
    nmiddle >>= RROP_UNROLL_SHIFT; \
    pdst = pdstRect; \
    while (h--) { \
	left \
	pdst += part; \
	switch (part) { \
	    RROP_UNROLL_CASE(pdst) \
	} \
	m = nmiddle; \
	while (m) { \
	    pdst += RROP_UNROLL; \
	    RROP_UNROLL_LOOP(pdst) \
	    m--; \
	} \
	right \
	pdst += widthStep; \
    } \
}

# else
#  define Expand(left, right, leftAdjust) { \
    while (h--) { \
	pdst = pdstRect; \
	left \
	m = nmiddle; \
	while (m--) {\
	    RROP_SOLID(pdst); \
	    pdst++; \
	} \
	right \
	pdstRect += widthDst; \
    } \
}
# endif
#endif

#include "amigaCL.h"

void
RROP_NAME(clFillRectSolid)(
    DrawablePtr	    pDrawable,
    GCPtr	    pGC,
    int		    nBox,
    BoxPtr	    pBox)
{
    register int    m;
    register unsigned long   *pdst;
    RROP_DECLARE
    register unsigned long   leftMask, rightMask;
    unsigned long   *pdstBase, *pdstRect;
    int		    nmiddle;
    int		    h;
    int		    w;
    int		    widthDst;
    cfbPrivGCPtr    devPriv;

    fbFd *inf = amigaInfo(pDrawable->pScreen);
    volatile unsigned char *regs = inf->regs;
    volatile unsigned char *fbaddr = inf->fb;
    unsigned short clwidth = (unsigned short)inf->info.gd_dwidth;
    unsigned short clheight = (unsigned short)inf->info.gd_dheight;
    unsigned int pat = ((clwidth*clheight)+8) & 0xfffffffb;/*align to 4-byte boundary*/
    volatile unsigned char *patptr = fbaddr+pat;
    int tmp=0;

    devPriv = cfbGetGCPrivate(pGC);

    cfbGetLongWidthAndPointer (pDrawable, widthDst, pdstBase);

    RROP_FETCH_GC(pGC);
    
#if RROP == GXcopy
    /* fill 8x8-bit pattern to solid 1s */
    for (tmp = 7; tmp >= 0;)
	 patptr[tmp--] = 0xff;
    InitCLBltPatCE(regs, clwidth, 0x0d, pat, 0x1c);	/* 0x1c = 50Mhz */
#endif

    for (; nBox; nBox--, pBox++)
    {
#if RROP == GXcopy
      if((pDrawable->type == DRAWABLE_WINDOW) && ((h = pBox->y2 - pBox->y1)>4)
	  && ((w = pBox->x2 - pBox->x1)>4) /*&& ((mask & 0xff) == 0xff)*/ ){
      unsigned short fg=(0x0000 | ((rrop_xor & 0xffff))),bg=0;
      CLBltPatCE(regs,w,h,clwidth,pBox->x1,pBox->y1,fg,bg);
	CLWaitBlt(regs);
      }else{
#endif
    	pdstRect = pdstBase + pBox->y1 * widthDst;
    	h = pBox->y2 - pBox->y1;
	w = pBox->x2 - pBox->x1;
#if PSZ == 8
	if (w == 1)
	{
	    register char    *pdstb = ((char *) pdstRect) + pBox->x1;
	    int	    incr = widthDst * PGSZB;

	    while (h--)
	    {
		RROP_SOLID (pdstb);
		pdstb += incr;
	    }
	}
	else
	{
#endif
	pdstRect += (pBox->x1 >> PWSH);
	if ((pBox->x1 & PIM) + w <= PPW)
	{
	    maskpartialbits(pBox->x1, w, leftMask);
	    pdst = pdstRect;
	    while (h--) {
		RROP_SOLID_MASK (pdst, leftMask);
		pdst += widthDst;
	    }
	}
	else
	{
	    maskbits (pBox->x1, w, leftMask, rightMask, nmiddle);
	    if (leftMask)
	    {
		if (rightMask)	/* left mask and right mask */
		{
		    Expand(RROP_SOLID_MASK (pdst, leftMask); pdst++;,
			   RROP_SOLID_MASK (pdst, rightMask);, 1)
		}
		else	/* left mask and no right mask */
		{
		    Expand(RROP_SOLID_MASK (pdst, leftMask); pdst++;,
			   ;, 1)
		}
	    }
	    else
	    {
		if (rightMask)	/* no left mask and right mask */
		{
		    Expand(;,
			   RROP_SOLID_MASK (pdst, rightMask);, 0)
		}
		else	/* no left mask and no right mask */
		{
		    Expand(;,
			    ;, 0)
		}
	    }
	}
#if PSZ == 8
	}
#endif
#if RROP == GXcopy
        }
#endif
    }
}
