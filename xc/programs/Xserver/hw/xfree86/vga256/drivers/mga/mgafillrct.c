/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mgafillrct.c,v 3.1 1996/10/19 15:16:23 dawes Exp $ */

#include "vga256.h"
#include "cfb16.h"
#include "cfb24.h"
#include "cfb32.h"
#include "xf86.h"

#include "mga.h"

extern int vgaBitsPerPixel;

#define NUM_STACK_RECTS	1024

#define mgaFillRectSolidXor \
(vgaBitsPerPixel > 16? \
(vgaBitsPerPixel == 32? cfb32FillRectSolidXor : cfb24FillRectSolidXor) : \
(vgaBitsPerPixel == 16? cfb16FillRectSolidXor : vga256FillRectSolidXor))

#define mgaFillRectSolidGeneral \
(vgaBitsPerPixel > 16? \
(vgaBitsPerPixel == 32? cfb32FillRectSolidGeneral : cfb24FillRectSolidGeneral) : \
(vgaBitsPerPixel == 16? cfb16FillRectSolidGeneral : vga256FillRectSolidGeneral))

#define mgaFillRectTileOdd \
(vgaBitsPerPixel > 16? \
(vgaBitsPerPixel == 32? cfb32FillRectTileOdd : cfb24FillRectTileOdd) : \
(vgaBitsPerPixel == 16? cfb16FillRectTileOdd : vga256FillRectTileOdd))

#define mgaFillRectTile32Copy \
(vgaBitsPerPixel > 16? \
(vgaBitsPerPixel == 32? cfb32FillRectTile32Copy : cfb24FillRectTile32Copy) : \
(vgaBitsPerPixel == 16? cfb16FillRectTile32Copy : vga256FillRectTile32Copy))

#define mgaFillRectTile32General \
(vgaBitsPerPixel > 16? \
(vgaBitsPerPixel == 32? cfb32FillRectTile32General : cfb24FillRectTile32General) : \
(vgaBitsPerPixel == 16? cfb16FillRectTile32General : vga256FillRectTile32General))

void
mgaFillRectSolidCopy(pDrawable, pGC, nBox, pBox)
	DrawablePtr     pDrawable;
	GCPtr           pGC;
	int             nBox;
	BoxPtr          pBox;
{
	switch(vgaBitsPerPixel)
	{
	case 8:
		if(xf86VTSema && pDrawable->type == DRAWABLE_WINDOW)
			mgaFillBoxSolid(pDrawable, nBox, pBox,
					pGC->fgPixel & 0xFF);
		else
			vga256FillRectSolidCopy(pDrawable, pGC, nBox, pBox);
		break;
	case 16:
		if(xf86VTSema && pDrawable->type == DRAWABLE_WINDOW)
			mgaFillBoxSolid(pDrawable, nBox, pBox,
					pGC->fgPixel & 0xFFFF);
		else
			cfb16FillRectSolidCopy(pDrawable, pGC, nBox, pBox);
		break;
	case 24:
		if(xf86VTSema && pDrawable->type == DRAWABLE_WINDOW)
			mgaFillBoxSolid(pDrawable, nBox, pBox, 
					pGC->fgPixel & 0xFFFFFF);
		else
			cfb24FillRectSolidCopy(pDrawable, pGC, nBox, pBox);
		break;
	case 32:
		if(xf86VTSema && pDrawable->type == DRAWABLE_WINDOW)
			mgaFillBoxSolid(pDrawable, nBox, pBox,
					pGC->fgPixel);
		else
			cfb32FillRectSolidCopy(pDrawable, pGC, nBox, pBox);
		break;
	}
}

