/* -*- mode: c; c-basic-offset: 3 -*-
 *
 * Copyright 2000 VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_context.c,v 1.1.2.1 2001/05/22 21:25:41 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *
 */

#include "dri_glide.h"
#include "tdfx_context.h"
#include "tdfx_dd.h"
#include "tdfx_state.h"
#include "tdfx_vb.h"
#include "tdfx_render.h"
#include "tdfx_pipeline.h"
#include "tdfx_span.h"
#include "tdfx_tex.h"
#include "tdfx_texman.h"
#include "extensions.h"

#ifndef TDFX_DEBUG
int TDFX_DEBUG = (0
/*  		  | DEBUG_ALWAYS_SYNC */
/*		  | DEBUG_VERBOSE_API */
/*		  | DEBUG_VERBOSE_MSG */
/*		  | DEBUG_VERBOSE_LRU */
/*  		  | DEBUG_VERBOSE_DRI */
/*  		  | DEBUG_VERBOSE_IOCTL */
/*   		  | DEBUG_VERBOSE_2D */
/*   		  | DEBUG_VERBOSE_TEXTURE */
   );
#endif


#if 0
/* Example extension function */
static void
fxFooBarEXT(GLint i)
{
   printf("You called glFooBarEXT(%d)\n", i);
}
#endif


/*
 * Enable/Disable the extensions for this context.
 */
static void tdfxDDInitExtensions( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   gl_extensions_disable( ctx, "GL_EXT_blend_logic_op" );
   gl_extensions_disable( ctx, "GL_EXT_blend_minmax" );
   gl_extensions_disable( ctx, "GL_EXT_blend_subtract" );
   gl_extensions_disable( ctx, "GL_EXT_blend_color" );
   gl_extensions_disable( ctx, "GL_EXT_blend_func_separate" );
   gl_extensions_disable( ctx, "GL_EXT_point_parameters" );
   gl_extensions_disable( ctx, "GL_EXT_shared_texture_palette" );
   gl_extensions_disable( ctx, "GL_INGR_blend_func_separate" );
   gl_extensions_enable( ctx, "GL_HP_occlusion_test" );

   if ( fxMesa->numTMUs == 1 ) {
      gl_extensions_disable( ctx, "GL_EXT_texture_env_add" );
      gl_extensions_disable( ctx, "GL_ARB_multitexture" );
   }

   if ( TDFX_IS_NAPALM( fxMesa ) ) {
      gl_extensions_enable( ctx, "GL_EXT_texture_env_combine" );
   }

   if (fxMesa->haveHwStencil) {
      gl_extensions_enable( ctx, "GL_EXT_stencil_wrap" );
   }

   /* Example of hooking in an extension function.
    * For DRI-based drivers, also see __driRegisterExtensions in the
    * tdfx_xmesa.c file.
    */
#if 0
   {
      void **dispatchTable = (void **) ctx->Exec;
      const int _gloffset_FooBarEXT = 555; /* just an example number! */
      const int tabSize = _glapi_get_dispatch_table_size();
      assert(_gloffset_FooBarEXT < tabSize);
      dispatchTable[_gloffset_FooBarEXT] = (void *) tdfxFooBarEXT;
      /* XXX You would also need to hook into the display list dispatch
       * table.  Really, the implementation of extensions might as well
       * be in the core of Mesa since core Mesa and the device driver
       * is one big shared lib.
       */
   }
#endif
}



