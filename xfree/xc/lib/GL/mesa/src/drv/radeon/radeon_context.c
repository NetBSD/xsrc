/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_context.c,v 1.2 2001/03/21 16:14:24 dawes Exp $ */
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

#include <stdlib.h>

#include "radeon_context.h"
#include "radeon_ioctl.h"
#include "radeon_dd.h"
#include "radeon_state.h"
#include "radeon_span.h"
#include "radeon_tex.h"
#include "radeon_vb.h"
#include "radeon_pipeline.h"

#include "context.h"
#include "simple_list.h"
#include "mem.h"

#ifndef RADEON_DEBUG
int RADEON_DEBUG = (0
/*		    | DEBUG_ALWAYS_SYNC */
/*		    | DEBUG_VERBOSE_API */
/*		    | DEBUG_VERBOSE_MSG */
/*		    | DEBUG_VERBOSE_LRU */
/*		    | DEBUG_VERBOSE_DRI */
/*		    | DEBUG_VERBOSE_IOCTL */
/*		    | DEBUG_VERBOSE_2D */
/*		    | DEBUG_VERBOSE_TEXTURE */
   );
#endif

#ifdef PER_CONTEXT_SAREA
char *radeonGetPerContextSAREA(int fd,
			       drmContext hHWContext,
			       drmSize size)
{
   drmHandle handle;
   drmAddress address;

   if(drmGetContextPrivateMapping(fd, hHWContext, &handle) < 0) {
      return NULL;
   }
   if(drmMap(fd, handle, size, &address) < 0) {
      return NULL;
   }

   return address;
}
#endif

/* Create the device specific context.
 */
GLboolean radeonCreateContext( Display *dpy, GLvisual *glVisual,
			       __DRIcontextPrivate *driContextPriv )
{
   GLcontext *ctx = driContextPriv->mesaContext;
   __DRIscreenPrivate *sPriv = driContextPriv->driScreenPriv;
   radeonContextPtr rmesa;
   radeonScreenPtr radeonScreen;
   int i;

   rmesa = (radeonContextPtr) CALLOC( sizeof(*rmesa) );
   if ( !rmesa ) return GL_FALSE;

   rmesa->glCtx = ctx;
   rmesa->display = dpy;

   rmesa->driContext = driContextPriv;
   rmesa->driScreen = sPriv;
   rmesa->driDrawable = NULL; /* Set by XMesaMakeCurrent */

   rmesa->hHWContext = driContextPriv->hHWContext;
   rmesa->driHwLock = &sPriv->pSAREA->lock;
   rmesa->driFd = sPriv->fd;

   radeonScreen = rmesa->radeonScreen = (radeonScreenPtr)(sPriv->private);

   rmesa->sarea = (RADEONSAREAPrivPtr)((char *)sPriv->pSAREA +
				       radeonScreen->sarea_priv_offset);

#ifdef PER_CONTEXT_SAREA
   rmesa->private_sarea = radeonGetPerContextSAREA(rmesa->driFd,
					   rmesa->hHWContext,
					   radeonScreen->private_sarea_size);
   if(!rmesa->private_sarea) {
      fprintf(stderr, "Can't map private SAREA\n");
      FREE( rmesa );
      return GL_FALSE;
   }
#endif

   rmesa->tmp_matrix = (GLfloat *) ALIGN_MALLOC( 16 * sizeof(GLfloat), 16 );
   if ( !rmesa->tmp_matrix ) {
      FREE( rmesa );
      return GL_FALSE;
   }

   make_empty_list( &rmesa->SwappedOut );

   for ( i = 0 ; i < radeonScreen->numTexHeaps ; i++ ) {
      rmesa->CurrentTexObj[i] = NULL;
      make_empty_list( &rmesa->TexObjList[i] );
      rmesa->texHeap[i] = mmInit( 0, radeonScreen->texSize[i] );
      rmesa->lastTexAge[i] = -1;
   }
   rmesa->lastTexHeap = radeonScreen->numTexHeaps;

   rmesa->RenderIndex = -1;		/* Impossible value */
   rmesa->OnFastPath = 0;

   rmesa->vert_buf = NULL;
   rmesa->num_verts = 0;

   rmesa->elt_buf = NULL;
   rmesa->retained_buf = NULL;
   rmesa->vert_heap = radeonScreen->buffers->list->address;

   /* KW: Set the maximum texture size small enough that we can
    * guarentee that both texture units can bind a maximal texture
    * and have them both in on-card memory at once.  (Kevin or
    * Gareth: Please check these numbers are OK)
    */
   if ( radeonScreen->texSize[0] < 2*1024*1024 ) {
      ctx->Const.MaxTextureLevels = 9;
      ctx->Const.MaxTextureSize = (1 << 8);
   } else if ( radeonScreen->texSize[0] < 8*1024*1024 ) {
      ctx->Const.MaxTextureLevels = 10;
      ctx->Const.MaxTextureSize = (1 << 9);
   } else {
      ctx->Const.MaxTextureLevels = 11;
      ctx->Const.MaxTextureSize = (1 << 10);
   }

   ctx->Const.MaxTextureUnits = 2;

   ctx->DriverCtx = (void *)rmesa;

   radeonDDInitExtensions( ctx );

   radeonDDInitDriverFuncs( ctx );
   radeonDDInitIoctlFuncs( ctx );
   radeonDDInitStateFuncs( ctx );
   radeonDDInitSpanFuncs( ctx );
   radeonDDInitTextureFuncs( ctx );

   ctx->Driver.TriangleCaps = (DD_TRI_CULL |
			       DD_TRI_LIGHT_TWOSIDE |
			       DD_TRI_STIPPLE |
			       DD_TRI_OFFSET);

   /* Ask Mesa to clip fog coordinates for us.
    */
   ctx->TriangleCaps |= DD_CLIP_FOG_COORD;

   if ( ctx->VB )
      radeonDDRegisterVB( ctx->VB );

   if ( ctx->NrPipelineStages ) {
      ctx->NrPipelineStages =
	 radeonDDRegisterPipelineStages( ctx->PipelineStage,
					 ctx->PipelineStage,
					 ctx->NrPipelineStages );
   }

   radeonDDInitState( rmesa );

   driContextPriv->driverPrivate = (void *)rmesa;

   return GL_TRUE;
}

