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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_span.c,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#include "types.h"

#include "i830_drv.h"
#include "i830_ioctl.h"


#define DBG 0

#define LOCAL_VARS					\
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;	\
   i830ScreenPrivate *i830Screen = imesa->i830Screen;	\
   GLuint pitch = i830Screen->backPitch * i830Screen->cpp;	\
   GLuint height = dPriv->h;				\
   char *buf = (char *)(imesa->drawMap +		\
			dPriv->x * i830Screen->cpp +			\
			dPriv->y * pitch);		\
   char *read_buf = (char *)(imesa->readMap +		\
			     dPriv->x * i830Screen->cpp +		\
			     dPriv->y * pitch); 	\
   GLushort p = I830_CONTEXT( ctx )->MonoColor;         \
   (void) read_buf; (void) buf; (void) p

#define LOCAL_DEPTH_VARS				\
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;	\
   i830ScreenPrivate *i830Screen = imesa->i830Screen;	\
   GLuint pitch = i830Screen->backPitch * i830Screen->cpp;	\
   GLuint height = dPriv->h;				\
   char *buf = (char *)(i830Screen->depth.map +	\
			dPriv->x * i830Screen->cpp +			\
			dPriv->y * pitch)

#define LOCAL_STENCIL_VARS LOCAL_DEPTH_VARS 

#define INIT_MONO_PIXEL(p)

#define CLIPPIXEL(_x,_y) (_x >= minx && _x < maxx && \
			  _y >= miny && _y < maxy)

#define CLIPSPAN( _x, _y, _n, _x1, _n1, _i )				\
   if ( _y < miny || _y >= maxy ) {					\
      _n1 = 0, _x1 = x; 						\
   } else {								\
      _n1 = _n; 							\
      _x1 = _x; 							\
      if ( _x1 < minx ) _i += (minx-_x1), n1 -= (minx-_x1), _x1 = minx; \
      if ( _x1 + _n1 >= maxx ) n1 -= (_x1 + n1 - maxx); 		\
   }

#define Y_FLIP(_y) (height - _y - 1)


#define HW_LOCK()					\
   i830ContextPtr imesa = I830_CONTEXT(ctx);	\
   FLUSH_BATCH(imesa);					\
   i830DmaFinish(imesa);				\
   LOCK_HARDWARE_QUIESCENT(imesa);

#define HW_CLIPLOOP()						\
  do {								\
    __DRIdrawablePrivate *dPriv = imesa->driDrawable;		\
    int _nc = dPriv->numClipRects;				\
    while (_nc--) {						\
       int minx = dPriv->pClipRects[_nc].x1 - dPriv->x;		\
       int miny = dPriv->pClipRects[_nc].y1 - dPriv->y; 	\
       int maxx = dPriv->pClipRects[_nc].x2 - dPriv->x;		\
       int maxy = dPriv->pClipRects[_nc].y2 - dPriv->y;


#define HW_ENDCLIPLOOP()			\
    }						\
  } while (0)

#define HW_UNLOCK()				\
    UNLOCK_HARDWARE(imesa);




/* 16 bit, 565 rgb color spanline and pixel functions
 */
#define WRITE_RGBA( _x, _y, r, g, b, a )				\
   *(GLushort *)(buf + _x*2 + _y*pitch)  = ( (((int)r & 0xf8) << 8) |	\
		                             (((int)g & 0xfc) << 3) |	\
		                             (((int)b & 0xf8) >> 3))
#define WRITE_PIXEL( _x, _y, p )  \
   *(GLushort *)(buf + _x*2 + _y*pitch) = p

#define READ_RGBA( rgba, _x, _y )				\
do {								\
   GLushort p = *(GLushort *)(read_buf + _x*2 + _y*pitch);	\
   rgba[0] = (((p >> 11) & 0x1f) * 255) / 31;			\
   rgba[1] = (((p >>  5) & 0x3f) * 255) / 63;			\
   rgba[2] = (((p >>  0) & 0x1f) * 255) / 31;			\
   rgba[3] = 255;						\
} while(0)

#define TAG(x) i830##x##_565
#include "spantmp.h"




/* 15 bit, 555 rgb color spanline and pixel functions
 */
#define WRITE_RGBA( _x, _y, r, g, b, a )			\
   *(GLushort *)(buf + _x*2 + _y*pitch)  = (((r & 0xf8) << 7) |	\
		                            ((g & 0xf8) << 3) |	\
                         		    ((b & 0xf8) >> 3))

