/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgaspan.c,v 1.5 2000/09/24 13:51:07 alanh Exp $ */

#include "types.h"
#include "mgadd.h"
#include "mgacontext.h"
#include "mgaspan.h"
#include "mgaioctl.h"

#define DBG 0


#define LOCAL_VARS					\
   __DRIdrawablePrivate *dPriv = mmesa->driDrawable;	\
   mgaScreenPrivate *mgaScreen = mmesa->mgaScreen;	\
   __DRIscreenPrivate *sPriv = mmesa->driScreen;	\
   GLuint pitch = mgaScreen->frontPitch;		\
   GLuint height = dPriv->h;				\
   char *read_buf = (char *)(sPriv->pFB +		\
			mmesa->readOffset +		\
			dPriv->x * mgaScreen->cpp +	\
			dPriv->y * pitch);		\
   char *buf = (char *)(sPriv->pFB +			\
			mmesa->drawOffset +		\
			dPriv->x * mgaScreen->cpp +	\
			dPriv->y * pitch);		\
   GLushort p = MGA_CONTEXT( ctx )->MonoColor;		\
   (void) read_buf; (void) buf; (void) p
   


#define LOCAL_DEPTH_VARS				\
   __DRIdrawablePrivate *dPriv = mmesa->driDrawable;	\
   mgaScreenPrivate *mgaScreen = mmesa->mgaScreen;	\
   __DRIscreenPrivate *sPriv = mmesa->driScreen;	\
   GLuint pitch = mgaScreen->frontPitch;		\
   GLuint height = dPriv->h;				\
   char *buf = (char *)(sPriv->pFB +			\
			mgaScreen->depthOffset +	\
			dPriv->x * 2 +			\
			dPriv->y * pitch)

#define LOCAL_STENCIL_VARS LOCAL_DEPTH_VARS 

#define INIT_MONO_PIXEL(p) 

#define CLIPPIXEL(_x,_y) (_x >= minx && _x < maxx && \
			  _y >= miny && _y < maxy)


#define CLIPSPAN(_x,_y,_n,_x1,_n1,_i)				\
	 if (_y < miny || _y >= maxy) _n1 = 0, _x1 = x;		\
         else {							\
            _n1 = _n;						\
	    _x1 = _x;						\
	    if (_x1 < minx) _i += (minx - _x1), _x1 = minx;	\
	    if (_x1 + _n1 >= maxx) n1 -= (_x1 + n1 - maxx) + 1;	\
         }



#define HW_LOCK()				\
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);	\
   LOCK_HARDWARE_QUIESCENT(mmesa);

#define HW_CLIPLOOP()						\
  do {								\
    int _nc = mmesa->numClipRects;				\
    while (_nc--) {						\
       int minx = mmesa->pClipRects[_nc].x1 - mmesa->drawX;	\
       int miny = mmesa->pClipRects[_nc].y1 - mmesa->drawY;	\
       int maxx = mmesa->pClipRects[_nc].x2 - mmesa->drawX;	\
       int maxy = mmesa->pClipRects[_nc].y2 - mmesa->drawY;

#define HW_ENDCLIPLOOP()			\
    }						\
  } while (0)

#define HW_UNLOCK()				\
    UNLOCK_HARDWARE(mmesa);







/* 16 bit, 565 rgb color spanline and pixel functions
 */
#define Y_FLIP(_y) (height - _y - 1)
#define WRITE_RGBA( _x, _y, r, g, b, a )				\
   *(GLushort *)(buf + _x*2 + _y*pitch)  = ( (((int)r & 0xf8) << 8) |	\
		                             (((int)g & 0xfc) << 3) |	\
		                             (((int)b & 0xf8) >> 3))

#define WRITE_PIXEL( _x, _y, p )  \
   *(GLushort *)(buf + _x*2 + _y*pitch) = p

#define READ_RGBA( rgba, _x, _y )				\
do {								\
   GLushort p = *(GLushort *)(read_buf + _x*2 + _y*pitch);	\
   rgba[0] = (p >> 8) & 0xf8;					\
   rgba[1] = (p >> 3) & 0xfc;					\
   rgba[2] = (p << 3) & 0xf8;					\
   rgba[3] = 255;						\
} while(0)

#define TAG(x) mga##x##_565
#include "spantmp.h"





/* 32 bit, 8888 argb color spanline and pixel functions
 */
#define WRITE_RGBA(_x, _y, r, g, b, a)			\
    *(GLuint *)(buf + _x*4 + _y*pitch) = ((r << 16) |	\
					  (g << 8)  |	\
					  (b << 0)  |	\
					  (a << 24) )

#define WRITE_PIXEL(_x, _y, p)			\
    *(GLuint *)(buf + _x*4 + _y*pitch) = p

#define READ_RGBA(rgba, _x, _y)					\
    do {							\
	GLuint p = *(GLuint *)(read_buf + _x*4 + _y*pitch);	\
	rgba[0] = (p >> 16) & 0xff;				\
	rgba[1] = (p >> 8)  & 0xff;				\
	rgba[2] = (p >> 0)  & 0xff;				\
	rgba[3] = (p >> 24) & 0xff;				\
    } while (0)

#define TAG(x) mga##x##_8888
#include "spantmp.h"




/* 16 bit depthbuffer functions.
 */
