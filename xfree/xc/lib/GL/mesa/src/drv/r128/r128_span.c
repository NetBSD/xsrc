/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_span.c,v 1.4 2000/12/12 17:17:07 dawes Exp $ */
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
 *   Keith Whitwell <keithw@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#include "r128_context.h"
#include "r128_ioctl.h"
#include "r128_state.h"
#include "r128_span.h"

#include "pb.h"

#define DBG 0

#define HAVE_HW_DEPTH_SPANS	1
#define HAVE_HW_DEPTH_PIXELS	1

#define LOCAL_VARS							\
   r128ContextPtr r128ctx = R128_CONTEXT(ctx);				\
   r128ScreenPtr r128scrn = r128ctx->r128Screen;			\
   __DRIdrawablePrivate *dPriv = r128ctx->driDrawable;			\
   GLuint pitch = r128scrn->fbStride;					\
   GLuint height = dPriv->h;						\
   char *buf = (char *)(r128scrn->fb +					\
			r128ctx->drawOffset +				\
			(dPriv->x * r128scrn->bpp/8) +			\
			(dPriv->y * pitch));				\
   char *read_buf = (char *)(r128scrn->fb +				\
			     r128ctx->readOffset +			\
			     (dPriv->x * r128scrn->bpp/8) +		\
			     (dPriv->y * pitch));			\
   GLushort p;								\
   (void) read_buf; (void) buf; (void) p

#define LOCAL_DEPTH_VARS						\
   r128ContextPtr r128ctx = R128_CONTEXT(ctx);				\
   __DRIdrawablePrivate *dPriv = r128ctx->driDrawable;			\
   GLuint height = dPriv->h;						\
   (void) height

#define LOCAL_STENCIL_VARS	LOCAL_DEPTH_VARS

#define INIT_MONO_PIXEL( p )						\
   p = r128ctx->Color

#define CLIPPIXEL( _x, _y )						\
   ((_x >= minx) && (_x < maxx) && (_y >= miny) && (_y < maxy))


#define CLIPSPAN( _x, _y, _n, _x1, _n1, _i )				\
   if (( _y < miny) || (_y >= maxy)) {					\
      _n1 = 0, _x1 = x;							\
   } else {								\
      _n1 = _n;								\
      _x1 = _x;								\
      if (_x1 < minx) _i += (minx - _x1), _x1 = minx;			\
      if (_x1 + _n1 >= maxx) n1 -= (_x1 + n1 - maxx) + 1;		\
   }

#define Y_FLIP( _y )	(height - _y - 1)


#define HW_LOCK()							\
   r128ContextPtr r128ctx = R128_CONTEXT(ctx);				\
   FLUSH_BATCH( r128ctx );						\
   LOCK_HARDWARE( r128ctx );						\
   r128WaitForIdleLocked( r128ctx );

#define HW_CLIPLOOP()							\
   do {									\
      __DRIdrawablePrivate *dPriv = r128ctx->driDrawable;		\
      int _nc = dPriv->numClipRects;					\
									\
      while ( _nc-- ) {							\
	 int minx = dPriv->pClipRects[_nc].x1 - dPriv->x;		\
	 int miny = dPriv->pClipRects[_nc].y1 - dPriv->y;		\
	 int maxx = dPriv->pClipRects[_nc].x2 - dPriv->x;		\
	 int maxy = dPriv->pClipRects[_nc].y2 - dPriv->y;

#define HW_ENDCLIPLOOP()						\
      }									\
   } while (0)

#define HW_UNLOCK()							\
   UNLOCK_HARDWARE( r128ctx )						\



/* ================================================================
 * Color buffer
 */

/* 16 bit, RGB565 color spanline and pixel functions
 */			\
#define WRITE_RGBA( _x, _y, r, g, b, a )				\
   *(GLushort *)(buf + _x*2 + _y*pitch) = ((((int)r & 0xf8) << 8) |	\
					   (((int)g & 0xfc) << 3) |	\
					   (((int)b & 0xf8) >> 3))

#define WRITE_PIXEL( _x, _y, p )					\
    *(GLushort *)(buf + _x*2 + _y*pitch) = p

#define READ_RGBA( rgba, _x, _y )					\
    do {								\
	GLushort p = *(GLushort *)(read_buf + _x*2 + _y*pitch);		\
	rgba[0] = (p >> 8) & 0xf8;					\
	rgba[1] = (p >> 3) & 0xfc;					\
	rgba[2] = (p << 3) & 0xf8;					\
	rgba[3] = 0xff;							\
    } while (0)

#define TAG(x) r128##x##_RGB565
#include "spantmp.h"


/* 32 bit, ARGB8888 color spanline and pixel functions
 */
