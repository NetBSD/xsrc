 /***************************************************************************\
|*                                                                           *|
|*        Copyright (c) 1996-1998 NVIDIA, Corp.  All rights reserved.        *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.   NVIDIA, Corp. of Sunnyvale, California owns     *|
|*     the copyright  and as design patents  pending  on the design  and     *|
|*     interface  of the NV chips.   Users and possessors of this source     *|
|*     code are hereby granted  a nonexclusive,  royalty-free  copyright     *|
|*     and  design  patent license  to use this code  in individual  and     *|
|*     commercial software.                                                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*     Copyright (c) 1996-1998  NVIDIA, Corp.    NVIDIA  design  patents     *|
|*     pending in the U.S. and foreign countries.                            *|
|*                                                                           *|
|*     NVIDIA, CORP.  MAKES  NO REPRESENTATION ABOUT  THE SUITABILITY OF     *|
|*     THIS SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT     *|
|*     EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORP. DISCLAIMS     *|
|*     ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,  INCLUDING  ALL     *|
|*     IMPLIED   WARRANTIES  OF  MERCHANTABILITY  AND   FITNESS   FOR  A     *|
|*     PARTICULAR  PURPOSE.   IN NO EVENT SHALL NVIDIA, CORP.  BE LIABLE     *|
|*     FOR ANY SPECIAL, INDIRECT, INCIDENTAL,  OR CONSEQUENTIAL DAMAGES,     *|
|*     OR ANY DAMAGES  WHATSOEVER  RESULTING  FROM LOSS OF USE,  DATA OR     *|
|*     PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR OTHER     *|
|*     TORTIOUS ACTION, ARISING OUT  OF OR IN CONNECTION WITH THE USE OR     *|
|*     PERFORMANCE OF THIS SOURCE CODE.                                      *|
|*                                                                           *|
 \***************************************************************************/

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv1ref.h,v 1.1.2.3 1998/01/24 00:04:37 robin Exp $ */

#ifndef __NV1REF_H_ 
#define __NV1REF_H_ 

/* Where the hardware units are. Used to generate context */

#define NV_UBLIT                              0x00501FFF:0x00500000 
#define NV_UCLIP                              0x00451FFF:0x00450000 
#define NV_URECT                              0x004C1FFF:0x004C0000
#define NV_UROP                               0x00421FFF:0x00420000
#define NV_ULINE                              0x00491FFF:0x00490000
#define NV_ULIN                               0x004A1FFF:0x004A0000
#define NV_UBITMAP                            0x00521FFF:0x00520000
#define NV_UTRI                               0x004B1FFF:0x004B0000
#define NV_UPATT                              0x00461FFF:0x00460000

/*****************************************
 * Definitions for DAC 
 *****************************************/

#define NV_PDAC                           0x00609FFF:0x00609000   /* RW--D */
#define NV_PDAC_WRITE_PALETTE             0x609000
#define NV_PDAC_COLOUR_DATA               0x609004
#define NV_PDAC_PIXEL_MASK                0x609008
#define NV_PDAC_READ_PALETTE              0x60900c
#define NV_PDAC_INDEX_LO                  0x609010
#define NV_PDAC_INDEX_HI                  0x609014
#define NV_PDAC_INDEX_DATA                0x609018
#define NV_PDAC_GAME_PORT                 0x60901c


/* Extended register values */
#define NV_PDAC_EXT_COMPANY_ID                0x00
#define NV_PDAC_SGS_ID                        0x44

#define NV_PDAC_EXT_DEVICE_ID                 0x01
#define NV_PDAC_1764_ID                       0x64
#define NV_PDAC_1732_ID                       0x32

#define NV_PDAC_EXT_REVISION_ID               0x02

#define NV_PDAC_EXT_CONF_0                    0x04
#define NV_PDAC_CONF_0_IDC_MODE               5:5
#define NV_PDAC_CONF_0_VGA_STATE              4:4
#define NV_PDAC_CONF_0_PORT_WIDTH             3:2
#define NV_PDAC_CONF_0_VISUAL_DEPTH           1:0

#define NV_PDAC_EXT_CONF_1                    0x05
#define NV_PDAC_CONF_1_VCLK_IMPEDANCE         3:3
#define NV_PDAC_CONF_1_PCLK_VCLK_RATIO        2:0

#define NV_PDAC_EXT_RGB_PAL_CTRL                        0x09 
#define NV_PDAC_EXT_RGB_PAL_CTRL_DAC_WIDTH              7:7
#define NV_PDAC_EXT_RGB_PAL_CTRL_DAC_WIDTH_BITS_6       0x1
#define NV_PDAC_EXT_RGB_PAL_CTRL_DAC_WIDTH_BITS_8       0x0


#define NV_PDAC_EXT_VPLL_M_PARAM              0x18
#define NV_PDAC_EXT_VPLL_N_PARAM              0x19
#define NV_PDAC_EXT_VPLL_O_PARAM              0x1a
#define NV_PDAC_EXT_VPLL_P_PARAM              0x1b

#define NV_PDAC_EXT_MPLL_M_PARAM              0x10
#define NV_PDAC_EXT_MPLL_N_PARAM              0x11
#define NV_PDAC_EXT_MPLL_O_PARAM              0x12
#define NV_PDAC_EXT_MPLL_P_PARAM              0x13


/* Hardware cursor registers */
#define NV_PDAC_EXT_CURSOR_CTRL_A             0x20
#define NV_PDAC_CURSOR_CTRL_A_TYPE            1:0
#define NV_PDAC_CURSOR_CTRL_A_OFF             0
#define NV_PDAC_CURSOR_CTRL_A_XWIN            3

#define NV_PDAC_EXT_CURSOR_X_POS_LO           0x22
#define NV_PDAC_EXT_CURSOR_X_POS_HI           0x23

#define NV_PDAC_EXT_CURSOR_Y_POS_LO           0x24
#define NV_PDAC_EXT_CURSOR_Y_POS_HI           0x25

#define NV_PDAC_EXT_CURSOR_COLOUR_1_RGB       0x50
#define NV_PDAC_EXT_CURSOR_COLOUR_1_RED       0x50
#define NV_PDAC_EXT_CURSOR_COLOUR_1_GREEN     0x51
#define NV_PDAC_EXT_CURSOR_COLOUR_1_BLUE      0x52

#define NV_PDAC_EXT_CURSOR_COLOUR_2_RGB       0x54
#define NV_PDAC_EXT_CURSOR_COLOUR_2_RED       0x54
#define NV_PDAC_EXT_CURSOR_COLOUR_2_GREEN     0x55
#define NV_PDAC_EXT_CURSOR_COLOUR_2_BLUE      0x56

#define NV_PDAC_EXT_CURSOR_COLOUR_3_RGB       0x58
#define NV_PDAC_EXT_CURSOR_COLOUR_3_RED       0x58
#define NV_PDAC_EXT_CURSOR_COLOUR_3_GREEN     0x59
#define NV_PDAC_EXT_CURSOR_COLOUR_3_BLUE      0x5a

#define NV_PDAC_EXT_CURSOR_PLANE_0            0x100
#define NV_PDAC_EXT_CURSOR_PLANE_1            0x180

#define NV_PDAC_EXT_CURSOR_PLANE_0_READ       0x500
#define NV_PDAC_EXT_CURSOR_PLANE_1_READ       0x580

#define NV_PDAC_CURSOR_SIZE                   32
#define NV_PDAC_CURSOR_PLANE_SIZE         (NV_PDAC_CURSOR_SIZE*4)

