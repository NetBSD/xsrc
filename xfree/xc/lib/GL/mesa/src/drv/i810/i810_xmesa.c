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
/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810_xmesa.c,v 1.9 2000/12/07 20:26:06 dawes Exp $ */

/*
 * Authors:
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 */

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>
#include <stdio.h>

#include "i810_init.h"
#include "context.h"
#include "vbxform.h"
#include "matrix.h"
#include "simple_list.h"

#include "i810dd.h"
#include "i810state.h"
#include "i810tex.h"
#include "i810span.h"
#include "i810tris.h"
#include "i810pipeline.h"
#include "i810ioctl.h"

#include "i810_dri.h"



#ifndef I810_DEBUG
int I810_DEBUG = (0
/*     		  | DEBUG_ALWAYS_SYNC  */
/*  		  | DEBUG_VERBOSE_RING    */
/*  		  | DEBUG_VERBOSE_OUTREG  */
/*  		  | DEBUG_VERBOSE_MSG */
/*  		  | DEBUG_NO_OUTRING */
/*  		  | DEBUG_NO_OUTREG */
/*  		  | DEBUG_VERBOSE_API */
/*  		  | DEBUG_VERBOSE_2D */
/*  		  | DEBUG_VERBOSE_DRI */
/*  		  | DEBUG_VALIDATE_RING */
/*  		  | DEBUG_VERBOSE_IOCTL */
		  );
#endif


static i810ContextPtr      i810Ctx = 0;


/* These functions are accessed externally to the driver:
 *
 * XMesaInitDriver
 * XMesaResetDriver
 * XMesaCreateVisual
 * XMesaDestroyVisual
 * XMesaCreateContext 
 * XMesaDestroyContext
 * XMesaCreateWindowBuffer
 * XMesaCreatePixmapBuffer
 * XMesaDestroyBuffer
 * XMesaSwapBuffers
 * XMesaMakeCurrent
 *
 */


static int i810_malloc_proxy_buf(drmBufMapPtr buffers)
{
   char *buffer;
   drmBufPtr buf;
   int i;
   
   buffer = Xmalloc(I810_DMA_BUF_SZ);
   if(buffer == NULL) return -1;
   for(i = 0; i < I810_DMA_BUF_NR; i++) {
      buf = &(buffers->list[i]);
      buf->address = (drmAddress)buffer;
   }
   return 0;
}

static drmBufMapPtr i810_create_empty_buffers(void)
{
   drmBufMapPtr retval;
   
   retval = (drmBufMapPtr)Xmalloc(sizeof(drmBufMap));
   if(retval == NULL) return NULL;
   memset(retval, 0, sizeof(drmBufMap));
   retval->list = (drmBufPtr)Xmalloc(sizeof(drmBuf) * I810_DMA_BUF_NR);
   if(retval->list == NULL) {
      Xfree(retval);
      return NULL;
   }
   memset(retval->list, 0, sizeof(drmBuf) * I810_DMA_BUF_NR);
   /*
   fprintf(stderr, "retval : %p, retval->list : %p\n", retval, retval->list);
   */
   return retval;
}

