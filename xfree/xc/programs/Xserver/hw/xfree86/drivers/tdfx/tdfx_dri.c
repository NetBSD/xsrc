/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/tdfx/tdfx_dri.c,v 1.14 2000/12/07 20:26:23 dawes Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Priv.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "fb.h"
#include "miline.h"
#include "GL/glxtokens.h"
#include "tdfx.h"
#include "tdfx_dri.h"
#include "tdfx_dripriv.h"

static char TDFXKernelDriverName[] = "tdfx";
static char TDFXClientDriverName[] = "tdfx";

static Bool TDFXCreateContext(ScreenPtr pScreen, VisualPtr visual, 
			      drmContext hwContext, void *pVisualConfigPriv,
			      DRIContextType contextStore);
static void TDFXDestroyContext(ScreenPtr pScreen, drmContext hwContext,
			       DRIContextType contextStore);
static void TDFXDRISwapContext(ScreenPtr pScreen, DRISyncType syncType, 
			       DRIContextType readContextType, 
			       void *readContextStore,
			       DRIContextType writeContextType, 
			       void *writeContextStore);
static Bool TDFXDRIOpenFullScreen(ScreenPtr pScreen);
static Bool TDFXDRICloseFullScreen(ScreenPtr pScreen);
static void TDFXDRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index);
static void TDFXDRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg, 
			       RegionPtr prgnSrc, CARD32 index);