/*****************************************
 * Definitions for DMA device
 *****************************************/

#define NV_PDMA                          0x00100FFF:0x00100000 /* RW--D */
#define NV_PDMA_GR_CHANNEL                               0x00100810 /* RW-4R */
#define NV_PDMA_GR_CHANNEL_ACCESS                               0:0 /* RWIVF */
#define NV_PDMA_GR_CHANNEL_ACCESS_DISABLED               0x00000000 /* RWI-V */
#define NV_PDMA_GR_CHANNEL_ACCESS_ENABLED                0x00000001 /* RW--V */
#define NV_PDMA_GR_INSTANCE                              0x00100880 /* RW-4R */
#define NV_PDMA_GR_INSTANCE_ID                                 15:0 /* RWIUF */
#define NV_PDMA_GR_INSTANCE_ID_0                         0x00000000 /* RWI-V */

/*****************************************
 * Defintions for frame buffer device
 *****************************************/

#define NV_PFB                           0x00600FFF:0x00600000

#define NV_PFB_BOOT_0                    0x600000
#define NV_PFB_BOOT_0_RAM_AMOUNT         1:0

#define NV_PFB_CONFIG_0                  0x600200
#define NV_PFB_CONFIG_0_VERTICAL         0:0     /* 1 during blank */
#define NV_PFB_CONFIG_0_PIXEL_DEPTH      9:8
#define NV_PFB_CONFIG_0_SCANLINE         20:20
#define NV_PFB_CONFIG_0_PCLK_VCLK_RATIO  26:24
#define NV_PFB_CONFIG_0_RESOLUTION       6:4

#define NV_PFB_START                     0x600400
#define NV_PFB_HOR_FRONT_PORCH           0x600500
#define NV_PFB_HOR_SYNC_WIDTH            0x600510
#define NV_PFB_HOR_BACK_PORCH            0x600520
#define NV_PFB_HOR_DISP_WIDTH            0x600530
#define NV_PFB_VER_FRONT_PORCH           0x600540
#define NV_PFB_VER_SYNC_WIDTH            0x600550
#define NV_PFB_VER_BACK_PORCH            0x600560
#define NV_PFB_VER_DISP_WIDTH            0x600570


/* Green registers */
#define NV_PFB_GREEN_0                                   0x006000C0 /* RW-4R */
#define NV_PFB_GREEN_0_LEVEL                                    1:0 /* RWIVF */
#define NV_PFB_GREEN_0_LEVEL_VIDEO_ENABLED               0x00000000 /* RW--V */
#define NV_PFB_GREEN_0_LEVEL_VIDEO_DISABLED              0x00000001 /* RW--V */
#define NV_PFB_GREEN_0_LEVEL_TIMING_DISABLED             0x00000002 /* RW--V */
#define NV_PFB_GREEN_0_LEVEL_MEMORY_DISABLED             0x00000003 /* RWI-V */
#define NV_PFB_GREEN_0_POLAR_HSYNC                            17:16 /* RWIVF */
#define NV_PFB_GREEN_0_POLAR_HSYNC_HIGH                  0x00000000 /* RWI-V */
#define NV_PFB_GREEN_0_POLAR_HSYNC_LOW                   0x00000001 /* RW--V */
#define NV_PFB_GREEN_0_POLAR_HSYNC_POSITIVE              0x00000002 /* RW--V */
#define NV_PFB_GREEN_0_POLAR_HSYNC_NEGATIVE              0x00000003 /* RW--V */
#define NV_PFB_GREEN_0_POLAR_VSYNC                            21:20 /* RWIVF */
#define NV_PFB_GREEN_0_POLAR_VSYNC_LOW                   0x00000000 /* RWI-V */
#define NV_PFB_GREEN_0_POLAR_VSYNC_HIGH                  0x00000001 /* RW--V */
#define NV_PFB_GREEN_0_POLAR_VSYNC_POSITIVE              0x00000002 /* RW--V */
#define NV_PFB_GREEN_0_POLAR_VSYNC_NEGATIVE              0x00000003 /* RW--V */
#define NV_PFB_GREEN_0_CSYNC                                  24:24 /* RWIVF */
#define NV_PFB_GREEN_0_CSYNC_DISABLED                    0x00000000 /* RWI-V */
#define NV_PFB_GREEN_0_CSYNC_ENABLED                     0x00000001 /* RW--V */


/*****************************************
 * Definitions for FIFO
 *****************************************/

#define NV_PFIFO                              0x00003FFF:0x00002000 /* RW--D */

#define NV_PFIFO_INTR_0                                  0x00002100 /* RW-4R */
#define NV_PFIFO_INTR_0_CACHE_ERROR                             0:0 /* RWXVF */
#define NV_PFIFO_INTR_0_CACHE_ERROR_NOT_PENDING          0x00000000 /* R---V */
#define NV_PFIFO_INTR_0_CACHE_ERROR_PENDING              0x00000001 /* R---V */
#define NV_PFIFO_INTR_0_CACHE_ERROR_RESET                0x00000001 /* -W--V */
#define NV_PFIFO_INTR_0_RUNOUT                                  4:4 /* RWXVF */
#define NV_PFIFO_INTR_0_RUNOUT_NOT_PENDING               0x00000000 /* R---V */
#define NV_PFIFO_INTR_0_RUNOUT_PENDING                   0x00000001 /* R---V */
#define NV_PFIFO_INTR_0_RUNOUT_RESET                     0x00000001 /* -W--V */
#define NV_PFIFO_INTR_0_RUNOUT_OVERFLOW                         8:8 /* RWXVF */
#define NV_PFIFO_INTR_0_RUNOUT_OVERFLOW_NOT_PENDING      0x00000000 /* R---V */
#define NV_PFIFO_INTR_0_RUNOUT_OVERFLOW_PENDING          0x00000001 /* R---V */
#define NV_PFIFO_INTR_0_RUNOUT_OVERFLOW_RESET            0x00000001 /* -W--V */

#define NV_PFIFO_INTR_EN_0                               0x00002140 /* RW-4R */
#define NV_PFIFO_INTR_EN_0_CACHE_ERROR                          0:0 /* RWIVF */
#define NV_PFIFO_INTR_EN_0_CACHE_ERROR_DISABLED          0x00000000 /* RWI-V */
#define NV_PFIFO_INTR_EN_0_CACHE_ERROR_ENABLED           0x00000001 /* RW--V */
#define NV_PFIFO_INTR_EN_0_RUNOUT                               4:4 /* RWIVF */
#define NV_PFIFO_INTR_EN_0_RUNOUT_DISABLED               0x00000000 /* RWI-V */
#define NV_PFIFO_INTR_EN_0_RUNOUT_ENABLED                0x00000001 /* RW--V */
#define NV_PFIFO_INTR_EN_0_RUNOUT_OVERFLOW                      8:8 /* RWIVF */
#define NV_PFIFO_INTR_EN_0_RUNOUT_OVERFLOW_DISABLED      0x00000000 /* RWI-V */
#define NV_PFIFO_INTR_EN_0_RUNOUT_OVERFLOW_ENABLED       0x00000001 /* RW--V */

#define NV_PFIFO_CONFIG_0                                0x00002200 /* RW-4R */
#define NV_PFIFO_CONFIG_0_FREE_LIE                              1:0 /* RWXVF */
#define NV_PFIFO_CONFIG_0_FREE_LIE_DISABLED              0x00000000 /* RW--V */
#define NV_PFIFO_CONFIG_0_FREE_LIE_252_BYTES             0x00000001 /* RW--V */
#define NV_PFIFO_CONFIG_0_FREE_LIE_508_BYTES             0x00000002 /* RW--V */
#define NV_PFIFO_CONFIG_0_FREE_LIE_1020_BYTES            0x00000003 /* RW--V */

