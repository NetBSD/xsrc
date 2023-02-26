/*
 * Copyright ® 2001 Keith Packard
 *
 * Partly based on code that is Copyright ® The XFree86 Project Inc.
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Michel Dänzer <michel@tungstengraphics.com>
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif
#include "uxa-priv.h"
#include "uxa.h"
#include "mipict.h"

static CARD32
format_for_depth(int depth)
{
	switch (depth) {
	case 1: return PICT_a1;
	case 4: return PICT_a4;
	case 8: return PICT_a8;
	case 15: return PICT_x1r5g5b5;
	case 16: return PICT_r5g6b5;
	default:
	case 24: return PICT_x8r8g8b8;
	case 32: return PICT_a8r8g8b8;
	}
}

static void
uxa_fill_spans(DrawablePtr pDrawable, GCPtr pGC, int n,
	       DDXPointPtr ppt, int *pwidth, int fSorted)
{
	ScreenPtr screen = pDrawable->pScreen;
	uxa_screen_t *uxa_screen = uxa_get_screen(screen);
	RegionPtr pClip = fbGetCompositeClip(pGC);
	PixmapPtr dst_pixmap, src_pixmap = NULL;
	BoxPtr pextent, pbox;
	int nbox;
	int extentX1, extentX2, extentY1, extentY2;
	int fullX1, fullX2, fullY1;
	int partX1, partX2;
	int off_x, off_y;
	xRenderColor color;
	PictFormatPtr format;
	PicturePtr dst, src;
	int error;

	if (uxa_screen->swappedOut || uxa_screen->force_fallback)
		goto fallback;

	if (pGC->fillStyle != FillSolid)
		goto fallback;

	dst_pixmap = uxa_get_offscreen_pixmap(pDrawable, &off_x, &off_y);
	if (!dst_pixmap)
		goto fallback;

	if (pGC->alu != GXcopy || pGC->planemask != FB_ALLONES)
		goto solid;

	format = PictureMatchFormat(screen,
				    dst_pixmap->drawable.depth,
				    format_for_depth(dst_pixmap->drawable.depth));
	dst = CreatePicture(0, &dst_pixmap->drawable, format, 0, 0, serverClient, &error);
	if (!dst)
		goto solid;

	ValidatePicture(dst);

	uxa_get_rgba_from_pixel(pGC->fgPixel,
				&color.red,
				&color.green,
				&color.blue,
				&color.alpha,
				format_for_depth(dst_pixmap->drawable.depth));
	src = CreateSolidPicture(0, &color, &error);
	if (!src) {
		FreePicture(dst, 0);
		goto solid;
	}

	if (!uxa_screen->info->check_composite(PictOpSrc, src, NULL, dst, 0, 0)) {
		FreePicture(src, 0);
		FreePicture(dst, 0);
		goto solid;
	}

	if (!uxa_screen->info->check_composite_texture ||
	    !uxa_screen->info->check_composite_texture(screen, src)) {
		PicturePtr solid;
		int src_off_x, src_off_y;

		solid = uxa_acquire_solid(screen, src->pSourcePict);
		FreePicture(src, 0);

		src = solid;
		src_pixmap = uxa_get_offscreen_pixmap(src->pDrawable,
						      &src_off_x, &src_off_y);
		if (!src_pixmap) {
			FreePicture(src, 0);
			FreePicture(dst, 0);
			goto solid;
		}
	}

	if (!uxa_screen->info->prepare_composite(PictOpSrc, src, NULL, dst, src_pixmap, NULL, dst_pixmap)) {
		FreePicture(src, 0);
		FreePicture(dst, 0);
		goto solid;
	}

	pextent = REGION_EXTENTS(pGC->screen, pClip);
	extentX1 = pextent->x1;
	extentY1 = pextent->y1;
	extentX2 = pextent->x2;
	extentY2 = pextent->y2;
	while (n--) {
		fullX1 = ppt->x;
		fullY1 = ppt->y;
		fullX2 = fullX1 + (int)*pwidth;
		ppt++;
		pwidth++;

		if (fullY1 < extentY1 || extentY2 <= fullY1)
			continue;

		if (fullX1 < extentX1)
			fullX1 = extentX1;

		if (fullX2 > extentX2)
			fullX2 = extentX2;

		if (fullX1 >= fullX2)
			continue;

		nbox = REGION_NUM_RECTS(pClip);
		if (nbox == 1) {
			uxa_screen->info->composite(dst_pixmap,
						    0, 0, 0, 0,
						    fullX1 + off_x,
						    fullY1 + off_y,
						    fullX2 - fullX1, 1);
		} else {
			pbox = REGION_RECTS(pClip);
			while (nbox--) {
				if (pbox->y1 > fullY1)
					break;

				if (pbox->y1 <= fullY1) {
					partX1 = pbox->x1;
					if (partX1 < fullX1)
						partX1 = fullX1;

					partX2 = pbox->x2;
					if (partX2 > fullX2)
						partX2 = fullX2;

					if (partX2 > partX1) {
						uxa_screen->info->composite(dst_pixmap,
									    0, 0, 0, 0,
									    partX1 + off_x,
									    fullY1 + off_y,
									    partX2 - partX1, 1);
					}
				}
				pbox++;
			}
		}
	}

	uxa_screen->info->done_composite(dst_pixmap);
	FreePicture(src, 0);
	FreePicture(dst, 0);
	return;

solid:
	if (uxa_screen->info->check_solid &&
	    !uxa_screen->info->check_solid(pDrawable, pGC->alu, pGC->planemask))
		goto fallback;

	if (!(*uxa_screen->info->prepare_solid) (dst_pixmap,
						 pGC->alu,
						 pGC->planemask,
						 pGC->fgPixel))
		goto fallback;

	pextent = REGION_EXTENTS(pGC->screen, pClip);
	extentX1 = pextent->x1;
	extentY1 = pextent->y1;
	extentX2 = pextent->x2;
	extentY2 = pextent->y2;
	while (n--) {
		fullX1 = ppt->x;
		fullY1 = ppt->y;
		fullX2 = fullX1 + (int)*pwidth;
		ppt++;
		pwidth++;

		if (fullY1 < extentY1 || extentY2 <= fullY1)
			continue;

		if (fullX1 < extentX1)
			fullX1 = extentX1;

		if (fullX2 > extentX2)
			fullX2 = extentX2;

		if (fullX1 >= fullX2)
			continue;

		nbox = REGION_NUM_RECTS(pClip);
		if (nbox == 1) {
			(*uxa_screen->info->solid) (dst_pixmap,
						    fullX1 + off_x,
						    fullY1 + off_y,
						    fullX2 + off_x,
						    fullY1 + 1 + off_y);
		} else {
			pbox = REGION_RECTS(pClip);
			while (nbox--) {
				if (pbox->y1 <= fullY1 && fullY1 < pbox->y2) {
					partX1 = pbox->x1;
					if (partX1 < fullX1)
						partX1 = fullX1;
					partX2 = pbox->x2;
					if (partX2 > fullX2)
						partX2 = fullX2;
					if (partX2 > partX1) {
						(*uxa_screen->info->
						 solid) (dst_pixmap,
							 partX1 + off_x,
							 fullY1 + off_y,
							 partX2 + off_x,
							 fullY1 + 1 + off_y);
					}
				}
				pbox++;
			}
		}
	}
	(*uxa_screen->info->done_solid) (dst_pixmap);

	return;

fallback:
	uxa_check_fill_spans(pDrawable, pGC, n, ppt, pwidth, fSorted);
}

static Bool
uxa_do_put_image(DrawablePtr pDrawable, GCPtr pGC, int depth, int x, int y,
		 int w, int h, int format, char *bits, int src_stride)
{
	uxa_screen_t *uxa_screen = uxa_get_screen(pDrawable->pScreen);
	PixmapPtr pPix;
	RegionPtr pClip;
	BoxPtr pbox;
	int nbox;
	int xoff, yoff;
	int bpp = pDrawable->bitsPerPixel;

	/* Don't bother with under 8bpp, XYPixmaps. */
	if (format != ZPixmap || bpp < 8)
		return FALSE;

	if (uxa_screen->swappedOut || uxa_screen->force_fallback)
		return FALSE;

	if (!uxa_screen->info->put_image)
		return FALSE;

	/* Only accelerate copies: no rop or planemask. */
	if (!UXA_PM_IS_SOLID(pDrawable, pGC->planemask) || pGC->alu != GXcopy)
		return FALSE;

	pPix = uxa_get_offscreen_pixmap(pDrawable, &xoff, &yoff);
	if (!pPix)
		return FALSE;

	x += pDrawable->x;
	y += pDrawable->y;

	pClip = fbGetCompositeClip(pGC);
	for (nbox = REGION_NUM_RECTS(pClip),
	     pbox = REGION_RECTS(pClip); nbox--; pbox++) {
		int x1 = x;
		int y1 = y;
		int x2 = x + w;
		int y2 = y + h;
		char *src;
		Bool ok;

		if (x1 < pbox->x1)
			x1 = pbox->x1;
		if (y1 < pbox->y1)
			y1 = pbox->y1;
		if (x2 > pbox->x2)
			x2 = pbox->x2;
		if (y2 > pbox->y2)
			y2 = pbox->y2;
		if (x1 >= x2 || y1 >= y2)
			continue;

		src = bits + (y1 - y) * src_stride + (x1 - x) * (bpp / 8);
		ok = uxa_screen->info->put_image(pPix, x1 + xoff, y1 + yoff,
						 x2 - x1, y2 - y1, src,
						 src_stride);
		/* If we fail to accelerate the upload, fall back to using
		 * unaccelerated fb calls.
		 */
		if (!ok) {
			FbStip *dst;
			FbStride dst_stride;
			int dstBpp;
			int dstXoff, dstYoff;

			if (!uxa_prepare_access(pDrawable, NULL, UXA_ACCESS_RW))
				return FALSE;

			fbGetStipDrawable(pDrawable, dst, dst_stride, dstBpp,
					  dstXoff, dstYoff);

			fbBltStip((FbStip *) bits +
				  (y1 - y) * (src_stride / sizeof(FbStip)),
				  src_stride / sizeof(FbStip),
				  (x1 - x) * dstBpp,
				  dst + (y1 + dstYoff) * dst_stride, dst_stride,
				  (x1 + dstXoff) * dstBpp, (x2 - x1) * dstBpp,
				  y2 - y1, GXcopy, FB_ALLONES, dstBpp);

			uxa_finish_access(pDrawable);
		}
	}


	return TRUE;
}