#define WRITE_PIXEL( _x, _y, p )  \
   *(GLushort *)(buf + _x*2 + _y*pitch)  = p

#define READ_RGBA( rgba, _x, _y )				\
do {								\
   GLushort p = *(GLushort *)(read_buf + _x*2 + _y*pitch);	\
   rgba[0] = (p >> 7) & 0xf8;					\
   rgba[1] = (p >> 3) & 0xf8;					\
   rgba[2] = (p << 3) & 0xf8;					\
   rgba[3] = 255;						\
} while(0)

#define TAG(x) i830##x##_555
#include "spantmp.h"

/* 16 bit depthbuffer functions.
 */
#define WRITE_DEPTH( _x, _y, d ) \
   *(GLushort *)(buf + _x*2 + _y*pitch)  = d;

#define READ_DEPTH( d, _x, _y )	\
   d = *(GLushort *)(buf + _x*2 + _y*pitch);	 

/*     d = 0xffff; */

#define TAG(x) i830##x##_16
#include "depthtmp.h"


#undef LOCAL_VARS
#define LOCAL_VARS					\
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;	\
   i830ScreenPrivate *i830Screen = imesa->i830Screen;	\
   GLuint pitch = i830Screen->backPitch * i830Screen->cpp;	\
   GLuint height = dPriv->h;				\
   char *buf = (char *)(imesa->drawMap +		\
			dPriv->x * i830Screen->cpp +			\
			dPriv->y * pitch);		\
   char *read_buf = (char *)(imesa->readMap +		\
			     dPriv->x * i830Screen->cpp +		\
			     dPriv->y * pitch); 	\
   GLuint p = I830_CONTEXT( ctx )->MonoColor;         \
   (void) read_buf; (void) buf; (void) p


/* 32 bit, 8888 argb color spanline and pixel functions
 */
#define WRITE_RGBA(_x, _y, r, g, b, a)			\
    *(GLuint *)(buf + _x*4 + _y*pitch) = ((r << 16) |	\
					  (g << 8)  |	\
					  (b << 0)  |	\
					  (a << 24) )

#define WRITE_PIXEL(_x, _y, p)			\
    *(GLuint *)(buf + _x*4 + _y*pitch) = p

/* Note to Self:
 * Don't read alpha from framebuffer, because its not correct.  From a
 * reading of the spec, this should not be the case, need to ask an
 * engineer at Intel.
 */

#define READ_RGBA(rgba, _x, _y)					\
    do {							\
	GLuint p = *(GLuint *)(read_buf + _x*4 + _y*pitch);	\
	rgba[0] = (p >> 16) & 0xff;				\
	rgba[1] = (p >> 8)  & 0xff;				\
	rgba[2] = (p >> 0)  & 0xff;				\
	rgba[3] = 255;						\
    } while (0)

#define TAG(x) i830##x##_8888
#include "spantmp.h"

/* 24 bit depthbuffer functions.
 */
#define WRITE_DEPTH( _x, _y, d )	\
   *(GLuint *)(buf + _x*4 + _y*pitch) = 0xffffff & d;

#define READ_DEPTH( d, _x, _y )		\
   d = *(GLuint *)(buf + _x*4 + _y*pitch) & 0xffffff;

#define TAG(x) i830##x##_24
#include "depthtmp.h"

/* 24/8 bit interleaved depth/stencil functions
 */
#define WRITE_DEPTH( _x, _y, d ) {			\
   GLuint tmp = *(GLuint *)(buf + _x*4 + _y*pitch);	\
   tmp &= 0xff000000;					\
   tmp |= (d) & 0xffffff;				\
   *(GLuint *)(buf + _x*4 + _y*pitch) = tmp;		\
}

#define READ_DEPTH( d, _x, _y )		\
   d = *(GLuint *)(buf + _x*4 + _y*pitch) & 0xffffff;


#define TAG(x) i830##x##_24_8
#include "depthtmp.h"

#define WRITE_STENCIL( _x, _y, d ) {			\
   GLuint tmp = *(GLuint *)(buf + _x*4 + _y*pitch);	\
   tmp &= 0xffffff;					\
   tmp |= (d<<24);					\
   *(GLuint *)(buf + _x*4 + _y*pitch) = tmp;		\
}

#define READ_STENCIL( d, _x, _y )			\
   d = *(GLuint *)(buf + _x*4 + _y*pitch) >> 24;

#define TAG(x) i830##x##_24_8
#include "stenciltmp.h"


