/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/mga/mga_dri.c,v 1.14 2000/12/11 22:34:55 dawes Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Priv.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"
#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb16.h"
#include "cfb32.h"

#include "miline.h"

#include "GL/glxtokens.h"

#include "mga_bios.h"
#include "mga_reg.h"
#include "mga.h"
#include "mga_macros.h"
#include "mga_dri.h"
#include "mga_wrap.h"

static char MGAKernelDriverName[] = "mga";
static char MGAClientDriverName[] = "mga";

static Bool MGAInitVisualConfigs(ScreenPtr pScreen);
static Bool MGACreateContext(ScreenPtr pScreen, VisualPtr visual, 
			      drmContext hwContext, void *pVisualConfigPriv,
			      DRIContextType contextStore);
static void MGADestroyContext(ScreenPtr pScreen, drmContext hwContext,
			       DRIContextType contextStore);
static void MGADRISwapContext(ScreenPtr pScreen, DRISyncType syncType, 
			       DRIContextType readContextType, 
			       void *readContextStore,
			       DRIContextType writeContextType, 
			       void *writeContextStore);
static void MGADRISwapContext_shared(ScreenPtr pScreen, DRISyncType syncType, 
			       DRIContextType readContextType, 
			       void *readContextStore,
			       DRIContextType writeContextType, 
			       void *writeContextStore);
extern void Mga8DRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index);
extern void Mga8DRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg, 
			       RegionPtr prgnSrc, CARD32 index);
extern void Mga16DRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index);
extern void Mga16DRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg, 
			       RegionPtr prgnSrc, CARD32 index);
extern void Mga24DRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index);
extern void Mga24DRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg, 
			       RegionPtr prgnSrc, CARD32 index);
extern void Mga32DRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index);
extern void Mga32DRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg, 
			       RegionPtr prgnSrc, CARD32 index);

Bool MgaCleanupDma(ScrnInfoPtr pScrn)
{
   MGAPtr pMGA = MGAPTR(pScrn);
   Bool ret_val;

   ret_val = drmMgaCleanupDma(pMGA->drmSubFD);
   if (ret_val == FALSE)
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Mga Dma Cleanup Failed\n");

   return ret_val;
}

Bool MgaLockUpdate(ScrnInfoPtr pScrn, drmLockFlags flags)
{
   MGAPtr pMGA = MGAPTR(pScrn);
   Bool ret_val;

   ret_val = drmMgaLockUpdate(pMGA->drmSubFD, flags);
   if (ret_val == FALSE)
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "LockUpdate failed\n");

   return ret_val;
}

Bool MgaInitDma(ScrnInfoPtr pScrn, int prim_size)
{
   MGAPtr pMGA = MGAPTR(pScrn);
   MGADRIPtr pMGADRI = (MGADRIPtr)pMGA->pDRIInfo->devPrivate;
   MGADRIServerPrivatePtr pMGADRIServer = pMGA->DRIServerInfo;
   drmMgaInit init;
   Bool ret_val;
   
   memset(&init, 0, sizeof(drmMgaInit));
   init.reserved_map_agpstart = 0;
   init.reserved_map_idx = 3;
   init.buffer_map_idx = 4;
   init.sarea_priv_offset = sizeof(XF86DRISAREARec);
   init.primary_size = prim_size;
   init.warp_ucode_size = pMGADRIServer->warp_ucode_size;

   switch(pMGA->Chipset) {
   case PCI_CHIP_MGAG400:
      init.chipset = MGA_CARD_TYPE_G400;
      break;
   case PCI_CHIP_MGAG200:
   case PCI_CHIP_MGAG200_PCI:
      init.chipset = MGA_CARD_TYPE_G200;
      break;
   default:
      return FALSE;
   }

   init.frontOffset = pMGADRI->frontOffset;
   init.backOffset = pMGADRI->backOffset;
   init.depthOffset = pMGADRI->depthOffset;
   init.textureOffset = pMGADRI->textureOffset;
   init.textureSize = pMGADRI->textureSize;
   init.agpTextureSize = pMGADRI->agpTextureSize;
   init.cpp = pMGADRI->cpp;
   init.stride = pMGADRI->frontPitch;
   init.mAccess = pMGA->MAccess;
   init.sgram = !pMGA->HasSDRAM;
   
   memcpy(&init.WarpIndex, &pMGADRIServer->WarpIndex, 
	  sizeof(drmMgaWarpIndex) * MGA_MAX_WARP_PIPES);

   xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[drm] Mga Dma Initialization start\n");

   ret_val = drmMgaInitDma(pMGA->drmSubFD, &init);
   if (ret_val == FALSE)
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "[drm] Mga Dma Initialization Failed\n");
   else
      xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[drm] Mga Dma Initialization done\n");
   return ret_val;
}

