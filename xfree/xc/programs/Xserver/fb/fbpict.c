/*
 * $XFree86: xc/programs/Xserver/fb/fbpict.c,v 1.5 2000/11/29 08:43:08 keithp Exp $
 *
 * Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#include "fb.h"
#include "picturestr.h"
#include "mipict.h"
#include "fbpict.h"

#define cvt8888to0565(s)    ((((s) >> 3) & 0x001f) | \
			     (((s) >> 5) & 0x07e0) | \
			     (((s) >> 8) & 0xf800))
#define cvt0565to8888(s)    (((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7)) | \
			     ((((s) << 5) & 0xfc00) | (((s) >> 1) & 0x300)) | \
			     ((((s) << 8) & 0xf80000) | (((s) << 3) & 0x70000)))

#if IMAGE_BYTE_ORDER == MSBFirst
#define Fetch24(a)  ((unsigned long) (a) & 1 ? \
		     ((*(a) << 16) | *((CARD16 *) ((a)+1))) : \
		     ((*((CARD16 *) (a)) << 8) | *((a)+2)))
#define Store24(a,v) ((unsigned long) (a) & 1 ? \
		      ((*(a) = (CARD8) ((v) >> 16)), \
		       (*((CARD16 *) ((a)+1)) = (CARD16) (v))) : \
		      ((*((CARD16 *) (a)) = (CARD16) ((v) >> 8)), \
		       (*((a)+2) = (CARD8) (v))))
#else
#define Fetch24(a)  ((unsigned long) (a) & 1 ? \
		     ((*(a)) | (*((CARD16 *) ((a)+1)) << 8)) : \
		     ((*((CARD16 *) (a))) | (*((a)+2) << 16)))
#define Store24(a,v) ((unsigned long) (a) & 1 ? \
		      ((*(a) = (CARD8) (v)), \
		       (*((CARD16 *) ((a)+1)) = (CARD16) ((v) >> 8))) : \
		      ((*((CARD16 *) (a)) = (CARD16) (v)),\
		       (*((a)+2) = (CARD8) ((v) >> 16))))
#endif
		      
CARD32
fbOver (CARD32 x, CARD32 y)
{
    CARD16  a = ~x >> 24;
    CARD16  t;
    CARD32  m,n,o,p;

    m = FbOverU(x,y,0,a,t);
    n = FbOverU(x,y,8,a,t);
    o = FbOverU(x,y,16,a,t);
    p = FbOverU(x,y,24,a,t);
    return m|n|o|p;
}

CARD32
fbIn (CARD32 x, CARD8 y)
{
    CARD16  a = y;
    CARD16  t;
    CARD32  m,n,o,p;

    m = FbInU(x,0,a,t);
    n = FbInU(x,8,a,t);
    o = FbInU(x,16,a,t);
    p = FbInU(x,24,a,t);
    return m|n|o|p;
}

/*
 * Naming convention:
 *
 *  opSRCxMASKxDST
 */

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
			       CARD16     height)
{
    CARD32	src, srca;
    CARD32	*dstLine, *dst, d, dstMask;
    CARD8	*maskLine, *mask, m;
    FbBits	*dstBits, *maskBits, *srcBits;
    FbStride	dstStride, maskStride, srcStride;
    int		dstBpp, maskBpp, srcBpp;
    CARD16	w;

    fbGetDrawable(pSrc->pDrawable, srcBits, srcStride, srcBpp);
    switch (srcBpp) {
    case 32:
	src = *(CARD32 *) srcBits;
	break;
    case 24:
	src = Fetch24 ((CARD8 *) srcBits);
	break;
    case 16:
	src = *(CARD16 *) srcBits;
	src = cvt0565to8888(src);
	break;
    }
    /* manage missing src alpha */
    if (pSrc->pFormat->direct.alphaMask == 0)
	src |= 0xff000000;
    dstMask = FbFullMask (pDst->pDrawable->depth);
    srca = src >> 24;
    if (srca == 0)
	return;
    
    fbGetDrawable(pDst->pDrawable, dstBits, dstStride, dstBpp);
    dstLine = (CARD32 *) dstBits;
    dstStride = dstStride * sizeof (FbBits) / sizeof (CARD32);
    dstLine += dstStride * yDst + xDst;
    
    fbGetDrawable(pMask->pDrawable, maskBits, maskStride, maskBpp);
    maskLine = (CARD8 *) maskBits;
    maskStride = maskStride * sizeof (FbBits) / sizeof (CARD8);
    maskLine += maskStride * yMask + xMask;
    
    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	mask = maskLine;
	maskLine += maskStride;
	w = width;

	while (w--)
	{
	    m = *mask++;
	    if (m == 0xff)
	    {
		if (srca == 0xff)
		    *dst = src & dstMask;
		else
		    *dst = fbOver (src, *dst) & dstMask;
	    }
	    else if (m)
	    {
		d = fbIn (src, m);
		*dst = fbOver (d, *dst) & dstMask;
	    }
	    dst++;
	}
    }
}

