/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/xf86drmMga.h,v 3.6 2001/04/16 15:02:13 tsi Exp $ */

#ifndef __XF86DRI_MGA_H__
#define __XF86DRI_MGA_H__

/*
 * WARNING: If you change any of these defines, make sure to change
 * the kernel include file as well (mga_drm.h)
 */

typedef struct {
   int installed;
   unsigned long phys_addr;
   int size;
} drmMGAWarpIndex;

typedef struct {
   unsigned long sarea_priv_offset;

   int chipset;
   int sgram;

   unsigned int maccess;

   unsigned int fb_cpp;
   unsigned int front_offset, front_pitch;
   unsigned int back_offset, back_pitch;

   unsigned int depth_cpp;
   unsigned int depth_offset, depth_pitch;

   unsigned int texture_offset[2];
   unsigned int texture_size[2];

   unsigned long fb_offset;
   unsigned long mmio_offset;
   unsigned long status_offset;
   unsigned long warp_offset;
   unsigned long primary_offset;
   unsigned long buffers_offset;
} drmMGAInit;

extern int drmMGAInitDMA( int fd, drmMGAInit *info );
extern int drmMGACleanupDMA( int fd );

extern int drmMGAFlushDMA( int fd, drmLockFlags flags );

extern int drmMGAEngineReset( int fd );

extern int drmMGAFullScreen( int fd, int enable );

extern int drmMGASwapBuffers( int fd );
extern int drmMGAClear( int fd, unsigned int flags,
			unsigned int clear_color, unsigned int clear_depth,
			unsigned int color_mask, unsigned int depth_mask );

extern int drmMGAFlushVertexBuffer( int fd, int indx, int used, int discard );
extern int drmMGAFlushIndices( int fd, int indx,
			       int start, int end, int discard );

extern int drmMGATextureLoad( int fd, int indx,
			      unsigned int dstorg, unsigned int length );

extern int drmMGAAgpBlit( int fd, unsigned int planemask,
			  unsigned int src, int src_pitch,
			  unsigned int dst, int dst_pitch,
			  int delta_sx, int delta_sy,
			  int delta_dx, int delta_dy,
			  int height, int ydir );

#endif
