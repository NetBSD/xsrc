/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/xf86drmMga.h,v 3.3 2000/09/26 15:57:19 tsi Exp $ */

/*
 * WARNING: If you change any of these defines, make sure to change
 * the kernel include file as well (mga_drm.h)
 */

#ifndef _XF86DRI_MGA_H_
#define _XF86DRI_MGA_H_
#ifndef _MGA_DEFINES_
#define _MGA_DEFINES_
#define MGA_F  0x1		/* fog */
#define MGA_A  0x2		/* alpha */
#define MGA_S  0x4		/* specular */
#define MGA_T2 0x8		/* multitexture */

#define MGA_WARP_TGZ            0
#define MGA_WARP_TGZF           (MGA_F)
#define MGA_WARP_TGZA           (MGA_A)
#define MGA_WARP_TGZAF          (MGA_F|MGA_A)
#define MGA_WARP_TGZS           (MGA_S)
#define MGA_WARP_TGZSF          (MGA_S|MGA_F)
#define MGA_WARP_TGZSA          (MGA_S|MGA_A)
#define MGA_WARP_TGZSAF         (MGA_S|MGA_F|MGA_A)
#define MGA_WARP_T2GZ           (MGA_T2)
#define MGA_WARP_T2GZF          (MGA_T2|MGA_F)
#define MGA_WARP_T2GZA          (MGA_T2|MGA_A)
#define MGA_WARP_T2GZAF         (MGA_T2|MGA_A|MGA_F)
#define MGA_WARP_T2GZS          (MGA_T2|MGA_S)
#define MGA_WARP_T2GZSF         (MGA_T2|MGA_S|MGA_F)
#define MGA_WARP_T2GZSA         (MGA_T2|MGA_S|MGA_A)
#define MGA_WARP_T2GZSAF        (MGA_T2|MGA_S|MGA_F|MGA_A)

#define MGA_MAX_G400_PIPES 16
#define MGA_MAX_G200_PIPES  8	/* no multitex */

#define MGA_MAX_WARP_PIPES MGA_MAX_G400_PIPES

#define MGA_CARD_TYPE_G200 1
#define MGA_CARD_TYPE_G400 2
#define MGA_FRONT   0x1
#define MGA_BACK    0x2
#define MGA_DEPTH   0x4

/* 3d state excluding texture units:
 */
#define MGA_CTXREG_DSTORG   0	/* validated */
#define MGA_CTXREG_MACCESS  1	
#define MGA_CTXREG_PLNWT    2	
#define MGA_CTXREG_DWGCTL    3	
#define MGA_CTXREG_ALPHACTRL 4
#define MGA_CTXREG_FOGCOLOR  5
#define MGA_CTXREG_WFLAG     6
#define MGA_CTXREG_TDUAL0    7
#define MGA_CTXREG_TDUAL1    8
#define MGA_CTXREG_FCOL      9
#define MGA_CTXREG_STENCIL    10
#define MGA_CTXREG_STENCILCTL 11
#define MGA_CTX_SETUP_SIZE   12

/* 2d state
 */
#define MGA_2DREG_PITCH 	0
#define MGA_2D_SETUP_SIZE 	1

/* Each texture unit has a state:
 */
#define MGA_TEXREG_CTL        0
#define MGA_TEXREG_CTL2       1
#define MGA_TEXREG_FILTER     2
#define MGA_TEXREG_BORDERCOL  3
#define MGA_TEXREG_ORG        4 /* validated */
#define MGA_TEXREG_ORG1       5
#define MGA_TEXREG_ORG2       6
#define MGA_TEXREG_ORG3       7
#define MGA_TEXREG_ORG4       8
#define MGA_TEXREG_WIDTH      9
#define MGA_TEXREG_HEIGHT     10
#define MGA_TEX_SETUP_SIZE    11

/* What needs to be changed for the current vertex dma buffer?
 */
#define MGA_UPLOAD_CTX        0x1
#define MGA_UPLOAD_TEX0       0x2
#define MGA_UPLOAD_TEX1       0x4
#define MGA_UPLOAD_PIPE       0x8
#define MGA_UPLOAD_TEX0IMAGE  0x10
#define MGA_UPLOAD_TEX1IMAGE  0x20
#define MGA_UPLOAD_2D 	      0x40
#define MGA_WAIT_AGE          0x80 /* handled client-side */
#define MGA_UPLOAD_CLIPRECTS  0x100 /* handled client-side */
#define MGA_DMA_FLUSH	      0x200 /* set when someone gets the lock
                                       quiescent */

/* 32 buffers of 64k each, total 1 meg.
 */
#define MGA_DMA_BUF_ORDER     16
#define MGA_DMA_BUF_SZ        (1<<MGA_DMA_BUF_ORDER)
#define MGA_DMA_BUF_NR        31

/* Keep these small for testing.
 */
#define MGA_NR_SAREA_CLIPRECTS 8

/* 2 heaps (1 for card, 1 for agp), each divided into upto 128
 * regions, subject to a minimum region size of (1<<16) == 64k. 
 *
 * Clients may subdivide regions internally, but when sharing between
 * clients, the region size is the minimum granularity. 
 */

#define MGA_CARD_HEAP 0
#define MGA_AGP_HEAP  1
#define MGA_NR_TEX_HEAPS 2
#define MGA_NR_TEX_REGIONS 16
#define MGA_LOG_MIN_TEX_REGION_SIZE 16
#endif

typedef struct _drmMgaWarpIndex {
   int installed;
   unsigned long phys_addr;
   int size;
} drmMgaWarpIndex;

typedef struct _drmMgaInit {
   int reserved_map_agpstart;
   int reserved_map_idx;
   int buffer_map_idx;
   int sarea_priv_offset;
   int primary_size;
   int warp_ucode_size;
   unsigned int frontOffset;
   unsigned int backOffset;
   unsigned int depthOffset;
   unsigned int textureOffset;
   unsigned int textureSize;
   unsigned int agpTextureSize;
   unsigned int agpTextureOffset;
   unsigned int cpp;
   unsigned int stride;
   int sgram;
   int chipset;
   drmMgaWarpIndex WarpIndex[MGA_MAX_WARP_PIPES];
   unsigned int mAccess;
} drmMgaInit;


Bool drmMgaCleanupDma(int driSubFD);
Bool drmMgaLockUpdate(int driSubFD, drmLockFlags flags);
Bool drmMgaInitDma(int driSubFD, drmMgaInit *info);
#endif
