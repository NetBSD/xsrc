/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_span.c,v 1.4 2001/04/10 16:07:53 dawes Exp $ */
/**************************************************************************

Copyright 2000, 2001 ATI Technologies Inc., Ontario, Canada, and
                     VA Linux Systems Inc., Fremont, California.

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

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *   Keith Whitwell <keithw@valinux.com>
 *
 */

#include "radeon_context.h"
#include "radeon_ioctl.h"
#include "radeon_state.h"
#include "radeon_span.h"

#define DBG 0

#define LOCAL_VARS							\
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);			\
   radeonScreenPtr radeonScreen = rmesa->radeonScreen;			\
   __DRIscreenPrivate *sPriv = rmesa->driScreen;			\
   __DRIdrawablePrivate *dPriv = rmesa->driDrawable;			\
   GLuint pitch = radeonScreen->frontPitch * radeonScreen->cpp;		\
   GLuint height = dPriv->h;						\
   char *buf = (char *)(sPriv->pFB +					\
			rmesa->drawOffset +				\
			(dPriv->x * radeonScreen->cpp) +		\
			(dPriv->y * pitch));				\
   char *read_buf = (char *)(sPriv->pFB +				\
			     rmesa->readOffset +			\
			     (dPriv->x * radeonScreen->cpp) +		\
			     (dPriv->y * pitch));			\
   GLuint p;								\
   (void) read_buf; (void) buf; (void) p

#define LOCAL_DEPTH_VARS						\
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);			\
   radeonScreenPtr radeonScreen = rmesa->radeonScreen;			\
   __DRIscreenPrivate *sPriv = rmesa->driScreen;			\
   __DRIdrawablePrivate *dPriv = rmesa->driDrawable;			\
   GLuint height = dPriv->h;						\
   GLuint xo = dPriv->x;						\
   GLuint yo = dPriv->y;						\
   char *buf = (char *)(sPriv->pFB + radeonScreen->depthOffset);	\
   (void) buf

#define LOCAL_STENCIL_VARS	LOCAL_DEPTH_VARS

#define INIT_MONO_PIXEL( p )	p = rmesa->Color

#define CLIPPIXEL( _x, _y )						\
   ((_x >= minx) && (_x < maxx) && (_y >= miny) && (_y < maxy))


#define CLIPSPAN( _x, _y, _n, _x1, _n1, _i )				\
   if ( _y < miny || _y >= maxy ) {					\
      _n1 = 0, _x1 = x;							\
   } else {								\
      _n1 = _n;								\
      _x1 = _x;								\
      if ( _x1 < minx ) _i += (minx-_x1), n1 -= (minx-_x1), _x1 = minx; \
      if ( _x1 + _n1 >= maxx ) n1 -= (_x1 + n1 - maxx);		        \
   }

#define Y_FLIP( _y )		(height - _y - 1)


#define HW_LOCK()							\
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);			\
   FLUSH_BATCH( rmesa );						\
   LOCK_HARDWARE( rmesa );						\
   radeonWaitForIdleLocked( rmesa );

