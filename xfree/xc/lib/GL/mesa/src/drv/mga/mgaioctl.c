/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgaioctl.c,v 1.8 2000/11/08 05:02:45 dawes Exp $ */

#include <stdio.h>

#include "types.h"
#include "pb.h"
#include "dd.h"

#include "mm.h"
#include "mgacontext.h"
#include "mgadd.h"
#include "mgastate.h"
#include "mgatex.h"
#include "mgavb.h"
#include "mgatris.h"
#include "mgabuffers.h"


#include "drm.h"
#include <sys/ioctl.h>

#define DEPTH_SCALE_16 ((GLfloat)0xffff)
#define DEPTH_SCALE_32 ((GLfloat)0xffffffff) 

static void mga_iload_dma_ioctl(mgaContextPtr mmesa,
				unsigned long dest, 
				int length)
{
   int retcode;
   drm_mga_iload_t iload;
   drmBufPtr buf = mmesa->iload_buffer;
   
   iload.idx = buf->idx;
   iload.destOrg = dest;	
   iload.length = length;

   if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL) 
      fprintf(stderr, "DRM_IOCTL_MGA_ILOAD idx %d dst %x length %d\n",
	   iload.idx, iload.destOrg, iload.length);
	   
   
   if ((retcode = ioctl(mmesa->driFd, DRM_IOCTL_MGA_ILOAD, &iload))) {
      printf("send iload retcode = %d\n", retcode);
      exit(1);
   }

   mmesa->iload_buffer = 0;

   if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL) 
      fprintf(stderr, "finished iload dma put\n");

}

int mgaUpdateLock( mgaContextPtr mmesa, drmLockFlags flags )
{
   drm_lock_t lock;
   int retcode;
   
   lock.flags = 0;

   if (mmesa->sarea->last_quiescent != mmesa->sarea->last_enqueue &&
       flags & DRM_LOCK_QUIESCENT) {
      if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
	fprintf(stderr, "mgaLockQuiescent\n");
      lock.flags |= _DRM_LOCK_QUIESCENT;
   }
   
   if (flags & DRM_LOCK_FLUSH)      lock.flags |= _DRM_LOCK_FLUSH;
   if (flags & DRM_LOCK_FLUSH_ALL)  lock.flags |= _DRM_LOCK_FLUSH_ALL;

   if (!lock.flags)
      return 0;

   retcode = ioctl(mmesa->driFd, DRM_IOCTL_MGA_FLUSH, &lock);
   if(retcode != 0) {
      fprintf(stderr, "Lockupdate failed\n");
      if(retcode == EACCES) exit(1);
      else return -1;
   }
   
   if(flags & DRM_LOCK_QUIESCENT)
     mmesa->sarea->last_quiescent = mmesa->sarea->last_enqueue;
 
   return 0;
}

drmBufPtr mga_get_buffer_ioctl( mgaContextPtr mmesa )
{
   int idx = 0;
   int size = 0;
   drmDMAReq dma;
   int retcode;
   drmBufPtr buf;
   
   if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr,  "Getting dma buffer\n");
   
   dma.context = mmesa->hHWContext;
   dma.send_count = 0;
   dma.send_list = NULL;
   dma.send_sizes = NULL;
   dma.flags = 0;
   dma.request_count = 1;
   dma.request_size = MGA_DMA_BUF_SZ;
   dma.request_list = &idx;
   dma.request_sizes = &size;
   dma.granted_count = 0;


   if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "drmDMA (get) ctx %d count %d size 0x%x\n",
	   dma.context, dma.request_count, 
	   dma.request_size);

   while (1) {
      retcode = drmDMA(mmesa->driFd, &dma);

      if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, "retcode %d sz %d idx %d count %d\n", 
		 retcode,
		 dma.request_sizes[0],
		 dma.request_list[0],
		 dma.granted_count);

      if (retcode == 0 && 
	  dma.request_sizes[0] &&
	  dma.granted_count) 
	 break;

      if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)  
	 fprintf(stderr, "\n\nflush");
      mgaUpdateLock( mmesa, DRM_LOCK_FLUSH );
   }

   buf = &(mmesa->mgaScreen->bufs->list[idx]);
   buf->used = 0;

   if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, 
	   "drmDMA (get) returns size[0] 0x%x idx[0] %d\n"
	   "dma_buffer now: buf idx: %d size: %d used: %d addr %p\n",
	   dma.request_sizes[0], dma.request_list[0],
	   buf->idx, buf->total,
	   buf->used, buf->address);

   if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "finished getbuffer\n");

   return buf;
}




