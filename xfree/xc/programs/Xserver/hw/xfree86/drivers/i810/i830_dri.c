/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/i810/i830_dri.c,v 1.3 2002/01/09 00:37:30 dawes Exp $ */
/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.


**************************************************************************/

/* Author: Jeff Hartmann <jhartmann@valinux.com> 
 */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Priv.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "windowstr.h"

#include "GL/glxtokens.h"

#include "i810.h"
#include "i830_dri.h"
#include "i830_3d_reg.h"

static char I830KernelDriverName[] = "i830";
static char I830ClientDriverName[] = "i830";

static Bool I830InitVisualConfigs(ScreenPtr pScreen);
static Bool I830CreateContext(ScreenPtr pScreen, VisualPtr visual, 
			      drmContext hwContext, void *pVisualConfigPriv,
			      DRIContextType contextStore);
static void I830DestroyContext(ScreenPtr pScreen, drmContext hwContext,
			       DRIContextType contextStore);
static void I830DRISwapContext(ScreenPtr pScreen, DRISyncType syncType, 
			       DRIContextType readContextType, 
			       void *readContextStore,
			       DRIContextType writeContextType, 
			       void *writeContextStore);
static void I830DRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index);
static void I830DRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg, 
			       RegionPtr prgnSrc, CARD32 index);

extern void GlxSetVisualConfigs(int nconfigs,
				__GLXvisualConfig *configs,
				void **configprivs);

static int i830_pitches[] = {
   512,
   1024,
   2048,
   4096,
   8192,
   0
};

Bool I830CleanupDma(ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR(pScrn);
   Bool ret_val;
   
   ret_val = drmI830CleanupDma(pI810->drmSubFD);
   if (ret_val == FALSE)
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "I830 Dma Cleanup Failed\n");
   return ret_val;
}

Bool I830InitDma(ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR(pScrn);
   I810RingBuffer *ring = &(pI810->LpRing);
   I830DRIPtr pI830DRI=(I830DRIPtr)pI810->pDRIInfo->devPrivate;
   drmI830Init info;
   Bool ret_val;

   info.start = ring->mem.Start;
   info.end = ring->mem.End; 
   info.size = ring->mem.Size;

   info.mmio_offset = (unsigned int)pI830DRI->regs;
   info.buffers_offset = (unsigned int)pI810->buffer_map;

   info.sarea_off = sizeof(XF86DRISAREARec);

   info.front_offset = 0;
   info.back_offset = pI810->BackBuffer.Start;
   info.depth_offset = pI810->DepthBuffer.Start;
   info.w = pScrn->virtualX;
   info.h = pScrn->virtualY;
   info.pitch = pI810->auxPitch;
   info.pitch_bits = pI810->auxPitchBits;
   info.cpp = pI810->cpp;

   ret_val = drmI830InitDma(pI810->drmSubFD, &info);
   if(ret_val == FALSE) ErrorF("i830 Dma Initialization Failed\n");
   return ret_val;
}