GLboolean XMesaInitDriver(__DRIscreenPrivate *sPriv)
{
   i810ScreenPrivate *i810Screen;
   I810DRIPtr         gDRIPriv = (I810DRIPtr)sPriv->pDevPriv;

   /* Check the DRI version */
   {
      int major, minor, patch;
      if (XF86DRIQueryVersion(sPriv->display, &major, &minor, &patch)) {
         if (major != 3 || minor != 0 || patch < 0) {
            char msg[1000];
            sprintf(msg, "i810 DRI driver expected DRI version 3.0.x but got version %d.%d.%d", major, minor, patch);
            __driMesaMessage(msg);
            return GL_FALSE;
         }
      }
   }

   /* Check that the DDX driver version is compatible */
   if (sPriv->ddxMajor != 1 ||
       sPriv->ddxMinor != 0 ||
       sPriv->ddxPatch < 0) {
      char msg[1000];
      sprintf(msg, "i810 DRI driver expected DDX driver version 1.0.x but got version %d.%d.%d", sPriv->ddxMajor, sPriv->ddxMinor, sPriv->ddxPatch);
      __driMesaMessage(msg);
      return GL_FALSE;
   }

   /* Check that the DRM driver version is compatible */
   if (sPriv->drmMajor != 1 ||
       sPriv->drmMinor != 1 ||
       sPriv->drmPatch < 0) {
      char msg[1000];
      sprintf(msg, "i810 DRI driver expected DRM driver version 1.0.x but got version %d.%d.%d", sPriv->drmMajor, sPriv->drmMinor, sPriv->drmPatch);
      __driMesaMessage(msg);
      return GL_FALSE;
   }

   /* Allocate the private area */
   i810Screen = (i810ScreenPrivate *)Xmalloc(sizeof(i810ScreenPrivate));
   if (!i810Screen)
      return GL_FALSE;

   i810Screen->driScrnPriv = sPriv;
   sPriv->private = (void *)i810Screen;

   i810Screen->deviceID=gDRIPriv->deviceID;
   i810Screen->width=gDRIPriv->width;
   i810Screen->height=gDRIPriv->height;
   i810Screen->mem=gDRIPriv->mem;
   i810Screen->cpp=gDRIPriv->cpp;
   i810Screen->fbStride=gDRIPriv->fbStride;
   i810Screen->fbOffset=gDRIPriv->fbOffset;

   if (gDRIPriv->bitsPerPixel == 15) 
      i810Screen->fbFormat = DV_PF_555;
   else
      i810Screen->fbFormat = DV_PF_565;

   i810Screen->backOffset=gDRIPriv->backOffset; 
   i810Screen->depthOffset=gDRIPriv->depthOffset;
   i810Screen->backPitch = gDRIPriv->auxPitch;
   i810Screen->backPitchBits = gDRIPriv->auxPitchBits;
   i810Screen->textureOffset=gDRIPriv->textureOffset;
   i810Screen->textureSize=gDRIPriv->textureSize;
   i810Screen->logTextureGranularity = gDRIPriv->logTextureGranularity;

   if (0)
      fprintf(stderr, "Tex heap size %x, granularity %x bytes\n",
	      i810Screen->textureSize, 1<<(i810Screen->logTextureGranularity));

   i810Screen->bufs = i810_create_empty_buffers();
   if(i810Screen->bufs == NULL)
   {
      Xfree(i810Screen);
      return GL_FALSE;
   }
   
   /* Check if you need to create a fake buffer */
   if(i810_check_copy(sPriv->fd) == 1)
   {
      i810_malloc_proxy_buf(i810Screen->bufs);
      i810Screen->use_copy_buf = 1;
   }
   else
   {
      i810Screen->use_copy_buf = 0;
   }
    
   i810Screen->back.handle = gDRIPriv->backbuffer;
   i810Screen->back.size = gDRIPriv->backbufferSize;

   if (drmMap(sPriv->fd, 
	      i810Screen->back.handle, 
	      i810Screen->back.size, 
	      (drmAddress *)&i810Screen->back.map) != 0) 
   {
      Xfree(i810Screen);
      sPriv->private = NULL;
      return GL_FALSE;
   }

   i810Screen->depth.handle = gDRIPriv->depthbuffer;
   i810Screen->depth.size = gDRIPriv->depthbufferSize;

   if (drmMap(sPriv->fd, 
	      i810Screen->depth.handle, 
	      i810Screen->depth.size, 
	      (drmAddress *)&i810Screen->depth.map) != 0) 
   {
      Xfree(i810Screen);
      drmUnmap(i810Screen->back.map, i810Screen->back.size);
      sPriv->private = NULL;
      return GL_FALSE;
   }

   i810Screen->tex.handle = gDRIPriv->textures;
   i810Screen->tex.size = gDRIPriv->textureSize;

   if (drmMap(sPriv->fd, 
	      i810Screen->tex.handle, 
	      i810Screen->tex.size, 
	      (drmAddress *)&i810Screen->tex.map) != 0) 
   {
      Xfree(i810Screen);
      drmUnmap(i810Screen->back.map, i810Screen->back.size);
      drmUnmap(i810Screen->depth.map, i810Screen->depth.size);
      sPriv->private = NULL;
      return GL_FALSE;
   }


   i810DDFastPathInit();
   i810DDTrifuncInit();
   i810DDSetupInit();

   return GL_TRUE;
}

/* Accessed by dlsym from dri_mesa_init.c
 */
void XMesaResetDriver(__DRIscreenPrivate *sPriv)
{
   i810ScreenPrivate *i810Screen = (i810ScreenPrivate *)sPriv->private;

   /* Need to unmap all the bufs and maps here:
    */
   drmUnmap(i810Screen->back.map, i810Screen->back.size);
   drmUnmap(i810Screen->depth.map, i810Screen->depth.size);
   drmUnmap(i810Screen->tex.map, i810Screen->tex.size);

   Xfree(i810Screen);
   sPriv->private = NULL;
}


