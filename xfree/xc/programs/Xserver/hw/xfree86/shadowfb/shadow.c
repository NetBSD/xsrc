/*
   Copyright (C) 1999.  The XFree86 Project Inc.

   Written by Mark Vojkovich (mvojkovi@ucsd.edu)
*/

/* $XFree86: xc/programs/Xserver/hw/xfree86/shadowfb/shadow.c,v 1.10 2000/02/08 13:13:33 eich Exp $ */

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "pixmapstr.h"
#include "input.h"
#include "font.h"
#include "mi.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "gcstruct.h"
#include "dixfontstr.h"
#include "fontstruct.h"
#include "xf86.h"
#include "xf86str.h"
#include "shadowfb.h"


static Bool ShadowCloseScreen (int i, ScreenPtr pScreen);
static void ShadowRestoreAreas (    
    PixmapPtr pPixmap,
    RegionPtr prgn,
    int       xorg,
    int       yorg,
    WindowPtr pWin 
);
static void ShadowPaintWindow (
    WindowPtr pWin,
    RegionPtr prgn,
    int what 
);
static void ShadowCopyWindow(
    WindowPtr pWin,
    DDXPointRec ptOldOrg,
    RegionPtr prgn 
);
static Bool ShadowCreateGC(GCPtr pGC);
static Bool ShadowModifyPixmapHeader(
    PixmapPtr pPixmap,
    int width,
    int height,
    int depth,
    int bitsPerPixel,
    int devKind,
    pointer pPixData
);

static Bool ShadowEnterVT(int index, int flags);
static void ShadowLeaveVT(int index, int flags);
static void ShadowEnableDisableFBAccess(int index, Bool enable);


typedef struct {
  ScrnInfoPtr 				pScrn;
  RefreshAreaFuncPtr			refresh;
  CloseScreenProcPtr			CloseScreen;
  PaintWindowBackgroundProcPtr		PaintWindowBackground;
  PaintWindowBorderProcPtr		PaintWindowBorder;
  CopyWindowProcPtr			CopyWindow;
  CreateGCProcPtr			CreateGC;
  BackingStoreRestoreAreasProcPtr	RestoreAreas;  
  ModifyPixmapHeaderProcPtr		ModifyPixmapHeader;
  Bool				(*EnterVT)(int, int);
  void				(*LeaveVT)(int, int);
  void				(*EnableDisableFBAccess)(int, Bool);
  Bool				vtSema;
} ShadowScreenRec, *ShadowScreenPtr;

typedef struct {
   GCOps   *ops;
   GCFuncs *funcs;
} ShadowGCRec, *ShadowGCPtr;


static int ShadowScreenIndex = -1;
static int ShadowGCIndex = -1;
static unsigned long ShadowGeneration = 0;

#define GET_SCREEN_PRIVATE(pScreen) \
	(ShadowScreenPtr)((pScreen)->devPrivates[ShadowScreenIndex].ptr)
#define GET_GC_PRIVATE(pGC) \
	(ShadowGCPtr)((pGC)->devPrivates[ShadowGCIndex].ptr)

#define SHADOW_GC_FUNC_PROLOGUE(pGC)\
    ShadowGCPtr pGCPriv = GET_GC_PRIVATE(pGC);\
    (pGC)->funcs = pGCPriv->funcs;\
    if(pGCPriv->ops)\
        (pGC)->ops = pGCPriv->ops

#define SHADOW_GC_FUNC_EPILOGUE(pGC)\
    pGCPriv->funcs = (pGC)->funcs;\
    (pGC)->funcs = &ShadowGCFuncs;\
    if(pGCPriv->ops) {\
        pGCPriv->ops = (pGC)->ops;\
        (pGC)->ops = &ShadowGCOps;\
    }

#define SHADOW_GC_OP_PROLOGUE(pGC)\
    ShadowScreenPtr pPriv = GET_SCREEN_PRIVATE(pGC->pScreen); \
    ShadowGCPtr pGCPriv = GET_GC_PRIVATE(pGC);\
    GCFuncs *oldFuncs = pGC->funcs;\
    pGC->funcs = pGCPriv->funcs;\
    pGC->ops = pGCPriv->ops

    
#define SHADOW_GC_OP_EPILOGUE(pGC)\
    pGCPriv->ops = pGC->ops;\
    pGC->funcs = oldFuncs;\
    pGC->ops   = &ShadowGCOps

#define IS_VISIBLE(pWin) (pPriv->vtSema && \
    (((WindowPtr)pWin)->visibility != VisibilityFullyObscured))

#define TRIM_BOX(box, pGC) { \
    BoxPtr extents = &pGC->pCompositeClip->extents;\
    if(box.x1 < extents->x1) box.x1 = extents->x1; \
    if(box.x2 > extents->x2) box.x2 = extents->x2; \
    if(box.y1 < extents->y1) box.y1 = extents->y1; \
    if(box.y2 > extents->y2) box.y2 = extents->y2; \
    }

#define TRANSLATE_BOX(box, pDraw) { \
    box.x1 += pDraw->x; \
    box.x2 += pDraw->x; \
    box.y1 += pDraw->y; \
    box.y2 += pDraw->y; \
    }

#define TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC) { \
    TRANSLATE_BOX(box, pDraw); \
    TRIM_BOX(box, pGC); \
    }

