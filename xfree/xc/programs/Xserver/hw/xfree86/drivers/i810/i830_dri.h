/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/i810/i830_dri.h,v 1.2 2001/10/28 03:33:33 tsi Exp $ */

#ifndef _I830_DRI_H
#define _I830_DRI_H

#include <xf86drm.h>
#include <xf86drmI830.h>

#define I830_MAX_DRAWABLES 256

#define I830_MAJOR_VERSION 1
#define I830_MINOR_VERSION 0
#define I830_PATCHLEVEL 0

#define I830_REG_SIZE 0x80000

typedef struct _I830DRIRec {
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
   int sarea_priv_offset;
} I830DRIRec, *I830DRIPtr;

typedef struct {
  /* Nothing here yet */
  int dummy;
} I830ConfigPrivRec, *I830ConfigPrivPtr;

typedef struct {
  /* Nothing here yet */
  int dummy;
} I830DRIContextRec, *I830DRIContextPtr;

/* Warning: If you change the SAREA structure you must change the kernel
 * structure as well */

typedef struct _I830TexRegion {
	unsigned char next, prev; /* indices to form a circular LRU  */
	unsigned char in_use;	/* owned by a client, or free? */
	int age;		/* tracked by clients to update local LRU's */
} I830TexRegion;

typedef struct _I830SAREA {
	unsigned int ContextState[I830_CTX_SETUP_SIZE];
   	unsigned int BufferState[I830_DEST_SETUP_SIZE];
	unsigned int TexState[I830_TEXTURE_COUNT][I830_TEX_SETUP_SIZE];
	unsigned int TexBlendState[I830_TEXBLEND_COUNT][I830_TEXBLEND_SIZE];
	unsigned int TexBlendStateWordsUsed[I830_TEXBLEND_COUNT];
	unsigned int Palette[2][256];
   	unsigned int dirty;

	unsigned int nbox;
	XF86DRIClipRectRec boxes[I830_NR_SAREA_CLIPRECTS];

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

	I830TexRegion texList[I830_NR_TEX_REGIONS+1]; 
				/* Last elt is sentinal */
        int texAge;		/* last time texture was uploaded */
        int last_enqueue;	/* last time a buffer was enqueued */
	int last_dispatch;	/* age of the most recently dispatched buffer */
	int last_quiescent;     /*  */
	int ctxOwner;		/* last context to upload state */

	int vertex_prim;
} I830SAREARec, *I830SAREAPtr;

#endif