static Bool
TDFXInitVisualConfigs(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  TDFXPtr pTDFX = TDFXPTR(pScrn);
  int numConfigs = 0;
  __GLXvisualConfig *pConfigs = 0;
  TDFXConfigPrivPtr pTDFXConfigs = 0;
  TDFXConfigPrivPtr *pTDFXConfigPtrs = 0;
  int i, db, stencil, accum, depth;

  switch (pScrn->bitsPerPixel) {
  case 8:
  case 16:
    numConfigs = 16;

    if (!(pConfigs = (__GLXvisualConfig*)xnfcalloc(sizeof(__GLXvisualConfig),
						   numConfigs))) {
      return FALSE;
    }
    if (!(pTDFXConfigs = (TDFXConfigPrivPtr)xnfcalloc(sizeof(TDFXConfigPrivRec),
						     numConfigs))) {
      xfree(pConfigs);
      return FALSE;
    }
    if (!(pTDFXConfigPtrs = (TDFXConfigPrivPtr*)xnfcalloc(sizeof(TDFXConfigPrivPtr),
							 numConfigs))) {
      xfree(pConfigs);
      xfree(pTDFXConfigs);
      return FALSE;
    }
    for (i=0; i<numConfigs; i++) 
      pTDFXConfigPtrs[i] = &pTDFXConfigs[i];

    i=0;
    depth=1;
    for (db = 0; db <=1; db++) {
      for (depth = 0; depth<=1; depth++) {
	for (accum = 0; accum <= 1; accum++) {
	  for (stencil = 0; stencil <= 1; stencil++) {
	    pConfigs[i].vid = -1;
	    pConfigs[i].class = -1;
	    pConfigs[i].rgba = TRUE;
	    pConfigs[i].redSize = 5;
	    pConfigs[i].greenSize = 6;
	    pConfigs[i].blueSize = 5;
	    pConfigs[i].redMask = 0x0000F800;
	    pConfigs[i].greenMask = 0x000007E0;
	    pConfigs[i].blueMask = 0x0000001F;
	    pConfigs[i].alphaMask = 0;
	    if (accum) {
	      pConfigs[i].accumRedSize = 16;
	      pConfigs[i].accumGreenSize = 16;
	      pConfigs[i].accumBlueSize = 16;
	      pConfigs[i].accumAlphaSize = 0;
	    } else {
	      pConfigs[i].accumRedSize = 0;
	      pConfigs[i].accumGreenSize = 0;
	      pConfigs[i].accumBlueSize = 0;
	      pConfigs[i].accumAlphaSize = 0;
	    }
	    if (db)
	      pConfigs[i].doubleBuffer = TRUE;
	    else
	      pConfigs[i].doubleBuffer = FALSE;
	    pConfigs[i].stereo = FALSE;
	    pConfigs[i].bufferSize = 16;
	    if (depth) {
	      if (pTDFX->cpp>2)
		pConfigs[i].depthSize = 24;
	      else 
		pConfigs[i].depthSize = 16;
	    } else {
	      pConfigs[i].depthSize = 0;
	    }
	    if (stencil)
	      pConfigs[i].stencilSize = 8;
	    else
	      pConfigs[i].stencilSize = 0;
	    pConfigs[i].auxBuffers = 0;
	    pConfigs[i].level = 0;
	    if (stencil || accum)
	      pConfigs[i].visualRating = GLX_SLOW_VISUAL_EXT;
	    else
	      pConfigs[i].visualRating = GLX_NONE_EXT;
	    pConfigs[i].transparentPixel = 0;
	    pConfigs[i].transparentRed = 0;
	    pConfigs[i].transparentGreen = 0;
	    pConfigs[i].transparentBlue = 0;
	    pConfigs[i].transparentAlpha = 0;
	    pConfigs[i].transparentIndex = 0;
	    i++;
	  }
	}
      }
    }
    if (i!=numConfigs) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "Incorrect initialization of visuals\n");
      return FALSE;
    }
    break; /* 16bpp */

  case 24:
  case 32:
    numConfigs = 8;

    pConfigs = (__GLXvisualConfig*) xnfcalloc(sizeof(__GLXvisualConfig), numConfigs);
    if (!pConfigs)
      return FALSE;

    pTDFXConfigs = (TDFXConfigPrivPtr) xnfcalloc(sizeof(TDFXConfigPrivRec), numConfigs);
    if (!pTDFXConfigs) {
      xfree(pConfigs);
      return FALSE;
    }

    pTDFXConfigPtrs = (TDFXConfigPrivPtr *) xnfcalloc(sizeof(TDFXConfigPrivPtr), numConfigs);
    if (!pTDFXConfigPtrs) {
      xfree(pConfigs);
      xfree(pTDFXConfigs);
      return FALSE;
    }

    for (i = 0; i < numConfigs; i++) 
      pTDFXConfigPtrs[i] = &pTDFXConfigs[i];

    i=0;
    for (db = 0; db <=1; db++) {
      for (depth = 0; depth<=1; depth++) {
         /*stencil = depth;*/  /* Z and stencil share the same memory */
	for (accum = 0; accum <= 1; accum++) {
           /*for (stencil = 0; stencil <=1; stencil++) {*/
           stencil = depth;
	    pConfigs[i].vid = -1;
	    pConfigs[i].class = -1;
	    pConfigs[i].rgba = TRUE;
	    pConfigs[i].redSize = 8;
	    pConfigs[i].greenSize = 8;
	    pConfigs[i].blueSize = 8;
	    pConfigs[i].alphaSize = (pScrn->bitsPerPixel==32) ? 8 : 0;
	    pConfigs[i].redMask   = 0x00ff0000;
	    pConfigs[i].greenMask = 0x0000ff00;
	    pConfigs[i].blueMask  = 0x000000ff;
	    pConfigs[i].alphaMask = (pScrn->bitsPerPixel==32) ? 0xff000000 : 0;
	    if (accum) {
	      pConfigs[i].accumRedSize = 16;
	      pConfigs[i].accumGreenSize = 16;
	      pConfigs[i].accumBlueSize = 16;
	      pConfigs[i].accumAlphaSize = (pScrn->bitsPerPixel==32) ? 16 : 0;
	    } else {
	      pConfigs[i].accumRedSize = 0;
	      pConfigs[i].accumGreenSize = 0;
	      pConfigs[i].accumBlueSize = 0;
	      pConfigs[i].accumAlphaSize = 0;
	    }
	    if (db)
	      pConfigs[i].doubleBuffer = TRUE;
	    else
	      pConfigs[i].doubleBuffer = FALSE;
	    pConfigs[i].stereo = FALSE;
	    pConfigs[i].bufferSize = 16;
	    if (depth) {
	      if (pTDFX->cpp > 2)
		pConfigs[i].depthSize = 24;
	      else 
		pConfigs[i].depthSize = 16;
	    } else {
	      pConfigs[i].depthSize = 0;
	    }
	    if (stencil)
	      pConfigs[i].stencilSize = 8;
	    else
	      pConfigs[i].stencilSize = 0;
	    pConfigs[i].auxBuffers = 0;
	    pConfigs[i].level = 0;
	    if (accum)
	      pConfigs[i].visualRating = GLX_SLOW_VISUAL_EXT;
	    else
	      pConfigs[i].visualRating = GLX_NONE_EXT;
	    pConfigs[i].transparentPixel = 0;
	    pConfigs[i].transparentRed = 0;
	    pConfigs[i].transparentGreen = 0;
	    pConfigs[i].transparentBlue = 0;
	    pConfigs[i].transparentAlpha = 0;
	    pConfigs[i].transparentIndex = 0;
	    i++;
         /*}*/
	}
      }
    }
    if (i!=numConfigs) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "Incorrect initialization of visuals\n");
      return FALSE;
    }
    break;
  }
  pTDFX->numVisualConfigs = numConfigs;
  pTDFX->pVisualConfigs = pConfigs;
  pTDFX->pVisualConfigsPriv = pTDFXConfigs;
  GlxSetVisualConfigs(numConfigs, pConfigs, (void**)pTDFXConfigPtrs);
  return TRUE;
}