static Bool
I830InitVisualConfigs(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   I810Ptr pI810 = I810PTR(pScrn);
   int numConfigs = 0;
   __GLXvisualConfig *pConfigs = 0;
   I810ConfigPrivPtr pI810Configs = 0;
   I810ConfigPrivPtr *pI810ConfigPtrs = 0;
   int accum, stencil, db, depth;
   int i;

   switch (pScrn->bitsPerPixel) {
   case 8:
   case 24:
      break;

   case 16:
      numConfigs = 8;

      pConfigs = (__GLXvisualConfig *) xcalloc(sizeof(__GLXvisualConfig), numConfigs);
      if (!pConfigs)
	 return FALSE;

      pI810Configs = (I810ConfigPrivPtr) xcalloc(sizeof(I810ConfigPrivRec), numConfigs);
      if (!pI810Configs) {
	 xfree(pConfigs);
	 return FALSE;
      }

      pI810ConfigPtrs = (I810ConfigPrivPtr *) xcalloc(sizeof(I810ConfigPrivPtr), numConfigs);
      if (!pI810ConfigPtrs) {
	 xfree(pConfigs);
	 xfree(pI810Configs);
	 return FALSE;
      }

      for (i=0; i<numConfigs; i++) 
	 pI810ConfigPtrs[i] = &pI810Configs[i];

      i = 0;
      depth = 1;
      for (accum = 0; accum <= 1; accum++) {
         for (stencil = 0; stencil <= 1; stencil++) {
            for (db = 1; db >= 0; db--) {
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
                  pConfigs[i].accumAlphaSize = 16;
               }
               else {
                  pConfigs[i].accumRedSize = 0;
                  pConfigs[i].accumGreenSize = 0;
                  pConfigs[i].accumBlueSize = 0;
                  pConfigs[i].accumAlphaSize = 0;
               }
               pConfigs[i].doubleBuffer = db ? TRUE : FALSE;
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
      assert(i == numConfigs);
      break;

   case 32:
      numConfigs = 8;

      pConfigs = (__GLXvisualConfig*)xcalloc( sizeof(__GLXvisualConfig),
						numConfigs );
      if ( !pConfigs ) {
	 return FALSE;
      }

      pI810Configs = (I810ConfigPrivPtr)xcalloc( sizeof(I810ConfigPrivRec),
						 numConfigs );
      if ( !pI810Configs ) {
	 xfree( pConfigs );
	 return FALSE;
      }

      pI810ConfigPtrs = (I810ConfigPrivPtr*)
				xcalloc(sizeof(I810ConfigPrivPtr),
					  numConfigs);
      if ( !pI810ConfigPtrs ) {
	 xfree( pConfigs );
	 xfree( pI810Configs );
	 return FALSE;
      }

      for ( i = 0 ; i < numConfigs ; i++ ) {
	 pI810ConfigPtrs[i] = &pI810Configs[i];
      }

      i = 0;
      for ( accum = 0 ; accum <= 1 ; accum++ ) {
         for ( depth = 0 ; depth <= 1 ; depth++ ) { /* and stencil */
            for ( db = 1 ; db >= 0 ; db-- ) {
               pConfigs[i].vid			= -1;
               pConfigs[i].class		= -1;
               pConfigs[i].rgba			= TRUE;
               pConfigs[i].redSize		= 8;
               pConfigs[i].greenSize		= 8;
               pConfigs[i].blueSize		= 8;
               pConfigs[i].alphaSize		= 0;
               pConfigs[i].redMask		= 0x00FF0000;
               pConfigs[i].greenMask		= 0x0000FF00;
               pConfigs[i].blueMask		= 0x000000FF;
               pConfigs[i].alphaMask		= 0xff000000;;
               if ( accum ) {
                  pConfigs[i].accumRedSize	= 16;
                  pConfigs[i].accumGreenSize	= 16;
                  pConfigs[i].accumBlueSize	= 16;
                  pConfigs[i].accumAlphaSize	= 0;
               } else {
                  pConfigs[i].accumRedSize	= 0;
                  pConfigs[i].accumGreenSize	= 0;
                  pConfigs[i].accumBlueSize	= 0;
                  pConfigs[i].accumAlphaSize	= 0;
               }
               if ( db ) {
                  pConfigs[i].doubleBuffer	= TRUE;
               } else {
                  pConfigs[i].doubleBuffer	= FALSE;
	       }
               pConfigs[i].stereo		= FALSE;
               pConfigs[i].bufferSize		= 32;
               if ( depth ) {
		     pConfigs[i].depthSize	= 24;
                     pConfigs[i].stencilSize	= 8;
               }
               else {
                     pConfigs[i].depthSize	= 0;
                     pConfigs[i].stencilSize	= 0;
               }
               pConfigs[i].auxBuffers		= 0;
               pConfigs[i].level		= 0;
               if ( accum ) {
                  pConfigs[i].visualRating	= GLX_SLOW_VISUAL_EXT;
               } else {
                  pConfigs[i].visualRating	= GLX_NONE_EXT;
	       }
               pConfigs[i].transparentPixel	= 0;
               pConfigs[i].transparentRed	= 0;
               pConfigs[i].transparentGreen	= 0;
               pConfigs[i].transparentBlue	= 0;
               pConfigs[i].transparentAlpha	= 0;
               pConfigs[i].transparentIndex	= 0;
               i++;
            }
         }
      }
      if ( i != numConfigs ) {
         xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		     "[drm] Incorrect initialization of visuals\n" );
         return FALSE;
      }
      break;

   }
   pI810->numVisualConfigs = numConfigs;
   pI810->pVisualConfigs = pConfigs;
   pI810->pVisualConfigsPriv = pI810Configs;
   GlxSetVisualConfigs(numConfigs, pConfigs, (void**)pI810ConfigPtrs);
   return TRUE;
}


static unsigned int mylog2(unsigned int n)
{
   unsigned int log2 = 1;
   while (n>1) n >>= 1, log2++;
   return log2;
}


Bool I830DRIScreenInit(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   I810Ptr pI810 = I810PTR(pScrn);
   DRIInfoPtr pDRIInfo;
   I830DRIPtr pI830DRI;
   unsigned long tom;
   unsigned long agpHandle;
   int sysmem_size = 0;
   int back_size = 0;
   int bufs;
   int i;

   /* Hardware 3D rendering only implemented for 16bpp and 32 bpp */
   if (((pScrn->bitsPerPixel / 8) != 2 && pScrn->depth != 16) && 
       (pScrn->bitsPerPixel / 8) != 4) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[drm] Direct rendering only supported in 16 and 32 bpp modes\n");
      return FALSE;
   }

   /* Check that the GLX, DRI, and DRM modules have been loaded by testing
    * for known symbols in each module. */
   if (!xf86LoaderCheckSymbol("GlxSetVisualConfigs")) return FALSE;
   if (!xf86LoaderCheckSymbol("DRIScreenInit"))       return FALSE;
   if (!xf86LoaderCheckSymbol("drmAvailable"))        return FALSE;
   if (!xf86LoaderCheckSymbol("DRIQueryVersion")) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[dri] %s failed (libdri.a too old)\n","I830DRIScreenInit");
      return FALSE;
   }
   
   /* Check the DRI version */
   {
      int major, minor, patch;
      DRIQueryVersion(&major, &minor, &patch);
      if (major != 4 || minor < 0) {
         xf86DrvMsg(pScreen->myNum, X_ERROR,
                    "[dri] %s failed because of a version mismatch.\n"
					"[dri] libDRI version is %d.%d.%d bug version 4.0.x is needed.\n"
					"[dri] Disabling DRI.\n",
					"I830DRIScreenInit", major, minor, patch);
         return FALSE;
      }
   }

   pDRIInfo = DRICreateInfoRec();
   if (!pDRIInfo) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[dri] DRICreateInfoRec failed. Disabling DRI.\n");
      return FALSE;
   }

   pI810->pDRIInfo = pDRIInfo;
   pI810->LockHeld = 0;

   pDRIInfo->drmDriverName = I830KernelDriverName;
   pDRIInfo->clientDriverName = I830ClientDriverName;
   pDRIInfo->busIdString = xalloc(64);

   sprintf(pDRIInfo->busIdString, "PCI:%d:%d:%d",
	   ((pciConfigPtr)pI810->PciInfo->thisCard)->busnum,
	   ((pciConfigPtr)pI810->PciInfo->thisCard)->devnum,
	   ((pciConfigPtr)pI810->PciInfo->thisCard)->funcnum);
   pDRIInfo->ddxDriverMajorVersion = I830_MAJOR_VERSION;
   pDRIInfo->ddxDriverMinorVersion = I830_MINOR_VERSION;
   pDRIInfo->ddxDriverPatchVersion = I830_PATCHLEVEL;
   pDRIInfo->frameBufferPhysicalAddress = pI810->LinearAddr;
   pDRIInfo->frameBufferSize = (((pScrn->displayWidth * 
				  pScrn->virtualY * pI810->cpp) + 
				 4096 - 1) / 4096) * 4096;

   pDRIInfo->frameBufferStride = pScrn->displayWidth*pI810->cpp;
   pDRIInfo->ddxDrawableTableEntry = I830_MAX_DRAWABLES;

   if (SAREA_MAX_DRAWABLES < I830_MAX_DRAWABLES)
      pDRIInfo->maxDrawableTableEntry = SAREA_MAX_DRAWABLES;
   else
      pDRIInfo->maxDrawableTableEntry = I830_MAX_DRAWABLES;

   if (sizeof(XF86DRISAREARec)+sizeof(I830SAREARec)>SAREA_MAX) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[dri] Data does not fit in SAREA\n");
      return FALSE;
   }
   /* This is a hack for now.  We have to have more than a 4k page here
    * because of the size of the state.  However, the state should be
    * in a per-context mapping.  This will be added in the Mesa 3.5 port
    * of the I830 driver.
    */
   pDRIInfo->SAREASize = SAREA_MAX;

   if (!(pI830DRI = (I830DRIPtr)xcalloc(sizeof(I830DRIRec),1))) {
      DRIDestroyInfoRec(pI810->pDRIInfo);
      pI810->pDRIInfo=0;
      return FALSE;
   }
   pDRIInfo->devPrivate = pI830DRI;
   pDRIInfo->devPrivateSize = sizeof(I830DRIRec);
   pDRIInfo->contextSize = sizeof(I830DRIContextRec);
   
   pDRIInfo->CreateContext = I830CreateContext;
   pDRIInfo->DestroyContext = I830DestroyContext;
   pDRIInfo->SwapContext = I830DRISwapContext;
   pDRIInfo->InitBuffers = I830DRIInitBuffers;
   pDRIInfo->MoveBuffers = I830DRIMoveBuffers;
   pDRIInfo->bufferRequests = DRI_ALL_WINDOWS;

   /* This adds the framebuffer as a drm map *before* we have asked agp
    * to allocate it.  Scary stuff, hold on...
    */
   if (!DRIScreenInit(pScreen, pDRIInfo, &pI810->drmSubFD)) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[dri] DRIScreenInit failed. Disabling DRI.\n");
      xfree(pDRIInfo->devPrivate);
      pDRIInfo->devPrivate=0;
      DRIDestroyInfoRec(pI810->pDRIInfo);
      pI810->pDRIInfo=0;
      return FALSE;
   }
   
   /* Check the i830 DRM version */
   {
      drmVersionPtr version = drmGetVersion(pI810->drmSubFD);
      if (version) {
         if (version->version_major != 1 ||
             version->version_minor < 2) {
            /* incompatible drm version */
            xf86DrvMsg(pScreen->myNum, X_ERROR,
                       "[dri] %s failed because of a version mismatch.\n"
					   "[dri] i830.o kernel module version is %d.%d.%d but version 1.2 or greater is needed.\n"
					   "[dri] Disabling DRI.\n",
					   "I830DRIScreenInit",
                       version->version_major,
                       version->version_minor,
                       version->version_patchlevel);
            I830DRICloseScreen(pScreen);
            drmFreeVersion(version);
            return FALSE;
         }
         drmFreeVersion(version);
      }
   }

   pI830DRI->regsSize=I830_REG_SIZE;
   if (drmAddMap(pI810->drmSubFD, (drmHandle)pI810->MMIOAddr, 
		 pI830DRI->regsSize, DRM_REGISTERS, 0, &pI830DRI->regs)<0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] drmAddMap(regs) failed\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   xf86DrvMsg(pScreen->myNum, X_INFO, "[drm] Registers = 0x%08lx\n",
	      pI830DRI->regs);
   
   pI810->backHandle = 0;
   pI810->zHandle = 0;
   pI810->cursorHandle = 0;
   pI810->sysmemHandle = 0;
   pI810->agpAcquired = FALSE;

   /* Agp Support - Need this just to get the framebuffer.
    */
   if(drmAgpAcquire(pI810->drmSubFD) < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] drmAgpAquire failed\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   pI810->agpAcquired = TRUE;
   
   if (drmAgpEnable(pI810->drmSubFD, 0) < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] drmAgpEnable failed\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   memset (&pI810->BackBuffer, 0, sizeof(I810MemRange));
   memset (&pI810->DepthBuffer, 0, sizeof(I810MemRange));
   pI810->CursorPhysical = 0;

   back_size = (((pScrn->displayWidth * 
		  pScrn->virtualY * pI810->cpp) + 
		 4096 - 1) / 4096) * 4096;

   sysmem_size = pScrn->videoRam * 1024;

   /* The 1 meg here is for texture space and the ring buffer. */
   if(sysmem_size < (3 * back_size + 0x100000)) {
      xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		 "[dri] Not enough memory to contain front, back, and depth buffers.\n");
      xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		 "[dri] You need at least %dK of VideoRam for this configuration.\n",
				 (3 * back_size + 0x100000) / 1024);
      xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		 "[dri] Disabling DRI\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   sysmem_size = sysmem_size - 2*back_size;

   if(sysmem_size > pI810->FbMapSize) {
      sysmem_size = pI810->FbMapSize;

      xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
		 "[dri] User requested more memory then fits in the agp aperture\n"
		 "[dri] Truncating to %d bytes of memory\n",
		 sysmem_size);
   }

   sysmem_size -= 4096;		/* remove 4k for the hw cursor */

   if(sysmem_size < 0) {
      xf86DrvMsg(pScrn->scrnIndex, X_INFO,
				 "[dri] Not enough memory to contain front, back, and depth buffers.\n"
				 "[dri] Disabling DRI.\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   drmAgpAlloc(pI810->drmSubFD, back_size, 0, NULL, &agpHandle);
   pI810->backHandle = agpHandle;

   if (agpHandle != 0) {
      if(drmAgpBind(pI810->drmSubFD, agpHandle, pI810->BackOffset) == 0) {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		    "[dri] Bound backbuffer memory\n");
	 
	 pI810->BackBuffer.Start = pI810->BackOffset;
	 pI810->BackBuffer.Size = back_size;
	 pI810->BackBuffer.End = (pI810->BackBuffer.Start + 
				  pI810->BackBuffer.Size);
      } else {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[dri] Unable to bind backbuffer\n");
	 DRICloseScreen(pScreen);
	 return FALSE;
      }
   } else {
      xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		 "[dri] Unable to allocate backbuffer memory\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
      
   drmAgpAlloc(pI810->drmSubFD, back_size, 0,
	       NULL, &agpHandle);
   pI810->zHandle = agpHandle;

   if(agpHandle != 0) {
      if(drmAgpBind(pI810->drmSubFD, agpHandle, pI810->DepthOffset) == 0) {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[dri] Bound depthbuffer memory\n");
	 pI810->DepthBuffer.Start = pI810->DepthOffset;
	 pI810->DepthBuffer.Size = back_size;
	 pI810->DepthBuffer.End = (pI810->DepthBuffer.Start + 
				   pI810->DepthBuffer.Size);
      } else {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
		    "[dri] Unable to bind depthbuffer\n");
	 DRICloseScreen(pScreen);
	 return FALSE;
      }
   } else {
      xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		 "[dri] Unable to allocate depthbuffer memory\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   /* Now allocate and bind the agp space.  This memory will include the
    * regular framebuffer as well as texture memory.
    */
   {
      int offset, alloc_size;

      alloc_size = sysmem_size;

      if(alloc_size <= pI810->StolenSize) {
	 /* We are cheating here.  We really should not scale the sysmem_size.
	  * However we always have at least stolen size here. */
	 alloc_size = 0;
	 offset = 0;
	 sysmem_size = pI810->StolenSize;
      } else {
	 alloc_size -= pI810->StolenSize; /* Stolen size is in bytes */
	 offset = pI810->StolenSize;
      }

      if(alloc_size) {
	 drmAgpAlloc(pI810->drmSubFD, alloc_size, 0, NULL, &agpHandle);
	 if (agpHandle == 0) {
	    xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] drmAgpAlloc failed\n");
	    DRICloseScreen(pScreen);
	    return FALSE;
	 }
	 if (drmAgpBind(pI810->drmSubFD, agpHandle, offset) != 0) {
	    xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] drmAgpBind failed\n");
	    DRICloseScreen(pScreen);
	    return FALSE;
	 }

	 pI810->sysmemHandle = agpHandle;
      } else {
	 pI810->sysmemHandle = 0;
      }

      pI810->SysMem.Start = 0;
      pI810->SysMem.Size = sysmem_size;
      pI810->SysMem.End = sysmem_size;
      pI810->SavedSysMem = pI810->SysMem;

      if(sysmem_size <= pI810->StolenSize) {
	 tom = pI810->StolenSize;
      } else {
	 tom = pI810->SysMem.End;
      }

   }

   drmAgpAlloc(pI810->drmSubFD, 4096, 2, 
	       (unsigned long *)&pI810->CursorPhysical, &agpHandle); 
   pI810->cursorHandle = agpHandle;

   if (agpHandle != 0) {
      if (drmAgpBind(pI810->drmSubFD, agpHandle, tom) == 0) { 
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
		    "[agp] GART: Allocated 4K for mouse cursor image\n");
	 pI810->CursorStart = tom;	 
	 tom += 4096;
      }
      else {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[agp] GART: cursor bind failed\n");
	 pI810->CursorPhysical = 0;    
      } 
   }
   else {
      xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[agp] GART: cursor alloc failed\n");
      pI810->CursorPhysical = 0;
   }

   /* Steal some of the excess cursor space for the overlay regs,
    * then allocate 202*2 pages for the overlay buffers.
    */
   pI810->OverlayPhysical = pI810->CursorPhysical + 1024;
   pI810->OverlayStart = pI810->CursorStart + 1024;

   /* drmAddMap happens later to preserve index order */

   /* The tiled registers always describe 8 meg regions, even if
    * the actually memory used is less than 8 meg (which is the normal
    * case.)
    */

   {
      int can_tile, i;

#define Elements(x) sizeof(x)/sizeof(*x)

      for(i = 0, can_tile = 0; i < Elements(i830_pitches) ; i++) {
	 if((pScrn->displayWidth * pI810->cpp) == i830_pitches[i])
	   can_tile = 1;
      }

      if(can_tile) {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[dri] Activating Tiled Memory\n");
	 I810SetTiledMemory(pScrn, 1,
			    pI810->DepthBuffer.Start,
			    pScrn->displayWidth * pI810->cpp,
			    8*1024*1024);

	 I810SetTiledMemory(pScrn, 2,
			    pI810->BackBuffer.Start,
			    pScrn->displayWidth * pI810->cpp,
			    8*1024*1024);
      }
   }

   pI810->auxPitch = pScrn->displayWidth;
   pI810->auxPitchBits = 0;

   pI830DRI->backbufferSize = pI810->BackBuffer.Size;

   if (drmAddMap(pI810->drmSubFD, (drmHandle)pI810->BackBuffer.Start,
		 pI810->BackBuffer.Size, DRM_AGP, 0, 
		 &pI830DRI->backbuffer) < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] drmAddMap(backbuffer) failed. Disabling DRI\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   
   pI830DRI->depthbufferSize = pI810->DepthBuffer.Size;
   if (drmAddMap(pI810->drmSubFD, (drmHandle)pI810->DepthBuffer.Start,
		 pI810->DepthBuffer.Size, DRM_AGP, 0, 
		 &pI830DRI->depthbuffer) < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] drmAddMap(depthbuffer) failed. Disabling DRI\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   
   /* Allocate FrontBuffer etc. */
   I810AllocateFront(pScrn);

   /* Allocate memory for the logical context, 32k is fine for right now */
   I810AllocHigh( &(pI810->ContextMem), &(pI810->SysMem),
		 32768 );

   /* Allocate buffer memory */
   I810AllocHigh( &(pI810->BufferMem), &(pI810->SysMem), 
		  I830_DMA_BUF_NR * I830_DMA_BUF_SZ);
   
   xf86DrvMsg(pScreen->myNum, X_INFO, "[dri] Buffer map : %lx\n",
              pI810->BufferMem.Start);
   
   if (pI810->BufferMem.Start == 0 || 
      pI810->BufferMem.End - pI810->BufferMem.Start > 
      I830_DMA_BUF_NR * I830_DMA_BUF_SZ) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[dri] Not enough memory for dma buffers. Disabling DRI\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   if (drmAddMap(pI810->drmSubFD, (drmHandle)pI810->BufferMem.Start,
		pI810->BufferMem.Size, DRM_AGP, 0,
		&pI810->buffer_map) < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] drmAddMap(buffer_map) failed. Disabling DRI\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   pI830DRI->agp_buffers = pI810->buffer_map;
   pI830DRI->agp_buf_size = pI810->BufferMem.Size;

   if (drmAddMap(pI810->drmSubFD, (drmHandle)pI810->LpRing.mem.Start,
		 pI810->LpRing.mem.Size, DRM_AGP, 0,
		 &pI810->ring_map) < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] drmAddMap(ring_map) failed. Disabling DRI\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   
   /* Use the rest of memory for textures. */
   pI830DRI->textureSize = pI810->SysMem.Size;

   i = mylog2(pI830DRI->textureSize / I830_NR_TEX_REGIONS);

   if (i < I830_LOG_MIN_TEX_REGION_SIZE)
      i = I830_LOG_MIN_TEX_REGION_SIZE;

   pI830DRI->logTextureGranularity = i;
   pI830DRI->textureSize = (pI830DRI->textureSize >> i) << i; /* truncate */

   if(pI830DRI->textureSize < 512*1024) {
      ErrorF("Less then 512k for textures\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   
   I810AllocLow( &(pI810->TexMem), &(pI810->SysMem),
		 pI830DRI->textureSize);
   
   if (drmAddMap(pI810->drmSubFD, (drmHandle)pI810->TexMem.Start,
		 pI810->TexMem.Size, DRM_AGP, 0,
		 &pI830DRI->textures) < 0) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[drm] drmAddMap(textures) failed. Disabling DRI\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }
   
   if((bufs = drmAddBufs(pI810->drmSubFD,
			 I830_DMA_BUF_NR,
			 I830_DMA_BUF_SZ,
			 DRM_AGP_BUFFER, pI810->BufferMem.Start)) <= 0) {
      xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		 "[drm] failure adding %d %d byte DMA buffers\n",
		 I830_DMA_BUF_NR,
		 I830_DMA_BUF_SZ);
      DRICloseScreen(pScreen);
      return FALSE;
   }

   xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[drm] added %d %d byte DMA buffers\n",
	      bufs, I830_DMA_BUF_SZ);


   xf86EnablePciBusMaster(pI810->PciInfo, TRUE);

   I830InitDma(pScrn);
   
   /* Okay now initialize the dma engine */