static void
uxa_put_image(DrawablePtr pDrawable, GCPtr pGC, int depth, int x, int y,
	      int w, int h, int leftPad, int format, char *bits)
{
	if (!uxa_do_put_image(pDrawable, pGC, depth, x, y, w, h, format, bits,
			      PixmapBytePad(w, pDrawable->depth))) {
		uxa_check_put_image(pDrawable, pGC, depth, x, y, w, h, leftPad,
				    format, bits);
	}
}

static Bool inline
uxa_copy_n_to_n_two_dir(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable,
			GCPtr pGC, BoxPtr pbox, int nbox, int dx, int dy)
{
	uxa_screen_t *uxa_screen = uxa_get_screen(pDstDrawable->pScreen);
	PixmapPtr pSrcPixmap, pDstPixmap;
	int src_off_x, src_off_y, dst_off_x, dst_off_y;
	int dirsetup;

	/* Need to get both pixmaps to call the driver routines */
	pSrcPixmap =
	    uxa_get_offscreen_pixmap(pSrcDrawable, &src_off_x, &src_off_y);
	pDstPixmap =
	    uxa_get_offscreen_pixmap(pDstDrawable, &dst_off_x, &dst_off_y);
	if (!pSrcPixmap || !pDstPixmap)
		return FALSE;

	/*
	 * Now the case of a chip that only supports xdir = ydir = 1 or
	 * xdir = ydir = -1, but we have xdir != ydir.
	 */
	dirsetup = 0;		/* No direction set up yet. */
	for (; nbox; pbox++, nbox--) {
		if (dx >= 0 && (src_off_y + pbox->y1 + dy) != pbox->y1) {
			/* Do a xdir = ydir = -1 blit instead. */
			if (dirsetup != -1) {
				if (dirsetup != 0)
					uxa_screen->info->done_copy(pDstPixmap);
				dirsetup = -1;
				if (!(*uxa_screen->info->prepare_copy)
				    (pSrcPixmap, pDstPixmap, -1, -1,
				     pGC ? pGC->alu : GXcopy,
				     pGC ? pGC->planemask : FB_ALLONES))
					return FALSE;
			}
			(*uxa_screen->info->copy) (pDstPixmap,
						   src_off_x + pbox->x1 + dx,
						   src_off_y + pbox->y1 + dy,
						   dst_off_x + pbox->x1,
						   dst_off_y + pbox->y1,
						   pbox->x2 - pbox->x1,
						   pbox->y2 - pbox->y1);
		} else if (dx < 0 && (src_off_y + pbox->y1 + dy) != pbox->y1) {
			/* Do a xdir = ydir = 1 blit instead. */
			if (dirsetup != 1) {
				if (dirsetup != 0)
					uxa_screen->info->done_copy(pDstPixmap);
				dirsetup = 1;
				if (!(*uxa_screen->info->prepare_copy)
				    (pSrcPixmap, pDstPixmap, 1, 1,
				     pGC ? pGC->alu : GXcopy,
				     pGC ? pGC->planemask : FB_ALLONES))
					return FALSE;
			}
			(*uxa_screen->info->copy) (pDstPixmap,
						   src_off_x + pbox->x1 + dx,
						   src_off_y + pbox->y1 + dy,
						   dst_off_x + pbox->x1,
						   dst_off_y + pbox->y1,
						   pbox->x2 - pbox->x1,
						   pbox->y2 - pbox->y1);
		} else if (dx >= 0) {
			/*
			 * xdir = 1, ydir = -1.
			 * Perform line-by-line xdir = ydir = 1 blits, going up.
			 */
			int i;
			if (dirsetup != 1) {
				if (dirsetup != 0)
					uxa_screen->info->done_copy(pDstPixmap);
				dirsetup = 1;
				if (!(*uxa_screen->info->prepare_copy)
				    (pSrcPixmap, pDstPixmap, 1, 1,
				     pGC ? pGC->alu : GXcopy,
				     pGC ? pGC->planemask : FB_ALLONES))
					return FALSE;
			}
			for (i = pbox->y2 - pbox->y1 - 1; i >= 0; i--)
				(*uxa_screen->info->copy) (pDstPixmap,
							   src_off_x +
							   pbox->x1 + dx,
							   src_off_y +
							   pbox->y1 + dy + i,
							   dst_off_x + pbox->x1,
							   dst_off_y +
							   pbox->y1 + i,
							   pbox->x2 - pbox->x1,
							   1);
		} else {
			/*
			 * xdir = -1, ydir = 1.
			 * Perform line-by-line xdir = ydir = -1 blits,
			 * going down.
			 */
			int i;
			if (dirsetup != -1) {
				if (dirsetup != 0)
					uxa_screen->info->done_copy(pDstPixmap);
				dirsetup = -1;
				if (!(*uxa_screen->info->prepare_copy)
				    (pSrcPixmap, pDstPixmap, -1, -1,
				     pGC ? pGC->alu : GXcopy,
				     pGC ? pGC->planemask : FB_ALLONES))
					return FALSE;
			}
			for (i = 0; i < pbox->y2 - pbox->y1; i++)
				(*uxa_screen->info->copy) (pDstPixmap,
							   src_off_x +
							   pbox->x1 + dx,
							   src_off_y +
							   pbox->y1 + dy + i,
							   dst_off_x + pbox->x1,
							   dst_off_y +
							   pbox->y1 + i,
							   pbox->x2 - pbox->x1,
							   1);
		}
	}
	if (dirsetup != 0)
		uxa_screen->info->done_copy(pDstPixmap);
	return TRUE;
}