#define NV_PFIFO_CACHES                                  0x00002500 /* RW-4R */
#define NV_PFIFO_CACHES_REASSIGN                                0:0 /* RWIVF */
#define NV_PFIFO_CACHES_REASSIGN_DISABLED                0x00000000 /* RWI-V */
#define NV_PFIFO_CACHES_REASSIGN_ENABLED                 0x00000001 /* RW--V */
#define NV_PFIFO_CACHE0_PUSH0                            0x00003000 /* RW-4R */
#define NV_PFIFO_CACHE0_PUSH0_ACCESS                            0:0 /* RWIVF */
#define NV_PFIFO_CACHE0_PUSH0_ACCESS_DISABLED            0x00000000 /* RWI-V */
#define NV_PFIFO_CACHE0_PUSH0_ACCESS_ENABLED             0x00000001 /* RW--V */
#define NV_PFIFO_CACHE1_PUSH0                            0x00003200 /* RW-4R */
#define NV_PFIFO_CACHE1_PUSH0_ACCESS                            0:0 /* RWIVF */
#define NV_PFIFO_CACHE1_PUSH0_ACCESS_DISABLED            0x00000000 /* RWI-V */
#define NV_PFIFO_CACHE1_PUSH0_ACCESS_ENABLED             0x00000001 /* RW--V */
#define NV_PFIFO_CACHE0_PUSH1                            0x00003010 /* RW-4R */
#define NV_PFIFO_CACHE0_PUSH1_CHID                              6:0 /* RWXUF */
#define NV_PFIFO_CACHE1_PUSH1                            0x00003210 /* RW-4R */
#define NV_PFIFO_CACHE1_PUSH1_CHID                              6:0 /* RWXUF */
#define NV_PFIFO_CACHE0_PULL0                            0x00003040 /* RW-4R */
#define NV_PFIFO_CACHE0_PULL0_ACCESS                            0:0 /* RWIVF */
#define NV_PFIFO_CACHE0_PULL0_ACCESS_DISABLED            0x00000000 /* RWI-V */
#define NV_PFIFO_CACHE0_PULL0_ACCESS_ENABLED             0x00000001 /* RW--V */
#define NV_PFIFO_CACHE1_PULL0                            0x00003240 /* R--4R */
#define NV_PFIFO_CACHE1_PULL0_ACCESS                            0:0 /* RWIVF */
#define NV_PFIFO_CACHE1_PULL0_ACCESS_DISABLED            0x00000000 /* RWI-V */
#define NV_PFIFO_CACHE1_PULL0_ACCESS_ENABLED             0x00000001 /* RW--V */
#define NV_PFIFO_CACHE1_PULL0_HASH                              4:4 /* R-XVF */
#define NV_PFIFO_CACHE1_PULL0_HASH_SUCCEEDED             0x00000000 /* R---V */
#define NV_PFIFO_CACHE1_PULL0_HASH_FAILED                0x00000001 /* R---V */
#define NV_PFIFO_CACHE1_PULL0_DEVICE                            8:8 /* R-XVF */
#define NV_PFIFO_CACHE1_PULL0_DEVICE_HARDWARE            0x00000000 /* R---V */
#define NV_PFIFO_CACHE1_PULL0_DEVICE_SOFTWARE            0x00000001 /* R---V */
#define NV_PFIFO_CACHE0_PULL1                            0x00003050 /* RW-4R */
#define NV_PFIFO_CACHE0_PULL1_OBJECT                            8:8 /* RWXVF */
#define NV_PFIFO_CACHE0_PULL1_OBJECT_UNCHANGED           0x00000000 /* RW--V */
#define NV_PFIFO_CACHE0_PULL1_OBJECT_CHANGED             0x00000001 /* RW--V */
#define NV_PFIFO_CACHE1_PULL1                            0x00003250 /* RW-4R */
#define NV_PFIFO_CACHE1_PULL1_SUBCHANNEL                        2:0 /* RWXUF */
#define NV_PFIFO_CACHE1_PULL1_CTX                               4:4 /* RWXVF */
#define NV_PFIFO_CACHE1_PULL1_CTX_CLEAN                  0x00000000 /* RW--V */
#define NV_PFIFO_CACHE1_PULL1_CTX_DIRTY                  0x00000001 /* RW--V */
#define NV_PFIFO_CACHE1_PULL1_OBJECT                            8:8 /* RWXVF */
#define NV_PFIFO_CACHE1_PULL1_OBJECT_UNCHANGED           0x00000000 /* RW--V */
#define NV_PFIFO_CACHE1_PULL1_OBJECT_CHANGED             0x00000001 /* RW--V */

#define NV_PFIFO_CACHE0_PUT                              0x00003030 /* RW-4R */
#define NV_PFIFO_CACHE0_PUT_ADDRESS                             2:2 /* RWXUF */
#define NV_PFIFO_CACHE1_PUT                              0x00003230 /* RW-4R */
#define NV_PFIFO_CACHE1_PUT_ADDRESS                             6:2 /* RWXUF */
#define NV_PFIFO_CACHE0_GET                              0x00003070 /* RW-4R */
#define NV_PFIFO_CACHE0_GET_ADDRESS                             2:2 /* RWXUF */
#define NV_PFIFO_CACHE1_GET                              0x00003270 /* RW-4R */
#define NV_PFIFO_CACHE1_GET_ADDRESS                             6:2 /* RWXUF */

#define NV_PFIFO_CACHE0_CTX(i)                  (0x00003080+(i)*16) /* RW-4A */
#define NV_PFIFO_CACHE0_CTX__SIZE_1                               1 /*       */

#define NV_PFIFO_CACHE1_CTX(i)                  (0x00003280+(i)*16) /* RW-4A */
#define NV_PFIFO_CACHE1_CTX__SIZE_1                               8 /*       */


#define NV_PFIFO_RUNOUT_PUT                              0x00002410 /* RW-4R */
#define NV_PFIFO_RUNOUT_GET                              0x00002420 /* RW-4R */

/*****************************************
 * Definitions for PGRAPH device
 *****************************************/

#define NV_PGRAPH                             0x00400FFF:0x00400000 /* RW--D */

#define NV_PGRAPH_DEBUG_0                                0x00400080 /* RW-4R */
#define NV_PGRAPH_DEBUG_0_STATE                                 0:0 /* CW-VF */
#define NV_PGRAPH_DEBUG_0_STATE_NORMAL                   0x00000000 /* CW--V */
#define NV_PGRAPH_DEBUG_0_STATE_RESET                    0x00000001 /* -W--V */
#define NV_PGRAPH_DEBUG_0_BULK_READS                            4:4 /* RWIVF */
#define NV_PGRAPH_DEBUG_0_BULK_READS_DISABLED            0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_0_BULK_READS_ENABLED             0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_0_BLOCK                                 8:8 /* RWIVF */
#define NV_PGRAPH_DEBUG_0_BLOCK_DISABLED                 0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_0_BLOCK_ENABLED                  0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_0_BLOCK_BROAD                         12:12 /* RWIVF */
#define NV_PGRAPH_DEBUG_0_BLOCK_BROAD_DISABLED           0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_0_BLOCK_BROAD_ENABLED            0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_0_NONBLOCK_BROAD                      16:16 /* RWIVF */
#define NV_PGRAPH_DEBUG_0_NONBLOCK_BROAD_DISABLED        0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_0_NONBLOCK_BROAD_ENABLED         0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_0_WRITE_ONLY_ROPS                     20:20 /* RWIVF */
#define NV_PGRAPH_DEBUG_0_WRITE_ONLY_ROPS_DISABLED       0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_0_WRITE_ONLY_ROPS_ENABLED        0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_0_EDGE_FILLING                        24:24 /* RWIVF */
#define NV_PGRAPH_DEBUG_0_EDGE_FILLING_DISABLED          0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_0_EDGE_FILLING_ENABLED           0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_0_ALPHA_ABORT                         28:28 /* RWIVF */
#define NV_PGRAPH_DEBUG_0_ALPHA_ABORT_DISABLED           0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_0_ALPHA_ABORT_ENABLED            0x00000001 /* RW--V */

