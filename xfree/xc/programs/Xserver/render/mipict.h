/*
 * $XFree86: xc/programs/Xserver/render/mipict.h,v 1.4 2000/12/05 03:13:32 keithp Exp $
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

#ifndef _MIPICT_H_
#define _MIPICT_H_

int
miCreatePicture (PicturePtr pPicture);

void
miDestroyPicture (PicturePtr pPicture);

void
miDestroyPictureClip (PicturePtr pPicture);

int
miChangePictureClip (PicturePtr    pPicture,
		     int	   type,
		     pointer	   value,
		     int	   n);

void
miChangePicture (PicturePtr pPicture,
		 Mask       mask);

void
miValidatePicture (PicturePtr pPicture,
		   Mask       mask);


Bool
miClipPicture (RegionPtr    pRegion,
	       PicturePtr   pPicture,
	       INT16	    xReg,
	       INT16	    yReg,
	       INT16	    xPict,
	       INT16	    yPict);

Bool
miComputeCompositeRegion (RegionPtr	pRegion,
			  PicturePtr	pSrc,
			  PicturePtr	pMask,
			  PicturePtr	pDst,
			  INT16		xSrc,
			  INT16		ySrc,
			  INT16		xMask,
			  INT16		yMask,
			  INT16		xDst,
			  INT16		yDst,
			  CARD16	width,
			  CARD16	height);

Bool
miPictureInit (ScreenPtr pScreen, PictFormatPtr formats, int nformats);

void
miGlyphExtents (int		nlist,
		GlyphListPtr	list,
		GlyphPtr	*glyphs,
		BoxPtr		extents);

void
miGlyphs (CARD8		op,
	  PicturePtr	pSrc,
	  PicturePtr	pDst,
	  PictFormatPtr	maskFormat,
	  INT16		xSrc,
	  INT16		ySrc,
	  int		nlist,
	  GlyphListPtr	list,
	  GlyphPtr	*glyphs);

void
miRenderColorToPixel (PictFormatPtr pPict,
		      xRenderColor  *color,
		      CARD32	    *pixel);

void
miRenderPixelToColor (PictFormatPtr pPict,
		      CARD32	    pixel,
		      xRenderColor  *color);

void
miCompositeRects (CARD8		op,
		  PicturePtr	pDst,
		  xRenderColor  *color,
		  int		nRect,
		  xRectangle    *rects);

#endif /* _MIPICT_H_ */