void
uxa_copy_n_to_n(DrawablePtr pSrcDrawable,
		DrawablePtr pDstDrawable,
		GCPtr pGC,
		BoxPtr pbox,
		int nbox,
		int dx,
		int dy,
		Bool reverse, Bool upsidedown, Pixel bitplane, void *closure)
{
	ScreenPtr screen = pDstDrawable->pScreen;
	uxa_screen_t *uxa_screen = uxa_get_screen(screen);
	int src_off_x, src_off_y;
	int dst_off_x, dst_off_y;
	PixmapPtr pSrcPixmap, pDstPixmap;
	RegionRec src_region;
	RegionRec dst_region;
	
	pSrcPixmap = uxa_get_drawable_pixmap(pSrcDrawable);
	pDstPixmap = uxa_get_drawable_pixmap(pDstDrawable);
	if (!pSrcPixmap || !pDstPixmap)
		goto fallback;

	if (uxa_screen->info->check_copy &&
	    !uxa_screen->info->check_copy(pSrcPixmap, pDstPixmap,
					  pGC ? pGC->alu : GXcopy,
					  pGC ? pGC->planemask : FB_ALLONES))
		goto fallback;

	uxa_get_drawable_deltas(pSrcDrawable, pSrcPixmap, &src_off_x,
				&src_off_y);
	uxa_get_drawable_deltas(pDstDrawable, pDstPixmap, &dst_off_x,
				&dst_off_y);

	/* Mixed directions must be handled specially if the card is lame */
	if ((uxa_screen->info->flags & UXA_TWO_BITBLT_DIRECTIONS) &&
	    reverse != upsidedown) {
		if (uxa_copy_n_to_n_two_dir
		    (pSrcDrawable, pDstDrawable, pGC, pbox, nbox, dx, dy))
			return;
		goto fallback;
	}

	if (!uxa_pixmap_is_offscreen(pDstPixmap))
	    goto fallback;

	if (uxa_pixmap_is_offscreen(pSrcPixmap)) {
	    if (!(*uxa_screen->info->prepare_copy) (pSrcPixmap, pDstPixmap,
						reverse ? -1 : 1,
						upsidedown ? -1 : 1,
						pGC ? pGC->alu : GXcopy,
						pGC ? pGC->
						planemask : FB_ALLONES))
		goto fallback;

	    while (nbox--) {
		(*uxa_screen->info->copy) (pDstPixmap,
					   pbox->x1 + dx + src_off_x,
					   pbox->y1 + dy + src_off_y,
					   pbox->x1 + dst_off_x,
					   pbox->y1 + dst_off_y,
					   pbox->x2 - pbox->x1,
					   pbox->y2 - pbox->y1);
		pbox++;
	    }

	    (*uxa_screen->info->done_copy) (pDstPixmap);
	} else {
	    int stride, bpp;
	    char *src;

	    if (!uxa_screen->info->put_image)
		goto fallback;

	    /* Don't bother with under 8bpp, XYPixmaps. */
	    bpp = pSrcPixmap->drawable.bitsPerPixel;
	    if (bpp != pDstDrawable->bitsPerPixel || bpp < 8)
		goto fallback;

	    /* Only accelerate copies: no rop or planemask. */
	    if (pGC && (!UXA_PM_IS_SOLID(pSrcDrawable, pGC->planemask) || pGC->alu != GXcopy))
		goto fallback;

	    src = pSrcPixmap->devPrivate.ptr;
	    stride = pSrcPixmap->devKind;
	    bpp /= 8;
	    while (nbox--) {
		if (!uxa_screen->info->put_image(pDstPixmap,
						 pbox->x1 + dst_off_x,
						 pbox->y1 + dst_off_y,
						 pbox->x2 - pbox->x1,
						 pbox->y2 - pbox->y1,
						 (char *) src +
						 (pbox->y1 + dy + src_off_y) * stride +
						 (pbox->x1 + dx + src_off_x) * bpp,
						 stride))
		    goto fallback;

		pbox++;
	    }
	}

	return;

fallback:
#if 0
	ErrorF ("falling back: %d boxes\n", nbox);
#endif

	pixman_region_init_rects (&dst_region, pbox, nbox);
	REGION_INIT (NULL, &src_region, (BoxPtr)NULL, 0);
	REGION_COPY (NULL, &src_region, &dst_region);
	REGION_TRANSLATE (NULL, &src_region, dx, dy);

	UXA_FALLBACK(("from %p to %p (%c,%c)\n", pSrcDrawable, pDstDrawable,
		      uxa_drawable_location(pSrcDrawable),
		      uxa_drawable_location(pDstDrawable)));
	if (uxa_prepare_access(pDstDrawable, &dst_region, UXA_ACCESS_RW))
	{
	    if (uxa_prepare_access(pSrcDrawable, &src_region, UXA_ACCESS_RO))
	    {
		fbCopyNtoN(pSrcDrawable, pDstDrawable, pGC, pbox, nbox,
			   dx, dy, reverse, upsidedown, bitplane,
			   closure);

		uxa_finish_access(pSrcDrawable);
	    }

	    uxa_finish_access(pDstDrawable);
	}

	REGION_UNINIT (NULL, &src_region);
	REGION_UNINIT (NULL, &dst_region);
}

