/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_screen.h,v 1.2 2001/03/21 16:14:25 dawes Exp $ */
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

#ifndef __RADEON_SCREEN_H__
#define __RADEON_SCREEN_H__

#ifdef GLX_DIRECT_RENDERING

#include "radeon_sarea.h"

typedef struct {
   drmHandle handle;			/* Handle to the DRM region */
   drmSize size;			/* Size of the DRM region */
   drmAddress map;			/* Mapping of the DRM region */
} radeonRegionRec, *radeonRegionPtr;

typedef struct {

   GLint chipset;
   GLint cpp;
   GLint IsPCI;				/* Current card is a PCI card */
   GLint AGPMode;

   GLuint frontOffset;
   GLuint frontPitch;
   GLuint backOffset;
   GLuint backPitch;

   GLuint depthOffset;
   GLuint depthPitch;

    /* Shared texture data */
   int numTexHeaps;
   int texOffset[RADEON_NR_TEX_HEAPS];
   int texSize[RADEON_NR_TEX_HEAPS];
   int logTexGranularity[RADEON_NR_TEX_HEAPS];

#ifdef PER_CONTEXT_SAREA
   drmSize private_sarea_size;
#endif

   radeonRegionRec mmio;
   radeonRegionRec status;
   radeonRegionRec agpTextures;

   drmBufMapPtr buffers;

   __volatile__ CARD32 *scratch;

   __DRIscreenPrivate *driScreen;
   unsigned int sarea_priv_offset;
} radeonScreenRec, *radeonScreenPtr;

extern radeonScreenPtr radeonCreateScreen( __DRIscreenPrivate *sPriv );
extern void radeonDestroyScreen( __DRIscreenPrivate *sPriv );

#endif
#endif /* __RADEON_SCREEN_H__ */