static void
TDFXDoWakeupHandler(int screenNum, pointer wakeupData, unsigned long result,
		    pointer pReadmask)
{
  ScreenPtr pScreen = screenInfo.screens[screenNum];
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

  TDFXNeedSync(pScrn);
}

static void 
TDFXDoBlockHandler(int screenNum, pointer blockData, pointer pTimeout,
		  pointer pReadmask)
{
  ScreenPtr pScreen = screenInfo.screens[screenNum];
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

  TDFXCheckSync(pScrn);
}

Bool TDFXDRIScreenInit(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  TDFXPtr pTDFX = TDFXPTR(pScrn);
  DRIInfoPtr pDRIInfo;
  TDFXDRIPtr pTDFXDRI;

  switch (pScrn->bitsPerPixel) {
  case 8:
    xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "DRI not supported in 8 bpp mode, disabling DRI.\n");
    return FALSE;
  case 16:
    break;
  case 24:
    xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "DRI not supported in 24 bpp mode, disabling DRI.\n");
    return FALSE;
  case 32:
     if (pTDFX->ChipType<=PCI_CHIP_VOODOO3) {
       xf86DrvMsg(pScreen->myNum, X_ERROR,
                  "DRI requires Voodoo3 or later, disabling DRI.\n");
       return FALSE;
     }
  }

    /* Check that the GLX, DRI, and DRM modules have been loaded by testing
       for canonical symbols in each module. */
    if (!xf86LoaderCheckSymbol("GlxSetVisualConfigs")) return FALSE;
    if (!xf86LoaderCheckSymbol("DRIScreenInit"))       return FALSE;
    if (!xf86LoaderCheckSymbol("drmAvailable"))        return FALSE;
    if (!xf86LoaderCheckSymbol("DRIQueryVersion")) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "TDFXDRIScreenInit failed (libdri.a too old)\n");
      return FALSE;
    }

  /* Check the DRI version */
  {
    int major, minor, patch;
    DRIQueryVersion(&major, &minor, &patch);
    if (major != 3 || minor != 0 || patch < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "TDFXDRIScreenInit failed (DRI version = %d.%d.%d, expected 3.0.x).  Disabling DRI.\n",
                 major, minor, patch);
      return FALSE;
    }
  }

  pDRIInfo = DRICreateInfoRec();
  if (!pDRIInfo) {
    xf86DrvMsg(pScreen->myNum, X_ERROR,
               "DRICreateInfoRect() failed, disabling DRI.\n");
    return FALSE;
  }

  pTDFX->pDRIInfo = pDRIInfo;

  pDRIInfo->drmDriverName = TDFXKernelDriverName;
  pDRIInfo->clientDriverName = TDFXClientDriverName;
  pDRIInfo->busIdString = xalloc(64);
  sprintf(pDRIInfo->busIdString, "PCI:%d:%d:%d",
	  ((pciConfigPtr)pTDFX->PciInfo->thisCard)->busnum,
	  ((pciConfigPtr)pTDFX->PciInfo->thisCard)->devnum,
	  ((pciConfigPtr)pTDFX->PciInfo->thisCard)->funcnum);
  pDRIInfo->ddxDriverMajorVersion = TDFX_MAJOR_VERSION;
  pDRIInfo->ddxDriverMinorVersion = TDFX_MINOR_VERSION;
  pDRIInfo->ddxDriverPatchVersion = TDFX_PATCHLEVEL;
  pDRIInfo->frameBufferPhysicalAddress = pTDFX->LinearAddr[0];
  pDRIInfo->frameBufferSize = pTDFX->FbMapSize;
  pDRIInfo->frameBufferStride = pTDFX->stride;
  pDRIInfo->ddxDrawableTableEntry = TDFX_MAX_DRAWABLES;

  pDRIInfo->wrap.ValidateTree = 0;
  pDRIInfo->wrap.PostValidateTree = 0;
  pDRIInfo->wrap.BlockHandler = TDFXDoBlockHandler;
  pDRIInfo->wrap.WakeupHandler = TDFXDoWakeupHandler;

  if (SAREA_MAX_DRAWABLES < TDFX_MAX_DRAWABLES)
    pDRIInfo->maxDrawableTableEntry = SAREA_MAX_DRAWABLES;
  else
    pDRIInfo->maxDrawableTableEntry = TDFX_MAX_DRAWABLES;

