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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_context.c,v 1.6 2001/12/13 00:34:21 alanh Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *
 */

#include <dlfcn.h>
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


   fxMesa = (tdfxContextPtr) MALLOC( sizeof(tdfxContextRec) );
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

   /* NOTE: This MUST be called before any Glide functions are called! */
   if (!tdfxInitGlide(fxMesa)) {
      FREE(fxMesa);
      return GL_FALSE;
   }

   fxMesa->Glide.grDRIOpen( (char*) sPriv->pFB, fxScreen->regs.map, fxScreen->deviceID,
	      fxScreen->width, fxScreen->height, fxScreen->mem, fxScreen->cpp,
	      fxScreen->stride, fxScreen->fifoOffset, fxScreen->fifoSize,
	      fxScreen->fbOffset, fxScreen->backOffset, fxScreen->depthOffset,
	      fxScreen->textureOffset, fxScreen->textureSize, &saPriv->fifoPtr,
	      &saPriv->fifoRead );

   if ( getenv( "FX_GLIDE_SWAPINTERVAL" ) ) {
      fxMesa->Glide.SwapInterval = atoi( getenv( "FX_GLIDE_SWAPINTERVAL" ) );
   } else {
      fxMesa->Glide.SwapInterval = 0;
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

   fxMesa->Glide.grGet( GR_GLIDE_VERTEXLAYOUT_SIZE, sizeof(FxI32), &result );
   for ( i = 0 ; i < TDFX_NUM_LAYOUTS ; i++ ) {
      fxMesa->layout[i] = MALLOC( result );
      if ( !fxMesa->layout[i] ) {
	 UNLOCK_HARDWARE( fxMesa );
	 return GL_FALSE;
      }
   }

   /* Single textured vertex format - 32 bytes.
    */
   fxMesa->Glide.grReset( GR_VERTEX_PARAMETER );

   fxMesa->Glide.grCoordinateSpace( GR_WINDOW_COORDS );
   fxMesa->Glide.grVertexLayout( GR_PARAM_XY,	TDFX_XY_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_Z, TDFX_Z_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_Q, TDFX_Q_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_PARGB, TDFX_ARGB_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_ST0, TDFX_ST0_OFFSET, GR_PARAM_ENABLE );
#if 0
   fxMesa->Glide.grVertexLayout( GR_PARAM_FOG_EXT, TDFX_FOG_OFFSET, GR_PARAM_ENABLE );
#endif

   fxMesa->Glide.grGlideGetVertexLayout( fxMesa->layout[TDFX_LAYOUT_SINGLE] );

   /* Multitextured vertex format - 40 bytes.
    */
   fxMesa->Glide.grReset( GR_VERTEX_PARAMETER );

   fxMesa->Glide.grCoordinateSpace( GR_WINDOW_COORDS );
   fxMesa->Glide.grVertexLayout( GR_PARAM_XY, TDFX_XY_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_Z, TDFX_Z_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_Q, TDFX_Q_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_PARGB, TDFX_ARGB_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_ST0, TDFX_ST0_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_ST1, TDFX_ST1_OFFSET, GR_PARAM_ENABLE );
#if 0
   fxMesa->Glide.grVertexLayout( GR_PARAM_FOG_EXT, TDFX_FOG_OFFSET, GR_PARAM_ENABLE );
#endif

   fxMesa->Glide.grGlideGetVertexLayout( fxMesa->layout[TDFX_LAYOUT_MULTI] );

   /* Projected texture vertex format - 48 bytes.
    */
   fxMesa->Glide.grReset( GR_VERTEX_PARAMETER );

   fxMesa->Glide.grCoordinateSpace( GR_WINDOW_COORDS );
   fxMesa->Glide.grVertexLayout( GR_PARAM_XY, TDFX_XY_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_Z, TDFX_Z_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_Q, TDFX_Q_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_PARGB, TDFX_ARGB_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_ST0, TDFX_ST0_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_ST1, TDFX_ST1_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_Q0, TDFX_Q0_OFFSET, GR_PARAM_ENABLE );
   fxMesa->Glide.grVertexLayout( GR_PARAM_Q1, TDFX_Q1_OFFSET, GR_PARAM_ENABLE );
#if 0
   fxMesa->Glide.grVertexLayout( GR_PARAM_FOG_EXT, TDFX_FOG_OFFSET, GR_PARAM_ENABLE );
#endif

   fxMesa->Glide.grGlideGetVertexLayout( fxMesa->layout[TDFX_LAYOUT_PROJECT] );

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

   fxMesa->Glide.grGlideInit();
   fxMesa->Glide.grSstSelect( fxMesa->Glide.Board );

   fxMesa->Glide.Context = fxMesa->Glide.grSstWinOpen( (FxU32) -1,
					 GR_RESOLUTION_NONE,
					 GR_REFRESH_NONE,
					 fxMesa->Glide.ColorFormat,
					 fxMesa->Glide.Origin,
					 2, 1 );

   fxMesa->Glide.grDRIResetSAREA();

   DRM_UNLOCK( fxMesa->driFd, fxMesa->driHwLock, fxMesa->hHWContext );

   if ( !fxMesa->Glide.Context )
      return GL_FALSE;


   /* Perform the Glide-dependant part of the context initialization.
    */
   FX_grColorMaskv( fxMesa->glCtx, true4 );

   tdfxTMInit( fxMesa );

   LOCK_HARDWARE( fxMesa );

   if ( fxMesa->glVis->DepthBits > 0 ) {
      fxMesa->Glide.grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
   } else {
      fxMesa->Glide.grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
   }

   fxMesa->Glide.grLfbWriteColorFormat( GR_COLORFORMAT_ABGR );

   fxMesa->Glide.grGet( GR_TEXTURE_ALIGN, sizeof(FxI32), result );
   fxMesa->Glide.TextureAlign = result[0];

   fxMesa->Glide.State = NULL;
   fxMesa->Glide.grGet( GR_GLIDE_STATE_SIZE, sizeof(FxI32), result );
   fxMesa->Glide.State = MALLOC( result[0] );

   fxMesa->Fog.Table = NULL;
   fxMesa->Glide.grGet( GR_FOG_TABLE_ENTRIES, sizeof(FxI32), result );
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

   fxMesa->Glide.grGlideGetState( fxMesa->Glide.State );

   if ( getenv( "FX_GLIDE_INFO" ) ) {
      printf( "GR_RENDERER  = %s\n", (char *) fxMesa->Glide.grGetString( GR_RENDERER ) );
      printf( "GR_VERSION   = %s\n", (char *) fxMesa->Glide.grGetString( GR_VERSION ) );
      printf( "GR_VENDOR    = %s\n", (char *) fxMesa->Glide.grGetString( GR_VENDOR ) );
      printf( "GR_HARDWARE  = %s\n", (char *) fxMesa->Glide.grGetString( GR_HARDWARE ) );
      printf( "GR_EXTENSION = %s\n", (char *) fxMesa->Glide.grGetString( GR_EXTENSION ) );
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
      FREE( fxMesa );
   }

#if 0
   glx_fini_prof();
#endif
}