static Bool
MGAInitVisualConfigs(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   MGAPtr pMGA = MGAPTR(pScrn);
   int numConfigs = 0;
   __GLXvisualConfig *pConfigs = 0;
   MGAConfigPrivPtr pMGAConfigs = 0;
   MGAConfigPrivPtr *pMGAConfigPtrs = 0;
   int i, db, depth, stencil, accum;

   switch (pScrn->bitsPerPixel) {
   case 8:
   case 24:
      break;
   case 16:
      numConfigs = 8;

      if (!(pConfigs = (__GLXvisualConfig*)xnfcalloc(sizeof(__GLXvisualConfig),
						     numConfigs))) {
	 return FALSE;
      }
      if (!(pMGAConfigs = (MGAConfigPrivPtr)xnfcalloc(sizeof(MGAConfigPrivRec),
						      numConfigs))) {
	 xfree(pConfigs);
	 return FALSE;
      }
      if (!(pMGAConfigPtrs = (MGAConfigPrivPtr*)xnfcalloc(sizeof(MGAConfigPrivPtr),
							  numConfigs))) {
	 xfree(pConfigs);
	 xfree(pMGAConfigs);
	 return FALSE;
      }
      for (i=0; i<numConfigs; i++) 
	 pMGAConfigPtrs[i] = &pMGAConfigs[i];

      i = 0;
      depth = 1;
      for (accum = 0; accum <= 1; accum++) {
         for (stencil = 0; stencil <= 1; stencil++) { /* no stencil for now */
            for (db=1; db>=0; db--) {
               pConfigs[i].vid = -1;
               pConfigs[i].class = -1;
               pConfigs[i].rgba = TRUE;
               pConfigs[i].redSize = 5;
               pConfigs[i].greenSize = 6;
               pConfigs[i].blueSize = 5;
               pConfigs[i].alphaSize = 0;
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
               if (depth)
                  pConfigs[i].depthSize = 16;
               else 
                  pConfigs[i].depthSize = 0;
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
      if (i != numConfigs) {
         xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "[drm] Incorrect initialization of visuals\n");
         return FALSE;
      }
      break;

   case 32:
      numConfigs = 8;

      if (!(pConfigs = (__GLXvisualConfig*)xnfcalloc(sizeof(__GLXvisualConfig),
						     numConfigs))) {
	 return FALSE;
      }
      if (!(pMGAConfigs = (MGAConfigPrivPtr)xnfcalloc(sizeof(MGAConfigPrivRec),
						      numConfigs))) {
	 xfree(pConfigs);
	 return FALSE;
      }
      if (!(pMGAConfigPtrs = (MGAConfigPrivPtr*)xnfcalloc(sizeof(MGAConfigPrivPtr),
							  numConfigs))) {
	 xfree(pConfigs);
	 xfree(pMGAConfigs);
	 return FALSE;
      }
      for (i=0; i<numConfigs; i++) 
	 pMGAConfigPtrs[i] = &pMGAConfigs[i];

      i = 0;
      for (accum = 0; accum <= 1; accum++) {
         for (depth = 0; depth <= 1; depth++) { /* and stencil */
            for (db=1; db>=0; db--) {
               pConfigs[i].vid = -1;
               pConfigs[i].class = -1;
               pConfigs[i].rgba = TRUE;
               pConfigs[i].redSize = 8;
               pConfigs[i].greenSize = 8;
               pConfigs[i].blueSize = 8;
               pConfigs[i].alphaSize = 0;
               pConfigs[i].redMask   = 0x00FF0000;
               pConfigs[i].greenMask = 0x0000FF00;
               pConfigs[i].blueMask  = 0x000000FF;
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
               pConfigs[i].bufferSize = 32;
               if (depth) {
		     pConfigs[i].depthSize = 24;
                     pConfigs[i].stencilSize = 8;
               }
               else {
                     pConfigs[i].depthSize = 0;
                     pConfigs[i].stencilSize = 0;
               }
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
            }
         }
      }
      if (i != numConfigs) {
         xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "[drm] Incorrect initialization of visuals\n");
         return FALSE;
      }
      break;

   default:
      ;  /* unexpected bits/pixelx */
   }
   pMGA->numVisualConfigs = numConfigs;
   pMGA->pVisualConfigs = pConfigs;
   pMGA->pVisualConfigsPriv = pMGAConfigs;
   GlxSetVisualConfigs(numConfigs, pConfigs, (void**)pMGAConfigPtrs);
   return TRUE;
}

static unsigned int mylog2(unsigned int n)
{
   unsigned int log2 = 1;
   while (n>1) n >>= 1, log2++;
   return log2;
}

static unsigned long MGAParseAgpMode(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   MGAPtr pMga = MGAPTR(pScrn);
   unsigned long mode_mask;

   switch(pMga->agp_mode) {
   case 4:
      mode_mask = ~0x00000003;
      break;
   case 2:
      if (pMga->Chipset == PCI_CHIP_MGAG200) {
	 xf86DrvMsg(pScreen->myNum, X_INFO, 
		    "[drm] Enabling AGP 2x pll encoding\n");
	 OUTREG(MGAREG_AGP_PLL, AGP_PLL_agp2xpllen_enable);
      }
      mode_mask = ~0x00000005;
      break;
   default:
   /* Default to 1X agp mode */
   case 1:
      if (pMga->Chipset == PCI_CHIP_MGAG200) {
	 xf86DrvMsg(pScreen->myNum, X_INFO, 
		    "[drm] Disabling AGP 2x pll encoding\n");
	 OUTREG(MGAREG_AGP_PLL, AGP_PLL_agp2xpllen_disable);
      }
      pMga->agp_mode = 1;
      mode_mask = ~0x00000006;
      break;
   }

   return mode_mask;
}

Bool MGADRIScreenInit(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   MGAPtr pMGA = MGAPTR(pScrn);
   DRIInfoPtr pDRIInfo;
   MGADRIPtr pMGADRI;
   MGADRIServerPrivatePtr pMGADRIServer;
   int bufs, size;
   int prim_size;
   int init_offset;
   int i;
   unsigned long mode_mask;

   switch(pMGA->Chipset) {
   case PCI_CHIP_MGAG400:
   case PCI_CHIP_MGAG200:
#if 0
   case PCI_CHIP_MGAG200_PCI:
#endif
      break;
   default:
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "[drm] Direct rendering only supported with G200/G400 AGP\n");
      return FALSE;
   }

   /* Check that the GLX, DRI, and DRM modules have been loaded by testing
    * for canonical symbols in each module. */
   if (!xf86LoaderCheckSymbol("GlxSetVisualConfigs")) return FALSE;
   if (!xf86LoaderCheckSymbol("DRIScreenInit"))       return FALSE;
   if (!xf86LoaderCheckSymbol("drmAvailable"))        return FALSE;
   if (!xf86LoaderCheckSymbol("DRIQueryVersion")) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "MGADRIScreenInit failed (libdri.a too old)\n");
      return FALSE;
   }
     
   /* Check the DRI version */
   {
      int major, minor, patch;
      DRIQueryVersion(&major, &minor, &patch);
      if (major != 3 || minor != 0 || patch < 0) {
         xf86DrvMsg(pScreen->myNum, X_ERROR,
                    "[drm] MGADRIScreenInit failed (DRI version = %d.%d.%d, expected 3.0.x).  Disabling DRI.\n",
                    major, minor, patch);
         return FALSE;
      }
   }

   xf86DrvMsg(pScreen->myNum, X_INFO, "[drm] bpp: %d depth: %d\n", pScrn->bitsPerPixel, pScrn->depth);

   if ((pScrn->bitsPerPixel / 8) != 2 &&
       (pScrn->bitsPerPixel / 8) != 4) {
      xf86DrvMsg(pScreen->myNum, X_INFO,
                 "[drm] Direct rendering only supported in 16 and 32 bpp modes\n");
      return FALSE;
   }
   
   pDRIInfo = DRICreateInfoRec();
   if (!pDRIInfo)
      return FALSE;
   pMGA->pDRIInfo = pDRIInfo;

   pDRIInfo->drmDriverName = MGAKernelDriverName;
   pDRIInfo->clientDriverName = MGAClientDriverName;
   pDRIInfo->busIdString = xalloc(64);
   sprintf(pDRIInfo->busIdString, "PCI:%d:%d:%d",
           ((pciConfigPtr)pMGA->PciInfo->thisCard)->busnum,
           ((pciConfigPtr)pMGA->PciInfo->thisCard)->devnum,
           ((pciConfigPtr)pMGA->PciInfo->thisCard)->funcnum);
   pDRIInfo->ddxDriverMajorVersion = MGA_MAJOR_VERSION;
   pDRIInfo->ddxDriverMinorVersion = MGA_MINOR_VERSION;
   pDRIInfo->ddxDriverPatchVersion = MGA_PATCHLEVEL;
   pDRIInfo->frameBufferPhysicalAddress = pMGA->FbAddress;
   pDRIInfo->frameBufferSize = pMGA->FbMapSize;
   pDRIInfo->frameBufferStride = pScrn->displayWidth*(pScrn->bitsPerPixel/8);
   pDRIInfo->ddxDrawableTableEntry = MGA_MAX_DRAWABLES;

   MGADRIWrapFunctions( pScreen, pDRIInfo );

   if (SAREA_MAX_DRAWABLES < MGA_MAX_DRAWABLES)
      pDRIInfo->maxDrawableTableEntry = SAREA_MAX_DRAWABLES;
   else
      pDRIInfo->maxDrawableTableEntry = MGA_MAX_DRAWABLES;

   /* For now the mapping works by using a fixed size defined
    * in the SAREA header
    */
   if (sizeof(XF86DRISAREARec)+sizeof(MGASAREARec)>SAREA_MAX) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "[drm] Data does not fit in SAREA\n");
      return FALSE;
   }
  
   xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[drm] Sarea %d+%d: %d\n",
              sizeof(XF86DRISAREARec), sizeof(MGASAREARec),
              sizeof(XF86DRISAREARec) + sizeof(MGASAREARec));

   pDRIInfo->SAREASize = SAREA_MAX;

   if (!(pMGADRI = (MGADRIPtr)xnfcalloc(sizeof(MGADRIRec),1))) {
      DRIDestroyInfoRec(pMGA->pDRIInfo);
      pMGA->pDRIInfo=0;
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                 "[drm] Failed to allocate memory for private record\n");
      return FALSE;
   }
   if (!(pMGADRIServer = (MGADRIServerPrivatePtr)
	 xnfcalloc(sizeof(MGADRIServerPrivateRec),1))) {
      xfree(pMGADRI);
      DRIDestroyInfoRec(pMGA->pDRIInfo);
      pMGA->pDRIInfo=0;
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                 "[drm] Failed to allocate memory for private record\n");
      return FALSE;
   }

   pDRIInfo->devPrivate = pMGADRI;
   pMGA->DRIServerInfo = pMGADRIServer;
   pDRIInfo->devPrivateSize = sizeof(MGADRIRec);
   pDRIInfo->contextSize = sizeof(MGADRIContextRec);

   pDRIInfo->CreateContext = MGACreateContext;
   pDRIInfo->DestroyContext = MGADestroyContext;
   if (xf86IsEntityShared(pScrn->entityList[0]))
      pDRIInfo->SwapContext = MGADRISwapContext_shared;
   else
      pDRIInfo->SwapContext = MGADRISwapContext;
  
   switch( pScrn->bitsPerPixel ) {
   case 8:
       pDRIInfo->InitBuffers = Mga8DRIInitBuffers;
       pDRIInfo->MoveBuffers = Mga8DRIMoveBuffers;
   case 16:
       pDRIInfo->InitBuffers = Mga16DRIInitBuffers;
       pDRIInfo->MoveBuffers = Mga16DRIMoveBuffers;
   case 24:
       pDRIInfo->InitBuffers = Mga24DRIInitBuffers;
       pDRIInfo->MoveBuffers = Mga24DRIMoveBuffers;
   case 32:
       pDRIInfo->InitBuffers = Mga32DRIInitBuffers;
       pDRIInfo->MoveBuffers = Mga32DRIMoveBuffers;
   }
   
   pDRIInfo->bufferRequests = DRI_ALL_WINDOWS;

   if (!DRIScreenInit(pScreen, pDRIInfo, &pMGA->drmSubFD)) {
      xfree(pMGADRIServer);
      pMGA->DRIServerInfo = 0;
      xfree(pDRIInfo->devPrivate);
      pDRIInfo->devPrivate = 0;
      DRIDestroyInfoRec(pMGA->pDRIInfo);
      pMGA->pDRIInfo = 0;
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] DRIScreenInit Failed\n");
      return FALSE;
   }

   /* Check the MGA DRM version */
   {
      drmVersionPtr version = drmGetVersion(pMGA->drmSubFD);
      if (version) {
         if (version->version_major != 2 ||
             version->version_minor != 0 ||
             version->version_patchlevel < 0) {
            /* incompatible drm version */
            xf86DrvMsg(pScreen->myNum, X_ERROR,
                       "[drm] MGADRIScreenInit failed (DRM version = %d.%d.%d, expected 2.0.x).  Disabling DRI.\n",
                       version->version_major,
                       version->version_minor,
                       version->version_patchlevel);
/*              MGADRICloseScreen(pScreen); */
	    
            drmFreeVersion(version);
            return FALSE;
         }
         drmFreeVersion(version);
      }
   }

   pMGADRIServer->regsSize = MGAIOMAPSIZE;
   if (drmAddMap(pMGA->drmSubFD, (drmHandle)pMGA->IOAddress, 
                 pMGADRIServer->regsSize, DRM_REGISTERS, 0, 
                 &pMGADRIServer->regs)<0) {
      DRICloseScreen(pScreen);
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[drm] drmAddMap failed Register MMIO region\n");
      return FALSE;
   }
   xf86DrvMsg(pScreen->myNum, X_INFO, "[drm] Registers = 0x%08lx\n",
              pMGADRIServer->regs);
   
   /* Agp Support */
   pMGADRIServer->agpAcquired = FALSE;
   pMGADRIServer->agpHandle = 0;
   pMGADRIServer->agpSizep = 0;
   pMGADRIServer->agp_map = 0;
   
   if (drmAgpAcquire(pMGA->drmSubFD) < 0) {
      DRICloseScreen(pScreen);
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] drmAgpAcquire failed\n");
      return FALSE;
   }
   pMGADRIServer->agpAcquired = TRUE;

   pMGADRIServer->warp_ucode_size = mgaGetMicrocodeSize(pScreen);
   if (pMGADRIServer->warp_ucode_size == 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] microcodeSize is zero\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   mode_mask = MGAParseAgpMode(pScreen);

   pMGADRIServer->agpMode = drmAgpGetMode(pMGA->drmSubFD);
   pMGADRIServer->agpMode &= mode_mask;
   if (drmAgpEnable(pMGA->drmSubFD, pMGADRIServer->agpMode) < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] drmAgpEnable failed\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   ErrorF("[drm] drmAgpEnabled succeeded for AGP mode %dx\n", pMGA->agp_mode);

   prim_size = 65536;
   init_offset = ((prim_size + pMGADRIServer->warp_ucode_size + 
		  4096 - 1) / 4096) * 4096;
   
   pMGADRIServer->agpSizep = init_offset;
   pMGADRI->agpSize = (drmAgpSize(pMGA->drmSubFD)) - init_offset;

   pMGADRIServer->agpBase = (drmAddress) drmAgpBase(pMGA->drmSubFD);
   if (drmAddMap(pMGA->drmSubFD, 0,
                 init_offset, DRM_AGP, 0, 
                 &pMGADRIServer->agp_private) < 0) {
      DRICloseScreen(pScreen);
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[drm] drmAddMap failed on AGP aperture\n");
      return FALSE;
   }
   
   if (drmMap(pMGA->drmSubFD, (drmHandle)pMGADRIServer->agp_private,
              init_offset, 
              (drmAddressPtr)&pMGADRIServer->agp_map) < -1) {
      DRICloseScreen(pScreen);
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[drm] drmMap failed on AGP aperture\n");
      return FALSE;
   }
   
   /* Now allocate and bind a default of 8 megs */
   drmAgpAlloc(pMGA->drmSubFD, 0x00800000, 0, 0,
               &pMGADRIServer->agpHandle);
   
   if (pMGADRIServer->agpHandle == 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[drm] drmAgpAlloc failed\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   
   if (drmAgpBind(pMGA->drmSubFD, pMGADRIServer->agpHandle, 0) < 0) {
      DRICloseScreen(pScreen);
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[drm] drmAgpBind failed\n");
      return FALSE;
   }

   mgaInstallMicrocode(pScreen, prim_size);

   if (drmAddMap(pMGA->drmSubFD, (drmHandle)init_offset,
                 pMGADRI->agpSize, DRM_AGP, 0, 
                 &pMGADRI->agp) < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[drm] Failed to map public agp area\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   switch(pMGA->Chipset) {
   case PCI_CHIP_MGAG400:
      pMGADRI->chipset = MGA_CARD_TYPE_G400;
      break;
   case PCI_CHIP_MGAG200:
   case PCI_CHIP_MGAG200_PCI:
      pMGADRI->chipset = MGA_CARD_TYPE_G200;
      break;
   default:
      return FALSE;
   }
   
   pMGADRI->width = pScrn->virtualX;
   pMGADRI->height = pScrn->virtualY;
   pMGADRI->mem = pScrn->videoRam * 1024;
   pMGADRI->cpp = pScrn->bitsPerPixel / 8;
   pMGADRI->frontPitch = pScrn->displayWidth * (pScrn->bitsPerPixel / 8);


   pMGADRI->frontOffset = 0; /* pMGA->YDstOrg * (pScrn->bitsPerPixel / 8) */
   pMGADRI->backOffset = ((pScrn->virtualY + pMGA->numXAALines + 1) * 
			  pScrn->displayWidth *
                          pMGADRI->cpp + 4095) & ~0xFFF;


   xf86DrvMsg(pScreen->myNum, X_INFO, "[drm] calced backoffset: 0x%x\n",
              pMGADRI->backOffset);


   size = pMGADRI->cpp * pScrn->virtualX * pScrn->virtualY;
   size += 4095;
   size &= ~4095;
   pMGADRI->depthOffset = pMGA->FbUsableSize - size;
   pMGADRI->depthOffset &= ~4095;  
   pMGADRI->textureOffset = pMGADRI->backOffset + size;
   pMGADRI->textureSize = pMGADRI->depthOffset - pMGADRI->textureOffset;

   if (pMGADRI->depthOffset < pMGADRI->textureOffset + 512*1024) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[drm] Insufficient memory for direct rendering\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   pMGADRI->mAccess = pMGA->MAccess;

   i = mylog2(pMGADRI->textureSize / MGA_NR_TEX_REGIONS);
   if (i < MGA_LOG_MIN_TEX_REGION_SIZE)
      i = MGA_LOG_MIN_TEX_REGION_SIZE;
  
   pMGADRI->logTextureGranularity = i;
   pMGADRI->textureSize = (pMGADRI->textureSize >> i) << i; /* truncate */


   /* Here is where we need to do initialization of the dma engine */
   if((bufs = drmAddBufs(pMGA->drmSubFD,
                         MGA_DMA_BUF_NR,
                         MGA_DMA_BUF_SZ,
                         DRM_AGP_BUFFER,
                         init_offset)) <= 0) {
     xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                "[drm] failure adding %d %d byte DMA buffers\n",
                MGA_DMA_BUF_NR,
                MGA_DMA_BUF_SZ);
     DRICloseScreen(pScreen);
     return FALSE;
   }

   pMGADRI->agpBufferOffset = init_offset + pMGADRIServer->agp_private;

   /* Calculate texture constants for AGP texture space
    */
   {
      CARD32 agpTextureOffset = MGA_DMA_BUF_SZ * MGA_DMA_BUF_NR;
      CARD32 agpTextureSize = pMGADRI->agpSize - agpTextureOffset;

      i = mylog2(agpTextureSize / MGA_NR_TEX_REGIONS);
      if (i < MGA_LOG_MIN_TEX_REGION_SIZE)
         i = MGA_LOG_MIN_TEX_REGION_SIZE;

      pMGADRI->logAgpTextureGranularity = i;
      pMGADRI->agpTextureSize = (agpTextureSize >> i) << i; 
      pMGADRI->agpTextureOffset = agpTextureOffset;
   }

   
   xf86DrvMsg(pScrn->scrnIndex, X_INFO,
              "[drm] added %d %d byte DMA buffers\n",
              bufs, MGA_DMA_BUF_SZ);


   if ((MgaInitDma(pScrn, prim_size)) != TRUE) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[drm] Failed to initialize dma engine\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   xf86DrvMsg(pScreen->myNum, X_INFO, "[drm] Initialized Dma Engine\n");
   
   if (!pMGADRIServer->irq) {
      pMGADRIServer->irq = drmGetInterruptFromBusID(pMGA->drmSubFD,
                                            ((pciConfigPtr)pMGA->PciInfo
					     ->thisCard)->busnum,
					    ((pciConfigPtr)pMGA->PciInfo
					     ->thisCard)->devnum,
					    ((pciConfigPtr)pMGA->PciInfo
					     ->thisCard)->funcnum);

      if(!pMGADRIServer->irq && !pMGA->ReallyUseIrqZero) {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		    "[drm] Your graphics card has Interrupt zero"
		    " assigned to it.\n"
		    "This is highly unlikely so I'm disabling the DRI.\n"
		    "If your graphics card really has Interrupt zero, please"
		    "add the config option UseIrqZero\n"
		    "to the device section in your XF86Config file.\n"
		    "Please be warned that Interrupt zero is normally "
		    "the timer interrupt on X86 systems.\n"
		    "Using this option could make your system unusable.\n"
		    "The more likely solution is that your graphics card has"
		    " no interrupt assigned to it.\nPlease consult your"
		    " system BIOS manual for instructions on how to enable "
		    "an interrupt for your graphics card.\n");
	 MGADRICloseScreen(pScreen);
	 return FALSE;
      }
      drmCtlInstHandler(pMGA->drmSubFD, pMGADRIServer->irq);
   }

   xf86DrvMsg(pScrn->scrnIndex, X_INFO,
              "[drm] dma control initialized, using IRQ %d\n",
              pMGADRIServer->irq);


   if (!(MGAInitVisualConfigs(pScreen))) {
      DRICloseScreen(pScreen);
      return FALSE;
   }
   xf86DrvMsg(pScrn->scrnIndex, X_INFO, "visual configs initialized\n" );

   return TRUE;
}