RegionPtr
uxa_copy_area(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, GCPtr pGC,
	      int srcx, int srcy, int width, int height, int dstx, int dsty)
{
	uxa_screen_t *uxa_screen = uxa_get_screen(pDstDrawable->pScreen);

	if (uxa_screen->swappedOut || uxa_screen->force_fallback) {
		return uxa_check_copy_area(pSrcDrawable, pDstDrawable, pGC,
					   srcx, srcy, width, height, dstx,
					   dsty);
	}

	return miDoCopy(pSrcDrawable, pDstDrawable, pGC,
			srcx, srcy, width, height,
			dstx, dsty, uxa_copy_n_to_n, 0, NULL);
}

static void
uxa_poly_point(DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
	       DDXPointPtr ppt)
{
	int i;
	xRectangle *prect;

	/* If we can't reuse the current GC as is, don't bother accelerating the
	 * points.
	 */
	if (pGC->fillStyle != FillSolid) {
		uxa_check_poly_point(pDrawable, pGC, mode, npt, ppt);
		return;
	}

	prect = malloc(sizeof(xRectangle) * npt);
	if (!prect)
		return;
	for (i = 0; i < npt; i++) {
		prect[i].x = ppt[i].x;
		prect[i].y = ppt[i].y;
		if (i > 0 && mode == CoordModePrevious) {
			prect[i].x += prect[i - 1].x;
			prect[i].y += prect[i - 1].y;
		}
		prect[i].width = 1;
		prect[i].height = 1;
	}
	pGC->ops->PolyFillRect(pDrawable, pGC, npt, prect);
	free(prect);
}

