/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwareimage[] =

    "Id: vmwareimage.c,v 1.3 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwareimage.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"

/*
 * vmwarePutImage does not optimize ops because a downstream call is made
 * to CopyArea.
 */

static __inline void
vmwareImageToBox(BoxPtr BB, DrawablePtr pDrawable, int x, int y, int w, int h)
{
    BB->x2 = (BB->x1 = pDrawable->x + x) + w;
    BB->y2 = (BB->y1 = pDrawable->y + y) + h;
}

void
vmwarePutImage(DrawablePtr pDrawable,
    GCPtr pGC,
    int depth,
    int x, int y, int w, int h, int leftPad, int format, char *pImage)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
    		    pGC->pScreen,
		    vmwareImageToBox(&BB, pDrawable, x, y, w, h),
		    GC_OPS(pGC)->PutImage(pDrawable, pGC, depth, x, y,
			w, h, leftPad, format, pImage));
}

void
vmwareGetImage(DrawablePtr pDrawable,
    int x,
    int y,
    int w,
    int h, unsigned int format, unsigned long planeMask, char *pBinImage)
{
    VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(pDrawable->pScreen));

    TRACEPOINT
    
    VM_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
    		    vmwareImageToBox(&BB, pDrawable, x, y, w, h),
		    pVMWARE->ScrnFuncs.GetImage(pDrawable, x, y, w, h, format, planeMask, pBinImage));
}