#if 1
   if (!pI830DRI->irq) {
      pI830DRI->irq = drmGetInterruptFromBusID(pI810->drmSubFD,
					       ((pciConfigPtr)pI810->PciInfo->thisCard)->busnum,
					       ((pciConfigPtr)pI810->PciInfo->thisCard)->devnum,
					       ((pciConfigPtr)pI810->PciInfo->thisCard)->funcnum);
      if((drmCtlInstHandler(pI810->drmSubFD, pI830DRI->irq)) != 0) {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		    "[drm] failure adding irq handler, there is a device already using that irq\n"
			"[drm] Consider rearranging your PCI cards\n");
	 DRICloseScreen(pScreen);
	 return FALSE;
      }
   }

   xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	      "[drm] dma control initialized, using IRQ %d\n",
	      pI830DRI->irq);
#endif

   pI830DRI=(I830DRIPtr)pI810->pDRIInfo->devPrivate;
   pI830DRI->deviceID=pI810->PciInfo->chipType;
   pI830DRI->width=pScrn->virtualX;
   pI830DRI->height=pScrn->virtualY;
   pI830DRI->mem=pScrn->videoRam*1024;
   pI830DRI->cpp=pI810->cpp;
   
   pI830DRI->fbOffset=pI810->FrontBuffer.Start;
   pI830DRI->fbStride=pI810->auxPitch;
   
   pI830DRI->bitsPerPixel = pScrn->bitsPerPixel;
   
   
   pI830DRI->textureOffset=pI810->TexMem.Start;
   
   pI830DRI->backOffset=pI810->BackBuffer.Start;
   pI830DRI->depthOffset=pI810->DepthBuffer.Start;
   
   pI830DRI->ringOffset=pI810->LpRing.mem.Start;
   pI830DRI->ringSize=pI810->LpRing.mem.Size;
   
   pI830DRI->auxPitch = pI810->auxPitch;
   pI830DRI->auxPitchBits = pI810->auxPitchBits;
   pI830DRI->sarea_priv_offset = sizeof(XF86DRISAREARec);

   if (!(I830InitVisualConfigs(pScreen))) {
      xf86DrvMsg(pScreen->myNum, X_ERROR, "[dri] I830InitVisualConfigs failed. Disabling DRI\n");
      DRICloseScreen(pScreen);
      return FALSE;
   }

   xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[dri] visual configs initialized\n" );
   pI810->pDRIInfo->driverSwapMethod = DRI_HIDE_X_CONTEXT;

   return TRUE;
}