#define NV_PGRAPH_DEBUG_1                                0x00400084 /* RW-4R */
#define NV_PGRAPH_DEBUG_1_VOLATILE_RESET                        0:0 /* RWIVF */
#define NV_PGRAPH_DEBUG_1_VOLATILE_RESET_NOT_LAST        0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_1_VOLATILE_RESET_LAST            0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_1_DMA_ACTIVITY                          4:4 /* CW-VF */
#define NV_PGRAPH_DEBUG_1_DMA_ACTIVITY_IGNORE            0x00000000 /* CW--V */
#define NV_PGRAPH_DEBUG_1_DMA_ACTIVITY_CANCEL            0x00000001 /* -W--V */
#define NV_PGRAPH_DEBUG_1_BI_RECTS                              8:8 /* RWIVF */
#define NV_PGRAPH_DEBUG_1_BI_RECTS_DISABLED              0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_1_BI_RECTS_ENABLED               0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_1_TRI_OPTS                            12:12 /* RWIVF */
#define NV_PGRAPH_DEBUG_1_TRI_OPTS_DISABLED              0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_1_TRI_OPTS_ENABLED               0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_1_PATT_BLOCK                          16:16 /* RWIVF */
#define NV_PGRAPH_DEBUG_1_PATT_BLOCK_DISABLED            0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_1_PATT_BLOCK_ENABLED             0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_1_FAST_RMW_BLITS                      20:20 /* RWIVF */
#define NV_PGRAPH_DEBUG_1_FAST_RMW_BLITS_DISABLED        0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_1_FAST_RMW_BLITS_ENABLED         0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_1_TM_QUAD_HANDOFF                     24:24 /* RWIVF */
#define NV_PGRAPH_DEBUG_1_TM_QUAD_HANDOFF_DISABLED       0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_1_TM_QUAD_HANDOFF_ENABLED        0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_1_FAST_BUS                            28:28 /* RWIVF */
#define NV_PGRAPH_DEBUG_1_FAST_BUS_DISABLED              0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_1_FAST_BUS_ENABLED               0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_1_HIRES_TM                            29:29 /* RWIVF */
#define NV_PGRAPH_DEBUG_1_HIRES_TM_DISABLED              0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_1_HIRES_TM_ENABLED               0x00000001 /* RW--V */

#define NV_PGRAPH_DEBUG_2                                0x00400088 /* RW-4R */
#define NV_PGRAPH_DEBUG_2_AVOID_RMW_BLEND                       0:0 /* RWIVF */
#define NV_PGRAPH_DEBUG_2_AVOID_RMW_BLEND_DISABLED       0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_2_AVOID_RMW_BLEND_ENABLED        0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_2_ALPHA_ABORT                           4:4 /* RWIVF */
#define NV_PGRAPH_DEBUG_2_ALPHA_ABORT_DISABLED           0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_2_ALPHA_ABORT_ENABLED            0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_2_BETA_ABORT                            8:8 /* RWIVF */
#define NV_PGRAPH_DEBUG_2_BETA_ABORT_DISABLED            0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_2_BETA_ABORT_ENABLED             0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_2_MONO_ABORT                          12:12 /* RWIVF */
#define NV_PGRAPH_DEBUG_2_MONO_ABORT_DISABLED            0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_2_MONO_ABORT_ENABLED             0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_2_TRAPEZOID_TEXEL                     16:16 /* RWIVF */
#define NV_PGRAPH_DEBUG_2_TRAPEZOID_TEXEL_DISABLED       0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_2_TRAPEZOID_TEXEL_ENABLED        0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_2_BUSY_PATIENCE                       20:20 /* RWIVF */
#define NV_PGRAPH_DEBUG_2_BUSY_PATIENCE_DISABLED         0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_2_BUSY_PATIENCE_ENABLED          0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_2_TM_FASTINPUT                        24:24 /* RWIVF */
#define NV_PGRAPH_DEBUG_2_TM_FASTINPUT_DISABLED          0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_2_TM_FASTINPUT_ENABLED           0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_2_VOLATILE_RESET                      28:28 /* RWIVF */
#define NV_PGRAPH_DEBUG_2_VOLATILE_RESET_DISABLED        0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_2_VOLATILE_RESET_ENABLED         0x00000001 /* RW--V */

#define NV_PGRAPH_DEBUG_3                                0x0040008c /* RW-4R */
#define NV_PGRAPH_DEBUG_3_TM_RANGE_INTERRUPT                    0:0 /* RWIVF */
#define NV_PGRAPH_DEBUG_3_TM_RANGE_INTERRUPT_DISABLED    0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_3_TM_RANGE_INTERRUPT_ENABLED     0x00000001 /* RW--V */
#define NV_PGRAPH_DEBUG_3_MONO_BLOCK                            4:4 /* RWIVF */
#define NV_PGRAPH_DEBUG_3_MONO_BLOCK_DISABLED            0x00000000 /* RWI-V */
#define NV_PGRAPH_DEBUG_3_MONO_BLOCK_ENABLED             0x00000001 /* RW--V */

#define NV_PGRAPH_INTR_0                                 0x00400100 /* RW-4R */
#define NV_PGRAPH_INTR_0_RESERVED                               0:0 /* RW-VF */
#define NV_PGRAPH_INTR_0_RESERVED_NOT_PENDING            0x00000000 /* R---V */
#define NV_PGRAPH_INTR_0_RESERVED_PENDING                0x00000001 /* R---V */
#define NV_PGRAPH_INTR_0_RESERVED_RESET                  0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_0_CONTEXT_SWITCH                         4:4 /* RWIVF */
#define NV_PGRAPH_INTR_0_CONTEXT_SWITCH_NOT_PENDING      0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_0_CONTEXT_SWITCH_PENDING          0x00000001 /* R---V */
#define NV_PGRAPH_INTR_0_CONTEXT_SWITCH_RESET            0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_0_VBLANK                                 8:8 /* RWIVF */
#define NV_PGRAPH_INTR_0_VBLANK_NOT_PENDING              0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_0_VBLANK_PENDING                  0x00000001 /* R---V */
#define NV_PGRAPH_INTR_0_VBLANK_RESET                    0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_0_RANGE                                12:12 /* RWIVF */
#define NV_PGRAPH_INTR_0_RANGE_NOT_PENDING               0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_0_RANGE_PENDING                   0x00000001 /* R---V */
#define NV_PGRAPH_INTR_0_RANGE_RESET                     0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_0_METHOD_COUNT                         16:16 /* RWIVF */
#define NV_PGRAPH_INTR_0_METHOD_COUNT_NOT_PENDING        0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_0_METHOD_COUNT_PENDING            0x00000001 /* R---V */
#define NV_PGRAPH_INTR_0_METHOD_COUNT_RESET              0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_0_SOFTWARE                             20:20 /* RWIVF */
#define NV_PGRAPH_INTR_0_SOFTWARE_NOT_PENDING            0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_0_SOFTWARE_PENDING                0x00000001 /* R---V */
#define NV_PGRAPH_INTR_0_SOFTWARE_RESET                  0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_0_COMPLEX_CLIP                         24:24 /* RWIVF */
#define NV_PGRAPH_INTR_0_COMPLEX_CLIP_NOT_PENDING        0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_0_COMPLEX_CLIP_PENDING            0x00000001 /* R---V */
#define NV_PGRAPH_INTR_0_COMPLEX_CLIP_RESET              0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_0_NOTIFY                               28:28 /* RWIVF */
#define NV_PGRAPH_INTR_0_NOTIFY_NOT_PENDING              0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_0_NOTIFY_PENDING                  0x00000001 /* R---V */
#define NV_PGRAPH_INTR_0_NOTIFY_RESET                    0x00000001 /* -W--V */

