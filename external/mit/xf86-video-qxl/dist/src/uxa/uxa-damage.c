/*
 * Copyright © 2003 Keith Packard
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdlib.h>
#include "uxa-priv.h"

#include    <dixfontstr.h>
#include    <gcstruct.h>
#include    <picturestr.h>
#include    <scrnintstr.h>
#include    <windowstr.h>
#include    <X11/X.h>
#include    <X11/fonts/font.h>
#include    <X11/fonts/fontstruct.h>
#ifdef HAVE_XFONT2
#include    <X11/fonts/libxfont2.h>
#else
#include    <X11/fonts/fontutil.h>
#endif

#include    "uxa-damage.h"

typedef struct _damageGCPriv {
    GCOps   *ops;
    GCFuncs *funcs;
} DamageGCPrivRec, *DamageGCPrivPtr;

#define DAMAGE_VALIDATE_ENABLE 0
#define DAMAGE_DEBUG_ENABLE 0
#if DAMAGE_DEBUG_ENABLE
#define DAMAGE_DEBUG(x)	ErrorF x
#else
#define DAMAGE_DEBUG(x)
#endif

#define TRIM_BOX(box, pGC) if (pGC->pCompositeClip) {			\
	BoxPtr extents = &pGC->pCompositeClip->extents;			\
	if(box.x1 < extents->x1) box.x1 = extents->x1;			\
	if(box.x2 > extents->x2) box.x2 = extents->x2;			\
	if(box.y1 < extents->y1) box.y1 = extents->y1;			\
	if(box.y2 > extents->y2) box.y2 = extents->y2;			\
    }

#define TRANSLATE_BOX(box, pDrawable) {					\
	box.x1 += pDrawable->x;						\
	box.x2 += pDrawable->x;						\
	box.y1 += pDrawable->y;						\
	box.y2 += pDrawable->y;						\
    }

#define TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC) {			\
	TRANSLATE_BOX(box, pDrawable);					\
	TRIM_BOX(box, pGC);						\
    }

#define BOX_NOT_EMPTY(box)						\
    (((box.x2 - box.x1) > 0) && ((box.y2 - box.y1) > 0))

#define checkGCDamage(g)	((!g->pCompositeClip ||			\
				  REGION_NOTEMPTY(d->pScreen,		\
						  g->pCompositeClip)))

#define TRIM_PICTURE_BOX(box, pDst) {					\
	BoxPtr extents = &pDst->pCompositeClip->extents;		\
	if(box.x1 < extents->x1) box.x1 = extents->x1;			\
	if(box.x2 > extents->x2) box.x2 = extents->x2;			\
	if(box.y1 < extents->y1) box.y1 = extents->y1;			\
	if(box.y2 > extents->y2) box.y2 = extents->y2;			\
    }

#define checkPictureDamage(p)	(REGION_NOTEMPTY(pScreen, p->pCompositeClip))

static void
trim_region (RegionPtr   pRegion,
	     DrawablePtr pDrawable,
	     int         subWindowMode)
{
    RegionRec       pixClip;
    int		    draw_x = 0;
    int		    draw_y = 0;
#ifdef COMPOSITE
    int             screen_x = 0, screen_y = 0;
#endif

    /* short circuit for empty regions */
    if (!REGION_NOTEMPTY(pScreen, pRegion))
        return;
    
#ifdef COMPOSITE
    /*
     * When drawing to a pixmap which is storing window contents,
     * the region presented is in pixmap relative coordinates which
     * need to be converted to screen relative coordinates
     */
    if (pDrawable->type != DRAWABLE_WINDOW)
    {
        screen_x = ((PixmapPtr) pDrawable)->screen_x - pDrawable->x;
        screen_y = ((PixmapPtr) pDrawable)->screen_y - pDrawable->y;
    }
    if (screen_x || screen_y)
        REGION_TRANSLATE (pScreen, pRegion, screen_x, screen_y);