#define BOX_NOT_EMPTY(box) \
    (((box.x2 - box.x1) > 0) && ((box.y2 - box.y1) > 0))



Bool
ShadowFBInit (
    ScreenPtr		pScreen,
    RefreshAreaFuncPtr  refreshArea
){
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    ShadowScreenPtr pPriv;

    if(!refreshArea) return FALSE;
    
    if (ShadowGeneration != serverGeneration) {
	if(((ShadowScreenIndex = AllocateScreenPrivateIndex ()) < 0) ||
	   ((ShadowGCIndex = AllocateGCPrivateIndex()) < 0))
	    return FALSE;
	ShadowGeneration = serverGeneration;
    }

    if(!AllocateGCPrivate(pScreen, ShadowGCIndex, sizeof(ShadowGCRec)))
	return FALSE;

    if(!(pPriv = (ShadowScreenPtr)xalloc(sizeof(ShadowScreenRec))))
	return FALSE;

    pScreen->devPrivates[ShadowScreenIndex].ptr = (pointer)pPriv;  

    pPriv->pScrn = pScrn;
    pPriv->refresh = refreshArea;
    pPriv->vtSema = TRUE;

    pPriv->CloseScreen = pScreen->CloseScreen;
    pPriv->PaintWindowBackground = pScreen->PaintWindowBackground;
    pPriv->PaintWindowBorder = pScreen->PaintWindowBorder;
    pPriv->CopyWindow = pScreen->CopyWindow;
    pPriv->CreateGC = pScreen->CreateGC;
    pPriv->RestoreAreas = pScreen->BackingStoreFuncs.RestoreAreas;
    pPriv->ModifyPixmapHeader = pScreen->ModifyPixmapHeader;

    pPriv->EnterVT = pScrn->EnterVT;
    pPriv->LeaveVT = pScrn->LeaveVT;
    pPriv->EnableDisableFBAccess = pScrn->EnableDisableFBAccess;

    pScreen->CloseScreen = ShadowCloseScreen;
    pScreen->PaintWindowBackground = ShadowPaintWindow;
    pScreen->PaintWindowBorder = ShadowPaintWindow;
    pScreen->CopyWindow = ShadowCopyWindow;
    pScreen->CreateGC = ShadowCreateGC;
    pScreen->BackingStoreFuncs.RestoreAreas = ShadowRestoreAreas;
    pScreen->ModifyPixmapHeader = ShadowModifyPixmapHeader;

    pScrn->EnterVT = ShadowEnterVT;
    pScrn->LeaveVT = ShadowLeaveVT;
    pScrn->EnableDisableFBAccess = ShadowEnableDisableFBAccess;

    return TRUE;
}

/**********************************************************/

static Bool
ShadowEnterVT(int index, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[index];
    ShadowScreenPtr pPriv = GET_SCREEN_PRIVATE(pScrn->pScreen);
    BoxRec box;

    if((*pPriv->EnterVT)(index, flags)) {
	pPriv->vtSema = TRUE;

	box.x1 = box.y1 = 0;
	box.x2 = pScrn->pScreen->width;
	box.y2 = pScrn->pScreen->height;

	(*pPriv->refresh)(pScrn, 1, &box);

        return TRUE;
    }

    return FALSE;
}

static void
ShadowLeaveVT(int index, int flags)
{
    ShadowScreenPtr pPriv = GET_SCREEN_PRIVATE(xf86Screens[index]->pScreen);

    pPriv->vtSema = FALSE;

    (*pPriv->LeaveVT)(index, flags);
}

static void
ShadowEnableDisableFBAccess(int index, Bool enable)
{
    /* nothing happens here; nothing touches the real frame buffer */
}


/**********************************************************/


static Bool
ShadowCloseScreen (int i, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    ShadowScreenPtr pPriv = GET_SCREEN_PRIVATE(pScreen);

    pScreen->CloseScreen = pPriv->CloseScreen;
    pScreen->PaintWindowBackground = pPriv->PaintWindowBackground;
    pScreen->PaintWindowBorder = pPriv->PaintWindowBorder;
    pScreen->CopyWindow = pPriv->CopyWindow;
    pScreen->CreateGC = pPriv->CreateGC;
    pScreen->BackingStoreFuncs.RestoreAreas = pPriv->RestoreAreas;
    pScreen->ModifyPixmapHeader = pPriv->ModifyPixmapHeader;

    pScrn->EnterVT = pPriv->EnterVT;
    pScrn->LeaveVT = pPriv->LeaveVT;
    pScrn->EnableDisableFBAccess = pPriv->EnableDisableFBAccess;

    xfree((pointer)pPriv);

    return (*pScreen->CloseScreen) (i, pScreen);
}


static void
ShadowRestoreAreas (    
    PixmapPtr pPixmap,
    RegionPtr prgn,
    int       xorg,
    int       yorg,
    WindowPtr pWin 
){
    ScreenPtr pScreen = pWin->drawable.pScreen;
    ShadowScreenPtr pPriv = GET_SCREEN_PRIVATE(pScreen);
    int num;

    pScreen->BackingStoreFuncs.RestoreAreas = pPriv->RestoreAreas;
    (*pScreen->BackingStoreFuncs.RestoreAreas) (
                pPixmap, prgn, xorg, yorg, pWin);
    pScreen->BackingStoreFuncs.RestoreAreas = ShadowRestoreAreas;

    if(pPriv->vtSema && (num = REGION_NUM_RECTS(prgn)))
	(*pPriv->refresh)(pPriv->pScrn, num, REGION_RECTS(prgn));
}