GLboolean tdfxCreateContext( Display *dpy, GLvisual *mesaVis,
			     __DRIcontextPrivate *driContextPriv )
{
   tdfxContextPtr fxMesa;
   GLcontext *ctx = driContextPriv->mesaContext;
   __DRIscreenPrivate *sPriv = driContextPriv->driScreenPriv;
   tdfxScreenPrivate *fxScreen = (tdfxScreenPrivate *) sPriv->private;
   TDFXSAREAPriv *saPriv = (TDFXSAREAPriv *) ((char *) sPriv->pSAREA +
					      fxScreen->sarea_priv_offset);


   fxMesa = (tdfxContextPtr) Xmalloc( sizeof(tdfxContextRec) );
   if ( !fxMesa ) {
      return GL_FALSE;
   }
   BZERO(fxMesa, sizeof(tdfxContextRec));

   /* Mirror some important DRI state
    */
   fxMesa->hHWContext = driContextPriv->hHWContext;
   fxMesa->driHwLock = &sPriv->pSAREA->lock;
   fxMesa->driFd = sPriv->fd;

   fxMesa->driScreen = sPriv;
   fxMesa->driContext = driContextPriv;
   fxMesa->fxScreen = fxScreen;
   fxMesa->sarea = saPriv;



   fxMesa->haveHwStencil = ( TDFX_IS_NAPALM( fxMesa ) &&
			     mesaVis->StencilBits &&
			     mesaVis->DepthBits == 24 );

   fxMesa->screen_width = fxScreen->width;
   fxMesa->screen_height = fxScreen->height;

   fxMesa->new_state = ~0;
   fxMesa->dirty = ~0;

   fxMesa->vertexFormat = 0;

   fxMesa->glCtx = driContextPriv->mesaContext;
   fxMesa->glVis = mesaVis;

   grDRIOpen( sPriv->pFB, fxScreen->regs.map, fxScreen->deviceID,
	      fxScreen->width, fxScreen->height, fxScreen->mem, fxScreen->cpp,
	      fxScreen->stride, fxScreen->fifoOffset, fxScreen->fifoSize,
	      fxScreen->fbOffset, fxScreen->backOffset, fxScreen->depthOffset,
	      fxScreen->textureOffset, fxScreen->textureSize, &saPriv->fifoPtr,
	      &saPriv->fifoRead );

   if ( getenv( "FX_GLIDE_SWAPINTERVAL" ) ) {
      fxMesa->Glide.SwapInterval = atoi( getenv( "FX_GLIDE_SWAPINTERVAL" ) );
   } else {
      fxMesa->Glide.SwapInterval = 1;
   }
   if ( getenv( "FX_MAX_PENDING_SWAPS" ) ) {
      fxMesa->Glide.MaxPendingSwaps = atoi( getenv( "FX_MAX_PENDING_SWAPS" ) );
   } else {
      fxMesa->Glide.MaxPendingSwaps = 2;
   }

   fxMesa->Glide.Initialized = GL_FALSE;
   fxMesa->Glide.Board = 0;

   if ( getenv( "FX_EMULATE_SINGLE_TMU" ) || TDFX_IS_BANSHEE( fxMesa ) ) {
      fxMesa->numTMUs = 1;
   } else {
      fxMesa->numTMUs = 2;
   }

   fxMesa->stats.swapBuffer = 0;
   fxMesa->stats.reqTexUpload = 0;
   fxMesa->stats.texUpload = 0;
   fxMesa->stats.memTexUpload = 0;

   fxMesa->tmuSrc = TDFX_TMU_NONE;

   if ( TDFX_IS_NAPALM( fxMesa ) ) {
      ctx->Const.MaxTextureLevels = 12;
      ctx->Const.MaxTextureSize = 2048;
      ctx->Const.NumCompressedTextureFormats = 1;
   } else {
      ctx->Const.MaxTextureLevels = 9;
      ctx->Const.MaxTextureSize = 256;
      ctx->Const.NumCompressedTextureFormats = 0;
   }
   ctx->Const.MaxTextureUnits = TDFX_IS_BANSHEE( fxMesa ) ? 1 : 2;
   ctx->NewState |= NEW_DRVSTATE1;


   ctx->DriverCtx = (void *) fxMesa;

   tdfxDDInitExtensions( ctx );

   tdfxDDInitDriverFuncs( ctx );
   tdfxDDInitStateFuncs( ctx );
   tdfxDDInitRenderFuncs( ctx );
   tdfxDDInitSpanFuncs( ctx );
   tdfxDDInitTextureFuncs( ctx );

   ctx->Driver.TriangleCaps = (DD_TRI_CULL |
			       DD_TRI_LIGHT_TWOSIDE |
			       DD_TRI_STIPPLE |
			       DD_TRI_OFFSET);

   if ( ctx->VB )
      tdfxDDRegisterVB( ctx->VB );

   if ( ctx->NrPipelineStages )
      ctx->NrPipelineStages =
	 tdfxDDRegisterPipelineStages( ctx->PipelineStage,
				       ctx->PipelineStage,
				       ctx->NrPipelineStages );

   /* Run the config file */
   gl_context_initialize( ctx );

#if 0
   /* HACK: Allocate buffer for vertex data.
    */
   if ( fxMesa->buffer ) {
      ALIGN_FREE( fxMesa->buffer );
   }
   fxMesa->buffer = ALIGN_MALLOC( 2048, 32 );
   fxMesa->buffer_total = 2048;
   fxMesa->buffer_used = 0;
#endif

   tdfxInitState( fxMesa );

   driContextPriv->driverPrivate = (void *) fxMesa;

   return GL_TRUE;
}


