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

/*
 * Authors:
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 */

#ifndef _I810_INIT_H_
#define _I810_INIT_H_

#ifdef GLX_DIRECT_RENDERING

#include <sys/time.h>
#include "dri_tmm.h"
#include "dri_mesaint.h"
#include "dri_mesa.h"
#include "types.h"

typedef struct {
   drmHandle handle;
   drmSize size;
   char *map;
} i810Region, *i810RegionPtr;

typedef struct {
   i810Region front;
   i810Region back;
   i810Region depth;
   i810Region tex;

   int deviceID;
   int width;
   int height;
   int mem;

   int cpp;			/* for front and back buffers */
   int bitsPerPixel;

   int fbFormat;
   int fbOffset;
   int fbStride;

   int backOffset;
   int depthOffset;

   int backPitch;
   int backPitchBits;

   int textureOffset;
   int textureSize;
   int logTextureGranularity;

   __DRIscreenPrivate *driScrnPriv;
   drmBufMapPtr  bufs;
   int use_copy_buf;
} i810ScreenPrivate;


#include "i810context.h"

extern void i810GetLock( i810ContextPtr imesa, GLuint flags );
extern void i810EmitHwStateLocked( i810ContextPtr imesa );
extern void i810EmitScissorValues( i810ContextPtr imesa, int box_nr, int emit );
extern void i810EmitDrawingRectangle( i810ContextPtr imesa );
extern void i810XMesaSetBackClipRects( i810ContextPtr imesa );
extern void i810XMesaSetFrontClipRects( i810ContextPtr imesa );


#define GET_DISPATCH_AGE( imesa ) imesa->sarea->last_dispatch
#define GET_ENQUEUE_AGE( imesa ) imesa->sarea->last_enqueue


/* Lock the hardware and validate our state.  
 */
#define LOCK_HARDWARE( imesa )				\
  do {							\
    char __ret=0;					\
    DRM_CAS(imesa->driHwLock, imesa->hHWContext,	\
	    (DRM_LOCK_HELD|imesa->hHWContext), __ret);	\
    if (__ret)						\
        i810GetLock( imesa, 0 );			\
  } while (0)



/* Unlock the hardware using the global current context 
 */
#define UNLOCK_HARDWARE(imesa)					\
    DRM_UNLOCK(imesa->driFd, imesa->driHwLock, imesa->hHWContext);	


/* This is the wrong way to do it, I'm sure.  Otherwise the drm
 * bitches that I've already got the heavyweight lock.  At worst,
 * this is 3 ioctls.  The best solution probably only gets me down 
 * to 2 ioctls in the worst case.
 */
#define LOCK_HARDWARE_QUIESCENT( imesa ) do {	\
   LOCK_HARDWARE( imesa );			\
   i810RegetLockQuiescent( imesa );		\
} while(0)
     

#endif
#endif