static void
ShadowPaintWindow(
  WindowPtr pWin,
  RegionPtr prgn,
  int what 
){
    ScreenPtr pScreen = pWin->drawable.pScreen;
    ShadowScreenPtr pPriv = GET_SCREEN_PRIVATE(pScreen);
    int num;

    if(what == PW_BACKGROUND) {
	pScreen->PaintWindowBackground = pPriv->PaintWindowBackground;
	(*pScreen->PaintWindowBackground) (pWin, prgn, what);
	pScreen->PaintWindowBackground = ShadowPaintWindow;
    } else {
	pScreen->PaintWindowBorder = pPriv->PaintWindowBorder;
	(*pScreen->PaintWindowBorder) (pWin, prgn, what);
	pScreen->PaintWindowBorder = ShadowPaintWindow;
    }

    if(pPriv->vtSema && (num = REGION_NUM_RECTS(prgn)))
	(*pPriv->refresh)(pPriv->pScrn, num, REGION_RECTS(prgn));    
}


static void 
ShadowCopyWindow(
   WindowPtr pWin,
   DDXPointRec ptOldOrg,
   RegionPtr prgn 
){
    ScreenPtr pScreen = pWin->drawable.pScreen;
    ShadowScreenPtr pPriv = GET_SCREEN_PRIVATE(pScreen);
    int num;

    pScreen->CopyWindow = pPriv->CopyWindow;
    (*pScreen->CopyWindow) (pWin, ptOldOrg, prgn);
    pScreen->CopyWindow = ShadowCopyWindow;

    if (pPriv->vtSema) {
	/* This is sortof cheating. We rely on the fact that
	   cfb translated prgn for us */

	REGION_INTERSECT(pScreen, prgn, &pWin->borderClip, prgn);
	if((num = REGION_NUM_RECTS(prgn)))
	    (*pPriv->refresh)(pPriv->pScrn, num, REGION_RECTS(prgn));    
    }

}

static Bool
ShadowModifyPixmapHeader(
    PixmapPtr pPixmap,
    int width,
    int height,
    int depth,
    int bitsPerPixel,
    int devKind,
    pointer pPixData
)
{
    ScreenPtr pScreen;
    ScrnInfoPtr pScrn;
    ShadowScreenPtr pPriv;
    Bool retval;
    PixmapPtr pScreenPix;

    if (!pPixmap)
	return FALSE;

    pScreen = pPixmap->drawable.pScreen;
    pScrn = xf86Screens[pScreen->myNum];

    pScreenPix = (*pScreen->GetScreenPixmap)(pScreen);
    
    if (pPixmap == pScreenPix && !pScrn->vtSema)
	pScreenPix->devPrivate = pScrn->pixmapPrivate;
    
    pPriv = GET_SCREEN_PRIVATE(pScreen);

    pScreen->ModifyPixmapHeader = pPriv->ModifyPixmapHeader;
    retval = (*pScreen->ModifyPixmapHeader)(pPixmap,
	width, height, depth, bitsPerPixel, devKind, pPixData);
    pScreen->ModifyPixmapHeader = ShadowModifyPixmapHeader;

    if (pPixmap == pScreenPix && !pScrn->vtSema)
    {
	pScrn->pixmapPrivate = pScreenPix->devPrivate;
	pScreenPix->devPrivate.ptr = 0;
    }
    return retval;
}

/**********************************************************/

static void ShadowValidateGC(GCPtr, unsigned long, DrawablePtr);
static void ShadowChangeGC(GCPtr, unsigned long);
static void ShadowCopyGC(GCPtr, unsigned long, GCPtr);
static void ShadowDestroyGC(GCPtr);
static void ShadowChangeClip(GCPtr, int, pointer, int);
static void ShadowDestroyClip(GCPtr);
static void ShadowCopyClip(GCPtr, GCPtr);

GCFuncs ShadowGCFuncs = {
    ShadowValidateGC, ShadowChangeGC, ShadowCopyGC, ShadowDestroyGC,
    ShadowChangeClip, ShadowDestroyClip, ShadowCopyClip
};


extern GCOps ShadowGCOps;

static Bool
ShadowCreateGC(GCPtr pGC)
{
    ScreenPtr pScreen = pGC->pScreen;
    ShadowScreenPtr pPriv = GET_SCREEN_PRIVATE(pScreen);
    ShadowGCPtr pGCPriv = GET_GC_PRIVATE(pGC);
    Bool ret;
   
    pScreen->CreateGC = pPriv->CreateGC;
    if((ret = (*pScreen->CreateGC) (pGC))) {
	pGCPriv->ops = NULL;
	pGCPriv->funcs = pGC->funcs;
	pGC->funcs = &ShadowGCFuncs;
    }
    pScreen->CreateGC = ShadowCreateGC;

    return ret;
}


static void
ShadowValidateGC(
   GCPtr         pGC,
   unsigned long changes,
   DrawablePtr   pDraw 
){
    SHADOW_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ValidateGC)(pGC, changes, pDraw);
    if(pDraw->type == DRAWABLE_WINDOW)
	pGCPriv->ops = pGC->ops;  /* just so it's not NULL */
    else 
	pGCPriv->ops = NULL;
    SHADOW_GC_FUNC_EPILOGUE (pGC);
}