void i830DDInitSpanFuncs( GLcontext *ctx )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   i830ScreenPrivate *i830Screen = imesa->i830Screen;

   switch (i830Screen->fbFormat) {
   case DV_PF_555:
      ctx->Driver.WriteRGBASpan = i830WriteRGBASpan_555;
      ctx->Driver.WriteRGBSpan = i830WriteRGBSpan_555;
      ctx->Driver.WriteMonoRGBASpan = i830WriteMonoRGBASpan_555;
      ctx->Driver.WriteRGBAPixels = i830WriteRGBAPixels_555;
      ctx->Driver.WriteMonoRGBAPixels = i830WriteMonoRGBAPixels_555;
      ctx->Driver.ReadRGBASpan = i830ReadRGBASpan_555;
      ctx->Driver.ReadRGBAPixels = i830ReadRGBAPixels_555;

      ctx->Driver.ReadDepthSpan = i830ReadDepthSpan_16;
      ctx->Driver.WriteDepthSpan = i830WriteDepthSpan_16;
      ctx->Driver.ReadDepthPixels = i830ReadDepthPixels_16;
      ctx->Driver.WriteDepthPixels = i830WriteDepthPixels_16;
      break;

   case DV_PF_565:
      ctx->Driver.WriteRGBASpan = i830WriteRGBASpan_565;
      ctx->Driver.WriteRGBSpan = i830WriteRGBSpan_565;
      ctx->Driver.WriteMonoRGBASpan = i830WriteMonoRGBASpan_565;
      ctx->Driver.WriteRGBAPixels = i830WriteRGBAPixels_565;
      ctx->Driver.WriteMonoRGBAPixels = i830WriteMonoRGBAPixels_565; 
      ctx->Driver.ReadRGBASpan = i830ReadRGBASpan_565;
      ctx->Driver.ReadRGBAPixels = i830ReadRGBAPixels_565;

      ctx->Driver.ReadDepthSpan = i830ReadDepthSpan_16;
      ctx->Driver.WriteDepthSpan = i830WriteDepthSpan_16;
      ctx->Driver.ReadDepthPixels = i830ReadDepthPixels_16;
      ctx->Driver.WriteDepthPixels = i830WriteDepthPixels_16;
      break;

   case DV_PF_8888:
      ctx->Driver.WriteRGBASpan = i830WriteRGBASpan_8888;
      ctx->Driver.WriteRGBSpan = i830WriteRGBSpan_8888;
      ctx->Driver.WriteMonoRGBASpan = i830WriteMonoRGBASpan_8888;
      ctx->Driver.WriteRGBAPixels = i830WriteRGBAPixels_8888;
      ctx->Driver.WriteMonoRGBAPixels = i830WriteMonoRGBAPixels_8888;
      ctx->Driver.ReadRGBASpan = i830ReadRGBASpan_8888;
      ctx->Driver.ReadRGBAPixels = i830ReadRGBAPixels_8888;

      if(imesa->hw_stencil) {
	 ctx->Driver.ReadDepthSpan = i830ReadDepthSpan_24_8;
	 ctx->Driver.WriteDepthSpan = i830WriteDepthSpan_24_8;
	 ctx->Driver.ReadDepthPixels = i830ReadDepthPixels_24_8;
	 ctx->Driver.WriteDepthPixels = i830WriteDepthPixels_24_8;

	 ctx->Driver.WriteStencilSpan = i830WriteStencilSpan_24_8;
	 ctx->Driver.ReadStencilSpan = i830ReadStencilSpan_24_8;
	 ctx->Driver.WriteStencilPixels = i830WriteStencilPixels_24_8;
	 ctx->Driver.ReadStencilPixels = i830ReadStencilPixels_24_8;
      } else {
	 ctx->Driver.ReadDepthSpan = i830ReadDepthSpan_24;
	 ctx->Driver.WriteDepthSpan = i830WriteDepthSpan_24;
	 ctx->Driver.ReadDepthPixels = i830ReadDepthPixels_24;
	 ctx->Driver.WriteDepthPixels = i830WriteDepthPixels_24;
      }
      break;
   }

   ctx->Driver.WriteCI8Span        =NULL;
   ctx->Driver.WriteCI32Span       =NULL;
   ctx->Driver.WriteMonoCISpan     =NULL;
   ctx->Driver.WriteCI32Pixels     =NULL;
   ctx->Driver.WriteMonoCIPixels   =NULL;
   ctx->Driver.ReadCI32Span        =NULL;
   ctx->Driver.ReadCI32Pixels      =NULL;
}