/**
 * uxa_poly_lines() checks if it can accelerate the lines as a group of
 * horizontal or vertical lines (rectangles), and uses existing rectangle fill
 * acceleration if so.
 */
static void
uxa_poly_lines(DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
	       DDXPointPtr ppt)
{
	xRectangle *prect;
	int x1, x2, y1, y2;
	int i;

	/* Don't try to do wide lines or non-solid fill style. */
	if (pGC->lineWidth != 0 || pGC->lineStyle != LineSolid ||
	    pGC->fillStyle != FillSolid) {
		uxa_check_poly_lines(pDrawable, pGC, mode, npt, ppt);
		return;
	}

	prect = malloc(sizeof(xRectangle) * (npt - 1));
	if (!prect)
		return;
	x1 = ppt[0].x;
	y1 = ppt[0].y;
	/* If we have any non-horizontal/vertical, fall back. */
	for (i = 0; i < npt - 1; i++) {
		if (mode == CoordModePrevious) {
			x2 = x1 + ppt[i + 1].x;
			y2 = y1 + ppt[i + 1].y;
		} else {
			x2 = ppt[i + 1].x;
			y2 = ppt[i + 1].y;
		}

		if (x1 != x2 && y1 != y2) {
			free(prect);
			uxa_check_poly_lines(pDrawable, pGC, mode, npt, ppt);
			return;
		}

		if (x1 < x2) {
			prect[i].x = x1;
			prect[i].width = x2 - x1 + 1;
		} else {
			prect[i].x = x2;
			prect[i].width = x1 - x2 + 1;
		}
		if (y1 < y2) {
			prect[i].y = y1;
			prect[i].height = y2 - y1 + 1;
		} else {
			prect[i].y = y2;
			prect[i].height = y1 - y2 + 1;
		}

		x1 = x2;
		y1 = y2;
	}
	pGC->ops->PolyFillRect(pDrawable, pGC, npt - 1, prect);
	free(prect);
}

/**
 * uxa_poly_segment() checks if it can accelerate the lines as a group of
 * horizontal or vertical lines (rectangles), and uses existing rectangle fill
 * acceleration if so.
 */
static void
uxa_poly_segment(DrawablePtr pDrawable, GCPtr pGC, int nseg, xSegment * pSeg)
{
	xRectangle *prect;
	int i;

	/* Don't try to do wide lines or non-solid fill style. */
	if (pGC->lineWidth != 0 || pGC->lineStyle != LineSolid ||
	    pGC->fillStyle != FillSolid) {
		uxa_check_poly_segment(pDrawable, pGC, nseg, pSeg);
		return;
	}

	/* If we have any non-horizontal/vertical, fall back. */
	for (i = 0; i < nseg; i++) {
		if (pSeg[i].x1 != pSeg[i].x2 && pSeg[i].y1 != pSeg[i].y2) {
			uxa_check_poly_segment(pDrawable, pGC, nseg, pSeg);
			return;
		}
	}

	prect = malloc(sizeof(xRectangle) * nseg);
	if (!prect)
		return;
	for (i = 0; i < nseg; i++) {
		if (pSeg[i].x1 < pSeg[i].x2) {
			prect[i].x = pSeg[i].x1;
			prect[i].width = pSeg[i].x2 - pSeg[i].x1 + 1;
		} else {
			prect[i].x = pSeg[i].x2;
			prect[i].width = pSeg[i].x1 - pSeg[i].x2 + 1;
		}
		if (pSeg[i].y1 < pSeg[i].y2) {
			prect[i].y = pSeg[i].y1;
			prect[i].height = pSeg[i].y2 - pSeg[i].y1 + 1;
		} else {
			prect[i].y = pSeg[i].y2;
			prect[i].height = pSeg[i].y1 - pSeg[i].y2 + 1;
		}

		/* don't paint last pixel */
		if (pGC->capStyle == CapNotLast) {
			if (prect[i].width == 1)
				prect[i].height--;
			else
				prect[i].width--;
		}
	}
	pGC->ops->PolyFillRect(pDrawable, pGC, nseg, prect);
	free(prect);
}

static Bool uxa_fill_region_solid(DrawablePtr pDrawable, RegionPtr pRegion,
				  Pixel pixel, CARD32 planemask, CARD32 alu);

