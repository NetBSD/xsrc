/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/xf86drmI810.h,v 3.3 2001/04/01 14:00:14 tsi Exp $ */

/* WARNING: If you change any of these defines, make sure to change
 * the kernel include file as well (i810_drm.h)
 */

#ifndef _XF86DRI_I810_H_
#define _XF86DRI_I810_H_

#ifndef _I810_DEFINES_
#define _I810_DEFINES_
#define I810_USE_BATCH 1

#define I810_DMA_BUF_ORDER     12
#define I810_DMA_BUF_SZ        (1<<I810_DMA_BUF_ORDER)
#define I810_DMA_BUF_NR        256

#define I810_NR_SAREA_CLIPRECTS 8

/* Each region is a minimum of 64k, and there are at most 64 of them.
 */
#define I810_NR_TEX_REGIONS 64
#define I810_LOG_MIN_TEX_REGION_SIZE 16
#endif

typedef struct _drmI810Init {
   unsigned int start; 
   unsigned int end; 
   unsigned int size;
   unsigned int mmio_offset;
   unsigned int buffers_offset;
   int sarea_off;

   unsigned int front_offset;
   unsigned int back_offset;
   unsigned int depth_offset;
   unsigned int w;
   unsigned int h;
   unsigned int pitch;
   unsigned int pitch_bits;
} drmI810Init;


Bool drmI810CleanupDma(int driSubFD);
Bool drmI810InitDma(int driSubFD, drmI810Init *info );

#endif
