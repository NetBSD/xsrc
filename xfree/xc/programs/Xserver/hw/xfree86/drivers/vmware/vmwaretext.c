/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwaretext[] =

    "Id: vmwaretext.c,v 1.3 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwaretext.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"
#include "dixfontstr.h"

static void
vmwareTextExtent(FontPtr pFont, int count, char* chars, FontEncoding fontEncoding, BoxPtr box)
{
    unsigned long n, i;
    int w;
    CharInfoPtr charinfo[255];	/* encoding only has 1 byte for count */

    GetGlyphs(pFont, (unsigned long)count, (unsigned char *)chars,
	      fontEncoding, &n, charinfo);
    w = 0;
    for (i=0; i < n; i++) {
        w += charinfo[i]->metrics.characterWidth;
    }
    if (i) {
    	w += charinfo[i - 1]->metrics.rightSideBearing;
    }
    
    box->x1 = 0;
    if (n) {
	if (charinfo[0]->metrics.leftSideBearing < 0) {
            box->x1 = charinfo[0]->metrics.leftSideBearing;
        }
    }
    box->x2 = w;
    box->y1 = -FONTMAXBOUNDS(pFont,ascent);
    box->y2 = FONTMAXBOUNDS(pFont,descent);
}

static __inline void
vmwareFontToBox(BoxPtr BB, DrawablePtr pDrawable,
    GCPtr pGC, int x, int y, int count, char *chars,
    int wide)
{
    FontPtr pFont;

    pFont = pGC->font;
    if (pFont->info.constantWidth) {
        int ascent, descent, left, right = 0;

	ascent =
	    MAX(pFont->info.fontAscent, pFont->info.maxbounds.ascent);
	descent =
	    MAX(pFont->info.fontDescent,
	    pFont->info.maxbounds.descent);
	left = pFont->info.maxbounds.leftSideBearing;
	if (count > 0) {
	    right =
		(count - 1) * pFont->info.maxbounds.characterWidth;
	}
	right += pFont->info.maxbounds.rightSideBearing;
	BB->x1 =
	    MAX(pDrawable->x + x - left, (REGION_EXTENTS(pGC->pScreen,
		&((WindowPtr) pDrawable)->winSize))->x1);
	BB->y1 =
	    MAX(pDrawable->y + y - ascent,
	    (REGION_EXTENTS(pGC->pScreen,
             &((WindowPtr) pDrawable)->winSize))->y1);
	BB->x2 =
	    MIN(pDrawable->x + x + right,
	    (REGION_EXTENTS(pGC->pScreen,
             &((WindowPtr) pDrawable)->winSize))->x2);
	BB->y2 =
	    MIN(pDrawable->y + y + descent,
	    (REGION_EXTENTS(pGC->pScreen,
             &((WindowPtr) pDrawable)->winSize))->y2);
    } else {
    	vmwareTextExtent(pFont, count, chars, wide ? (FONTLASTROW(pFont) == 0) ? Linear16Bit : TwoD16Bit : Linear8Bit, BB);
	BB->x1 =
	    MAX(pDrawable->x + x + BB->x1, (REGION_EXTENTS(pGC->pScreen,
		&((WindowPtr) pDrawable)->winSize))->x1);
	BB->y1 =
	    MAX(pDrawable->y + y + BB->y1,
	    (REGION_EXTENTS(pGC->pScreen,
             &((WindowPtr) pDrawable)->winSize))->y1);
	BB->x2 =
	    MIN(pDrawable->x + x + BB->x2,
	    (REGION_EXTENTS(pGC->pScreen,
	     &((WindowPtr) pDrawable)->winSize))->x2);
	BB->y2 =
	    MIN(pDrawable->y + y + BB->y2,
	    (REGION_EXTENTS(pGC->pScreen, 
	     &((WindowPtr) pDrawable)->winSize))->y2);
    }
}

static __inline void
vmwareImageFontToBox(BoxPtr BB, DrawablePtr pDrawable,
    GCPtr pGC, int x, int y, int count, char *chars,
    int wide)
{
    FontPtr pFont;

    pFont = pGC->font;
    vmwareTextExtent(pFont, count, chars, 
    	wide ? (FONTLASTROW(pFont) == 0) ? Linear16Bit : TwoD16Bit : Linear8Bit, 
	BB);
    BB->x1 =
	    MAX(pDrawable->x + x + BB->x1, (REGION_EXTENTS(pGC->pScreen,
		&((WindowPtr) pDrawable)->winSize))->x1);
    BB->y1 =
	    MAX(pDrawable->y + y + BB->y1,
	    (REGION_EXTENTS(pGC->pScreen,
	     &((WindowPtr) pDrawable)->winSize))->y1);
    BB->x2 =
	    MIN(pDrawable->x + x + BB->x2,
	    (REGION_EXTENTS(pGC->pScreen,
	     &((WindowPtr) pDrawable)->winSize))->x2);
    BB->y2 =
	    MIN(pDrawable->y + y + BB->y2,
	    (REGION_EXTENTS(pGC->pScreen,
	     &((WindowPtr) pDrawable)->winSize))->y2);
}

int
vmwarePolyText8(DrawablePtr pDrawable,
    GCPtr pGC, int x, int y, int count, char *chars)
{
    int n;

    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
                    pGC->pScreen,
		    vmwareFontToBox(&BB, pDrawable, pGC, x, y, count, chars, 0),
		    n = GC_OPS(pGC)->PolyText8(pDrawable, pGC, x, y, count, chars));
    return n;
}

int
vmwarePolyText16(DrawablePtr pDrawable,
    GCPtr pGC, int x, int y, int count, unsigned short *chars)
{
    int n;

    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
		    pGC->pScreen,
		    vmwareFontToBox(&BB, pDrawable, pGC, x, y, count, (char*)chars, 1),
		    n = GC_OPS(pGC)->PolyText16(pDrawable, pGC, x, y, count, chars));
    return n;
}

void
vmwareImageText8(DrawablePtr pDrawable,
    GCPtr pGC, int x, int y, int count, char *chars)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
                    pGC->pScreen,
		    vmwareImageFontToBox(&BB, pDrawable, pGC, x, y, count, chars, 0),
		    GC_OPS(pGC)->ImageText8(pDrawable, pGC, x, y, count, chars));
}

void
vmwareImageText16(DrawablePtr pDrawable,
    GCPtr pGC, int x, int y, int count, unsigned short *chars)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
		    pGC->pScreen,
		    vmwareImageFontToBox(&BB, pDrawable, pGC, x, y, count, (char*)chars, 1),
		    GC_OPS(pGC)->ImageText16(pDrawable, pGC, x, y, count, chars));
}
