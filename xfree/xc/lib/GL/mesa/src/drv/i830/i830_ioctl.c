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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_ioctl.c,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#include <stdio.h>
#include <unistd.h>

#include "types.h"
#include "pb.h"
#include "dd.h"

#include "mm.h"
#include "i830_drv.h"
#include "i830_ioctl.h"

#include "drm.h"
#include <sys/ioctl.h>

drmBufPtr i830_get_buffer_ioctl( i830ContextPtr imesa )
{
   drm_i830_dma_t dma;
   drmBufPtr buf;
   int retcode;
   
   if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr,  "Getting dma buffer\n");

   while (1) {
      retcode = ioctl(imesa->driFd, DRM_IOCTL_I830_GETBUF, &dma);

      if (dma.granted == 1 && retcode == 0) 
	 break;

      if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, "Retcode : %d, granted : %d\n", retcode, dma.granted);

      ioctl(imesa->driFd, DRM_IOCTL_I830_FLUSH);
   }

   if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, 
	      "imesa->i830Screen->bufs->list : %p, "
	      "dma.request_idx : %d\n", 
	      imesa->i830Screen->bufs->list, dma.request_idx);

   buf = &(imesa->i830Screen->bufs->list[dma.request_idx]);
   buf->idx = dma.request_idx;
   buf->used = 4;		/* leave room for instruction header */
   buf->total = dma.request_size;

   if(imesa->i830Screen->use_copy_buf != 1) 
       buf->address = (drmAddress)dma.virtual;
   return buf;
}


static void i830ClearDrawQuad(i830ContextPtr imesa, float left, 
				 float right,
				 float bottom, float top, GLubyte red,
				 GLubyte green, GLubyte blue, GLubyte alpha)
{
    GLuint *vb = i830AllocDwordsInlineLocked( imesa, 32 );
    i830Vertex tmp;
    int i;

    /* PRIM3D_TRIFAN */

    /* initial vertex, left bottom */
    tmp.v.x = left;
    tmp.v.y = bottom;
    tmp.v.z = 1.0;
    tmp.v.oow = 1.0;
    tmp.v.color.red = red;
    tmp.v.color.green = green;
    tmp.v.color.blue = blue;
    tmp.v.color.alpha = alpha;
    tmp.v.specular.red = 0;
    tmp.v.specular.green = 0;
    tmp.v.specular.blue = 0;
    tmp.v.specular.alpha = 0;
    tmp.v.tu0 = 0.0f;
    tmp.v.tv0 = 0.0f;
    for (i = 0 ; i < 8 ; i++)
        vb[i] = tmp.ui[i];

    /* right bottom */
    vb += 8;
    tmp.v.x = right;
    for (i = 0 ; i < 8 ; i++)
        vb[i] = tmp.ui[i];

    /* right top */
    vb += 8;
    tmp.v.y = top;
    for (i = 0 ; i < 8 ; i++)
        vb[i] = tmp.ui[i];

    /* left top */
    vb += 8;
    tmp.v.x = left;
    for (i = 0 ; i < 8 ; i++)
        vb[i] = tmp.ui[i];
}