static void
ShadowDestroyGC(GCPtr pGC)
{
    SHADOW_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->DestroyGC)(pGC);
    SHADOW_GC_FUNC_EPILOGUE (pGC);
}

static void
ShadowChangeGC (
    GCPtr	    pGC,
    unsigned long   mask
){
    SHADOW_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeGC) (pGC, mask);
    SHADOW_GC_FUNC_EPILOGUE (pGC);
}

static void
ShadowCopyGC (
    GCPtr	    pGCSrc, 
    unsigned long   mask,
    GCPtr	    pGCDst
){
    SHADOW_GC_FUNC_PROLOGUE (pGCDst);
    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
    SHADOW_GC_FUNC_EPILOGUE (pGCDst);
}

static void
ShadowChangeClip (
    GCPtr   pGC,
    int		type,
    pointer	pvalue,
    int		nrects 
){
    SHADOW_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);
    SHADOW_GC_FUNC_EPILOGUE (pGC);
}

static void
ShadowCopyClip(GCPtr pgcDst, GCPtr pgcSrc)
{
    SHADOW_GC_FUNC_PROLOGUE (pgcDst);
    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);
    SHADOW_GC_FUNC_EPILOGUE (pgcDst);
}

static void
ShadowDestroyClip(GCPtr pGC)
{
    SHADOW_GC_FUNC_PROLOGUE (pGC);
    (* pGC->funcs->DestroyClip)(pGC);
    SHADOW_GC_FUNC_EPILOGUE (pGC);
}




/**********************************************************/


static void
ShadowFillSpans(
    DrawablePtr pDraw,
    GC		*pGC,
    int		nInit,	
    DDXPointPtr pptInit,	
    int 	*pwidthInit,		
    int 	fSorted 
){
    SHADOW_GC_OP_PROLOGUE(pGC);    

    if(IS_VISIBLE(pDraw) && nInit) {
	DDXPointPtr ppt = pptInit;
	int *pwidth = pwidthInit;
	int i = nInit;
	BoxRec box;

	box.x1 = ppt->x;
	box.x2 = box.x1 + *pwidth;
	box.y2 = box.y1 = ppt->y;

	while(--i) {
	   ppt++;
	   pwidthInit++;
	   if(box.x1 > ppt->x) box.x1 = ppt->x;
	   if(box.x2 < (ppt->x + *pwidth)) 
		box.x2 = ppt->x + *pwidth;
	   if(box.y1 > ppt->y) box.y1 = ppt->y;
	   else if(box.y2 < ppt->y) box.y2 = ppt->y;
	}

	box.y2++;

	(*pGC->ops->FillSpans)(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted);

	TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    } else
	(*pGC->ops->FillSpans)(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted);

    SHADOW_GC_OP_EPILOGUE(pGC);
}

static void
ShadowSetSpans(
    DrawablePtr		pDraw,
    GCPtr		pGC,
    char		*pcharsrc,
    DDXPointPtr 	pptInit,
    int			*pwidthInit,
    int			nspans,
    int			fSorted 
){
    SHADOW_GC_OP_PROLOGUE(pGC);

    if(IS_VISIBLE(pDraw) && nspans) {
	DDXPointPtr ppt = pptInit;
	int *pwidth = pwidthInit;
	int i = nspans;
	BoxRec box;

	box.x1 = ppt->x;
	box.x2 = box.x1 + *pwidth;
	box.y2 = box.y1 = ppt->y;

	while(--i) {
	   ppt++;
	   pwidth++;
	   if(box.x1 > ppt->x) box.x1 = ppt->x;
	   if(box.x2 < (ppt->x + *pwidth)) 
		box.x2 = ppt->x + *pwidth;
	   if(box.y1 > ppt->y) box.y1 = ppt->y;
	   else if(box.y2 < ppt->y) box.y2 = ppt->y;
	}

	box.y2++;

	(*pGC->ops->SetSpans)(pDraw, pGC, pcharsrc, pptInit, 
				pwidthInit, nspans, fSorted);

	TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    } else
	(*pGC->ops->SetSpans)(pDraw, pGC, pcharsrc, pptInit, 
				pwidthInit, nspans, fSorted);

    SHADOW_GC_OP_EPILOGUE(pGC);
}

static void
ShadowPutImage(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		depth, 
    int x, int y, int w, int h,
    int		leftPad,
    int		format,
    char 	*pImage 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PutImage)(pDraw, pGC, depth, x, y, w, h, 
		leftPad, format, pImage);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw)) {
	BoxRec box;

	box.x1 = x + pDraw->x;
	box.x2 = box.x1 + w;
	box.y1 = y + pDraw->y;
	box.y2 = box.y1 + h;

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}

static RegionPtr
ShadowCopyArea(
    DrawablePtr pSrc,
    DrawablePtr pDst,
    GC *pGC,
    int srcx, int srcy,
    int width, int height,
    int dstx, int dsty 
){
    RegionPtr ret;
    SHADOW_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->CopyArea)(pSrc, pDst,
            pGC, srcx, srcy, width, height, dstx, dsty);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDst)) {
	BoxRec box;

	box.x1 = dstx + pDst->x;
	box.x2 = box.x1 + width;
	box.y1 = dsty + pDst->y;
	box.y2 = box.y1 + height;

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }

    return ret;
}

