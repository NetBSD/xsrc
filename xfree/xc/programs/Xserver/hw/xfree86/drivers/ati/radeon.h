/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/radeon.h,v 1.8 2000/12/08 14:41:16 alanh Exp $ */
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

				/* Xv support */
#include "xf86xv.h"

				/* DRI support */
#undef XF86DRI			/* Not yet */
#ifdef XF86DRI
#define _XF86DRI_SERVER_
#include "r128_dripriv.h"
#include "dri.h"
#include "GL/glxint.h"
#endif

#define RADEON_DEBUG    0       /* Turn off debugging output                */
#define RADEON_TIMEOUT  2000000 /* Fall out of wait loops after this count */
#define RADEON_MMIOSIZE 0x80000
/* Atomic updates of PLL clock don't seem to always work and stick, thus
 * the bit never resets. Here - we use our own check by reading back the
 * register we've just wrote to make sure it's got the Right! value */
#define RADEON_ATOMIC_UPDATE 0  /* Use PLL Atomic updates (seems broken) */

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

				/* DDA register */
    CARD32     dda_config;
    CARD32     dda_on_off;

				/* Pallet */
    Bool       palette_valid;
    CARD32     palette[256];
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
    Bool              BIOSFromPCI; /* BIOS is read from PCI space            */

    unsigned char     *MMIO;      /* Map of MMIO region                      */
    unsigned char     *FB;        /* Map of frame buffer                     */
    CARD8             *VBIOS;     /* Video BIOS pointer                      */

    CARD32            MemCntl;
    CARD32            BusCntl;
    unsigned long     FbMapSize;  /* Size of frame buffer, in bytes          */
    int               Flags;      /* Saved copy of mode flags                */

#ifdef ENABLE_FLAT_PANEL
    Bool              HasPanelRegs; /* Current chip can connect to a FP      */
    Bool              CRTOnly;      /* Only use External CRT instead of FP   */
    int               FPBIOSstart;  /* Start of the flat panel info          */

				/* Computed values for FPs */
    int               PanelXRes;
    int               PanelYRes;
    int               PanelPwrDly;
#endif

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

    drmSize           agpSize;
    drmHandle         agpMemHandle;     /* Handle from drmAgpAlloc */
    unsigned long     agpOffset;
    unsigned char     *AGP;             /* Map */
    int               agpMode;

    Bool              CPInUse;          /* CP is currently active */
    int               CPMode;           /* CP mode that server/clients use */
    int               CPFifoSize;       /* Size of the CP command FIFO */
    Bool              CPSecure;         /* CP security enabled */
    int               CPusecTimeout;    /* CP timeout in usecs */
    Bool              CP2D;             /* CP is used for X server 2D prims */

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

				/* CP vertex buffer data */
    unsigned long     vbStart;          /* Offset into AGP space */
    drmHandle         vbHandle;         /* Handle from drmAddMap */
    drmSize           vbMapSize;        /* Size of map */
    int               vbSize;           /* Size of vert bufs (in MB) */
    unsigned char     *vb;              /* Map */
    int               vbBufSize;        /* Size of individual vert buf */
    int               vbNumBufs;        /* Number of vert bufs */
    drmBufMapPtr      vbBufs;           /* Buffer map */

				/* CP indirect buffer data */
    unsigned long     indStart;         /* Offset into AGP space */
    drmHandle         indHandle;        /* Handle from drmAddMap */
    drmSize           indMapSize;       /* Size of map */
    int               indSize;          /* Size of indirect bufs (in MB) */
    unsigned char     *ind;             /* Map */

				/* CP AGP Texture data */
    unsigned long     agpTexStart;      /* Offset into AGP space */
    drmHandle         agpTexHandle;     /* Handle from drmAddMap */
    drmSize           agpTexMapSize;    /* Size of map */
    int               agpTexSize;       /* Size of AGP tex space (in MB) */
    unsigned char     *agpTex;          /* Map */
    int               log2AGPTexGran;

				/* DRI screen private data */
    int               fbX;
    int               fbY;
    int               backX;
    int               backY;
    int               depthX;
    int               depthY;
    int               textureX;
    int               textureY;
    int               textureSize;
    int               log2TexGran;
#endif
    XF86VideoAdaptorPtr adaptor;
    void              (*VideoTimerCallback)(ScrnInfoPtr, Time);
    int               videoKey;
    Bool              showCache;
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

extern unsigned    RADEONINPLL(ScrnInfoPtr pScrn, int addr);
extern void        RADEONWaitForVerticalSync(ScrnInfoPtr pScrn);

extern Bool        RADEONAccelInit(ScreenPtr pScreen);
extern void        RADEONEngineInit(ScrnInfoPtr pScrn);
extern Bool        RADEONCursorInit(ScreenPtr pScreen);
extern Bool        RADEONDGAInit(ScreenPtr pScreen);

extern int         RADEONMinBits(int val);

extern void        RADEONInitVideo(ScreenPtr);

#ifdef XF86DRI
extern Bool        RADEONDRIScreenInit(ScreenPtr pScreen);
extern void        RADEONDRICloseScreen(ScreenPtr pScreen);
extern Bool        RADEONDRIFinishScreenInit(ScreenPtr pScreen);
extern void        RADEONCPStart(ScrnInfoPtr pScrn);
extern void        RADEONCPStop(ScrnInfoPtr pScrn);
extern void        RADEONCPResetRing(ScrnInfoPtr pScrn);
extern void        RADEONCPWaitForIdle(ScrnInfoPtr pScrn);
#endif

#endif