static void i830ClearWithTris(GLcontext *ctx, GLbitfield mask,
				 GLboolean all,
				 GLint cx, GLint cy, GLint cw, GLint ch)
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   i830ScreenPrivate *i830Screen = imesa->i830Screen;
   I830SAREAPtr sarea = imesa->sarea;
   GLuint old_vertex_prim;
   GLuint old_dirty;
   int x0, y0, x1, y1;

   if(I830_DEBUG&DEBUG_VERBOSE_IOCTL)
     fprintf(stderr, "Clearing with triangles\n");

   old_dirty = imesa->dirty & ~I830_UPLOAD_CLIPRECTS;
   /* Discard all the dirty flags except the cliprect one, reset later */
   imesa->dirty &= I830_UPLOAD_CLIPRECTS;

   if(!all) {
      x0 = cx;
      y0 = cy;
      x1 = x0 + cw;
      y1 = y0 + ch;
   } else {
      x0 = 0;
      y0 = 0;
      x1 = x0 + dPriv->w;
      y1 = y0 + dPriv->h;
   }

   /* Clip to Screen */
   if (x0 < 0) x0 = 0;
   if (y0 < 0) y0 = 0;
   if (x1 > i830Screen->width-1) x1 = i830Screen->width-1;
   if (y1 > i830Screen->height-1) y1 = i830Screen->height-1;

   LOCK_HARDWARE(imesa);
   memcpy(sarea->ContextState,
	  imesa->Init_Setup,
	  sizeof(imesa->Setup) );
   memcpy(sarea->BufferState,
	  imesa->BufferSetup,
	  sizeof(imesa->BufferSetup) );

   old_vertex_prim = imesa->vertex_prim;
   imesa->vertex_prim = PRIM3D_TRIFAN;

   if(mask & DD_FRONT_LEFT_BIT) {
      GLuint tmp = sarea->ContextState[I830_CTXREG_ENABLES_2];

      sarea->dirty |= (I830_UPLOAD_CTX | I830_UPLOAD_BUFFERS |
		       I830_UPLOAD_TEXBLEND0);

      sarea->TexBlendState[0][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				    TEXPIPE_COLOR |
				    ENABLE_TEXOUTPUT_WRT_SEL |
				    TEXOP_OUTPUT_CURRENT |
				    DISABLE_TEX_CNTRL_STAGE |
				    TEXOP_SCALE_1X |
				    TEXOP_MODIFY_PARMS |
				    TEXOP_LAST_STAGE |
				    TEXBLENDOP_ARG1);
      sarea->TexBlendState[0][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				    TEXPIPE_ALPHA |
				    ENABLE_TEXOUTPUT_WRT_SEL |
				    TEXOP_OUTPUT_CURRENT |
				    TEXOP_SCALE_1X |
				    TEXOP_MODIFY_PARMS |
				    TEXBLENDOP_ARG1);
      sarea->TexBlendState[0][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				    TEXPIPE_COLOR |
				    TEXBLEND_ARG1 |
				    TEXBLENDARG_MODIFY_PARMS |
				    TEXBLENDARG_CURRENT);
      sarea->TexBlendState[0][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				    TEXPIPE_ALPHA |
				    TEXBLEND_ARG1 |
				    TEXBLENDARG_MODIFY_PARMS |
				    TEXBLENDARG_CURRENT);
      sarea->TexBlendStateWordsUsed[0] = 4;

      tmp &= ~(ENABLE_STENCIL_WRITE | ENABLE_DEPTH_WRITE);
      tmp |= (DISABLE_STENCIL_WRITE | 
	      DISABLE_DEPTH_WRITE |
	      (imesa->mask_red << WRITEMASK_RED_SHIFT) |
	      (imesa->mask_green << WRITEMASK_GREEN_SHIFT) |
	      (imesa->mask_blue << WRITEMASK_BLUE_SHIFT) |
	      (imesa->mask_alpha << WRITEMASK_ALPHA_SHIFT));
      sarea->ContextState[I830_CTXREG_ENABLES_2] = tmp;

      if(0)
	fprintf(stderr, "fcdq : r_mask(%d) g_mask(%d) b_mask(%d) a_mask(%d)\n",
		imesa->mask_red, imesa->mask_green, imesa->mask_blue,
		imesa->mask_alpha);

      sarea->BufferState[I830_DESTREG_CBUFADDR] = i830Screen->fbOffset;

      if(0)
	fprintf(stderr, "fcdq : x0(%d) x1(%d) y0(%d) y1(%d)\n"
		"r(0x%x) g(0x%x) b(0x%x) a(0x%x)\n",
		x0, x1, y0, y1, imesa->clear_red, imesa->clear_green,
		imesa->clear_blue, imesa->clear_alpha);

      i830ClearDrawQuad(imesa, (float)x0, (float)x1, (float)y0, (float)y1,
			   imesa->clear_red, imesa->clear_green,
		   imesa->clear_blue, imesa->clear_alpha);
      i830FlushVerticesLocked(imesa);
   }

   if(mask & DD_BACK_LEFT_BIT) {
      GLuint tmp = sarea->ContextState[I830_CTXREG_ENABLES_2];

      sarea->dirty |= (I830_UPLOAD_CTX | I830_UPLOAD_BUFFERS |
		       I830_UPLOAD_TEXBLEND0);

      sarea->TexBlendState[0][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				    TEXPIPE_COLOR |
				    ENABLE_TEXOUTPUT_WRT_SEL |
				    TEXOP_OUTPUT_CURRENT |
				    DISABLE_TEX_CNTRL_STAGE |
				    TEXOP_SCALE_1X |
				    TEXOP_MODIFY_PARMS |
				    TEXOP_LAST_STAGE |
				    TEXBLENDOP_ARG1);
      sarea->TexBlendState[0][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				    TEXPIPE_ALPHA |
				    ENABLE_TEXOUTPUT_WRT_SEL |
				    TEXOP_OUTPUT_CURRENT |
				    TEXOP_SCALE_1X |
				    TEXOP_MODIFY_PARMS |
				    TEXBLENDOP_ARG1);
      sarea->TexBlendState[0][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				    TEXPIPE_COLOR |
				    TEXBLEND_ARG1 |
				    TEXBLENDARG_MODIFY_PARMS |
				    TEXBLENDARG_CURRENT);
      sarea->TexBlendState[0][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				    TEXPIPE_ALPHA |
				    TEXBLEND_ARG2 |
				    TEXBLENDARG_MODIFY_PARMS |
				    TEXBLENDARG_CURRENT);
      sarea->TexBlendStateWordsUsed[0] = 4;

      tmp &= ~(ENABLE_STENCIL_WRITE | ENABLE_DEPTH_WRITE);
      tmp |= (DISABLE_STENCIL_WRITE | 
	      DISABLE_DEPTH_WRITE |
	      (imesa->mask_red << WRITEMASK_RED_SHIFT) |
	      (imesa->mask_green << WRITEMASK_GREEN_SHIFT) |
	      (imesa->mask_blue << WRITEMASK_BLUE_SHIFT) |
	      (imesa->mask_alpha << WRITEMASK_ALPHA_SHIFT));

      if(0)
	fprintf(stderr, "bcdq : r_mask(%d) g_mask(%d) b_mask(%d) a_mask(%d)\n",
		imesa->mask_red, imesa->mask_green, imesa->mask_blue,
		imesa->mask_alpha);

      sarea->ContextState[I830_CTXREG_ENABLES_2] = tmp;

      sarea->BufferState[I830_DESTREG_CBUFADDR] = i830Screen->backOffset;

      if(0)
	fprintf(stderr, "bcdq : x0(%d) x1(%d) y0(%d) y1(%d)\n"
		"r(0x%x) g(0x%x) b(0x%x) a(0x%x)\n",
		x0, x1, y0, y1, imesa->clear_red, imesa->clear_green,
		imesa->clear_blue, imesa->clear_alpha);

      i830ClearDrawQuad(imesa, (float)x0, (float)x1, (float)y0, (float)y1,
		      imesa->clear_red, imesa->clear_green,
		      imesa->clear_blue, imesa->clear_alpha);
      i830FlushVerticesLocked(imesa);
   }

   if(mask & DD_STENCIL_BIT) {
      GLuint s_mask = ctx->Stencil.WriteMask;

      sarea->dirty |= (I830_UPLOAD_CTX | I830_UPLOAD_BUFFERS |
		       I830_UPLOAD_TEXBLEND0);

      sarea->TexBlendState[0][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				    TEXPIPE_COLOR |
				    ENABLE_TEXOUTPUT_WRT_SEL |
				    TEXOP_OUTPUT_CURRENT |
				    DISABLE_TEX_CNTRL_STAGE |
				    TEXOP_SCALE_1X |
				    TEXOP_MODIFY_PARMS |
				    TEXOP_LAST_STAGE |
				    TEXBLENDOP_ARG1);
      sarea->TexBlendState[0][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
				    TEXPIPE_ALPHA |
				    ENABLE_TEXOUTPUT_WRT_SEL |
				    TEXOP_OUTPUT_CURRENT |
				    TEXOP_SCALE_1X |
				    TEXOP_MODIFY_PARMS |
				    TEXBLENDOP_ARG1);
      sarea->TexBlendState[0][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				    TEXPIPE_COLOR |
				    TEXBLEND_ARG1 |
				    TEXBLENDARG_MODIFY_PARMS |
				    TEXBLENDARG_CURRENT);
      sarea->TexBlendState[0][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
				    TEXPIPE_ALPHA |
				    TEXBLEND_ARG2 |
				    TEXBLENDARG_MODIFY_PARMS |
				    TEXBLENDARG_CURRENT);
      sarea->TexBlendStateWordsUsed[0] = 4;

      sarea->ContextState[I830_CTXREG_ENABLES_1] |= (ENABLE_STENCIL_TEST |
						     ENABLE_DEPTH_TEST);

      sarea->ContextState[I830_CTXREG_ENABLES_2] &= ~(ENABLE_STENCIL_WRITE |
						     ENABLE_DEPTH_WRITE |
						     ENABLE_COLOR_WRITE);

      sarea->ContextState[I830_CTXREG_ENABLES_2] |= (ENABLE_STENCIL_WRITE |
					    DISABLE_DEPTH_WRITE |
					    (1 << WRITEMASK_RED_SHIFT) |
					    (1 << WRITEMASK_GREEN_SHIFT) |
					    (1 << WRITEMASK_BLUE_SHIFT) |
					    (1 << WRITEMASK_ALPHA_SHIFT) |
					    ENABLE_COLOR_WRITE);

      sarea->ContextState[I830_CTXREG_STATE4] &= ~MODE4_ENABLE_STENCIL_MASK;
      sarea->ContextState[I830_CTXREG_STATE4] |= (ENABLE_STENCIL_TEST_MASK |
						 ENABLE_STENCIL_WRITE_MASK |
						 STENCIL_TEST_MASK(s_mask) |
						 STENCIL_WRITE_MASK(s_mask));

      sarea->ContextState[I830_CTXREG_STENCILTST] &= ~(STENCIL_OPS_MASK |
					       STENCIL_REF_VALUE_MASK |
					       ENABLE_STENCIL_TEST_FUNC_MASK);
      sarea->ContextState[I830_CTXREG_STENCILTST] |= (ENABLE_STENCIL_PARMS |
			      ENABLE_STENCIL_REF_VALUE |
			      ENABLE_STENCIL_TEST_FUNC |
			      STENCIL_FAIL_OP(STENCILOP_REPLACE) |
			      STENCIL_PASS_DEPTH_FAIL_OP(STENCILOP_REPLACE) |
			      STENCIL_PASS_DEPTH_PASS_OP(STENCILOP_REPLACE) |
			      STENCIL_REF_VALUE((ctx->Stencil.Clear & 0xff)) |
			      STENCIL_TEST_FUNC(COMPAREFUNC_ALWAYS));

      if(0) 
	fprintf(stderr, "Enables_1 (0x%x) Enables_2 (0x%x) StenTst (0x%x)\n"
		"Modes_4 (0x%x)\n",
		sarea->ContextState[I830_CTXREG_ENABLES_1],
		sarea->ContextState[I830_CTXREG_ENABLES_2],
		sarea->ContextState[I830_CTXREG_STENCILTST],
		sarea->ContextState[I830_CTXREG_STATE4]);

      sarea->BufferState[I830_DESTREG_CBUFADDR] = i830Screen->fbOffset;
      
      i830ClearDrawQuad(imesa, (float)x0, (float)x1, (float)y0, (float)y1,
			   255, 255, 255, 255);
      i830FlushVerticesLocked(imesa);
   }

   UNLOCK_HARDWARE(imesa);
   imesa->dirty = old_dirty;
   imesa->dirty |= (I830_UPLOAD_CTX |
		    I830_UPLOAD_BUFFERS |
		    I830_UPLOAD_TEXBLEND0);

   imesa->vertex_prim = old_vertex_prim;
}