#define NV_PGRAPH_INTR_1                                 0x00400104 /* RW-4R */
#define NV_PGRAPH_INTR_1_METHOD                                 0:0 /* RWIVF */
#define NV_PGRAPH_INTR_1_METHOD_NOT_PENDING              0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_1_METHOD_PENDING                  0x00000001 /* R---V */
#define NV_PGRAPH_INTR_1_METHOD_RESET                    0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_1_DATA                                   4:4 /* RWIVF */
#define NV_PGRAPH_INTR_1_DATA_NOT_PENDING                0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_1_DATA_PENDING                    0x00000001 /* R---V */
#define NV_PGRAPH_INTR_1_DATA_RESET                      0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_1_NOTIFY_INST                            8:8 /* RWIVF */
#define NV_PGRAPH_INTR_1_NOTIFY_INST_NOT_PENDING         0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_1_NOTIFY_INST_PENDING             0x00000001 /* R---V */
#define NV_PGRAPH_INTR_1_NOTIFY_INST_RESET               0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_1_DOUBLE_NOTIFY                        12:12 /* RWIVF */
#define NV_PGRAPH_INTR_1_DOUBLE_NOTIFY_NOT_PENDING       0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_1_DOUBLE_NOTIFY_PENDING           0x00000001 /* R---V */
#define NV_PGRAPH_INTR_1_DOUBLE_NOTIFY_RESET             0x00000001 /* -W--V */
#define NV_PGRAPH_INTR_1_CTXSW_NOTIFY                         16:16 /* RWIVF */
#define NV_PGRAPH_INTR_1_CTXSW_NOTIFY_NOT_PENDING        0x00000000 /* R-I-V */
#define NV_PGRAPH_INTR_1_CTXSW_NOTIFY_PENDING            0x00000001 /* R---V */
#define NV_PGRAPH_INTR_1_CTXSW_NOTIFY_RESET              0x00000001 /* -W--V */

#define NV_PGRAPH_INTR_EN_0                              0x00400140 /* RW-4R */
#define NV_PGRAPH_INTR_EN_0_RESERVED                            0:0 /* RWIVF */
#define NV_PGRAPH_INTR_EN_0_RESERVED_DISABLED            0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_0_RESERVED_ENABLED             0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_0_CONTEXT_SWITCH                      4:4 /* RWIVF */
#define NV_PGRAPH_INTR_EN_0_CONTEXT_SWITCH_DISABLED      0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_0_CONTEXT_SWITCH_ENABLED       0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_0_VBLANK                              8:8 /* RWIVF */
#define NV_PGRAPH_INTR_EN_0_VBLANK_DISABLED              0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_0_VBLANK_ENABLED               0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_0_RANGE                             12:12 /* RWIVF */
#define NV_PGRAPH_INTR_EN_0_RANGE_DISABLED               0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_0_RANGE_ENABLED                0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_0_METHOD_COUNT                      16:16 /* RWIVF */
#define NV_PGRAPH_INTR_EN_0_METHOD_COUNT_DISABLED        0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_0_METHOD_COUNT_ENABLED         0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_0_SOFTWARE                          20:20 /* RWIVF */
#define NV_PGRAPH_INTR_EN_0_SOFTWARE_DISABLED            0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_0_SOFTWARE_ENABLED             0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_0_COMPLEX_CLIP                      24:24 /* RWIVF */
#define NV_PGRAPH_INTR_EN_0_COMPLEX_CLIP_DISABLED        0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_0_COMPLEX_CLIP_ENABLED         0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_0_NOTIFY                            28:28 /* RWIVF */
#define NV_PGRAPH_INTR_EN_0_NOTIFY_DISABLED              0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_0_NOTIFY_ENABLED               0x00000001 /* RW--V */

#define NV_PGRAPH_INTR_EN_1                              0x00400144 /* RW-4R */
#define NV_PGRAPH_INTR_EN_1_METHOD                              0:0 /* RWIVF */
#define NV_PGRAPH_INTR_EN_1_METHOD_DISABLED              0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_1_METHOD_ENABLED               0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_1_DATA                                4:4 /* RWIVF */
#define NV_PGRAPH_INTR_EN_1_DATA_DISABLED                0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_1_DATA_ENABLED                 0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_1_NOTIFY_INST                         8:8 /* RWIVF */
#define NV_PGRAPH_INTR_EN_1_NOTIFY_INST_DISABLED         0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_1_NOTIFY_INST_ENABLED          0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_1_DOUBLE_NOTIFY                     12:12 /* RWIVF */
#define NV_PGRAPH_INTR_EN_1_DOUBLE_NOTIFY_DISABLED       0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_1_DOUBLE_NOTIFY_ENABLED        0x00000001 /* RW--V */
#define NV_PGRAPH_INTR_EN_1_CTXSW_NOTIFY                      16:16 /* RWIVF */
#define NV_PGRAPH_INTR_EN_1_CTXSW_NOTIFY_DISABLED        0x00000000 /* RWI-V */
#define NV_PGRAPH_INTR_EN_1_CTXSW_NOTIFY_ENABLED         0x00000001 /* RW--V */

#define NV_PGRAPH_CTX_SWITCH                             0x00400180 /* RW-4R */