void
MGADRICloseScreen(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  MGAPtr pMGA = MGAPTR(pScrn);
  MGADRIServerPrivatePtr pMGADRIServer = pMGA->DRIServerInfo;

/* The DRI will automagically clean these up when driFD is closed */
  if(pMGADRIServer->agp_map) {  
     drmUnmap(pMGADRIServer->agp_map, pMGADRIServer->agpSizep);
     pMGADRIServer->agp_map = 0;
  }
  if(pMGADRIServer->agpHandle) {
     pMGADRIServer->agpHandle = 0;
     pMGADRIServer->agpSizep = 0;
  }
  if(pMGADRIServer->agpAcquired == TRUE) {
     pMGADRIServer->agpAcquired = FALSE;
  }

  DRICloseScreen(pScreen);

  if (pMGA->pDRIInfo) {
    if (pMGA->pDRIInfo->devPrivate) {
      xfree(pMGA->pDRIInfo->devPrivate);
      pMGA->pDRIInfo->devPrivate = 0;
    }
    DRIDestroyInfoRec(pMGA->pDRIInfo);
    pMGA->pDRIInfo = 0;
  }
  if(pMGA->DRIServerInfo) {
     xfree(pMGA->DRIServerInfo);
     pMGA->DRIServerInfo = 0;
  }
  if (pMGA->pVisualConfigs) {
    xfree(pMGA->pVisualConfigs);
  }
  if (pMGA->pVisualConfigsPriv) { 
    xfree(pMGA->pVisualConfigsPriv);
  }
}