/* Destroy the device specific context.
 */
void radeonDestroyContext( radeonContextPtr rmesa )
{
   if ( rmesa ) {
      radeonScreenPtr radeonScreen = rmesa->radeonScreen;
      radeonTexObjPtr t, next_t;
      int i;

#ifdef PER_CONTEXT_SAREA
      if ( rmesa->private_sarea ) {
	 drmUnmap( (drmAddress)rmesa->private_sarea,
		   radeonScreen->private_sarea_size );
	 rmesa->private_sarea = NULL;
      }
#endif

      for ( i = 0 ; i < radeonScreen->numTexHeaps ; i++ ) {
	 foreach_s ( t, next_t, &rmesa->TexObjList[i] ) {
	    radeonDestroyTexObj( rmesa, t );
	 }
	 mmDestroy( rmesa->texHeap[i] );
      }

      foreach_s ( t, next_t, &rmesa->SwappedOut ) {
	 radeonDestroyTexObj( rmesa, t );
      }

      ALIGN_FREE( rmesa->tmp_matrix );
      FREE( rmesa );
   }

#if 0
   /* Use this to force shared object profiling. */
   glx_fini_prof();
#endif
}

/* Load the device specific context into the hardware.  The actual
 * setting of the hardware state is done in the radeonUpdateHWState().
 */
radeonContextPtr radeonMakeCurrent( radeonContextPtr oldCtx,
				    radeonContextPtr newCtx,
				    __DRIdrawablePrivate *dPriv )
{
   if ( oldCtx ) {
      if ( oldCtx != newCtx ) {
	 newCtx->new_state |= RADEON_NEW_CONTEXT;
	 newCtx->dirty = RADEON_UPLOAD_ALL;
      }
      if ( oldCtx->driDrawable != dPriv ) {
	 newCtx->new_state |= RADEON_NEW_WINDOW | RADEON_NEW_CLIP;
      }
   } else {
      newCtx->new_state |= RADEON_NEW_CONTEXT;
      newCtx->dirty = RADEON_UPLOAD_ALL;
   }

   newCtx->driDrawable = dPriv;

   return newCtx;
}