#define WRITE_RGBA( _x, _y, r, g, b, a )				\
   *(GLuint *)(buf + _x*4 + _y*pitch) = ((b <<  0) |			\
					 (g <<  8) |			\
					 (r << 16) |			\
					 (a << 24) )

#define WRITE_PIXEL( _x, _y, p )					\
   *(GLuint *)(buf + _x*4 + _y*pitch) = p

#define READ_RGBA( rgba, _x, _y )					\
do {									\
   GLuint p = *(GLuint *)(read_buf + _x*4 + _y*pitch);			\
   rgba[0] = (p >> 16) & 0xff;						\
   rgba[1] = (p >>  8) & 0xff;						\
   rgba[2] = (p >>  0) & 0xff;						\
   rgba[3] = (p >> 24) & 0xff;						\
} while (0)

#define TAG(x) r128##x##_ARGB8888
#include "spantmp.h"



/* ================================================================
 * Depth buffer
 */

/* 16-bit depth buffer functions
 */

#define WRITE_DEPTH_SPAN()						\
   r128WriteDepthSpanLocked( r128ctx, n,				\
			     x + dPriv->x,				\
			     y + dPriv->y,				\
			     depth, mask );

#define WRITE_DEPTH_PIXELS()						\
do {									\
   GLint ox[PB_SIZE];							\
   GLint oy[PB_SIZE];							\
   for ( i = 0 ; i < n ; i++ ) {					\
      ox[i] = x[i] + dPriv->x;						\
   }									\
   for ( i = 0 ; i < n ; i++ ) {					\
      oy[i] = Y_FLIP( y[i] ) + dPriv->y;				\
   }									\
   r128WriteDepthPixelsLocked( r128ctx, n, ox, oy, depth, mask );	\
} while (0)

#define READ_DEPTH_SPAN()						\
do {									\
   r128ScreenPtr r128scrn = r128ctx->r128Screen;			\
   GLushort *buf = (GLushort *)((GLubyte *)r128scrn->fb +		\
				r128scrn->spanOffset);			\
   GLint i;								\
									\
   r128ReadDepthSpanLocked( r128ctx, n,					\
			    x + dPriv->x,				\
			    y + dPriv->y );				\
   r128WaitForIdleLocked( r128ctx );					\
									\
   for ( i = 0 ; i < n ; i++ ) {					\
      depth[i] = buf[i];						\
   }									\
} while (0)

#define READ_DEPTH_PIXELS()						\
do {									\
   r128ScreenPtr r128scrn = r128ctx->r128Screen;			\
   GLushort *buf = (GLushort *)((GLubyte *)r128scrn->fb +		\
				r128scrn->spanOffset);			\
   GLint i, remaining = n;						\
									\
   while ( remaining > 0 ) {						\
      GLint ox[PB_SIZE];						\
      GLint oy[PB_SIZE];						\
      GLint count;							\
									\
      if ( remaining <= 128 ) {						\
	 count = remaining;						\
      } else {								\
	 count = 128;							\
      }									\
      for ( i = 0 ; i < count ; i++ ) {					\
	 ox[i] = x[i] + dPriv->x;					\
      }									\
      for ( i = 0 ; i < count ; i++ ) {					\
	 oy[i] = Y_FLIP( y[i] ) + dPriv->y;				\
      }									\
									\
      r128ReadDepthPixelsLocked( r128ctx, count, ox, oy );		\
      r128WaitForIdleLocked( r128ctx );					\
									\
      for ( i = 0 ; i < count ; i++ ) {					\
	 depth[i] = buf[i];						\
      }									\
      depth += count;							\
      x += count;							\
      y += count;							\
      remaining -= count;						\
   }									\
} while (0)

#define TAG(x) r128##x##_16
#include "depthtmp.h"


/* 24-bit depth, 8-bit stencil buffer functions
 */
#define WRITE_DEPTH_SPAN()						\
   r128WriteDepthSpanLocked( r128ctx, n,				\
			     x + dPriv->x,				\
			     y + dPriv->y,				\
			     depth, mask );

#define WRITE_DEPTH_PIXELS()						\
do {									\
   GLint ox[PB_SIZE];							\
   GLint oy[PB_SIZE];							\
   for ( i = 0 ; i < n ; i++ ) {					\
      ox[i] = x[i] + dPriv->x;						\
   }									\
   for ( i = 0 ; i < n ; i++ ) {					\
      oy[i] = Y_FLIP( y[i] ) + dPriv->y;				\
   }									\
   r128WriteDepthPixelsLocked( r128ctx, n, ox, oy, depth, mask );	\
} while (0)