GLvisual *XMesaCreateVisual(Display *dpy,
                            __DRIscreenPrivate *driScrnPriv,
                            const XVisualInfo *visinfo,
                            const __GLXvisualConfig *config)
{
   /* Drivers may change the args to _mesa_create_visual() in order to
    * setup special visuals.
    */
   return _mesa_create_visual( config->rgba,
                               config->doubleBuffer,
                               config->stereo,
                               _mesa_bitcount(visinfo->red_mask),
                               _mesa_bitcount(visinfo->green_mask),
                               _mesa_bitcount(visinfo->blue_mask),
                               config->alphaSize,
                               0, /* index bits */
                               config->depthSize,
                               config->stencilSize,
                               config->accumRedSize,
                               config->accumGreenSize,
                               config->accumBlueSize,
                               config->accumAlphaSize,
                               0 /* num samples */ );
}


GLboolean XMesaCreateContext( Display *dpy, GLvisual *mesaVis,
                              __DRIcontextPrivate *driContextPriv )
{
   GLcontext *ctx = driContextPriv->mesaContext;
   i810ContextPtr imesa;
   __DRIscreenPrivate *sPriv = driContextPriv->driScreenPriv;
   i810ScreenPrivate *i810Screen = (i810ScreenPrivate *)sPriv->private;
   drm_i810_sarea_t *saPriv=(drm_i810_sarea_t *)(((char*)sPriv->pSAREA)+
						 sizeof(XF86DRISAREARec));

   imesa = (i810ContextPtr)Xcalloc(sizeof(i810Context), 1);
   if (!imesa) {
      return GL_FALSE;
   }


   /* Set the maximum texture size small enough that we can guarentee
    * that both texture units can bind a maximal texture and have them
    * in memory at once.
    */
   if (i810Screen->textureSize < 2*1024*1024) {
      ctx->Const.MaxTextureLevels = 9;
      ctx->Const.MaxTextureSize = 1<<8;
   } else if (i810Screen->textureSize < 8*1024*1024) {
      ctx->Const.MaxTextureLevels = 10;
      ctx->Const.MaxTextureSize = 1<<9;     
   } else {
      ctx->Const.MaxTextureLevels = 11;
      ctx->Const.MaxTextureSize = 1<<10;
   }      

   ctx->Const.MinLineWidth = 1.0;
   ctx->Const.MinLineWidthAA = 1.0;
   ctx->Const.MaxLineWidth = 3.0;
   ctx->Const.MaxLineWidthAA = 3.0;
   ctx->Const.LineWidthGranularity = 1.0;


   /* Dri stuff
    */
   imesa->display = dpy;
   imesa->hHWContext = driContextPriv->hHWContext;
   imesa->driFd = sPriv->fd;
   imesa->driHwLock = &sPriv->pSAREA->lock;

   imesa->i810Screen = i810Screen;
   imesa->driScreen = sPriv;
   imesa->sarea = saPriv;
   imesa->glBuffer = NULL;

   imesa->texHeap = mmInit( 0, i810Screen->textureSize );


   /* Utah stuff
    */
   imesa->renderindex = -1;		/* impossible value */
   imesa->new_state = ~0;
   imesa->dirty = ~0;

   make_empty_list(&imesa->TexObjList);
   make_empty_list(&imesa->SwappedOut);
   
   imesa->TextureMode = ctx->Texture.Unit[0].EnvMode;
   imesa->CurrentTexObj[0] = 0;
   imesa->CurrentTexObj[1] = 0;

   ctx->DriverCtx = (void *) imesa;
   imesa->glCtx = ctx;
   
   i810DDExtensionsInit( ctx );

   i810DDInitStateFuncs( ctx );
   i810DDInitTextureFuncs( ctx );
   i810DDInitSpanFuncs( ctx );
   i810DDInitDriverFuncs( ctx );
   i810DDInitIoctlFuncs( ctx );

   ctx->Driver.TriangleCaps = (DD_TRI_CULL|
			       DD_TRI_LIGHT_TWOSIDE|
			       DD_TRI_STIPPLE|
			       DD_TRI_OFFSET);

   /* Ask mesa to clip fog coordinates for us.
    */
   ctx->TriangleCaps |= DD_CLIP_FOG_COORD;

   if (ctx->VB) 
      i810DDRegisterVB( ctx->VB );

   if (ctx->NrPipelineStages)
      ctx->NrPipelineStages =
	 i810DDRegisterPipelineStages(ctx->PipelineStage,
				      ctx->PipelineStage,
				      ctx->NrPipelineStages);

   i810DDInitState( imesa );

   driContextPriv->driverPrivate = (void *) imesa;

   return GL_TRUE;
}

