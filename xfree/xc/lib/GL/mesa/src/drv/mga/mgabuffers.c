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
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgabuffers.c,v 1.4 2000/09/24 13:51:05 alanh Exp $ */

/*
 * Authors:
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 * $PI: $
 */

#include <stdio.h>
#include "mgacontext.h"
#include "mgabuffers.h"
#include "mgastate.h"
#include "mgaioctl.h"

static void mgaXMesaSetFrontClipRects( mgaContextPtr mmesa )
{
   __DRIdrawablePrivate *driDrawable = mmesa->driDrawable;

   mmesa->numClipRects = driDrawable->numClipRects;
   mmesa->pClipRects = driDrawable->pClipRects;
   mmesa->drawX = driDrawable->x;
   mmesa->drawY = driDrawable->y;

   mmesa->Setup[MGA_CTXREG_DSTORG] = mmesa->drawOffset;
   mmesa->dirty |= MGA_UPLOAD_CTX;
   mmesa->dirty |= MGA_UPLOAD_CLIPRECTS;
}


static void mgaXMesaSetBackClipRects( mgaContextPtr mmesa )
{
   __DRIdrawablePrivate *driDrawable = mmesa->driDrawable;

   if (driDrawable->numBackClipRects == 0) 
   {
      mmesa->numClipRects = driDrawable->numClipRects;
      mmesa->pClipRects = driDrawable->pClipRects;
      mmesa->drawX = driDrawable->x;
      mmesa->drawY = driDrawable->y;
   } else {
      mmesa->numClipRects = driDrawable->numBackClipRects;
      mmesa->pClipRects = driDrawable->pBackClipRects;
      mmesa->drawX = driDrawable->backX;
      mmesa->drawY = driDrawable->backY;
   }

   mmesa->Setup[MGA_CTXREG_DSTORG] = mmesa->drawOffset;

   mmesa->dirty |= MGA_UPLOAD_CTX;
   mmesa->dirty |= MGA_UPLOAD_CLIPRECTS;
}



static void mgaUpdateRectsFromSarea( mgaContextPtr mmesa )
{
   __DRIdrawablePrivate *driDrawable = mmesa->driDrawable;
   __DRIscreenPrivate *driScreen = mmesa->driScreen;
   drm_mga_sarea_t *sarea = mmesa->sarea;
   int i = 0, top = 0;


   if (sarea->exported_buffers & MGA_BACK) {

      driDrawable->numBackClipRects = sarea->exported_nback;
      driDrawable->pBackClipRects = mmesa->tmp_boxes[0];

      top = sarea->exported_nback;
      for (i = 0 ; i < top ; i++)
	 driDrawable->pBackClipRects[i] = 
	    *(XF86DRIClipRectPtr)&(sarea->exported_boxes[i]);
   }


   if (sarea->exported_buffers & MGA_FRONT) 
   {
      int start = top;

      driDrawable->numClipRects = sarea->exported_nfront;
      driDrawable->pClipRects = mmesa->tmp_boxes[1];

      top += sarea->exported_nfront;
      for ( ; i < top ; i++)
	 driDrawable->pClipRects[i-start] = 
	    *(XF86DRIClipRectPtr)&(sarea->exported_boxes[i]);
      
   } 



   driDrawable->index = sarea->exported_index;
   driDrawable->lastStamp = sarea->exported_stamp;
   driDrawable->x = sarea->exported_front_x;
   driDrawable->y = sarea->exported_front_y;
   driDrawable->backX = sarea->exported_back_x;
   driDrawable->backY = sarea->exported_back_y;
   driDrawable->w = sarea->exported_w;
   driDrawable->h = sarea->exported_h;
   driDrawable->pStamp = 
      &(driScreen->pSAREA->drawableTable[driDrawable->index].stamp);

   mmesa->dirty_cliprects = (MGA_FRONT|MGA_BACK) & ~(sarea->exported_buffers);
}



static void printSareaRects( mgaContextPtr mmesa )
{
   __DRIscreenPrivate *driScreen = mmesa->driScreen;
   drm_mga_sarea_t *sarea = mmesa->sarea;
   int i;

   fprintf(stderr, "sarea->exported: %d\n", sarea->exported_drawable);
   fprintf(stderr, "sarea->exported_index: %d\n", sarea->exported_index);
   fprintf(stderr, "sarea->exported_stamp: %d\n", sarea->exported_stamp);
   fprintf(stderr, "sarea->exported_front_x: %d\n", sarea->exported_front_x);
   fprintf(stderr, "sarea->exported_front_y: %d\n", sarea->exported_front_y);
   fprintf(stderr, "sarea->exported_back_x: %d\n", sarea->exported_back_x);
   fprintf(stderr, "sarea->exported_back_y: %d\n", sarea->exported_back_y);
   fprintf(stderr, "sarea->exported_w: %d\n", sarea->exported_w);
   fprintf(stderr, "sarea->exported_h: %d\n", sarea->exported_h);
   fprintf(stderr, "sarea->exported_buffers: %d\n", sarea->exported_buffers);
   fprintf(stderr, "sarea->exported_nfront: %d\n", sarea->exported_nfront);
   fprintf(stderr, "sarea->exported_nback: %d\n", sarea->exported_nback);

   i = 0;
   if (sarea->exported_buffers & MGA_BACK)
      for ( ; i < sarea->exported_nback ; i++) 
	 fprintf(stderr, "back %d: %d,%d-%d,%d\n", i,
		 sarea->exported_boxes[i].x1, sarea->exported_boxes[i].y1,
		 sarea->exported_boxes[i].x2, sarea->exported_boxes[i].y2);

   if (sarea->exported_buffers & MGA_FRONT) {
      int start = i;
      int top = i + sarea->exported_nfront;
      for ( ; i < top ; i++) 
	 fprintf(stderr, "front %d: %d,%d-%d,%d\n", 
		 i - start,
		 sarea->exported_boxes[i].x1, sarea->exported_boxes[i].y1,
		 sarea->exported_boxes[i].x2, sarea->exported_boxes[i].y2);
   }

   fprintf(stderr, "drawableTable[%d].stamp: %d\n", 
	   sarea->exported_index,
	   driScreen->pSAREA->drawableTable[sarea->exported_index].stamp);
}