void
mgaPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    register GCPtr pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    xRectangle	    *prect;
    RegionPtr	    prgnClip;
    register BoxPtr pbox;
    register BoxPtr pboxClipped;
    BoxPtr	    pboxClippedBase;
    BoxPtr	    pextent;
    BoxRec	    stackRects[NUM_STACK_RECTS];
    cfbPrivGC	    *priv;
    int		    numRects;
    void	    (*BoxFill)();
    int		    n;
    int		    xorg, yorg;

    if (vgaBitsPerPixel != 8)
    	if ((pGC->fillStyle == FillStippled) ||
    	    (pGC->fillStyle == FillOpaqueStippled)) {
        miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
        return;
        }

    priv = cfbGetGCPrivate(pGC);
    prgnClip = priv->pCompositeClip;

    BoxFill = 0;
    switch (pGC->fillStyle)
    {
    case FillSolid:
	switch (priv->rop) {
	case GXcopy:
	    BoxFill = mgaFillRectSolidCopy;
	    break;
	case GXxor:
	    BoxFill = mgaFillRectSolidXor;
	    break;
	default:
	    BoxFill = mgaFillRectSolidGeneral;
	    break;
	}
	break;
    case FillTiled:
	if (!cfbGetGCPrivate(pGC)->pRotatedPixmap)
	    BoxFill = mgaFillRectTileOdd;
	else
	{
	    if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
		BoxFill = mgaFillRectTile32Copy;
	    else
		BoxFill = mgaFillRectTile32General;
	}
	break;
    case FillStippled:
	if (!cfbGetGCPrivate(pGC)->pRotatedPixmap)
	    BoxFill = cfb8FillRectStippledUnnatural;
	else
	    BoxFill = cfb8FillRectTransparentStippled32;
	break;
    case FillOpaqueStippled:
	if (!cfbGetGCPrivate(pGC)->pRotatedPixmap)
	    BoxFill = cfb8FillRectStippledUnnatural;
	else
	    BoxFill = cfb8FillRectOpaqueStippled32;
	break;
    }
    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (xorg || yorg)
    {
	prect = prectInit;
	n = nrectFill;
	while(n--)
	{
	    prect->x += xorg;
	    prect->y += yorg;
	    prect++;
	}
    }

    prect = prectInit;

    numRects = REGION_NUM_RECTS(prgnClip) * nrectFill;
    if (numRects > NUM_STACK_RECTS)
    {
	pboxClippedBase = (BoxPtr)ALLOCATE_LOCAL(numRects * sizeof(BoxRec));
	if (!pboxClippedBase)
	    return;
    }
    else
	pboxClippedBase = stackRects;

    pboxClipped = pboxClippedBase;
	
    if (REGION_NUM_RECTS(prgnClip) == 1)
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_RECTS(prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    if ((pboxClipped->x1 = prect->x) < x1)
		pboxClipped->x1 = x1;
    
	    if ((pboxClipped->y1 = prect->y) < y1)
		pboxClipped->y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    pboxClipped->x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    pboxClipped->y2 = by2;

	    prect++;
	    if ((pboxClipped->x1 < pboxClipped->x2) &&
		(pboxClipped->y1 < pboxClipped->y2))
	    {
		pboxClipped++;
	    }
    	}
    }
    else
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    BoxRec box;
    
	    if ((box.x1 = prect->x) < x1)
		box.x1 = x1;
    
	    if ((box.y1 = prect->y) < y1)
		box.y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    box.x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    box.y2 = by2;
    
	    prect++;
    
	    if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    	continue;
    
	    n = REGION_NUM_RECTS (prgnClip);
	    pbox = REGION_RECTS(prgnClip);
    
	    /* clip the rectangle to each box in the clip region
	       this is logically equivalent to calling Intersect()
	    */
	    while(n--)
	    {
		pboxClipped->x1 = max(box.x1, pbox->x1);
		pboxClipped->y1 = max(box.y1, pbox->y1);
		pboxClipped->x2 = min(box.x2, pbox->x2);
		pboxClipped->y2 = min(box.y2, pbox->y2);
		pbox++;

		/* see if clipping left anything */
		if(pboxClipped->x1 < pboxClipped->x2 && 
		   pboxClipped->y1 < pboxClipped->y2)
		{
		    pboxClipped++;
		}
	    }
    	}
    }
    if (pboxClipped != pboxClippedBase)
	(*BoxFill) (pDrawable, pGC,
		    pboxClipped-pboxClippedBase, pboxClippedBase);
    if (pboxClippedBase != stackRects)
    	DEALLOCATE_LOCAL(pboxClippedBase);
}