#ifdef NOT_DONE
  /* FIXME need to extend DRI protocol to pass this size back to client 
   * for SAREA mapping that includes a device private record
   */
  pDRIInfo->SAREASize = 
    ((sizeof(XF86DRISAREARec) + 0xfff) & 0x1000); /* round to page */
  /* + shared memory device private rec */
#else
  /* For now the mapping works by using a fixed size defined
   * in the SAREA header
   */
  if (sizeof(XF86DRISAREARec)+sizeof(TDFXSAREAPriv)>SAREA_MAX) {
    xf86DrvMsg(pScreen->myNum, X_ERROR, "Data does not fit in SAREA\n");
    return FALSE;
  }
  pDRIInfo->SAREASize = SAREA_MAX;
#endif

  if (!(pTDFXDRI = (TDFXDRIPtr)xnfcalloc(sizeof(TDFXDRIRec),1))) {
    xf86DrvMsg(pScreen->myNum, X_ERROR, "DRI memory allocation failed, disabling DRI.\n");
    DRIDestroyInfoRec(pTDFX->pDRIInfo);
    pTDFX->pDRIInfo=0;
    return FALSE;
  }
  pDRIInfo->devPrivate = pTDFXDRI;
  pDRIInfo->devPrivateSize = sizeof(TDFXDRIRec);
  pDRIInfo->contextSize = sizeof(TDFXDRIContextRec);

  pDRIInfo->CreateContext = TDFXCreateContext;
  pDRIInfo->DestroyContext = TDFXDestroyContext;
  pDRIInfo->SwapContext = TDFXDRISwapContext;
  pDRIInfo->InitBuffers = TDFXDRIInitBuffers;
  pDRIInfo->MoveBuffers = TDFXDRIMoveBuffers;
  pDRIInfo->OpenFullScreen = TDFXDRIOpenFullScreen;
  pDRIInfo->CloseFullScreen = TDFXDRICloseFullScreen;
  pDRIInfo->bufferRequests = DRI_ALL_WINDOWS;

  if (!DRIScreenInit(pScreen, pDRIInfo, &pTDFX->drmSubFD)) {
    xfree(pDRIInfo->devPrivate);
    pDRIInfo->devPrivate=0;
    DRIDestroyInfoRec(pTDFX->pDRIInfo);
    pTDFX->pDRIInfo=0;
    xf86DrvMsg(pScreen->myNum, X_ERROR, "DRIScreenInit failed, disabling DRI.\n");

    return FALSE;
  }

  /* Check the TDFX DRM version */
  {
     drmVersionPtr version = drmGetVersion(pTDFX->drmSubFD);
     if (version) {
        if (version->version_major != 1 ||
            version->version_minor != 0 ||
            version->version_patchlevel < 0) {
           /* incompatible drm version */
           xf86DrvMsg(pScreen->myNum, X_ERROR,
                      "TDFXDRIScreenInit failed (DRM version = %d.%d.%d, expected 1.0.x).  Disabling DRI.\n",
                      version->version_major,
                      version->version_minor,
                      version->version_patchlevel);
           TDFXDRICloseScreen(pScreen);
           drmFreeVersion(version);
           return FALSE;
        }
        drmFreeVersion(version);
     }
  }

  pTDFXDRI->regsSize=TDFXIOMAPSIZE;
  if (drmAddMap(pTDFX->drmSubFD, (drmHandle)pTDFX->MMIOAddr[0], 
		pTDFXDRI->regsSize, DRM_REGISTERS, 0, &pTDFXDRI->regs)<0) {
    TDFXDRICloseScreen(pScreen);
    xf86DrvMsg(pScreen->myNum, X_ERROR, "drmAddMap failed, disabling DRI.\n");
    return FALSE;
  }
  xf86DrvMsg(pScreen->myNum, X_INFO, "[drm] Registers = 0x%08lx\n",
	       pTDFXDRI->regs);

  if (!(TDFXInitVisualConfigs(pScreen))) {
    TDFXDRICloseScreen(pScreen);
    xf86DrvMsg(pScreen->myNum, X_ERROR, "TDFXInitVisualConfigs failed, disabling DRI.\n");
    return FALSE;
  }
  xf86DrvMsg(pScrn->scrnIndex, X_INFO, "visual configs initialized\n" );

  return TRUE;
}

