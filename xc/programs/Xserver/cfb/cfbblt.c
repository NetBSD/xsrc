/*
 * cfb copy area
 */

/*

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

Author: Keith Packard

*/
/* $XConsortium: cfbblt.c,v 1.13 94/04/17 20:28:44 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbblt.c,v 3.1.2.1 1998/12/13 14:12:03 dawes Exp $ */

/* 24-bit bug fixes: Peter Wainwright, 1998/11/28 */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"

#ifdef notdef /* XXX fails right now, walks off end of pixmaps */
#if defined (FAST_UNALIGNED_READS) && PSZ == 8
#define DO_UNALIGNED_BITBLT
#endif
#endif

#if defined(FAST_MEMCPY) && (MROP == Mcopy) && PSZ == 8
#define DO_MEMCPY
#endif

/* ................................................. */
/* SPECIAL CODE FOR 24 BITS      by Peter Wainwright */

#if PSZ == 24 && (MROP) == 0

/* The default macros are defined in mergerop.h, and none of them are
   really appropriate for what we want to do.

   There are two ways of fixing this: either define SLOW_24BIT_COPY
   to copy pixel by pixel, or (by default) use the following macros
   modified from mergerop.h

   MROP_SOLID and MROP_MASK are defined for each of the operations,
   i.e. each value of MROP.

   There are special cases for Mcopy, McopyInverted, Mxor, and Mor.
   There is a completely generic version for MROP=0, and a simplified
   generic version which works for (Mcopy|Mxor|MandReverse|Mor).

   However, the generic version does not work for the 24-bit case
   because the pixels cannot be packed exactly into a machine word (32
   bits).

   Alternative macros MROP_SOLID24 and MROP_MASK24 are provided for
   the 24-bit case. However, these each copy a single *pixel*, not a
   single machine word. They take an rvalue source pixel, an lvalue
   destination, and the pixel index. The latter is used to find the
   position of the pixel data within the two words *dst and *(dst+1).

   Further macros MROP_SOLID24P and MROP_MASK24P are used to copy from
   an lvalue source to an lvalue destination. MROP_PIXEL24 is used to
   assemble the source pixel from the adjacent words *src and
   *(src+1), and this is then split between the destination words
   using the non-P macros above.

   But we want to copy entire words for the sake of efficiency.
   Unfortunately if a plane mask is specified this must be shifted
   from one word to the next.  Fortunately the pattern repeats after 3
   words, so we unroll the planemask here and redefine MROP_SOLID
   and MROP_MASK. */

#define DeclareMergeRop24i() unsigned long   _ca1[3], _cx1[3], _ca2[3], _cx2[3];

#if	(BITMAP_BIT_ORDER == MSBFirst)
#define InitializeMergeRop24i(alu,pm) {		\
  unsigned long _pm;				\
  mergeRopPtr  _bits;				\
  int i;					\
  for (i = 0; i < 3; i++) {			\
    _pm = ((pm << (8+8*i)) | (pm >> (16-8*i)));	\
    _bits = &mergeRopBits[alu];			\
    _ca1[i] = _bits->ca1 &  _pm;		\
    _cx1[i] = _bits->cx1 | ~_pm;		\
    _ca2[i] = _bits->ca2 &  _pm;		\
    _cx2[i] = _bits->cx2 &  _pm;		\
  }						\
}
#else	/* (BITMAP_BIT_ORDER == LSBFirst) */
#define InitializeMergeRop24i(alu,pm) {		\
  unsigned long _pm;				\
  mergeRopPtr  _bits;				\
  int i;					\
  for (i = 0; i < 3; i++) {			\
    _pm = ((pm >> (8*i)) | (pm << (24-8*i)));	\
    _bits = &mergeRopBits[alu];			\
    _ca1[i] = _bits->ca1 &  _pm;		\
    _cx1[i] = _bits->cx1 | ~_pm;		\
    _ca2[i] = _bits->ca2 &  _pm;		\
    _cx2[i] = _bits->cx2 &  _pm;		\
  }						\
}
#endif	/* (BITMAP_BIT_ORDER == MSBFirst) */

