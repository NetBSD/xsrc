/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mgapntwin.c,v 3.4 1996/10/23 13:10:34 dawes Exp $ */

#include "vga256.h"
#include "cfb16.h"
#include "cfb24.h"
#include "cfb32.h"
#include "xf86.h"

#include "mga.h"
#include "mgareg.h"

extern int vgaBitsPerPixel;

#define mgaFillBoxTile32 \
(vgaBitsPerPixel > 16? \
(vgaBitsPerPixel == 32? cfb32FillBoxTile32 : cfb24FillBoxTile32) : \
(vgaBitsPerPixel == 16? cfb16FillBoxTile32 : vga256FillBoxTile32))
	
#define mgaFillBoxTileOdd \
(vgaBitsPerPixel > 16? \
(vgaBitsPerPixel == 32? cfb32FillBoxTileOdd : cfb24FillBoxTileOdd) : \
(vgaBitsPerPixel == 16? cfb16FillBoxTileOdd : vga256FillBoxTileOdd))
	
void
mgaFillBoxSolid (pDrawable, nBox, pBox, pixel)
    DrawablePtr	    pDrawable;
    int		    nBox;
    BoxPtr	    pBox;
    unsigned long   pixel;
{
	if(!xf86VTSema) 
		switch(vgaBitsPerPixel)
		{
		case 32:
			cfb32FillBoxSolid(pDrawable, nBox, pBox, pixel);
			return;
		case 24:
			cfb24FillBoxSolid(pDrawable, nBox, pBox, pixel);
			return;
		case 16:
			cfb16FillBoxSolid(pDrawable, nBox, pBox, pixel);
			return;
		case 8:
			vga256FillBoxSolid(pDrawable, nBox, pBox, pixel,
						0, GXcopy);
			return;
		}
	
	switch(vgaBitsPerPixel)
	{
	case 16:
		pixel |= pixel << 16;
		break;
	case 8:
		pixel |= (pixel << 8) | (pixel << 16) | (pixel << 24);
		break;
	}

	MGAWAITFIFOSLOTS(4);
	
	MGAREG(MGAREG_CXBNDRY) = 0xFFFF0000;  /* (maxX << 16) | minX */
	MGAREG(MGAREG_YTOP) = 0x00000000;  /* minPixelPointer */
	MGAREG(MGAREG_YBOT) = 0x007FFFFF;  /* maxPixelPointer */
	
	MGAREG(MGAREG_FCOL) = pixel;
	
	for (; nBox; nBox--, pBox++)
	{
		int h = pBox->y2 - pBox->y1;
		
		MGAWAITFIFOSLOTS(3);
		MGAREG(MGAREG_FXBNDRY) = (pBox->x2 << 16) | pBox->x1;
		MGAREG(MGAREG_YDSTLEN) = (pBox->y1 << 16) | h;
#if 0  /* RK - for what? We've got fifos. */
		if(!MGAWaitForBlitter())
			ErrorF("MGA: BitBlt Engine timeout\n");
#endif
		MGAREG(MGAREG_DWGCTL + MGAREG_EXEC) = 0x000C7804;
	}
	MGAWAITFREE();
}

void
mgaPaintWindow(pWin, pRegion, what)
    WindowPtr	pWin;
    RegionPtr	pRegion;
    int		what;
{
	register cfbPrivWin *pPrivWin;
	WindowPtr pBgWin;
	
	pPrivWin = cfbGetWindowPrivate(pWin);
	
	switch (what)
	{
	case PW_BACKGROUND:
		switch (pWin->backgroundState)
		{
		case None:
			break;
		case ParentRelative:
			do
			{
				pWin = pWin->parent;
			} while (pWin->backgroundState == ParentRelative);
			(*pWin->drawable.pScreen->PaintWindowBackground)(pWin,
					 		pRegion, what);
			break;
		case BackgroundPixmap:
			if (pPrivWin->fastBackground)
			{
				mgaFillBoxTile32 ((DrawablePtr)pWin,
					(int)REGION_NUM_RECTS(pRegion),
					REGION_RECTS(pRegion),
					pPrivWin->pRotatedBackground);
			}
			else
			{
				mgaFillBoxTileOdd ((DrawablePtr)pWin,
					(int)REGION_NUM_RECTS(pRegion),
					REGION_RECTS(pRegion),
					pWin->background.pixmap,
					(int) pWin->drawable.x,
					(int) pWin->drawable.y);
			}
			break;
		case BackgroundPixel:
			mgaFillBoxSolid ((DrawablePtr)pWin,
					(int)REGION_NUM_RECTS(pRegion),
					REGION_RECTS(pRegion),
					pWin->background.pixel);
					break;
		}
		break;
	case PW_BORDER:
		if (pWin->borderIsPixel)
		{
			mgaFillBoxSolid ((DrawablePtr)pWin,
				(int)REGION_NUM_RECTS(pRegion),
				REGION_RECTS(pRegion),
				pWin->border.pixel);
		}
		else if (pPrivWin->fastBorder)
		{
			mgaFillBoxTile32 ((DrawablePtr)pWin,
				(int)REGION_NUM_RECTS(pRegion),
				REGION_RECTS(pRegion),
				pPrivWin->pRotatedBorder);
		}
		else
		{
			for (pBgWin = pWin;
				pBgWin->backgroundState == ParentRelative;
				pBgWin = pBgWin->parent);

			mgaFillBoxTileOdd ((DrawablePtr)pWin,
				(int)REGION_NUM_RECTS(pRegion),
				REGION_RECTS(pRegion),
				pWin->border.pixmap,
				(int) pBgWin->drawable.x,
				(int) pBgWin->drawable.y);
		}
		break;
	}
}