#define NV_PGRAPH_CTX_CONTROL                            0x00400190 /* RW-4R */
#define NV_PGRAPH_CTX_CONTROL_MINIMUM_TIME                      1:0 /* RWIVF */
#define NV_PGRAPH_CTX_CONTROL_MINIMUM_TIME_33US          0x00000000 /* RWI-V */
#define NV_PGRAPH_CTX_CONTROL_MINIMUM_TIME_262US         0x00000001 /* RW--V */
#define NV_PGRAPH_CTX_CONTROL_MINIMUM_TIME_2MS           0x00000002 /* RW--V */
#define NV_PGRAPH_CTX_CONTROL_MINIMUM_TIME_17MS          0x00000003 /* RW--V */
#define NV_PGRAPH_CTX_CONTROL_TIME                              8:8 /* RWIVF */
#define NV_PGRAPH_CTX_CONTROL_TIME_EXPIRED               0x00000000 /* RWI-V */
#define NV_PGRAPH_CTX_CONTROL_TIME_NOT_EXPIRED           0x00000001 /* RW--V */
#define NV_PGRAPH_CTX_CONTROL_CHID                            16:16 /* RWIVF */
#define NV_PGRAPH_CTX_CONTROL_CHID_INVALID               0x00000000 /* RWI-V */
#define NV_PGRAPH_CTX_CONTROL_CHID_VALID                 0x00000001 /* RW--V */
#define NV_PGRAPH_CTX_CONTROL_SWITCH                          20:20 /* R--VF */
#define NV_PGRAPH_CTX_CONTROL_SWITCH_UNAVAILABLE         0x00000000 /* R---V */
#define NV_PGRAPH_CTX_CONTROL_SWITCH_AVAILABLE           0x00000001 /* R---V */
#define NV_PGRAPH_CTX_CONTROL_SWITCHING                       24:24 /* RWIVF */
#define NV_PGRAPH_CTX_CONTROL_SWITCHING_IDLE             0x00000000 /* RWI-V */
#define NV_PGRAPH_CTX_CONTROL_SWITCHING_BUSY             0x00000001 /* RW--V */
#define NV_PGRAPH_CTX_CONTROL_DEVICE                          28:28 /* RWIVF */
#define NV_PGRAPH_CTX_CONTROL_DEVICE_DISABLED            0x00000000 /* RWI-V */
#define NV_PGRAPH_CTX_CONTROL_DEVICE_ENABLED             0x00000001 /* RW--V */

#define NV_PGRAPH_MISC                                   0x004006A4 /* RW-4R */
#define NV_PGRAPH_MISC_FIFO                                     0:0 /* RWIVF */
#define NV_PGRAPH_MISC_FIFO_DISABLED                     0x00000000 /* RW--V */
#define NV_PGRAPH_MISC_FIFO_ENABLED                      0x00000001 /* RWI-V */
#define NV_PGRAPH_MISC_DMA                                      4:4 /* RWIVF */
#define NV_PGRAPH_MISC_DMA_DISABLED                      0x00000000 /* RW--V */
#define NV_PGRAPH_MISC_DMA_ENABLED                       0x00000001 /* RWI-V */
#define NV_PGRAPH_MISC_FLOWTHRU                                 8:8 /* RWIVF */
#define NV_PGRAPH_MISC_FLOWTHRU_DISABLED                 0x00000000 /* RW--V */
#define NV_PGRAPH_MISC_FLOWTHRU_ENABLED                  0x00000001 /* RWI-V */
#define NV_PGRAPH_MISC_CLASS                                  16:12 /* RWXVF */
#define NV_PGRAPH_MISC_FIFO_WRITE                             24:24 /* CW-VF */
#define NV_PGRAPH_MISC_FIFO_WRITE_IGNORED                0x00000000 /* -W--V */
#define NV_PGRAPH_MISC_FIFO_WRITE_ENABLED                0x00000001 /* CW--V */
#define NV_PGRAPH_MISC_DMA_WRITE                              25:25 /* CW-VF */
#define NV_PGRAPH_MISC_DMA_WRITE_IGNORED                 0x00000000 /* -W--V */
#define NV_PGRAPH_MISC_DMA_WRITE_ENABLED                 0x00000001 /* CW--V */
#define NV_PGRAPH_MISC_FLOWTHRU_WRITE                         26:26 /* CW-VF */
#define NV_PGRAPH_MISC_FLOWTHRU_WRITE_IGNORED            0x00000000 /* -W--V */
#define NV_PGRAPH_MISC_FLOWTHRU_WRITE_ENABLED            0x00000001 /* CW--V */
#define NV_PGRAPH_MISC_CLASS_WRITE                            27:27 /* CW-VF */
#define NV_PGRAPH_MISC_CLASS_WRITE_IGNORED               0x00000000 /* -W--V */
#define NV_PGRAPH_MISC_CLASS_WRITE_ENABLED               0x00000001 /* CW--V */

#define NV_PGRAPH_STATUS                                 0x004006B0 /* R--4R */
#define NV_PGRAPH_STATUS_STATE                                  0:0 /* R-IVF */
#define NV_PGRAPH_STATUS_STATE_IDLE                      0x00000000 /* R-I-V */
#define NV_PGRAPH_STATUS_STATE_BUSY                      0x00000001 /* R---V */
#define NV_PGRAPH_STATUS_XY_LOGIC                               4:4 /* R-IVF */
#define NV_PGRAPH_STATUS_XY_LOGIC_IDLE                   0x00000000 /* R-I-V */
#define NV_PGRAPH_STATUS_XY_LOGIC_BUSY                   0x00000001 /* R---V */
#define NV_PGRAPH_STATUS_PORT_NOTIFY                            8:8 /* R-IVF */
#define NV_PGRAPH_STATUS_PORT_NOTIFY_IDLE                0x00000000 /* R-I-V */
#define NV_PGRAPH_STATUS_PORT_NOTIFY_BUSY                0x00000001 /* R---V */
#define NV_PGRAPH_STATUS_PORT_REGISTER                        12:12 /* R-IVF */
#define NV_PGRAPH_STATUS_PORT_REGISTER_IDLE              0x00000000 /* R-I-V */
#define NV_PGRAPH_STATUS_PORT_REGISTER_BUSY              0x00000001 /* R---V */
#define NV_PGRAPH_STATUS_PORT_DMA                             16:16 /* R-IVF */
#define NV_PGRAPH_STATUS_PORT_DMA_IDLE                   0x00000000 /* R-I-V */
#define NV_PGRAPH_STATUS_PORT_DMA_BUSY                   0x00000001 /* R---V */
#define NV_PGRAPH_STATUS_DMA_NOTIFY                           20:20 /* R-IVF */
#define NV_PGRAPH_STATUS_DMA_NOTIFY_IDLE                 0x00000000 /* R-I-V */
#define NV_PGRAPH_STATUS_DMA_NOTIFY_BUSY                 0x00000001 /* R---V */
#define NV_PGRAPH_STATUS_PORT_FIFO                            24:24 /* R-IVF */
#define NV_PGRAPH_STATUS_PORT_FIFO_IDLE                  0x00000000 /* R-I-V */
#define NV_PGRAPH_STATUS_PORT_FIFO_BUSY                  0x00000001 /* R---V */

#define NV_PGRAPH_TRAPPED_ADDR                           0x004006A8 /* R--4R */
#define NV_PGRAPH_TRAPPED_ADDR_VALUE                           20:2 /* R-XUF */
#define NV_PGRAPH_TRAPPED_DATA                           0x004006AC /* R--4R */
#define NV_PGRAPH_TRAPPED_DATA_VALUE                           31:0 /* R-XVF */

#define NV_PGRAPH_CANVAS_MISC                            0x00400634 /* RW-4R */
#define NV_PGRAPH_CANVAS_MISC_DAC_BYPASS                        0:0 /* RWXVF */
#define NV_PGRAPH_CANVAS_MISC_DAC_BYPASS_DISABLED        0x00000000 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_DAC_BYPASS_ENABLED         0x00000001 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_RETAINED                          4:4 /* RWXVF */
#define NV_PGRAPH_CANVAS_MISC_RETAINED_DISABLED          0x00000000 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_RETAINED_ENABLED           0x00000001 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_DAC_DECODE                      12:12 /* RWXVF */
#define NV_PGRAPH_CANVAS_MISC_DAC_DECODE_SINGLE          0x00000000 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_DAC_DECODE_TRIPLE          0x00000001 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_DITHER                          16:16 /* RWXVF */
#define NV_PGRAPH_CANVAS_MISC_DITHER_DISABLED            0x00000000 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_DITHER_ENABLED             0x00000001 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_REPLICATE                       20:20 /* RWXVF */
#define NV_PGRAPH_CANVAS_MISC_REPLICATE_DISABLED         0x00000000 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_REPLICATE_ENABLED          0x00000001 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_SOFTWARE                        24:24 /* RWXVF */
#define NV_PGRAPH_CANVAS_MISC_SOFTWARE_DISABLED          0x00000000 /* RW--V */
#define NV_PGRAPH_CANVAS_MISC_SOFTWARE_ENABLED           0x00000001 /* RW--V */

