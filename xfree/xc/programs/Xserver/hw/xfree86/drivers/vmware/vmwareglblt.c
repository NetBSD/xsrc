/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwareglblt[] =

    "Id: vmwareglblt.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwareglblt.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"

static __inline void
vmwareDrawableToBox(BoxPtr BB, const DrawablePtr pDrawable)
{
    BB->x2 = (BB->x1 = pDrawable->x) + pDrawable->width;
    BB->y2 = (BB->y1 = pDrawable->y) + pDrawable->height;
}

void
vmwareImageGlyphBlt(DrawablePtr pDrawable,
    GCPtr pGC,
    int x, int y, unsigned int nglyph, CharInfoPtr * ppci, pointer pglyphBase)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
		    pGC->pScreen,
		    vmwareDrawableToBox(&BB, pDrawable),
		    GC_OPS(pGC)->ImageGlyphBlt(pDrawable, pGC, x, y,
			nglyph, ppci, pglyphBase));
}

void
vmwarePolyGlyphBlt(DrawablePtr pDrawable,
    GCPtr pGC,
    int x, int y, unsigned int nglyph, CharInfoPtr * ppci, pointer pglyphBase)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
		    pGC->pScreen,
		    vmwareDrawableToBox(&BB, pDrawable),
		    GC_OPS(pGC)->PolyGlyphBlt(pDrawable, pGC, x, y,
			nglyph, ppci, pglyphBase));
}