static RegionPtr
ShadowCopyPlane(
    DrawablePtr	pSrc,
    DrawablePtr	pDst,
    GCPtr pGC,
    int	srcx, int srcy,
    int	width, int height,
    int	dstx, int dsty,
    unsigned long bitPlane 
){
    RegionPtr ret;
    SHADOW_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->CopyPlane)(pSrc, pDst,
	       pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDst)) {
	BoxRec box;

	box.x1 = dstx + pDst->x;
	box.x2 = box.x1 + width;
	box.y1 = dsty + pDst->y;
	box.y2 = box.y1 + height;

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }

    return ret;
}

static void
ShadowPolyPoint(
    DrawablePtr pDraw,
    GCPtr pGC,
    int mode,
    int npt,
    xPoint *pptInit 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyPoint)(pDraw, pGC, mode, npt, pptInit);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw) && npt) {
	BoxRec box;

	box.x2 = box.x1 = pptInit->x;
	box.y2 = box.y1 = pptInit->y;

	/* this could be slow if the points were spread out */

	while(--npt) {
	   pptInit++;
	   if(box.x1 > pptInit->x) box.x1 = pptInit->x;
	   else if(box.x2 < pptInit->x) box.x2 = pptInit->x;
	   if(box.y1 > pptInit->y) box.y1 = pptInit->y;
	   else if(box.y2 < pptInit->y) box.y2 = pptInit->y;
	}

	box.x2++;
	box.y2++;

	TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}

static void
ShadowPolylines(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		mode,		
    int		npt,		
    DDXPointPtr pptInit 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->Polylines)(pDraw, pGC, mode, npt, pptInit);
    SHADOW_GC_OP_EPILOGUE(pGC);


    if(IS_VISIBLE(pDraw) && npt) {
	BoxRec box;
	int extra = pGC->lineWidth >> 1;

	box.x2 = box.x1 = pptInit->x;
	box.y2 = box.y1 = pptInit->y;

	if(npt > 1) {
	   if(pGC->joinStyle == JoinMiter)
		extra = 6 * pGC->lineWidth;
	   else if(pGC->capStyle == CapProjecting)
		extra = pGC->lineWidth;
        }

	if(mode == CoordModePrevious) {
	   int x = box.x1;
	   int y = box.y1;
	   while(--npt) {
		pptInit++;
		x += pptInit->x;
		y += pptInit->y;
		if(box.x1 > x) box.x1 = x;
		else if(box.x2 < x) box.x2 = x;
		if(box.y1 > y) box.y1 = y;
		else if(box.y2 < y) box.y2 = y;
	    }
	} else {
	   while(--npt) {
		pptInit++;
		if(box.x1 > pptInit->x) box.x1 = pptInit->x;
		else if(box.x2 < pptInit->x) box.x2 = pptInit->x;
		if(box.y1 > pptInit->y) box.y1 = pptInit->y;
		else if(box.y2 < pptInit->y) box.y2 = pptInit->y;
	    }
	}

	box.x2++;
	box.y2++;

	if(extra) {
	   box.x1 -= extra;
	   box.x2 += extra;
	   box.y1 -= extra;
	   box.y2 += extra;
        }

	TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}

static void 
ShadowPolySegment(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nseg,
    xSegment	*pSeg 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolySegment)(pDraw, pGC, nseg, pSeg);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw) && nseg) {
	BoxRec box;
	int extra = pGC->lineWidth;

        if(pGC->capStyle != CapProjecting)	
	   extra >>= 1;

	if(pSeg->x2 > pSeg->x1) {
	    box.x1 = pSeg->x1;
	    box.x2 = pSeg->x2;
	} else {
	    box.x2 = pSeg->x1;
	    box.x1 = pSeg->x2;
	}

	if(pSeg->y2 > pSeg->y1) {
	    box.y1 = pSeg->y1;
	    box.y2 = pSeg->y2;
	} else {
	    box.y2 = pSeg->y1;
	    box.y1 = pSeg->y2;
	}

	while(--nseg) {
	    pSeg++;
	    if(pSeg->x2 > pSeg->x1) {
		if(pSeg->x1 < box.x1) box.x1 = pSeg->x1;
		if(pSeg->x2 > box.x2) box.x2 = pSeg->x2;
	    } else {
		if(pSeg->x2 < box.x1) box.x1 = pSeg->x2;
		if(pSeg->x1 > box.x2) box.x2 = pSeg->x1;
	    }
	    if(pSeg->y2 > pSeg->y1) {
		if(pSeg->y1 < box.y1) box.y1 = pSeg->y1;
		if(pSeg->y2 > box.y2) box.y2 = pSeg->y2;
	    } else {
		if(pSeg->y2 < box.y1) box.y1 = pSeg->y2;
		if(pSeg->y1 > box.y2) box.y2 = pSeg->y1;
	    }
	}

	box.x2++;
	box.y2++;

	if(extra) {
	   box.x1 -= extra;
	   box.x2 += extra;
	   box.y1 -= extra;
	   box.y2 += extra;
        }

	TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}