#define NV_PGRAPH_CLIP_MISC                              0x004006A0 /* RW-4R */
#define NV_PGRAPH_CLIP_MISC_REGIONS                             1:0 /* RWIUF */
#define NV_PGRAPH_CLIP_MISC_REGIONS_DISABLED             0x00000000 /* RWI-V */
#define NV_PGRAPH_CLIP_MISC_REGIONS_1                    0x00000001 /* RW--V */
#define NV_PGRAPH_CLIP_MISC_REGIONS_2                    0x00000002 /* RW--V */
#define NV_PGRAPH_CLIP_MISC_RENDER                              4:4 /* RWIVF */
#define NV_PGRAPH_CLIP_MISC_RENDER_INCLUDED              0x00000000 /* RWI-V */
#define NV_PGRAPH_CLIP_MISC_RENDER_OCCLUDED              0x00000001 /* RW--V */
#define NV_PGRAPH_CLIP_MISC_COMPLEX                             8:8 /* RWIVF */
#define NV_PGRAPH_CLIP_MISC_COMPLEX_DISABLED             0x00000000 /* RWI-V */
#define NV_PGRAPH_CLIP_MISC_COMPLEX_ENABLED              0x00000001 /* RW--V */

#define NV_PGRAPH_CANVAS_MIN                             0x00400688 /* RW-4R */
#define NV_PGRAPH_CANVAS_MIN_X                                 15:0 /* RWXSF */
#define NV_PGRAPH_CANVAS_MIN_Y                                31:16 /* RWXSF */
#define NV_PGRAPH_CANVAS_MAX                             0x0040068C /* RW-4R */
#define NV_PGRAPH_CANVAS_MAX_X                                 11:0 /* RWXUF */
#define NV_PGRAPH_CANVAS_MAX_Y                                27:16 /* RWXUF */

#define NV_PGRAPH_CLIP0_MIN                              0x00400690 /* RW-4R */
#define NV_PGRAPH_CLIP0_MIN_X                                  11:0 /* RWXSF */
#define NV_PGRAPH_CLIP0_MIN_Y                                 27:16 /* RWXSF */
#define NV_PGRAPH_CLIP1_MIN                              0x00400698 /* RW-4R */
#define NV_PGRAPH_CLIP1_MIN_X                                  11:0 /* RWXSF */
#define NV_PGRAPH_CLIP1_MIN_Y                                 27:16 /* RWXSF */
#define NV_PGRAPH_CLIP0_MAX                              0x00400694 /* RW-4R */
#define NV_PGRAPH_CLIP0_MAX_X                                  11:0 /* RWXSF */
#define NV_PGRAPH_CLIP0_MAX_Y                                 27:16 /* RWXSF */
#define NV_PGRAPH_CLIP1_MAX                              0x0040069C /* RW-4R */
#define NV_PGRAPH_CLIP1_MAX_X                                  11:0 /* RWXSF */
#define NV_PGRAPH_CLIP1_MAX_Y                                 27:16 /* RWXSF */

#define NV_PGRAPH_DMA                                    0x00400680 /* RW-4R */
#define NV_PGRAPH_NOTIFY                                 0x00400684 /* RW-4R */

#define NV_PGRAPH_PATT_COLOR0_0                          0x00400600 /* RW-4R */
#define NV_PGRAPH_PATT_COLOR0_1                          0x00400604 /* RW-4R */
#define NV_PGRAPH_PATT_COLOR1_0                          0x00400608 /* RW-4R */
#define NV_PGRAPH_PATT_COLOR1_1                          0x0040060C /* RW-4R */
#define NV_PGRAPH_PATTERN(i)                     (0x00400610+(i)*4) /* RW-4A */
#define NV_PGRAPH_PATTERN__SIZE_1                                 2 /*       */
#define NV_PGRAPH_PATTERN_SHAPE                          0x00400618 /* RW-4R */

#define NV_PGRAPH_MONO_COLOR0                            0x0040061C /* RW-4R */
#define NV_PGRAPH_MONO_COLOR1                            0x00400620 /* RW-4R */

#define NV_PGRAPH_ROP3                                   0x00400624 /* RW-4R */
#define NV_PGRAPH_PLANE_MASK                             0x00400628 /* RW-4R */

#define NV_PGRAPH_CHROMA                                 0x0040062C /* RW-4R */

#define NV_PGRAPH_BETA                                   0x00400630 /* RW-4R */

#define NV_PGRAPH_ABS_X_RAM(i)                   (0x00400400+(i)*4) /* RW-4A */
#define NV_PGRAPH_ABS_X_RAM__SIZE_1                              18 /*       */

#define NV_PGRAPH_ABS_Y_RAM(i)                   (0x00400480+(i)*4) /* RW-4A */
#define NV_PGRAPH_ABS_Y_RAM__SIZE_1                              18 /*       */
#define NV_PGRAPH_REL_Y_RAM(i)                   (0x00400580+(i)*4) /* RW-4A */
#define NV_PGRAPH_REL_Y_RAM__SIZE_1                              18 /*       */

#define NV_PGRAPH_XY_LOGIC_MISC0                         0x00400640 /* RW-4R */
#define NV_PGRAPH_XY_LOGIC_MISC1                         0x00400644 /* RW-4R */

#define NV_PGRAPH_X_MISC                                 0x00400648 /* RW-4R */
#define NV_PGRAPH_Y_MISC                                 0x0040064c /* RW-4R */

#define NV_PGRAPH_ABS_UCLIP_XMIN                         0x00400460 /* RW-4R */
#define NV_PGRAPH_ABS_UCLIP_XMAX                         0x00400464 /* RW-4R */
#define NV_PGRAPH_ABS_UCLIP_YMIN                         0x00400468 /* RW-4R */
#define NV_PGRAPH_ABS_UCLIP_YMAX                         0x0040046C /* RW-4R */


#define NV_PGRAPH_ABS_ICLIP_XMAX                         0x00400450 /* RW-4R */
#define NV_PGRAPH_ABS_ICLIP_YMAX                         0x00400454 /* RW-4R */

#define NV_PGRAPH_SOURCE_COLOR                           0x00400654 /* RW-4R */
#define NV_PGRAPH_SUBDIVIDE                              0x00400658 /* RW-4R */
#define NV_PGRAPH_EXCEPTIONS                             0x00400650 /* RW-4R */
#define NV_PGRAPH_EDGEFILL                               0x0040065c /* RW-4R */

#define NV_PGRAPH_BETA_RAM(i)                    (0x00400700+(i)*4) /* RW-4A */
#define NV_PGRAPH_BETA_RAM__SIZE_1                               14 /*       */

#define NV_PGRAPH_BETA_RAM_BPORT(i)              (0x00400d00+(i)*4) /* R--4A */
#define NV_PGRAPH_BETA_RAM_BPORT__SIZE_1                         14 /*       */

#define NV_PGRAPH_BIT33                                  0x00400660 /* RW-4R */