GLbitfield mgaClear( GLcontext *ctx, GLbitfield mask, GLboolean all,
		     GLint cx, GLint cy, GLint cw, GLint ch ) 
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   __DRIdrawablePrivate *dPriv = mmesa->driDrawable;
   drm_mga_clear_t clear;
   int retcode;
   int i;
   static int nrclears;

   if (0) fprintf(stderr, "clear %d: %d,%d %dx%d\n", all,cx,cy,cw,ch);

   clear.flags = 0;
   clear.clear_color = mmesa->ClearColor;
   clear.clear_depth = 0;
   clear.clear_depth_mask = 0;

   FLUSH_BATCH( mmesa );
	
   if (mask & DD_FRONT_LEFT_BIT) {
      clear.flags |= MGA_FRONT;
      clear.clear_color_mask = mmesa->Setup[MGA_CTXREG_PLNWT];
      mask &= ~DD_FRONT_LEFT_BIT;
   }

   if (mask & DD_BACK_LEFT_BIT) {
      clear.flags |= MGA_BACK;
      clear.clear_color_mask = mmesa->Setup[MGA_CTXREG_PLNWT];
      mask &= ~DD_BACK_LEFT_BIT;
   }

   if ((mask & DD_DEPTH_BIT) && ctx->Depth.Mask) {
      clear.flags |= MGA_DEPTH;
      clear.clear_depth_mask |= mmesa->depth_clear_mask;
      clear.clear_depth = (mmesa->ClearDepth &
			   mmesa->depth_clear_mask);
      mask &= ~DD_DEPTH_BIT;
   }

   if ((mask & DD_STENCIL_BIT) && mmesa->hw_stencil) {
      clear.flags |= MGA_DEPTH;
      clear.clear_depth_mask |= mmesa->stencil_clear_mask;
      clear.clear_depth |= (ctx->Stencil.Clear &
			    mmesa->stencil_clear_mask);
      mask &= ~DD_STENCIL_BIT;
   }

   if (!clear.flags)
      return mask;

   LOCK_HARDWARE( mmesa );

   if (mmesa->dirty_cliprects)
      mgaUpdateRects( mmesa, (MGA_FRONT|MGA_BACK));

   /* flip top to bottom */
   cy = dPriv->h-cy-ch;
   cx += mmesa->drawX;
   cy += mmesa->drawY;

   if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "Clear, bufs %x nbox %d\n", 
	      (int)clear.flags, (int)mmesa->numClipRects);

   for (i = 0 ; i < mmesa->numClipRects ; ) 
   { 	 
      int nr = MIN2(i + MGA_NR_SAREA_CLIPRECTS, mmesa->numClipRects);
      XF86DRIClipRectRec *box = mmesa->pClipRects;	 
      drm_clip_rect_t *b = mmesa->sarea->boxes;
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


      if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, 
		 "DRM_IOCTL_MGA_CLEAR flag 0x%x color %x depth %x nbox %d\n",
		 clear.flags, clear.clear_color, 
		 clear.clear_depth, mmesa->sarea->nbox);
		

      mmesa->sarea->nbox = n;

      retcode = ioctl(mmesa->driFd, DRM_IOCTL_MGA_CLEAR, &clear);
      if (retcode) {
	 printf("send clear retcode = %d\n", retcode);
	 exit(1);
      }
      if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, "finished clear %d\n", ++nrclears);
   }

   UNLOCK_HARDWARE( mmesa );
   mmesa->dirty |= MGA_UPLOAD_CLIPRECTS;

   return mask;
}