static void
uxa_poly_fill_rect(DrawablePtr pDrawable,
		   GCPtr pGC, int nrect, xRectangle * prect)
{
	uxa_screen_t *uxa_screen = uxa_get_screen(pDrawable->pScreen);
	RegionPtr pClip = fbGetCompositeClip(pGC);
	PixmapPtr pPixmap;
	register BoxPtr pbox;
	BoxPtr pextent;
	int extentX1, extentX2, extentY1, extentY2;
	int fullX1, fullX2, fullY1, fullY2;
	int partX1, partX2, partY1, partY2;
	int xoff, yoff;
	int xorg, yorg;
	int n;
	RegionPtr pReg = RECTS_TO_REGION(pScreen, nrect, prect, CT_UNSORTED);

	/* Compute intersection of rects and clip region */
	REGION_TRANSLATE(pScreen, pReg, pDrawable->x, pDrawable->y);
	REGION_INTERSECT(pScreen, pReg, pClip, pReg);

	if (!REGION_NUM_RECTS(pReg))
		goto out;

	if (uxa_screen->swappedOut || uxa_screen->force_fallback)
		goto fallback;

	pPixmap = uxa_get_offscreen_pixmap (pDrawable, &xoff, &yoff);
	if (!pPixmap)
		goto fallback;

	/* For ROPs where overlaps don't matter, convert rectangles to region
	 * and call uxa_fill_region_{solid,tiled}.
	 */
	if ((pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled) &&
	    (nrect == 1 || pGC->alu == GXcopy || pGC->alu == GXclear ||
	     pGC->alu == GXnoop || pGC->alu == GXcopyInverted ||
	     pGC->alu == GXset)) {
		if (((pGC->fillStyle == FillSolid || pGC->tileIsPixel) &&
		     uxa_fill_region_solid(pDrawable, pReg,
					   pGC->fillStyle ==
					   FillSolid ? pGC->fgPixel : pGC->tile.
					   pixel, pGC->planemask, pGC->alu))
		    || (pGC->fillStyle == FillTiled && !pGC->tileIsPixel
			&& uxa_fill_region_tiled(pDrawable, pReg,
						 pGC->tile.pixmap, &pGC->patOrg,
						 pGC->planemask, pGC->alu))) {
			goto out;
		}
	}

	if (pGC->fillStyle != FillSolid &&
	    !(pGC->tileIsPixel && pGC->fillStyle == FillTiled)) {
		goto fallback;
	}

	if (uxa_screen->info->check_solid &&
	    !uxa_screen->info->check_solid(pDrawable, pGC->alu, pGC->planemask)) {
		goto fallback;
	}

	if (!(*uxa_screen->info->prepare_solid) (pPixmap,
						 pGC->alu,
						 pGC->planemask,
						 pGC->fgPixel)) {
fallback:
		uxa_check_poly_fill_rect(pDrawable, pGC, nrect, prect);
		goto out;
	}

	xorg = pDrawable->x;
	yorg = pDrawable->y;

	pextent = REGION_EXTENTS(pGC->pScreen, pClip);
	extentX1 = pextent->x1;
	extentY1 = pextent->y1;
	extentX2 = pextent->x2;
	extentY2 = pextent->y2;
	while (nrect--) {
		fullX1 = prect->x + xorg;
		fullY1 = prect->y + yorg;
		fullX2 = fullX1 + (int)prect->width;
		fullY2 = fullY1 + (int)prect->height;
		prect++;

		if (fullX1 < extentX1)
			fullX1 = extentX1;

		if (fullY1 < extentY1)
			fullY1 = extentY1;

		if (fullX2 > extentX2)
			fullX2 = extentX2;

		if (fullY2 > extentY2)
			fullY2 = extentY2;

		if ((fullX1 >= fullX2) || (fullY1 >= fullY2))
			continue;
		n = REGION_NUM_RECTS(pClip);
		if (n == 1) {
			(*uxa_screen->info->solid) (pPixmap,
						    fullX1 + xoff,
						    fullY1 + yoff,
						    fullX2 + xoff,
						    fullY2 + yoff);
		} else {
			pbox = REGION_RECTS(pClip);
			/*
			 * clip the rectangle to each box in the clip region
			 * this is logically equivalent to calling Intersect(),
			 * but rectangles may overlap each other here.
			 */
			while (n--) {
				partX1 = pbox->x1;
				if (partX1 < fullX1)
					partX1 = fullX1;
				partY1 = pbox->y1;
				if (partY1 < fullY1)
					partY1 = fullY1;
				partX2 = pbox->x2;
				if (partX2 > fullX2)
					partX2 = fullX2;
				partY2 = pbox->y2;
				if (partY2 > fullY2)
					partY2 = fullY2;

				pbox++;

				if (partX1 < partX2 && partY1 < partY2) {
					(*uxa_screen->info->solid) (pPixmap,
								    partX1 +
								    xoff,
								    partY1 +
								    yoff,
								    partX2 +
								    xoff,
								    partY2 +
								    yoff);
				}
			}
		}
	}
	(*uxa_screen->info->done_solid) (pPixmap);

out:
	REGION_UNINIT(pScreen, pReg);
	REGION_DESTROY(pScreen, pReg);
}

GCOps uxa_ops = {
	uxa_fill_spans,
	uxa_check_set_spans,
	uxa_put_image,
	uxa_copy_area,
	uxa_check_copy_plane,
	uxa_poly_point,
	uxa_poly_lines,
	uxa_poly_segment,
	miPolyRectangle,
	uxa_check_poly_arc,
	miFillPolygon,
	uxa_poly_fill_rect,
	miPolyFillArc,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	uxa_check_image_glyph_blt,
	uxa_check_poly_glyph_blt,
	uxa_check_push_pixels,
};

void uxa_copy_window(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
	RegionRec rgnDst;
	int dx, dy;
	PixmapPtr pPixmap = (*pWin->drawable.pScreen->GetWindowPixmap) (pWin);

	dx = ptOldOrg.x - pWin->drawable.x;
	dy = ptOldOrg.y - pWin->drawable.y;
	REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);

	REGION_INIT(pWin->drawable.pScreen, &rgnDst, NullBox, 0);

	REGION_INTERSECT(pWin->drawable.pScreen, &rgnDst, &pWin->borderClip,
			 prgnSrc);
#ifdef COMPOSITE
	if (pPixmap->screen_x || pPixmap->screen_y)
		REGION_TRANSLATE(pWin->drawable.pScreen, &rgnDst,
				 -pPixmap->screen_x, -pPixmap->screen_y);