static void
ShadowPolyRectangle(
    DrawablePtr  pDraw,
    GCPtr        pGC,
    int	         nRects,
    xRectangle  *pRects 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyRectangle)(pDraw, pGC, nRects, pRects);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw) && nRects) {
	if(nRects >= 32) {
	    int extra = pGC->lineWidth >> 1;
	    BoxRec box;

	    box.x1 = pRects->x;
	    box.x2 = box.x1 + pRects->width;
	    box.y1 = pRects->y;
	    box.y2 = box.y1 + pRects->height;

	    while(--nRects) {
		pRects++;
		if(box.x1 > pRects->x) box.x1 = pRects->x;
		if(box.x2 < (pRects->x + pRects->width))
			box.x2 = pRects->x + pRects->width;
		if(box.y1 > pRects->y) box.y1 = pRects->y;
		if(box.y2 < (pRects->y + pRects->height))
			box.y2 = pRects->y + pRects->height;
	    }

	    if(extra) {
		box.x1 -= extra;
		box.x2 += extra;
		box.y1 -= extra;
		box.y2 += extra;
	    }

	    box.x2++;
	    box.y2++;

	    TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	    if(BOX_NOT_EMPTY(box))
		(*pPriv->refresh)(pPriv->pScrn, 1, &box);
	} else {
	    BoxPtr pbox, pBoxInit;
	    int offset1, offset2, offset3;
	    int num = 0;

	    offset2 = pGC->lineWidth;
	    if(!offset2) offset2 = 1;
	    offset1 = offset2 >> 1;
	    offset3 = offset2 - offset1;

	    pBoxInit = (BoxPtr)ALLOCATE_LOCAL(nRects * 4 * sizeof(BoxRec));
	    pbox = pBoxInit;

	    while(nRects--) {
		pbox->x1 = pRects->x - offset1;
		pbox->y1 = pRects->y - offset1;
		pbox->x2 = pbox->x1 + pRects->width + offset2;
		pbox->y2 = pbox->y1 + offset2;		
		TRIM_AND_TRANSLATE_BOX((*pbox), pDraw, pGC);
		if(BOX_NOT_EMPTY((*pbox))) {
		   num++;
		   pbox++;
		}

		pbox->x1 = pRects->x - offset1;
		pbox->y1 = pRects->y + offset3;
		pbox->x2 = pbox->x1 + offset2;
		pbox->y2 = pbox->y1 + pRects->height - offset2;		
		TRIM_AND_TRANSLATE_BOX((*pbox), pDraw, pGC);
		if(BOX_NOT_EMPTY((*pbox))) {
		   num++;
		   pbox++;
		}

		pbox->x1 = pRects->x + pRects->width - offset1;
		pbox->y1 = pRects->y + offset3;
		pbox->x2 = pbox->x1 + offset2;
		pbox->y2 = pbox->y1 + pRects->height - offset2;		
		TRIM_AND_TRANSLATE_BOX((*pbox), pDraw, pGC);
		if(BOX_NOT_EMPTY((*pbox))) {
		   num++;
		   pbox++;
		}

		pbox->x1 = pRects->x - offset1;
		pbox->y1 = pRects->y + pRects->height - offset1;
		pbox->x2 = pbox->x1 + pRects->width + offset2;
		pbox->y2 = pbox->y1 + offset2;		
		TRIM_AND_TRANSLATE_BOX((*pbox), pDraw, pGC);
		if(BOX_NOT_EMPTY((*pbox))) {
		   num++;
		   pbox++;
		}

		pRects++;
	    }
	    
	    if(num)
		(*pPriv->refresh)(pPriv->pScrn, num, pBoxInit);

	    DEALLOCATE_LOCAL(pBoxInit);
	}
    }
 }

static void
ShadowPolyArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyArc)(pDraw, pGC, narcs, parcs);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw) && narcs) {
	int extra = pGC->lineWidth >> 1;
	BoxRec box;

	box.x1 = parcs->x;
	box.x2 = box.x1 + parcs->width;
	box.y1 = parcs->y;
	box.y2 = box.y1 + parcs->height;

	/* should I break these up instead ? */

	while(--narcs) {
	   parcs++;
	   if(box.x1 > parcs->x) box.x1 = parcs->x;
	   if(box.x2 < (parcs->x + parcs->width))
		box.x2 = parcs->x + parcs->width;
	   if(box.y1 > parcs->y) box.y1 = parcs->y;
	   if(box.y2 < (parcs->y + parcs->height))
		box.y2 = parcs->y + parcs->height;
        }

	if(extra) {
	   box.x1 -= extra;
	   box.x2 += extra;
	   box.y1 -= extra;
	   box.y2 += extra;
        }

	box.x2++;
	box.y2++;

	TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}