static Bool
MGACreateContext(ScreenPtr pScreen, VisualPtr visual, 
		  drmContext hwContext, void *pVisualConfigPriv,
		  DRIContextType contextStore)
{
  return TRUE;
}

static void
MGADestroyContext(ScreenPtr pScreen, drmContext hwContext, 
		   DRIContextType contextStore)
{
}

Bool
MGADRIFinishScreenInit(ScreenPtr pScreen)
{
  ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
  MGASAREAPtr sPriv;
  MGAPtr pMGA = MGAPTR(pScrn);

  if (!pMGA->pDRIInfo) return FALSE;

  sPriv = (MGASAREAPtr)DRIGetSAREAPrivate(pScreen);
  pMGA->pDRIInfo->driverSwapMethod = DRI_HIDE_X_CONTEXT;

  xf86memset( sPriv, 0, sizeof(MGASAREARec) );

  return DRIFinishScreenInit(pScreen);
}


void
mgaGetQuiescence( ScrnInfoPtr pScrn )
{
   MGAPtr pMga = MGAPTR(pScrn);

   pMga->have_quiescense = 1;					

   if (pMga->directRenderingEnabled) {
      MGAFBLayout *pLayout = &pMga->CurrentLayout;

      MgaLockUpdate(pScrn, (DRM_LOCK_QUIESCENT | DRM_LOCK_FLUSH));	

      WAITFIFO(11);
      OUTREG(MGAREG_MACCESS, pMga->MAccess);
      OUTREG(MGAREG_PITCH, pLayout->displayWidth);
      pMga->PlaneMask = ~0;
      OUTREG(MGAREG_PLNWT, pMga->PlaneMask);
      pMga->BgColor = 0;
      pMga->FgColor = 0;
      OUTREG(MGAREG_BCOL, pMga->BgColor);
      OUTREG(MGAREG_FCOL, pMga->FgColor);
      OUTREG(MGAREG_SRCORG, pMga->realSrcOrg);
      pMga->SrcOrg = 0;
      OUTREG(MGAREG_DSTORG, pMga->DstOrg);
      OUTREG(MGAREG_OPMODE, MGAOPM_DMA_BLIT);
      OUTREG(MGAREG_CXBNDRY, 0xFFFF0000); /* (maxX << 16) | minX */
      OUTREG(MGAREG_YTOP, 0x00000000);    /* minPixelPointer */
      OUTREG(MGAREG_YBOT, 0x007FFFFF);    /* maxPixelPointer */ 
      pMga->AccelFlags &= ~CLIPPER_ON;
   }
}