#endif
    
    /* Clip against any children */
    if (pDrawable->type == DRAWABLE_WINDOW &&
        ((WindowPtr)(pDrawable))->backingStore == NotUseful)
    {
        if (subWindowMode == ClipByChildren)
        {
            REGION_INTERSECT(pScreen, pRegion, pRegion,
                             &((WindowPtr)(pDrawable))->clipList);
        }
        else if (subWindowMode == IncludeInferiors)
        {
	    RegionPtr pTempRegion =
                NotClippedByChildren((WindowPtr)(pDrawable));
            REGION_INTERSECT(pScreen, pRegion, pRegion, pTempRegion);
            REGION_DESTROY(pScreen, pTempRegion);
        }
        /* If subWindowMode is set to an invalid value, don't perform
         * any drawable-based clipping. */
    }

    /* Clip against border or pixmap bounds */
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	REGION_INTERSECT (pScreen, pRegion, pRegion,
			  &((WindowPtr)(pDrawable))->borderClip);
    }
    else
    {
	BoxRec  box;

	draw_x = pDrawable->x;
	draw_y = pDrawable->y;
#ifdef COMPOSITE
	/*
	 * Need to move everyone to screen coordinates
	 * XXX what about off-screen pixmaps with non-zero x/y?
	 */
	if (!WindowDrawable(pDrawable->type))
	{
	    draw_x += ((PixmapPtr) pDrawable)->screen_x;
	    draw_y += ((PixmapPtr) pDrawable)->screen_y;
	}
#endif
	
	box.x1 = draw_x;
	box.y1 = draw_y;
	box.x2 = draw_x + pDrawable->width;
	box.y2 = draw_y + pDrawable->height;
	
	REGION_INIT(pScreen, &pixClip, &box, 1);
	REGION_INTERSECT (pScreen, pRegion, pRegion, &pixClip);
	REGION_UNINIT(pScreen, &pixClip);
    }
    
    /*
     * Move region to target coordinate space
     */
    if (draw_x || draw_y)
	REGION_TRANSLATE (pScreen, pRegion, -draw_x, -draw_y);
    
    /* Now do something with the damage */
}

static void
add_region (RegionPtr	existing,
	    RegionPtr	new,
	    DrawablePtr pDrawable,
	    int         subWindowMode)
{
    trim_region (new, pDrawable, subWindowMode);
    
    REGION_UNION (pDrawable->pScreen, existing, existing, new);
}

static void
add_box (RegionPtr   existing,
	 BoxPtr	     box,
	 DrawablePtr drawable,
	 int	     subwindow_mode)
{
    RegionRec   region;
    
    REGION_INIT (pDrawable->pScreen, &region, box, 1);
    
    add_region (existing, &region, drawable, subwindow_mode);
    
    REGION_UNINIT (pDrawable->pScreen, &region);
}


void
uxa_damage_composite (RegionPtr  region,
		      CARD8      op,
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
    if (checkPictureDamage (pDst))
    {
	BoxRec	box;
	
	box.x1 = xDst + pDst->pDrawable->x;
	box.y1 = yDst + pDst->pDrawable->y;
	box.x2 = box.x1 + width;
	box.y2 = box.y1 + height;
	
	TRIM_PICTURE_BOX(box, pDst);
	
	if (BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDst->pDrawable, pDst->subWindowMode);
    }
}

void
uxa_damage_glyphs (RegionPtr		region,
		   CARD8		op,
		   PicturePtr	pSrc,
		   PicturePtr	pDst,
		   PictFormatPtr	maskFormat,
		   INT16		xSrc,
		   INT16		ySrc,
		   int		nlist,
		   GlyphListPtr	list,
		   GlyphPtr	       *glyphs)
{
    if (checkPictureDamage (pDst))
    {
	int		nlistTmp = nlist;
	GlyphListPtr	listTmp = list;
	GlyphPtr	*glyphsTmp = glyphs;
	int		x, y;
	int		n;
	GlyphPtr	glyph;
	BoxRec		box;
	int		x1, y1, x2, y2;
	
	box.x1 = 32767;
	box.y1 = 32767;
	box.x2 = -32767;
	box.y2 = -32767;
	x = pDst->pDrawable->x;
	y = pDst->pDrawable->y;
	while (nlistTmp--)
	{
	    x += listTmp->xOff;
	    y += listTmp->yOff;
	    n = listTmp->len;
	    while (n--)
	    {
		glyph = *glyphsTmp++;
		x1 = x - glyph->info.x;
		y1 = y - glyph->info.y;
		x2 = x1 + glyph->info.width;
		y2 = y1 + glyph->info.height;
		if (x1 < box.x1)
		    box.x1 = x1;
		if (y1 < box.y1)
		    box.y1 = y1;
		if (x2 > box.x2)
		    box.x2 = x2;
		if (y2 > box.y2)
		    box.y2 = y2;
		x += glyph->info.xOff;
		y += glyph->info.yOff;
	    }
	    listTmp++;
	}
	TRIM_PICTURE_BOX (box, pDst);
	if (BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDst->pDrawable, pDst->subWindowMode);
    }
}