void
TDFXDRICloseScreen(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  TDFXPtr pTDFX = TDFXPTR(pScrn);

  DRICloseScreen(pScreen);

  if (pTDFX->pDRIInfo) {
    if (pTDFX->pDRIInfo->devPrivate) {
      xfree(pTDFX->pDRIInfo->devPrivate);
      pTDFX->pDRIInfo->devPrivate=0;
    }
    DRIDestroyInfoRec(pTDFX->pDRIInfo);
    pTDFX->pDRIInfo=0;
  }
  if (pTDFX->pVisualConfigs) xfree(pTDFX->pVisualConfigs);
  if (pTDFX->pVisualConfigsPriv) xfree(pTDFX->pVisualConfigsPriv);
}

static Bool
TDFXCreateContext(ScreenPtr pScreen, VisualPtr visual, 
		  drmContext hwContext, void *pVisualConfigPriv,
		  DRIContextType contextStore)
{
  return TRUE;
}

static void
TDFXDestroyContext(ScreenPtr pScreen, drmContext hwContext, 
		   DRIContextType contextStore)
{
}

Bool
TDFXDRIFinishScreenInit(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  TDFXPtr pTDFX = TDFXPTR(pScrn);
  TDFXDRIPtr pTDFXDRI;

  pTDFX->pDRIInfo->driverSwapMethod = DRI_HIDE_X_CONTEXT;

  pTDFXDRI=(TDFXDRIPtr)pTDFX->pDRIInfo->devPrivate;
  pTDFXDRI->deviceID=pTDFX->PciInfo->chipType;
  pTDFXDRI->width=pScrn->virtualX;
  pTDFXDRI->height=pScrn->virtualY;
  pTDFXDRI->mem=pScrn->videoRam*1024;
  pTDFXDRI->cpp=pTDFX->cpp;
  pTDFXDRI->stride=pTDFX->stride;
  pTDFXDRI->fifoOffset=pTDFX->fifoOffset;
  pTDFXDRI->fifoSize=pTDFX->fifoSize;
  pTDFXDRI->textureOffset=pTDFX->texOffset;
  pTDFXDRI->textureSize=pTDFX->texSize;
  pTDFXDRI->fbOffset=pTDFX->fbOffset;
  pTDFXDRI->backOffset=pTDFX->backOffset;
  pTDFXDRI->depthOffset=pTDFX->depthOffset;
  return DRIFinishScreenInit(pScreen);
}