static void
ShadowFillPolygon(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		shape,
    int		mode,
    int		count,
    DDXPointPtr	pptInit 
){
    SHADOW_GC_OP_PROLOGUE(pGC);

    if(IS_VISIBLE(pDraw) && (count > 2)) {
	DDXPointPtr ppt = pptInit;
	int i = count;
	BoxRec box;

	box.x2 = box.x1 = ppt->x;
	box.y2 = box.y1 = ppt->y;

	if(mode != CoordModeOrigin) {
	   int x = box.x1;
	   int y = box.y1;
	   while(--i) {
		ppt++;
		x += ppt->x;
		y += ppt->y;
		if(box.x1 > x) box.x1 = x;
		else if(box.x2 < x) box.x2 = x;
		if(box.y1 > y) box.y1 = y;
		else if(box.y2 < y) box.y2 = y;
	    }
	} else {
	   while(--i) {
		ppt++;
		if(box.x1 > ppt->x) box.x1 = ppt->x;
		else if(box.x2 < ppt->x) box.x2 = ppt->x;
		if(box.y1 > ppt->y) box.y1 = ppt->y;
		else if(box.y2 < ppt->y) box.y2 = ppt->y;
	    }
	}

	box.x2++;
	box.y2++;

	(*pGC->ops->FillPolygon)(pDraw, pGC, shape, mode, count, pptInit);

	TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    } else
	(*pGC->ops->FillPolygon)(pDraw, pGC, shape, mode, count, pptInit);

    SHADOW_GC_OP_EPILOGUE(pGC);
}


static void 
ShadowPolyFillRect(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nRectsInit, 
    xRectangle	*pRectsInit 
){
    SHADOW_GC_OP_PROLOGUE(pGC);

    if(IS_VISIBLE(pDraw) && nRectsInit) {
	BoxRec box;
	xRectangle *pRects = pRectsInit;
	int nRects = nRectsInit;

	box.x1 = pRects->x;
	box.x2 = box.x1 + pRects->width;
	box.y1 = pRects->y;
	box.y2 = box.y1 + pRects->height;

	while(--nRects) {
	    pRects++;
	    if(box.x1 > pRects->x) box.x1 = pRects->x;
	    if(box.x2 < (pRects->x + pRects->width))
		box.x2 = pRects->x + pRects->width;
	    if(box.y1 > pRects->y) box.y1 = pRects->y;
	    if(box.y2 < (pRects->y + pRects->height))
		box.y2 = pRects->y + pRects->height;
	}

	/* cfb messes with the pRectsInit so we have to do our
	   calculations first */

	(*pGC->ops->PolyFillRect)(pDraw, pGC, nRectsInit, pRectsInit);

	TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	if(BOX_NOT_EMPTY(box))
	    (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    } else
	(*pGC->ops->PolyFillRect)(pDraw, pGC, nRectsInit, pRectsInit);

    SHADOW_GC_OP_EPILOGUE(pGC);
}


static void
ShadowPolyFillArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyFillArc)(pDraw, pGC, narcs, parcs);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw) && narcs) {
	BoxRec box;

	box.x1 = parcs->x;
	box.x2 = box.x1 + parcs->width;
	box.y1 = parcs->y;
	box.y2 = box.y1 + parcs->height;

	/* should I break these up instead ? */

	while(--narcs) {
	   parcs++;
	   if(box.x1 > parcs->x) box.x1 = parcs->x;
	   if(box.x2 < (parcs->x + parcs->width))
		box.x2 = parcs->x + parcs->width;
	   if(box.y1 > parcs->y) box.y1 = parcs->y;
	   if(box.y2 < (parcs->y + parcs->height))
		box.y2 = parcs->y + parcs->height;
        }

	TRIM_AND_TRANSLATE_BOX(box, pDraw, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }

}

static int
ShadowPolyText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int 	y,
    int 	count,
    char	*chars 
){
    int width;

    SHADOW_GC_OP_PROLOGUE(pGC);
    width = (*pGC->ops->PolyText8)(pDraw, pGC, x, y, count, chars);
    SHADOW_GC_OP_EPILOGUE(pGC);

    width -= x;

    if(IS_VISIBLE(pDraw) && (width > 0)) {
	BoxRec box;

	/* ugh */
	box.x1 = pDraw->x + x + FONTMINBOUNDS(pGC->font, leftSideBearing);
	box.x2 = pDraw->x + x + FONTMAXBOUNDS(pGC->font, rightSideBearing);

	if(count > 1) {
	   if(width > 0) box.x2 += width;
	   else box.x1 += width;
	}

	box.y1 = pDraw->y + y - FONTMAXBOUNDS(pGC->font, ascent);
	box.y2 = pDraw->y + y + FONTMAXBOUNDS(pGC->font, descent);

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }

    return (width + x);
}

static int
ShadowPolyText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars 
){
    int width;

    SHADOW_GC_OP_PROLOGUE(pGC);
    width = (*pGC->ops->PolyText16)(pDraw, pGC, x, y, count, chars);
    SHADOW_GC_OP_EPILOGUE(pGC);

    width -= x;

    if(IS_VISIBLE(pDraw) && (width > 0)) {
	BoxRec box;

	/* ugh */
	box.x1 = pDraw->x + x + FONTMINBOUNDS(pGC->font, leftSideBearing);
	box.x2 = pDraw->x + x + FONTMAXBOUNDS(pGC->font, rightSideBearing);

	if(count > 1) {
	   if(width > 0) box.x2 += width;
	   else box.x1 += width;
	}

	box.y1 = pDraw->y + y - FONTMAXBOUNDS(pGC->font, ascent);
	box.y2 = pDraw->y + y + FONTMAXBOUNDS(pGC->font, descent);

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }

    return (width + x);
}

