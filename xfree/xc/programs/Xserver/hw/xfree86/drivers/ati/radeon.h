/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/radeon.h,v 1.24 2001/11/14 16:50:44 alanh Exp $ */
/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Rickard E. Faith <faith@valinux.com>
 *   Alan Hourihane <ahourihane@valinux.com>
 *
 */

#ifndef _RADEON_H_
#define _RADEON_H_

#include "xf86str.h"

				/* PCI support */
#include "xf86Pci.h"

				/* XAA and Cursor Support */
#include "xaa.h"
#include "xf86Cursor.h"

				/* DDC support */
#include "xf86DDC.h"

				/* Xv support */
#include "xf86xv.h"

				/* DRI support */
#ifdef XF86DRI
#define _XF86DRI_SERVER_
#include "radeon_dripriv.h"
#include "dri.h"
#include "GL/glxint.h"
#endif

				/* Render support */
#ifdef RENDER
#include "picturestr.h"
#endif

#define RADEON_DEBUG    0       /* Turn off debugging output                */
#define RADEON_TIMEOUT  2000000 /* Fall out of wait loops after this count */
#define RADEON_MMIOSIZE 0x80000

#define RADEON_VBIOS_SIZE 0x00010000

#if RADEON_DEBUG
#define RADEONTRACE(x)                                          \
    do {                                                        \
	ErrorF("(**) %s(%d): ", RADEON_NAME, pScrn->scrnIndex); \
	ErrorF x;                                               \
    } while (0);
#else
#define RADEONTRACE(x)
#endif


/* Other macros */
#define RADEON_ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))
#define RADEON_ALIGN(x,bytes) (((x) + ((bytes) - 1)) & ~((bytes) - 1))
#define RADEONPTR(pScrn) ((RADEONInfoPtr)(pScrn)->driverPrivate)

typedef struct {        /* All values in XCLKS    */
    int  ML;            /* Memory Read Latency    */
    int  MB;            /* Memory Burst Length    */
    int  Trcd;          /* RAS to CAS delay       */
    int  Trp;           /* RAS percentage         */
    int  Twr;           /* Write Recovery         */
    int  CL;            /* CAS Latency            */
    int  Tr2w;          /* Read to Write Delay    */
    int  Rloop;         /* Loop Latency           */
    int  Rloop_fudge;   /* Add to ML to get Rloop */
    char *name;
} RADEONRAMRec, *RADEONRAMPtr;

