/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810ioctl.c,v 1.5 2001/03/21 16:14:21 dawes Exp $ */

#include <stdio.h>
#include <unistd.h>

#include "types.h"
#include "pb.h"
#include "dd.h"

#include "mm.h"
#include "i810context.h"
#include "i810log.h"
#include "i810ioctl.h"

#include "drm.h"
#include <sys/ioctl.h>

drmBufPtr i810_get_buffer_ioctl( i810ContextPtr imesa )
{
   drm_i810_dma_t dma;
   drmBufPtr buf;
   int retcode;
   
   if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr,  "Getting dma buffer\n");

   while (1) {
      retcode = ioctl(imesa->driFd, DRM_IOCTL_I810_GETBUF, &dma);

      if (dma.granted == 1 && retcode == 0) 
	 break;

      if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, "Retcode : %d, granted : %d\n", retcode, dma.granted);

      ioctl(imesa->driFd, DRM_IOCTL_I810_FLUSH);
   }

   if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, 
	      "imesa->i810Screen->bufs->list : %p, "
	      "dma.request_idx : %d\n", 
	      imesa->i810Screen->bufs->list, dma.request_idx);

   buf = &(imesa->i810Screen->bufs->list[dma.request_idx]);
   buf->idx = dma.request_idx;
   buf->used = 4;		/* leave room for instruction header */
   buf->total = dma.request_size;

   if(imesa->i810Screen->use_copy_buf != 1) 
       buf->address = (drmAddress)dma.virtual;
   return buf;
}



#define DEPTH_SCALE ((1<<16)-1)

GLbitfield i810Clear( GLcontext *ctx, GLbitfield mask, GLboolean all,
		      GLint cx, GLint cy, GLint cw, GLint ch ) 
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   const GLuint colorMask = *((GLuint *) &ctx->Color.ColorMask);
   drm_i810_clear_t clear;
   int i;

   clear.flags = 0;
   clear.clear_color = imesa->ClearColor;
   clear.clear_depth = (GLuint) (ctx->Depth.Clear * DEPTH_SCALE);

   FLUSH_BATCH( imesa );
	
   if ((mask & DD_FRONT_LEFT_BIT) && colorMask == ~0) {
      clear.flags |= I810_FRONT;
      mask &= ~DD_FRONT_LEFT_BIT;
   }

   if ((mask & DD_BACK_LEFT_BIT) && colorMask == ~0) {
      clear.flags |= I810_BACK;
      mask &= ~DD_BACK_LEFT_BIT;
   }

   if ((mask & DD_DEPTH_BIT) && ctx->Depth.Mask) {
      clear.flags |= I810_DEPTH;
      mask &= ~DD_DEPTH_BIT;
   }

   if (!clear.flags)
      return mask;

   LOCK_HARDWARE( imesa );

   /* flip top to bottom */
   cy = dPriv->h-cy-ch;
   cx += imesa->drawX;
   cy += imesa->drawY;

   if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "Clear, bufs %x nbox %d\n", 
	      (int)clear.flags, (int)imesa->numClipRects);

   for (i = 0 ; i < imesa->numClipRects ; ) 
   { 	 
      int nr = MIN2(i + I810_NR_SAREA_CLIPRECTS, imesa->numClipRects);
      XF86DRIClipRectRec *box = imesa->pClipRects;	 
      drm_clip_rect_t *b = imesa->sarea->boxes;
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
      ioctl(imesa->driFd, DRM_IOCTL_I810_CLEAR, &clear);
   }

   UNLOCK_HARDWARE( imesa );
   imesa->dirty |= I810_UPLOAD_CLIPRECTS;

   return mask;
}




/*
 * Copy the back buffer to the front buffer. 
 */
void i810SwapBuffers( i810ContextPtr imesa ) 
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
      int nr = MIN2(i + I810_NR_SAREA_CLIPRECTS, dPriv->numClipRects);
      XF86DRIClipRectRec *b = (XF86DRIClipRectRec *)imesa->sarea->boxes;

      imesa->sarea->nbox = nr - i;

      for ( ; i < nr ; i++) 
	 *b++ = pbox[i];

      ioctl(imesa->driFd, DRM_IOCTL_I810_SWAP);
   }

   tmp = GET_ENQUEUE_AGE(imesa);
   UNLOCK_HARDWARE( imesa );

   if (GET_DISPATCH_AGE(imesa) < imesa->lastSwap)
      i810WaitAge(imesa, imesa->lastSwap);

   imesa->lastSwap = tmp;
   imesa->dirty |= I810_UPLOAD_CLIPRECTS;
}