void
uxa_damage_add_traps (RegionPtr   region,
		      PicturePtr  pPicture,
		      INT16	    x_off,
		      INT16	    y_off,
		      int	    ntrap,
		      xTrap	    *traps)
{
    if (checkPictureDamage (pPicture))
    {
	BoxRec	box;
	int	i;
	int	x, y;
	xTrap	*t = traps;
	
	box.x1 = 32767;
	box.y1 = 32767;
	box.x2 = -32767;
	box.y2 = -32767;
	x = pPicture->pDrawable->x + x_off;
	y = pPicture->pDrawable->y + y_off;
	for (i = 0; i < ntrap; i++)
	{
	    pixman_fixed_t   l = min (t->top.l, t->bot.l);
	    pixman_fixed_t   r = max (t->top.r, t->bot.r);
	    int	    x1 = x + pixman_fixed_to_int (l);
	    int	    x2 = x + pixman_fixed_to_int (pixman_fixed_ceil (r));
	    int	    y1 = y + pixman_fixed_to_int (t->top.y);
	    int	    y2 = y + pixman_fixed_to_int (pixman_fixed_ceil (t->bot.y));
	    
	    if (x1 < box.x1)
		box.x1 = x1;
	    if (x2 > box.x2)
		box.x2 = x2;
	    if (y1 < box.y1)
		box.y1 = y1;
	    if (y2 > box.y2)
		box.y2 = y2;
	}
	TRIM_PICTURE_BOX (box, pPicture);
	if (BOX_NOT_EMPTY(box))
	    add_box (region, &box, pPicture->pDrawable, pPicture->subWindowMode);
    }
}

/**********************************************************/