typedef struct {
				/* Common registers */
    CARD32     ovr_clr;
    CARD32     ovr_wid_left_right;
    CARD32     ovr_wid_top_bottom;
    CARD32     ov0_scale_cntl;
    CARD32     mpp_tb_config;
    CARD32     mpp_gp_config;
    CARD32     subpic_cntl;
    CARD32     viph_control;
    CARD32     i2c_cntl_1;
    CARD32     gen_int_cntl;
    CARD32     cap0_trig_cntl;
    CARD32     cap1_trig_cntl;
    CARD32     bus_cntl;
    CARD32     surface_cntl;

				/* Other registers to save for VT switches */
    CARD32     dp_datatype;
    CARD32     rbbm_soft_reset;
    CARD32     clock_cntl_index;
    CARD32     amcgpio_en_reg;
    CARD32     amcgpio_mask;

				/* CRTC registers */
    CARD32     crtc_gen_cntl;
    CARD32     crtc_ext_cntl;
    CARD32     dac_cntl;
    CARD32     crtc_h_total_disp;
    CARD32     crtc_h_sync_strt_wid;
    CARD32     crtc_v_total_disp;
    CARD32     crtc_v_sync_strt_wid;
    CARD32     crtc_offset;
    CARD32     crtc_offset_cntl;
    CARD32     crtc_pitch;

				/* CRTC2 registers */
    CARD32     crtc2_gen_cntl;

    CARD32     dac2_cntl;
    CARD32     disp_output_cntl;
    CARD32     crtc2_h_total_disp;
    CARD32     crtc2_h_sync_strt_wid;
    CARD32     crtc2_v_total_disp;
    CARD32     crtc2_v_sync_strt_wid;
    CARD32     crtc2_offset;
    CARD32     crtc2_offset_cntl;
    CARD32     crtc2_pitch;
				/* Flat panel registers */
    CARD32     fp_crtc_h_total_disp;
    CARD32     fp_crtc_v_total_disp;
    CARD32     fp_gen_cntl;
    CARD32     fp_h_sync_strt_wid;
    CARD32     fp_horz_stretch;
    CARD32     fp_panel_cntl;
    CARD32     fp_v_sync_strt_wid;
    CARD32     fp_vert_stretch;
    CARD32     lvds_gen_cntl;
    CARD32     lvds_pll_cntl;
    CARD32     tmds_crc;

				/* Computed values for PLL */
    CARD32     dot_clock_freq;
    CARD32     pll_output_freq;
    int        feedback_div;
    int        post_div;

				/* PLL registers */
    CARD32     ppll_ref_div;
    CARD32     ppll_div_3;
    CARD32     htotal_cntl;

				/* Computed values for PLL2 */
    CARD32     dot_clock_freq_2;
    CARD32     pll_output_freq_2;
    int        feedback_div_2;
    int        post_div_2;

				/* PLL2 registers */
    CARD32     p2pll_ref_div;
    CARD32     p2pll_div_0;
    CARD32     htotal_cntl2;

				/* DDA register */
    CARD32     dda_config;
    CARD32     dda_on_off;

				/* Pallet */
    Bool       palette_valid;
    CARD32     palette[256];
    CARD32     palette2[256];
} RADEONSaveRec, *RADEONSavePtr;

typedef struct {
    CARD16        reference_freq;
    CARD16        reference_div;
    CARD32        min_pll_freq;
    CARD32        max_pll_freq;
    CARD16        xclk;
} RADEONPLLRec, *RADEONPLLPtr;

typedef struct {
    int                bitsPerPixel;
    int                depth;
    int                displayWidth;
    int                pixel_code;
    int                pixel_bytes;
    DisplayModePtr     mode;
} RADEONFBLayout;

typedef enum
{
    MT_NONE,
    MT_CRT,
    MT_LCD,
    MT_DFP,
    MT_CTV,
    MT_STV
}RADEONMonitorType;

typedef enum
{
    DDC_NONE_DETECTED,
    DDC_MONID,
    DDC_DVI,
    DDC_VGA,
    DDC_CRT2
}RADEONDDCType;

typedef enum
{
    CONNECTOR_NONE,
    CONNECTOR_PROPRIETARY,
    CONNECTOR_CRT,
    CONNECTOR_DVI_I,
    CONNECTOR_DVI_D
}RADEONConnectorType;