int nrswaps;



/*
 * Copy the back buffer to the front buffer. 
 */
void mgaSwapBuffers( mgaContextPtr mmesa ) 
{
   __DRIdrawablePrivate *dPriv = mmesa->driDrawable;
   XF86DRIClipRectPtr pbox;
   int nbox;
   drm_mga_swap_t swap;
   int retcode;
   int i;
   int tmp;


   FLUSH_BATCH( mmesa );

   LOCK_HARDWARE( mmesa );
   
   /* Use the frontbuffer cliprects
    */
   if (mmesa->dirty_cliprects & MGA_FRONT) 
      mgaUpdateRects( mmesa, MGA_FRONT );
   

   pbox = dPriv->pClipRects;
   nbox = dPriv->numClipRects;

   if (0) fprintf(stderr, "swap, nbox %d\n", nbox);

   for (i = 0 ; i < nbox ; )
   {
      int nr = MIN2(i + MGA_NR_SAREA_CLIPRECTS, dPriv->numClipRects);
      XF86DRIClipRectRec *b = (XF86DRIClipRectRec *)mmesa->sarea->boxes;

      mmesa->sarea->nbox = nr - i;
	 
      for ( ; i < nr ; i++) 
	 *b++ = pbox[i];
      
      if (0)
	 fprintf(stderr, "DRM_IOCTL_MGA_SWAP\n"); 

#if 1
      if((retcode = ioctl(mmesa->driFd, DRM_IOCTL_MGA_SWAP, &swap))) {
	 printf("send swap retcode = %d\n", retcode);
	 exit(1);
      }
#else
      mgaUpdateLock( mmesa, DRM_LOCK_FLUSH );
#endif

      if (0)
	 fprintf(stderr, "finished swap %d\n", ++nrswaps);
   }

   tmp = GET_ENQUEUE_AGE(mmesa);

   UNLOCK_HARDWARE( mmesa );

   if (GET_DISPATCH_AGE(mmesa) < mmesa->lastSwap)
      mgaWaitAge(mmesa, mmesa->lastSwap);

   mmesa->lastSwap = tmp;
   mmesa->dirty |= MGA_UPLOAD_CLIPRECTS;
}


/* This is overkill
 */
void mgaDDFinish( GLcontext *ctx  ) 
{
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);

   FLUSH_BATCH( mmesa );

   if (mmesa->sarea->last_quiescent != mmesa->sarea->last_enqueue) {
      if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, "mgaRegetLockQuiescent\n");

      LOCK_HARDWARE( mmesa );
      mgaUpdateLock( mmesa, DRM_LOCK_QUIESCENT | DRM_LOCK_FLUSH);
      UNLOCK_HARDWARE( mmesa );

      mmesa->sarea->last_quiescent = mmesa->sarea->last_enqueue;
   }
}

void mgaWaitAgeLocked( mgaContextPtr mmesa, int age  ) 
{
   if (GET_DISPATCH_AGE(mmesa) < age) {
      if (0) fprintf(stderr, "\n\n\nmgaWaitAgeLocked\n");
      mgaUpdateLock( mmesa, DRM_LOCK_FLUSH );
   }
}


void mgaWaitAge( mgaContextPtr mmesa, int age  ) 
{
   if (GET_DISPATCH_AGE(mmesa) < age) {
      LOCK_HARDWARE(mmesa);
      if (GET_DISPATCH_AGE(mmesa) < age) {
	 if (0) fprintf(stderr, "\n\n\nmgaWaitAge\n");
	 mgaUpdateLock( mmesa, DRM_LOCK_FLUSH );
      }
      UNLOCK_HARDWARE(mmesa);
   }
}