GLbitfield i830Clear( GLcontext *ctx, GLbitfield mask, GLboolean all,
		      GLint cx, GLint cy, GLint cw, GLint ch )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   const GLuint colorMask = *((GLuint *) &ctx->Color.ColorMask);
   drm_i830_clear_t clear;
   GLbitfield tri_mask = 0;
   int i;

   FLUSH_BATCH( imesa );

   /* flip top to bottom */
   cy = dPriv->h-cy-ch;
   cx += imesa->drawX;
   cy += imesa->drawY;

   clear.flags = 0;
   clear.clear_color = imesa->ClearColor;
   clear.clear_depth = 0;
   clear.clear_colormask = 0;
   clear.clear_depthmask = 0;

   if (mask & DD_FRONT_LEFT_BIT) {
      if(colorMask == ~0) {
	 clear.flags |= I830_FRONT;
      } else {
	 tri_mask |= DD_FRONT_LEFT_BIT;
      }
      mask &= ~DD_FRONT_LEFT_BIT;
   }

   if (mask & DD_BACK_LEFT_BIT) {
      if(colorMask == ~0) {
	 clear.flags |= I830_BACK;
      } else {
	 tri_mask |= DD_BACK_LEFT_BIT;
      }
      mask &= ~DD_BACK_LEFT_BIT;
   }

   if (mask & DD_DEPTH_BIT) {
      clear.flags |= I830_DEPTH;
      clear.clear_depthmask = imesa->depth_clear_mask;
      clear.clear_depth = (GLuint)(ctx->Depth.Clear * imesa->ClearDepth);
      mask &= ~DD_DEPTH_BIT;
   }

   if((mask & DD_STENCIL_BIT) && imesa->hw_stencil) {
      if (ctx->Stencil.WriteMask != 0xff) {
	 tri_mask |= DD_STENCIL_BIT;
      } else {
	 clear.flags |= I830_DEPTH;
	 clear.clear_depthmask |= imesa->stencil_clear_mask;
	 clear.clear_depth |= ((ctx->Stencil.Clear<<24) & 
			       imesa->stencil_clear_mask);
      }
      mask &= ~DD_STENCIL_BIT;
   }

   /* First check for clears that need to happen with triangles */

   if(tri_mask) {
      i830ClearWithTris(ctx, tri_mask, all, cx, cy, cw, ch);
   }

   if (!clear.flags)
      return mask;

   LOCK_HARDWARE( imesa );

   if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "Clear, bufs %x nbox %d\n", 
	      (int)clear.flags, (int)imesa->numClipRects);

   for (i = 0 ; i < imesa->numClipRects ; ) 
   { 	 
      int nr = MIN2(i + I830_NR_SAREA_CLIPRECTS, imesa->numClipRects);
      XF86DRIClipRectRec *box = imesa->pClipRects;	 
      drm_clip_rect_t *b = (drm_clip_rect_t *)imesa->sarea->boxes;
      int n = 0;

      if (!all) {
	 for ( ; i < nr ; i++) {
	    GLint x = box[i].x1;
	    GLint y = box[i].y1;
	    GLint w = box[i].x2 - x;
	    GLint h = box[i].y2 - y;

	    if (x < cx) w -= cx - x, x = cx; 
	    if (y < cy) h -= cy - y, y = cy;
	    if (x + w > cx + cw) w = cx + cw - x;
	    if (y + h > cy + ch) h = cy + ch - y;
	    if (w <= 0) continue;
	    if (h <= 0) continue;

	    b->x1 = x;
	    b->y1 = y;
	    b->x2 = x + w;
	    b->y2 = y + h;
	    b++;
	    n++;
	 }
      } else {
	 for ( ; i < nr ; i++) {
	    *b++ = *(drm_clip_rect_t *)&box[i];
	    n++;
	 }
      }

      imesa->sarea->nbox = n;
      ioctl(imesa->driFd, DRM_IOCTL_I830_CLEAR, &clear);
   }

   UNLOCK_HARDWARE( imesa );
   imesa->dirty |= I830_UPLOAD_CLIPRECTS;

   return mask;
}