#endif

	miCopyRegion(&pPixmap->drawable, &pPixmap->drawable,
		     NULL, &rgnDst, dx, dy, uxa_copy_n_to_n, 0, NULL);

	REGION_UNINIT(pWin->drawable.pScreen, &rgnDst);
}

static Bool
uxa_fill_region_solid(DrawablePtr pDrawable,
		      RegionPtr pRegion,
		      Pixel pixel, CARD32 planemask, CARD32 alu)
{
	ScreenPtr screen = pDrawable->pScreen;
	uxa_screen_t *uxa_screen = uxa_get_screen(screen);
	PixmapPtr pixmap;
	int xoff, yoff;
	int nbox;
	BoxPtr pBox, extents;
	Bool ret = FALSE;

	pixmap = uxa_get_offscreen_pixmap(pDrawable, &xoff, &yoff);
	if (!pixmap)
		return FALSE;

	REGION_TRANSLATE(screen, pRegion, xoff, yoff);

	nbox = REGION_NUM_RECTS(pRegion);
	pBox = REGION_RECTS(pRegion);
	extents = REGION_EXTENTS(screen, pRegion);

	/* Using GEM, the relocation costs outweigh the advantages of the blitter */
	if (nbox == 1 || (alu != GXcopy && alu != GXclear) || planemask != FB_ALLONES) {
try_solid:
		if (uxa_screen->info->check_solid &&
		    !uxa_screen->info->check_solid(&pixmap->drawable, alu, planemask))
			goto err;

		if (!uxa_screen->info->prepare_solid(pixmap, alu, planemask, pixel))
			goto err;

		while (nbox--) {
			uxa_screen->info->solid(pixmap,
						pBox->x1, pBox->y1,
						pBox->x2, pBox->y2);
			pBox++;
		}

		uxa_screen->info->done_solid(pixmap);
	} else {
		PicturePtr dst, src;
		PixmapPtr src_pixmap = NULL;
		xRenderColor color;
		int error;

		dst = CreatePicture(0, &pixmap->drawable,
				    PictureMatchFormat(screen,
						       pixmap->drawable.depth,
						       format_for_depth(pixmap->drawable.depth)),
				    0, 0, serverClient, &error);
		if (!dst)
			goto err;

		ValidatePicture(dst);

		uxa_get_rgba_from_pixel(pixel,
					&color.red,
					&color.green,
					&color.blue,
					&color.alpha,
					format_for_depth(pixmap->drawable.depth));
		src = CreateSolidPicture(0, &color, &error);
		if (!src) {
			FreePicture(dst, 0);
			goto err;
		}

		if (!uxa_screen->info->check_composite(PictOpSrc, src, NULL, dst,
						       extents->x2 - extents->x1,
						       extents->y2 - extents->y1)) {
			FreePicture(src, 0);
			FreePicture(dst, 0);
			goto try_solid;
		}

		if (!uxa_screen->info->check_composite_texture ||
		    !uxa_screen->info->check_composite_texture(screen, src)) {
			PicturePtr solid;
			int src_off_x, src_off_y;

			solid = uxa_acquire_solid(screen, src->pSourcePict);
			FreePicture(src, 0);

			src = solid;
			src_pixmap = uxa_get_offscreen_pixmap(src->pDrawable,
							      &src_off_x, &src_off_y);
			if (!src_pixmap) {
				FreePicture(src, 0);
				FreePicture(dst, 0);
				goto err;
			}
		}

		if (!uxa_screen->info->prepare_composite(PictOpSrc, src, NULL, dst, src_pixmap, NULL, pixmap)) {
			FreePicture(src, 0);
			FreePicture(dst, 0);
			goto err;
		}

		while (nbox--) {
			uxa_screen->info->composite(pixmap,
						    0, 0, 0, 0,
						    pBox->x1,
						    pBox->y1,
						    pBox->x2 - pBox->x1,
						    pBox->y2 - pBox->y1);
			pBox++;
		}

		uxa_screen->info->done_composite(pixmap);
		FreePicture(src, 0);
		FreePicture(dst, 0);
	}

	ret = TRUE;

err:
	REGION_TRANSLATE(screen, pRegion, -xoff, -yoff);
	return ret;
}

/* Try to do an accelerated tile of the pTile into pRegion of pDrawable.
 * Based on fbFillRegionTiled(), fbTile().
 */
