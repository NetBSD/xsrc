/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwarefs[] =

    "Id: vmwarefs.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwarefs.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"

static __inline void
vmwareDrawableToBox(BoxPtr BB, const DrawablePtr pDrawable)
{
    BB->x2 = (BB->x1 = pDrawable->x) + pDrawable->width;
    BB->y2 = (BB->y1 = pDrawable->y) + pDrawable->height;
}

void
vmwareFillSpans(DrawablePtr pDrawable,
    GCPtr pGC, int nInit, DDXPointPtr pptInit, int *pwidthInit, int fSorted)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
                    pDrawable->pScreen,
		    vmwareDrawableToBox(&BB, pDrawable),
		    GC_OPS(pGC)->FillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted));
}

void
vmwareSetSpans(DrawablePtr pDrawable,
    GCPtr pGC,
    char *pcharsrc, DDXPointPtr ppt, int *pwidth, int nspans, int fSorted)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
    		    pDrawable->pScreen,
		    vmwareDrawableToBox(&BB, pDrawable),
		    GC_OPS(pGC)->SetSpans(pDrawable, pGC, pcharsrc, ppt, pwidth, nspans, fSorted));
}

void
vmwareGetSpans(DrawablePtr pDrawable,
    int wMax, DDXPointPtr pPoints, int *pWidths, int nSpans, char *pDst)
{
    VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(pDrawable->pScreen));

    TRACEPOINT
    
    VM_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
                    (BB.x1 = 0, BB.y1 = 0, 
		     BB.x2 = pDrawable->pScreen->width,
		     BB.y2 = pDrawable->pScreen->height),
		    pVMWARE->ScrnFuncs.GetSpans(pDrawable, wMax, pPoints, pWidths, nSpans, pDst)); 
}