void
I830DRICloseScreen(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   I810Ptr pI810 = I810PTR(pScrn);

   I830CleanupDma(pScrn);

   if(pI810->backHandle) drmAgpFree(pI810->drmSubFD, pI810->backHandle);
   if(pI810->zHandle) drmAgpFree(pI810->drmSubFD, pI810->zHandle);
   if(pI810->cursorHandle) drmAgpFree(pI810->drmSubFD, pI810->cursorHandle);
   if(pI810->sysmemHandle) drmAgpFree(pI810->drmSubFD, pI810->sysmemHandle);

   if(pI810->agpAcquired == TRUE) drmAgpRelease(pI810->drmSubFD);
   
   pI810->backHandle = 0;
   pI810->zHandle = 0;
   pI810->cursorHandle = 0;
   pI810->sysmemHandle = 0;
   pI810->agpAcquired = FALSE;

   
   DRICloseScreen(pScreen);

   if (pI810->pDRIInfo) {
      if (pI810->pDRIInfo->devPrivate) {
	 xfree(pI810->pDRIInfo->devPrivate);
	 pI810->pDRIInfo->devPrivate=0;
      }
      DRIDestroyInfoRec(pI810->pDRIInfo);
      pI810->pDRIInfo=0;
   }
   if (pI810->pVisualConfigs) xfree(pI810->pVisualConfigs);
   if (pI810->pVisualConfigsPriv) xfree(pI810->pVisualConfigsPriv);
}

static Bool
I830CreateContext(ScreenPtr pScreen, VisualPtr visual, 
		  drmContext hwContext, void *pVisualConfigPriv,
		  DRIContextType contextStore)
{
   return TRUE;
}

static void
I830DestroyContext(ScreenPtr pScreen, drmContext hwContext, 
		   DRIContextType contextStore)
{
}


Bool
I830DRIFinishScreenInit(ScreenPtr pScreen)
{
   I830SAREARec *sPriv = (I830SAREARec *)DRIGetSAREAPrivate(pScreen);
   memset( sPriv, 0, sizeof(sPriv) );
   return DRIFinishScreenInit(pScreen);
}

void
I830DRISwapContext(ScreenPtr pScreen, DRISyncType syncType, 
		   DRIContextType oldContextType, void *oldContext,
		   DRIContextType newContextType, void *newContext)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   I810Ptr pI810 = I810PTR(pScrn);

   if (syncType == DRI_3D_SYNC && 
       oldContextType == DRI_2D_CONTEXT &&
       newContextType == DRI_2D_CONTEXT) 
   { 
      ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

      if (I810_DEBUG & DEBUG_VERBOSE_DRI)
	 ErrorF("i830DRISwapContext (in)\n");
   
      pI810->LockHeld = 1;
      I810RefreshRing( pScrn );
   }
   else if (syncType == DRI_2D_SYNC && 
	    oldContextType == DRI_NO_CONTEXT &&
	    newContextType == DRI_2D_CONTEXT) 
   { 
      pI810->LockHeld = 0;
      if (I810_DEBUG & DEBUG_VERBOSE_DRI)
	 ErrorF("i830DRISwapContext (out)\n");
   }
   else if (I810_DEBUG & DEBUG_VERBOSE_DRI)
      ErrorF("i830DRISwapContext (other)\n");
}