typedef struct {
    EntityInfoPtr     pEnt;
    pciVideoPtr       PciInfo;
    PCITAG            PciTag;
    int               Chipset;
    Bool              Primary;

    Bool              FBDev;

    unsigned long     LinearAddr; /* Frame buffer physical address           */
    unsigned long     MMIOAddr;   /* MMIO region physical address            */
    unsigned long     BIOSAddr;   /* BIOS physical address                   */

    unsigned char     *MMIO;      /* Map of MMIO region                      */
    unsigned char     *FB;        /* Map of frame buffer                     */
    CARD8             *VBIOS;     /* Video BIOS pointer                      */

    CARD32            MemCntl;
    CARD32            BusCntl;
    unsigned long     FbMapSize;  /* Size of frame buffer, in bytes          */
    int               Flags;      /* Saved copy of mode flags                */

    /****** Added for VE/M6 support *******************/
    RADEONMonitorType DisplayType;  /* Monitor connected on*/
    RADEONDDCType     DDCType;
    RADEONConnectorType ConnectorType;
    BOOL              HasCRTC2;     /* VE/M6/M7 */
    BOOL              IsR200;       /* R200 chip */
    BOOL              IsRV200;      /* RV200 chip */
    BOOL              IsSecondary;  /* second Screen */
    BOOL              UseCRT;       /* force use CRT port as primary */
    BOOL              IsM6;         /* M6 card, for some workarounds */
    BOOL              SwitchingMode;
    int               FPBIOSstart;  /* Start of the flat panel info          */

				/* EDID or BIOS values for FPs */
    int               PanelXRes;
    int               PanelYRes;
    int               HOverPlus;
    int               HSyncWidth;
    int               HBlank;
    int               VOverPlus;
    int               VSyncWidth;
    int               VBlank;
    int               PanelPwrDly;
    int               DotClock;
    /****************************************************/

    /*for getting EDID data using DDC interface*/
    BOOL                      ddc_bios;
    BOOL                      ddc1;
    BOOL                      ddc2;
    I2CBusPtr		      pI2CBus;
    CARD32                    DDCReg;
    BOOL                      HasEDID;

    RADEONPLLRec        pll;
    RADEONRAMPtr        ram;

    RADEONSaveRec       SavedReg;         /* Original (text) mode                    */
    RADEONSaveRec       ModeReg;          /* Current mode                            */
    Bool              (*CloseScreen)(int, ScreenPtr);

    void              (*BlockHandler)(int, pointer, pointer, pointer);

    Bool              PaletteSavedOnVT; /* Palette saved on last VT switch   */

    XAAInfoRecPtr     accel;
    Bool              accelOn;
    xf86CursorInfoPtr cursor;
    unsigned long     cursor_start;
    unsigned long     cursor_end;

    /*
     * XAAForceTransBlit is used to change the behavior of the XAA
     * SetupForScreenToScreenCopy function, to make it DGA-friendly.
     */
    Bool              XAAForceTransBlit;

    int               fifo_slots; /* Free slots in the FIFO (64 max)         */
    int               pix24bpp;   /* Depth of pixmap for 24bpp framebuffer   */
    Bool              dac6bits;   /* Use 6 bit DAC?                          */

				/* Computed values for Radeon */
    int               pitch;
    int               datatype;
    CARD32            dp_gui_master_cntl;

				/* Saved values for ScreenToScreenCopy */
    int               xdir;
    int               ydir;

				/* ScanlineScreenToScreenColorExpand support */
    unsigned char     *scratch_buffer[1];
    unsigned char     *scratch_save;
    int               scanline_x;
    int               scanline_y;
    int               scanline_h;
    int               scanline_h_w;
    int               scanline_words;
    int               scanline_direct;
    int               scanline_bpp; /* Only used for ImageWrite */

    DGAModePtr        DGAModes;
    int               numDGAModes;
    Bool              DGAactive;
    int               DGAViewportStatus;
    DGAFunctionRec    DGAFuncs;

    RADEONFBLayout      CurrentLayout;
#ifdef XF86DRI
    Bool              directRenderingEnabled;
    DRIInfoPtr        pDRIInfo;
    int               drmFD;
    int               numVisualConfigs;
    __GLXvisualConfig *pVisualConfigs;
    RADEONConfigPrivPtr pVisualConfigsPriv;

    drmHandle         fbHandle;

    drmSize           registerSize;
    drmHandle         registerHandle;

    Bool              IsPCI;            /* Current card is a PCI card */
    drmSize           pciSize;
    drmHandle         pciMemHandle;
    unsigned char     *PCI;             /* Map */

    Bool              depthMoves;       /* Enable depth moves -- slow! */

    drmSize           agpSize;
    drmHandle         agpMemHandle;     /* Handle from drmAgpAlloc */
    unsigned long     agpOffset;
    unsigned char     *AGP;             /* Map */
    int               agpMode;

    CARD32            pciCommand;

    Bool              CPInUse;          /* CP has been used by X server */
    int               CPMode;           /* CP mode that server/clients use */
    int               CPFifoSize;       /* Size of the CP command FIFO */
    int               CPusecTimeout;    /* CP timeout in usecs */

				/* CP ring buffer data */
    unsigned long     ringStart;        /* Offset into AGP space */
    drmHandle         ringHandle;       /* Handle from drmAddMap */
    drmSize           ringMapSize;      /* Size of map */
    int               ringSize;         /* Size of ring (in MB) */
    unsigned char     *ring;            /* Map */
    int               ringSizeLog2QW;

    unsigned long     ringReadOffset;   /* Offset into AGP space */
    drmHandle         ringReadPtrHandle; /* Handle from drmAddMap */
    drmSize           ringReadMapSize;  /* Size of map */
    unsigned char     *ringReadPtr;     /* Map */

				/* CP vertex/indirect buffer data */
    unsigned long     bufStart;        /* Offset into AGP space */
    drmHandle         bufHandle;       /* Handle from drmAddMap */
    drmSize           bufMapSize;      /* Size of map */
    int               bufSize;         /* Size of buffers (in MB) */
    unsigned char     *buf;            /* Map */
    int               bufNumBufs;      /* Number of buffers */
    drmBufMapPtr      buffers;         /* Buffer map */

				/* CP AGP Texture data */
    unsigned long     agpTexStart;      /* Offset into AGP space */
    drmHandle         agpTexHandle;     /* Handle from drmAddMap */
    drmSize           agpTexMapSize;    /* Size of map */
    int               agpTexSize;       /* Size of AGP tex space (in MB) */
    unsigned char     *agpTex;          /* Map */
    int               log2AGPTexGran;

				/* CP accleration */
    drmBufPtr         indirectBuffer;
    int               indirectStart;

				/* DRI screen private data */
    int               fbX;
    int               fbY;
    int               backX;
    int               backY;
    int               depthX;
    int               depthY;

    int               frontOffset;
    int               frontPitch;
    int               backOffset;
    int               backPitch;
    int               depthOffset;
    int               depthPitch;
    int               textureOffset;
    int               textureSize;
    int               log2TexGran;

    CARD32            frontPitchOffset;
    CARD32            backPitchOffset;
    CARD32            depthPitchOffset;

    CARD32            dst_pitch_offset;

				/* Saved scissor values */
    CARD32            sc_left;
    CARD32            sc_right;
    CARD32            sc_top;
    CARD32            sc_bottom;

    CARD32            re_top_left;
    CARD32            re_width_height;

    CARD32            aux_sc_cntl;

#ifdef PER_CONTEXT_SAREA
    int 	      perctx_sarea_size;
#endif
#endif

    XF86VideoAdaptorPtr adaptor;
    void              (*VideoTimerCallback)(ScrnInfoPtr, Time);
    int               videoKey;
    Bool              showCache;
    OptionInfoPtr     Options;
} RADEONInfoRec, *RADEONInfoPtr;