static GLboolean tdfxInitVertexFormats( tdfxContextPtr fxMesa )
{
   FxI32 result;
   int i;

   LOCK_HARDWARE( fxMesa );

   grGet( GR_GLIDE_VERTEXLAYOUT_SIZE, sizeof(FxI32), &result );
   for ( i = 0 ; i < TDFX_NUM_LAYOUTS ; i++ ) {
      fxMesa->layout[i] = MALLOC( result );
      if ( !fxMesa->layout[i] ) {
	 UNLOCK_HARDWARE( fxMesa );
	 return GL_FALSE;
      }
   }

   /* Single textured vertex format - 32 bytes.
    */
   grReset( GR_VERTEX_PARAMETER );

   grCoordinateSpace( GR_WINDOW_COORDS );
   grVertexLayout( GR_PARAM_XY,	TDFX_XY_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_Z, TDFX_Z_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_Q, TDFX_Q_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_PARGB, TDFX_ARGB_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_ST0, TDFX_ST0_OFFSET, GR_PARAM_ENABLE );
#if 0
   grVertexLayout( GR_PARAM_FOG_EXT, TDFX_FOG_OFFSET, GR_PARAM_ENABLE );
#endif

   grGlideGetVertexLayout( fxMesa->layout[TDFX_LAYOUT_SINGLE] );

   /* Multitextured vertex format - 40 bytes.
    */
   grReset( GR_VERTEX_PARAMETER );

   grCoordinateSpace( GR_WINDOW_COORDS );
   grVertexLayout( GR_PARAM_XY, TDFX_XY_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_Z, TDFX_Z_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_Q, TDFX_Q_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_PARGB, TDFX_ARGB_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_ST0, TDFX_ST0_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_ST1, TDFX_ST1_OFFSET, GR_PARAM_ENABLE );
#if 0
   grVertexLayout( GR_PARAM_FOG_EXT, TDFX_FOG_OFFSET, GR_PARAM_ENABLE );
#endif

   grGlideGetVertexLayout( fxMesa->layout[TDFX_LAYOUT_MULTI] );

   /* Projected texture vertex format - 48 bytes.
    */
   grReset( GR_VERTEX_PARAMETER );

   grCoordinateSpace( GR_WINDOW_COORDS );
   grVertexLayout( GR_PARAM_XY, TDFX_XY_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_Z, TDFX_Z_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_Q, TDFX_Q_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_PARGB, TDFX_ARGB_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_ST0, TDFX_ST0_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_ST1, TDFX_ST1_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_Q0, TDFX_Q0_OFFSET, GR_PARAM_ENABLE );
   grVertexLayout( GR_PARAM_Q1, TDFX_Q1_OFFSET, GR_PARAM_ENABLE );
#if 0
   grVertexLayout( GR_PARAM_FOG_EXT, TDFX_FOG_OFFSET, GR_PARAM_ENABLE );
#endif

   grGlideGetVertexLayout( fxMesa->layout[TDFX_LAYOUT_PROJECT] );

   UNLOCK_HARDWARE( fxMesa );

   return GL_TRUE;
}


/*
 * Initialize the state in an tdfxContextPtr struct.
 */
GLboolean tdfxInitContext( __DRIdrawablePrivate *driDrawPriv,
			   tdfxContextPtr fxMesa )
{
   /* KW: Would be nice to make one of these a member of the other.
    */
   FxI32 result[2];

   if ( TDFX_DEBUG & DEBUG_VERBOSE_DRI ) {
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, fxMesa );
   }

#if DEBUG_LOCKING
   fprintf(stderr, "Debug locking enabled\n");