/*
 * Copy the back buffer to the front buffer. 
 */
void i830SwapBuffers( i830ContextPtr imesa ) 
{
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   XF86DRIClipRectPtr pbox;
   int nbox;
   int i;
   int tmp;

   FLUSH_BATCH( imesa );
   LOCK_HARDWARE( imesa );
   
   pbox = dPriv->pClipRects;
   nbox = dPriv->numClipRects;

   for (i = 0 ; i < nbox ; )
   {
      int nr = MIN2(i + I830_NR_SAREA_CLIPRECTS, dPriv->numClipRects);
      XF86DRIClipRectRec *b = (XF86DRIClipRectRec *)imesa->sarea->boxes;

      imesa->sarea->nbox = nr - i;

      for ( ; i < nr ; i++) 
	 *b++ = pbox[i];

      ioctl(imesa->driFd, DRM_IOCTL_I830_SWAP);
   }

   tmp = GET_ENQUEUE_AGE(imesa);
   UNLOCK_HARDWARE( imesa );

   if (GET_DISPATCH_AGE(imesa) < imesa->lastSwap)
      i830WaitAge(imesa, imesa->lastSwap);

   imesa->lastSwap = tmp;
   imesa->dirty |= I830_UPLOAD_CLIPRECTS;
}






/* This waits for *everybody* to finish rendering -- overkill.
 */