void
fbCompositeSolidMask_nx8888x8888C (CARD8      op,
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
				   CARD16     height)
{
    CARD32	src, srca;
    CARD32	*dstLine, *dst, d, dstMask;
    CARD32	*maskLine, *mask, ma;
    FbBits	*dstBits, *maskBits, *srcBits;
    FbStride	dstStride, maskStride, srcStride;
    int		dstBpp, maskBpp, srcBpp;
    CARD16	w;
    CARD32	m, n, o, p;

    fbGetDrawable(pSrc->pDrawable, srcBits, srcStride, srcBpp);
    switch (srcBpp) {
    case 32:
	src = *(CARD32 *) srcBits;
	break;
    case 24:
	src = Fetch24 ((CARD8 *) srcBits);
	break;
    case 16:
	src = *(CARD16 *) srcBits;
	src = cvt0565to8888(src);
	break;
    }
    /* manage missing src alpha */
    if (pSrc->pFormat->direct.alphaMask == 0)
	src |= 0xff000000;
    dstMask = FbFullMask (pDst->pDrawable->depth);
    srca = src >> 24;
    if (srca == 0)
	return;
    
    fbGetDrawable(pDst->pDrawable, dstBits, dstStride, dstBpp);
    dstLine = (CARD32 *) dstBits;
    dstStride = dstStride * sizeof (FbBits) / sizeof (CARD32);
    dstLine += dstStride * yDst + xDst;
    
    fbGetDrawable(pMask->pDrawable, maskBits, maskStride, maskBpp);
    maskLine = (CARD32 *) maskBits;
    maskStride = maskStride * sizeof (FbBits) / sizeof (CARD32);
    maskLine += maskStride * yMask + xMask;
    
    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	mask = maskLine;
	maskLine += maskStride;
	w = width;

	while (w--)
	{
	    ma = *mask++;
	    if (ma == 0xffffffff)
	    {
		if (srca == 0xff)
		    *dst = src & dstMask;
		else
		    *dst = fbOver (src, *dst) & dstMask;
	    }
	    else if (ma)
	    {
		d = *dst;
#define FbInOverC(src,srca,msk,dst,i,result) { \
    CARD16  __a = FbGet8(msk,i); \
    CARD32  __t, __ta; \
    CARD32  __i; \
    __t = FbIntMult (FbGet8(src,i), __a,__i); \
    __ta = (CARD8) ~FbIntMult (srca, __a,__i); \
    __t = __t + FbIntMult(FbGet8(dst,i),__ta,__i); \
    __t = (CARD32) (CARD8) (__t | (-(__t >> 8))); \
    result = __t << (i); \
}
		FbInOverC (src, srca, ma, d, 0, m);
		FbInOverC (src, srca, ma, d, 8, n);
		FbInOverC (src, srca, ma, d, 16, o);
		FbInOverC (src, srca, ma, d, 24, p);
		*dst = m|n|o|p;
	    }
	    dst++;
	}
    }
}

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
			       CARD16     height)
{
    CARD32	src, srca;
    CARD8	*dstLine, *dst;
    CARD32	d;
    CARD8	*maskLine, *mask, m;
    FbBits	*dstBits, *maskBits, *srcBits;
    FbStride	dstStride, maskStride, srcStride;
    int		dstBpp, maskBpp, srcBpp;
    CARD16	w;

    fbGetDrawable(pSrc->pDrawable, srcBits, srcStride, srcBpp);
    switch (srcBpp) {
    case 32:
	src = *(CARD32 *) srcBits;
	break;
    case 24:
	src = Fetch24 ((CARD8 *) srcBits);
	break;
    case 16:
	src = *(CARD16 *) srcBits;
	src = cvt0565to8888(src);
	break;
    }
    /* manage missing src alpha */
    if (pSrc->pFormat->direct.alphaMask == 0)
	src |= 0xff000000;
    srca = src >> 24;
    if (srca == 0)
	return;
    
    fbGetDrawable(pDst->pDrawable, dstBits, dstStride, dstBpp);
    dstLine = (CARD8 *) dstBits;
    dstStride = dstStride * sizeof (FbBits) / sizeof (CARD8);
    dstLine += dstStride * yDst + xDst * 3;
    
    fbGetDrawable(pMask->pDrawable, maskBits, maskStride, maskBpp);
    maskLine = (CARD8 *) maskBits;
    maskStride = maskStride * sizeof (FbBits) / sizeof (CARD8);
    maskLine += maskStride * yMask + xMask;
    
    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	mask = maskLine;
	maskLine += maskStride;
	w = width;

	while (w--)
	{
	    m = *mask++;
	    if (m == 0xff)
	    {
		if (srca == 0xff)
		    d = src;
		else
		{
		    d = Fetch24(dst);
		    d = fbOver (src, d);
		}
		Store24(dst,d);
	    }
	    else if (m)
	    {
		d = fbOver (fbIn(src,m), Fetch24(dst));
		Store24(dst,d);
	    }
	    dst += 3;
	}
    }
}

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
				  CARD16     height)
{
    CARD32	src, srca;
    CARD16	*dstLine, *dst;
    CARD32	d;
    CARD8	*maskLine, *mask, m;
    FbBits	*dstBits, *maskBits, *srcBits;
    FbStride	dstStride, maskStride, srcStride;
    int		dstBpp, maskBpp, srcBpp;
    CARD16	w;

    fbGetDrawable(pSrc->pDrawable, srcBits, srcStride, srcBpp);
    switch (srcBpp) {
    case 32:
	src = *(CARD32 *) srcBits;
	break;
    case 24:
	src = Fetch24 ((CARD8 *) srcBits);
	break;
    case 16:
	src = *(CARD16 *) srcBits;
	src = cvt0565to8888(src);
	break;
    }
    /* manage missing src alpha */
    if (pSrc->pFormat->direct.alphaMask == 0)
	src |= 0xff000000;
    srca = src >> 24;
    if (srca == 0)
	return;
    
    fbGetDrawable(pDst->pDrawable, dstBits, dstStride, dstBpp);
    dstLine = (CARD16 *) dstBits;
    dstStride = dstStride * sizeof (FbBits) / sizeof (CARD16);
    dstLine += dstStride * yDst + xDst;
    
    fbGetDrawable(pMask->pDrawable, maskBits, maskStride, maskBpp);
    maskLine = (CARD8 *) maskBits;
    maskStride = maskStride * sizeof (FbBits) / sizeof (CARD8);
    maskLine += maskStride * yMask + xMask;
    
    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	mask = maskLine;
	maskLine += maskStride;
	w = width;

	while (w--)
	{
	    m = *mask++;
	    if (m == 0xff)
	    {
		if (srca == 0xff)
		    d = src;
		else
		{
		    d = *dst;
		    d = fbOver (src, cvt0565to8888(d));
		}
		*dst = cvt8888to0565(d);
	    }
	    else if (m)
	    {
		d = *dst;
		d = fbOver (fbIn(src,m), cvt0565to8888(d));
		*dst = cvt8888to0565(d);
	    }
	    dst++;
	}
    }
}

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
			 CARD16     height)
{
    CARD32	*dstLine, *dst, dstMask;
    CARD32	*srcLine, *src, s;
    CARD8	a;
    FbBits	*dstBits, *srcBits;
    FbStride	dstStride, srcStride;
    int		dstBpp, srcBpp;
    CARD16	w;
    
    fbGetDrawable(pSrc->pDrawable, srcBits, srcStride, srcBpp);
    srcLine = (CARD32 *) srcBits;
    srcStride = srcStride * sizeof (FbBits) / sizeof (CARD32);
    srcLine += srcStride * ySrc + xSrc;

    dstMask = FbFullMask (pDst->pDrawable->depth);
    fbGetDrawable(pDst->pDrawable, dstBits, dstStride, dstBpp);
    dstLine = (CARD32 *) dstBits;
    dstStride = dstStride * sizeof (FbBits) / sizeof (CARD32);
    dstLine += dstStride * yDst + xDst;

    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	src = srcLine;
	srcLine += srcStride;
	w = width;

	while (w--)
	{
	    s = *src++;
	    a = s >> 24;
	    if (a == 0xff)
		*dst = s & dstMask;
	    else if (a)
		*dst = fbOver (s, *dst) & dstMask;
	    dst++;
	}
    }
}

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
			 CARD16     height)
{
    CARD8	*dstLine, *dst;
    CARD32	d;
    CARD32	*srcLine, *src, s;
    CARD8	a;
    FbBits	*dstBits, *srcBits;
    FbStride	dstStride, srcStride;
    int		dstBpp, srcBpp;
    CARD16	w;
    
    fbGetDrawable(pSrc->pDrawable, srcBits, srcStride, srcBpp);
    srcLine = (CARD32 *) srcBits;
    srcStride = srcStride * sizeof (FbBits) / sizeof (CARD32);
    srcLine += srcStride * ySrc + xSrc;

    fbGetDrawable(pDst->pDrawable, dstBits, dstStride, dstBpp);
    dstLine = (CARD8 *) dstBits;
    dstStride = dstStride * sizeof (FbBits) / sizeof (CARD8);
    dstLine += dstStride * yDst + xDst * 3;

    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	src = srcLine;
	srcLine += srcStride;
	w = width;

	while (w--)
	{
	    s = *src++;
	    a = s >> 24;
	    if (a)
	    {
		if (a == 0xff)
		    d = s;
		else
		    d = fbOver (s, Fetch24(dst));
		Store24(dst,d);
	    }
	    dst += 3;
	}
    }
}

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
			 CARD16     height)
{
    CARD16	*dstLine, *dst;
    CARD32	d;
    CARD32	*srcLine, *src, s;
    CARD8	a;
    FbBits	*dstBits, *srcBits;
    FbStride	dstStride, srcStride;
    int		dstBpp, srcBpp;
    CARD16	w;
    
    fbGetDrawable(pSrc->pDrawable, srcBits, srcStride, srcBpp);
    srcLine = (CARD32 *) srcBits;
    srcStride = srcStride * sizeof (FbBits) / sizeof (CARD32);
    srcLine += srcStride * ySrc + xSrc;

    fbGetDrawable(pDst->pDrawable, dstBits, dstStride, dstBpp);
    dstLine = (CARD16 *) dstBits;
    dstStride = dstStride * sizeof (FbBits) / sizeof (CARD16);
    dstLine += dstStride * yDst + xDst;

    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	src = srcLine;
	srcLine += srcStride;
	w = width;

	while (w--)
	{
	    s = *src++;
	    a = s >> 24;
	    if (a)
	    {
		if (a == 0xff)
		    d = s;
		else
		{
		    d = *dst;
		    d = fbOver (s, cvt0565to8888(d));
		}
		*dst = cvt8888to0565(d);
	    }
	    dst++;
	}
    }
}

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
			  CARD16     height)
{
    CARD16	*dstLine, *dst;
    CARD16	*srcLine, *src;
    FbBits	*dstBits, *srcBits;
    FbStride	dstStride, srcStride;
    int		dstBpp, srcBpp;
    CARD16	w;
    
    fbGetDrawable(pSrc->pDrawable, srcBits, srcStride, srcBpp);
    srcLine = (CARD16 *) srcBits;
    srcStride = srcStride * sizeof (FbBits) / sizeof (CARD16);
    srcLine += srcStride * ySrc + xSrc;

    fbGetDrawable(pDst->pDrawable, dstBits, dstStride, dstBpp);
    dstLine = (CARD16 *) dstBits;
    dstStride = dstStride * sizeof (FbBits) / sizeof (CARD16);
    dstLine += dstStride * yDst + xDst;

    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	src = srcLine;
	srcLine += srcStride;
	w = width;

	while (w--)
	    *dst++ = *src++;
    }
}

