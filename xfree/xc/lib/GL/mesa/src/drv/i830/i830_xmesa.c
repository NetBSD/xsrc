/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_xmesa.c,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>
#include <stdio.h>

#include "context.h"
#include "vbxform.h"
#include "matrix.h"
#include "simple_list.h"

#include "i830_drv.h"
#include "i830_tris.h"
#include "i830_ioctl.h"

#include "i830_dri.h"



#ifndef I830_DEBUG
int I830_DEBUG = (0
		  | DEBUG_VERBOSE_TRACE
/*		  | DEBUG_VERBOSE_STATE */
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


static i830ContextPtr      i830Ctx = 0;


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


static int i830_malloc_proxy_buf(drmBufMapPtr buffers)
{
   char *buffer;
   drmBufPtr buf;
   int i;
   
   buffer = Xmalloc(I830_DMA_BUF_SZ);
   if(buffer == NULL) return -1;
   for(i = 0; i < I830_DMA_BUF_NR; i++) {
      buf = &(buffers->list[i]);
      buf->address = (drmAddress)buffer;
   }
   return 0;
}

static drmBufMapPtr i830_create_empty_buffers(void)
{
   drmBufMapPtr retval;
   
   retval = (drmBufMapPtr)Xmalloc(sizeof(drmBufMap));
   if(retval == NULL) return NULL;
   memset(retval, 0, sizeof(drmBufMap));
   retval->list = (drmBufPtr)Xmalloc(sizeof(drmBuf) * I830_DMA_BUF_NR);
   if(retval->list == NULL) {
      Xfree(retval);
      return NULL;
   }
   memset(retval->list, 0, sizeof(drmBuf) * I830_DMA_BUF_NR);
   /*
   fprintf(stderr, "retval : %p, retval->list : %p\n", retval, retval->list);
   */
   return retval;
}

GLboolean XMesaInitDriver(__DRIscreenPrivate *sPriv)
{
   i830ScreenPrivate *i830Screen;
   I830DRIPtr         gDRIPriv = (I830DRIPtr)sPriv->pDevPriv;

   /* Check the DRI version */
   {
      int major, minor, patch;
      if (XF86DRIQueryVersion(sPriv->display, &major, &minor, &patch)) {
         if (major != 4 || minor < 0) {
            char msg[1000];
            sprintf(msg, "i830 DRI driver expected DRI version 4.0.x but got version %d.%d.%d", major, minor, patch);
            __driMesaMessage(msg);
            return GL_FALSE;
         }
      }
   }

   /* Check that the DDX driver version is compatible */
   if (sPriv->ddxMajor != 1 ||
       sPriv->ddxMinor < 0) {
      char msg[1000];
      sprintf(msg, "i830 DRI driver expected DDX driver version 1.0.x but got version %d.%d.%d", sPriv->ddxMajor, sPriv->ddxMinor, sPriv->ddxPatch);
      __driMesaMessage(msg);
      return GL_FALSE;
   }

   /* Check that the DRM driver version is compatible */
   if (sPriv->drmMajor != 1 ||
       sPriv->drmMinor < 2) {
      char msg[1000];
      sprintf(msg, "i830 DRI driver expected DRM driver version 1.2.x but got version %d.%d.%d", sPriv->drmMajor, sPriv->drmMinor, sPriv->drmPatch);
      __driMesaMessage(msg);
      return GL_FALSE;
   }

   /* Allocate the private area */
   i830Screen = (i830ScreenPrivate *)Xmalloc(sizeof(i830ScreenPrivate));
   if (!i830Screen)
      return GL_FALSE;

   i830Screen->driScrnPriv = sPriv;
   sPriv->private = (void *)i830Screen;

   i830Screen->deviceID=gDRIPriv->deviceID;
   i830Screen->width=gDRIPriv->width;
   i830Screen->height=gDRIPriv->height;
   i830Screen->mem=gDRIPriv->mem;
   i830Screen->cpp=gDRIPriv->cpp;
   i830Screen->fbStride=gDRIPriv->fbStride;
   i830Screen->fbOffset=gDRIPriv->fbOffset;

   switch (gDRIPriv->bitsPerPixel) {
   case 15: i830Screen->fbFormat = DV_PF_555; break;
   case 16: i830Screen->fbFormat = DV_PF_565; break;
   case 32: i830Screen->fbFormat = DV_PF_8888; break;
   }

   i830Screen->backOffset=gDRIPriv->backOffset; 
   i830Screen->depthOffset=gDRIPriv->depthOffset;
   i830Screen->backPitch = gDRIPriv->auxPitch;
   i830Screen->backPitchBits = gDRIPriv->auxPitchBits;
   i830Screen->textureOffset=gDRIPriv->textureOffset;
   i830Screen->textureSize=gDRIPriv->textureSize;
   i830Screen->logTextureGranularity = gDRIPriv->logTextureGranularity;
   i830Screen->sarea_priv_offset = gDRIPriv->sarea_priv_offset;

   if (0)
      fprintf(stderr, "Tex heap size %x, granularity %x bytes\n",
	      i830Screen->textureSize, 1<<(i830Screen->logTextureGranularity));

   i830Screen->bufs = i830_create_empty_buffers();
   if(i830Screen->bufs == NULL)
   {
      Xfree(i830Screen);
      return GL_FALSE;
   }
   
   /* Check if you need to create a fake buffer */
   if(i830_check_copy(sPriv->fd) == 1)
   {
      i830_malloc_proxy_buf(i830Screen->bufs);
      i830Screen->use_copy_buf = 1;
   }
   else
   {
      i830Screen->use_copy_buf = 0;
   }
    
   i830Screen->back.handle = gDRIPriv->backbuffer;
   i830Screen->back.size = gDRIPriv->backbufferSize;

   if (drmMap(sPriv->fd, 
	      i830Screen->back.handle, 
	      i830Screen->back.size, 
	      (drmAddress *)&i830Screen->back.map) != 0) 
   {
      Xfree(i830Screen);
      sPriv->private = NULL;
      return GL_FALSE;
   }

   i830Screen->depth.handle = gDRIPriv->depthbuffer;
   i830Screen->depth.size = gDRIPriv->depthbufferSize;

   if (drmMap(sPriv->fd, 
	      i830Screen->depth.handle, 
	      i830Screen->depth.size, 
	      (drmAddress *)&i830Screen->depth.map) != 0) 
   {
      Xfree(i830Screen);
      drmUnmap(i830Screen->back.map, i830Screen->back.size);
      sPriv->private = NULL;
      return GL_FALSE;
   }

   i830Screen->tex.handle = gDRIPriv->textures;
   i830Screen->tex.size = gDRIPriv->textureSize;

   if (drmMap(sPriv->fd, 
	      i830Screen->tex.handle, 
	      i830Screen->tex.size, 
	      (drmAddress *)&i830Screen->tex.map) != 0) 
   {
      Xfree(i830Screen);
      drmUnmap(i830Screen->back.map, i830Screen->back.size);
      drmUnmap(i830Screen->depth.map, i830Screen->depth.size);
      sPriv->private = NULL;
      return GL_FALSE;
   }


   i830DDFastPathInit();
   i830DDTrifuncInit();
   i830DDSetupInit();

   return GL_TRUE;
}

/* Accessed by dlsym from dri_mesa_init.c
 */
void XMesaResetDriver(__DRIscreenPrivate *sPriv)
{
   i830ScreenPrivate *i830Screen = (i830ScreenPrivate *)sPriv->private;

   /* Need to unmap all the bufs and maps here:
    */
   drmUnmap(i830Screen->back.map, i830Screen->back.size);
   drmUnmap(i830Screen->depth.map, i830Screen->depth.size);
   drmUnmap(i830Screen->tex.map, i830Screen->tex.size);

   Xfree(i830Screen);
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
   i830ContextPtr imesa;
   __DRIscreenPrivate *sPriv = driContextPriv->driScreenPriv;
   i830ScreenPrivate *i830Screen = (i830ScreenPrivate *)sPriv->private;
   I830SAREAPtr saPriv=(I830SAREAPtr)(((char*)sPriv->pSAREA)+
				    i830Screen->sarea_priv_offset);

   imesa = (i830ContextPtr)Xcalloc(sizeof(i830Context), 1);
   if (!imesa) {
      return GL_FALSE;
   }


   /* Set the maximum texture size small enough that we can guarentee
    * that both texture units can bind a maximal texture and have them
    * in memory at once.
    */
   if (i830Screen->textureSize < 2*1024*1024) {
      ctx->Const.MaxTextureLevels = 9;
      ctx->Const.MaxTextureSize = 1<<8;
   } else if (i830Screen->textureSize < 8*1024*1024) {
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

   imesa->i830Screen = i830Screen;
   imesa->driScreen = sPriv;
   imesa->sarea = saPriv;
   imesa->glBuffer = NULL;

   imesa->texHeap = mmInit( 0, i830Screen->textureSize );


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
   
   i830DDExtensionsInit( ctx );

   imesa->hw_stencil = mesaVis->StencilBits && mesaVis->DepthBits == 24;
   
   switch(mesaVis->DepthBits) {
   case 16:
      imesa->depth_scale = 1.0/0x10000;
      imesa->depth_clear_mask = ~0;
      imesa->ClearDepth = 0xffff;
      break;
   case 24:
      imesa->depth_scale = 1.0/0x1000000;
      imesa->depth_clear_mask = 0x00ffffff;
      imesa->stencil_clear_mask = 0xff000000;
      imesa->ClearDepth = 0x00ffffff;
      break;
   case 32: /* Not supported I don't believe */
	default:
#define DUMMY()		/* keep gcc happy */
	  DUMMY();
   }

   /* Completely disable stenciling for now, there are some serious issues
    * with stencil.
    */
#if 1
   imesa->hw_stencil = 0;
#endif

   i830DDInitStateFuncs( ctx );
   i830DDInitTextureFuncs( ctx );
   i830DDInitSpanFuncs( ctx );
   i830DDInitDriverFuncs( ctx );
   i830DDInitIoctlFuncs( ctx );

   ctx->Driver.TriangleCaps = (DD_TRI_CULL|
			       DD_TRI_LIGHT_TWOSIDE|
			       DD_TRI_STIPPLE|
			       DD_TRI_OFFSET);

   /* Ask mesa to clip fog coordinates for us.
    */
   ctx->TriangleCaps |= DD_CLIP_FOG_COORD;

   if (ctx->VB) 
      i830DDRegisterVB( ctx->VB );

   if (ctx->NrPipelineStages)
      ctx->NrPipelineStages =
	 i830DDRegisterPipelineStages(ctx->PipelineStage,
				      ctx->PipelineStage,
				      ctx->NrPipelineStages);
   
   i830DDInitState( imesa );

   driContextPriv->driverPrivate = (void *) imesa;

   return GL_TRUE;
}

void XMesaDestroyContext(__DRIcontextPrivate *driContextPriv)
{
   i830ContextPtr imesa = (i830ContextPtr) driContextPriv->driverPrivate;

   if (imesa) {
      i830TextureObjectPtr next_t, t;

      foreach_s (t, next_t, &(imesa->TexObjList))
	 i830DestroyTexObj(imesa, t);

      foreach_s (t, next_t, &(imesa->SwappedOut))
	 i830DestroyTexObj(imesa, t);

      Xfree(imesa);
   }
}

GLframebuffer *XMesaCreateWindowBuffer( Display *dpy,
                                        __DRIscreenPrivate *driScrnPriv,
                                        __DRIdrawablePrivate *driDrawPriv,
                                        GLvisual *mesaVis)
{
#if 0
   GLboolean swStencil = mesaVis->StencilBits > 0 && mesaVis->DepthBits != 24;
#else
   GLboolean swStencil = mesaVis->StencilBits > 0;
#endif
   return gl_create_framebuffer(mesaVis,
                                GL_FALSE,
				swStencil,
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
   i830ContextPtr imesa = i830Ctx; 

   if( !driDrawPriv->mesaBuffer->Visual->DBflag ) return;

   FLUSH_VB( imesa->glCtx, "swap buffers" );
   i830SwapBuffers(imesa);
}



void i830XMesaSetFrontClipRects( i830ContextPtr imesa )
{
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;

   imesa->numClipRects = dPriv->numClipRects;
   imesa->pClipRects = dPriv->pClipRects;
   imesa->dirty |= I830_UPLOAD_CLIPRECTS;
   imesa->drawX = dPriv->x;
   imesa->drawY = dPriv->y;

   i830EmitDrawingRectangle( imesa );
}


void i830XMesaSetBackClipRects( i830ContextPtr imesa )
{
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   int i;

   if (dPriv->numBackClipRects == 0) 
   {
      if (I830_DEBUG & DEBUG_VERBOSE_DRI)
	 fprintf(stderr, "FRONT_CLIPRECTS, %d rects\n", 
		 dPriv->numClipRects);

      imesa->numClipRects = dPriv->numClipRects;
      imesa->pClipRects = dPriv->pClipRects;
      imesa->drawX = dPriv->x;
      imesa->drawY = dPriv->y;
   } else {
      if (I830_DEBUG & DEBUG_VERBOSE_DRI)
	 fprintf(stderr, "BACK_RECTS, %d rects\n", 
		 dPriv->numBackClipRects);

      imesa->numClipRects = dPriv->numBackClipRects;
      imesa->pClipRects = dPriv->pBackClipRects;
      imesa->drawX = dPriv->backX;
      imesa->drawY = dPriv->backY;
   }

   i830EmitDrawingRectangle( imesa );
   imesa->dirty |= I830_UPLOAD_CLIPRECTS;

   if (I830_DEBUG & DEBUG_VERBOSE_DRI)
      for (i = 0 ; i < imesa->numClipRects ; i++) 
	 fprintf(stderr, "cliprect %d: %d,%d - %d,%d\n",
		 i,
		 imesa->pClipRects[i].x1,
		 imesa->pClipRects[i].y1,
		 imesa->pClipRects[i].x2,
		 imesa->pClipRects[i].y2);
}


static void i830XMesaWindowMoved( i830ContextPtr imesa ) 
{
   if (0)
      fprintf(stderr, "i830XMesaWindowMoved\n\n");

   switch (imesa->glCtx->Color.DriverDrawBuffer) {
   case GL_FRONT_LEFT:
      i830XMesaSetFrontClipRects( imesa );
      break;
   case GL_BACK_LEFT:
      i830XMesaSetBackClipRects( imesa );
      break;
   default:
      /*fprintf(stderr, "fallback buffer\n");*/
      break;
   }
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

GLboolean XMesaUnbindContext(__DRIcontextPrivate *driContextPriv)
{
   i830ContextPtr i830 = (i830ContextPtr) driContextPriv->driverPrivate;
   if (i830)
      i830->dirty = ~0;

   return GL_TRUE;
}


GLboolean XMesaMakeCurrent(__DRIcontextPrivate *driContextPriv,
			   __DRIdrawablePrivate *driDrawPriv,
			   __DRIdrawablePrivate *driReadPriv)
{
   if (driContextPriv) {
      i830Ctx = (i830ContextPtr) driContextPriv->driverPrivate;
      
      gl_make_current2(i830Ctx->glCtx, driDrawPriv->mesaBuffer, 
		       driReadPriv->mesaBuffer);
      
      
      i830Ctx->driDrawable = driDrawPriv;
      i830Ctx->dirty = ~0;
      
      i830XMesaWindowMoved( i830Ctx );
      
      if (!i830Ctx->glCtx->Viewport.Width)
	 gl_Viewport(i830Ctx->glCtx, 0, 0, driDrawPriv->w, driDrawPriv->h);
   }
   else 
   {
      gl_make_current(0,0);
      i830Ctx = NULL;
   }
   return GL_TRUE;
}


void i830GetLock( i830ContextPtr imesa, GLuint flags ) 
{
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   __DRIscreenPrivate *sPriv = imesa->driScreen;
   I830SAREAPtr sarea = imesa->sarea;
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
   fprintf(stderr, "i830GetLock, last enque: %d last dispatch: %d\n",
	   sarea->last_enqueue, 
	   sarea->last_dispatch);

   /* If we lost context, need to dump all registers to hardware.
    * Note that we don't care about 2d contexts, even if they perform
    * accelerated commands, so the DRI locking in the X server is even
    * more broken than usual.
    */
   if (sarea->ctxOwner != me) {
      imesa->dirty |= (I830_UPLOAD_CTX |
		       I830_UPLOAD_CLIPRECTS |
		       I830_UPLOAD_BUFFERS |
		       I830_UPLOAD_TEX0 |
		       I830_UPLOAD_TEX1);

      if(imesa->TexBlendWordsUsed[0]) imesa->dirty |= I830_UPLOAD_TEXBLEND0;
      if(imesa->TexBlendWordsUsed[1]) imesa->dirty |= I830_UPLOAD_TEXBLEND1;

      sarea->ctxOwner = me;
   }

   /* Shared texture managment - if another client has played with
    * texture space, figure out which if any of our textures have been
    * ejected, and update our global LRU.
    */
   if (sarea->texAge != imesa->texAge) {
      int sz = 1 << (imesa->i830Screen->logTextureGranularity);
      int idx, nr = 0;

      /* Have to go right round from the back to ensure stuff ends up
       * LRU in our local list...
       */
      for (idx = sarea->texList[I830_NR_TEX_REGIONS].prev ; 
	   idx != I830_NR_TEX_REGIONS && nr < I830_NR_TEX_REGIONS ; 
	   idx = sarea->texList[idx].prev, nr++)
      {
	 if (sarea->texList[idx].age > imesa->texAge)
	    i830TexturesGone(imesa, idx * sz, sz, sarea->texList[idx].in_use);
      }

      if (nr == I830_NR_TEX_REGIONS) {
	 i830TexturesGone(imesa, 0, imesa->i830Screen->textureSize, 0);
	 i830ResetGlobalLRU( imesa );
      }

      if (0) fprintf(stderr, "imesa %d sarea %d\n", imesa->texAge, sarea->texAge);
      imesa->dirty |= I830_UPLOAD_TEX0_IMAGE;
      imesa->dirty |= I830_UPLOAD_TEX1_IMAGE;
      imesa->texAge = sarea->texAge;
   }

   
   if (dPriv->lastStamp != stamp)
      i830XMesaWindowMoved( imesa );

   
   sarea->last_quiescent = -1;	/* just kill it for now */
}


#endif