Bool
uxa_fill_region_tiled(DrawablePtr pDrawable,
		      RegionPtr pRegion,
		      PixmapPtr pTile,
		      DDXPointPtr pPatOrg, CARD32 planemask, CARD32 alu)
{
	uxa_screen_t *uxa_screen = uxa_get_screen(pDrawable->pScreen);
	PixmapPtr pPixmap;
	int xoff, yoff;
	int tileWidth, tileHeight;
	int nbox = REGION_NUM_RECTS(pRegion);
	BoxPtr pBox = REGION_RECTS(pRegion);
	Bool ret = FALSE;
	int i;
	tileWidth = pTile->drawable.width;
	tileHeight = pTile->drawable.height;

	/* If we're filling with a solid color, grab it out and go to
	 * FillRegionsolid, saving numerous copies.
	 */
	if (tileWidth == 1 && tileHeight == 1)
		return uxa_fill_region_solid(pDrawable, pRegion,
					     uxa_get_pixmap_first_pixel(pTile),
					     planemask, alu);

	pPixmap = uxa_get_offscreen_pixmap(pDrawable, &xoff, &yoff);
	if (!pPixmap || !uxa_pixmap_is_offscreen(pTile))
		goto out;

	if (uxa_screen->info->check_copy &&
	    !uxa_screen->info->check_copy(pTile, pPixmap, alu, planemask))
		return FALSE;


	if ((*uxa_screen->info->prepare_copy) (pTile, pPixmap, 1, 1, alu,
					       planemask)) {
		if (xoff || yoff)
			REGION_TRANSLATE(pScreen, pRegion, xoff, yoff);

		  for (i = 0; i < nbox; i++) {
			int height = pBox[i].y2 - pBox[i].y1;
			int dstY = pBox[i].y1;
			int tileY;

			if (alu == GXcopy)
			    height = min(height, tileHeight);

			modulus(dstY - yoff - pDrawable->y - pPatOrg->y,
				tileHeight, tileY);

			while (height > 0) {
				int width = pBox[i].x2 - pBox[i].x1;
				int dstX = pBox[i].x1;
				int tileX;
				int h = tileHeight - tileY;

				if (alu == GXcopy)
				    width = min(width, tileWidth);
				if (h > height)
					h = height;
				height -= h;

				modulus(dstX - xoff - pDrawable->x - pPatOrg->x,
					tileWidth, tileX);

				while (width > 0) {
					int w = tileWidth - tileX;
					if (w > width)
						w = width;
					width -= w;

					(*uxa_screen->info->copy) (pPixmap,
								   tileX, tileY,
								   dstX, dstY,
								   w, h);
					dstX += w;
					tileX = 0;
				}
				dstY += h;
				tileY = 0;
			}
		}
		(*uxa_screen->info->done_copy) (pPixmap);

		if (alu != GXcopy)
			ret = TRUE;
		else {
			Bool more_copy = FALSE;

			for (i = 0; i < nbox; i++) {
				int dstX = pBox[i].x1 + tileWidth;
				int dstY = pBox[i].y1 + tileHeight;

				if ((dstX < pBox[i].x2) || (dstY < pBox[i].y2)) {
					more_copy = TRUE;
					break;
				}
			}

			if (more_copy == FALSE)
				ret = TRUE;

			if (more_copy && (*uxa_screen->info->prepare_copy) (pPixmap, pPixmap, 1, 1, alu, planemask)) {
				for (i = 0; i < nbox; i++) {
					int dstX = pBox[i].x1 + tileWidth;
					int dstY = pBox[i].y1 + tileHeight;
					int width = min(pBox[i].x2 - dstX, tileWidth);
					int height = min(pBox[i].y2 - pBox[i].y1, tileHeight);

					while (dstX < pBox[i].x2) {
						(*uxa_screen->info->copy) (pPixmap,
									   pBox[i].x1, pBox[i].y1,
									   dstX, pBox[i].y1, 
									   width, height);
						dstX += width;
						width = min(pBox[i].x2 - dstX, width * 2);
					}

					width = pBox[i].x2 - pBox[i].x1;
					height = min(pBox[i].y2 - dstY, tileHeight);

					while (dstY < pBox[i].y2) {
						(*uxa_screen->info->copy) (pPixmap,
									   pBox[i].x1, pBox[i].y1,
									   pBox[i].x1, dstY, 
									   width, height);
						dstY += height;
						height = min(pBox[i].y2 - dstY, height * 2);
					}
				}
				(*uxa_screen->info->done_copy) (pPixmap);
				ret = TRUE;
			}
		}

		if (xoff || yoff)
			REGION_TRANSLATE(pScreen, pRegion, -xoff, -yoff);

	}

out:

	return ret;
}

/**
 * Accelerates GetImage for solid ZPixmap downloads from framebuffer memory.
 *
 * This is probably the only case we actually care about.  The rest fall through
 * to migration and fbGetImage, which hopefully will result in migration pushing
 * the pixmap out of framebuffer.
 */
void
uxa_get_image(DrawablePtr pDrawable, int x, int y, int w, int h,
	      unsigned int format, unsigned long planeMask, char *d)
{
	ScreenPtr screen = pDrawable->pScreen;
	uxa_screen_t *uxa_screen = uxa_get_screen(screen);
	BoxRec Box;
	PixmapPtr pPix = uxa_get_drawable_pixmap(pDrawable);
	int xoff, yoff;
	Bool ok;
	RegionRec region;

	uxa_get_drawable_deltas(pDrawable, pPix, &xoff, &yoff);

	Box.x1 = pDrawable->y + x + xoff;
	Box.y1 = pDrawable->y + y + yoff;
	Box.x2 = Box.x1 + w;
	Box.y2 = Box.y1 + h;

	if (uxa_screen->swappedOut || uxa_screen->force_fallback)
		goto fallback;

	pPix = uxa_get_offscreen_pixmap(pDrawable, &xoff, &yoff);

	if (pPix == NULL || uxa_screen->info->get_image == NULL)
		goto fallback;

	/* Only cover the ZPixmap, solid copy case. */
	if (format != ZPixmap || !UXA_PM_IS_SOLID(pDrawable, planeMask))
		goto fallback;

	/* Only try to handle the 8bpp and up cases, since we don't want to
	 * think about <8bpp.
	 */
	if (pDrawable->bitsPerPixel < 8)
		goto fallback;

	ok = uxa_screen->info->get_image(pPix, pDrawable->x + x + xoff,
					 pDrawable->y + y + yoff, w, h, d,
					 PixmapBytePad(w, pDrawable->depth));
	if (ok)
		return;

fallback:
	UXA_FALLBACK(("from %p (%c)\n", pDrawable,
		      uxa_drawable_location(pDrawable)));

	REGION_INIT(screen, &region, &Box, 1);
	
	if (uxa_prepare_access(pDrawable, &region, UXA_ACCESS_RO)) {
		fbGetImage(pDrawable, x, y, w, h, format, planeMask, d);
		uxa_finish_access(pDrawable);
	}

	REGION_UNINIT(screen, &region);
	
	return;
}
