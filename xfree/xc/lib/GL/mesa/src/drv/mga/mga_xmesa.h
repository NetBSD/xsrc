/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mga_xmesa.h,v 1.6 2000/09/24 13:51:05 alanh Exp $ */

/*
 * Authors:
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 */

#ifndef _MGA_INIT_H_
#define _MGA_INIT_H_

#ifdef GLX_DIRECT_RENDERING

#include <sys/time.h>
#include "dri_tmm.h"
#include "dri_mesaint.h"
#include "dri_mesa.h"
#include "types.h"
#include "mgaregs.h"

typedef struct {
   drmHandle handle;
   drmSize size;
   char *map;
} mgaRegion, *mgaRegionPtr;

typedef struct mga_screen_private_s {

   int chipset;
   int width;
   int height;
   int mem;

   int cpp;			/* for front and back buffers */

   unsigned int mAccess;

   unsigned int frontOffset;
   unsigned int frontPitch;
   unsigned int backOffset;
   unsigned int backPitch;

   unsigned int depthOffset;
   unsigned int depthPitch;
   int depthCpp;

   unsigned int dmaOffset;		

   unsigned int textureOffset[MGA_NR_TEX_HEAPS];
   unsigned int textureSize[MGA_NR_TEX_HEAPS];
   int logTextureGranularity[MGA_NR_TEX_HEAPS];
   char *texVirtual[MGA_NR_TEX_HEAPS];


   __DRIscreenPrivate *sPriv;
   drmBufMapPtr  bufs;

   /* Maps the dma buffers as well as textures ? 
    */
   mgaRegion agp;

} mgaScreenPrivate;


#include "mgacontext.h"

extern void mgaGetLock( mgaContextPtr mmesa, GLuint flags );
extern void mgaEmitHwStateLocked( mgaContextPtr mmesa );
extern void mgaEmitScissorValues( mgaContextPtr mmesa, int box_nr, int emit );

#define GET_DISPATCH_AGE( mmesa ) mmesa->sarea->last_dispatch
#define GET_ENQUEUE_AGE( mmesa ) mmesa->sarea->last_enqueue



/* Lock the hardware and validate our state.  
 */
#define LOCK_HARDWARE( mmesa )					\
  do {								\
    char __ret=0;						\
    DRM_CAS(mmesa->driHwLock, mmesa->hHWContext,		\
	    (DRM_LOCK_HELD|mmesa->hHWContext), __ret);		\
    if (__ret)							\
        mgaGetLock( mmesa, 0 );					\
  } while (0)


/* 
 */
#define LOCK_HARDWARE_QUIESCENT( mmesa ) do {	                        \
	LOCK_HARDWARE( mmesa );			                        \
	mgaUpdateLock( mmesa, DRM_LOCK_QUIESCENT | DRM_LOCK_FLUSH );	\
} while (0)


/* Unlock the hardware using the global current context 
 */
#define UNLOCK_HARDWARE(mmesa) 				\
    DRM_UNLOCK(mmesa->driFd, mmesa->driHwLock, mmesa->hHWContext);	


/* Freshen our snapshot of the drawables
 */
#define REFRESH_DRAWABLE_INFO( mmesa )		\
do {						\
   LOCK_HARDWARE( mmesa );			\
   mmesa->lastX = mmesa->drawX; 		\
   mmesa->lastY = mmesa->drawY; 		\
   UNLOCK_HARDWARE( mmesa );			\
} while (0)


#define GET_DRAWABLE_LOCK( mmesa ) while(0)
#define RELEASE_DRAWABLE_LOCK( mmesa ) while(0)

#endif
#endif
