/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_screen.c,v 1.5 2001/03/21 16:14:23 dawes Exp $ */
/**************************************************************************

Copyright 1999, 2000 ATI Technologies Inc. and Precision Insight, Inc.,
                                               Cedar Park, Texas.
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
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Gareth Hughes <gareth@valinux.com>
 *   Kevin E. Martin <martin@valinux.com>
 *
 */

#include "r128_dri.h"

#include "r128_context.h"
#include "r128_ioctl.h"
#include "r128_tris.h"
#include "r128_vb.h"
#include "r128_pipeline.h"

#include "mem.h"

#if 1
/* Including xf86PciInfo.h introduces a bunch of errors...
 */
#define PCI_CHIP_RAGE128LE	0x4C45
#define PCI_CHIP_RAGE128LF	0x4C46
#define PCI_CHIP_RAGE128PF	0x5046
#define PCI_CHIP_RAGE128PR	0x5052
#define PCI_CHIP_RAGE128RE	0x5245
#define PCI_CHIP_RAGE128RF	0x5246
#define PCI_CHIP_RAGE128RK	0x524B
#define PCI_CHIP_RAGE128RL	0x524C
#endif


/* Create the device specific screen private data struct.
 */
r128ScreenPtr r128CreateScreen( __DRIscreenPrivate *sPriv )
{
   r128ScreenPtr r128Screen;
   R128DRIPtr r128DRIPriv = (R128DRIPtr)sPriv->pDevPriv;

   /* Allocate the private area */
   r128Screen = (r128ScreenPtr) CALLOC( sizeof(*r128Screen) );
   if ( !r128Screen ) return NULL;

   /* This is first since which regions we map depends on whether or
    * not we are using a PCI card.
    */
   r128Screen->IsPCI = r128DRIPriv->IsPCI;
   r128Screen->sarea_priv_offset = r128DRIPriv->sarea_priv_offset;

   r128Screen->mmio.handle = r128DRIPriv->registerHandle;
   r128Screen->mmio.size   = r128DRIPriv->registerSize;
   if ( drmMap( sPriv->fd,
		r128Screen->mmio.handle,
		r128Screen->mmio.size,
		(drmAddressPtr)&r128Screen->mmio.map ) ) {
      FREE( r128Screen );
      return NULL;
   }

   r128Screen->buffers = drmMapBufs( sPriv->fd );
   if ( !r128Screen->buffers ) {
      drmUnmap( (drmAddress)r128Screen->mmio.map, r128Screen->mmio.size );
      FREE( r128Screen );
      return NULL;
   }

   if ( !r128Screen->IsPCI ) {
      r128Screen->agpTextures.handle = r128DRIPriv->agpTexHandle;
      r128Screen->agpTextures.size   = r128DRIPriv->agpTexMapSize;
      if ( drmMap( sPriv->fd,
		   r128Screen->agpTextures.handle,
		   r128Screen->agpTextures.size,
		   (drmAddressPtr)&r128Screen->agpTextures.map ) ) {
	 drmUnmapBufs( r128Screen->buffers );
	 drmUnmap( (drmAddress)r128Screen->mmio.map, r128Screen->mmio.size );
	 FREE( r128Screen );
	 return NULL;
      }
   }

   switch ( r128DRIPriv->deviceID ) {
   case PCI_CHIP_RAGE128RE:
   case PCI_CHIP_RAGE128RF:
   case PCI_CHIP_RAGE128RK:
   case PCI_CHIP_RAGE128RL:
      r128Screen->chipset = R128_CARD_TYPE_R128;
      break;
   case PCI_CHIP_RAGE128PF:
      r128Screen->chipset = R128_CARD_TYPE_R128_PRO;
      break;
   case PCI_CHIP_RAGE128LE:
   case PCI_CHIP_RAGE128LF:
      r128Screen->chipset = R128_CARD_TYPE_R128_MOBILITY;
      break;
   default:
      r128Screen->chipset = R128_CARD_TYPE_R128;
      break;
   }

   r128Screen->cpp = r128DRIPriv->bpp / 8;
   r128Screen->AGPMode = r128DRIPriv->AGPMode;

   r128Screen->frontOffset	= r128DRIPriv->frontOffset;
   r128Screen->frontPitch	= r128DRIPriv->frontPitch;
   r128Screen->backOffset	= r128DRIPriv->backOffset;
   r128Screen->backPitch	= r128DRIPriv->backPitch;
   r128Screen->depthOffset	= r128DRIPriv->depthOffset;
   r128Screen->depthPitch	= r128DRIPriv->depthPitch;
   r128Screen->spanOffset	= r128DRIPriv->spanOffset;

   r128Screen->texOffset[R128_CARD_HEAP] = r128DRIPriv->textureOffset;
   r128Screen->texSize[R128_CARD_HEAP] = r128DRIPriv->textureSize;
   r128Screen->logTexGranularity[R128_CARD_HEAP] = r128DRIPriv->log2TexGran;

   if ( r128Screen->IsPCI ) {
      r128Screen->numTexHeaps = R128_NR_TEX_HEAPS - 1;
      r128Screen->texOffset[R128_AGP_HEAP] = 0;
      r128Screen->texSize[R128_AGP_HEAP] = 0;
      r128Screen->logTexGranularity[R128_AGP_HEAP] = 0;
   } else {
      r128Screen->numTexHeaps = R128_NR_TEX_HEAPS;
      r128Screen->texOffset[R128_AGP_HEAP] =
	 r128DRIPriv->agpTexOffset + R128_AGP_TEX_OFFSET;
      r128Screen->texSize[R128_AGP_HEAP] = r128DRIPriv->agpTexMapSize;
      r128Screen->logTexGranularity[R128_AGP_HEAP] =
	 r128DRIPriv->log2AGPTexGran;
   }

   r128Screen->driScreen = sPriv;

   r128DDSetupInit();
   r128DDTriangleFuncsInit();
   r128DDFastPathInit();
   r128DDEltPathInit();

   return r128Screen;
}

/* Destroy the device specific screen private data struct.
 */
void r128DestroyScreen( __DRIscreenPrivate *sPriv )
{
   r128ScreenPtr r128Screen = (r128ScreenPtr)sPriv->private;

   if ( !r128Screen->IsPCI ) {
      drmUnmap( (drmAddress)r128Screen->agpTextures.map,
		r128Screen->agpTextures.size );
   }
   drmUnmapBufs( r128Screen->buffers );
   drmUnmap( (drmAddress)r128Screen->mmio.map, r128Screen->mmio.size );

   FREE( r128Screen );
   sPriv->private = NULL;
}
