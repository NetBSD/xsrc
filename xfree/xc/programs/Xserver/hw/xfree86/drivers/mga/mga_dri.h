/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/mga/mga_dri.h,v 1.3 2000/06/22 03:58:25 tsi Exp $ */

#ifndef _MGA_DRI_
#define _MGA_DRI_

#include <xf86drm.h>
#include <xf86drmMga.h>

#define MGA_MAX_DRAWABLES 256

typedef struct {
   int reserved_map_agpstart;
   int reserved_map_idx;
   int buffer_map_idx;
   int sarea_priv_offset;
   int primary_size;
   int warp_ucode_size;
   int chipset;
   int sgram;
   unsigned long agpMode;
   unsigned long agpHandle;
   Bool agpAcquired;
   drmHandle agp_private;
   drmSize agpSizep;
   drmAddress agpBase;
   int irq;
   drmHandle regs;
   drmSize regsSize;
   drmAddress regsMap;
   drmMgaWarpIndex WarpIndex[MGA_MAX_WARP_PIPES];
   drmBufMapPtr drmBufs;
   CARD8 *agp_map;
} MGADRIServerPrivateRec, *MGADRIServerPrivatePtr;

typedef struct {
   int chipset;
   int width;
   int height;
   int mem;
   int cpp;
   unsigned int frontOffset;
   unsigned int frontPitch;

   unsigned int backOffset;
   unsigned int backPitch;

   unsigned int depthOffset;
   unsigned int depthPitch;

   unsigned int textureOffset;
   unsigned int textureSize;
   int logTextureGranularity;

   /* Allow calculation of setup dma addresses.
    */
   unsigned int agpBufferOffset;

   unsigned int agpTextureOffset;
   unsigned int agpTextureSize;
   int logAgpTextureGranularity;

   /* Redundant?
    */
   unsigned int frontOrg;
   unsigned int backOrg;
   unsigned int depthOrg;

   unsigned int mAccess;

   drmHandle agp;
   drmSize agpSize;
} MGADRIRec, *MGADRIPtr;


/* WARNING: Do not change the SAREA structure without changing the kernel
 * as well */
typedef struct {
   unsigned char next, prev;
   unsigned char in_use;
   unsigned int age;
} MGATexRegionRec, *MGATexRegionPtr;

typedef struct {
   /* The channel for communication of state information to the kernel
    * on firing a vertex dma buffer.
    */
   unsigned int ContextState[MGA_CTX_SETUP_SIZE];
   unsigned int ServerState[MGA_2D_SETUP_SIZE];
   unsigned int TexState[2][MGA_TEX_SETUP_SIZE];
   unsigned int WarpPipe;
   unsigned int dirty;
   
   unsigned int nbox;
   XF86DRIClipRectRec boxes[MGA_NR_SAREA_CLIPRECTS];
   
   /* Information about the most recently used 3d drawable.  The
    * client fills in the req_* fields, the server fills in the 
    * exported_ fields and puts the cliprects into boxes, above.
    *
    * The client clears the exported_drawable field before
    * clobbering the boxes data.
    */
   unsigned int req_drawable;       /* the X drawable id */
   unsigned int req_draw_buffer;    /* MGA_FRONT or MGA_BACK */
   
   unsigned int exported_drawable;
   unsigned int exported_index;
   unsigned int exported_stamp;
   unsigned int exported_buffers;
   int exported_nfront;
   int exported_nback;
   int exported_back_x, exported_front_x, exported_w;
   int exported_back_y, exported_front_y, exported_h;
   XF86DRIClipRectRec exported_boxes[MGA_NR_SAREA_CLIPRECTS];
   
   /* Counters for aging textures and for client-side throttling.
    */
   unsigned int last_enqueue;       /* last time a buffer was enqueued */
   unsigned int last_dispatch;      /* age of the most recently dispatched buffer */
   unsigned int last_quiescent;     /*  */
   
   /* LRU lists for texture memory in agp space and on the card */
   
   MGATexRegionRec texList[MGA_NR_TEX_HEAPS][MGA_NR_TEX_REGIONS+1];
   unsigned int texAge[MGA_NR_TEX_HEAPS];
   /* Mechanism to validate card state.
    */
   int ctxOwner;
} MGASAREARec, *MGASAREAPtr;

typedef struct {
  /* Nothing here yet */
  int dummy;
} MGAConfigPrivRec, *MGAConfigPrivPtr;

typedef struct {
  /* Nothing here yet */
  int dummy;
} MGADRIContextRec, *MGADRIContextPtr;

#endif
