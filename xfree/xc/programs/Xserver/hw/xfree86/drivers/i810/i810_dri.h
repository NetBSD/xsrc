/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/i810/i810_dri.h,v 1.3 2000/06/17 00:03:18 martin Exp $ */

#ifndef _I810_DRI_
#define _I810_DRI_

#include <xf86drm.h>
#include <xf86drmI810.h>

#define I810_MAX_DRAWABLES 256

typedef struct {
   drmHandle regs;
   drmSize regsSize;
   drmAddress regsMap;

   drmSize backbufferSize;
   drmHandle backbuffer;

   drmSize depthbufferSize;
   drmHandle depthbuffer;

   drmHandle textures;
   int textureSize;

   drmHandle agp_buffers;
   drmSize agp_buf_size;
   
   int deviceID;
   int width;
   int height;
   int mem;
   int cpp;
   int bitsPerPixel;
   int fbOffset;
   int fbStride;

   int backOffset;
   int depthOffset;

   int auxPitch;
   int auxPitchBits;

   int logTextureGranularity;
   int textureOffset;

   /* For non-dma direct rendering.
    */
   int ringOffset;
   int ringSize;

   drmBufMapPtr drmBufs;
   int irq;

} I810DRIRec, *I810DRIPtr;

/* WARNING: Do not change the SAREA structure without changing the kernel
 * as well */

typedef struct {
   unsigned char next, prev; /* indices to form a circular LRU  */
   unsigned char in_use;   /* owned by a client, or free? */
   int age;                /* tracked by clients to update local LRU's */
} I810TexRegionRec, *I810TexRegionPtr;

typedef struct {
   unsigned int nbox;
   XF86DRIClipRectRec boxes[I810_NR_SAREA_CLIPRECTS];
   
   /* Maintain an LRU of contiguous regions of texture space.  If
    * you think you own a region of texture memory, and it has an
    * age different to the one you set, then you are mistaken and
    * it has been stolen by another client.  If global texAge
    * hasn't changed, there is no need to walk the list.
    *
    * These regions can be used as a proxy for the fine-grained
    * texture information of other clients - by maintaining them
    * in the same lru which is used to age their own textures,
    * clients have an approximate lru for the whole of global
    * texture space, and can make informed decisions as to which
    * areas to kick out.  There is no need to choose whether to
    * kick out your own texture or someone else's - simply eject
    * them all in LRU order.  
    */
   I810TexRegionRec texList[I810_NR_TEX_REGIONS+1]; /* Last elt is sentinal */
   
   int texAge;             /* last time texture was uploaded */
   
   int last_enqueue;       /* last time a buffer was enqueued */
   int last_dispatch;      /* age of the most recently dispatched buffer */
   int last_quiescent;     /*  */
   
   int ctxOwner;           /* last context to upload state */
} I810SAREARec, *I810SAREAPtr;

typedef struct {
  /* Nothing here yet */
  int dummy;
} I810ConfigPrivRec, *I810ConfigPrivPtr;

typedef struct {
  /* Nothing here yet */
  int dummy;
} I810DRIContextRec, *I810DRIContextPtr;


#endif