#define RADEONWaitForFifo(pScrn, entries)                                    \
do {                                                                         \
    if (info->fifo_slots < entries)                                          \
	RADEONWaitForFifoFunction(pScrn, entries);                           \
    info->fifo_slots -= entries;                                             \
} while (0)

extern void        RADEONWaitForFifoFunction(ScrnInfoPtr pScrn, int entries);
extern void        RADEONWaitForIdle(ScrnInfoPtr pScrn);

extern void        RADEONEngineReset(ScrnInfoPtr pScrn);
extern void        RADEONEngineFlush(ScrnInfoPtr pScrn);
extern void        RADEONEngineRestore(ScrnInfoPtr pScrn);

extern unsigned    RADEONINPLL(ScrnInfoPtr pScrn, int addr);
extern void        RADEONWaitForVerticalSync(ScrnInfoPtr pScrn);

extern void        RADEONSelectBuffer(ScrnInfoPtr pScrn, int buffer);

extern Bool        RADEONAccelInit(ScreenPtr pScreen);
extern Bool        RADEONCursorInit(ScreenPtr pScreen);
extern Bool        RADEONDGAInit(ScreenPtr pScreen);

extern int         RADEONMinBits(int val);

extern void        RADEONInitVideo(ScreenPtr);

#ifdef XF86DRI
extern Bool        RADEONDRIScreenInit(ScreenPtr pScreen);
extern void        RADEONDRICloseScreen(ScreenPtr pScreen);
extern Bool        RADEONDRIFinishScreenInit(ScreenPtr pScreen);