static int intersect_rect( drm_clip_rect_t *out,
			   drm_clip_rect_t *a,
			   drm_clip_rect_t *b )
{
   *out = *a;
   if (b->x1 > out->x1) out->x1 = b->x1;
   if (b->y1 > out->y1) out->y1 = b->y1;
   if (b->x2 < out->x2) out->x2 = b->x2;
   if (b->y2 < out->y2) out->y2 = b->y2;
   if (out->x1 >= out->x2) return 0;
   if (out->y1 >= out->y2) return 0;
   return 1;
}




static void age_mmesa( mgaContextPtr mmesa, int age )
{
   if (mmesa->CurrentTexObj[0]) mmesa->CurrentTexObj[0]->age = age;
   if (mmesa->CurrentTexObj[1]) mmesa->CurrentTexObj[1]->age = age;
}

void mgaFlushVerticesLocked( mgaContextPtr mmesa )
{
   drm_clip_rect_t *pbox = (drm_clip_rect_t *)mmesa->pClipRects;
   int nbox = mmesa->numClipRects;
   drmBufPtr buffer = mmesa->vertex_dma_buffer;
   drm_mga_vertex_t vertex;
   int i;

   mmesa->vertex_dma_buffer = 0;

   if (!buffer)
      return;

   if (mmesa->dirty_cliprects & mmesa->draw_buffer)
      mgaUpdateRects( mmesa, mmesa->draw_buffer );

   if (mmesa->dirty & ~MGA_UPLOAD_CLIPRECTS) 
      mgaEmitHwStateLocked( mmesa );

   /* FIXME: Workaround bug in kernel module.
    */
   mmesa->sarea->dirty |= MGA_UPLOAD_CTX; 
      
   /* FIXME: dstorg bug
    */
   if (0)
      if (mmesa->lastX != mmesa->drawX || mmesa->lastY != mmesa->drawY)
	 fprintf(stderr, "****** last: %d,%d current: %d,%d\n",
		 mmesa->lastX, mmesa->lastY,
		 mmesa->drawX, mmesa->drawY);

   vertex.idx = buffer->idx;
   vertex.used = buffer->used;
   vertex.discard = 0;

   if (!nbox)
      vertex.used = 0;

   if (nbox >= MGA_NR_SAREA_CLIPRECTS)
      mmesa->dirty |= MGA_UPLOAD_CLIPRECTS;

   if (!vertex.used || !(mmesa->dirty & MGA_UPLOAD_CLIPRECTS)) 
   {
      if (nbox == 1) 
	 mmesa->sarea->nbox = 0;
      else
	 mmesa->sarea->nbox = nbox;

      if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)  
	 fprintf(stderr, "Firing vertex -- case a nbox %d\n", nbox);

      vertex.discard = 1;
      ioctl(mmesa->driFd, DRM_IOCTL_MGA_VERTEX, &vertex);
      age_mmesa(mmesa, mmesa->sarea->last_enqueue);
   } 
   else 
   {      
      for (i = 0 ; i < nbox ; )
      {
	 int nr = MIN2(i + MGA_NR_SAREA_CLIPRECTS, nbox);
	 drm_clip_rect_t *b = mmesa->sarea->boxes;

	 if (mmesa->scissor) {
	    mmesa->sarea->nbox = 0;	 

	    for ( ; i < nr ; i++) {	       
	       *b = pbox[i];
	       if (intersect_rect(b, b, &mmesa->scissor_rect)) {
		  mmesa->sarea->nbox++;
		  b++;
	       } 
	    }

	    /* Culled?
	     */
	    if (!mmesa->sarea->nbox) {
	       if (nr < nbox) continue;
	       vertex.used = 0;
	    }
	 } else {
	    mmesa->sarea->nbox = nr - i;
	    for ( ; i < nr ; i++) 
	       *b++ = pbox[i];
	 }

	 /* Finished with the buffer?
	  */
	 if (nr == nbox) 
	    vertex.discard = 1;

	 mmesa->sarea->dirty |= MGA_UPLOAD_CLIPRECTS;
	 ioctl(mmesa->driFd, DRM_IOCTL_MGA_VERTEX, &vertex);
	 age_mmesa(mmesa, mmesa->sarea->last_enqueue);
      }
   }

   mmesa->dirty &= ~MGA_UPLOAD_CLIPRECTS;
}

