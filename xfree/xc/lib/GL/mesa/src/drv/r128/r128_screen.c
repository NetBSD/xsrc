/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_screen.c,v 1.3 2000/12/04 19:21:46 dawes Exp $ */
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
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#include "r128_dri.h"

#include "r128_context.h"
#include "r128_ioctl.h"
#include "r128_tris.h"
#include "r128_vb.h"
#include "r128_pipeline.h"

#include <sys/mman.h>

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


/* Create the device specific screen private data struct */
r128ScreenPtr r128CreateScreen(__DRIscreenPrivate *sPriv)
{
    r128ScreenPtr    r128Screen;
    R128DRIPtr       r128DRIPriv = (R128DRIPtr)sPriv->pDevPriv;
    int cpp;

    /* Allocate the private area */
    r128Screen = (r128ScreenPtr)Xmalloc(sizeof(*r128Screen));
    if (!r128Screen) return NULL;

    /* This is first since which regions we map depends on whether or
       not we are using a PCI card */
    r128Screen->IsPCI          = r128DRIPriv->IsPCI;

    r128Screen->mmioRgn.handle = r128DRIPriv->registerHandle;
    r128Screen->mmioRgn.size   = r128DRIPriv->registerSize;
    if (drmMap(sPriv->fd,
	       r128Screen->mmioRgn.handle,
	       r128Screen->mmioRgn.size,
	       (drmAddressPtr)&r128Screen->mmio)) {
	Xfree(r128Screen);
	return NULL;
    }

    if (!r128Screen->IsPCI) {
	r128Screen->ringRgn.handle = r128DRIPriv->ringHandle;
	r128Screen->ringRgn.size   = r128DRIPriv->ringMapSize;
	if (drmMap(sPriv->fd,
		   r128Screen->ringRgn.handle,
		   r128Screen->ringRgn.size,
		   (drmAddressPtr)&r128Screen->ring)) {
	    drmUnmap((drmAddress)r128Screen->mmio, r128Screen->mmioRgn.size);
	    Xfree(r128Screen);
	    return NULL;
	}

	r128Screen->ringReadRgn.handle = r128DRIPriv->ringReadPtrHandle;
	r128Screen->ringReadRgn.size   = r128DRIPriv->ringReadMapSize;
	if (drmMap(sPriv->fd,
		   r128Screen->ringReadRgn.handle,
		   r128Screen->ringReadRgn.size,
		   (drmAddressPtr)&r128Screen->ringReadPtr)) {
	    drmUnmap((drmAddress)r128Screen->ring, r128Screen->ringRgn.size);
	    drmUnmap((drmAddress)r128Screen->mmio, r128Screen->mmioRgn.size);
	    Xfree(r128Screen);
	    return NULL;
	}

	r128Screen->bufRgn.handle = r128DRIPriv->bufHandle;
	r128Screen->bufRgn.size   = r128DRIPriv->bufMapSize;
	if (drmMap(sPriv->fd,
		   r128Screen->bufRgn.handle,
		   r128Screen->bufRgn.size,
		   (drmAddressPtr)&r128Screen->buf)) {
	    drmUnmap((drmAddress)r128Screen->ringReadPtr,
		     r128Screen->ringReadRgn.size);
	    drmUnmap((drmAddress)r128Screen->ring, r128Screen->ringRgn.size);
	    drmUnmap((drmAddress)r128Screen->mmio, r128Screen->mmioRgn.size);
	    Xfree(r128Screen);
	    return NULL;
	}
	r128Screen->bufOffset     = r128DRIPriv->bufOffset;

	r128Screen->agpTexRgn.handle = r128DRIPriv->agpTexHandle;
	r128Screen->agpTexRgn.size   = r128DRIPriv->agpTexMapSize;
	if (drmMap(sPriv->fd,
		   r128Screen->agpTexRgn.handle,
		   r128Screen->agpTexRgn.size,
		   (drmAddressPtr)&r128Screen->agpTex)) {
	    drmUnmap((drmAddress)r128Screen->buf,   r128Screen->bufRgn.size);
	    drmUnmap((drmAddress)r128Screen->ringReadPtr,
		     r128Screen->ringReadRgn.size);
	    drmUnmap((drmAddress)r128Screen->ring, r128Screen->ringRgn.size);
	    drmUnmap((drmAddress)r128Screen->mmio, r128Screen->mmioRgn.size);
	    Xfree(r128Screen);
	    return NULL;
	}
	r128Screen->agpTexOffset   = r128DRIPriv->agpTexOffset;

	if (!(r128Screen->buffers = drmMapBufs(sPriv->fd))) {
	    drmUnmap((drmAddress)r128Screen->agpTex,
		     r128Screen->agpTexRgn.size);
	    drmUnmap((drmAddress)r128Screen->buf,   r128Screen->bufRgn.size);
	    drmUnmap((drmAddress)r128Screen->ringReadPtr,
		     r128Screen->ringReadRgn.size);
	    drmUnmap((drmAddress)r128Screen->ring, r128Screen->ringRgn.size);
	    drmUnmap((drmAddress)r128Screen->mmio, r128Screen->mmioRgn.size);
	    Xfree(r128Screen);
	    return NULL;
	}
    }

    /* Allow both AGP and PCI cards to use vertex buffers.  PCI cards use
     * the ring walker method, ie. the vertex buffer data is actually part
     * of the command stream.
     */
    r128Screen->bufMapSize        = r128DRIPriv->bufMapSize;

    r128Screen->deviceID         = r128DRIPriv->deviceID;

    r128Screen->depth            = r128DRIPriv->depth;
    r128Screen->bpp              = r128DRIPriv->bpp;
    r128Screen->pixel_code       = (r128Screen->bpp != 16 ?
				    r128Screen->bpp :
				    r128Screen->depth);

    cpp                          = r128Screen->bpp / 8;

    r128Screen->fb               = sPriv->pFB;
    r128Screen->fbOffset         = sPriv->fbOrigin;
    r128Screen->fbStride         = sPriv->fbStride;
    r128Screen->fbSize           = sPriv->fbSize;

    r128Screen->frontOffset      = r128DRIPriv->frontOffset;
    r128Screen->frontPitch       = r128DRIPriv->frontPitch;
    r128Screen->backOffset       = r128DRIPriv->backOffset;
    r128Screen->backPitch        = r128DRIPriv->backPitch;
    r128Screen->depthOffset      = r128DRIPriv->depthOffset;
    r128Screen->depthPitch       = r128DRIPriv->depthPitch;
    r128Screen->spanOffset       = r128DRIPriv->spanOffset;

    r128Screen->texOffset[R128_LOCAL_TEX_HEAP]   = r128DRIPriv->textureOffset;
    r128Screen->texSize[R128_LOCAL_TEX_HEAP]     = r128DRIPriv->textureSize;
    r128Screen->log2TexGran[R128_LOCAL_TEX_HEAP] = r128DRIPriv->log2TexGran;

    if (r128Screen->IsPCI) {
	r128Screen->texOffset[R128_AGP_TEX_HEAP]   = 0;
	r128Screen->texSize[R128_AGP_TEX_HEAP]     = 0;
	r128Screen->log2TexGran[R128_AGP_TEX_HEAP] = 0;
	r128Screen->NRTexHeaps                     = R128_NR_TEX_HEAPS-1;
    } else {
	r128Screen->texOffset[R128_AGP_TEX_HEAP]   = 0;
	r128Screen->texSize[R128_AGP_TEX_HEAP]     =
	    r128DRIPriv->agpTexMapSize;
	r128Screen->log2TexGran[R128_AGP_TEX_HEAP] =
	    r128DRIPriv->log2AGPTexGran;
	r128Screen->NRTexHeaps                     = R128_NR_TEX_HEAPS;
    }

    r128Screen->AGPMode          = r128DRIPriv->AGPMode;

    r128Screen->CCEMode          = r128DRIPriv->CCEMode;
    r128Screen->CCEFifoSize      = r128DRIPriv->CCEFifoSize;

    r128Screen->ringEntries      = r128DRIPriv->ringSize/sizeof(CARD32);
    if (!r128Screen->IsPCI) {
	r128Screen->ringStartPtr = (int *)r128Screen->ring;
	r128Screen->ringEndPtr   = (int *)(r128Screen->ring
					   + r128DRIPriv->ringSize);
    }

    r128Screen->MMIOFifoSlots    = 0;
    r128Screen->CCEFifoSlots     = 0;

    r128Screen->CCEFifoAddr      = R128_PM4_FIFO_DATA_EVEN;

    r128Screen->driScreen        = sPriv;

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

    r128DDFastPathInit();
    r128DDEltPathInit();
    r128DDTriangleFuncsInit();
    r128DDSetupInit();

    return r128Screen;
}

/* Destroy the device specific screen private data struct */
void r128DestroyScreen(__DRIscreenPrivate *sPriv)
{
    r128ScreenPtr r128Screen = (r128ScreenPtr)sPriv->private;

    if (!r128Screen->IsPCI) {
	drmUnmapBufs(r128Screen->buffers);

	drmUnmap((drmAddress)r128Screen->agpTex, r128Screen->agpTexRgn.size);
	drmUnmap((drmAddress)r128Screen->buf,    r128Screen->bufRgn.size);
	drmUnmap((drmAddress)r128Screen->ringReadPtr,
		 r128Screen->ringReadRgn.size);
	drmUnmap((drmAddress)r128Screen->ring,   r128Screen->ringRgn.size);
    }
    drmUnmap((drmAddress)r128Screen->mmio,   r128Screen->mmioRgn.size);

    Xfree(r128Screen);
    sPriv->private = NULL;
}