void
mgaGetQuiescence_shared( ScrnInfoPtr pScrn )
{
   MGAPtr pMga = MGAPTR(pScrn);
   MGAEntPtr pMgaEnt = pMga->entityPrivate;
   MGAPtr pMga2 = MGAPTR(pMgaEnt->pScrn_2);

   pMga = MGAPTR(pMgaEnt->pScrn_1);
   pMga->have_quiescense = 1;
   pMga2->have_quiescense = 1;
   
   if (pMgaEnt->directRenderingEnabled) {
      MgaLockUpdate(pMgaEnt->pScrn_1, (DRM_LOCK_QUIESCENT | DRM_LOCK_FLUSH));
      pMga->RestoreAccelState(pScrn);
      xf86SetLastScrnFlag(pScrn->entityList[0], pScrn->scrnIndex);
   }
}
   

void
MGASwapContext(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   MGAPtr pMga = MGAPTR(pScrn);

   /* Arrange for dma_quiescence and xaa sync to be called as
    * appropriate.
    */
   pMga->have_quiescense = 0;
   pMga->AccelInfoRec->NeedToSync = TRUE;
}

void
MGASwapContext_shared(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   MGAPtr pMga = MGAPTR(pScrn);
   MGAEntPtr pMgaEnt = pMga->entityPrivate;
   MGAPtr pMga2 = MGAPTR(pMgaEnt->pScrn_2);

   pMga = MGAPTR(pMgaEnt->pScrn_1);
   pMga->have_quiescense = 0;
   pMga->AccelInfoRec->NeedToSync = TRUE;
   pMga2->have_quiescense = 0;
   pMga2->AccelInfoRec->NeedToSync = TRUE;
}