#define READ_DEPTH_SPAN()						\
do {									\
   r128ScreenPtr r128scrn = r128ctx->r128Screen;			\
   GLuint *buf = (GLuint *)((GLubyte *)r128scrn->fb +			\
			    r128scrn->spanOffset);			\
   GLint i;								\
									\
   r128ReadDepthSpanLocked( r128ctx, n,					\
			    x + dPriv->x,				\
			    y + dPriv->y );				\
   r128WaitForIdleLocked( r128ctx );					\
									\
   for ( i = 0 ; i < n ; i++ ) {					\
      depth[i] = buf[i] & 0x00ffffff;					\
   }									\
} while (0)

#define READ_DEPTH_PIXELS()						\
do {									\
   r128ScreenPtr r128scrn = r128ctx->r128Screen;			\
   GLuint *buf = (GLuint *)((GLubyte *)r128scrn->fb +			\
			    r128scrn->spanOffset);			\
   GLint i, remaining = n;						\
									\
   while ( remaining > 0 ) {						\
      GLint ox[PB_SIZE];						\
      GLint oy[PB_SIZE];						\
      GLint count;							\
									\
      if ( remaining <= 128 ) {						\
	 count = remaining;						\
      } else {								\
	 count = 128;							\
      }									\
      for ( i = 0 ; i < count ; i++ ) {					\
	 ox[i] = x[i] + dPriv->x;					\
      }									\
      for ( i = 0 ; i < count ; i++ ) {					\
	 oy[i] = Y_FLIP( y[i] ) + dPriv->y;				\
      }									\
									\
      r128ReadDepthPixelsLocked( r128ctx, count, ox, oy );		\
      r128WaitForIdleLocked( r128ctx );					\
									\
      for ( i = 0 ; i < count ; i++ ) {					\
	 depth[i] = buf[i] & 0x00ffffff;				\
      }									\
      depth += count;							\
      x += count;							\
      y += count;							\
      remaining -= count;						\
   }									\
} while (0)

#define TAG(x) r128##x##_24_8
#include "depthtmp.h"



/* ================================================================
 * Stencil buffer
 */

/* FIXME: Add support for hardware stencil buffers.
 */



void r128DDInitSpanFuncs( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT(ctx);

   switch ( r128ctx->r128Screen->bpp ) {
   case 16:
      ctx->Driver.WriteRGBASpan		= r128WriteRGBASpan_RGB565;
      ctx->Driver.WriteRGBSpan		= r128WriteRGBSpan_RGB565;
      ctx->Driver.WriteMonoRGBASpan	= r128WriteMonoRGBASpan_RGB565;
      ctx->Driver.WriteRGBAPixels	= r128WriteRGBAPixels_RGB565;
      ctx->Driver.WriteMonoRGBAPixels	= r128WriteMonoRGBAPixels_RGB565;
      ctx->Driver.ReadRGBASpan		= r128ReadRGBASpan_RGB565;
      ctx->Driver.ReadRGBAPixels	= r128ReadRGBAPixels_RGB565;
      break;

   case 32:
      ctx->Driver.WriteRGBASpan		= r128WriteRGBASpan_ARGB8888;
      ctx->Driver.WriteRGBSpan		= r128WriteRGBSpan_ARGB8888;
      ctx->Driver.WriteMonoRGBASpan	= r128WriteMonoRGBASpan_ARGB8888;
      ctx->Driver.WriteRGBAPixels	= r128WriteRGBAPixels_ARGB8888;
      ctx->Driver.WriteMonoRGBAPixels	= r128WriteMonoRGBAPixels_ARGB8888;
      ctx->Driver.ReadRGBASpan		= r128ReadRGBASpan_ARGB8888;
      ctx->Driver.ReadRGBAPixels	= r128ReadRGBAPixels_ARGB8888;
      break;

   default:
      break;
   }

   switch ( r128ctx->DepthSize ) {
   case 16:
      ctx->Driver.ReadDepthSpan		= r128ReadDepthSpan_16;
      ctx->Driver.WriteDepthSpan	= r128WriteDepthSpan_16;
      ctx->Driver.ReadDepthPixels	= r128ReadDepthPixels_16;
      ctx->Driver.WriteDepthPixels	= r128WriteDepthPixels_16;
      break;

   case 24:
      ctx->Driver.ReadDepthSpan		= r128ReadDepthSpan_24_8;
      ctx->Driver.WriteDepthSpan	= r128WriteDepthSpan_24_8;
      ctx->Driver.ReadDepthPixels	= r128ReadDepthPixels_24_8;
      ctx->Driver.WriteDepthPixels	= r128WriteDepthPixels_24_8;
      break;

   default:
      break;
   }

   ctx->Driver.WriteCI8Span		= NULL;
   ctx->Driver.WriteCI32Span		= NULL;
   ctx->Driver.WriteMonoCISpan		= NULL;
   ctx->Driver.WriteCI32Pixels		= NULL;
   ctx->Driver.WriteMonoCIPixels	= NULL;
   ctx->Driver.ReadCI32Span		= NULL;
   ctx->Driver.ReadCI32Pixels		= NULL;
}