void
uxa_damage_fill_spans (RegionPtr   region,
		       DrawablePtr pDrawable,
		       GC	    *pGC,
		       int	     npt,
		       DDXPointPtr ppt,
		       int	    *pwidth,
		       int	     fSorted)
{
    if (npt && checkGCDamage (pGC))
    {
	int	    nptTmp = npt;
	DDXPointPtr pptTmp = ppt;
	int	    *pwidthTmp = pwidth;
	BoxRec	    box;
	
	box.x1 = pptTmp->x;
	box.x2 = box.x1 + *pwidthTmp;
	box.y2 = box.y1 = pptTmp->y;
	
	while(--nptTmp) 
	{
	    pptTmp++;
	    pwidthTmp++;
	    if(box.x1 > pptTmp->x) box.x1 = pptTmp->x;
	    if(box.x2 < (pptTmp->x + *pwidthTmp))
		box.x2 = pptTmp->x + *pwidthTmp;
	    if(box.y1 > pptTmp->y) box.y1 = pptTmp->y;
	    else if(box.y2 < pptTmp->y) box.y2 = pptTmp->y;
	}
	
	box.y2++;
	
        if(!pGC->miTranslate) {
	    TRANSLATE_BOX(box, pDrawable);
        }
        TRIM_BOX(box, pGC); 
	
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}

void
uxa_damage_set_spans (RegionPtr    region,
		      DrawablePtr  pDrawable,
		      GCPtr	     pGC,
		      char	    *pcharsrc,
		      DDXPointPtr  ppt,
		      int	    *pwidth,
		      int	     npt,
		      int	     fSorted)
{
    if (npt && checkGCDamage (pGC))
    {
	DDXPointPtr pptTmp = ppt;
	int	    *pwidthTmp = pwidth;
	int	    nptTmp = npt;
	BoxRec	    box;
	
	box.x1 = pptTmp->x;
	box.x2 = box.x1 + *pwidthTmp;
	box.y2 = box.y1 = pptTmp->y;
	
	while(--nptTmp) 
	{
	    pptTmp++;
	    pwidthTmp++;
	    if(box.x1 > pptTmp->x) box.x1 = pptTmp->x;
	    if(box.x2 < (pptTmp->x + *pwidthTmp))
		box.x2 = pptTmp->x + *pwidthTmp;
	    if(box.y1 > pptTmp->y) box.y1 = pptTmp->y;
	    else if(box.y2 < pptTmp->y) box.y2 = pptTmp->y;
	}
	
	box.y2++;
	
        if(!pGC->miTranslate) {
	    TRANSLATE_BOX(box, pDrawable);
        }
        TRIM_BOX(box, pGC); 
	
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}

void
uxa_damage_put_image (RegionPtr    region,
		      DrawablePtr  pDrawable,
		      GCPtr	     pGC,
		      int	     depth,
		      int	     x,
		      int	     y,
		      int	     w,
		      int	     h,
		      int	     leftPad,
		      int	     format,
		      char	    *pImage)
{
    if (checkGCDamage (pGC))
    {
	BoxRec box;
	
	box.x1 = x + pDrawable->x;
	box.x2 = box.x1 + w;
	box.y1 = y + pDrawable->y;
	box.y2 = box.y1 + h;
	
	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}

void
uxa_damage_copy_area(RegionPtr    region,
		     DrawablePtr  pSrc,
		     DrawablePtr  pDst,
		     GC	         *pGC,
		     int	  srcx,
		     int	  srcy,
		     int	    width,
		     int	    height,
		     int	    dstx,
		     int	    dsty)
{
    if (checkGCDamage (pGC))
    {
	BoxRec box;
	
	box.x1 = dstx + pDst->x;
	box.x2 = box.x1 + width;
	box.y1 = dsty + pDst->y;
	box.y2 = box.y1 + height;
	
	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDst, pGC->subWindowMode);
    }
}

void
uxa_damage_copy_plane (RegionPtr	region,
		       DrawablePtr	pSrc,
		       DrawablePtr	pDst,
		       GCPtr		pGC,
		       int		srcx,
		       int		srcy,
		       int		width,
		       int		height,
		       int		dstx,
		       int		dsty,
		       unsigned long	bitPlane)
{
    if (checkGCDamage (pGC))
    {
	BoxRec box;
	
	box.x1 = dstx + pDst->x;
	box.x2 = box.x1 + width;
	box.y1 = dsty + pDst->y;
	box.y2 = box.y1 + height;
	
	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDst, pGC->subWindowMode);
    }
}

void
uxa_damage_poly_point (RegionPtr   region,
		       DrawablePtr pDrawable,
		       GCPtr	    pGC,
		       int	    mode,
		       int	    npt,
		       xPoint	    *ppt)
{
    if (npt && checkGCDamage (pGC))
    {
	BoxRec	box;
	int	nptTmp = npt;
	xPoint	*pptTmp = ppt;
	
	box.x2 = box.x1 = pptTmp->x;
	box.y2 = box.y1 = pptTmp->y;
	
	/* this could be slow if the points were spread out */
	
	while(--nptTmp) 
	{
	    pptTmp++;
	    if(box.x1 > pptTmp->x) box.x1 = pptTmp->x;
	    else if(box.x2 < pptTmp->x) box.x2 = pptTmp->x;
	    if(box.y1 > pptTmp->y) box.y1 = pptTmp->y;
	    else if(box.y2 < pptTmp->y) box.y2 = pptTmp->y;
	}
	
	box.x2++;
	box.y2++;
	
	TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}

void
uxa_damage_poly_lines (RegionPtr  region,
		       DrawablePtr pDrawable,
		       GCPtr	    pGC,
		       int	    mode,
		       int	    npt,
		       DDXPointPtr ppt)
{
    if (npt && checkGCDamage (pGC))
    {
	int	    nptTmp = npt;
	DDXPointPtr pptTmp = ppt;
	BoxRec	    box;
	int	    extra = pGC->lineWidth >> 1;
	
	box.x2 = box.x1 = pptTmp->x;
	box.y2 = box.y1 = pptTmp->y;
	
	if(nptTmp > 1) 
	{
	    if(pGC->joinStyle == JoinMiter)
		extra = 6 * pGC->lineWidth;
	    else if(pGC->capStyle == CapProjecting)
		extra = pGC->lineWidth;
        }
	
	if(mode == CoordModePrevious) 
	{
	    int x = box.x1;
	    int y = box.y1;
	    while(--nptTmp) 
	    {
		pptTmp++;
		x += pptTmp->x;
		y += pptTmp->y;
		if(box.x1 > x) box.x1 = x;
		else if(box.x2 < x) box.x2 = x;
		if(box.y1 > y) box.y1 = y;
		else if(box.y2 < y) box.y2 = y;
	    }
	}
	else 
	{
	    while(--nptTmp) 
	    {
		pptTmp++;
		if(box.x1 > pptTmp->x) box.x1 = pptTmp->x;
		else if(box.x2 < pptTmp->x) box.x2 = pptTmp->x;
		if(box.y1 > pptTmp->y) box.y1 = pptTmp->y;
		else if(box.y2 < pptTmp->y) box.y2 = pptTmp->y;
	    }
	}
	
	box.x2++;
	box.y2++;
	
	if(extra) 
	{
	    box.x1 -= extra;
	    box.x2 += extra;
	    box.y1 -= extra;
	    box.y2 += extra;
        }
	
	TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}

void
uxa_damage_poly_segment (RegionPtr    region,
			 DrawablePtr	pDrawable,
			 GCPtr	pGC,
			 int		nSeg,
			 xSegment	*pSeg)
{
    if (nSeg && checkGCDamage (pGC))
    {
	BoxRec	    box;
	int	    extra = pGC->lineWidth;
	int	    nsegTmp = nSeg;
	xSegment    *pSegTmp = pSeg;
	
        if(pGC->capStyle != CapProjecting)
	    extra >>= 1;
	
	if(pSegTmp->x2 > pSegTmp->x1) {
	    box.x1 = pSegTmp->x1;
	    box.x2 = pSegTmp->x2;
	} else {
	    box.x2 = pSegTmp->x1;
	    box.x1 = pSegTmp->x2;
	}
	
	if(pSegTmp->y2 > pSegTmp->y1) {
	    box.y1 = pSegTmp->y1;
	    box.y2 = pSegTmp->y2;
	} else {
	    box.y2 = pSegTmp->y1;
	    box.y1 = pSegTmp->y2;
	}
	
	while(--nsegTmp) 
	{
	    pSegTmp++;
	    if(pSegTmp->x2 > pSegTmp->x1) 
	    {
		if(pSegTmp->x1 < box.x1) box.x1 = pSegTmp->x1;
		if(pSegTmp->x2 > box.x2) box.x2 = pSegTmp->x2;
	    }
	    else 
	    {
		if(pSegTmp->x2 < box.x1) box.x1 = pSegTmp->x2;
		if(pSegTmp->x1 > box.x2) box.x2 = pSegTmp->x1;
	    }
	    if(pSegTmp->y2 > pSegTmp->y1) 
	    {
		if(pSegTmp->y1 < box.y1) box.y1 = pSegTmp->y1;
		if(pSegTmp->y2 > box.y2) box.y2 = pSegTmp->y2;
	    }
	    else
	    {
		if(pSegTmp->y2 < box.y1) box.y1 = pSegTmp->y2;
		if(pSegTmp->y1 > box.y2) box.y2 = pSegTmp->y1;
	    }
	}
	
	box.x2++;
	box.y2++;
	
	if(extra) 
	{
	    box.x1 -= extra;
	    box.x2 += extra;
	    box.y1 -= extra;
	    box.y2 += extra;
        }
	
	TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}

void
uxa_damage_poly_rectangle (RegionPtr    region,
			   DrawablePtr  pDrawable,
			   GCPtr        pGC,
			   int	  nRects,
			   xRectangle  *pRects)
{
    if (nRects && checkGCDamage (pGC))
    {
	BoxRec	    box;
	int	    offset1, offset2, offset3;
	int	    nRectsTmp = nRects;
	xRectangle  *pRectsTmp = pRects;
	
	offset2 = pGC->lineWidth;
	if(!offset2) offset2 = 1;
	offset1 = offset2 >> 1;
	offset3 = offset2 - offset1;
	
	while(nRectsTmp--)
	{
	    box.x1 = pRectsTmp->x - offset1;
	    box.y1 = pRectsTmp->y - offset1;
	    box.x2 = box.x1 + pRectsTmp->width + offset2;
	    box.y2 = box.y1 + offset2;
	    TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	    if(BOX_NOT_EMPTY(box))
		add_box (region, &box, pDrawable, pGC->subWindowMode);
	    
	    box.x1 = pRectsTmp->x - offset1;
	    box.y1 = pRectsTmp->y + offset3;
	    box.x2 = box.x1 + offset2;
	    box.y2 = box.y1 + pRectsTmp->height - offset2;
	    TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	    if(BOX_NOT_EMPTY(box))
		add_box (region, &box, pDrawable, pGC->subWindowMode);
	    
	    box.x1 = pRectsTmp->x + pRectsTmp->width - offset1;
	    box.y1 = pRectsTmp->y + offset3;
	    box.x2 = box.x1 + offset2;
	    box.y2 = box.y1 + pRectsTmp->height - offset2;
	    TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	    if(BOX_NOT_EMPTY(box))
		add_box (region, &box, pDrawable, pGC->subWindowMode);
	    
	    box.x1 = pRectsTmp->x - offset1;
	    box.y1 = pRectsTmp->y + pRectsTmp->height - offset1;
	    box.x2 = box.x1 + pRectsTmp->width + offset2;
	    box.y2 = box.y1 + offset2;
	    TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	    if(BOX_NOT_EMPTY(box))
		add_box (region, &box, pDrawable, pGC->subWindowMode);
	    
	    pRectsTmp++;
	}
    }
}

void
uxa_damage_poly_arc (RegionPtr    region,
		     DrawablePtr  pDrawable,
		     GCPtr	    pGC,
		     int	    nArcs,
		     xArc	    *pArcs)
{
    if (nArcs && checkGCDamage (pGC))
    {
	int	extra = pGC->lineWidth >> 1;
	BoxRec	box;
	int	nArcsTmp = nArcs;
	xArc	*pArcsTmp = pArcs;
	
	box.x1 = pArcsTmp->x;
	box.x2 = box.x1 + pArcsTmp->width;
	box.y1 = pArcsTmp->y;
	box.y2 = box.y1 + pArcsTmp->height;
	
	while(--nArcsTmp) 
	{
	    pArcsTmp++;
	    if(box.x1 > pArcsTmp->x)
		box.x1 = pArcsTmp->x;
	    if(box.x2 < (pArcsTmp->x + pArcsTmp->width))
		box.x2 = pArcsTmp->x + pArcsTmp->width;
	    if(box.y1 > pArcsTmp->y) 
		box.y1 = pArcsTmp->y;
	    if(box.y2 < (pArcsTmp->y + pArcsTmp->height))
		box.y2 = pArcsTmp->y + pArcsTmp->height;
        }
	
	if(extra) 
	{
	    box.x1 -= extra;
	    box.x2 += extra;
	    box.y1 -= extra;
	    box.y2 += extra;
        }
	
	box.x2++;
	box.y2++;
	
	TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}

void
uxa_damage_fill_polygon (RegionPtr     region,
			 DrawablePtr	pDrawable,
			 GCPtr		pGC,
			 int		shape,
			 int		mode,
			 int		npt,
			 DDXPointPtr	ppt)
{
    if (npt > 2 && checkGCDamage (pGC))
    {
	DDXPointPtr pptTmp = ppt;
	int	    nptTmp = npt;
	BoxRec	    box;
	
	box.x2 = box.x1 = pptTmp->x;
	box.y2 = box.y1 = pptTmp->y;
	
	if(mode != CoordModeOrigin) 
	{
	    int x = box.x1;
	    int y = box.y1;
	    while(--nptTmp) 
	    {
		pptTmp++;
		x += pptTmp->x;
		y += pptTmp->y;
		if(box.x1 > x) box.x1 = x;
		else if(box.x2 < x) box.x2 = x;
		if(box.y1 > y) box.y1 = y;
		else if(box.y2 < y) box.y2 = y;
	    }
	}
	else 
	{
	    while(--nptTmp) 
	    {
		pptTmp++;
		if(box.x1 > pptTmp->x) box.x1 = pptTmp->x;
		else if(box.x2 < pptTmp->x) box.x2 = pptTmp->x;
		if(box.y1 > pptTmp->y) box.y1 = pptTmp->y;
		else if(box.y2 < pptTmp->y) box.y2 = pptTmp->y;
	    }
	}
	
	box.x2++;
	box.y2++;
	
	TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}


void
uxa_damage_poly_fill_rect (RegionPtr   region,
			   DrawablePtr	pDrawable,
			   GCPtr	pGC,
			   int		nRects,
			   xRectangle	*pRects)
{
    if (nRects && checkGCDamage (pGC))
    {
	int i;

	for (i = 0; i < nRects; ++i)
	{
	    xRectangle *rect = &(pRects[i]);
	    BoxRec box;

	    box.x1 = rect->x;
	    box.x2 = rect->x + rect->width;
	    box.y1 = rect->y;
	    box.y2 = rect->y + rect->height;

	    TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	    if(BOX_NOT_EMPTY(box))
		add_box (region, &box, pDrawable, pGC->subWindowMode);
	}
    }
}


void
uxa_damage_poly_fill_arc (RegionPtr    region,
			  DrawablePtr	pDrawable,
			  GCPtr		pGC,
			  int		nArcs,
			  xArc		*pArcs)
{
    if (nArcs && checkGCDamage (pGC))
    {
	BoxRec	box;
	int	nArcsTmp = nArcs;
	xArc	*pArcsTmp = pArcs;
	
	box.x1 = pArcsTmp->x;
	box.x2 = box.x1 + pArcsTmp->width;
	box.y1 = pArcsTmp->y;
	box.y2 = box.y1 + pArcsTmp->height;
	
	while(--nArcsTmp) 
	{
	    pArcsTmp++;
	    if(box.x1 > pArcsTmp->x)
		box.x1 = pArcsTmp->x;
	    if(box.x2 < (pArcsTmp->x + pArcsTmp->width))
		box.x2 = pArcsTmp->x + pArcsTmp->width;
	    if(box.y1 > pArcsTmp->y)
		box.y1 = pArcsTmp->y;
	    if(box.y2 < (pArcsTmp->y + pArcsTmp->height))
		box.y2 = pArcsTmp->y + pArcsTmp->height;
        }
	
	TRIM_AND_TRANSLATE_BOX(box, pDrawable, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}

/*
 * general Poly/Image text function.  Extract glyph information,
 * compute bounding box and remove cursor if it is overlapped.
 */

void
uxa_damage_chars (RegionPtr	region,
		  DrawablePtr	pDrawable,
		  FontPtr	font,
		  int		x,
		  int		y,
		  unsigned int	n,
		  CharInfoPtr	*charinfo,
		  Bool		imageblt,
		  int		subWindowMode)
{
    ExtentInfoRec   extents;
    BoxRec	    box;

#ifdef HAVE_XFONT2
    xfont2_query_glyph_extents(font, charinfo, n, &extents);
#else
    QueryGlyphExtents(font, charinfo, n, &extents);
#endif
    if (imageblt)
    {
	if (extents.overallWidth > extents.overallRight)
	    extents.overallRight = extents.overallWidth;
	if (extents.overallWidth < extents.overallLeft)
	    extents.overallLeft = extents.overallWidth;
	if (extents.overallLeft > 0)
	    extents.overallLeft = 0;
	if (extents.fontAscent > extents.overallAscent)
	    extents.overallAscent = extents.fontAscent;
	if (extents.fontDescent > extents.overallDescent)
	    extents.overallDescent = extents.fontDescent;
    }
    box.x1 = x + extents.overallLeft;
    box.y1 = y - extents.overallAscent;
    box.x2 = x + extents.overallRight;
    box.y2 = y + extents.overallDescent;
    add_box (region, &box, pDrawable, subWindowMode);
}

/*
 * values for textType:
 */
#define TT_POLY8   0
#define TT_IMAGE8  1
#define TT_POLY16  2
#define TT_IMAGE16 3

int
uxa_damage_text (RegionPtr	region,
		 DrawablePtr	    pDrawable,
		 GCPtr	    pGC,
		 int		    x,
		 int		    y,
		 unsigned long   count,
		 char	    *chars,
		 FontEncoding    fontEncoding,
		 Bool	    textType)
{
    CharInfoPtr	    *charinfo;
    CharInfoPtr	    *info;
    unsigned long   i;
    unsigned int    n;
    int		    w;
    Bool	    imageblt;
    
    imageblt = (textType == TT_IMAGE8) || (textType == TT_IMAGE16);
    
    charinfo = malloc(count * sizeof(CharInfoPtr));
    if (!charinfo)
	return x;
    
    GetGlyphs(pGC->font, count, (unsigned char *)chars,
	      fontEncoding, &i, charinfo);
    n = (unsigned int)i;
    w = 0;
    if (!imageblt)
	for (info = charinfo; i--; info++)
	    w += (*info)->metrics.characterWidth;
    
    if (n != 0) {
	uxa_damage_chars (region,
			  pDrawable, pGC->font, x + pDrawable->x, y + pDrawable->y, n,
			  charinfo, imageblt, pGC->subWindowMode);
    }
    free(charinfo);
    return x + w;
}

int
uxa_damage_poly_text_8(RegionPtr	region,
		       DrawablePtr pDrawable,
		       GCPtr	    pGC,
		       int	    x,
		       int	    y,
		       int	    count,
		       char	    *chars)
{
    if (checkGCDamage (pGC))
	x = uxa_damage_text (region,
			     pDrawable, pGC, x, y, (unsigned long) count, chars,
			     Linear8Bit, TT_POLY8);
    return x;
}

int
uxa_damage_poly_text_16 (RegionPtr	region,
			 DrawablePtr	pDrawable,
			 GCPtr		pGC,
			 int		x,
			 int		y,
			 int		count,
			 unsigned short	*chars)
{
    if (checkGCDamage (pGC))
	x = uxa_damage_text (region,
			     pDrawable, pGC, x, y, (unsigned long) count, (char *) chars,
			     FONTLASTROW(pGC->font) == 0 ? Linear16Bit : TwoD16Bit,
			     TT_POLY16);
    
    return x;
}

void
uxa_damage_image_text_8(RegionPtr	region,
			DrawablePtr	pDrawable,
			GCPtr		pGC,
			int		x,
			int		y,
			int		count,
			char		*chars)
{
    if (checkGCDamage (pGC))
	uxa_damage_text (region,
			 pDrawable, pGC, x, y, (unsigned long) count, chars,
			 Linear8Bit, TT_IMAGE8);
}

void
uxa_damage_image_text_16 (RegionPtr	region,
			  DrawablePtr	pDrawable,
			  GCPtr		pGC,
			  int		x,
			  int		y,
			  int		count,
			  unsigned short *chars)
{
    if (checkGCDamage (pGC))
	uxa_damage_text (region,
			 pDrawable, pGC, x, y, (unsigned long) count, (char *) chars,
			 FONTLASTROW(pGC->font) == 0 ? Linear16Bit : TwoD16Bit,
			 TT_IMAGE16);
}


void
uxa_damage_image_glyph_blt(RegionPtr	region,
			   DrawablePtr	    pDrawable,
			   GCPtr	    pGC,
			   int		    x,
			   int		    y,
			   unsigned int    nglyph,
			   CharInfoPtr	    *ppci,
			   pointer	    pglyphBase)
{
    uxa_damage_chars (region,
		      pDrawable, pGC->font, x + pDrawable->x, y + pDrawable->y,
		      nglyph, ppci, TRUE, pGC->subWindowMode);
}

void
uxa_damage_poly_glyph_blt(RegionPtr	region,
			  DrawablePtr	pDrawable,
			  GCPtr	pGC,
			  int		x,
			  int		y,
			  unsigned int	nglyph,
			  CharInfoPtr	*ppci,
			  pointer	pglyphBase)
{
    uxa_damage_chars (region,
		      pDrawable, pGC->font, x + pDrawable->x, y + pDrawable->y,
		      nglyph, ppci, FALSE, pGC->subWindowMode);
}

void
uxa_damage_push_pixels (RegionPtr	region,
			GCPtr		pGC,
			PixmapPtr	pBitMap,
			DrawablePtr	pDrawable,
			int		dx,
			int		dy,
			int		xOrg,
			int		yOrg)
{
    if(checkGCDamage (pGC))
    {
	BoxRec box;
	
        box.x1 = xOrg;
        box.y1 = yOrg;
	
        if(!pGC->miTranslate) {
	    box.x1 += pDrawable->x;          
	    box.y1 += pDrawable->y;          
        }
	
	box.x2 = box.x1 + dx;
	box.y2 = box.y1 + dy;
	
	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	    add_box (region, &box, pDrawable, pGC->subWindowMode);
    }
}

void
uxa_damage_copy_window(RegionPtr	region,
		       WindowPtr	pWindow,
		       DDXPointRec	ptOldOrg,
		       RegionPtr	prgnSrc)
{
#if 0
    ScreenPtr pScreen = pWindow->drawable.pScreen;
    damageScrPriv(pScreen);
    int dx = pWindow->drawable.x - ptOldOrg.x;
    int dy = pWindow->drawable.y - ptOldOrg.y;
    
    /*
     * The region comes in source relative, but the damage occurs
     * at the destination location.  Translate back and forth.
     */
    REGION_TRANSLATE (pScreen, prgnSrc, dx, dy);
    damageRegionAppend (&pWindow->drawable, prgnSrc, FALSE, -1);
    REGION_TRANSLATE (pScreen, prgnSrc, -dx, -dy);
#endif
    
    /* FIXME */
}