void i830DmaFinish( i830ContextPtr imesa  ) 
{
   FLUSH_BATCH( imesa );


   if (1 || imesa->sarea->last_quiescent != imesa->sarea->last_enqueue) {
     
      if (I830_DEBUG&DEBUG_VERBOSE_IOCTL) 
	 fprintf(stderr, "i830DmaFinish\n");

      LOCK_HARDWARE( imesa );
      i830RegetLockQuiescent( imesa );
      UNLOCK_HARDWARE( imesa );
      imesa->sarea->last_quiescent = imesa->sarea->last_enqueue;
   }
}


void i830RegetLockQuiescent( i830ContextPtr imesa  ) 
{
   if (1 || imesa->sarea->last_quiescent != imesa->sarea->last_enqueue) {
      if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, "i830RegetLockQuiescent\n");

      drmUnlock(imesa->driFd, imesa->hHWContext);
      i830GetLock( imesa, DRM_LOCK_QUIESCENT ); 
      imesa->sarea->last_quiescent = imesa->sarea->last_enqueue;
   }
}

void i830WaitAgeLocked( i830ContextPtr imesa, int age  ) 
{
   int i = 0;


   while (++i < 500000 && GET_DISPATCH_AGE(imesa) < age) {
      ioctl(imesa->driFd, DRM_IOCTL_I830_GETAGE);
   }

   if (GET_DISPATCH_AGE(imesa) < age) {
      if (0)
	 fprintf(stderr, "wait locked %d %d\n", age, GET_DISPATCH_AGE(imesa));
      ioctl(imesa->driFd, DRM_IOCTL_I830_FLUSH);
   }
}


