/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_screen.h,v 1.3 2000/12/04 19:21:47 dawes Exp $ */
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

#ifndef _R128_SCREEN_H_
#define _R128_SCREEN_H_

#ifdef GLX_DIRECT_RENDERING

#include "r128_sarea.h"

typedef struct {
    drmHandle  handle;			/* Handle to the DRM region */
    drmSize    size;			/* Size of the DRM region */
} r128RegionRec, *r128RegionPtr;

typedef struct {
    /* MMIO register data */
    r128RegionRec       mmioRgn;
    unsigned char      *mmio;

    /* CCE ring buffer data */
    r128RegionRec       ringRgn;
    unsigned char      *ring;

    /* CCE ring read pointer data */
    r128RegionRec       ringReadRgn;

    /* CCE vertex/indirect buffer data */
    r128RegionRec       bufRgn;
    unsigned char      *buf;
    int                 bufOffset;
    int                 bufMapSize;
    drmBufMapPtr        buffers;

    /* CCE AGP Texture data */
    r128RegionRec       agpTexRgn;
    unsigned char      *agpTex;
    int                 agpTexOffset;

    /* Frame buffer data */
    unsigned char      *fb;
    unsigned long       fbOffset;
    int                 fbStride;
    int                 fbSize;

    unsigned int	frontX, frontY;	/* Start of front buffer */
    unsigned int	frontOffset, frontPitch;
    unsigned int	backX, backY;	/* Start of shared back buffer */
    unsigned int	backOffset, backPitch;
    unsigned int	depthX, depthY;	/* Start of shared depth buffer */
    unsigned int	depthOffset, depthPitch;
    unsigned int	spanOffset;

    int			chipset;
    int                 IsPCI;		/* Current card is a PCI card */
    int                 AGPMode;

    int                 CCEMode;	/* CCE mode that server/clients use */
    int                 CCEFifoSize;	/* Size of the CCE command FIFO */

    /* CCE ring buffer data */
    int                 ringEntries;

    volatile int       *ringReadPtr;	/* Pointer to current read addr */
    int                *ringStartPtr;   /* Pointer to end of ring buffer */
    int                *ringEndPtr;     /* Pointer to end of ring buffer */

    /* DRI screen private data */
    int                 deviceID;	/* PCI device ID */
    int                 depth;		/* Depth of display (8, 15, 16, 24) */
    int                 bpp;		/* Bit depth of disp (8, 16, 24, 32) */
    int                 pixel_code;	/* 8, 15, 16, 24, 32 */

    /* Shared texture data */
    int                 NRTexHeaps;
    int                 texOffset[R128_NR_TEX_HEAPS];
    int                 texSize[R128_NR_TEX_HEAPS];
    int                 log2TexGran[R128_NR_TEX_HEAPS];

    int                 MMIOFifoSlots;	/* Free slots in the FIFO (64 max) */
    int                 CCEFifoSlots;	/* Free slots in the CCE FIFO */

    int                 CCEFifoAddr;    /* MMIO offset to write next CCE
					   value (only used when CCE is
					   in PIO mode). */
    __DRIscreenPrivate *driScreen;
} r128ScreenRec, *r128ScreenPtr;

r128ScreenPtr r128CreateScreen(__DRIscreenPrivate *sPriv);
void          r128DestroyScreen(__DRIscreenPrivate *sPriv);

#endif
#endif /* _R128_SCREEN_H_ */
