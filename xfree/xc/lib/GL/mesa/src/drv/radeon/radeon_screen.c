/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_screen.c,v 1.3 2001/05/02 15:06:04 dawes Exp $ */
/**************************************************************************

Copyright 2000, 2001 ATI Technologies Inc., Ontario, Canada, and
                     VA Linux Systems Inc., Fremont, California.

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

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#include "radeon_dri.h"

#include "radeon_context.h"
#include "radeon_ioctl.h"
#include "radeon_tris.h"
#include "radeon_vb.h"
#include "radeon_pipeline.h"

#include "mem.h"

#if 1
/* Including xf86PciInfo.h introduces a bunch of errors...
 */
#define PCI_CHIP_RADEON_QD	0x5144
#define PCI_CHIP_RADEON_QE	0x5145
#define PCI_CHIP_RADEON_QF	0x5146
#define PCI_CHIP_RADEON_QG	0x5147
#define PCI_CHIP_RADEON_VE     0x5159
#endif


/* Create the device specific screen private data struct.
 */
radeonScreenPtr radeonCreateScreen( __DRIscreenPrivate *sPriv )
{
   radeonScreenPtr radeonScreen;
   RADEONDRIPtr radeonDRIPriv = (RADEONDRIPtr)sPriv->pDevPriv;

   /* Allocate the private area */
   radeonScreen = (radeonScreenPtr) CALLOC( sizeof(*radeonScreen) );
   if ( !radeonScreen ) return NULL;

   /* This is first since which regions we map depends on whether or
    * not we are using a PCI card.
    */
   radeonScreen->IsPCI = radeonDRIPriv->IsPCI;

   radeonScreen->mmio.handle = radeonDRIPriv->registerHandle;
   radeonScreen->mmio.size   = radeonDRIPriv->registerSize;
   if ( drmMap( sPriv->fd,
		radeonScreen->mmio.handle,
		radeonScreen->mmio.size,
		&radeonScreen->mmio.map ) ) {
      FREE( radeonScreen );
      return NULL;
   }

   radeonScreen->status.handle = radeonDRIPriv->statusHandle;
   radeonScreen->status.size   = radeonDRIPriv->statusSize;
   if ( drmMap( sPriv->fd,
		radeonScreen->status.handle,
		radeonScreen->status.size,
		&radeonScreen->status.map ) ) {
      drmUnmap( radeonScreen->mmio.map, radeonScreen->mmio.size );
      FREE( radeonScreen );
      return NULL;
   }
   radeonScreen->scratch = (__volatile__ CARD32 *)
      ((GLubyte *)radeonScreen->status.map + RADEON_SCRATCH_REG_OFFSET);

   radeonScreen->buffers = drmMapBufs( sPriv->fd );
   if ( !radeonScreen->buffers ) {
      drmUnmap( radeonScreen->status.map, radeonScreen->status.size );
      drmUnmap( radeonScreen->mmio.map, radeonScreen->mmio.size );
      FREE( radeonScreen );
      return NULL;
   }

   if ( !radeonScreen->IsPCI ) {
      radeonScreen->agpTextures.handle = radeonDRIPriv->agpTexHandle;
      radeonScreen->agpTextures.size   = radeonDRIPriv->agpTexMapSize;
      if ( drmMap( sPriv->fd,
		   radeonScreen->agpTextures.handle,
		   radeonScreen->agpTextures.size,
		   (drmAddressPtr)&radeonScreen->agpTextures.map ) ) {
	 drmUnmapBufs( radeonScreen->buffers );
	 drmUnmap( radeonScreen->status.map, radeonScreen->status.size );
	 drmUnmap( radeonScreen->mmio.map, radeonScreen->mmio.size );
	 FREE( radeonScreen );
	 return NULL;
      }
   }


   switch ( radeonDRIPriv->deviceID ) {
   case PCI_CHIP_RADEON_QD:
   case PCI_CHIP_RADEON_QE:
   case PCI_CHIP_RADEON_QF:
   case PCI_CHIP_RADEON_QG:
   case PCI_CHIP_RADEON_VE:
      radeonScreen->chipset = RADEON_CARD_TYPE_RADEON;
      break;
   default:
      radeonScreen->chipset = RADEON_CARD_TYPE_RADEON;
      break;
   }

   radeonScreen->cpp = radeonDRIPriv->bpp / 8;
   radeonScreen->AGPMode = radeonDRIPriv->AGPMode;

   radeonScreen->frontOffset	= radeonDRIPriv->frontOffset;
   radeonScreen->frontPitch	= radeonDRIPriv->frontPitch;
   radeonScreen->backOffset	= radeonDRIPriv->backOffset;
   radeonScreen->backPitch	= radeonDRIPriv->backPitch;
   radeonScreen->depthOffset	= radeonDRIPriv->depthOffset;
   radeonScreen->depthPitch	= radeonDRIPriv->depthPitch;

   radeonScreen->texOffset[RADEON_CARD_HEAP] = radeonDRIPriv->textureOffset;
   radeonScreen->texSize[RADEON_CARD_HEAP] = radeonDRIPriv->textureSize;
   radeonScreen->logTexGranularity[RADEON_CARD_HEAP] =
      radeonDRIPriv->log2TexGran;

   if ( radeonScreen->IsPCI ) {
      radeonScreen->numTexHeaps = RADEON_NR_TEX_HEAPS - 1;
      radeonScreen->texOffset[RADEON_AGP_HEAP] = 0;
      radeonScreen->texSize[RADEON_AGP_HEAP] = 0;
      radeonScreen->logTexGranularity[RADEON_AGP_HEAP] = 0;
   } else {
      radeonScreen->numTexHeaps = RADEON_NR_TEX_HEAPS;
      radeonScreen->texOffset[RADEON_AGP_HEAP] =
	 radeonDRIPriv->agpTexOffset + RADEON_AGP_TEX_OFFSET;
      radeonScreen->texSize[RADEON_AGP_HEAP] = radeonDRIPriv->agpTexMapSize;
      radeonScreen->logTexGranularity[RADEON_AGP_HEAP] =
	 radeonDRIPriv->log2AGPTexGran;
   }

   radeonScreen->driScreen = sPriv;
   radeonScreen->sarea_priv_offset = radeonDRIPriv->sarea_priv_offset;

#ifdef PER_CONTEXT_SAREA
   radeonScreen->perctx_sarea_size = radeonDRIPriv->perctx_sarea_size;
#endif

   radeonDDSetupInit();
   radeonDDTriangleFuncsInit();
   radeonDDFastPathInit();
   radeonDDEltPathInit();

   return radeonScreen;
}

/* Destroy the device specific screen private data struct.
 */
void radeonDestroyScreen( __DRIscreenPrivate *sPriv )
{
   radeonScreenPtr radeonScreen = (radeonScreenPtr)sPriv->private;

   if ( !radeonScreen->IsPCI ) {
      drmUnmap( radeonScreen->agpTextures.map,
		radeonScreen->agpTextures.size );
   }
   drmUnmapBufs( radeonScreen->buffers );
   drmUnmap( radeonScreen->status.map, radeonScreen->status.size );
   drmUnmap( radeonScreen->mmio.map, radeonScreen->mmio.size );

   FREE( radeonScreen );
   sPriv->private = NULL;
}
