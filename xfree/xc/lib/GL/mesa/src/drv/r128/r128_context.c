/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_context.c,v 1.6 2001/03/21 16:14:23 dawes Exp $ */
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

#include <stdlib.h>

#include "r128_context.h"
#include "r128_ioctl.h"
#include "r128_dd.h"
#include "r128_state.h"
#include "r128_span.h"
#include "r128_tex.h"
#include "r128_vb.h"
#include "r128_pipeline.h"

#include "context.h"
#include "simple_list.h"
#include "mem.h"

#ifndef R128_DEBUG
int R128_DEBUG = (0
/*		  | DEBUG_ALWAYS_SYNC */
/*		  | DEBUG_VERBOSE_API */
/*		  | DEBUG_VERBOSE_MSG */
/*		  | DEBUG_VERBOSE_LRU */
/*		  | DEBUG_VERBOSE_DRI */
/*		  | DEBUG_VERBOSE_IOCTL */
/*		  | DEBUG_VERBOSE_2D */
   );
#endif

/* Create the device specific context.
 */
GLboolean r128CreateContext( Display *dpy, GLvisual *glVisual,
			     __DRIcontextPrivate *driContextPriv )
{
   GLcontext *ctx = driContextPriv->mesaContext;
   __DRIscreenPrivate *sPriv = driContextPriv->driScreenPriv;
   r128ContextPtr rmesa;
   r128ScreenPtr r128scrn;
   int i;

   rmesa = (r128ContextPtr) CALLOC( sizeof(*rmesa) );
   if ( !rmesa ) return GL_FALSE;

   rmesa->glCtx = ctx;
   rmesa->display = dpy;

   rmesa->driContext = driContextPriv;
   rmesa->driScreen = sPriv;
   rmesa->driDrawable = NULL; /* Set by XMesaMakeCurrent */

   rmesa->hHWContext = driContextPriv->hHWContext;
   rmesa->driHwLock = &sPriv->pSAREA->lock;
   rmesa->driFd = sPriv->fd;

   r128scrn = rmesa->r128Screen = (r128ScreenPtr)(sPriv->private);

   rmesa->sarea = (R128SAREAPrivPtr)((char *)sPriv->pSAREA +
				     r128scrn->sarea_priv_offset);

   rmesa->tmp_matrix = (GLfloat *) ALIGN_MALLOC( 16 * sizeof(GLfloat), 16 );
   if ( !rmesa->tmp_matrix ) {
      FREE( rmesa );
      return GL_FALSE;
   }

   rmesa->CurrentTexObj[0] = NULL;
   rmesa->CurrentTexObj[1] = NULL;

   make_empty_list( &rmesa->SwappedOut );

   for ( i = 0 ; i < r128scrn->numTexHeaps ; i++ ) {
      make_empty_list( &rmesa->TexObjList[i] );
      rmesa->texHeap[i] = mmInit( 0, r128scrn->texSize[i] );
      rmesa->lastTexAge[i] = -1;
   }
   rmesa->lastTexHeap = r128scrn->numTexHeaps;

   rmesa->RenderIndex = -1;		/* Impossible value */
   rmesa->OnFastPath = 0;

   rmesa->vert_buf = NULL;
   rmesa->num_verts = 0;

   rmesa->elt_buf = NULL;
   rmesa->retained_buf = NULL;
   rmesa->vert_heap = r128scrn->buffers->list->address;

   /* KW: Set the maximum texture size small enough that we can
    * guarentee that both texture units can bind a maximal texture
    * and have them both in on-card memory at once.  (Kevin or
    * Gareth: Please check these numbers are OK)
    */
   if ( r128scrn->texSize[0] < 2*1024*1024 ) {
      ctx->Const.MaxTextureLevels = 9;
      ctx->Const.MaxTextureSize = (1 << 8);
   } else if ( r128scrn->texSize[0] < 8*1024*1024 ) {
      ctx->Const.MaxTextureLevels = 10;
      ctx->Const.MaxTextureSize = (1 << 9);
   } else {
      ctx->Const.MaxTextureLevels = 11;
      ctx->Const.MaxTextureSize = (1 << 10);
   }

   ctx->Const.MaxTextureUnits = 2;

#if ENABLE_PERF_BOXES
   if ( getenv( "LIBGL_PERFORMANCE_BOXES" ) ) {
      rmesa->boxes = 1;
   } else {
      rmesa->boxes = 0;
   }
#endif

   ctx->DriverCtx = (void *)rmesa;

   r128DDInitExtensions( ctx );

   r128DDInitDriverFuncs( ctx );
   r128DDInitIoctlFuncs( ctx );
   r128DDInitStateFuncs( ctx );
   r128DDInitSpanFuncs( ctx );
   r128DDInitTextureFuncs( ctx );

   ctx->Driver.TriangleCaps = (DD_TRI_CULL |
			       DD_TRI_LIGHT_TWOSIDE |
			       DD_TRI_STIPPLE |
			       DD_TRI_OFFSET);

   /* Ask Mesa to clip fog coordinates for us.
    */
   ctx->TriangleCaps |= DD_CLIP_FOG_COORD;

   if ( ctx->VB )
      r128DDRegisterVB( ctx->VB );

   if ( ctx->NrPipelineStages ) {
      ctx->NrPipelineStages =
	 r128DDRegisterPipelineStages( ctx->PipelineStage,
				       ctx->PipelineStage,
				       ctx->NrPipelineStages );
   }

   r128DDInitState( rmesa );

   driContextPriv->driverPrivate = (void *)rmesa;

   return GL_TRUE;
}

/* Destroy the device specific context.
 */
void r128DestroyContext( r128ContextPtr rmesa )
{
   if ( rmesa ) {
      r128TexObjPtr t, next_t;
      int i;

      for ( i = 0 ; i < rmesa->r128Screen->numTexHeaps ; i++ ) {
	 foreach_s ( t, next_t, &rmesa->TexObjList[i] ) {
	    r128DestroyTexObj( rmesa, t );
	 }
	 mmDestroy( rmesa->texHeap[i] );
      }

      foreach_s ( t, next_t, &rmesa->SwappedOut ) {
	 r128DestroyTexObj( rmesa, t );
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
 * setting of the hardware state is done in the r128UpdateHWState().
 */
r128ContextPtr r128MakeCurrent( r128ContextPtr oldCtx,
				r128ContextPtr newCtx,
				__DRIdrawablePrivate *dPriv )
{
   if ( oldCtx ) {
      if ( oldCtx != newCtx ) {
	 newCtx->new_state |= R128_NEW_CONTEXT;
	 newCtx->dirty = R128_UPLOAD_ALL;
      }
      if ( oldCtx->driDrawable != dPriv ) {
	 newCtx->new_state |= R128_NEW_WINDOW | R128_NEW_CLIP;
      }
   } else {
      newCtx->new_state |= R128_NEW_CONTEXT;
      newCtx->dirty = R128_UPLOAD_ALL;
   }

   newCtx->driDrawable = dPriv;

   return newCtx;
}