#define WRITE_DEPTH( _x, _y, d )	\
   *(GLushort *)(buf + _x*2 + _y*pitch) = d;

#define READ_DEPTH( d, _x, _y )		\
   d = *(GLushort *)(buf + _x*2 + _y*pitch);	

#define TAG(x) mga##x##_16
#include "depthtmp.h"




/* 32 bit depthbuffer functions.
 */
#define WRITE_DEPTH( _x, _y, d )	\
   *(GLuint *)(buf + _x*4 + _y*pitch) = d;

#define READ_DEPTH( d, _x, _y )		\
   d = *(GLuint *)(buf + _x*4 + _y*pitch);	

#define TAG(x) mga##x##_32
#include "depthtmp.h"



/* 24/8 bit interleaved depth/stencil functions
 */
#define WRITE_DEPTH( _x, _y, d ) {			\
   GLuint tmp = *(GLuint *)(buf + _x*4 + _y*pitch);	\
   tmp &= 0xff;						\
   tmp |= (d) & 0xffffff00;				\
   *(GLuint *)(buf + _x*4 + _y*pitch) = tmp;		\
}

#define READ_DEPTH( d, _x, _y )		\
   d = *(GLuint *)(buf + _x*4 + _y*pitch) >> 8;	


#define TAG(x) mga##x##_24_8
#include "depthtmp.h"

#define WRITE_STENCIL( _x, _y, d ) {			\
   GLuint tmp = *(GLuint *)(buf + _x*4 + _y*pitch);	\
   tmp &= 0xffffff00;					\
   tmp |= d & 0xff;					\
   *(GLuint *)(buf + _x*4 + _y*pitch) = tmp;		\
}

#define READ_STENCIL( d, _x, _y )		\
   d = *(GLuint *)(buf + _x*4 + _y*pitch) & 0xff;	

#define TAG(x) mga##x##_24_8
#include "stenciltmp.h"




void mgaDDInitSpanFuncs( GLcontext *ctx )
{
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);

   switch (mmesa->mgaScreen->cpp) {
   case 2:
      ctx->Driver.WriteRGBASpan = mgaWriteRGBASpan_565;
      ctx->Driver.WriteRGBSpan = mgaWriteRGBSpan_565;
      ctx->Driver.WriteMonoRGBASpan = mgaWriteMonoRGBASpan_565;
      ctx->Driver.WriteRGBAPixels = mgaWriteRGBAPixels_565;
      ctx->Driver.WriteMonoRGBAPixels = mgaWriteMonoRGBAPixels_565;
      ctx->Driver.ReadRGBASpan = mgaReadRGBASpan_565;
      ctx->Driver.ReadRGBAPixels = mgaReadRGBAPixels_565;

      ctx->Driver.ReadDepthSpan = mgaReadDepthSpan_16;
      ctx->Driver.WriteDepthSpan = mgaWriteDepthSpan_16;
      ctx->Driver.ReadDepthPixels = mgaReadDepthPixels_16;
      ctx->Driver.WriteDepthPixels = mgaWriteDepthPixels_16;
      break;

   case 4:
      ctx->Driver.WriteRGBASpan = mgaWriteRGBASpan_8888;
      ctx->Driver.WriteRGBSpan = mgaWriteRGBSpan_8888;
      ctx->Driver.WriteMonoRGBASpan = mgaWriteMonoRGBASpan_8888;
      ctx->Driver.WriteRGBAPixels = mgaWriteRGBAPixels_8888;
      ctx->Driver.WriteMonoRGBAPixels = mgaWriteMonoRGBAPixels_8888;
      ctx->Driver.ReadRGBASpan = mgaReadRGBASpan_8888;
      ctx->Driver.ReadRGBAPixels = mgaReadRGBAPixels_8888;
      
      if (mmesa->hw_stencil) {
	 ctx->Driver.ReadDepthSpan = mgaReadDepthSpan_32;
	 ctx->Driver.WriteDepthSpan = mgaWriteDepthSpan_32;
	 ctx->Driver.ReadDepthPixels = mgaReadDepthPixels_32;
	 ctx->Driver.WriteDepthPixels = mgaWriteDepthPixels_32;
      } else {
	 ctx->Driver.ReadDepthSpan = mgaReadDepthSpan_24_8;
	 ctx->Driver.WriteDepthSpan = mgaWriteDepthSpan_24_8;
	 ctx->Driver.ReadDepthPixels = mgaReadDepthPixels_24_8;
	 ctx->Driver.WriteDepthPixels = mgaWriteDepthPixels_24_8;

	 ctx->Driver.ReadStencilSpan = mgaReadStencilSpan_24_8;
	 ctx->Driver.WriteStencilSpan = mgaWriteStencilSpan_24_8;
	 ctx->Driver.ReadStencilPixels = mgaReadStencilPixels_24_8;
	 ctx->Driver.WriteStencilPixels = mgaWriteStencilPixels_24_8;
      }
      break;
   }


   ctx->Driver.WriteCI8Span = 0;
   ctx->Driver.WriteCI32Span = 0;
   ctx->Driver.WriteMonoCISpan = 0;
   ctx->Driver.WriteCI32Pixels = 0;
   ctx->Driver.WriteMonoCIPixels = 0;
   ctx->Driver.ReadCI32Span = 0;
   ctx->Driver.ReadCI32Pixels = 0;
}
