/*
 * $XFree86: xc/programs/Xserver/fb/fbpict.h,v 1.3 2001/01/21 21:19:09 tsi Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _FBPICT_H_
#define _FBPICT_H_

#define FbIntMult(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )

#define FbGet8(v,i)   ((CARD16) (CARD8) ((v) >> i))

/*
 * There are two ways of handling alpha -- either as a single unified value or
 * a separate value for each component, hence each macro must have two
 * versions.  The unified alpha version has a 'U' at the end of the name,
 * the component version has a 'C'.  Similarly, functions which deal with
 * this difference will have two versions using the same convention.
 */

#define FbOverU(x,y,i,a,t) ((t) = FbIntMult(FbGet8(y,i),(a),(t)) + FbGet8(x,i),\
			   (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))

#define FbOverC(x,y,i,a,t) ((t) = FbIntMult(FbGet8(y,i),FbGet8(a,i),(t)) + FbGet8(x,i),\
			    (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))

#define FbInU(x,i,a,t) ((CARD32) FbIntMult(FbGet8(x,i),(a),(t)) << (i))

#define FbInC(x,i,a,t) ((CARD32) FbIntMult(FbGet8(x,i),FbGet8(a,i),(t)) << (i))

#define FbGen(x,y,i,ax,ay,t) ((t) = (FbIntMult(FbGet8(y,i),ay,(t)) + \
				     FbIntMult(FbGet8(x,i),ax,(t))),\
			      (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))

#define FbAdd(x,y,i,t)	((t) = FbGet8(x,i) + FbGet8(y,i), \
			 (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))


typedef void	(*CompositeFunc) (CARD8      op,
				  PicturePtr pSrc,
				  PicturePtr pMask,
				  PicturePtr pDst,
				  INT16      xSrc,
				  INT16      ySrc,
				  INT16      xMask,
				  INT16      yMask,
				  INT16      xDst,
				  INT16      yDst,
				  CARD16     width,
				  CARD16     height);

typedef CARD32 (*FbCompositeFetch)(FbBits *line, CARD32 offset);
typedef void (*FbCompositeStore) (FbBits *line, CARD32 offset, CARD32 value);

typedef struct _FbCompositeOperand {
    FbBits		*line;
    CARD32		offset;
    FbStride		stride;
    int			bpp;
    FbCompositeFetch	fetch;
    FbCompositeFetch	fetcha;
    FbCompositeStore	store;
} FbCompositeOperand;

typedef void (*FbCombineFunc) (FbCompositeOperand	*src,
			       FbCompositeOperand	*msk,
			       FbCompositeOperand	*dst);

/*
 * indexed by op
 */
extern FbCombineFunc	fbCombineFunc[];

typedef struct _FbAccessMap {
    CARD32		format;
    FbCompositeFetch	fetch;
    FbCompositeFetch	fetcha;
    FbCompositeStore	store;
} FbAccessMap;

/*
 * search on format
 */
extern FbAccessMap  fbAccessMap[];

/* fbcompose.c */

/*
 * All compositing operators *
 */

CARD32
FbCombineMask (FbCompositeOperand   *src,
	       FbCompositeOperand   *msk);

void
FbCombineClear (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst);

void
FbCombineSrc (FbCompositeOperand    *src,
	      FbCompositeOperand    *msk,
	      FbCompositeOperand    *dst);

void
FbCombineDst (FbCompositeOperand    *src,
	      FbCompositeOperand    *msk,
	      FbCompositeOperand    *dst);

void
FbCombineOver (FbCompositeOperand   *src,
	       FbCompositeOperand   *msk,
	       FbCompositeOperand   *dst);

void
FbCombineOverReverse (FbCompositeOperand    *src,
		      FbCompositeOperand    *msk,
		      FbCompositeOperand    *dst);

void
FbCombineIn (FbCompositeOperand	    *src,
	     FbCompositeOperand	    *msk,
	     FbCompositeOperand	    *dst);

void
FbCombineInReverse (FbCompositeOperand  *src,
		    FbCompositeOperand  *msk,
		    FbCompositeOperand  *dst);

void
FbCombineOut (FbCompositeOperand    *src,
	      FbCompositeOperand    *msk,
	      FbCompositeOperand    *dst);

void
FbCombineOutReverse (FbCompositeOperand *src,
		     FbCompositeOperand *msk,
		     FbCompositeOperand *dst);

void
FbCombineAtop (FbCompositeOperand   *src,
	       FbCompositeOperand   *msk,
	       FbCompositeOperand   *dst);

void
FbCombineAtopReverse (FbCompositeOperand    *src,
		      FbCompositeOperand    *msk,
		      FbCompositeOperand    *dst);

void
FbCombineXor (FbCompositeOperand    *src,
	      FbCompositeOperand    *msk,
	      FbCompositeOperand    *dst);

void
FbCombineAdd (FbCompositeOperand    *src,
	      FbCompositeOperand    *msk,
	      FbCompositeOperand    *dst);

void
FbCombineSaturate (FbCompositeOperand   *src,
		   FbCompositeOperand   *msk,
		   FbCompositeOperand   *dst);


/*
 * All fetch functions
 */

CARD32
fbFetch_a8r8g8b8 (FbBits *line, CARD32 offset);

CARD32
fbFetch_x8r8g8b8 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a8b8g8r8 (FbBits *line, CARD32 offset);

CARD32
fbFetch_x8b8g8r8 (FbBits *line, CARD32 offset);

CARD32
fbFetch_r8g8b8 (FbBits *line, CARD32 offset);