void XMesaDestroyContext(__DRIcontextPrivate *driContextPriv)
{
   i810ContextPtr imesa = (i810ContextPtr) driContextPriv->driverPrivate;

   if (imesa) {
      i810TextureObjectPtr next_t, t;

      foreach_s (t, next_t, &(imesa->TexObjList))
	 i810DestroyTexObj(imesa, t);

      foreach_s (t, next_t, &(imesa->SwappedOut))
	 i810DestroyTexObj(imesa, t);

      Xfree(imesa);
   }
}

GLframebuffer *XMesaCreateWindowBuffer( Display *dpy,
                                        __DRIscreenPrivate *driScrnPriv,
                                        __DRIdrawablePrivate *driDrawPriv,
                                        GLvisual *mesaVis)
{
   return gl_create_framebuffer(mesaVis,
                                GL_FALSE,  /* software depth buffer? */
                                mesaVis->StencilBits > 0,
                                mesaVis->AccumRedBits > 0,
                                mesaVis->AlphaBits > 0
                                );
}


GLframebuffer *XMesaCreatePixmapBuffer( Display *dpy,
                                        __DRIscreenPrivate *driScrnPriv,
                                        __DRIdrawablePrivate *driDrawPriv,
                                        GLvisual *mesaVis)
{
#if 0
   /* Different drivers may have different combinations of hardware and
    * software ancillary buffers.
    */
   return gl_create_framebuffer(mesaVis,
                                GL_FALSE,  /* software depth buffer? */
                                mesaVis->StencilBits > 0,
                                mesaVis->AccumRedBits > 0,
                                mesaVis->AlphaBits > 0
                                );
#else
   return NULL;  /* not implemented yet */
#endif
}


void XMesaSwapBuffers(__DRIdrawablePrivate *driDrawPriv)
{
   /* XXX should do swap according to the buffer, not the context! */
   i810ContextPtr imesa = i810Ctx; 

   FLUSH_VB( imesa->glCtx, "swap buffers" );
   i810SwapBuffers(imesa);
}



void i810XMesaSetFrontClipRects( i810ContextPtr imesa )
{
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;

   imesa->numClipRects = dPriv->numClipRects;
   imesa->pClipRects = dPriv->pClipRects;
   imesa->dirty |= I810_UPLOAD_CLIPRECTS;
   imesa->drawX = dPriv->x;
   imesa->drawY = dPriv->y;

   i810EmitDrawingRectangle( imesa );
}


void i810XMesaSetBackClipRects( i810ContextPtr imesa )
{
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   int i;

   if (dPriv->numBackClipRects == 0) 
   {
      if (I810_DEBUG & DEBUG_VERBOSE_DRI)
	 fprintf(stderr, "FRONT_CLIPRECTS, %d rects\n", 
		 dPriv->numClipRects);

      imesa->numClipRects = dPriv->numClipRects;
      imesa->pClipRects = dPriv->pClipRects;
      imesa->drawX = dPriv->x;
      imesa->drawY = dPriv->y;
   } else {
      if (I810_DEBUG & DEBUG_VERBOSE_DRI)
	 fprintf(stderr, "BACK_RECTS, %d rects\n", 
		 dPriv->numBackClipRects);

      imesa->numClipRects = dPriv->numBackClipRects;
      imesa->pClipRects = dPriv->pBackClipRects;
      imesa->drawX = dPriv->backX;
      imesa->drawY = dPriv->backY;
   }

   i810EmitDrawingRectangle( imesa );
   imesa->dirty |= I810_UPLOAD_CLIPRECTS;

   if (I810_DEBUG & DEBUG_VERBOSE_DRI)
      for (i = 0 ; i < imesa->numClipRects ; i++) 
	 fprintf(stderr, "cliprect %d: %d,%d - %d,%d\n",
		 i,
		 imesa->pClipRects[i].x1,
		 imesa->pClipRects[i].y1,
		 imesa->pClipRects[i].x2,
		 imesa->pClipRects[i].y2);
}


static void i810XMesaWindowMoved( i810ContextPtr imesa ) 
{
   if (0)
      fprintf(stderr, "i810XMesaWindowMoved\n\n");

   switch (imesa->glCtx->Color.DriverDrawBuffer) {
   case GL_FRONT_LEFT:
      i810XMesaSetFrontClipRects( imesa );
      break;
   case GL_BACK_LEFT:
      i810XMesaSetBackClipRects( imesa );
      break;
   default:
      /*fprintf(stderr, "fallback buffer\n");*/
      break;
   }
}