/* This is really only called from validate/postvalidate as we
 * override the dri lock/unlock.  Want to remove validate/postvalidate
 * processing, but need to remove all client-side use of drawable lock
 * first (otherwise there is noone recover when a client dies holding
 * the drawable lock).
 *
 * What does this mean? 
 *
 *   - The above code gets executed every time a
 *     window changes shape or the focus changes, which isn't really
 *     optimal.  
 *   - The X server therefore believes it needs to do an XAA sync
 *     *and* a dma quiescense ioctl each time that happens.
 *
 * We don't wrap wakeuphandler any longer, so at least we can say that
 * this doesn't happen *every time the mouse moves*...
 */
static void
MGADRISwapContext(ScreenPtr pScreen, DRISyncType syncType, 
		   DRIContextType oldContextType, void *oldContext,
		   DRIContextType newContextType, void *newContext)
{
   if (syncType == DRI_3D_SYNC && 
       oldContextType == DRI_2D_CONTEXT &&
       newContextType == DRI_2D_CONTEXT)
   {
      MGASwapContext(pScreen);
   }
}

static void
MGADRISwapContext_shared(ScreenPtr pScreen, DRISyncType syncType, 
			 DRIContextType oldContextType, void *oldContext,
			 DRIContextType newContextType, void *newContext)
{   
   if (syncType == DRI_3D_SYNC && 
       oldContextType == DRI_2D_CONTEXT &&
       newContextType == DRI_2D_CONTEXT)
   {
      MGASwapContext_shared(pScreen);
   }
}

void 
MGASelectBuffer(ScrnInfoPtr pScrn, int which)
{
   MGAPtr pMga = MGAPTR(pScrn);
   MGADRIPtr pMGADRI = (MGADRIPtr)pMga->pDRIInfo->devPrivate;

   switch (which) {
   case MGA_BACK:
      OUTREG(MGAREG_DSTORG, pMGADRI->backOffset);
      OUTREG(MGAREG_SRCORG, pMGADRI->backOffset);
      break;
   case MGA_DEPTH:
      OUTREG(MGAREG_DSTORG, pMGADRI->depthOffset);
      OUTREG(MGAREG_SRCORG, pMGADRI->depthOffset);
      break;
   default:
   case MGA_FRONT:
      OUTREG(MGAREG_DSTORG, pMGADRI->frontOffset);
      OUTREG(MGAREG_SRCORG, pMGADRI->frontOffset);
      break;
   }
}