static void
TDFXDRISwapContext(ScreenPtr pScreen, DRISyncType syncType, 
		   DRIContextType oldContextType, void *oldContext,
		   DRIContextType newContextType, void *newContext)
{
}

static void
TDFXDRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index)
{
  ScreenPtr pScreen = pWin->drawable.pScreen;
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  TDFXPtr pTDFX = TDFXPTR(pScrn);
  BoxPtr pbox;
  int nbox;

  /* It looks nicer if these start out black */
  pbox = REGION_RECTS(prgn);
  nbox = REGION_NUM_RECTS(prgn);

  TDFXSetupForSolidFill(pScrn, 0, GXcopy, -1);
  while (nbox--) {
    TDFXSelectBuffer(pTDFX, TDFX_BACK);
    TDFXSubsequentSolidFillRect(pScrn, pbox->x1, pbox->y1, 
				pbox->x2-pbox->x1, pbox->y2-pbox->y1);
    TDFXSelectBuffer(pTDFX, TDFX_DEPTH);
    TDFXSubsequentSolidFillRect(pScrn, pbox->x1, pbox->y1, 
				pbox->x2-pbox->x1, pbox->y2-pbox->y1);
    pbox++;
  }
  TDFXSelectBuffer(pTDFX, TDFX_FRONT);

  pTDFX->AccelInfoRec->NeedToSync = TRUE;
}