static void
ShadowImageText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int		y,
    int 	count,
    char	*chars 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->ImageText8)(pDraw, pGC, x, y, count, chars);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw) && count) {
	int top, bot, Min, Max;
	BoxRec box;

	top = max(FONTMAXBOUNDS(pGC->font, ascent), FONTASCENT(pGC->font));
	bot = max(FONTMAXBOUNDS(pGC->font, descent), FONTDESCENT(pGC->font));

	Min = count * FONTMINBOUNDS(pGC->font, characterWidth);
	if(Min > 0) Min = 0;
	Max = count * FONTMAXBOUNDS(pGC->font, characterWidth);	
	if(Max < 0) Max = 0;

	/* ugh */
	box.x1 = pDraw->x + x + Min +
		FONTMINBOUNDS(pGC->font, leftSideBearing);
	box.x2 = pDraw->x + x + Max + 
		FONTMAXBOUNDS(pGC->font, rightSideBearing);

	box.y1 = pDraw->y + y - top;
	box.y2 = pDraw->y + y + bot;

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}
static void
ShadowImageText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->ImageText16)(pDraw, pGC, x, y, count, chars);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw) && count) {
	int top, bot, Min, Max;
	BoxRec box;

	top = max(FONTMAXBOUNDS(pGC->font, ascent), FONTASCENT(pGC->font));
	bot = max(FONTMAXBOUNDS(pGC->font, descent), FONTDESCENT(pGC->font));

	Min = count * FONTMINBOUNDS(pGC->font, characterWidth);
	if(Min > 0) Min = 0;
	Max = count * FONTMAXBOUNDS(pGC->font, characterWidth);	
	if(Max < 0) Max = 0;

	/* ugh */
	box.x1 = pDraw->x + x + Min +
		FONTMINBOUNDS(pGC->font, leftSideBearing);
	box.x2 = pDraw->x + x + Max + 
		FONTMAXBOUNDS(pGC->font, rightSideBearing);

	box.y1 = pDraw->y + y - top;
	box.y2 = pDraw->y + y + bot;

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}


static void
ShadowImageGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int x, int y,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, nglyph, 
					ppci, pglyphBase);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw) && nglyph) {
	int top, bot, width = 0;
	BoxRec box;

	top = max(FONTMAXBOUNDS(pGC->font, ascent), FONTASCENT(pGC->font));
	bot = max(FONTMAXBOUNDS(pGC->font, descent), FONTDESCENT(pGC->font));

	box.x1 = ppci[0]->metrics.leftSideBearing;
	if(box.x1 > 0) box.x1 = 0;
	box.x2 = ppci[nglyph - 1]->metrics.rightSideBearing - 
		ppci[nglyph - 1]->metrics.characterWidth;
	if(box.x2 < 0) box.x2 = 0;

	box.x2 += pDraw->x + x;
	box.x1 += pDraw->x + x;
	   
	while(nglyph--) {
	    width += (*ppci)->metrics.characterWidth;
	    ppci++;
	}

	if(width > 0) 
	   box.x2 += width;
	else 
	   box.x1 += width;

	box.y1 = pDraw->y + y - top;
	box.y2 = pDraw->y + y + bot;

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}

static void
ShadowPolyGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int x, int y,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyGlyphBlt)(pDraw, pGC, x, y, nglyph, 
				ppci, pglyphBase);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw) && nglyph) {
	BoxRec box;

	/* ugh */
	box.x1 = pDraw->x + x + ppci[0]->metrics.leftSideBearing;
	box.x2 = pDraw->x + x + ppci[nglyph - 1]->metrics.rightSideBearing;

	if(nglyph > 1) {
	    int width = 0;

	    while(--nglyph) { 
		width += (*ppci)->metrics.characterWidth;
		ppci++;
	    }
	
	    if(width > 0) box.x2 += width;
	    else box.x1 += width;
	}

	box.y1 = pDraw->y + y - FONTMAXBOUNDS(pGC->font, ascent);
	box.y2 = pDraw->y + y + FONTMAXBOUNDS(pGC->font, descent);

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}

static void
ShadowPushPixels(
    GCPtr	pGC,
    PixmapPtr	pBitMap,
    DrawablePtr pDraw,
    int	dx, int dy, int xOrg, int yOrg 
){
    SHADOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PushPixels)(pGC, pBitMap, pDraw, dx, dy, xOrg, yOrg);
    SHADOW_GC_OP_EPILOGUE(pGC);

    if(IS_VISIBLE(pDraw)) {
	BoxRec box;

	box.x1 = xOrg + pDraw->x;
	box.x2 = box.x1 + dx;
	box.y1 = yOrg + pDraw->y;
	box.y2 = box.y1 + dy;

	TRIM_BOX(box, pGC);
	if(BOX_NOT_EMPTY(box))
	   (*pPriv->refresh)(pPriv->pScrn, 1, &box);
    }
}


GCOps ShadowGCOps = {
    ShadowFillSpans, ShadowSetSpans, 
    ShadowPutImage, ShadowCopyArea, 
    ShadowCopyPlane, ShadowPolyPoint, 
    ShadowPolylines, ShadowPolySegment, 
    ShadowPolyRectangle, ShadowPolyArc, 
    ShadowFillPolygon, ShadowPolyFillRect, 
    ShadowPolyFillArc, ShadowPolyText8, 
    ShadowPolyText16, ShadowImageText8, 
    ShadowImageText16, ShadowImageGlyphBlt, 
    ShadowPolyGlyphBlt, ShadowPushPixels,
#ifdef NEED_LINEHELPER
    NULL,
#endif
    {NULL}		/* devPrivate */
};