CARD32
fbFetch_b8g8r8 (FbBits *line, CARD32 offset);

CARD32
fbFetch_r5g6b5 (FbBits *line, CARD32 offset);

CARD32
fbFetch_b5g6r5 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a1r5g5b5 (FbBits *line, CARD32 offset);

CARD32
fbFetch_x1r5g5b5 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a1b5g5r5 (FbBits *line, CARD32 offset);

CARD32
fbFetch_x1b5g5r5 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a8 (FbBits *line, CARD32 offset);

CARD32
fbFetch_r3g3b2 (FbBits *line, CARD32 offset);

CARD32
fbFetch_b2g3r3 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a2r2g2b2 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a2b2g2r2 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a4 (FbBits *line, CARD32 offset);

CARD32
fbFetch_r1g2b1 (FbBits *line, CARD32 offset);

CARD32
fbFetch_b1g2r1 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a1r1g1b1 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a1b1g1r1 (FbBits *line, CARD32 offset);

CARD32
fbFetch_a1 (FbBits *line, CARD32 offset);

void
fbStore_a8r8g8b8 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_x8r8g8b8 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_a8b8g8r8 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_x8b8g8r8 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_r8g8b8 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_b8g8r8 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_r5g6b5 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_b5g6r5 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_a1r5g5b5 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_x1r5g5b5 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_a1b5g5r5 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_x1b5g5r5 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_a8 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_r3g3b2 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_b2g3r3 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_a2r2g2b2 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_a4 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_r1g2b1 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_b1g2r1 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_a1r1g1b1 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_a1b1g1r1 (FbBits *line, CARD32 offset, CARD32 value);

void
fbStore_a1 (FbBits *line, CARD32 offset, CARD32 value);

Bool
fbBuildCompositeOperand (PicturePtr	    pPict,
			 FbCompositeOperand *op,
			 INT16		    x,
			 INT16		    y);
void
fbCompositeGeneral (CARD8	op,
		    PicturePtr	pSrc,
		    PicturePtr	pMask,
		    PicturePtr	pDst,
		    INT16	xSrc,
		    INT16	ySrc,
		    INT16	xMask,
		    INT16	yMask,
		    INT16	xDst,
		    INT16	yDst,
		    CARD16	width,
		    CARD16	height);


/* fbpict.c */
CARD32
fbOver (CARD32 x, CARD32 y);

CARD32
fbIn (CARD32 x, CARD8 y);

void
fbCompositeSolidMask_nx8x8888 (CARD8      op,
			       PicturePtr pSrc,
			       PicturePtr pMask,
			       PicturePtr pDst,
			       INT16      xSrc,
			       INT16      ySrc,
			       INT16      xMask,
			       INT16      yMask,
			       INT16      xDst,
			       INT16      yDst,
			       CARD16     width,
			       CARD16     height);

void
fbCompositeSolidMask_nx8x0888 (CARD8      op,
			       PicturePtr pSrc,
			       PicturePtr pMask,
			       PicturePtr pDst,
			       INT16      xSrc,
			       INT16      ySrc,
			       INT16      xMask,
			       INT16      yMask,
			       INT16      xDst,
			       INT16      yDst,
			       CARD16     width,
			       CARD16     height);

void
fbCompositeSolidMask_nx8x0565 (CARD8      op,
			       PicturePtr pSrc,
			       PicturePtr pMask,
			       PicturePtr pDst,
			       INT16      xSrc,
			       INT16      ySrc,
			       INT16      xMask,
			       INT16      yMask,
			       INT16      xDst,
			       INT16      yDst,
			       CARD16     width,
			       CARD16     height);

void
fbCompositeSrc_8888x8888 (CARD8      op,
			  PicturePtr pSrc,
			  PicturePtr pMask,
			  PicturePtr pDst,
			  INT16      xSrc,
			  INT16      ySrc,
			  INT16      xMask,
			  INT16      yMask,
			  INT16      xDst,
			  INT16      yDst,
			  CARD16     width,
			  CARD16     height);

void
fbCompositeSrc_8888x0888 (CARD8      op,
			 PicturePtr pSrc,
			 PicturePtr pMask,
			 PicturePtr pDst,
			 INT16      xSrc,
			 INT16      ySrc,
			 INT16      xMask,
			 INT16      yMask,
			 INT16      xDst,
			 INT16      yDst,
			 CARD16     width,
			 CARD16     height);

void
fbCompositeSrc_8888x0565 (CARD8      op,
			  PicturePtr pSrc,
			  PicturePtr pMask,
			  PicturePtr pDst,
			  INT16      xSrc,
			  INT16      ySrc,
			  INT16      xMask,
			  INT16      yMask,
			  INT16      xDst,
			  INT16      yDst,
			  CARD16     width,
			  CARD16     height);

void
fbCompositeSrc_0565x0565 (CARD8      op,
			  PicturePtr pSrc,
			  PicturePtr pMask,
			  PicturePtr pDst,
			  INT16      xSrc,
			  INT16      ySrc,
			  INT16      xMask,
			  INT16      yMask,
			  INT16      xDst,
			  INT16      yDst,
			  CARD16     width,
			  CARD16     height);

void
fbComposite (CARD8      op,
	     PicturePtr pSrc,
	     PicturePtr pMask,
	     PicturePtr pDst,
	     INT16      xSrc,
	     INT16      ySrc,
	     INT16      xMask,
	     INT16      yMask,
	     INT16      xDst,
	     INT16      yDst,
	     CARD16     width,
	     CARD16     height);

#endif /* _FBPICT_H_ */