#define DoMergeRop24i(src, dst, i)					\
((dst) & ((src) & _ca1[i] ^ _cx1[i]) ^ ((src) & _ca2[i] ^ _cx2[i]))

#define DoMaskMergeRop24i(src, dst, mask, i)							\
((dst) & (((src) & _ca1[i] ^ _cx1[i]) | ~(mask)) ^ (((src) & _ca2[i] ^ _cx2[i]) & (mask)))

#undef MROP_DECLARE
#define MROP_DECLARE()			DeclareMergeRop24i()
/* We can't put the arrays in registers */
#undef MROP_DECLARE_REG
#define MROP_DECLARE_REG()		DeclareMergeRop24i()
#undef MROP_INITIALIZE
#define MROP_INITIALIZE(alu, pm)	InitializeMergeRop24i(alu, pm)
#undef MROP_SOLID
#define MROP_SOLID(src, dst)		\
	DoMergeRop24i(src, dst, (&(dst)-pdstBase) % 3)
#undef MROP_MASK
#define MROP_MASK(src, dst, mask)	\
	DoMaskMergeRop24i(src, dst, mask, (&(dst)-pdstBase) % 3)

#endif /* MROP == 0 && PSZ == 24 */

/* ................................................. */

void
MROP_NAME(cfbDoBitblt)(pSrc, pDst, alu, prgnDst, pptSrc, planemask)
    DrawablePtr	    pSrc, pDst;
    int		    alu;
    RegionPtr	    prgnDst;
    DDXPointPtr	    pptSrc;
    unsigned long   planemask;
{
    unsigned long *psrcBase, *pdstBase;	
				/* start of src and dst bitmaps */
    int widthSrc, widthDst;	/* add to get to same position in next line */

    BoxPtr pbox;
    int nbox;

    BoxPtr pboxTmp, pboxNext, pboxBase, pboxNew1, pboxNew2;
				/* temporaries for shuffling rectangles */
    DDXPointPtr pptTmp, pptNew1, pptNew2;
				/* shuffling boxes entails shuffling the
				   source points too */
    int w, h;
    int xdir;			/* 1 = left right, -1 = right left/ */
    int ydir;			/* 1 = top down, -1 = bottom up */

    unsigned long *psrcLine, *pdstLine;	
				/* pointers to line with current src and dst */
    register unsigned long *psrc;/* pointer to current src longword */
    register unsigned long *pdst;/* pointer to current dst longword */

    MROP_DECLARE_REG()

				/* following used for looping through a line */
    unsigned long startmask, endmask;	/* masks for writing ends of dst */
    int nlMiddle;		/* whole longwords in dst */
    int xoffSrc, xoffDst;
    register int leftShift, rightShift;
    register unsigned long bits;
    register unsigned long bits1;
    register int nl;		/* temp copy of nlMiddle */

				/* place to store full source word */
    int nstart;			/* number of ragged bits at start of dst */
    int nend;			/* number of ragged bits at end of dst */
    int srcStartOver;		/* pulling nstart bits from src
				   overflows into the next word? */
    int careful;
    int tmpSrc;
#if PSZ == 24
#ifdef DO_MEMCPY
    int w2;
#endif
#endif

    MROP_INITIALIZE(alu,planemask);

    cfbGetLongWidthAndPointer (pSrc, widthSrc, psrcBase)

    cfbGetLongWidthAndPointer (pDst, widthDst, pdstBase)

    /* XXX we have to err on the side of safety when both are windows,
     * because we don't know if IncludeInferiors is being used.
     */
    careful = ((pSrc == pDst) ||
	       ((pSrc->type == DRAWABLE_WINDOW) &&
		(pDst->type == DRAWABLE_WINDOW)));

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    pboxNew1 = NULL;
    pptNew1 = NULL;
    pboxNew2 = NULL;
    pptNew2 = NULL;
    if (careful && (pptSrc->y < pbox->y1))
    {
        /* walk source botttom to top */
	ydir = -1;
	widthSrc = -widthSrc;
	widthDst = -widthDst;

	if (nbox > 1)
	{
	    /* keep ordering in each band, reverse order of bands */
	    pboxNew1 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    if(!pboxNew1)
		return;
	    pptNew1 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pptNew1)
	    {
	        DEALLOCATE_LOCAL(pboxNew1);
	        return;
	    }
	    pboxBase = pboxNext = pbox+nbox-1;
	    while (pboxBase >= pbox)
	    {
	        while ((pboxNext >= pbox) &&
		       (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
	        pboxTmp = pboxNext+1;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp <= pboxBase)
	        {
		    *pboxNew1++ = *pboxTmp++;
		    *pptNew1++ = *pptTmp++;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNew1 -= nbox;
	    pbox = pboxNew1;
	    pptNew1 -= nbox;
	    pptSrc = pptNew1;
        }
    }
    else
    {
	/* walk source top to bottom */
	ydir = 1;
    }

    if (careful && (pptSrc->x < pbox->x1))
    {
	/* walk source right to left */
        xdir = -1;

	if (nbox > 1)
	{
	    /* reverse order of rects in each band */
	    pboxNew2 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    pptNew2 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pboxNew2 || !pptNew2)
	    {
		if (pptNew2) DEALLOCATE_LOCAL(pptNew2);
		if (pboxNew2) DEALLOCATE_LOCAL(pboxNew2);
		if (pboxNew1)
		{
		    DEALLOCATE_LOCAL(pptNew1);
		    DEALLOCATE_LOCAL(pboxNew1);
		}
	        return;
	    }
	    pboxBase = pboxNext = pbox;
	    while (pboxBase < pbox+nbox)
	    {
	        while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
	        pboxTmp = pboxNext;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp != pboxBase)
	        {
		    *pboxNew2++ = *--pboxTmp;
		    *pptNew2++ = *--pptTmp;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNew2 -= nbox;
	    pbox = pboxNew2;
	    pptNew2 -= nbox;
	    pptSrc = pptNew2;
	}
    }
    else
    {
	/* walk source left to right */
        xdir = 1;
    }

    while(nbox--)
    {
	w = pbox->x2 - pbox->x1;
	h = pbox->y2 - pbox->y1;

#if PSZ == 24
#ifdef DO_MEMCPY
	w2 = w * 3;
#endif
#endif
	if (ydir == -1) /* start at last scanline of rectangle */
	{
	    psrcLine = psrcBase + ((pptSrc->y+h-1) * -widthSrc);
	    pdstLine = pdstBase + ((pbox->y2-1) * -widthDst);
	}
	else /* start at first scanline */
	{
	    psrcLine = psrcBase + (pptSrc->y * widthSrc);
	    pdstLine = pdstBase + (pbox->y1 * widthDst);
	}
#if PSZ == 24
	if (w == 1 && ((pbox->x1 & 3) == 0  ||  (pbox->x1 & 3) == 3))
#else
	if ((pbox->x1 & PIM) + w <= PPW)
#endif
	{
	    maskpartialbits (pbox->x1, w, endmask);
	    startmask = 0;
	    nlMiddle = 0;
	}
	else
	{
	    maskbits(pbox->x1, w, startmask, endmask, nlMiddle);
	}

#ifdef DO_MEMCPY
	/* If the src and dst scanline don't overlap, do forward case.  */

	if ((xdir == 1) || (pptSrc->y != pbox->y1)
		|| (pptSrc->x + w <= pbox->x1))
	{
#if PSZ == 24
	    char *psrc = (char *) psrcLine + (pptSrc->x * 3);
	    char *pdst = (char *) pdstLine + (pbox->x1 * 3);
#else
	    char *psrc = (char *) psrcLine + pptSrc->x;
	    char *pdst = (char *) pdstLine + pbox->x1;
#endif
	    while (h--)
	    {
#if PSZ == 24
	    	memcpy(pdst, psrc, w2);
#else
	    	memcpy(pdst, psrc, w);
#endif
		pdst += widthDst << PWSH;
		psrc += widthSrc << PWSH;
	    }
	}
#else /* ! DO_MEMCPY */
	if (xdir == 1)
	{
#if defined(SLOW_24BIT_COPY) && PSZ == 24 && MROP == 0
	  while (h--)
	    {
	      /* Unfortunately a word-by-word copy is difficult for 24-bit
		 pixmaps, since the planemask is in a different position
		 for each word (though the pattern repeats every 3
		 words). So instead we copy pixel by pixel.

		 We can accelerate this by unrolling the loop and copying
		 3 words at a time. */
	      register int i, si, sii, di;

	      for (i = 0, si = pptSrc->x, di = pbox->x1;
		   i < w;
		   i++, si++, di++) {
		psrc = psrcLine + ((si * 3) >> 2);
		pdst = pdstLine + ((di * 3) >> 2);
		sii = (si & 3);
		MROP_SOLID24P(psrc, pdst, sii, di);
	      }
	      pdstLine += widthDst;
	      psrcLine += widthSrc;
	    }
#else /* ! (defined(SLOW_24BIT_COPY) && PSZ == 24 && MROP == 0) */
#if PSZ == 24
	    /* Note: x is a pixel number; the byte offset is 3*x;
	       therefore the offset within a word is (3*x) & 3 ==
	       (4*x-x) & 3 == (-x) & 3.  The offsets therefore
	       DECREASE by 1 for each pixel.
	       Note: We are assuming 4 bytes per word here!
	    */
	    xoffSrc = (-pptSrc->x) & 3;
	    xoffDst = (-pbox->x1) & 3;
	    pdstLine += (pbox->x1 * 3) >> 2;
	    psrcLine += (pptSrc->x * 3) >> 2;
#else
	    xoffSrc = pptSrc->x & PIM;
	    xoffDst = pbox->x1 & PIM;
	    pdstLine += (pbox->x1 >> PWSH);
	    psrcLine += (pptSrc->x >> PWSH);
#endif
#ifdef DO_UNALIGNED_BITBLT
	    nl = xoffSrc - xoffDst;
	    psrcLine = (unsigned long *)
			(((unsigned char *) psrcLine) + nl);
#else
	    if (xoffSrc == xoffDst)
#endif
	    {
		while (h--)
		{
		    psrc = psrcLine;
		    pdst = pdstLine;
		    pdstLine += widthDst;
		    psrcLine += widthSrc;
		    if (startmask)
		    {
			*pdst = MROP_MASK(*psrc, *pdst, startmask);
			psrc++;
			pdst++;
		    }
		    nl = nlMiddle;

#ifdef LARGE_INSTRUCTION_CACHE
#ifdef FAST_CONSTANT_OFFSET_MODE

		    psrc += nl & (UNROLL-1);
		    pdst += nl & (UNROLL-1);

#define BodyOdd(n) pdst[-n] = MROP_SOLID (psrc[-n], pdst[-n]);
#define BodyEven(n) pdst[-n] = MROP_SOLID (psrc[-n], pdst[-n]);

#define LoopReset \
pdst += UNROLL; \
psrc += UNROLL;

#else

#define BodyOdd(n)  *pdst = MROP_SOLID (*psrc, *pdst); pdst++; psrc++;
#define BodyEven(n) BodyOdd(n)

#define LoopReset   ;

#endif
		    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
#ifdef NOTDEF
		    /* you'd think this would be faster --
		     * a single instruction instead of 6
		     * but measurements show it to be ~15% slower
		     */
		    while ((nl -= 6) >= 0)
		    {
			asm ("moveml %1+,#0x0c0f;moveml#0x0c0f,%0"
			     : "=m" (*(char *)pdst)
			     : "m" (*(char *)psrc)
			     : "d0", "d1", "d2", "d3",
			       "a2", "a3");
			pdst += 6;
		    }
		    nl += 6;
		    while (nl--)
			*pdst++ = *psrc++;
#endif
		    DuffL(nl, label1,
			    *pdst = MROP_SOLID (*psrc, *pdst);
			    pdst++; psrc++;)
#endif

		    if (endmask)
			*pdst = MROP_MASK(*psrc, *pdst, endmask);
		}
	    }
#ifndef DO_UNALIGNED_BITBLT
	    else
	    {
		if (xoffSrc > xoffDst)
		{
#if PSZ == 24
		    leftShift = (xoffSrc - xoffDst) << 3;
#else
#if PGSZ == 32
		    leftShift = (xoffSrc - xoffDst) << (5 - PWSH);
#else /* PGSZ == 64 */
		    leftShift = (xoffSrc - xoffDst) << (6 - PWSH);
#endif /* PGSZ */
#endif
		    rightShift = PGSZ - leftShift;
		}
		else
		{
#if PSZ == 24
		    rightShift = (xoffDst - xoffSrc) << 3;
#else
#if PGSZ == 32
		    rightShift = (xoffDst - xoffSrc) << (5 - PWSH);
#else /* PGSZ == 64 */
		    rightShift = (xoffDst - xoffSrc) << (6 - PWSH);
#endif /* PGSZ */
#endif
		    leftShift = PGSZ - rightShift;
		}
		while (h--)
		{
		    psrc = psrcLine;
		    pdst = pdstLine;
		    pdstLine += widthDst;
		    psrcLine += widthSrc;
		    bits = 0;
		    if (xoffSrc > xoffDst)
			bits = *psrc++;
		    if (startmask)
		    {
			bits1 = BitLeft(bits,leftShift);
			bits = *psrc++;
			bits1 |= BitRight(bits,rightShift);
			*pdst = MROP_MASK(bits1, *pdst, startmask);
			pdst++;
		    }
		    nl = nlMiddle;

#ifdef LARGE_INSTRUCTION_CACHE
		    bits1 = bits;

#ifdef FAST_CONSTANT_OFFSET_MODE

		    psrc += nl & (UNROLL-1);
		    pdst += nl & (UNROLL-1);

#define BodyOdd(n) \
bits = psrc[-n]; \
pdst[-n] = MROP_SOLID(BitLeft(bits1, leftShift) | BitRight(bits, rightShift), pdst[-n]);

#define BodyEven(n) \
bits1 = psrc[-n]; \
pdst[-n] = MROP_SOLID(BitLeft(bits, leftShift) | BitRight(bits1, rightShift), pdst[-n]);

#define LoopReset \
pdst += UNROLL; \
psrc += UNROLL;

#else

#define BodyOdd(n) \
bits = *psrc++; \
*pdst = MROP_SOLID(BitLeft(bits1, leftShift) | BitRight(bits, rightShift), *pdst); \
pdst++;
		   
#define BodyEven(n) \
bits1 = *psrc++; \
*pdst = MROP_SOLID(BitLeft(bits, leftShift) | BitRight(bits1, rightShift), *pdst); \
pdst++;

#define LoopReset   ;

#endif	/* !FAST_CONSTANT_OFFSET_MODE */

		    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
		    DuffL (nl,label2,
			bits1 = BitLeft(bits, leftShift);
			bits = *psrc++;
			*pdst = MROP_SOLID (bits1 | BitRight(bits, rightShift), *pdst);
			pdst++;
		    )
#endif

		    if (endmask)
		    {
			bits1 = BitLeft(bits, leftShift);
			if (BitLeft(endmask, rightShift))
			{
			    bits = *psrc;
			    bits1 |= BitRight(bits, rightShift);
			}
			*pdst = MROP_MASK (bits1, *pdst, endmask);
		    }
		}
	    }
#endif /* DO_UNALIGNED_BITBLT */
#endif /* defined(SLOW_24BIT_COPY) && PSZ == 24 && MROP == 0 */
	}
#endif /* ! DO_MEMCPY */
	else	/* xdir == -1 */
	{
#if defined(SLOW_24BIT_COPY) && PSZ == 24 && MROP == 0
	  while (h--)
	    {
	      /* Unfortunately a word-by-word copy is difficult for 24-bit
		 pixmaps, since the planemask is in a different position
		 for each word (though the pattern repeats every 3
		 words). So instead we copy pixel by pixel.

		 We can accelerate this by unrolling the loop and copying
		 3 words at a time. */
	      register int i, si, sii, di;

	      for (i = 0, si = pptSrc->x, di = pbox->x1;
		   i < w;
		   i++, si++, di++) {
		psrc = psrcLine + ((si * 3) >> 2);
		pdst = pdstLine + ((di * 3) >> 2);
		sii = (si & 3);
		MROP_SOLID24P(psrc, pdst, sii, di);
	      }
	      psrcLine += widthSrc;
	      pdstLine += widthDst;
	    }
#else /* ! (defined(SLOW_24BIT_COPY) && PSZ == 24 && MROP == 0) */
#if PSZ == 24
	    xoffSrc = (pptSrc->x + w) & 3;
	    xoffDst = pbox->x2 & 3;
	    pdstLine += ((pbox->x2 * 3 - 1) >> 2) + 1;
	    psrcLine += (((pptSrc->x+w) * 3 - 1) >> 2) + 1;
#else
	    xoffSrc = (pptSrc->x + w - 1) & PIM;
	    xoffDst = (pbox->x2 - 1) & PIM;
	    pdstLine += ((pbox->x2-1) >> PWSH) + 1;
	    psrcLine += ((pptSrc->x+w - 1) >> PWSH) + 1;
#endif
#ifdef DO_UNALIGNED_BITBLT
#if PSZ == 24
	    nl = xoffDst - xoffSrc;
#else
	    nl = xoffSrc - xoffDst;
#endif
	    psrcLine = (unsigned long *)
			(((unsigned char *) psrcLine) + nl);
#else
	    if (xoffSrc == xoffDst)
#endif
	    {
		while (h--)
		{
		    psrc = psrcLine;
		    pdst = pdstLine;
		    pdstLine += widthDst;
		    psrcLine += widthSrc;
		    if (endmask)
		    {
			pdst--;
			psrc--;
			*pdst = MROP_MASK (*psrc, *pdst, endmask);
		    }
		    nl = nlMiddle;

#ifdef LARGE_INSTRUCTION_CACHE
#ifdef FAST_CONSTANT_OFFSET_MODE
		    psrc -= nl & (UNROLL - 1);
		    pdst -= nl & (UNROLL - 1);

#define BodyOdd(n) pdst[n-1] = MROP_SOLID (psrc[n-1], pdst[n-1]);

#define BodyEven(n) BodyOdd(n)

#define LoopReset \
pdst -= UNROLL;\
psrc -= UNROLL;

#else

#define BodyOdd(n)  --pdst; --psrc; *pdst = MROP_SOLID(*psrc, *pdst);
#define BodyEven(n) BodyOdd(n)
#define LoopReset   ;

#endif
		    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
		    DuffL(nl,label3,
			 --pdst; --psrc; *pdst = MROP_SOLID (*psrc, *pdst);)
#endif

		    if (startmask)
		    {
			--pdst;
			--psrc;
			*pdst = MROP_MASK(*psrc, *pdst, startmask);
		    }
		}
	    }
#ifndef DO_UNALIGNED_BITBLT
	    else
	    {
		if (xoffDst > xoffSrc)
		{
#if PSZ == 24
		    leftShift = (xoffDst - xoffSrc) << 3;
		    rightShift = PGSZ - leftShift;
#else
#if PGSZ == 32
		    rightShift = (xoffDst - xoffSrc) << (5 - PWSH);
#else /* PGSZ == 64 */
		    rightShift = (xoffDst - xoffSrc) << (6 - PWSH);
#endif /* PGSZ */
		    leftShift = PGSZ - rightShift;
#endif
		}
		else
		{
#if PSZ == 24
		    rightShift = (xoffSrc - xoffDst) << 3;
		    leftShift = PGSZ - rightShift;
#else
#if PGSZ == 32
		    leftShift = (xoffSrc - xoffDst) << (5 - PWSH);
#else /* PGSZ == 64 */
		    leftShift = (xoffSrc - xoffDst) << (6 - PWSH);
#endif /* PGSZ */
		    rightShift = PGSZ - leftShift;
#endif
		}
		while (h--)
		{
		    psrc = psrcLine;
		    pdst = pdstLine;
		    pdstLine += widthDst;
		    psrcLine += widthSrc;
		    bits = 0;
#if PSZ == 24
		    if (xoffSrc > xoffDst)
#else
		    if (xoffDst > xoffSrc)
#endif
			bits = *--psrc;
		    if (endmask)
		    {
			bits1 = BitRight(bits, rightShift);
			bits = *--psrc;
			bits1 |= BitLeft(bits, leftShift);
			pdst--;
			*pdst = MROP_MASK(bits1, *pdst, endmask);
		    }
		    nl = nlMiddle;

#ifdef LARGE_INSTRUCTION_CACHE
		    bits1 = bits;
#ifdef FAST_CONSTANT_OFFSET_MODE
		    psrc -= nl & (UNROLL - 1);
		    pdst -= nl & (UNROLL - 1);

#define BodyOdd(n) \
bits = psrc[n-1]; \
pdst[n-1] = MROP_SOLID(BitRight(bits1, rightShift) | BitLeft(bits, leftShift),pdst[n-1]);

#define BodyEven(n) \
bits1 = psrc[n-1]; \
pdst[n-1] = MROP_SOLID(BitRight(bits, rightShift) | BitLeft(bits1, leftShift),pdst[n-1]);

#define LoopReset \
pdst -= UNROLL; \
psrc -= UNROLL;

#else

#define BodyOdd(n) \
bits = *--psrc; --pdst; \
*pdst = MROP_SOLID(BitRight(bits1, rightShift) | BitLeft(bits, leftShift),*pdst);

#define BodyEven(n) \
bits1 = *--psrc; --pdst; \
*pdst = MROP_SOLID(BitRight(bits, rightShift) | BitLeft(bits1, leftShift),*pdst);

#define LoopReset   ;

#endif

		    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
		    DuffL (nl, label4,
			bits1 = BitRight(bits, rightShift);
			bits = *--psrc;
			--pdst;
			*pdst = MROP_SOLID(bits1 | BitLeft(bits, leftShift),*pdst);
		    )
#endif

		    if (startmask)
		    {
			bits1 = BitRight(bits, rightShift);
			if (BitRight (startmask, leftShift))
			{
			    bits = *--psrc;
			    bits1 |= BitLeft(bits, leftShift);
			}
			--pdst;
			*pdst = MROP_MASK(bits1, *pdst, startmask);
		    }
		}
	    }
#endif
#endif /* defined(SLOW_24BIT_COPY) && PSZ == 24 && MROP == 0 */
	}
	pbox++;
	pptSrc++;
    }
    if (pboxNew2)
    {
	DEALLOCATE_LOCAL(pptNew2);
	DEALLOCATE_LOCAL(pboxNew2);
    }
    if (pboxNew1)
    {
	DEALLOCATE_LOCAL(pptNew1);
	DEALLOCATE_LOCAL(pboxNew1);
    }
}