void i830WaitAge( i830ContextPtr imesa, int age  ) 
{
   int i = 0;

   while (++i < 500000 && GET_DISPATCH_AGE(imesa) < age) {
      ioctl(imesa->driFd, DRM_IOCTL_I830_GETAGE);
   }

   if (GET_DISPATCH_AGE(imesa) >= age)
      return;

   i = 0;
   while (++i < 1000 && GET_DISPATCH_AGE(imesa) < age) {
      ioctl(imesa->driFd, DRM_IOCTL_I830_GETAGE);
      usleep(1000);
   }
   
   /* To be effective at letting other clients at the hardware,
    * particularly the X server which regularly needs quiescence to
    * touch the framebuffer, we really need to sleep *beyond* the
    * point where our last buffer clears the hardware.  
    */
   if (imesa->any_contend) {
      usleep(3000); 
   }

   imesa->any_contend = 0;

   if (GET_DISPATCH_AGE(imesa) < age) {
      LOCK_HARDWARE(imesa);
      if (GET_DISPATCH_AGE(imesa) < age) 
	 ioctl(imesa->driFd, DRM_IOCTL_I830_FLUSH);
      UNLOCK_HARDWARE(imesa);
   }
}

void i830FlushVertices( i830ContextPtr imesa ) 
{
   if (!imesa->vertex_dma_buffer) return;

   LOCK_HARDWARE( imesa );
   i830FlushVerticesLocked( imesa );
   UNLOCK_HARDWARE( imesa );
}

static void age_imesa( i830ContextPtr imesa, int age )
{
   if (imesa->CurrentTexObj[0]) imesa->CurrentTexObj[0]->age = age;
   if (imesa->CurrentTexObj[1]) imesa->CurrentTexObj[1]->age = age;
}