/*
 * Examine the context's deviceID to determine what kind of 3dfx hardware
 * is installed.  dlopen() the appropriate Glide library and initialize
 * this context's Glide function pointers.
 * Return:  true/false = success/failure
 */
GLboolean tdfxInitGlide(tdfxContextPtr tmesa)
{
   static const char *defaultGlide = "libglide3.so";
   const char *libName;
   void *libHandle;

   /*
    * XXX this code which selects a Glide library filename given the
    * deviceID may need to be cleaned up a bit.
    * Non-Linux systems may have different filenames, for example.
    */
   switch (tmesa->fxScreen->deviceID) {
   case PCI_CHIP_BANSHEE:
   case PCI_CHIP_VOODOO3:
      libName = "libglide3-v3.so";
      break;
   case PCI_CHIP_VOODOO5:   /* same as PCI_CHIP_VOODOO4 */
      libName = "libglide3-v5.so";
      break;
   default:
      {
         char err[1000];
         sprintf(err, "unrecognized 3dfx deviceID: 0x%x",
                 tmesa->fxScreen->deviceID);
         __driMesaMessage(err);
      }
      return GL_FALSE;
   }

   libHandle = dlopen(libName, RTLD_NOW);
   if (!libHandle) {
      /* The device-specific Glide library filename didn't work, try the
       * old, generic libglide3.so library.
       */
      libHandle = dlopen(defaultGlide, RTLD_NOW); 
      if (!libHandle) {
         char err[1000];
         sprintf(err,
            "can't find Glide library, dlopen(%s) and dlopen(%s) both failed.",
            libName, defaultGlide);
         __driMesaMessage(err);
	 sprintf(err, "dlerror() message: %s", dlerror());
	 __driMesaMessage(err);
         return GL_FALSE;
      }
      libName = defaultGlide;
   }

   {
      const char *env = getenv("LIBGL_DEBUG");
      if (env && strstr(env, "verbose")) {
         fprintf(stderr, "libGL: using Glide library %s\n", libName);
      }
   }         

#define GET_FUNCTION(PTR, NAME)						\
   tmesa->Glide.PTR = dlsym(libHandle, NAME);				\
   if (!tmesa->Glide.PTR) {						\
      char err[1000];							\
      sprintf(err, "couldn't find Glide function %s in %s.",		\
              NAME, libName);						\
      __driMesaMessage(err);						\
   }

   GET_FUNCTION(grDrawPoint, "grDrawPoint");
   GET_FUNCTION(grDrawLine, "grDrawLine");
   GET_FUNCTION(grDrawTriangle, "grDrawTriangle");
   GET_FUNCTION(grVertexLayout, "grVertexLayout");
   GET_FUNCTION(grDrawVertexArray, "grDrawVertexArray");
   GET_FUNCTION(grDrawVertexArrayContiguous, "grDrawVertexArrayContiguous");
   GET_FUNCTION(grBufferClear, "grBufferClear");
   /*GET_FUNCTION(grBufferSwap, "grBufferSwap");*/
   GET_FUNCTION(grRenderBuffer, "grRenderBuffer");
   GET_FUNCTION(grErrorSetCallback, "grErrorSetCallback");
   GET_FUNCTION(grFinish, "grFinish");
   GET_FUNCTION(grFlush, "grFlush");
   GET_FUNCTION(grSstWinOpen, "grSstWinOpen");
   GET_FUNCTION(grSstWinClose, "grSstWinClose");
#if 0
   /* Not in V3 lib, and not used anyway. */
   GET_FUNCTION(grSetNumPendingBuffers, "grSetNumPendingBuffers");
#endif
   GET_FUNCTION(grSelectContext, "grSelectContext");
   GET_FUNCTION(grSstOrigin, "grSstOrigin");
   GET_FUNCTION(grSstSelect, "grSstSelect");
   GET_FUNCTION(grAlphaBlendFunction, "grAlphaBlendFunction");
   GET_FUNCTION(grAlphaCombine, "grAlphaCombine");
   GET_FUNCTION(grAlphaControlsITRGBLighting, "grAlphaControlsITRGBLighting");
   GET_FUNCTION(grAlphaTestFunction, "grAlphaTestFunction");
   GET_FUNCTION(grAlphaTestReferenceValue, "grAlphaTestReferenceValue");
   GET_FUNCTION(grChromakeyMode, "grChromakeyMode");
   GET_FUNCTION(grChromakeyValue, "grChromakeyValue");
   GET_FUNCTION(grClipWindow, "grClipWindow");
   GET_FUNCTION(grColorCombine, "grColorCombine");
   GET_FUNCTION(grColorMask, "grColorMask");
   GET_FUNCTION(grCullMode, "grCullMode");
   GET_FUNCTION(grConstantColorValue, "grConstantColorValue");
   GET_FUNCTION(grDepthBiasLevel, "grDepthBiasLevel");
   GET_FUNCTION(grDepthBufferFunction, "grDepthBufferFunction");
   GET_FUNCTION(grDepthBufferMode, "grDepthBufferMode");
   GET_FUNCTION(grDepthMask, "grDepthMask");
   GET_FUNCTION(grDisableAllEffects, "grDisableAllEffects");
   GET_FUNCTION(grDitherMode, "grDitherMode");
   GET_FUNCTION(grFogColorValue, "grFogColorValue");
   GET_FUNCTION(grFogMode, "grFogMode");
   GET_FUNCTION(grFogTable, "grFogTable");
   GET_FUNCTION(grLoadGammaTable, "grLoadGammaTable");
   GET_FUNCTION(grSplash, "grSplash");
   GET_FUNCTION(grGet, "grGet");
   GET_FUNCTION(grGetString, "grGetString");
   GET_FUNCTION(grQueryResolutions, "grQueryResolutions");
   GET_FUNCTION(grReset, "grReset");
   GET_FUNCTION(grGetProcAddress, "grGetProcAddress");
   GET_FUNCTION(grEnable, "grEnable");
   GET_FUNCTION(grDisable, "grDisable");
   GET_FUNCTION(grCoordinateSpace, "grCoordinateSpace");
   GET_FUNCTION(grDepthRange, "grDepthRange");
#if defined(__linux__) || defined(__FreeBSD__) 
   GET_FUNCTION(grStippleMode, "grStippleMode");
   GET_FUNCTION(grStipplePattern, "grStipplePattern");
#endif /* __linux__ || __FreeBSD__ */
   GET_FUNCTION(grViewport, "grViewport");
   GET_FUNCTION(grTexCalcMemRequired, "grTexCalcMemRequired");
   GET_FUNCTION(grTexTextureMemRequired, "grTexTextureMemRequired");
   GET_FUNCTION(grTexMinAddress, "grTexMinAddress");
   GET_FUNCTION(grTexMaxAddress, "grTexMaxAddress");
   GET_FUNCTION(grTexNCCTable, "grTexNCCTable");
   GET_FUNCTION(grTexSource, "grTexSource");
   GET_FUNCTION(grTexClampMode, "grTexClampMode");
   GET_FUNCTION(grTexCombine, "grTexCombine");
   GET_FUNCTION(grTexDetailControl, "grTexDetailControl");
   GET_FUNCTION(grTexFilterMode, "grTexFilterMode");
   GET_FUNCTION(grTexLodBiasValue, "grTexLodBiasValue");
   GET_FUNCTION(grTexDownloadMipMap, "grTexDownloadMipMap");
   GET_FUNCTION(grTexDownloadMipMapLevel, "grTexDownloadMipMapLevel");
   GET_FUNCTION(grTexDownloadMipMapLevelPartial, "grTexDownloadMipMapLevelPartial");
   GET_FUNCTION(grTexDownloadTable, "grTexDownloadTable");
   GET_FUNCTION(grTexDownloadTablePartial, "grTexDownloadTablePartial");
   GET_FUNCTION(grTexMipMapMode, "grTexMipMapMode");
   GET_FUNCTION(grTexMultibase, "grTexMultibase");
   GET_FUNCTION(grTexMultibaseAddress, "grTexMultibaseAddress");
   GET_FUNCTION(grLfbLock, "grLfbLock");
   GET_FUNCTION(grLfbUnlock, "grLfbUnlock");
   GET_FUNCTION(grLfbConstantAlpha, "grLfbConstantAlpha");
   GET_FUNCTION(grLfbConstantDepth, "grLfbConstantDepth");
   GET_FUNCTION(grLfbWriteColorSwizzle, "grLfbWriteColorSwizzle");
   GET_FUNCTION(grLfbWriteColorFormat, "grLfbWriteColorFormat");
   GET_FUNCTION(grLfbWriteRegion, "grLfbWriteRegion");
   GET_FUNCTION(grLfbReadRegion, "grLfbReadRegion");
   GET_FUNCTION(grGlideInit, "grGlideInit");
   GET_FUNCTION(grGlideShutdown, "grGlideShutdown");
   GET_FUNCTION(grGlideGetState, "grGlideGetState");
   GET_FUNCTION(grGlideSetState, "grGlideSetState");
   GET_FUNCTION(grGlideGetVertexLayout, "grGlideGetVertexLayout");
   GET_FUNCTION(grGlideSetVertexLayout, "grGlideSetVertexLayout");

   /* Glide utility functions */
   GET_FUNCTION(guFogGenerateExp, "guFogGenerateExp");
   GET_FUNCTION(guFogGenerateExp2, "guFogGenerateExp2");
   GET_FUNCTION(guFogGenerateLinear, "guFogGenerateLinear");

   /* DRI functions */
   GET_FUNCTION(grDRIOpen, "grDRIOpen");
   GET_FUNCTION(grDRIPosition, "grDRIPosition");
   /*GET_FUNCTION(grDRILostContext, "grDRILostContext");*/
   GET_FUNCTION(grDRIImportFifo, "grDRIImportFifo");
   GET_FUNCTION(grDRIInvalidateAll, "grDRIInvalidateAll");
   GET_FUNCTION(grDRIResetSAREA, "grDRIResetSAREA");
   GET_FUNCTION(grDRIBufferSwap, "grDRIBufferSwap");

   /*
    * Extension functions:
    * Just use dlysm() because we want a NULL pointer if the function is
    * not found.
    */
   /* PIXEXT extension */
   tmesa->Glide.grStencilFunc = dlsym(libHandle, "grStencilFunc");
   tmesa->Glide.grStencilMask = dlsym(libHandle, "grStencilMask");
   tmesa->Glide.grStencilOp = dlsym(libHandle, "grStencilOp");
   tmesa->Glide.grBufferClearExt = dlsym(libHandle, "grBufferClearExt");
   tmesa->Glide.grColorMaskExt = dlsym(libHandle, "grColorMaskExt");
   /* COMBINE extension */
   tmesa->Glide.grColorCombineExt = dlsym(libHandle, "grColorCombineExt");
   tmesa->Glide.grTexColorCombineExt = dlsym(libHandle, "grTexColorCombineExt");
   tmesa->Glide.grAlphaCombineExt = dlsym(libHandle, "grAlphaCombineExt");
   tmesa->Glide.grTexAlphaCombineExt = dlsym(libHandle, "grTexAlphaCombineExt");
   tmesa->Glide.grAlphaBlendFunctionExt = dlsym(libHandle, "grAlphaBlendFunctionExt");
   tmesa->Glide.grConstantColorValueExt = dlsym(libHandle, "grConstantColorValueExt");
   /* Texus 2 */
   tmesa->Glide.txImgQuantize = dlsym(libHandle, "txImgQuantize");
   tmesa->Glide.txImgDequantizeFXT1 = dlsym(libHandle, "_txImgDequantizeFXT1");
   tmesa->Glide.txErrorSetCallback = dlsym(libHandle, "txErrorSetCallback");
   
   return GL_TRUE;
}