/* This waits for *everybody* to finish rendering -- overkill.
 */
void i810DmaFinish( i810ContextPtr imesa  ) 
{
   FLUSH_BATCH( imesa );

   if (imesa->sarea->last_quiescent != imesa->sarea->last_enqueue) {
      if (I810_DEBUG&DEBUG_VERBOSE_IOCTL) 
	 fprintf(stderr, "i810DmaFinish\n");

      LOCK_HARDWARE( imesa );
      i810RegetLockQuiescent( imesa );
      UNLOCK_HARDWARE( imesa );
      imesa->sarea->last_quiescent = imesa->sarea->last_enqueue;
   }
}


void i810RegetLockQuiescent( i810ContextPtr imesa  ) 
{
   /* XXX I disabled this conditional.  Doing so fixes all the readpixels
    * problems.  The problem was that we'd sometimes read from the frame
    * buffer (via the span functions) before rendering was completed.
    * Taking out this conditional solves that problem.  (BrianP)
    *
   if (imesa->sarea->last_quiescent != imesa->sarea->last_enqueue) {
   */
      if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, "i810RegetLockQuiescent\n");

      drmUnlock(imesa->driFd, imesa->hHWContext);
      i810GetLock( imesa, DRM_LOCK_QUIESCENT ); 
      imesa->sarea->last_quiescent = imesa->sarea->last_enqueue;
   /*
   }
   */
}

void i810WaitAgeLocked( i810ContextPtr imesa, int age  ) 
{
   int i = 0;


   while (++i < 500000 && GET_DISPATCH_AGE(imesa) < age) {
      ioctl(imesa->driFd, DRM_IOCTL_I810_GETAGE);
   }

   if (GET_DISPATCH_AGE(imesa) < age) {
      if (0)
	 fprintf(stderr, "wait locked %d %d\n", age, GET_DISPATCH_AGE(imesa));
      ioctl(imesa->driFd, DRM_IOCTL_I810_FLUSH);
   }
}


void i810WaitAge( i810ContextPtr imesa, int age  ) 
{
   int i = 0;

   while (++i < 500000 && GET_DISPATCH_AGE(imesa) < age) {
      ioctl(imesa->driFd, DRM_IOCTL_I810_GETAGE);
   }

   if (GET_DISPATCH_AGE(imesa) >= age)
      return;

   i = 0;
   while (++i < 1000 && GET_DISPATCH_AGE(imesa) < age) {
      ioctl(imesa->driFd, DRM_IOCTL_I810_GETAGE);
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
	 ioctl(imesa->driFd, DRM_IOCTL_I810_FLUSH);
      UNLOCK_HARDWARE(imesa);
   }
}



