#include "types.h"
#include "i810dd.h"
#include "i810log.h"
#include "i810span.h"
#include "i810ioctl.h"


#define DBG 0

#define LOCAL_VARS					\
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;	\
   i810ScreenPrivate *i810Screen = imesa->i810Screen;	\
   GLuint pitch = i810Screen->backPitch;		\
   GLuint height = dPriv->h;				\
   char *buf = (char *)(imesa->drawMap +		\
			dPriv->x * 2 +			\
			dPriv->y * pitch);		\
   char *read_buf = (char *)(imesa->readMap +		\
			     dPriv->x * 2 +		\
			     dPriv->y * pitch); 	\
   GLushort p = I810_CONTEXT( ctx )->MonoColor;         \
   (void) read_buf; (void) buf; (void) p

#define LOCAL_DEPTH_VARS				\
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;	\
   i810ScreenPrivate *i810Screen = imesa->i810Screen;	\
   GLuint pitch = i810Screen->backPitch;		\
   GLuint height = dPriv->h;				\
   char *buf = (char *)(i810Screen->depth.map +	\
			dPriv->x * 2 +			\
			dPriv->y * pitch)

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

#define Y_FLIP(_y) (height - _y - 1)


#define HW_LOCK()				\
   i810ContextPtr imesa = I810_CONTEXT(ctx);	\
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
   rgba[0] = (p >> 8) & 0xf8;					\
   rgba[1] = (p >> 3) & 0xfc;					\
   rgba[2] = (p << 3) & 0xf8;					\
   rgba[3] = 255;						\
} while(0)

#define TAG(x) i810##x##_565
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

#define TAG(x) i810##x##_555
#include "spantmp.h"




/* 16 bit depthbuffer functions.
 */
#define WRITE_DEPTH( _x, _y, d ) \
   *(GLushort *)(buf + _x*2 + _y*pitch)  = d;

#define READ_DEPTH( d, _x, _y )	\
   d = *(GLushort *)(buf + _x*2 + _y*pitch);	 

/*     d = 0xffff; */

#define TAG(x) i810##x##_16
#include "depthtmp.h"



void i810DDInitSpanFuncs( GLcontext *ctx )
{
   if (1) {
      ctx->Driver.WriteRGBASpan = i810WriteRGBASpan_565;
      ctx->Driver.WriteRGBSpan = i810WriteRGBSpan_565;
      ctx->Driver.WriteMonoRGBASpan = i810WriteMonoRGBASpan_565;
      ctx->Driver.WriteRGBAPixels = i810WriteRGBAPixels_565;
      ctx->Driver.WriteMonoRGBAPixels = i810WriteMonoRGBAPixels_565; 
      ctx->Driver.ReadRGBASpan = i810ReadRGBASpan_565;
      ctx->Driver.ReadRGBAPixels = i810ReadRGBAPixels_565;
   } else {
      ctx->Driver.WriteRGBASpan = i810WriteRGBASpan_555;
      ctx->Driver.WriteRGBSpan = i810WriteRGBSpan_555;
      ctx->Driver.WriteMonoRGBASpan = i810WriteMonoRGBASpan_555;
      ctx->Driver.WriteRGBAPixels = i810WriteRGBAPixels_555;
      ctx->Driver.WriteMonoRGBAPixels = i810WriteMonoRGBAPixels_555;
      ctx->Driver.ReadRGBASpan = i810ReadRGBASpan_555;
      ctx->Driver.ReadRGBAPixels = i810ReadRGBAPixels_555;
   }

   ctx->Driver.ReadDepthSpan = i810ReadDepthSpan_16;
   ctx->Driver.WriteDepthSpan = i810WriteDepthSpan_16;
   ctx->Driver.ReadDepthPixels = i810ReadDepthPixels_16;
   ctx->Driver.WriteDepthPixels = i810WriteDepthPixels_16;

   ctx->Driver.WriteCI8Span        =NULL;
   ctx->Driver.WriteCI32Span       =NULL;
   ctx->Driver.WriteMonoCISpan     =NULL;
   ctx->Driver.WriteCI32Pixels     =NULL;
   ctx->Driver.WriteMonoCIPixels   =NULL;
   ctx->Driver.ReadCI32Span        =NULL;
   ctx->Driver.ReadCI32Pixels      =NULL;
}