#define HW_CLIPLOOP()							\
   do {									\
      __DRIdrawablePrivate *dPriv = rmesa->driDrawable;			\
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
   UNLOCK_HARDWARE( rmesa )



/* ================================================================
 * Color buffer
 */

/* 16 bit, RGB565 color spanline and pixel functions
 */
#define WRITE_RGBA( _x, _y, r, g, b, a )				\
   *(GLushort *)(buf + _x*2 + _y*pitch) = ((((int)r & 0xf8) << 8) |	\
					   (((int)g & 0xfc) << 3) |	\
					   (((int)b & 0xf8) >> 3))

#define WRITE_PIXEL( _x, _y, p )					\
   *(GLushort *)(buf + _x*2 + _y*pitch) = p

#define READ_RGBA( rgba, _x, _y )					\
   do {									\
      GLushort p = *(GLushort *)(read_buf + _x*2 + _y*pitch);		\
      rgba[0] = (p >> 8) & 0xf8;					\
      rgba[1] = (p >> 3) & 0xfc;					\
      rgba[2] = (p << 3) & 0xf8;					\
      rgba[3] = 0xff;							\
      if ( rgba[0] & 0x08 ) rgba[0] |= 0x07;				\
      if ( rgba[1] & 0x04 ) rgba[1] |= 0x03;				\
      if ( rgba[2] & 0x08 ) rgba[2] |= 0x07;				\
   } while (0)

#define TAG(x) radeon##x##_RGB565
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

#define TAG(x) radeon##x##_ARGB8888
#include "spantmp.h"



/* ================================================================
 * Depth buffer
 */

/* The Radeon has depth tiling on all the time, so we have to convert
 * the x,y coordinates into the memory bus address (mba) in the same
 * manner as the engine.  In each case, the linear block address (ba)
 * is calculated, and then wired with x and y to produce the final
 * memory address.
 */
static __inline GLuint radeon_mba_z16( radeonContextPtr rmesa,
				       GLint x, GLint y )
{
   radeonScreenPtr radeonScreen = rmesa->radeonScreen;
   GLuint pitch = radeonScreen->frontPitch;
   GLuint ba, address = 0;			/* a[0]    = 0           */

   ba = (y / 16) * (pitch / 32) + (x / 32);

   address |= (x & 0x7) << 1;			/* a[1..3] = x[0..2]     */
   address |= (y & 0x7) << 4;			/* a[4..6] = y[0..2]     */
   address |= (x & 0x8) << 4;			/* a[7]    = x[3]        */
   address |= (ba & 0x3) << 8;			/* a[8..9] = ba[0..1]    */
   address |= (y & 0x8) << 7;			/* a[10]   = y[3]        */
   address |= ((x & 0x10) ^ (y & 0x10)) << 7;	/* a[11]   = x[4] ^ y[4] */
   address |= (ba & ~0x3) << 10;		/* a[12..] = ba[2..]     */

   return address;
}

static GLuint radeon_mba_z32( radeonContextPtr rmesa,
				       GLint x, GLint y )
{
   radeonScreenPtr radeonScreen = rmesa->radeonScreen;
   GLuint pitch = radeonScreen->frontPitch;
   GLuint ba, address = 0;			/* a[0..1] = 0           */

   ba = (y / 16) * (pitch / 16) + (x / 16);

   address |= (x & 0x7) << 2;			/* a[2..4] = x[0..2]     */
   address |= (y & 0x3) << 5;			/* a[5..6] = y[0..1]     */
   address |=
      (((x & 0x10) >> 2) ^ (y & 0x4)) << 5;	/* a[7]    = x[4] ^ y[2] */
   address |= (ba & 0x3) << 8;			/* a[8..9] = ba[0..1]    */

   address |= (y & 0x8) << 7;			/* a[10]   = y[3]        */
   address |=
      (((x & 0x8) << 1) ^ (y & 0x10)) << 7;	/* a[11]   = x[3] ^ y[4] */
   address |= (ba & ~0x3) << 10;		/* a[12..] = ba[2..]     */

   return address;
}


/* 16-bit depth buffer functions
 */
#define WRITE_DEPTH( _x, _y, d )					\
   *(GLushort *)(buf + radeon_mba_z16( rmesa, _x + xo, _y + yo )) = d;

#define READ_DEPTH( d, _x, _y )						\
   d = *(GLushort *)(buf + radeon_mba_z16( rmesa, _x + xo, _y + yo ));

#define TAG(x) radeon##x##_16
#include "depthtmp.h"

/* 24 bit depth, 8 bit stencil depthbuffer functions
 */
#define WRITE_DEPTH( _x, _y, d )					\
do {									\
   GLuint offset = radeon_mba_z32( rmesa, _x + xo, _y + yo );		\
   GLuint tmp = *(GLuint *)(buf + offset);				\
   tmp &= 0xff000000;							\
   tmp |= ((d) & 0x00ffffff);						\
   *(GLuint *)(buf + offset) = tmp;					\
} while (0)

#define READ_DEPTH( d, _x, _y )						\
   d = *(GLuint *)(buf + radeon_mba_z32( rmesa, _x + xo,		\
					 _y + yo )) & 0x00ffffff;

#define TAG(x) radeon##x##_24_8
#include "depthtmp.h"


/* ================================================================
 * Stencil buffer
 */

#if 0
/* 24 bit depth, 8 bit stencil depthbuffer functions
 */
#define WRITE_STENCIL( _x, _y, d )					\
do {									\
   GLuint offset = radeon_mba_z32( rmesa, _x + xo, _y + yo );		\
   GLuint tmp = *(GLuint *)(buf + offset);				\
   tmp &= 0x00ffffff;							\
   tmp |= (((d) & 0xff) << 24);						\
   *(GLuint *)(buf + offset) = tmp;					\
} while (0)

#define READ_STENCIL( d, _x, _y )					\
do {									\
   GLuint offset = radeon_mba_z32( rmesa, _x + xo, _y + yo );		\
   GLuint tmp = *(GLuint *)(buf + offset);				\
   tmp &= 0xff000000;							\
   d = tmp >> 24;							\
} while (0)

#define TAG(x) radeon##x##_24_8
#include "stenciltmp.h"
#endif


void radeonDDInitSpanFuncs( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   switch ( rmesa->radeonScreen->cpp ) {
   case 2:
      ctx->Driver.WriteRGBASpan		= radeonWriteRGBASpan_RGB565;
      ctx->Driver.WriteRGBSpan		= radeonWriteRGBSpan_RGB565;
      ctx->Driver.WriteMonoRGBASpan	= radeonWriteMonoRGBASpan_RGB565;
      ctx->Driver.WriteRGBAPixels	= radeonWriteRGBAPixels_RGB565;
      ctx->Driver.WriteMonoRGBAPixels	= radeonWriteMonoRGBAPixels_RGB565;
      ctx->Driver.ReadRGBASpan		= radeonReadRGBASpan_RGB565;
      ctx->Driver.ReadRGBAPixels	= radeonReadRGBAPixels_RGB565;
      break;

   case 4:
      ctx->Driver.WriteRGBASpan		= radeonWriteRGBASpan_ARGB8888;
      ctx->Driver.WriteRGBSpan		= radeonWriteRGBSpan_ARGB8888;
      ctx->Driver.WriteMonoRGBASpan	= radeonWriteMonoRGBASpan_ARGB8888;
      ctx->Driver.WriteRGBAPixels	= radeonWriteRGBAPixels_ARGB8888;
      ctx->Driver.WriteMonoRGBAPixels	= radeonWriteMonoRGBAPixels_ARGB8888;
      ctx->Driver.ReadRGBASpan		= radeonReadRGBASpan_ARGB8888;
      ctx->Driver.ReadRGBAPixels	= radeonReadRGBAPixels_ARGB8888;
      break;

   default:
      break;
   }

   switch ( rmesa->glCtx->Visual->DepthBits ) {
   case 16:
      ctx->Driver.ReadDepthSpan		= radeonReadDepthSpan_16;
      ctx->Driver.WriteDepthSpan	= radeonWriteDepthSpan_16;
      ctx->Driver.ReadDepthPixels	= radeonReadDepthPixels_16;
      ctx->Driver.WriteDepthPixels	= radeonWriteDepthPixels_16;
      break;

    case 24:
	ctx->Driver.ReadDepthSpan	= radeonReadDepthSpan_24_8;
	ctx->Driver.WriteDepthSpan	= radeonWriteDepthSpan_24_8;
	ctx->Driver.ReadDepthPixels	= radeonReadDepthPixels_24_8;
	ctx->Driver.WriteDepthPixels	= radeonWriteDepthPixels_24_8;

#if 0 /* only need these for hardware stencil buffers */
	ctx->Driver.ReadStencilSpan	= radeonReadStencilSpan_24_8;
	ctx->Driver.WriteStencilSpan	= radeonWriteStencilSpan_24_8;
	ctx->Driver.ReadStencilPixels	= radeonReadStencilPixels_24_8;
	ctx->Driver.WriteStencilPixels	= radeonWriteStencilPixels_24_8;
#endif
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