static void printMmesaRects( mgaContextPtr mmesa )
{
   __DRIscreenPrivate *driScreen = mmesa->driScreen;
   __DRIdrawablePrivate *driDrawable = mmesa->driDrawable;
   int nr = mmesa->numClipRects;
   int i;

   fprintf(stderr, "driDrawable->draw: %ld\n", driDrawable->draw);
   fprintf(stderr, "driDrawable->index: %d\n", driDrawable->index);
   fprintf(stderr, "driDrawable->lastStamp: %d\n", driDrawable->lastStamp);
   fprintf(stderr, "mmesa->drawX: %d\n", mmesa->drawX);
   fprintf(stderr, "mmesa->drawY: %d\n", mmesa->drawY);
   fprintf(stderr, "driDrawable->w: %d\n", driDrawable->w);
   fprintf(stderr, "driDrawable->h: %d\n", driDrawable->h);

   for (i = 0 ; i < nr ; i++) 
      fprintf(stderr, "box %d: %d,%d-%d,%d\n", i,
	      mmesa->pClipRects[i].x1, mmesa->pClipRects[i].y1,
	      mmesa->pClipRects[i].x2, mmesa->pClipRects[i].y2);

   fprintf(stderr, "mmesa->draw_buffer: %d\n", mmesa->draw_buffer);
   fprintf(stderr, "drawableTable[%d].stamp: %d\n", 
	   driDrawable->index,
	   driScreen->pSAREA->drawableTable[driDrawable->index].stamp);
}



void mgaUpdateRects( mgaContextPtr mmesa, GLuint buffers )
{
   __DRIdrawablePrivate *driDrawable = mmesa->driDrawable;
   drm_mga_sarea_t *sarea = mmesa->sarea;

   /* The MGA X driver will try to push one set of cliprects (back or
    * front, the last active) for one drawable (the last used) into
    * the sarea.  See if that window was ours, else retrieve both back
    * and front rects from the X server via a protocol request.  
    *
    * If there isn't room for the cliprects in the sarea, the X server
    * clears the drawable value to indicate failure.  
    */
   if (0) printSareaRects(mmesa);

   if (0 && 
       sarea->exported_drawable == driDrawable->draw && 
       (sarea->exported_buffers & buffers) == buffers)
   {
      mgaUpdateRectsFromSarea( mmesa );
   } 
   else 
   {
      if(MGA_DEBUG & DEBUG_VERBOSE_MSG)
		fprintf(stderr, "^");
      driDrawable->lastStamp = 0;

      XMESA_VALIDATE_DRAWABLE_INFO(mmesa->display, 
				   mmesa->driScreen, 
				   driDrawable); 
      mmesa->dirty_cliprects = 0;	
   }

   if (mmesa->draw_buffer == MGA_FRONT)
      mgaXMesaSetFrontClipRects( mmesa );
   else
      mgaXMesaSetBackClipRects( mmesa );


   if (0) printMmesaRects(mmesa);

   sarea->req_drawable = driDrawable->draw;
   sarea->req_draw_buffer = mmesa->draw_buffer;

   
   mgaUpdateClipping( mmesa->glCtx );
   
   mmesa->dirty |= MGA_UPLOAD_CLIPRECTS;
}



GLboolean mgaDDSetDrawBuffer(GLcontext *ctx, GLenum mode )
{
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);

   FLUSH_BATCH( MGA_CONTEXT(ctx) );

   mmesa->Fallback &= ~MGA_FALLBACK_BUFFER;

   if (mode == GL_FRONT_LEFT) 
   {
      mmesa->drawOffset = mmesa->mgaScreen->frontOffset;
      mmesa->readOffset = mmesa->mgaScreen->frontOffset;
      mmesa->Setup[MGA_CTXREG_DSTORG] = mmesa->mgaScreen->frontOffset;
      mmesa->dirty |= MGA_UPLOAD_CTX;
      mmesa->draw_buffer = MGA_FRONT;
      mgaXMesaSetFrontClipRects( mmesa );
      return GL_TRUE;
   } 
   else if (mode == GL_BACK_LEFT) 
   {
      mmesa->drawOffset = mmesa->mgaScreen->backOffset;
      mmesa->readOffset = mmesa->mgaScreen->backOffset;
      mmesa->Setup[MGA_CTXREG_DSTORG] = mmesa->mgaScreen->backOffset;
      mmesa->draw_buffer = MGA_BACK;
      mmesa->dirty |= MGA_UPLOAD_CTX;
      mgaXMesaSetBackClipRects( mmesa );
      return GL_TRUE;
   }
   else 
   {
      mmesa->Fallback |= MGA_FALLBACK_BUFFER;
      return GL_FALSE;
   }
}

void mgaDDSetReadBuffer(GLcontext *ctx, GLframebuffer *buffer,
			GLenum mode )
{
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);

   if (mode == GL_FRONT_LEFT) 
   {
      mmesa->readOffset = mmesa->mgaScreen->frontOffset;
      mmesa->read_buffer = MGA_FRONT;
   } 
   else 
   {
      mmesa->readOffset = mmesa->mgaScreen->backOffset;
      mmesa->read_buffer = MGA_BACK;
   }
}