void
fbCompositeSrcAdd_8000x8000 (CARD8	op,
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
			     CARD16     height)
{
    CARD8	*dstLine, *dst;
    CARD8	*srcLine, *src;
    FbBits	*dstBits, *srcBits;
    FbStride	dstStride, srcStride;
    int		dstBpp, srcBpp;
    CARD8	w;
    CARD8	s, d;
    CARD16	t;
    
    fbGetDrawable(pSrc->pDrawable, srcBits, srcStride, srcBpp);
    srcLine = (CARD8 *) srcBits;
    srcStride = srcStride * sizeof (FbBits) / sizeof (CARD8);
    srcLine += srcStride * ySrc + xSrc;

    fbGetDrawable(pDst->pDrawable, dstBits, dstStride, dstBpp);
    dstLine = (CARD8 *) dstBits;
    dstStride = dstStride * sizeof (FbBits) / sizeof (CARD8);
    dstLine += dstStride * yDst + xDst;

    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	src = srcLine;
	srcLine += srcStride;
	w = width;

	while (w--)
	{
	    s = *src++;
	    if (s != 0xff)
	    {
		d = *dst;
		t = d + s;
		s = t | (0 - (t >> 8));
	    }
	    *dst++ = s;
	}
    }
}

# define mod(a,b)	((b) == 1 ? 0 : (a) >= 0 ? (a) % (b) : (b) - (-a) % (b))

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
	     CARD16     height)
{
    RegionRec	    region;
    int		    n;
    BoxPtr	    pbox;
    CompositeFunc   func;
    Bool	    srcRepeat = pSrc->repeat;
    Bool	    maskRepeat = FALSE;
    int		    x_msk, y_msk, x_src, y_src, x_dst, y_dst;
    int		    w, h, w_this, h_this;
    
    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;
    xSrc += pSrc->pDrawable->x;
    ySrc += pSrc->pDrawable->y;
    if (pMask)
    {
	xMask += pMask->pDrawable->x;
	yMask += pMask->pDrawable->y;
	maskRepeat = pMask->repeat;
    }
    
    if (!miComputeCompositeRegion (&region,
				   pSrc,
				   pMask,
				   pDst,
				   xSrc,
				   ySrc,
				   xMask,
				   yMask,
				   xDst,
				   yDst,
				   width,
				   height))
	return;
				   
    func = fbCompositeGeneral;
    switch (op) {
    case PictOpOver:
	if (pMask)
	{
	    if (srcRepeat && 
		pSrc->pDrawable->width == 1 &&
		pSrc->pDrawable->height == 1)
	    {
		srcRepeat = FALSE;
		if (PICT_FORMAT_COLOR(pSrc->format)) {
		    switch (pMask->format) {
		    case PICT_a8:
			switch (pDst->format) {
			case PICT_r5g6b5:
			case PICT_b5g6r5:
			    func = fbCompositeSolidMask_nx8x0565;
			    break;
			case PICT_r8g8b8:
			case PICT_b8g8r8:
			    func = fbCompositeSolidMask_nx8x0888;
			    break;
			case PICT_a8r8g8b8:
			case PICT_x8r8g8b8:
			case PICT_a8b8g8r8:
			case PICT_x8b8g8r8:
			    func = fbCompositeSolidMask_nx8x8888;
			    break;
			}
			break;
		    case PICT_a8r8g8b8:
			if (pMask->componentAlpha) {
			    switch (pDst->format) {
			    case PICT_a8r8g8b8:
			    case PICT_x8r8g8b8:
				func = fbCompositeSolidMask_nx8888x8888C;
				break;
			    }
			}
			break;
		    case PICT_a8b8g8r8:
			if (pMask->componentAlpha) {
			    switch (pDst->format) {
			    case PICT_a8b8g8r8:
			    case PICT_x8b8g8r8:
				func = fbCompositeSolidMask_nx8888x8888C;
				break;
			    }
			}
			break;
		    }
		}
	    }
	}
	else
	{
	    switch (pSrc->format) {
	    case PICT_a8r8g8b8:
	    case PICT_x8r8g8b8:
		switch (pDst->format) {
		case PICT_a8r8g8b8:
		case PICT_x8r8g8b8:
		    func = fbCompositeSrc_8888x8888;
		    break;
		case PICT_r8g8b8:
		    func = fbCompositeSrc_8888x0888;
		    break;
		case PICT_r5g6b5:
		    func = fbCompositeSrc_8888x0565;
		    break;
		}
		break;
	    case PICT_a8b8g8r8:
	    case PICT_x8b8g8r8:
		switch (pDst->format) {
		case PICT_a8b8g8r8:
		case PICT_x8b8g8r8:
		    func = fbCompositeSrc_8888x8888;
		    break;
		case PICT_b8g8r8:
		    func = fbCompositeSrc_8888x0888;
		    break;
		case PICT_b5g6r5:
		    func = fbCompositeSrc_8888x0565;
		    break;
		}
		break;
	    case PICT_r5g6b5:
		switch (pDst->format) {
		case PICT_r5g6b5:
		    func = fbCompositeSrc_0565x0565;
		    break;
		}
		break;
	    case PICT_b5g6r5:
		switch (pDst->format) {
		case PICT_b5g6r5:
		    func = fbCompositeSrc_0565x0565;
		    break;
		}
		break;
	    }
	}
	break;
    case PictOpAdd:
	if (pMask == 0)
	{
	    switch (pSrc->format) {
	    case PICT_a8:
		switch (pDst->format) {
		case PICT_a8:
		    func = fbCompositeSrcAdd_8000x8000;
		    break;
		}
	    }
	}
	break;
    }
    n = REGION_NUM_RECTS (&region);
    pbox = REGION_RECTS (&region);
    while (n--)
    {
	h = pbox->y2 - pbox->y1;
	y_src = pbox->y1 - yDst + ySrc;
	y_msk = pbox->y1 - yDst + yMask;
	y_dst = pbox->y1;
	while (h)
	{
	    h_this = h;
	    w = pbox->x2 - pbox->x1;
	    x_src = pbox->x1 - xDst + xSrc;
	    x_msk = pbox->x1 - xDst + xMask;
	    x_dst = pbox->x1;
	    if (maskRepeat)
	    {
		y_msk = mod (y_msk, pMask->pDrawable->height);
		if (h_this > pMask->pDrawable->height - y_msk)
		    h_this = pMask->pDrawable->height - y_msk;
	    }
	    if (srcRepeat)
	    {
		y_src = mod (y_src, pSrc->pDrawable->height);
		if (h_this > pSrc->pDrawable->height - y_src)
		    h_this = pSrc->pDrawable->height - y_src;
	    }
	    while (w)
	    {
		w_this = w;
		if (maskRepeat)
		{
		    x_msk = mod (x_msk, pMask->pDrawable->width);
		    if (w_this > pMask->pDrawable->width - x_msk)
			w_this = pMask->pDrawable->width - x_msk;
		}
		if (srcRepeat)
		{
		    x_src = mod (x_src, pSrc->pDrawable->width);
		    if (w_this > pSrc->pDrawable->width - x_src)
			w_this = pSrc->pDrawable->width - x_src;
		}
		(*func) (op, pSrc, pMask, pDst,
			 x_src, y_src, x_msk, y_msk, x_dst, y_dst, 
			 w_this, h_this);
		w -= w_this;
		x_src += w_this;
		x_msk += w_this;
		x_dst += w_this;
	    }
	    h -= h_this;
	    y_src += h_this;
	    y_msk += h_this;
	    y_dst += h_this;
	}
	pbox++;
    }
    REGION_UNINIT (pDst->pDrawable->pScreen, &region);
}

Bool
fbPictureInit (ScreenPtr pScreen, PictFormatPtr formats, int nformats)
{
    PictureScreenPtr    ps;

    if (!miPictureInit (pScreen, formats, nformats))
	return FALSE;
    ps = GetPictureScreen(pScreen);
    ps->Composite = fbComposite;
    ps->Glyphs = miGlyphs;
    ps->CompositeRects = miCompositeRects;
    return TRUE;
}