void i810FlushVertices( i810ContextPtr imesa ) 
{
   if (!imesa->vertex_dma_buffer) return;

   LOCK_HARDWARE( imesa );
   i810FlushVerticesLocked( imesa );
   UNLOCK_HARDWARE( imesa );
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


static void age_imesa( i810ContextPtr imesa, int age )
{
   if (imesa->CurrentTexObj[0]) imesa->CurrentTexObj[0]->age = age;
   if (imesa->CurrentTexObj[1]) imesa->CurrentTexObj[1]->age = age;
}

void i810FlushVerticesLocked( i810ContextPtr imesa )
{
   drm_clip_rect_t *pbox = (drm_clip_rect_t *)imesa->pClipRects;
   int nbox = imesa->numClipRects;
   drmBufPtr buffer = imesa->vertex_dma_buffer;
   drm_i810_vertex_t vertex;
   int i;

   if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "i810FlushVerticesLocked, buf->used %d\n", 
	      buffer->used);

   if (!buffer)
      return;

   if (imesa->dirty & ~I810_UPLOAD_CLIPRECTS)
      i810EmitHwStateLocked( imesa );

   if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "i810FlushVerticesLocked, used %d\n",
	      buffer->used);
   
   imesa->vertex_dma_buffer = 0;

   vertex.idx = buffer->idx;
   vertex.used = buffer->used;
   vertex.discard = 0;

   if (!nbox)
      vertex.used = 0;

   if (nbox > I810_NR_SAREA_CLIPRECTS)
      imesa->dirty |= I810_UPLOAD_CLIPRECTS;
   
   if(imesa->i810Screen->use_copy_buf == 1 && vertex.used) {
      drm_i810_copy_t copy;
      
      copy.idx = buffer->idx;
      copy.used = buffer->used;
      copy.address = buffer->address;
      ioctl(imesa->driFd, DRM_IOCTL_I810_COPY, &copy);
   }


   imesa->sarea->vertex_prim = imesa->vertex_prim;

   if (!nbox || !(imesa->dirty & I810_UPLOAD_CLIPRECTS)) 
   {
      if (nbox == 1) 
	 imesa->sarea->nbox = 0;
      else
	 imesa->sarea->nbox = nbox;

      if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
	 fprintf(stderr, "DRM_IOCTL_I810_VERTEX CASE1 nbox %d used %d\n", 
		 nbox, vertex.used);

      vertex.discard = 1;
      ioctl(imesa->driFd, DRM_IOCTL_I810_VERTEX, &vertex);
      age_imesa(imesa, imesa->sarea->last_enqueue);
   }  
   else 
   {
      for (i = 0 ; i < nbox ; )
      {
	 int nr = MIN2(i + I810_NR_SAREA_CLIPRECTS, nbox);
	 drm_clip_rect_t *b = imesa->sarea->boxes;

	 if (imesa->scissor) {
	    imesa->sarea->nbox = 0;
	 
	    for ( ; i < nr ; i++) {
	       b->x1 = pbox[i].x1 - imesa->drawX;
	       b->y1 = pbox[i].y1 - imesa->drawY;
	       b->x2 = pbox[i].x2 - imesa->drawX;
	       b->y2 = pbox[i].y2 - imesa->drawY;

	       if (intersect_rect(b, b, &imesa->scissor_rect)) {
		  imesa->sarea->nbox++;
		  b++;
	       }
	    }

	    /* Culled?
	     */
	    if (!imesa->sarea->nbox) {
	       if (nr < nbox) continue;
	       vertex.used = 0;
	    }
	 } else {
	    imesa->sarea->nbox = nr - i;
	    for ( ; i < nr ; i++, b++) {
	       b->x1 = pbox[i].x1 - imesa->drawX;
	       b->y1 = pbox[i].y1 - imesa->drawY;
	       b->x2 = pbox[i].x2 - imesa->drawX;
	       b->y2 = pbox[i].y2 - imesa->drawY;
	    }
	 }
	 
	 /* Finished with the buffer?
	  */
	 if (nr == nbox) 
	    vertex.discard = 1;

     	 if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
	    fprintf(stderr, "DRM_IOCTL_I810_VERTEX nbox %d used %d\n", 
		    nbox, vertex.used);

	 ioctl(imesa->driFd, DRM_IOCTL_I810_VERTEX, &vertex);
	 age_imesa(imesa, imesa->sarea->last_enqueue);
      }
   }

   imesa->dirty = 0;
   if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)
      fprintf(stderr, "finished i810FlushVerticesLocked\n");
}


GLuint *i810AllocDwords( i810ContextPtr imesa, int dwords )
{
   GLuint *start;

   if (!imesa->vertex_dma_buffer) 
   {
      LOCK_HARDWARE(imesa);
      imesa->vertex_dma_buffer = i810_get_buffer_ioctl( imesa );
      UNLOCK_HARDWARE(imesa);
   } 
   else if (imesa->vertex_dma_buffer->used + dwords * 4 > 
	    imesa->vertex_dma_buffer->total) 
   {
      LOCK_HARDWARE(imesa);
      i810FlushVerticesLocked( imesa );
      imesa->vertex_dma_buffer = i810_get_buffer_ioctl( imesa );
      UNLOCK_HARDWARE(imesa);
   }

   start = (GLuint *)((char *)imesa->vertex_dma_buffer->address + 
		      imesa->vertex_dma_buffer->used);

   imesa->vertex_dma_buffer->used += dwords * 4;
   return start;
}

int i810_check_copy(int fd)
{
   return(ioctl(fd, DRM_IOCTL_I810_DOCOPY));
}

static void i810DDFlush( GLcontext *ctx )
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   FLUSH_BATCH( imesa );
}

static void i810DDFinish( GLcontext *ctx  ) 
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   i810DmaFinish( imesa );
}

void i810DDInitIoctlFuncs( GLcontext *ctx )
{
   ctx->Driver.Flush = i810DDFlush;
   ctx->Driver.Finish = i810DDFinish;
}