void mgaFlushVertices( mgaContextPtr mmesa ) 
{
   LOCK_HARDWARE( mmesa );
   mgaFlushVerticesLocked( mmesa );
   UNLOCK_HARDWARE( mmesa );
}

void mgaFlushEltsLocked( mgaContextPtr mmesa ) 
{
   if (mmesa->first_elt != mmesa->next_elt) {
      mgaFireEltsLocked( mmesa, 
			 ((char *)mmesa->first_elt - 
			  (char *)mmesa->elt_buf->address),
			 ((char *)mmesa->next_elt - 
			  (char *)mmesa->elt_buf->address),
			 0 );
      mmesa->first_elt = mmesa->next_elt;
   }
}

void mgaFlushElts( mgaContextPtr mmesa ) 
{
   LOCK_HARDWARE( mmesa );
   mgaFlushEltsLocked( mmesa );
   UNLOCK_HARDWARE( mmesa );
}


GLuint *mgaAllocVertexDwords( mgaContextPtr mmesa, int dwords )
{
   int bytes = dwords * 4;
   GLuint *head;

   if (!mmesa->vertex_dma_buffer) {
      LOCK_HARDWARE( mmesa );

      if (mmesa->first_elt != mmesa->next_elt) 
	 mgaFlushEltsLocked(mmesa);

      mmesa->vertex_dma_buffer = mga_get_buffer_ioctl( mmesa );
      UNLOCK_HARDWARE( mmesa );
   } else if (mmesa->vertex_dma_buffer->used + bytes > 
	      mmesa->vertex_dma_buffer->total) {
      LOCK_HARDWARE( mmesa );
      mgaFlushVerticesLocked( mmesa );
      mmesa->vertex_dma_buffer = mga_get_buffer_ioctl( mmesa );
      UNLOCK_HARDWARE( mmesa );
   }

   head = (GLuint *)((char *)mmesa->vertex_dma_buffer->address + 
		      mmesa->vertex_dma_buffer->used);

   mmesa->vertex_dma_buffer->used += bytes;
   return head;
}


void mgaFireILoadLocked( mgaContextPtr mmesa, 
			 GLuint offset, GLuint length )
{
   if (!mmesa->iload_buffer) {
      fprintf(stderr, "mgaFireILoad: no buffer\n");
      return;
   }

   if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "mgaFireILoad idx %d ofs 0x%x length %d\n",
	      mmesa->iload_buffer->idx, (int)offset, (int)length );
   
   /* HACK 
    */
   mgaUpdateLock( mmesa, DRM_LOCK_QUIESCENT | DRM_LOCK_FLUSH);
   mga_iload_dma_ioctl( mmesa, offset, length );
}

void mgaGetILoadBufferLocked( mgaContextPtr mmesa )
{
   if (MGA_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "mgaGetIloadBuffer (buffer now %p)\n",
	   mmesa->iload_buffer);

   mmesa->iload_buffer = mga_get_buffer_ioctl( mmesa );
}


void mgaDDFlush( GLcontext *ctx )
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );


   FLUSH_BATCH( mmesa );

   /* This may be called redundantly - dispatch_age may trail what
    * has actually been sent and processed by the hardware.
    */
   if (1 || GET_DISPATCH_AGE( mmesa ) < mmesa->sarea->last_enqueue) {
      LOCK_HARDWARE( mmesa );
      if (0) fprintf(stderr, "mgaDDFlush %d %d\n", GET_DISPATCH_AGE( mmesa ),  mmesa->sarea->last_enqueue);
      mgaUpdateLock( mmesa, DRM_LOCK_FLUSH );
      UNLOCK_HARDWARE( mmesa );
   }
}