/*
  This routine is a modified form of XAADoBitBlt with the calls to
  ScreenToScreenBitBlt built in. My routine has the prgnSrc as source
  instead of destination. My origin is upside down so the ydir cases
  are reversed.
*/
static void
TDFXDRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg, 
		   RegionPtr prgnSrc, CARD32 index)
{
  ScreenPtr pScreen = pParent->drawable.pScreen;
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  TDFXPtr pTDFX = TDFXPTR(pScrn);
  int nbox;
  BoxPtr pbox, pboxTmp, pboxNext, pboxBase, pboxNew1, pboxNew2;
  DDXPointPtr pptTmp, pptNew1, pptNew2;
  int xdir, ydir;
  int dx, dy, x, y, w, h;
  DDXPointPtr pptSrc;

  pbox = REGION_RECTS(prgnSrc);
  nbox = REGION_NUM_RECTS(prgnSrc);
  pboxNew1 = 0;
  pptNew1 = 0;
  pboxNew2 = 0;
  pboxNew2 = 0;
  pptSrc = &ptOldOrg;

  dx = pParent->drawable.x - ptOldOrg.x;
  dy = pParent->drawable.y - ptOldOrg.y;

  /* If the copy will overlap in Y, reverse the order */
  if (dy>0) {
    ydir = -1;

    if (nbox>1) {
      /* Keep ordering in each band, reverse order of bands */
      pboxNew1 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec)*nbox);
      if (!pboxNew1) return;
      pptNew1 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec)*nbox);
      if (!pptNew1) {
	DEALLOCATE_LOCAL(pboxNew1);
	return;
      }
      pboxBase = pboxNext = pbox+nbox-1;
      while (pboxBase >= pbox) {
	while ((pboxNext >= pbox) && (pboxBase->y1 == pboxNext->y1))
	  pboxNext--;
	pboxTmp = pboxNext+1;
	pptTmp = pptSrc + (pboxTmp - pbox);
	while (pboxTmp <= pboxBase) {
	  *pboxNew1++ = *pboxTmp++;
	  *pptNew1++ = *pptTmp++;
	}
	pboxBase = pboxNext;
      }
      pboxNew1 -= nbox;
      pbox = pboxNew1;
      pptNew1 -= nbox;
      pptSrc = pptNew1;
    }
  } else {
    /* No changes required */
    ydir = 1;
  }

  /* If the regions will overlap in X, reverse the order */
  if (dx>0) {
    xdir = -1;

    if (nbox > 1) {
      /*reverse orderof rects in each band */
      pboxNew2 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec)*nbox);
      pptNew2 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec)*nbox);
      if (!pboxNew2 || !pptNew2) {
	if (pptNew2) DEALLOCATE_LOCAL(pptNew2);
	if (pboxNew2) DEALLOCATE_LOCAL(pboxNew2);
	if (pboxNew1) {
	  DEALLOCATE_LOCAL(pptNew1);
	  DEALLOCATE_LOCAL(pboxNew1);
	}
	return;
      }
      pboxBase = pboxNext = pbox;
      while (pboxBase < pbox+nbox) {
	while ((pboxNext < pbox+nbox) && (pboxNext->y1 == pboxBase->y1))
	  pboxNext++;
	pboxTmp = pboxNext;
	pptTmp = pptSrc + (pboxTmp - pbox);
	while (pboxTmp != pboxBase) {
	  *pboxNew2++ = *--pboxTmp;
	  *pptNew2++ = *--pptTmp;
	}
	pboxBase = pboxNext;
      }
      pboxNew2 -= nbox;
      pbox = pboxNew2;
      pptNew2 -= nbox;
      pptSrc = pptNew2;
    }
  } else {
    /* No changes are needed */
    xdir = 1;
  }

  TDFXSetupForScreenToScreenCopy(pScrn, xdir, ydir, GXcopy, -1, -1);
  while (nbox--) {
    w=pbox->x2-pbox->x1+1;
    h=pbox->y2-pbox->y1+1;

    /* Unlike XAA, we don't get handed clipped values */
    if (pbox->x1+dx<0) {
      x=-dx;
      w-=x-pbox->x1;
    } else {
      if (pbox->x1+dx+w>pScrn->virtualX) {
        x=pScrn->virtualX-dx-w-1;
        w-=pbox->x1-x;
      } else x=pbox->x1;
    }
    if (pbox->y1+dy<0) {
      y=-dy;
      h-=y-pbox->y1;
    } else {
      if (pbox->y1+dy+h>pScrn->virtualY) {
        y=pScrn->virtualY-dy-h-1;
        h-=pbox->y1-y;
      } else y=pbox->y1;
    }
    if (w<0 || h<0 || x>pScrn->virtualX || y>pScrn->virtualY) continue;

    TDFXSelectBuffer(pTDFX, TDFX_BACK);
    TDFXSubsequentScreenToScreenCopy(pScrn, x, y, x+dx, y+dy, w, h);
    TDFXSelectBuffer(pTDFX, TDFX_DEPTH);
    TDFXSubsequentScreenToScreenCopy(pScrn, x, y, x+dx, y+dy, w, h);
    pbox++;
  }
  TDFXSelectBuffer(pTDFX, TDFX_FRONT);

  if (pboxNew2) {
    DEALLOCATE_LOCAL(pptNew2);
    DEALLOCATE_LOCAL(pboxNew2);
  }
  if (pboxNew1) {
    DEALLOCATE_LOCAL(pptNew1);
    DEALLOCATE_LOCAL(pboxNew1);
  }

  pTDFX->AccelInfoRec->NeedToSync = TRUE;
}

static Bool
TDFXDRIOpenFullScreen(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn;
  TDFXPtr pTDFX;

  xf86DrvMsg(pScreen->myNum, X_INFO, "OpenFullScreen\n");
#if 0
  pScrn = xf86Screens[pScreen->myNum];
  pTDFX=TDFXPTR(pScrn);
  if (pTDFX->numChips>1) {
    TDFXSetupSLI(pScrn);
  }
#endif
  return TRUE;
}

static Bool
TDFXDRICloseFullScreen(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn;

  xf86DrvMsg(pScreen->myNum, X_INFO, "CloseFullScreen\n");
#if 0
  pScrn = xf86Screens[pScreen->myNum];
  TDFXDisableSLI(pScrn);
#endif
  return TRUE;
}