extern drmBufPtr   RADEONCPGetBuffer(ScrnInfoPtr pScrn);
extern void        RADEONCPFlushIndirect(ScrnInfoPtr pScrn);
extern void        RADEONCPReleaseIndirect(ScrnInfoPtr pScrn);


#define RADEONCP_START(pScrn, info)					\
do {									\
    int _ret = drmRadeonStartCP(info->drmFD);				\
    if (_ret) {								\
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,				\
		   "%s: CP start %d\n", __FUNCTION__, _ret);		\
    }									\
} while (0)

#define RADEONCP_STOP(pScrn, info)					\
do {									\
    int _ret = drmRadeonStopCP(info->drmFD);				\
    if (_ret) {								\
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,				\
		   "%s: CP stop %d\n", __FUNCTION__, _ret);		\
    }									\
    RADEONEngineRestore(pScrn);						\
} while (0)

#define RADEONCP_RESET(pScrn, info)					\
do {									\
    if (RADEONCP_USE_RING_BUFFER(info->CPMode)) {			\
	int _ret = drmRadeonResetCP(info->drmFD);			\
	if (_ret) {							\
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,			\
		       "%s: CP reset %d\n", __FUNCTION__, _ret);	\
	}								\
    }									\
} while (0)

#define RADEONCP_REFRESH(pScrn, info)					\
do {									\
   if ( !info->CPInUse ) {						\
      RADEON_WAIT_UNTIL_IDLE();						\
      BEGIN_RING( 6 );							\
      OUT_RING_REG( RADEON_RE_TOP_LEFT,     info->re_top_left );	\
      OUT_RING_REG( RADEON_RE_WIDTH_HEIGHT, info->re_width_height );	\
      OUT_RING_REG( RADEON_AUX_SC_CNTL,     info->aux_sc_cntl );	\
      ADVANCE_RING();							\
      info->CPInUse = TRUE;						\
   }									\
} while (0)


#define CP_PACKET0( reg, n )						\
	(RADEON_CP_PACKET0 | ((n) << 16) | ((reg) >> 2))
#define CP_PACKET1( reg0, reg1 )					\
	(RADEON_CP_PACKET1 | (((reg1) >> 2) << 11) | ((reg0) >> 2))
#define CP_PACKET2()							\
	(RADEON_CP_PACKET2)
#define CP_PACKET3( pkt, n )						\
	(RADEON_CP_PACKET3 | (pkt) | ((n) << 16))


#define RADEON_VERBOSE	0

#define RING_LOCALS	CARD32 *__head; int __count;
#define RING_THRESHOLD	256

#define BEGIN_RING( n ) do {						\
   if ( RADEON_VERBOSE ) {						\
      xf86DrvMsg( pScrn->scrnIndex, X_INFO,				\
		  "BEGIN_RING( %d ) in %s\n", n, __FUNCTION__ );	\
   }									\
   if ( !info->indirectBuffer ) {					\
      info->indirectBuffer = RADEONCPGetBuffer( pScrn );		\
      info->indirectStart = 0;						\
   } else if ( info->indirectBuffer->used - info->indirectStart +	\
	       (n) * (int)sizeof(CARD32) > RING_THRESHOLD ) {		\
      RADEONCPFlushIndirect( pScrn );					\
   }									\
   __head = (pointer)((char *)info->indirectBuffer->address +		\
		       info->indirectBuffer->used);			\
   __count = 0;								\
} while (0)