/*****************************************
 * Defintions for Master control device
 *****************************************/

#define NV_PMC                              0x00000FFF:0x00000000   /* RW--D */

#define NV_PMC_INTR_0                       0x00000100              /* RW-4R */
#define NV_PMC_INTR_0_PAUDIO                0:0                     /* R--VF */
#define NV_PMC_INTR_0_PAUDIO_NOT_PENDING    0x00000000              /* R---V */
#define NV_PMC_INTR_0_PAUDIO_PENDING        0x00000001              /* R---V */
#define NV_PMC_INTR_0_PDMA                  4:4                     /* R--VF */
#define NV_PMC_INTR_0_PDMA_NOT_PENDING      0x00000000              /* R---V */
#define NV_PMC_INTR_0_PDMA_PENDING          0x00000001              /* R---V */
#define NV_PMC_INTR_0_PFIFO                 8:8                     /* R--VF */
#define NV_PMC_INTR_0_PFIFO_NOT_PENDING     0x00000000              /* R---V */
#define NV_PMC_INTR_0_PFIFO_PENDING         0x00000001              /* R---V */
#define NV_PMC_INTR_0_PGRAPH                12:12                   /* R--VF */
#define NV_PMC_INTR_0_PGRAPH_NOT_PENDING    0x00000000              /* R---V */
#define NV_PMC_INTR_0_PGRAPH_PENDING        0x00000001              /* R---V */
#define NV_PMC_INTR_0_PRM                   16:16                   /* R--VF */
#define NV_PMC_INTR_0_PRM_NOT_PENDING       0x00000000              /* R---V */
#define NV_PMC_INTR_0_PRM_PENDING           0x00000001              /* R---V */
#define NV_PMC_INTR_0_PTIMER                20:20                   /* R--VF */
#define NV_PMC_INTR_0_PTIMER_NOT_PENDING    0x00000000              /* R---V */
#define NV_PMC_INTR_0_PTIMER_PENDING        0x00000001              /* R---V */
#define NV_PMC_INTR_0_PFB                   24:24                   /* R--VF */
#define NV_PMC_INTR_0_PFB_NOT_PENDING       0x00000000              /* R---V */
#define NV_PMC_INTR_0_PFB_PENDING           0x00000001              /* R---V */
#define NV_PMC_INTR_0_SOFTWARE              28:28                   /* RWIVF */
#define NV_PMC_INTR_0_SOFTWARE_NOT_PENDING  0x00000000              /* RWI-V */
#define NV_PMC_INTR_0_SOFTWARE_PENDING      0x00000001              /* RW--V */

#define NV_PMC_ENABLE                       0x00000200              /* RW-4R */
#define NV_PMC_ENABLE_PAUDIO                0:0                     /* RWIVF */
#define NV_PMC_ENABLE_PAUDIO_DISABLED       0x00000000              /* RWI-V */
#define NV_PMC_ENABLE_PAUDIO_ENABLED        0x00000001              /* RW--V */
#define NV_PMC_ENABLE_PDMA                  4:4                     /* RWIVF */
#define NV_PMC_ENABLE_PDMA_DISABLED         0x00000000              /* RWI-V */
#define NV_PMC_ENABLE_PDMA_ENABLED          0x00000001              /* RW--V */
#define NV_PMC_ENABLE_PFIFO                 8:8                     /* RWIVF */
#define NV_PMC_ENABLE_PFIFO_DISABLED        0x00000000              /* RWI-V */
#define NV_PMC_ENABLE_PFIFO_ENABLED         0x00000001              /* RW--V */
#define NV_PMC_ENABLE_PGRAPH                12:12                   /* RWIVF */
#define NV_PMC_ENABLE_PGRAPH_DISABLED       0x00000000              /* RWI-V */
#define NV_PMC_ENABLE_PGRAPH_ENABLED        0x00000001              /* RW--V */
#define NV_PMC_ENABLE_PRM                   16:16                   /* RWIVF */
#define NV_PMC_ENABLE_PRM_DISABLED          0x00000000              /* RWI-V */
#define NV_PMC_ENABLE_PRM_ENABLED           0x00000001              /* RW--V */
#define NV_PMC_ENABLE_PFB                   24:24                   /* RWIVF */
#define NV_PMC_ENABLE_PFB_DISABLED          0x00000000              /* RWI-V */
#define NV_PMC_ENABLE_PFB_ENABLED           0x00000001              /* RW--V */

/*****************************************
 * Definitions for priviliged RAM device
 *****************************************/

#define NV_PRAM                               0x00602FFF:0x00602000 /* RW--D */
#define NV_PRAM_CONFIG_0                                 0x00602200 /* RW-4R */
#define NV_PRAM_CONFIG_0_SIZE                                   1:0 /* RWIVF */
#define NV_PRAM_CONFIG_0_SIZE_12KB                       0x00000000 /* RWI-V */
#define NV_PRAM_CONFIG_0_SIZE_20KB                       0x00000001 /* RW--V */
#define NV_PRAM_CONFIG_0_SIZE_36KB                       0x00000002 /* RW--V */
#define NV_PRAM_CONFIG_0_SIZE_68KB                       0x00000003 /* RW--V */
#define NV_PRAM_HASH_VIRTUAL(i)                  (0x00602400+(i)*4) /* -W-4A */
#define NV_PRAM_HASH_VIRTUAL__SIZE_1                            128 /*       */
#define NV_PRAM_HASH_VIRTUAL_HANDLE                            31:0 /* -W-VF */
#define NV_PRAM_HASH_PHYSICAL                            0x00602600 /* R--4R */
#define NV_PRAM_HASH_PHYSICAL_INSTANCE                         15:0 /* R-IUF */
#define NV_PRAM_HASH_PHYSICAL_INSTANCE_0                 0x00000000 /* R-I-V */
#define NV_PRAM_HASH_PHYSICAL_DEVICE                          22:16 /* R-IUF */
#define NV_PRAM_HASH_PHYSICAL_DEVICE_NOT_FOUND           0x00000000 /* R-I-V */
#define NV_PRAM_HASH_PHYSICAL_FREE_LIE                        24:24 /* R-IVF */
#define NV_PRAM_HASH_PHYSICAL_FREE_LIE_DISABLED          0x00000000 /* R-I-V */
#define NV_PRAM_HASH_PHYSICAL_FREE_LIE_ENABLED           0x00000001 /* R---V */

#define NV_PRAMFC                             0x0064BFFF:0x00648000 /* RW--D */


#define NV_PRAMHT                             0x00647FFF:0x00640000 /* RW--D */


/*****************************************
 * Defintions for real mode device
 *****************************************/

#define NV_PRM                              0x006C7FFF:0x006C0000   /* RW--D */

#define NV_PRM_CONFIG_0                     0x006C0200 /* RW-4R */
#define NV_PRM_CONFIG_0_TEXT                0:0         /* RWIVF */
#define NV_PRM_CONFIG_0_TEXT_DISABLED       0x00000000 /* RWI-V */
#define NV_PRM_CONFIG_0_TEXT_ENABLED        0x00000001 /* RW--V */
#define NV_PRM_CONFIG_0_DAC_WIDTH           4:4         /* RWIVF */
#define NV_PRM_CONFIG_0_DAC_WIDTH_BITS_6    0x00000000 /* RWI-V */
#define NV_PRM_CONFIG_0_DAC_WIDTH_BITS_8    0x00000001 /* RW--V */

#define NV_PRM_TRACE                        0x006C1F00 /* RW-4R */


#endif