GLboolean XMesaUnbindContext(__DRIcontextPrivate *driContextPriv)
{
   i810ContextPtr i810 = (i810ContextPtr) driContextPriv->driverPrivate;
   if (i810)
      i810->dirty = ~0;

   return GL_TRUE;
}

GLboolean
XMesaOpenFullScreen(__DRIcontextPrivate *driContextPriv)
{
    return GL_TRUE;
}

GLboolean
XMesaCloseFullScreen(__DRIcontextPrivate *driContextPriv)
{
    return GL_TRUE;
}


GLboolean XMesaMakeCurrent(__DRIcontextPrivate *driContextPriv,
			   __DRIdrawablePrivate *driDrawPriv,
			   __DRIdrawablePrivate *driReadPriv)
{
   if (driContextPriv) {
      i810Ctx = (i810ContextPtr) driContextPriv->driverPrivate;
      
      gl_make_current2(i810Ctx->glCtx, driDrawPriv->mesaBuffer, 
		       driReadPriv->mesaBuffer);
      
      
      i810Ctx->driDrawable = driDrawPriv;
      i810Ctx->dirty = ~0;
      
      i810XMesaWindowMoved( i810Ctx );
      
      if (!i810Ctx->glCtx->Viewport.Width)
	 gl_Viewport(i810Ctx->glCtx, 0, 0, driDrawPriv->w, driDrawPriv->h);
   }
   else 
   {
      gl_make_current(0,0);
      i810Ctx = NULL;
   }
   return GL_TRUE;
}


void i810GetLock( i810ContextPtr imesa, GLuint flags ) 
{
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   __DRIscreenPrivate *sPriv = imesa->driScreen;
   drm_i810_sarea_t *sarea = imesa->sarea;
   int me = imesa->hHWContext;
   int stamp = dPriv->lastStamp; 


   if (0) fprintf(stderr, ".\n");

   /* We know there has been contention.
    */
   drmGetLock(imesa->driFd, imesa->hHWContext, flags);	


   /* Note contention for throttling hint
    */
   imesa->any_contend = 1;

   /* If the window moved, may need to set a new cliprect now.
    *
    * NOTE: This releases and regains the hw lock, so all state
    * checking must be done *after* this call:
    */
   XMESA_VALIDATE_DRAWABLE_INFO(imesa->display, sPriv, dPriv);		


   if (0)
   fprintf(stderr, "i810GetLock, last enque: %d last dispatch: %d\n",
	   sarea->last_enqueue, 
	   sarea->last_dispatch);

   /* If we lost context, need to dump all registers to hardware.
    * Note that we don't care about 2d contexts, even if they perform
    * accelerated commands, so the DRI locking in the X server is even
    * more broken than usual.
    */
   if (sarea->ctxOwner != me) {
      imesa->dirty |= (I810_UPLOAD_CTX |
		       I810_UPLOAD_CLIPRECTS |
		       I810_UPLOAD_BUFFERS |
		       I810_UPLOAD_TEX0 |
		       I810_UPLOAD_TEX1);
      sarea->ctxOwner = me;
   }

   /* Shared texture managment - if another client has played with
    * texture space, figure out which if any of our textures have been
    * ejected, and update our global LRU.
    */
   if (sarea->texAge != imesa->texAge) {
      int sz = 1 << (imesa->i810Screen->logTextureGranularity);
      int idx, nr = 0;

      /* Have to go right round from the back to ensure stuff ends up
       * LRU in our local list...
       */
      for (idx = sarea->texList[I810_NR_TEX_REGIONS].prev ; 
	   idx != I810_NR_TEX_REGIONS && nr < I810_NR_TEX_REGIONS ; 
	   idx = sarea->texList[idx].prev, nr++)
      {
	 if (sarea->texList[idx].age > imesa->texAge)
	    i810TexturesGone(imesa, idx * sz, sz, sarea->texList[idx].in_use);
      }

      if (nr == I810_NR_TEX_REGIONS) {
	 i810TexturesGone(imesa, 0, imesa->i810Screen->textureSize, 0);
	 i810ResetGlobalLRU( imesa );
      }

      if (0) fprintf(stderr, "imesa %d sarea %d\n", imesa->texAge, sarea->texAge);
      imesa->dirty |= I810_UPLOAD_TEX0IMAGE;
      imesa->dirty |= I810_UPLOAD_TEX1IMAGE;
      imesa->texAge = sarea->texAge;
   }

   
   if (dPriv->lastStamp != stamp)
      i810XMesaWindowMoved( imesa );

   
   sarea->last_quiescent = -1;	/* just kill it for now */
}


#endif
