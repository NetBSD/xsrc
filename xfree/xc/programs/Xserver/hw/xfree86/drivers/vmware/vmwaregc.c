/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwaregc[] =

    "Id: vmwaregc.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwaregc.c,v 1.2 2001/05/16 06:48:12 keithp Exp $ */

#include "X.h"
#include "fb.h"
#include "mi.h"
#include "migc.h"
#include "vmware.h"

GCFuncs vmwareGCFuncs = {
    vmwareValidateGC,
    vmwareChangeGC,
    vmwareCopyGC,
    vmwareDestroyGC,
    vmwareChangeClip,
    vmwareDestroyClip,
    vmwareCopyClip
};

GCOps vmwareGCOps = {
    vmwareFillSpans,
    vmwareSetSpans,
    vmwarePutImage,
    vmwareCopyArea,
    vmwareCopyPlane,
    vmwarePolyPoint,
    vmwarePolylines,
    vmwarePolySegment,
    vmwarePolyRectangle,
    vmwarePolyArc,
    vmwareFillPolygon,
    vmwarePolyFillRect,
    vmwarePolyFillArc,
    vmwarePolyText8,
    vmwarePolyText16,
    vmwareImageText8,
    vmwareImageText16,
    vmwareImageGlyphBlt,
    vmwarePolyGlyphBlt,
    vmwarePushPixels,
#ifdef NEED_LINEHELPER
    , NULL
#endif
};

unsigned long Pmsk;

Bool
vmwareCreateGC(GCPtr pGC)
{
    Bool ret;

    TRACEPOINT
    GC_FUNC_PROLOGUE(pGC);
    ret = VMWAREPTR(infoFromScreen(pGC->pScreen))->ScrnFuncs.CreateGC(pGC);
    GC_FUNC_EPILOGUE(pGC);
    return ret;
}

void
vmwareValidateGC(GCPtr pGC, unsigned long changes, DrawablePtr pDrawable)
{
    TRACEPOINT
    GC_FUNC_PROLOGUE(pGC);
    pGC->funcs->ValidateGC(pGC, changes, pDrawable);
    GC_FUNC_EPILOGUE(pGC);
}

void
vmwareChangeGC(GCPtr pGC, unsigned long changes)
{
    TRACEPOINT
    GC_FUNC_PROLOGUE(pGC);
    pGC->funcs->ChangeGC(pGC, changes);
    GC_FUNC_EPILOGUE(pGC);
}

void
vmwareCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst)
{
    TRACEPOINT
    GC_FUNC_PROLOGUE(pGCDst);
    pGCDst->funcs->CopyGC(pGCSrc, mask, pGCDst);
    GC_FUNC_EPILOGUE(pGCDst);
}

void
vmwareDestroyGC(GCPtr pGC)
{
    TRACEPOINT
    GC_FUNC_PROLOGUE(pGC);
    pGC->funcs->DestroyGC(pGC);
    GC_FUNC_EPILOGUE(pGC);
}

void
vmwareChangeClip(GCPtr pGC, int type, pointer pValue, int nrects)
{
    TRACEPOINT
    GC_FUNC_PROLOGUE(pGC);
    pGC->funcs->ChangeClip(pGC, type, pValue, nrects);
    GC_FUNC_EPILOGUE(pGC);
}

void
vmwareDestroyClip(GCPtr pGC)
{
    TRACEPOINT
    GC_FUNC_PROLOGUE(pGC);
    pGC->funcs->DestroyClip(pGC);
    GC_FUNC_EPILOGUE(pGC);
}

void
vmwareCopyClip(GCPtr pGCDst, GCPtr pGCSrc)
{
    TRACEPOINT
    GC_FUNC_PROLOGUE(pGCDst);
    pGCDst->funcs->CopyClip(pGCDst, pGCSrc);
    GC_FUNC_EPILOGUE(pGCDst);
}