void mgaFireEltsLocked( mgaContextPtr mmesa, 
			GLuint start, 
			GLuint end,
			GLuint discard )
{
   drm_clip_rect_t *pbox = (drm_clip_rect_t *)mmesa->pClipRects;
   int nbox = mmesa->numClipRects;
   drmBufPtr buffer = mmesa->elt_buf;
   drm_mga_indices_t elts;
   int i;


   if (0) fprintf(stderr, "FireElts %d %d\n", start/4, end/4);

   if (!buffer)
      return;

   if (mmesa->dirty_cliprects & mmesa->draw_buffer)
      mgaUpdateRects( mmesa, mmesa->draw_buffer );

   if (mmesa->dirty & ~MGA_UPLOAD_CLIPRECTS) 
      mgaEmitHwStateLocked( mmesa );

   /* FIXME: Workaround bug in kernel module.
    */
   mmesa->sarea->dirty |= MGA_UPLOAD_CTX; 
      
   elts.idx = buffer->idx;
   elts.start = start;
   elts.end = end;
   elts.discard = 0;

   if (!nbox)
      elts.end = start;

   if (nbox >= MGA_NR_SAREA_CLIPRECTS)
      mmesa->dirty |= MGA_UPLOAD_CLIPRECTS;

   if (elts.end == start || !(mmesa->dirty & MGA_UPLOAD_CLIPRECTS)) 
   {
      if (nbox == 1) 
	 mmesa->sarea->nbox = 0;
      else
	 mmesa->sarea->nbox = nbox;

      if (0)
	 fprintf(stderr, "Firing elts -- case a nbox %d\n", nbox);

      elts.discard = discard;
      ioctl(mmesa->driFd, DRM_IOCTL_MGA_INDICES, &elts);
      age_mmesa(mmesa, mmesa->sarea->last_enqueue);
   } 
   else 
   {      
      for (i = 0 ; i < nbox ; )
      {
	 int nr = MIN2(i + MGA_NR_SAREA_CLIPRECTS, nbox);
	 drm_clip_rect_t *b = mmesa->sarea->boxes;

	 if (mmesa->scissor) {
	    mmesa->sarea->nbox = 0;	 

	    for ( ; i < nr ; i++) {	       
	       *b = pbox[i];
	       if (intersect_rect(b, b, &mmesa->scissor_rect)) {
		  mmesa->sarea->nbox++;
		  b++;
	       } 
	    }

	    /* Culled?
	     */
	    if (!mmesa->sarea->nbox) {
	       if (nr < nbox) continue;
	       elts.end = start;
	    }
	 } else {
	    mmesa->sarea->nbox = nr - i;
	    for ( ; i < nr ; i++) 
	       *b++ = pbox[i];
	 }

	 /* Potentially finished with the buffer?
	  */
	 if (nr == nbox) 
	    elts.discard = discard;

	 if (0)
	    fprintf(stderr, "Firing elts -- case b nbox %d\n", nbox);

	 mmesa->sarea->dirty |= MGA_UPLOAD_CLIPRECTS;
	 ioctl(mmesa->driFd, DRM_IOCTL_MGA_INDICES, &elts);
	 age_mmesa(mmesa, mmesa->sarea->last_enqueue);
      }
   }

   mmesa->dirty &= ~MGA_UPLOAD_CLIPRECTS;
}

void mgaGetEltBufLocked( mgaContextPtr mmesa )
{
   mmesa->elt_buf = mga_get_buffer_ioctl( mmesa );
}

void mgaReleaseBufLocked( mgaContextPtr mmesa, drmBufPtr buffer )
{
   drm_mga_vertex_t vertex;

   if (!buffer) return;
   
   vertex.idx = buffer->idx;
   vertex.used = 0;
   vertex.discard = 1;
   ioctl(mmesa->driFd, DRM_IOCTL_MGA_VERTEX, &vertex);
}


void mgaDDInitIoctlFuncs( GLcontext *ctx )
{
   ctx->Driver.Clear = mgaClear;
   ctx->Driver.Flush = mgaDDFlush;
   ctx->Driver.Finish = mgaDDFinish;
}