static void
I830DRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index)
{
   ScreenPtr pScreen = pWin->drawable.pScreen;
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   I810Ptr pI810 = I810PTR(pScrn);
   BoxPtr pbox = REGION_RECTS(prgn);
   int nbox = REGION_NUM_RECTS(prgn);

   if (I810_DEBUG & DEBUG_VERBOSE_DRI)
      ErrorF( "I830DRIInitBuffers\n");

   I830SetupForSolidFill(pScrn, 0, GXcopy, -1);
   while (nbox--) {
      I810SelectBuffer(pScrn, I810_BACK);
      I830SubsequentSolidFillRect(pScrn, pbox->x1, pbox->y1, 
				  pbox->x2-pbox->x1, pbox->y2-pbox->y1);
      pbox++;
   }

   /* Clear the depth buffer - uses 0xffff rather than 0.
    */
   pbox = REGION_RECTS(prgn);
   nbox = REGION_NUM_RECTS(prgn);

   I810SelectBuffer(pScrn, I810_DEPTH);

   switch (pScrn->bitsPerPixel) {
   case 16: I830SetupForSolidFill(pScrn, 0xffff, GXcopy, -1); break;
   case 32: I830SetupForSolidFill(pScrn, 0xffffff, GXcopy, -1); break;
   }

   while (nbox--) {  
      I830SubsequentSolidFillRect(pScrn, pbox->x1, pbox->y1, 
				  pbox->x2-pbox->x1, pbox->y2-pbox->y1);
      pbox++;
   }

   I810SelectBuffer(pScrn, I810_FRONT);
   pI810->AccelInfoRec->NeedToSync = TRUE;
}

/* This routine is a modified form of XAADoBitBlt with the calls to
 * ScreenToScreenBitBlt built in. My routine has the prgnSrc as source
 * instead of destination. My origin is upside down so the ydir cases
 * are reversed. 
 */
static void
I830DRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg, 
		   RegionPtr prgnSrc, CARD32 index)
{
   ScreenPtr pScreen = pParent->drawable.pScreen;
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   I810Ptr pI810 = I810PTR(pScrn);
   BoxPtr pboxTmp, pboxNext, pboxBase;
   DDXPointPtr pptTmp, pptNew2;
   int xdir, ydir;

   int screenwidth = pScrn->virtualX;
   int screenheight = pScrn->virtualY;

   BoxPtr pbox = REGION_RECTS(prgnSrc);
   int nbox = REGION_NUM_RECTS(prgnSrc);

   BoxPtr pboxNew1 = 0;
   BoxPtr pboxNew2 = 0;
   DDXPointPtr pptNew1 = 0;
   DDXPointPtr pptSrc = &ptOldOrg;

   int dx = pParent->drawable.x - ptOldOrg.x;
   int dy = pParent->drawable.y - ptOldOrg.y;

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

   /* SelectBuffer isn't really a good concept for the i810.
    */
   I810EmitFlush(pScrn);
   I830SetupForScreenToScreenCopy(pScrn, xdir, ydir, GXcopy, -1, -1);
   for ( ; nbox-- ; pbox++ ) {
      
      int x1 = pbox->x1;
      int y1 = pbox->y1;
      int destx = x1 + dx;
      int desty = y1 + dy;
      int w = pbox->x2 - x1 + 1;
      int h = pbox->y2 - y1 + 1;
      
      if ( destx < 0 ) x1 -= destx, w += destx, destx = 0; 
      if ( desty < 0 ) y1 -= desty, h += desty, desty = 0;
      if ( destx + w > screenwidth ) w = screenwidth - destx;
      if ( desty + h > screenheight ) h = screenheight - desty;
      if ( w <= 0 ) continue;
      if ( h <= 0 ) continue;
      

      if (I810_DEBUG & DEBUG_VERBOSE_DRI)
	 ErrorF( "MoveBuffers %d,%d %dx%d dx: %d dy: %d\n",
		 x1, y1, w, h, dx, dy);

      I810SelectBuffer(pScrn, I810_BACK);
      I830SubsequentScreenToScreenCopy(pScrn, x1, y1, destx, desty, w, h);
      I810SelectBuffer(pScrn, I810_DEPTH);
      I830SubsequentScreenToScreenCopy(pScrn, x1, y1, destx, desty, w, h);
   }
   I810SelectBuffer(pScrn, I810_FRONT);
   I810EmitFlush(pScrn);

   if (pboxNew2) {
      DEALLOCATE_LOCAL(pptNew2);
      DEALLOCATE_LOCAL(pboxNew2);
   }
   if (pboxNew1) {
      DEALLOCATE_LOCAL(pptNew1);
      DEALLOCATE_LOCAL(pboxNew1);
   }

   pI810->AccelInfoRec->NeedToSync = TRUE;
}