#define ADVANCE_RING() do {						\
   if ( RADEON_VERBOSE ) {						\
      xf86DrvMsg( pScrn->scrnIndex, X_INFO,				\
		  "ADVANCE_RING() used: %d+%d=%d/%d\n",			\
		  info->indirectBuffer->used - info->indirectStart,	\
		  __count * sizeof(CARD32),				\
		  info->indirectBuffer->used - info->indirectStart +	\
		  __count * sizeof(CARD32),				\
		  RING_THRESHOLD );					\
   }									\
   info->indirectBuffer->used += __count * (int)sizeof(CARD32);		\
} while (0)

#define OUT_RING( x ) do {						\
   if ( RADEON_VERBOSE ) {						\
      xf86DrvMsg( pScrn->scrnIndex, X_INFO,				\
		  "   OUT_RING( 0x%08x )\n", (unsigned int)(x) );	\
   }									\
   __head[__count++] = (x);						\
} while (0)

#define OUT_RING_REG( reg, val )					\
do {									\
   OUT_RING( CP_PACKET0( reg, 0 ) );					\
   OUT_RING( val );							\
} while (0)

#define FLUSH_RING()							\
do {									\
   if ( RADEON_VERBOSE )						\
      xf86DrvMsg( pScrn->scrnIndex, X_INFO,				\
		  "FLUSH_RING in %s\n", __FUNCTION__ );			\
   if ( info->indirectBuffer ) {					\
      RADEONCPFlushIndirect( pScrn );					\
   }									\
} while (0)


#define RADEON_WAIT_UNTIL_2D_IDLE()					\
do {									\
   BEGIN_RING( 2 );							\
   OUT_RING( CP_PACKET0( RADEON_WAIT_UNTIL, 0 ) );			\
   OUT_RING( (RADEON_WAIT_2D_IDLECLEAN |				\
	      RADEON_WAIT_HOST_IDLECLEAN) );				\
   ADVANCE_RING();							\
} while (0)

#define RADEON_WAIT_UNTIL_3D_IDLE()					\
do {									\
   BEGIN_RING( 2 );							\
   OUT_RING( CP_PACKET0( RADEON_WAIT_UNTIL, 0 ) );			\
   OUT_RING( (RADEON_WAIT_3D_IDLECLEAN |				\
	      RADEON_WAIT_HOST_IDLECLEAN) );				\
   ADVANCE_RING();							\
} while (0)

#define RADEON_WAIT_UNTIL_IDLE()					\
do {									\
   if ( RADEON_VERBOSE ) {						\
      xf86DrvMsg( pScrn->scrnIndex, X_INFO,				\
		  "WAIT_UNTIL_IDLE() in %s\n", __FUNCTION__ );		\
   }									\
   BEGIN_RING( 2 );							\
   OUT_RING( CP_PACKET0( RADEON_WAIT_UNTIL, 0 ) );			\
   OUT_RING( (RADEON_WAIT_2D_IDLECLEAN |				\
	      RADEON_WAIT_3D_IDLECLEAN |				\
	      RADEON_WAIT_HOST_IDLECLEAN) );				\
   ADVANCE_RING();							\
} while (0)

#define RADEON_FLUSH_CACHE()						\
do {									\
   BEGIN_RING( 2 );							\
   OUT_RING( CP_PACKET0( RADEON_RB2D_DSTCACHE_CTLSTAT, 0 ) );		\
   OUT_RING( RADEON_RB2D_DC_FLUSH );					\
   ADVANCE_RING();							\
} while (0)

#define RADEON_PURGE_CACHE()						\
do {									\
   BEGIN_RING( 2 );							\
   OUT_RING( CP_PACKET0( RADEON_RB2D_DSTCACHE_CTLSTAT, 0 ) );		\
   OUT_RING( RADEON_RB2D_DC_FLUSH_ALL );				\
   ADVANCE_RING();							\
} while (0)

#endif /* XF86DRI */

#endif /* _RADEON_H_ */