void i830FlushVerticesLocked( i830ContextPtr imesa )
{
   drm_clip_rect_t *pbox = (drm_clip_rect_t *)imesa->pClipRects;
   int nbox = imesa->numClipRects;
   drmBufPtr buffer = imesa->vertex_dma_buffer;
   drm_i830_vertex_t vertex;
   int i;

   if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "i830FlushVerticesLocked, buf->used %d\n", 
	      buffer->used);

   if (!buffer)
      return;

   if (imesa->dirty & ~I830_UPLOAD_CLIPRECTS)
      i830EmitHwStateLocked( imesa );

   if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "i830FlushVerticesLocked, used %d\n",
	      buffer->used);
   
   imesa->vertex_dma_buffer = 0;

   vertex.idx = buffer->idx;
   vertex.used = buffer->used;
   vertex.discard = 0;

   if (!nbox)
      vertex.used = 0;

   if (nbox > I830_NR_SAREA_CLIPRECTS)
      imesa->dirty |= I830_UPLOAD_CLIPRECTS;
   
   if(imesa->i830Screen->use_copy_buf == 1 && vertex.used) {
      drm_i830_copy_t copy;
      
      copy.idx = buffer->idx;
      copy.used = buffer->used;
      copy.address = buffer->address;
      ioctl(imesa->driFd, DRM_IOCTL_I830_COPY, &copy);
   }


   imesa->sarea->vertex_prim = imesa->vertex_prim;

   if (!nbox || !(imesa->dirty & I830_UPLOAD_CLIPRECTS)) 
   {
      if (nbox == 1) 
	 imesa->sarea->nbox = 0;
      else
	 imesa->sarea->nbox = nbox;

      if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, "DRM_IOCTL_I830_VERTEX CASE1 nbox %d used %d\n", 
		 nbox, vertex.used);

      vertex.discard = 1;
      ioctl(imesa->driFd, DRM_IOCTL_I830_VERTEX, &vertex);
      age_imesa(imesa, imesa->sarea->last_enqueue);
   }
   else 
   {
      for (i = 0 ; i < nbox ; )
      {
	 int nr = MIN2(i + I810_NR_SAREA_CLIPRECTS, nbox);
	 drm_clip_rect_t *b = (drm_clip_rect_t *)imesa->sarea->boxes;

	 imesa->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++, b++) {
	    *b++ = pbox[i];
	    if(0) fprintf(stderr, "x1: %d y1: %d x2: %d y2: %d\n",
			  pbox[i].x1,
			  pbox[i].y1,
			  pbox[i].x2,
			  pbox[i].y2);
	 }

	 /* Finished with the buffer?
	  */
	 if (nr == nbox) 
	    vertex.discard = 1;

     	 if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
	    fprintf(stderr, "DRM_IOCTL_I830_VERTEX nbox %d used %d\n", 
		    nbox, vertex.used);

	 ioctl(imesa->driFd, DRM_IOCTL_I830_VERTEX, &vertex);
	 age_imesa(imesa, imesa->sarea->last_enqueue);
      }
   }

   imesa->dirty = 0;
   if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "finished i830FlushVerticesLocked\n");
}


GLuint *i830AllocDwords( i830ContextPtr imesa, int dwords )
{
   GLuint *start;

   if (!imesa->vertex_dma_buffer) 
   {
      LOCK_HARDWARE(imesa);
      imesa->vertex_dma_buffer = i830_get_buffer_ioctl( imesa );
      UNLOCK_HARDWARE(imesa);
   } 
   else if (imesa->vertex_dma_buffer->used + dwords * 4 > 
	    imesa->vertex_dma_buffer->total) 
   {
      LOCK_HARDWARE(imesa);
      i830FlushVerticesLocked( imesa );
      imesa->vertex_dma_buffer = i830_get_buffer_ioctl( imesa );
      UNLOCK_HARDWARE(imesa);
   }

   start = (GLuint *)((char *)imesa->vertex_dma_buffer->address + 
		      imesa->vertex_dma_buffer->used);

   imesa->vertex_dma_buffer->used += dwords * 4;
   return start;
}

int i830_check_copy(int fd)
{
   return(ioctl(fd, DRM_IOCTL_I830_DOCOPY));
}

static void i830DDFlush( GLcontext *ctx )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   FLUSH_BATCH( imesa );
}

static void i830DDFinish( GLcontext *ctx  ) 
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   i830DmaFinish( imesa );
}

void i830DDInitIoctlFuncs( GLcontext *ctx )
{
   ctx->Driver.Flush = i830DDFlush;
   ctx->Driver.Finish = i830DDFinish;
}