#endif

   if ( fxMesa->Glide.Initialized )
      return GL_TRUE;

   fxMesa->width = driDrawPriv->w;
   fxMesa->height = driDrawPriv->h;

   /* We have to use a light lock here, because we can't do any glide
    * operations yet. No use of FX_* functions in this function.
    */
   DRM_LIGHT_LOCK( fxMesa->driFd, fxMesa->driHwLock, fxMesa->hHWContext );

   grGlideInit();
   grSstSelect( fxMesa->Glide.Board );

   fxMesa->Glide.Context = grSstWinOpen( (FxU32) -1,
					 GR_RESOLUTION_NONE,
					 GR_REFRESH_NONE,
					 fxMesa->Glide.ColorFormat,
					 fxMesa->Glide.Origin,
					 2, 1 );

   grDRIResetSAREA();

   DRM_UNLOCK( fxMesa->driFd, fxMesa->driHwLock, fxMesa->hHWContext );

   if ( !fxMesa->Glide.Context )
      return GL_FALSE;


   /* Perform the Glide-dependant part of the context initialization.
    */
   FX_grColorMaskv( fxMesa->glCtx, true4 );

   tdfxTMInit( fxMesa );

   LOCK_HARDWARE( fxMesa );

   if ( fxMesa->glVis->DepthBits > 0 ) {
      grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
   } else {
      grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
   }

   grLfbWriteColorFormat( GR_COLORFORMAT_ABGR );

   grGet( GR_TEXTURE_ALIGN, sizeof(FxI32), result );
   fxMesa->Glide.TextureAlign = result[0];

   fxMesa->Glide.State = NULL;
   grGet( GR_GLIDE_STATE_SIZE, sizeof(FxI32), result );
   fxMesa->Glide.State = MALLOC( result[0] );

   fxMesa->Fog.Table = NULL;
   grGet( GR_FOG_TABLE_ENTRIES, sizeof(FxI32), result );
   fxMesa->Fog.Table = MALLOC( result[0] * sizeof(GrFog_t) );

   UNLOCK_HARDWARE( fxMesa );

   if ( !fxMesa->Glide.State || !fxMesa->Fog.Table ) {
      if ( fxMesa->Glide.State )
	 FREE( fxMesa->Glide.State );
      if ( fxMesa->Fog.Table )
	 FREE( fxMesa->Fog.Table );
      return GL_FALSE;
   }

   if ( !tdfxInitVertexFormats( fxMesa ) ) {
      return GL_FALSE;
   }

   LOCK_HARDWARE( fxMesa );

   grGlideGetState( fxMesa->Glide.State );

   if ( getenv( "FX_GLIDE_INFO" ) ) {
      printf( "GR_RENDERER  = %s\n", (char *) grGetString( GR_RENDERER ) );
      printf( "GR_VERSION   = %s\n", (char *) grGetString( GR_VERSION ) );
      printf( "GR_VENDOR    = %s\n", (char *) grGetString( GR_VENDOR ) );
      printf( "GR_HARDWARE  = %s\n", (char *) grGetString( GR_HARDWARE ) );
      printf( "GR_EXTENSION = %s\n", (char *) grGetString( GR_EXTENSION ) );
   }

   UNLOCK_HARDWARE( fxMesa );

   fxMesa->numClipRects = 0;
   fxMesa->pClipRects = NULL;
   fxMesa->scissoredClipRects = GL_FALSE;

   fxMesa->Glide.Initialized = GL_TRUE;

   return GL_TRUE;
}


void tdfxDestroyContext( tdfxContextPtr fxMesa )
{
   if ( TDFX_DEBUG & DEBUG_VERBOSE_DRI ) {
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, fxMesa );
   }

   if ( fxMesa ) {
      GLcontext *ctx = fxMesa->glCtx;
      struct gl_texture_object *tObj;

      if ( ctx->Shared->RefCount == 1 && fxMesa->driDrawable ) {
         /* This share group is about to go away, free our private
          * texture object data.
          */
	 LOCK_HARDWARE( fxMesa );
         for ( tObj = ctx->Shared->TexObjectList ; tObj ; tObj = tObj->Next ) {
            tdfxTMFreeTextureLocked( fxMesa, tObj );
         }
	 UNLOCK_HARDWARE( fxMesa );
      }

      tdfxTMClose( fxMesa );  /* free texture memory */
      XFree( fxMesa );
   }

#if 0
   glx_fini_prof();
#endif
}