/* Completely Initialize the first context */
void 
I830EmitInvarientState(ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR(pScrn);
   I830DRIPtr pI830DRI = (I830DRIPtr)pI810->pDRIInfo->devPrivate;
   CARD32 ctx_addr, vtx_addr, vtx1_addr, temp;
   BEGIN_LP_RING( 128 );

   ctx_addr = pI810->ContextMem.Start;
   /* Align to a 2k boundry */
   ctx_addr = ((ctx_addr + 2048 - 1) / 2048) * 2048;

   /* Carve out some context memory for the vertex buffers, if we ever
    * use these directly, we will need to change this
    */
   vtx_addr = ctx_addr + 8192;
   vtx1_addr = ctx_addr + 16384;

   OUT_RING(MI_SET_CONTEXT);
   OUT_RING(ctx_addr |
	    CTXT_NO_RESTORE |
	    CTXT_PALETTE_SAVE_DISABLE |
	    CTXT_PALETTE_RESTORE_DISABLE);

   /* Zero pitch and width make the vertex buffer match vertex format */
   OUT_RING(MI_VERTEX_BUFFER |
	    MI_VERTEX_BUFFER_IDX(0) |
	    MI_VERTEX_BUFFER_PITCH(0) |
	    MI_VERTEX_BUFFER_WIDTH(0));
   OUT_RING(vtx_addr);

   /* Setting zero pitch and width is undefined so we have to set these to
    * one, even though we just disable this buffer.
    */

   OUT_RING(MI_VERTEX_BUFFER |
	    MI_VERTEX_BUFFER_IDX(1) |
	    MI_VERTEX_BUFFER_PITCH(1) |
	    MI_VERTEX_BUFFER_WIDTH(1));
   OUT_RING(vtx1_addr |
	    MI_VERTEX_BUFFER_DISABLE);

   OUT_RING(STATE3D_AA_CMD |
	    AA_LINE_ECAAR_WIDTH_ENABLE |
	    AA_LINE_ECAAR_WIDTH_1_0 |
	    AA_LINE_REGION_WIDTH_ENABLE |
	    AA_LINE_REGION_WIDTH_1_0 |
	    AA_LINE_DISABLE);

   OUT_RING(STATE3D_BUF_INFO_CMD);
   OUT_RING(BUF_3D_ID_COLOR_BACK |
	    BUF_3D_USE_FENCE |
	    BUF_3D_PITCH((pI810->cpp * pScrn->displayWidth) / 4));
   OUT_RING(BUF_3D_ADDR(pI830DRI->backOffset));

   OUT_RING(STATE3D_BUF_INFO_CMD);
   OUT_RING(BUF_3D_ID_DEPTH |
	    BUF_3D_USE_FENCE |
	    BUF_3D_PITCH((pI810->cpp * pScrn->displayWidth) / 4));
   OUT_RING(BUF_3D_ADDR(pI830DRI->depthOffset));

   OUT_RING(STATE3D_COLOR_FACTOR);
   OUT_RING(0);

   OUT_RING(STATE3D_COLOR_FACTOR_CMD(0));
   OUT_RING(0);

   OUT_RING(STATE3D_COLOR_FACTOR_CMD(1));
   OUT_RING(0);

   OUT_RING(STATE3D_COLOR_FACTOR_CMD(2));
   OUT_RING(0);

   OUT_RING(STATE3D_COLOR_FACTOR_CMD(3));
   OUT_RING(0);

   OUT_RING(STATE3D_CONST_BLEND_COLOR_CMD);
   OUT_RING(0);

   OUT_RING(STATE3D_DFLT_DIFFUSE_CMD);
   OUT_RING(0);

   OUT_RING(STATE3D_DFLT_SPEC_CMD);
   OUT_RING(0);

   OUT_RING(STATE3D_DFLT_Z_CMD);
   OUT_RING(0);

   switch(pScrn->bitsPerPixel) {
   case 15:
      temp = DEPTH_FRMT_16_FIXED | COLR_BUF_RGB555;
      break;
   case 16:
      temp = DEPTH_FRMT_16_FIXED | COLR_BUF_RGB565;
      break;
   case 32:
      temp = DEPTH_FRMT_24_FIXED_8_OTHER | COLR_BUF_ARGB8888;
      break;
   default:
      temp = DEPTH_FRMT_16_FIXED | COLR_BUF_RGB565;
      break;
   }

   OUT_RING(STATE3D_DST_BUF_VARS_CMD);
   OUT_RING(DSTORG_HORT_BIAS(0x8) |
	    DSTORG_VERT_BIAS(0x8) |
	    DEPTH_IS_Z |
	    temp);

   OUT_RING(STATE3D_DRAW_RECT_CMD);
   OUT_RING(DRAW_RECT_DIS_DEPTH_OFS);
   OUT_RING(0);
   OUT_RING((pI830DRI->height<<16) |
	    pI830DRI->width);
   OUT_RING(0);

   OUT_RING(STATE3D_ENABLES_1_CMD |
	    DISABLE_LOGIC_OP |
	    DISABLE_STENCIL_TEST |
	    DISABLE_DEPTH_BIAS |
	    DISABLE_SPEC_ADD |
	    I830_DISABLE_FOG |
	    DISABLE_ALPHA_TEST |
	    DISABLE_COLOR_BLEND |
	    DISABLE_DEPTH_TEST);

   OUT_RING(STATE3D_ENABLES_2_CMD |
	    DISABLE_STENCIL_WRITE |
	    ENABLE_TEX_CACHE |
	    ENABLE_DITHER |
	    ENABLE_COLOR_MASK |
	    ENABLE_COLOR_WRITE |
	    ENABLE_DEPTH_WRITE);

   OUT_RING(STATE3D_FOG_COLOR_CMD |
	    FOG_COLOR_RED(0) |
	    FOG_COLOR_GREEN(0) |
	    FOG_COLOR_BLUE(0));

   OUT_RING(STATE3D_FOG_MODE);
   OUT_RING(FOG_MODE_VERTEX |
	    ENABLE_FOG_CONST |
	    ENABLE_FOG_SOURCE |
	    ENABLE_FOG_DENSITY);
   OUT_RING(0);
   OUT_RING(0);

   OUT_RING(STATE3D_INDPT_ALPHA_BLEND_CMD |
	    DISABLE_INDPT_ALPHA_BLEND |
	    ENABLE_ALPHA_BLENDFUNC |
	    ABLENDFUNC_ADD |
	    ENABLE_SRC_ABLEND_FACTOR |
	    SRC_ABLEND_FACT(BLENDFACT_ONE) |
	    ENABLE_DST_ABLEND_FACTOR |
	    SRC_ABLEND_FACT(BLENDFACT_ZERO));

   /* I need to come back to texture state */
   OUT_RING(STATE3D_MAP_BLEND_ARG_CMD(0) |
	    TEXPIPE_COLOR |
	    TEXBLEND_ARG1 |
	    TEXBLENDARG_MODIFY_PARMS |
	    TEXBLENDARG_DIFFUSE);
   OUT_RING(STATE3D_MAP_BLEND_ARG_CMD(1) |
	    TEXPIPE_COLOR |
	    TEXBLEND_ARG1 |
	    TEXBLENDARG_MODIFY_PARMS |
	    TEXBLENDARG_DIFFUSE);
   OUT_RING(STATE3D_MAP_BLEND_ARG_CMD(2) |
	    TEXPIPE_COLOR |
	    TEXBLEND_ARG1 |
	    TEXBLENDARG_MODIFY_PARMS |
	    TEXBLENDARG_DIFFUSE);
   OUT_RING(STATE3D_MAP_BLEND_ARG_CMD(3) |
	    TEXPIPE_COLOR |
	    TEXBLEND_ARG1 |
	    TEXBLENDARG_MODIFY_PARMS |
	    TEXBLENDARG_DIFFUSE);

   OUT_RING(STATE3D_MAP_BLEND_ARG_CMD(0) |
	    TEXPIPE_ALPHA |
	    TEXBLEND_ARG1 |
	    TEXBLENDARG_MODIFY_PARMS |
	    TEXBLENDARG_DIFFUSE);
   OUT_RING(STATE3D_MAP_BLEND_ARG_CMD(1) |
	    TEXPIPE_ALPHA |
	    TEXBLEND_ARG1 |
	    TEXBLENDARG_MODIFY_PARMS |
	    TEXBLENDARG_DIFFUSE);
   OUT_RING(STATE3D_MAP_BLEND_ARG_CMD(2) |
	    TEXPIPE_ALPHA |
	    TEXBLEND_ARG1 |
	    TEXBLENDARG_MODIFY_PARMS |
	    TEXBLENDARG_DIFFUSE);
   OUT_RING(STATE3D_MAP_BLEND_ARG_CMD(3) |
	    TEXPIPE_ALPHA |
	    TEXBLEND_ARG1 |
	    TEXBLENDARG_MODIFY_PARMS |
	    TEXBLENDARG_DIFFUSE);

   OUT_RING(STATE3D_MAP_BLEND_OP_CMD(0) |
	    TEXPIPE_COLOR |
	    ENABLE_TEXOUTPUT_WRT_SEL |
	    TEXOP_OUTPUT_CURRENT |
	    DISABLE_TEX_CNTRL_STAGE |
	    TEXOP_SCALE_1X |
	    TEXOP_MODIFY_PARMS |
	    TEXOP_LAST_STAGE |
	    TEXBLENDOP_ARG1);
   OUT_RING(STATE3D_MAP_BLEND_OP_CMD(0) |
	    TEXPIPE_ALPHA |
	    ENABLE_TEXOUTPUT_WRT_SEL |
	    TEXOP_OUTPUT_CURRENT |
	    TEXOP_SCALE_1X |
	    TEXOP_MODIFY_PARMS |
	    TEXBLENDOP_ARG1);

   OUT_RING(STATE3D_MAP_COORD_SETBIND_CMD);
   OUT_RING(TEXBIND_SET3(TEXCOORDSRC_DEFAULT) |
	    TEXBIND_SET2(TEXCOORDSRC_DEFAULT) |
	    TEXBIND_SET1(TEXCOORDSRC_DEFAULT) |
	    TEXBIND_SET0(TEXCOORDSRC_DEFAULT));

   OUT_RING(STATE3D_MAP_COORD_SET_CMD |
	    MAP_UNIT(0) |
	    TEXCOORDS_ARE_IN_TEXELUNITS |
	    TEXCOORDTYPE_CARTESIAN |
	    ENABLE_ADDR_V_CNTL |
	    ENABLE_ADDR_U_CNTL |
	    TEXCOORD_ADDR_V_MODE(TEXCOORDMODE_CLAMP) |
	    TEXCOORD_ADDR_U_MODE(TEXCOORDMODE_CLAMP));
   OUT_RING(STATE3D_MAP_COORD_SET_CMD |
	    MAP_UNIT(1) |
	    TEXCOORDS_ARE_IN_TEXELUNITS |
	    TEXCOORDTYPE_CARTESIAN |
	    ENABLE_ADDR_V_CNTL |
	    ENABLE_ADDR_U_CNTL |
	    TEXCOORD_ADDR_V_MODE(TEXCOORDMODE_CLAMP) |
	    TEXCOORD_ADDR_U_MODE(TEXCOORDMODE_CLAMP));
   OUT_RING(STATE3D_MAP_COORD_SET_CMD |
	    MAP_UNIT(2) |
	    TEXCOORDS_ARE_IN_TEXELUNITS |
	    TEXCOORDTYPE_CARTESIAN |
	    ENABLE_ADDR_V_CNTL |
	    ENABLE_ADDR_U_CNTL |
	    TEXCOORD_ADDR_V_MODE(TEXCOORDMODE_CLAMP) |
	    TEXCOORD_ADDR_U_MODE(TEXCOORDMODE_CLAMP));
   OUT_RING(STATE3D_MAP_COORD_SET_CMD |
	    MAP_UNIT(3) |
	    TEXCOORDS_ARE_IN_TEXELUNITS |
	    TEXCOORDTYPE_CARTESIAN |
	    ENABLE_ADDR_V_CNTL |
	    ENABLE_ADDR_U_CNTL |
	    TEXCOORD_ADDR_V_MODE(TEXCOORDMODE_CLAMP) |
	    TEXCOORD_ADDR_U_MODE(TEXCOORDMODE_CLAMP));

   OUT_RING(STATE3D_MAP_TEX_STREAM_CMD |
	    MAP_UNIT(0) |
	    DISABLE_TEX_STREAM_BUMP |
	    ENABLE_TEX_STREAM_COORD_SET |
	    TEX_STREAM_COORD_SET(0) |
	    ENABLE_TEX_STREAM_MAP_IDX |
	    TEX_STREAM_MAP_IDX(0));
   OUT_RING(STATE3D_MAP_TEX_STREAM_CMD |
	    MAP_UNIT(1) |
	    DISABLE_TEX_STREAM_BUMP |
	    ENABLE_TEX_STREAM_COORD_SET |
	    TEX_STREAM_COORD_SET(1) |
	    ENABLE_TEX_STREAM_MAP_IDX |
	    TEX_STREAM_MAP_IDX(1));
   OUT_RING(STATE3D_MAP_TEX_STREAM_CMD |
	    MAP_UNIT(2) |
	    DISABLE_TEX_STREAM_BUMP |
	    ENABLE_TEX_STREAM_COORD_SET |
	    TEX_STREAM_COORD_SET(2) |
	    ENABLE_TEX_STREAM_MAP_IDX |
	    TEX_STREAM_MAP_IDX(2));
   OUT_RING(STATE3D_MAP_TEX_STREAM_CMD |
	    MAP_UNIT(3) |
	    DISABLE_TEX_STREAM_BUMP |
	    ENABLE_TEX_STREAM_COORD_SET |
	    TEX_STREAM_COORD_SET(3) |
	    ENABLE_TEX_STREAM_MAP_IDX |
	    TEX_STREAM_MAP_IDX(3));

   OUT_RING(STATE3D_MAP_FILTER_CMD |
	    MAP_UNIT(0) |
	    ENABLE_CHROMA_KEY_PARAMS |
	    ENABLE_MIP_MODE_FILTER |
	    MIPFILTER_NEAREST |
	    ENABLE_MAG_MODE_FILTER |
	    ENABLE_MIN_MODE_FILTER |
	    MAG_FILTER(FILTER_NEAREST) |
	    MIN_FILTER(FILTER_NEAREST));
   OUT_RING(STATE3D_MAP_FILTER_CMD |
	    MAP_UNIT(1) |
	    ENABLE_CHROMA_KEY_PARAMS |
	    ENABLE_MIP_MODE_FILTER |
	    MIPFILTER_NEAREST |
	    ENABLE_MAG_MODE_FILTER |
	    ENABLE_MIN_MODE_FILTER |
	    MAG_FILTER(FILTER_NEAREST) |
	    MIN_FILTER(FILTER_NEAREST));
   OUT_RING(STATE3D_MAP_FILTER_CMD |
	    MAP_UNIT(2) |
	    ENABLE_CHROMA_KEY_PARAMS |
	    ENABLE_MIP_MODE_FILTER |
	    MIPFILTER_NEAREST |
	    ENABLE_MAG_MODE_FILTER |
	    ENABLE_MIN_MODE_FILTER |
	    MAG_FILTER(FILTER_NEAREST) |
	    MIN_FILTER(FILTER_NEAREST));
   OUT_RING(STATE3D_MAP_FILTER_CMD |
	    MAP_UNIT(3) |
	    ENABLE_CHROMA_KEY_PARAMS |
	    ENABLE_MIP_MODE_FILTER |
	    MIPFILTER_NEAREST |
	    ENABLE_MAG_MODE_FILTER |
	    ENABLE_MIN_MODE_FILTER |
	    MAG_FILTER(FILTER_NEAREST) |
	    MIN_FILTER(FILTER_NEAREST));

   OUT_RING(STATE3D_MAP_INFO_COLR_CMD);
   OUT_RING(MAP_INFO_TEX(0) |
	    MAPSURF_32BIT |
	    MT_32BIT_ARGB8888 |
	    MAP_INFO_OUTMUX_F0F1F2F3 |
	    MAP_INFO_VERTLINESTRIDEOFS_0 |
	    MAP_INFO_FORMAT_2D |
	    MAP_INFO_USE_FENCE);
   OUT_RING(MAP_INFO_HEIGHT(0) |
	    MAP_INFO_WIDTH(0));
   OUT_RING(MAP_INFO_BASEADDR(pI810->TexMem.Start));
   OUT_RING(MAP_INFO_DWORD_PITCH(31));
   OUT_RING(MAP_INFO_DFLT_COLR(0));

   OUT_RING(STATE3D_MAP_INFO_COLR_CMD);
   OUT_RING(MAP_INFO_TEX(1) |
	    MAPSURF_32BIT |
	    MT_32BIT_ARGB8888 |
	    MAP_INFO_OUTMUX_F0F1F2F3 |
	    MAP_INFO_VERTLINESTRIDEOFS_0 |
	    MAP_INFO_FORMAT_2D |
	    MAP_INFO_USE_FENCE);
   OUT_RING(MAP_INFO_HEIGHT(0) |
	    MAP_INFO_WIDTH(0));
   OUT_RING(MAP_INFO_BASEADDR(pI810->TexMem.Start));
   OUT_RING(MAP_INFO_DWORD_PITCH(31));
   OUT_RING(MAP_INFO_DFLT_COLR(0));

   OUT_RING(STATE3D_MAP_INFO_COLR_CMD);
   OUT_RING(MAP_INFO_TEX(2) |
	    MAPSURF_32BIT |
	    MT_32BIT_ARGB8888 |
	    MAP_INFO_OUTMUX_F0F1F2F3 |
	    MAP_INFO_VERTLINESTRIDEOFS_0 |
	    MAP_INFO_FORMAT_2D |
	    MAP_INFO_USE_FENCE);
   OUT_RING(MAP_INFO_HEIGHT(0) |
	    MAP_INFO_WIDTH(0));
   OUT_RING(MAP_INFO_BASEADDR(pI810->TexMem.Start));
   OUT_RING(MAP_INFO_DWORD_PITCH(31));
   OUT_RING(MAP_INFO_DFLT_COLR(0));

   OUT_RING(STATE3D_MAP_INFO_COLR_CMD);
   OUT_RING(MAP_INFO_TEX(3) |
	    MAPSURF_32BIT |
	    MT_32BIT_ARGB8888 |
	    MAP_INFO_OUTMUX_F0F1F2F3 |
	    MAP_INFO_VERTLINESTRIDEOFS_0 |
	    MAP_INFO_FORMAT_2D |
	    MAP_INFO_USE_FENCE);
   OUT_RING(MAP_INFO_HEIGHT(0) |
	    MAP_INFO_WIDTH(0));
   OUT_RING(MAP_INFO_BASEADDR(pI810->TexMem.Start));
   OUT_RING(MAP_INFO_DWORD_PITCH(31));
   OUT_RING(MAP_INFO_DFLT_COLR(0));

   OUT_RING(STATE3D_MAP_LOD_CNTL_CMD |
	    MAP_UNIT(0) |
	    ENABLE_TEXLOD_BIAS |
	    MAP_LOD_BIAS(0));
   OUT_RING(STATE3D_MAP_LOD_CNTL_CMD |
	    MAP_UNIT(1) |
	    ENABLE_TEXLOD_BIAS |
	    MAP_LOD_BIAS(0));
   OUT_RING(STATE3D_MAP_LOD_CNTL_CMD |
	    MAP_UNIT(2) |
	    ENABLE_TEXLOD_BIAS |
	    MAP_LOD_BIAS(0));
   OUT_RING(STATE3D_MAP_LOD_CNTL_CMD |
	    MAP_UNIT(3) |
	    ENABLE_TEXLOD_BIAS |
	    MAP_LOD_BIAS(0));

   OUT_RING(STATE3D_MAP_LOD_LIMITS_CMD |
	    MAP_UNIT(0) |
	    ENABLE_MAX_MIP_LVL |
	    ENABLE_MIN_MIP_LVL |
	    LOD_MAX(0) |
	    LOD_MIN(0));
   OUT_RING(STATE3D_MAP_LOD_LIMITS_CMD |
	    MAP_UNIT(1) |
	    ENABLE_MAX_MIP_LVL |
	    ENABLE_MIN_MIP_LVL |
	    LOD_MAX(0) |
	    LOD_MIN(0));
   OUT_RING(STATE3D_MAP_LOD_LIMITS_CMD |
	    MAP_UNIT(2) |
	    ENABLE_MAX_MIP_LVL |
	    ENABLE_MIN_MIP_LVL |
	    LOD_MAX(0) |
	    LOD_MIN(0));
   OUT_RING(STATE3D_MAP_LOD_LIMITS_CMD |
	    MAP_UNIT(3) |
	    ENABLE_MAX_MIP_LVL |
	    ENABLE_MIN_MIP_LVL |
	    LOD_MAX(0) |
	    LOD_MIN(0));

   OUT_RING(STATE3D_MAP_COORD_TRANSFORM);
   OUT_RING(DISABLE_TEX_TRANSFORM | TEXTURE_SET(0));
   OUT_RING(STATE3D_MAP_COORD_TRANSFORM);
   OUT_RING(DISABLE_TEX_TRANSFORM | TEXTURE_SET(1));
   OUT_RING(STATE3D_MAP_COORD_TRANSFORM);
   OUT_RING(DISABLE_TEX_TRANSFORM | TEXTURE_SET(2));
   OUT_RING(STATE3D_MAP_COORD_TRANSFORM);
   OUT_RING(DISABLE_TEX_TRANSFORM | TEXTURE_SET(3));

   /* End texture state */

   OUT_RING(STATE3D_MODES_1_CMD |
	    ENABLE_COLR_BLND_FUNC |
	    BLENDFUNC_ADD |
	    ENABLE_SRC_BLND_FACTOR |
	    ENABLE_DST_BLND_FACTOR |
	    SRC_BLND_FACT(BLENDFACT_ONE) |
	    DST_BLND_FACT(BLENDFACT_ZERO));

   OUT_RING(STATE3D_MODES_2_CMD |
	    ENABLE_GLOBAL_DEPTH_BIAS |
	    GLOBAL_DEPTH_BIAS(0) |
	    ENABLE_ALPHA_TEST_FUNC |
	    ALPHA_TEST_FUNC(COMPAREFUNC_ALWAYS) |
	    ALPHA_REF_VALUE(0));

   OUT_RING(STATE3D_MODES_3_CMD |
	    ENABLE_DEPTH_TEST_FUNC |
	    DEPTH_TEST_FUNC(COMPAREFUNC_LESS) |
	    ENABLE_ALPHA_SHADE_MODE |
	    ALPHA_SHADE_MODE(SHADE_MODE_LINEAR) |
	    ENABLE_FOG_SHADE_MODE |
	    FOG_SHADE_MODE(SHADE_MODE_LINEAR) |
	    ENABLE_SPEC_SHADE_MODE |
	    SPEC_SHADE_MODE(SHADE_MODE_LINEAR) |
	    ENABLE_COLOR_SHADE_MODE |
	    COLOR_SHADE_MODE(SHADE_MODE_LINEAR) |
	    ENABLE_CULL_MODE |
	    CULLMODE_NONE);

   OUT_RING(STATE3D_MODES_4_CMD |
	    ENABLE_LOGIC_OP_FUNC |
	    LOGIC_OP_FUNC(LOGICOP_COPY) |
	    ENABLE_STENCIL_TEST_MASK |
	    STENCIL_TEST_MASK(0xff) |
	    ENABLE_STENCIL_WRITE_MASK |
	    STENCIL_WRITE_MASK(0xff));

   OUT_RING(STATE3D_MODES_5_CMD |
	    ENABLE_SPRITE_POINT_TEX |
	    SPRITE_POINT_TEX_OFF |
	    FLUSH_RENDER_CACHE |
	    FLUSH_TEXTURE_CACHE |
	    ENABLE_FIXED_LINE_WIDTH |
	    FIXED_LINE_WIDTH(0x2) |
	    ENABLE_FIXED_POINT_WIDTH |
	    FIXED_POINT_WIDTH(1));

   OUT_RING(STATE3D_RASTER_RULES_CMD |
	    ENABLE_POINT_RASTER_RULE |
	    OGL_POINT_RASTER_RULE |
	    ENABLE_LINE_STRIP_PROVOKE_VRTX |
	    ENABLE_TRI_FAN_PROVOKE_VRTX |
	    ENABLE_TRI_STRIP_PROVOKE_VRTX |
	    LINE_STRIP_PROVOKE_VRTX(1) |
	    TRI_FAN_PROVOKE_VRTX(2) |
	    TRI_STRIP_PROVOKE_VRTX(2));

   OUT_RING(STATE3D_SCISSOR_ENABLE_CMD |
	    DISABLE_SCISSOR_RECT);

   OUT_RING(STATE3D_SCISSOR_RECT_0_CMD);
   OUT_RING(0);
   OUT_RING(0);

   OUT_RING(STATE3D_STENCIL_TEST_CMD |
	    ENABLE_STENCIL_PARMS |
	    STENCIL_FAIL_OP(STENCILOP_KEEP) |
	    STENCIL_PASS_DEPTH_FAIL_OP(STENCILOP_KEEP) |
	    STENCIL_PASS_DEPTH_PASS_OP(STENCILOP_KEEP) |
	    ENABLE_STENCIL_TEST_FUNC |
	    STENCIL_TEST_FUNC(COMPAREFUNC_ALWAYS) |
	    ENABLE_STENCIL_REF_VALUE |
	    STENCIL_REF_VALUE(0));

   OUT_RING(VRTX_FORMAT_NTEX(1));

   OUT_RING(STATE3D_VERTEX_FORMAT_2_CMD |
	    VRTX_TEX_SET_0_FMT(TEXCOORDFMT_2D) |
	    VRTX_TEX_SET_1_FMT(TEXCOORDFMT_2D) |
	    VRTX_TEX_SET_2_FMT(TEXCOORDFMT_2D) |
	    VRTX_TEX_SET_3_FMT(TEXCOORDFMT_2D) |
	    VRTX_TEX_SET_4_FMT(TEXCOORDFMT_2D) |
	    VRTX_TEX_SET_5_FMT(TEXCOORDFMT_2D) |
	    VRTX_TEX_SET_6_FMT(TEXCOORDFMT_2D) |
	    VRTX_TEX_SET_7_FMT(TEXCOORDFMT_2D));

   OUT_RING(STATE3D_VERTEX_TRANSFORM);
   OUT_RING(DISABLE_VIEWPORT_TRANSFORM |
	    DISABLE_PERSPECTIVE_DIVIDE);

   OUT_RING(STATE3D_W_STATE_CMD);
   OUT_RING(MAGIC_W_STATE_DWORD1);
   OUT_RING(0x3f800000 /* 1.0 in IEEE float */);

   ADVANCE_LP_RING();
}
