/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/radeon_driver.c,v 1.25.2.2 2001/05/31 08:36:15 alanh Exp $ */
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
 * Credits:
 *
 *   Thanks to Ani Joshi <ajoshi@shell.unixbox.com> for providing source
 *   code to his Radeon driver.  Portions of this file are based on the
 *   initialization code for that driver.
 *
 * References:
 *
 * !!!! FIXME !!!!
 *   RAGE 128 VR/ RAGE 128 GL Register Reference Manual (Technical
 *   Reference Manual P/N RRG-G04100-C Rev. 0.04), ATI Technologies: April
 *   1999.
 *
 *   RAGE 128 Software Development Manual (Technical Reference Manual P/N
 *   SDK-G04000 Rev. 0.01), ATI Technologies: June 1999.
 *
 * This server does not yet support these XFree86 4.0 features:
 * !!!! FIXME !!!!
 *   DDC1 & DDC2
 *   shadowfb
 *   overlay planes
 *
 * Modified by Marc Aurele La France (tsi@xfree86.org) for ATI driver merge.
 */

				/* Driver data structures */
#include "radeon.h"
#include "radeon_probe.h"
#include "radeon_reg.h"
#include "radeon_version.h"

#ifdef XF86DRI
#define _XF86DRI_SERVER_
#include "radeon_dri.h"
#include "radeon_sarea.h"
#endif

#define USE_FB                  /* not until overlays */
#ifdef USE_FB
#include "fb.h"
#else

				/* CFB support */
#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb16.h"
#include "cfb24.h"
#include "cfb32.h"
#include "cfb24_32.h"
#endif

				/* colormap initialization */
#include "micmap.h"
#include "dixstruct.h"

				/* X and server generic header files */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86PciInfo.h"
#include "xf86RAC.h"
#include "xf86cmap.h"
#include "vbe.h"

				/* fbdevhw * vgaHW definitions */
#include "fbdevhw.h"
#include "vgaHW.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

				/* Forward definitions for driver functions */
static Bool RADEONCloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool RADEONSaveScreen(ScreenPtr pScreen, int mode);
static void RADEONSave(ScrnInfoPtr pScrn);
static void RADEONRestore(ScrnInfoPtr pScrn);
static Bool RADEONModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
static void RADEONDisplayPowerManagementSet(ScrnInfoPtr pScrn,
					    int PowerManagementMode,
					    int flags);
static Bool RADEONEnterVTFBDev(int scrnIndex, int flags);
static void RADEONLeaveVTFBDev(int scrnIndex, int flags);

typedef enum {
    OPTION_NOACCEL,
    OPTION_SW_CURSOR,
    OPTION_DAC_6BIT,
    OPTION_DAC_8BIT,
#ifdef XF86DRI
    OPTION_IS_PCI,
    OPTION_CP_PIO,
    OPTION_NO_SECURITY,
    OPTION_USEC_TIMEOUT,
    OPTION_AGP_MODE,
    OPTION_AGP_SIZE,
    OPTION_RING_SIZE,
    OPTION_BUFFER_SIZE,
    OPTION_DEPTH_MOVE,
#endif
#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
#if 0
    /* FIXME: Disable CRTOnly until it is tested */
    OPTION_CRT,
#endif
    OPTION_PANEL_WIDTH,
    OPTION_PANEL_HEIGHT,
#endif
    OPTION_FBDEV
} RADEONOpts;

const OptionInfoRec RADEONOptions[] = {
    { OPTION_NOACCEL,      "NoAccel",          OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_SW_CURSOR,    "SWcursor",         OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_DAC_6BIT,     "Dac6Bit",          OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_DAC_8BIT,     "Dac8Bit",          OPTV_BOOLEAN, {0}, TRUE  },
#ifdef XF86DRI
    { OPTION_IS_PCI,       "ForcePCIMode",     OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_CP_PIO,       "CPPIOMode",        OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_USEC_TIMEOUT, "CPusecTimeout",    OPTV_INTEGER, {0}, FALSE },
    { OPTION_AGP_MODE,     "AGPMode",          OPTV_INTEGER, {0}, FALSE },
    { OPTION_AGP_SIZE,     "AGPSize",          OPTV_INTEGER, {0}, FALSE },
    { OPTION_RING_SIZE,    "RingSize",         OPTV_INTEGER, {0}, FALSE },
    { OPTION_BUFFER_SIZE,  "BufferSize",       OPTV_INTEGER, {0}, FALSE },
    { OPTION_DEPTH_MOVE,   "EnableDepthMoves", OPTV_BOOLEAN, {0}, FALSE },
#endif
#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
#if 0
    /* FIXME: Disable CRTOnly until it is tested */
    { OPTION_CRT,          "CRTOnly",          OPTV_BOOLEAN, {0}, FALSE },
#endif
    { OPTION_PANEL_WIDTH,  "PanelWidth",       OPTV_INTEGER, {0}, FALSE },
    { OPTION_PANEL_HEIGHT, "PanelHeight",      OPTV_INTEGER, {0}, FALSE },
#endif
    { OPTION_FBDEV,        "UseFBDev",         OPTV_BOOLEAN, {0}, FALSE },
    { -1,                  NULL,               OPTV_NONE,    {0}, FALSE }
};

RADEONRAMRec RADEONRAM[] = {    /* Memory Specifications
				   From Radeon Manual */
    { 4, 4, 1, 2, 1, 2, 1, 16, 12, "64-bit SDR SDRAM" },
    { 4, 4, 3, 3, 2, 3, 1, 16, 12, "64-bit DDR SDRAM" },
};

static const char *vgahwSymbols[] = {
    "vgaHWGetHWRec",
    "vgaHWFreeHWRec",
    "vgaHWLock",
    "vgaHWUnlock",
    "vgaHWSave",
    "vgaHWRestore",
    NULL
};

static const char *fbdevHWSymbols[] = {
    "fbdevHWInit",
    "fbdevHWUseBuildinMode",

    "fbdevHWGetDepth",
    "fbdevHWGetVidmem",

    /* colormap */
    "fbdevHWLoadPalette",

    /* ScrnInfo hooks */
    "fbdevHWSwitchMode",
    "fbdevHWAdjustFrame",
    "fbdevHWEnterVT",
    "fbdevHWLeaveVT",
    "fbdevHWValidMode",
    "fbdevHWRestore",
    "fbdevHWModeInit",
    "fbdevHWSave",

    "fbdevHWUnmapMMIO",
    "fbdevHWUnmapVidmem",
    "fbdevHWMapMMIO",
    "fbdevHWMapVidmem",

    NULL
};

static const char *ddcSymbols[] = {
    "xf86PrintEDID",
    "xf86DoEDID_DDC1",
    "xf86DoEDID_DDC2",
    NULL
};

#ifdef XFree86LOADER
#ifdef USE_FB
static const char *fbSymbols[] = {
    "fbScreenInit",
    NULL
};
#else
static const char *cfbSymbols[] = {
    "cfbScreenInit",
    "cfb16ScreenInit",
    "cfb24ScreenInit",
    "cfb32ScreenInit",
    "cfb24_32ScreenInit",
    NULL
};
#endif

static const char *xaaSymbols[] = {
    "XAADestroyInfoRec",
    "XAACreateInfoRec",
    "XAAInit",
    "XAAStippleScanlineFuncLSBFirst",
    "XAAOverlayFBfuncs",
    "XAACachePlanarMonoStipple",
    "XAAScreenIndex",
    NULL
};

static const char *xf8_32bppSymbols[] = {
    "xf86Overlay8Plus32Init",
    NULL
};

static const char *ramdacSymbols[] = {
    "xf86InitCursor",
    "xf86CreateCursorInfoRec",
    "xf86DestroyCursorInfoRec",
    NULL
};

#ifdef XF86DRI
static const char *drmSymbols[] = {
    "drmAddBufs",
    "drmAddMap",
    "drmAvailable",
    "drmCtlAddCommand",
    "drmCtlInstHandler",
    "drmGetInterruptFromBusID",
    "drmMapBufs",
    "drmMarkBufs",
    "drmUnmapBufs",
    "drmFreeVersion",
    "drmGetVersion",
    NULL
};

static const char *driSymbols[] = {
    "DRIGetDrawableIndex",
    "DRIFinishScreenInit",
    "DRIDestroyInfoRec",
    "DRICloseScreen",
    "DRIDestroyInfoRec",
    "DRIScreenInit",
    "DRIDestroyInfoRec",
    "DRICreateInfoRec",
    "DRILock",
    "DRIUnlock",
    "DRIGetSAREAPrivate",
    "DRIGetContext",
    "DRIQueryVersion",
    "GlxSetVisualConfigs",
    NULL
};
#endif

static const char *vbeSymbols[] = {
    "VBEInit",
    "vbeDoEDID",
    NULL
};
#endif

#if !defined(__alpha__)
# define RADEONPreInt10Save(s, r1, r2)
# define RADEONPostInt10Check(s, r1, r2)
#else /* __alpha__ */
static void
RADEONSaveRegsZapMemCntl(ScrnInfoPtr pScrn, CARD32 *MEM_CNTL, CARD32 *MEMSIZE)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO;
    int mapped = 0;

    /*
     * First make sure we have the pci and mmio info and that mmio is mapped
     */
    if (!info->PciInfo)
	info->PciInfo = xf86GetPciInfoForEntity(info->pEnt->index);
    if (!info->PciTag)
	info->PciTag = pciTag(info->PciInfo->bus, info->PciInfo->device,
			      info->PciInfo->func);
    if (!info->MMIOAddr) 
	info->MMIOAddr = info->PciInfo->memBase[2] & 0xffffff00;
    if (!info->MMIO) {
	RADEONMapMMIO(pScrn);
	mapped = 1;
    }
    RADEONMMIO = info->MMIO;

    /*
     * Save the values and zap MEM_CNTL
     */
    *MEM_CNTL = INREG(RADEON_MEM_CNTL);
    *MEMSIZE = INREG(RADEON_CONFIG_MEMSIZE);
    OUTREG(RADEON_MEM_CNTL, 0);

    /*
     * Unmap mmio space if we mapped it
     */
    if (mapped) 
	RADEONUnmapMMIO(pScrn);
}

static void
RADEONCheckRegs(ScrnInfoPtr pScrn, CARD32 Saved_MEM_CNTL, CARD32 Saved_MEMSIZE)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO;
    CARD32 MEM_CNTL;
    int mapped = 0;

    /*
     * If we don't have a valid (non-zero) saved MEM_CNTL, get out now
     */
    if (!Saved_MEM_CNTL)
	return;

    /*
     * First make sure that mmio is mapped
     */
    if (!info->MMIO) {
	RADEONMapMMIO(pScrn);
	mapped = 1;
    }
    RADEONMMIO = info->MMIO;

    /*
     * If either MEM_CNTL is currently zero or inconistent (configured for 
     * two channels with the two channels configured differently), restore
     * the saved registers.
     */
    MEM_CNTL = INREG(RADEON_MEM_CNTL);
    if (!MEM_CNTL || 
	((MEM_CNTL & 1) && 
	 (((MEM_CNTL >> 8) & 0xff) != ((MEM_CNTL >> 24) & 0xff)))) {
	/*
	 * Restore the saved registers
	 */
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "Restoring MEM_CNTL (%08x), setting to %08x\n",
		   MEM_CNTL, Saved_MEM_CNTL);
	OUTREG(RADEON_MEM_CNTL, Saved_MEM_CNTL);

	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "Restoring CONFIG_MEMSIZE (%08x), setting to %08x\n",
		   INREG(RADEON_CONFIG_MEMSIZE), Saved_MEMSIZE);
	OUTREG(RADEON_CONFIG_MEMSIZE, Saved_MEMSIZE);
    }

    /*
     * Unmap mmio space if we mapped it
     */
    if (mapped) 
	RADEONUnmapMMIO(pScrn);
}

# define RADEONPreInt10Save(s, r1, r2) 		\
    RADEONSaveRegsZapMemCntl((s), (r1), (r2))
# define RADEONPostInt10Check(s, r1, r2)	\
    RADEONCheckRegs((s), (r1), (r2))
#endif /* __alpha__ */

/* Allocate our private RADEONInfoRec. */
static Bool RADEONGetRec(ScrnInfoPtr pScrn)
{
    if (pScrn->driverPrivate) return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(RADEONInfoRec), 1);
    return TRUE;
}

/* Free our private RADEONInfoRec. */
static void RADEONFreeRec(ScrnInfoPtr pScrn)
{
    if (!pScrn || !pScrn->driverPrivate) return;
    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;
}

/* Memory map the MMIO region.  Used during pre-init and by RADEONMapMem,
   below. */
static Bool RADEONMapMMIO(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info          = RADEONPTR(pScrn);

    if (info->FBDev) {
	info->MMIO = fbdevHWMapMMIO(pScrn);
    } else {
	info->MMIO = xf86MapPciMem(pScrn->scrnIndex,
				   VIDMEM_MMIO | VIDMEM_READSIDEEFFECT,
				   info->PciTag,
				   info->MMIOAddr,
				   RADEON_MMIOSIZE);
    }

    if (!info->MMIO) return FALSE;
    return TRUE;
}

/* Unmap the MMIO region.  Used during pre-init and by RADEONUnmapMem,
   below. */
static Bool RADEONUnmapMMIO(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info          = RADEONPTR(pScrn);

    if (info->FBDev)
	fbdevHWUnmapMMIO(pScrn);
    else {
	xf86UnMapVidMem(pScrn->scrnIndex, info->MMIO, RADEON_MMIOSIZE);
    }
    info->MMIO = NULL;
    return TRUE;
}

/* Memory map the frame buffer.  Used by RADEONMapMem, below. */
static Bool RADEONMapFB(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info          = RADEONPTR(pScrn);

    if (info->FBDev) {
	info->FB = fbdevHWMapVidmem(pScrn);
    } else {
	info->FB = xf86MapPciMem(pScrn->scrnIndex,
				 VIDMEM_FRAMEBUFFER,
				 info->PciTag,
				 info->LinearAddr,
				 info->FbMapSize);
    }

    if (!info->FB) return FALSE;
    return TRUE;
}

/* Unmap the frame buffer.  Used by RADEONUnmapMem, below. */
static Bool RADEONUnmapFB(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info          = RADEONPTR(pScrn);

    if (info->FBDev)
	fbdevHWUnmapVidmem(pScrn);
    else
	xf86UnMapVidMem(pScrn->scrnIndex, info->FB, info->FbMapSize);
    info->FB = NULL;
    return TRUE;
}

/* Memory map the MMIO region and the frame buffer. */
static Bool RADEONMapMem(ScrnInfoPtr pScrn)
{
    if (!RADEONMapMMIO(pScrn)) return FALSE;
    if (!RADEONMapFB(pScrn)) {
	RADEONUnmapMMIO(pScrn);
	return FALSE;
    }
    return TRUE;
}

/* Unmap the MMIO region and the frame buffer. */
static Bool RADEONUnmapMem(ScrnInfoPtr pScrn)
{
    if (!RADEONUnmapMMIO(pScrn) || !RADEONUnmapFB(pScrn)) return FALSE;
    return TRUE;
}

/* Read PLL information */
unsigned RADEONINPLL(ScrnInfoPtr pScrn, int addr)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

#if !RADEON_ATOMIC_UPDATE
    while ( (INREG8(RADEON_CLOCK_CNTL_INDEX) & 0x9f) != addr) {
#endif
	OUTREG8(RADEON_CLOCK_CNTL_INDEX, addr & 0x1f);
#if !RADEON_ATOMIC_UPDATE
    }
#endif
    return INREG(RADEON_CLOCK_CNTL_DATA);
}

#if 0
/* Read PAL information (only used for debugging). */
static int RADEONINPAL(int idx)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTREG(RADEON_PALETTE_INDEX, idx << 16);
    return INREG(RADEON_PALETTE_DATA);
}
#endif

/* Wait for vertical sync. */
void RADEONWaitForVerticalSync(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           i;

    OUTREG(RADEON_GEN_INT_STATUS, RADEON_VSYNC_INT_AK);
    for (i = 0; i < RADEON_TIMEOUT; i++) {
	if (INREG(RADEON_GEN_INT_STATUS) & RADEON_VSYNC_INT) break;
    }
}

/* Blank screen. */
static void RADEONBlank(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTREGP(RADEON_CRTC_EXT_CNTL,
	    RADEON_CRTC_DISPLAY_DIS |
	    RADEON_CRTC_VSYNC_DIS |
	    RADEON_CRTC_HSYNC_DIS,
	  ~(RADEON_CRTC_DISPLAY_DIS |
	    RADEON_CRTC_VSYNC_DIS |
	    RADEON_CRTC_HSYNC_DIS));
}

/* Unblank screen. */
static void RADEONUnblank(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTREGP(RADEON_CRTC_EXT_CNTL, 0,
	  ~(RADEON_CRTC_DISPLAY_DIS |
	    RADEON_CRTC_VSYNC_DIS |
	    RADEON_CRTC_HSYNC_DIS));
}

/* Compute log base 2 of val. */
int RADEONMinBits(int val)
{
    int bits;

    if (!val) return 1;
    for (bits = 0; val; val >>= 1, ++bits);
    return bits;
}

/* Compute n/d with rounding. */
static int RADEONDiv(int n, int d)
{
    return (n + (d / 2)) / d;
}

/* Read the Video BIOS block and the FP registers (if applicable). */
static Bool RADEONGetBIOSParameters(ScrnInfoPtr pScrn, xf86Int10InfoPtr pInt10)
{
    RADEONInfoPtr info     = RADEONPTR(pScrn);
#ifdef ENABLE_FLAT_PANEL
    int           i;
    int           FPHeader = 0;
#endif

#define RADEON_BIOS8(v)  (info->VBIOS[v])
#define RADEON_BIOS16(v) (info->VBIOS[v] | \
			  (info->VBIOS[(v) + 1] << 8))
#define RADEON_BIOS32(v) (info->VBIOS[v] | \
			  (info->VBIOS[(v) + 1] << 8) | \
			  (info->VBIOS[(v) + 2] << 16) | \
			  (info->VBIOS[(v) + 3] << 24))

    if (!(info->VBIOS = xalloc(RADEON_VBIOS_SIZE))) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Cannot allocate space for hold Video BIOS!\n");
	return FALSE;
    }

    if (pInt10) {
	info->BIOSAddr = pInt10->BIOSseg << 4;
	(void)memcpy(info->VBIOS, xf86int10Addr(pInt10, info->BIOSAddr),
		     RADEON_VBIOS_SIZE);
    } else {
	xf86ReadPciBIOS(0, info->PciTag, 0, info->VBIOS, RADEON_VBIOS_SIZE);
	if (info->VBIOS[0] != 0x55 || info->VBIOS[1] != 0xaa) {
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		       "Video BIOS not detected in PCI space!\n");
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		       "Attempting to read Video BIOS from legacy ISA space!\n");
	    info->BIOSAddr = 0x000c0000;
	    xf86ReadBIOS(info->BIOSAddr, 0, info->VBIOS, RADEON_VBIOS_SIZE);
	}
    }
    if (info->VBIOS[0] != 0x55 || info->VBIOS[1] != 0xaa) {
	info->BIOSAddr = 0x00000000;
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "Video BIOS not found!\n");
    }

#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    if (info->HasPanelRegs) {
	info->FPBIOSstart = 0;

	/* FIXME: There should be direct access to the start of the FP info
	   tables, but until we find out where that offset is stored, we
	   must search for the ATI signature string: "M3      ". */
	for (i = 4; i < RADEON_VBIOS_SIZE-8; i++) {
	    if (RADEON_BIOS8(i)   == 'M' &&
		RADEON_BIOS8(i+1) == '3' &&
		RADEON_BIOS8(i+2) == ' ' &&
		RADEON_BIOS8(i+3) == ' ' &&
		RADEON_BIOS8(i+4) == ' ' &&
		RADEON_BIOS8(i+5) == ' ' &&
		RADEON_BIOS8(i+6) == ' ' &&
		RADEON_BIOS8(i+7) == ' ') {
		FPHeader = i-2;
		break;
	    }
	}

	if (!FPHeader) return TRUE;

	/* Assume that only one panel is attached and supported */
	for (i = FPHeader+20; i < FPHeader+84; i += 2) {
	    if (RADEON_BIOS16(i) != 0) {
		info->FPBIOSstart = RADEON_BIOS16(i);
		break;
	    }
	}
	if (!info->FPBIOSstart) return TRUE;

	if (!info->PanelXRes)
	    info->PanelXRes = RADEON_BIOS16(info->FPBIOSstart+25);
	if (!info->PanelYRes)
	    info->PanelYRes = RADEON_BIOS16(info->FPBIOSstart+27);
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel size: %dx%d\n",
		   info->PanelXRes, info->PanelYRes);

	info->PanelPwrDly = RADEON_BIOS8(info->FPBIOSstart+56);

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel ID: ");
	for (i = 1; i <= 24; i++)
	    ErrorF("%c", RADEON_BIOS8(info->FPBIOSstart+i));
	ErrorF("\n");
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel Type: ");
	i = RADEON_BIOS16(info->FPBIOSstart+29);
	if (i & 1) ErrorF("Color, ");
	else       ErrorF("Monochrome, ");
	if (i & 2) ErrorF("Dual(split), ");
	else       ErrorF("Single, ");
	switch ((i >> 2) & 0x3f) {
	case 0:  ErrorF("STN");        break;
	case 1:  ErrorF("TFT");        break;
	case 2:  ErrorF("Active STN"); break;
	case 3:  ErrorF("EL");         break;
	case 4:  ErrorF("Plasma");     break;
	default: ErrorF("UNKNOWN");    break;
	}
	ErrorF("\n");
	if (RADEON_BIOS8(info->FPBIOSstart+61) & 1) {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel Interface: LVDS\n");
	} else {
	    /* FIXME: Add Non-LVDS flat pael support */
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		       "Non-LVDS panel interface detected!  "
		       "This support is untested and may not "
		       "function properly\n");
	}
    }
#endif

    return TRUE;
}

/* Read PLL parameters from BIOS block.  Default to typical values if there
   is no BIOS. */
static Bool RADEONGetPLLParameters(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr   info = RADEONPTR(pScrn);
    RADEONPLLPtr    pll  = &info->pll;
    CARD16          bios_header;
    CARD16          pll_info_block;

    if (!info->VBIOS) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "Video BIOS not detected, using default PLL parameters!\n");
				/* These probably aren't going to work for
				   the card you are using.  Specifically,
				   reference freq can be 29.50MHz,
				   28.63MHz, or 14.32MHz.  YMMV. */
	pll->reference_freq = 2950;
	pll->reference_div  = 65;
	pll->min_pll_freq   = 12500;
	pll->max_pll_freq   = 35000;
	pll->xclk           = 10300;
    } else {
	bios_header    = RADEON_BIOS16(0x48);
	pll_info_block = RADEON_BIOS16(bios_header + 0x30);
	RADEONTRACE(("Header at 0x%04x; PLL Information at 0x%04x\n",
		     bios_header, pll_info_block));

	pll->reference_freq = RADEON_BIOS16(pll_info_block + 0x0e);
	pll->reference_div  = RADEON_BIOS16(pll_info_block + 0x10);
	pll->min_pll_freq   = RADEON_BIOS32(pll_info_block + 0x12);
	pll->max_pll_freq   = RADEON_BIOS32(pll_info_block + 0x16);
	pll->xclk           = RADEON_BIOS16(pll_info_block + 0x08);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "PLL parameters: rf=%d rd=%d min=%d max=%d; xclk=%d\n",
	       pll->reference_freq,
	       pll->reference_div,
	       pll->min_pll_freq,
	       pll->max_pll_freq,
	       pll->xclk);

    return TRUE;
}

/* This is called by RADEONPreInit to set up the default visual. */
static Bool RADEONPreInitVisual(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info          = RADEONPTR(pScrn);

    if (!xf86SetDepthBpp(pScrn, 8, 8, 8, Support32bppFb))
	return FALSE;

    switch (pScrn->depth) {
    case 8:
    case 15:
    case 16:
    case 24:
	break;
    default:
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Given depth (%d) is not supported by %s driver\n",
		   pScrn->depth, RADEON_DRIVER_NAME);
	return FALSE;
    }

    xf86PrintDepthBpp(pScrn);

    info->fifo_slots  = 0;
    info->pix24bpp    = xf86GetBppFromDepth(pScrn, pScrn->depth);
    info->CurrentLayout.bitsPerPixel = pScrn->bitsPerPixel;
    info->CurrentLayout.depth        = pScrn->depth;
    info->CurrentLayout.pixel_bytes  = pScrn->bitsPerPixel / 8;
    info->CurrentLayout.pixel_code   = (pScrn->bitsPerPixel != 16
				       ? pScrn->bitsPerPixel
				       : pScrn->depth);

    if (info->pix24bpp == 24) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	       "Radeon does NOT support 24bpp\n");
	return FALSE;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Pixel depth = %d bits stored in %d byte%s (%d bpp pixmaps)\n",
	       pScrn->depth,
	       info->CurrentLayout.pixel_bytes,
	       info->CurrentLayout.pixel_bytes > 1 ? "s" : "",
	       info->pix24bpp);


    if (!xf86SetDefaultVisual(pScrn, -1)) return FALSE;

    if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Default visual (%s) is not supported at depth %d\n",
		   xf86GetVisualName(pScrn->defaultVisual), pScrn->depth);
	return FALSE;
    }
    return TRUE;

}

/* This is called by RADEONPreInit to handle all color weight issues. */
static Bool RADEONPreInitWeight(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info          = RADEONPTR(pScrn);

				/* Save flag for 6 bit DAC to use for
				   setting CRTC registers.  Otherwise use
				   an 8 bit DAC, even if xf86SetWeight sets
				   pScrn->rgbBits to some value other than
				   8. */
    info->dac6bits = FALSE;
    if (pScrn->depth > 8) {
	rgb defaultWeight = { 0, 0, 0 };
	if (!xf86SetWeight(pScrn, defaultWeight, defaultWeight)) return FALSE;
    } else {
	pScrn->rgbBits = 8;
	if (xf86ReturnOptValBool(info->Options, OPTION_DAC_6BIT, FALSE)) {
	    pScrn->rgbBits = 6;
	    info->dac6bits = TRUE;
	}
    }
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Using %d bits per RGB (%d bit DAC)\n",
	       pScrn->rgbBits, info->dac6bits ? 6 : 8);

    return TRUE;

}

/* This is called by RADEONPreInit to handle config file overrides for things
   like chipset and memory regions.  Also determine memory size and type.
   If memory type ever needs an override, put it in this routine. */
static Bool RADEONPreInitConfig(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr   info   = RADEONPTR(pScrn);
    EntityInfoPtr   pEnt   = info->pEnt;
    GDevPtr         dev    = pEnt->device;
    int             offset = 0; /* RAM Type */
    MessageType     from;
    unsigned char   *RADEONMMIO;

				/* Chipset */
    from = X_PROBED;
    if (dev->chipset && *dev->chipset) {
	info->Chipset  = xf86StringToToken(RADEONChipsets, dev->chipset);
	from           = X_CONFIG;
    } else if (dev->chipID >= 0) {
	info->Chipset  = dev->chipID;
	from           = X_CONFIG;
    } else {
	info->Chipset = info->PciInfo->chipType;
    }
    pScrn->chipset = (char *)xf86TokenToString(RADEONChipsets, info->Chipset);

    if (!pScrn->chipset) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "ChipID 0x%04x is not recognized\n", info->Chipset);
	return FALSE;
    }

    if (info->Chipset < 0) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Chipset \"%s\" is not recognized\n", pScrn->chipset);
	return FALSE;
    }

    xf86DrvMsg(pScrn->scrnIndex, from,
	       "Chipset: \"%s\" (ChipID = 0x%04x)\n",
	       pScrn->chipset,
	       info->Chipset);

				/* Framebuffer */

    from             = X_PROBED;
    info->LinearAddr = info->PciInfo->memBase[0] & 0xfc000000;
    if (dev->MemBase) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Linear address override, using 0x%08x instead of 0x%08x\n",
		   dev->MemBase,
		   info->LinearAddr);
	info->LinearAddr = dev->MemBase;
	from             = X_CONFIG;
    } else if (!info->LinearAddr) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "No valid linear framebuffer address\n");
	return FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, from,
	       "Linear framebuffer at 0x%08lx\n", info->LinearAddr);

				/* MMIO registers */
    from             = X_PROBED;
    info->MMIOAddr   = info->PciInfo->memBase[2] & 0xffffff00;
    if (dev->IOBase) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "MMIO address override, using 0x%08x instead of 0x%08x\n",
		   dev->IOBase,
		   info->MMIOAddr);
	info->MMIOAddr = dev->IOBase;
	from           = X_CONFIG;
    } else if (!info->MMIOAddr) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid MMIO address\n");
	return FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, from,
	       "MMIO registers at 0x%08lx\n", info->MMIOAddr);

				/* BIOS */
    from              = X_PROBED;
    info->BIOSAddr    = info->PciInfo->biosBase & 0xfffe0000;
    if (dev->BiosBase) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "BIOS address override, using 0x%08x instead of 0x%08x\n",
		   dev->BiosBase,
		   info->BIOSAddr);
	info->BIOSAddr = dev->BiosBase;
	from           = X_CONFIG;
    }
    if (info->BIOSAddr) {
	xf86DrvMsg(pScrn->scrnIndex, from,
		   "BIOS at 0x%08lx\n", info->BIOSAddr);
    }

#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
				/* Flat panel (part 1) */
    /* FIXME: Make this an option */
    switch (info->Chipset) {
#if 0
    case PCI_CHIP_RADEON_XX: info->HasPanelRegs = TRUE;  break;
#endif
    case PCI_CHIP_RADEON_QD:
    case PCI_CHIP_RADEON_QE:
    case PCI_CHIP_RADEON_QF:
    case PCI_CHIP_RADEON_QG:
    case PCI_CHIP_RADEON_VE:
    default:                 info->HasPanelRegs = FALSE; break;
    }
#endif

				/* Read registers used to determine options */
    from                     = X_PROBED;
    RADEONMapMMIO(pScrn);
    RADEONMMIO               = info->MMIO;
    if (info->FBDev)
	pScrn->videoRam      = fbdevHWGetVidmem(pScrn) / 1024;
    else
	pScrn->videoRam      = INREG(RADEON_CONFIG_MEMSIZE) / 1024;
    info->MemCntl            = INREG(RADEON_SDRAM_MODE_REG);
    info->BusCntl            = INREG(RADEON_BUS_CNTL);
    RADEONMMIO               = NULL;
    RADEONUnmapMMIO(pScrn);

				/* RAM */
    switch (info->MemCntl >> 30) {
    case 0:            offset = 0; break; /*  64-bit SDR SDRAM */
    case 1:            offset = 1; break; /*  64-bit DDR SDRAM */
    default:           offset = 0;
    }
    info->ram = &RADEONRAM[offset];

    if (dev->videoRam) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Video RAM override, using %d kB instead of %d kB\n",
		   dev->videoRam,
		   pScrn->videoRam);
	from             = X_CONFIG;
	pScrn->videoRam  = dev->videoRam;
    }
    pScrn->videoRam  &= ~1023;
    info->FbMapSize  = pScrn->videoRam * 1024;
    xf86DrvMsg(pScrn->scrnIndex, from,
	       "VideoRAM: %d kByte (%s)\n", pScrn->videoRam, info->ram->name);

#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
				/* Flat panel (part 2) */
    if (info->HasPanelRegs) {
#if 1
	info->CRTOnly = FALSE;
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Using flat panel for display\n");
#else
				/* Panel CRT mode override */
	if ((info->CRTOnly = xf86ReturnOptValBool(info->Options,
						  OPTION_CRT, FALSE))) {
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		       "Using external CRT instead of "
		       "flat panel for display\n");
	} else {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Using flat panel for display\n");
	}
#endif

				/* Panel width/height overrides */
	info->PanelXRes = 0;
	info->PanelYRes = 0;
	if (xf86GetOptValInteger(info->Options,
				 OPTION_PANEL_WIDTH, &(info->PanelXRes))) {
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		       "Flat panel width: %d\n", info->PanelXRes);
	}
	if (xf86GetOptValInteger(info->Options,
				 OPTION_PANEL_HEIGHT, &(info->PanelYRes))) {
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		       "Flat panel height: %d\n", info->PanelYRes);
	}
    } else {
	info->CRTOnly = FALSE;
    }
#endif

#ifdef XF86DRI
				/* AGP/PCI */
    if (xf86ReturnOptValBool(info->Options, OPTION_IS_PCI, FALSE)) {
	info->IsPCI = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Forced into PCI-only mode\n");
    } else {
	switch (info->Chipset) {
#if 0
	case PCI_CHIP_RADEON_XX: info->IsPCI = TRUE;  break;
#endif
	case PCI_CHIP_RADEON_QD:
	case PCI_CHIP_RADEON_QE:
	case PCI_CHIP_RADEON_QF:
	case PCI_CHIP_RADEON_QG:
	case PCI_CHIP_RADEON_VE:
	default:                 info->IsPCI = FALSE; break;
	}
    }
#endif

    return TRUE;
}

static Bool RADEONPreInitDDC(ScrnInfoPtr pScrn, xf86Int10InfoPtr pInt10)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    vbeInfoPtr pVbe;

    if (!xf86LoadSubModule(pScrn, "ddc")) return FALSE;
    xf86LoaderReqSymLists(ddcSymbols, NULL);
    if (xf86LoadSubModule(pScrn, "vbe")) {
	pVbe = VBEInit(pInt10, info->pEnt->index);
	if (!pVbe) return FALSE;

	xf86SetDDCproperties(pScrn,xf86PrintEDID(vbeDoEDID(pVbe,NULL)));
	return TRUE;
    } else
	return FALSE;
}

/* This is called by RADEONPreInit to initialize gamma correction. */
static Bool RADEONPreInitGamma(ScrnInfoPtr pScrn)
{
    Gamma zeros = { 0.0, 0.0, 0.0 };

    if (!xf86SetGamma(pScrn, zeros)) return FALSE;
    return TRUE;
}

/* This is called by RADEONPreInit to validate modes and compute parameters
   for all of the valid modes. */
static Bool RADEONPreInitModes(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    ClockRangePtr clockRanges;
    int           modesFound;
    char          *mod = NULL;
    const char    *Sym = NULL;

				/* Get mode information */
    pScrn->progClock                   = TRUE;
    clockRanges                        = xnfcalloc(sizeof(*clockRanges), 1);
    clockRanges->next                  = NULL;
    clockRanges->minClock              = info->pll.min_pll_freq;
    clockRanges->maxClock              = info->pll.max_pll_freq * 10;
    clockRanges->clockIndex            = -1;
#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    if (info->HasPanelRegs) {
	clockRanges->interlaceAllowed  = FALSE;
	clockRanges->doubleScanAllowed = FALSE;
    } else {
	clockRanges->interlaceAllowed  = TRUE;
	clockRanges->doubleScanAllowed = TRUE;
    }
#else
    clockRanges->interlaceAllowed  = TRUE;
    clockRanges->doubleScanAllowed = TRUE;
#endif

    modesFound = xf86ValidateModes(pScrn,
				   pScrn->monitor->Modes,
				   pScrn->display->modes,
				   clockRanges,
				   NULL,        /* linePitches */
				   8 * 64,      /* minPitch */
				   8 * 1024,    /* maxPitch */
				   64 * pScrn->bitsPerPixel, /* pitchInc */
				   128,         /* minHeight */
				   2048,        /* maxHeight */
				   pScrn->display->virtualX,
				   pScrn->display->virtualY,
				   info->FbMapSize,
				   LOOKUP_BEST_REFRESH);

    if (modesFound < 1 && info->FBDev) {
	fbdevHWUseBuildinMode(pScrn);
	pScrn->displayWidth = pScrn->virtualX; /* FIXME: might be wrong */
	modesFound = 1;
    }

    if (modesFound == -1) return FALSE;
    xf86PruneDriverModes(pScrn);
    if (!modesFound || !pScrn->modes) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid modes found\n");
	return FALSE;
    }
    xf86SetCrtcForModes(pScrn, 0);
    pScrn->currentMode = pScrn->modes;
    xf86PrintModes(pScrn);

				/* Set DPI */
    xf86SetDpi(pScrn, 0, 0);

				/* Get ScreenInit function */
#ifdef USE_FB
    mod = "fb";
    Sym = "fbScreenInit";
#else
    switch (pScrn->bitsPerPixel) {
    case  8: mod = "cfb";   Sym = "cfbScreenInit";   break;
    case 16: mod = "cfb16"; Sym = "cfb16ScreenInit"; break;
    case 32: mod = "cfb32"; Sym = "cfb32ScreenInit"; break;
    }
#endif
    if (mod && !xf86LoadSubModule(pScrn, mod)) return FALSE;
    xf86LoaderReqSymbols(Sym, NULL);

#ifdef USE_FB
    xf86LoaderReqSymbols("fbPictureInit", NULL);
#endif

    info->CurrentLayout.displayWidth = pScrn->displayWidth;
    info->CurrentLayout.mode = pScrn->currentMode;

    return TRUE;
}

/* This is called by RADEONPreInit to initialize the hardware cursor. */
static Bool RADEONPreInitCursor(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);

    if (!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE)) {
	if (!xf86LoadSubModule(pScrn, "ramdac")) return FALSE;
    }
    return TRUE;
}

/* This is called by RADEONPreInit to initialize hardware acceleration. */
static Bool RADEONPreInitAccel(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);

    if (!xf86ReturnOptValBool(info->Options, OPTION_NOACCEL, FALSE)) {
	if (!xf86LoadSubModule(pScrn, "xaa")) return FALSE;
    }
    return TRUE;
}

static Bool RADEONPreInitInt10(ScrnInfoPtr pScrn, xf86Int10InfoPtr *ppInt10)
{
    RADEONInfoPtr   info = RADEONPTR(pScrn);

    if (xf86LoadSubModule(pScrn, "int10")) {
	xf86DrvMsg(pScrn->scrnIndex,X_INFO,"initializing int10\n");
	*ppInt10 = xf86InitInt10(info->pEnt->index);
    }
    return TRUE;
}

#ifdef XF86DRI
static Bool RADEONPreInitDRI(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr   info = RADEONPTR(pScrn);

    if (xf86ReturnOptValBool(info->Options, OPTION_CP_PIO, FALSE)) {
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Forcing CP into PIO mode\n");
	info->CPMode = RADEON_DEFAULT_CP_PIO_MODE;
    } else {
	info->CPMode = RADEON_DEFAULT_CP_BM_MODE;
    }

    info->agpMode       = RADEON_DEFAULT_AGP_MODE;
    info->agpSize       = RADEON_DEFAULT_AGP_SIZE;
    info->ringSize      = RADEON_DEFAULT_RING_SIZE;
    info->bufSize       = RADEON_DEFAULT_BUFFER_SIZE;
    info->agpTexSize    = RADEON_DEFAULT_AGP_TEX_SIZE;

    info->CPusecTimeout = RADEON_DEFAULT_CP_TIMEOUT;

    if (!info->IsPCI) {
	if (xf86GetOptValInteger(info->Options,
				 OPTION_AGP_MODE, &(info->agpMode))) {
	    if (info->agpMode < 1 || info->agpMode > RADEON_AGP_MAX_MODE) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Illegal AGP Mode: %d\n", info->agpMode);
		return FALSE;
	    }
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		       "Using AGP %dx mode\n", info->agpMode);
	}

	if (xf86GetOptValInteger(info->Options,
				 OPTION_AGP_SIZE, (int *)&(info->agpSize))) {
	    switch (info->agpSize) {
	    case 4:
	    case 8:
	    case 16:
	    case 32:
	    case 64:
	    case 128:
	    case 256:
		break;
	    default:
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Illegal AGP size: %d MB\n", info->agpSize);
		return FALSE;
	    }
	}

	if (xf86GetOptValInteger(info->Options,
				 OPTION_RING_SIZE, &(info->ringSize))) {
	    if (info->ringSize < 1 || info->ringSize >= (int)info->agpSize) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Illegal ring buffer size: %d MB\n",
			   info->ringSize);
		return FALSE;
	    }
	}

	if (xf86GetOptValInteger(info->Options,
				 OPTION_BUFFER_SIZE, &(info->bufSize))) {
	    if (info->bufSize < 1 || info->bufSize >= (int)info->agpSize) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Illegal vertex/indirect buffers size: %d MB\n",
			   info->bufSize);
		return FALSE;
	    }
	    if (info->bufSize > 2) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Illegal vertex/indirect buffers size: %d MB\n",
			   info->bufSize);
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Clamping vertex/indirect buffers size to 2 MB\n");
		info->bufSize = 2;
	    }
	}

	if (info->ringSize + info->bufSize + info->agpTexSize >
	    (int)info->agpSize) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "Buffers are too big for requested AGP space\n");
	    return FALSE;
	}

	info->agpTexSize = info->agpSize - (info->ringSize + info->bufSize);
    }

    if (xf86GetOptValInteger(info->Options, OPTION_USEC_TIMEOUT,
			     &(info->CPusecTimeout))) {
	/* This option checked by the RADEON DRM kernel module */
    }

    /* Depth moves are disabled by default since they are extremely slow */
    if ((info->depthMoves = xf86ReturnOptValBool(info->Options,
						 OPTION_DEPTH_MOVE, FALSE))) {
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Enabling depth moves\n");
    } else {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Depth moves disabled by default\n");
    }

    return TRUE;
}
#endif

static void
RADEONProbeDDC(ScrnInfoPtr pScrn, int indx)
{
    vbeInfoPtr pVbe;
    if (xf86LoadSubModule(pScrn, "vbe")) {
	pVbe = VBEInit(NULL,indx);
	ConfiguredMonitor = vbeDoEDID(pVbe, NULL);
    }
}

/* RADEONPreInit is called once at server startup. */
Bool RADEONPreInit(ScrnInfoPtr pScrn, int flags)
{
    RADEONInfoPtr    info;
    xf86Int10InfoPtr pInt10 = NULL;
    CARD32 save1, save2;

#ifdef XFree86LOADER
    /*
     * Tell the loader about symbols from other modules that this module might
     * refer to.
     */
    LoaderRefSymLists(vgahwSymbols,
#ifdef USE_FB
		      fbSymbols,
#else
		      cfbSymbols,
#endif
		      xaaSymbols,
		      xf8_32bppSymbols,
		      ramdacSymbols,
#ifdef XF86DRI
		      drmSymbols,
		      driSymbols,
#endif
		      fbdevHWSymbols,
		      vbeSymbols,
		      /* ddcsymbols, */
		      /* i2csymbols, */
		      /* shadowSymbols, */
		      NULL);
#endif

    RADEONTRACE(("RADEONPreInit\n"));
    if (pScrn->numEntities != 1) return FALSE;

    if (!RADEONGetRec(pScrn)) return FALSE;

    info               = RADEONPTR(pScrn);

    info->pEnt         = xf86GetEntityInfo(pScrn->entityList[0]);
    if (info->pEnt->location.type != BUS_PCI) goto fail;

    RADEONPreInt10Save(pScrn, &save1, &save2);

    if (flags & PROBE_DETECT) {
	RADEONProbeDDC(pScrn, info->pEnt->index);
	RADEONPostInt10Check(pScrn, save1, save2);
	return TRUE;
    }

    if (!xf86LoadSubModule(pScrn, "vgahw")) return FALSE;
    xf86LoaderReqSymLists(vgahwSymbols, NULL);
    if (!vgaHWGetHWRec(pScrn)) {
	RADEONFreeRec(pScrn);
	return FALSE;
    }

    info->PciInfo      = xf86GetPciInfoForEntity(info->pEnt->index);
    info->PciTag       = pciTag(info->PciInfo->bus,
				info->PciInfo->device,
				info->PciInfo->func);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "PCI bus %d card %d func %d\n",
	       info->PciInfo->bus,
	       info->PciInfo->device,
	       info->PciInfo->func);

    if (xf86RegisterResources(info->pEnt->index, 0, ResNone)) goto fail;

    pScrn->racMemFlags = RAC_FB | RAC_COLORMAP;
    pScrn->monitor     = pScrn->confScreen->monitor;

    if (!RADEONPreInitVisual(pScrn))    goto fail;

				/* We can't do this until we have a
				   pScrn->display. */
    xf86CollectOptions(pScrn, NULL);
    if (!(info->Options = xalloc(sizeof(RADEONOptions))))     goto fail;
    memcpy(info->Options, RADEONOptions, sizeof(RADEONOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, info->Options);

    if (!RADEONPreInitWeight(pScrn))    goto fail;

    if (xf86ReturnOptValBool(info->Options, OPTION_FBDEV, FALSE)) {
	info->FBDev = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		   "Using framebuffer device\n");
    }

    if (info->FBDev) {
	/* check for linux framebuffer device */
	if (!xf86LoadSubModule(pScrn, "fbdevhw")) return FALSE;
	xf86LoaderReqSymLists(fbdevHWSymbols, NULL);
	if (!fbdevHWInit(pScrn, info->PciInfo, NULL)) return FALSE;
	pScrn->SwitchMode    = fbdevHWSwitchMode;
	pScrn->AdjustFrame   = fbdevHWAdjustFrame;
	pScrn->EnterVT       = RADEONEnterVTFBDev;
	pScrn->LeaveVT       = RADEONLeaveVTFBDev;
	pScrn->ValidMode     = fbdevHWValidMode;
    }

    if (!info->FBDev)
	if (!RADEONPreInitInt10(pScrn, &pInt10)) goto fail;

    RADEONPostInt10Check(pScrn, save1, save2);

    if (!RADEONPreInitConfig(pScrn))             goto fail;

    if (!RADEONGetBIOSParameters(pScrn, pInt10)) goto fail;

    if (!RADEONGetPLLParameters(pScrn))          goto fail;

    /* shouldn't fail just because we can't get DDC */
    RADEONPreInitDDC(pScrn, pInt10);

    RADEONPostInt10Check(pScrn, save1, save2);

    if (!RADEONPreInitGamma(pScrn))              goto fail;

    if (!RADEONPreInitModes(pScrn))              goto fail;

    if (!RADEONPreInitCursor(pScrn))             goto fail;

    if (!RADEONPreInitAccel(pScrn))              goto fail;

#ifdef XF86DRI
    if (!RADEONPreInitDRI(pScrn))                goto fail;
#endif

				/* Free the video bios (if applicable) */
    if (info->VBIOS) {
	xfree(info->VBIOS);
	info->VBIOS = NULL;
    }

				/* Free int10 info */
    if (pInt10)
	xf86FreeInt10(pInt10);

    return TRUE;

  fail:
				/* Pre-init failed. */

				/* Free the video bios (if applicable) */
    if (info->VBIOS) {
	xfree(info->VBIOS);
	info->VBIOS = NULL;
    }

				/* Free int10 info */
    if (pInt10)
	xf86FreeInt10(pInt10);

    vgaHWFreeHWRec(pScrn);
    RADEONFreeRec(pScrn);
    return FALSE;
}

/* Load a palette. */
static void RADEONLoadPalette(ScrnInfoPtr pScrn, int numColors,
			      int *indices, LOCO *colors, VisualPtr pVisual)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           i;
    int           idx;
    unsigned char r, g, b;

#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    /* Select palette 0 (main CRTC) if using FP-enabled chip */
    if (info->HasPanelRegs) PAL_SELECT(0);
#endif

    if (info->CurrentLayout.depth == 15) {
	/* 15bpp mode.  This sends 32 values. */
	for (i = 0; i < numColors; i++) {
	    idx = indices[i];
	    r   = colors[idx].red;
	    g   = colors[idx].green;
	    b   = colors[idx].blue;
	    RADEONWaitForFifo(pScrn, 32); /* delay */
	    OUTPAL(idx * 8, r, g, b);
	}
    }
    else if (info->CurrentLayout.depth == 16) {
	/* 16bpp mode.  This sends 64 values. */
				/* There are twice as many green values as
				   there are values for red and blue.  So,
				   we take each red and blue pair, and
				   combine it with each of the two green
				   values. */
	for (i = 0; i < numColors; i++) {
	    idx = indices[i];
	    r   = colors[idx / 2].red;
	    g   = colors[idx].green;
	    b   = colors[idx / 2].blue;
	    RADEONWaitForFifo(pScrn, 32); /* delay */
	    OUTPAL(idx * 4, r, g, b);

	    /* AH - Added to write extra green data - How come this isn't
	     * needed on R128 ? We didn't load the extra green data in the
	     * other routine */
	    if (idx <= 31) {
		r   = colors[idx].red;
		g   = colors[(idx * 2) + 1].green;
		b   = colors[idx].blue;
		RADEONWaitForFifo(pScrn, 32); /* delay */
		OUTPAL(idx * 8, r, g, b);
	    }
	}
    }
    else {
	/* 8bpp mode.  This sends 256 values. */
	for (i = 0; i < numColors; i++) {
	    idx = indices[i];
	    r   = colors[idx].red;
	    b   = colors[idx].blue;
	    g   = colors[idx].green;
	    RADEONWaitForFifo(pScrn, 32); /* delay */
	    OUTPAL(idx, r, g, b);
	}
    }
}

static void
RADEONBlockHandler(int i, pointer blockData, pointer pTimeout, pointer pReadmask)
{
    ScreenPtr     pScreen = screenInfo.screens[i];
    ScrnInfoPtr   pScrn   = xf86Screens[i];
    RADEONInfoPtr info    = RADEONPTR(pScrn);

    pScreen->BlockHandler = info->BlockHandler;
    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);
    pScreen->BlockHandler = RADEONBlockHandler;

    if(info->VideoTimerCallback) {
        (*info->VideoTimerCallback)(pScrn, currentTime.milliseconds);
    }
}

/* Called at the start of each server generation. */
Bool RADEONScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr   pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info  = RADEONPTR(pScrn);
    BoxRec        MemBox;
    int           y2;

    RADEONTRACE(("RADEONScreenInit %x %d\n",
		 pScrn->memPhysBase, pScrn->fbOffset));

#ifdef XF86DRI
				/* Turn off the CP for now. */
    info->CPInUse      = FALSE;
#endif

    if (!RADEONMapMem(pScrn)) return FALSE;
    pScrn->fbOffset    = 0;
#ifdef XF86DRI
    info->fbX          = 0;
    info->fbY          = 0;
#endif

    info->PaletteSavedOnVT = FALSE;

    RADEONSave(pScrn);
    if (info->FBDev) {
	if (!fbdevHWModeInit(pScrn, pScrn->currentMode)) return FALSE;
    } else {
	if (!RADEONModeInit(pScrn, pScrn->currentMode)) return FALSE;
    }

    RADEONSaveScreen(pScreen, SCREEN_SAVER_ON);
    pScrn->AdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

				/* Visual setup */
    miClearVisualTypes();
    if (!miSetVisualTypes(pScrn->depth,
			  miGetDefaultVisualMask(pScrn->depth),
			  pScrn->rgbBits,
			  pScrn->defaultVisual)) return FALSE;
    miSetPixmapDepths ();

#ifdef XF86DRI
				/* Setup DRI after visuals have been
				   established, but before cfbScreenInit is
				   called.  cfbScreenInit will eventually
				   call the driver's InitGLXVisuals call
				   back. */
    {
	/* FIXME: When we move to dynamic allocation of back and depth
	   buffers, we will want to revisit the following check for 3
	   times the virtual size of the screen below. */
	int width_bytes = (pScrn->displayWidth *
			   info->CurrentLayout.pixel_bytes);
	int maxy        = info->FbMapSize / width_bytes;

	if (xf86ReturnOptValBool(info->Options, OPTION_NOACCEL, FALSE)) {
	    xf86DrvMsg(scrnIndex, X_WARNING,
		       "Acceleration disabled, not initializing the DRI\n");
	    info->directRenderingEnabled = FALSE;
	} else if (maxy <= pScrn->virtualY * 3) {
	    xf86DrvMsg(scrnIndex, X_WARNING,
		       "Static buffer allocation failed -- "
		       "need at least %d kB video memory\n",
		       (pScrn->displayWidth * pScrn->virtualY *
			info->CurrentLayout.pixel_bytes * 3 + 1023) / 1024);
	    info->directRenderingEnabled = FALSE;
	} else {
	    info->directRenderingEnabled = RADEONDRIScreenInit(pScreen);
	}
    }
#endif

#ifdef USE_FB
    if (!fbScreenInit (pScreen, info->FB,
		       pScrn->virtualX, pScrn->virtualY,
		       pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth,
		       pScrn->bitsPerPixel))
	return FALSE;
    fbPictureInit (pScreen, 0, 0);
#else
    switch (pScrn->bitsPerPixel) {
    case 8:
	if (!cfbScreenInit(pScreen, info->FB,
			   pScrn->virtualX, pScrn->virtualY,
			   pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth))
	    return FALSE;
	break;
    case 16:
	if (!cfb16ScreenInit(pScreen, info->FB,
			     pScrn->virtualX, pScrn->virtualY,
			     pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth))
	    return FALSE;
	break;
    case 32:
	if (!cfb32ScreenInit(pScreen, info->FB,
			     pScrn->virtualX, pScrn->virtualY,
			     pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth))
	    return FALSE;
	break;
    default:
	xf86DrvMsg(scrnIndex, X_ERROR,
		   "Invalid bpp (%d)\n", pScrn->bitsPerPixel);
	return FALSE;
    }
#endif
    xf86SetBlackWhitePixels(pScreen);

    if (pScrn->bitsPerPixel > 8) {
	VisualPtr visual;

	visual = pScreen->visuals + pScreen->numVisuals;
	while (--visual >= pScreen->visuals) {
	    if ((visual->class | DynamicClass) == DirectColor) {
		visual->offsetRed   = pScrn->offset.red;
		visual->offsetGreen = pScrn->offset.green;
		visual->offsetBlue  = pScrn->offset.blue;
		visual->redMask     = pScrn->mask.red;
		visual->greenMask   = pScrn->mask.green;
		visual->blueMask    = pScrn->mask.blue;
	    }
	}
    }

    RADEONDGAInit(pScreen);

				/* Memory manager setup */
#ifdef XF86DRI
    if (info->directRenderingEnabled) {
	FBAreaPtr fbarea;
	int width_bytes = (pScrn->displayWidth *
			   info->CurrentLayout.pixel_bytes);
	int cpp = info->CurrentLayout.pixel_bytes;
	int bufferSize = ((pScrn->virtualY * width_bytes + RADEON_BUFFER_ALIGN)
			  & ~RADEON_BUFFER_ALIGN);
	int l;
	int scanlines;

	info->frontOffset = 0;
	info->frontPitch = pScrn->displayWidth;

	switch (info->CPMode) {
	case RADEON_DEFAULT_CP_PIO_MODE:
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CP in PIO mode\n");
	    break;
	case RADEON_DEFAULT_CP_BM_MODE:
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CP in BM mode\n");
	    break;
	default:
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "CP in UNKNOWN mode\n");
	    break;
	}

	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Using %d MB AGP aperture\n", info->agpSize);
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Using %d MB for the ring buffer\n", info->ringSize);
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Using %d MB for vertex/indirect buffers\n", info->bufSize);
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Using %d MB for AGP textures\n", info->agpTexSize);

	/* Try for front, back, depth, and three framebuffers worth of
	 * pixmap cache.  Should be enough for a fullscreen background
	 * image plus some leftovers.
	 */
	info->textureSize = info->FbMapSize - 6 * bufferSize;

	/* If that gives us less than half the available memory, let's
	 * be greedy and grab some more.  Sorry, I care more about 3D
	 * performance than playing nicely, and you'll get around a full
	 * framebuffer's worth of pixmap cache anyway.
	 */
	if (info->textureSize < (int)info->FbMapSize / 2) {
	    info->textureSize = info->FbMapSize - 5 * bufferSize;
	}
	if (info->textureSize < (int)info->FbMapSize / 2) {
	    info->textureSize = info->FbMapSize - 4 * bufferSize;
	}

	/* Check to see if there is more room available after the 8192nd
	   scanline for textures */
	if ((int)info->FbMapSize - 8192*width_bytes - bufferSize*2
	    > info->textureSize) {
	    info->textureSize =
		info->FbMapSize - 8192*width_bytes - bufferSize*2;
	}

	if (info->textureSize > 0) {
	    l = RADEONMinBits((info->textureSize-1) / RADEON_NR_TEX_REGIONS);
	    if (l < RADEON_LOG_TEX_GRANULARITY) l = RADEON_LOG_TEX_GRANULARITY;

	    /* Round the texture size up to the nearest whole number of
	     * texture regions.  Again, be greedy about this, don't
	     * round down.
	     */
	    info->log2TexGran = l;
	    info->textureSize = (info->textureSize >> l) << l;
	} else {
	    info->textureSize = 0;
	}

	/* Set a minimum usable local texture heap size.  This will fit
	 * two 256x256x32bpp textures.
	 */
	if (info->textureSize < 512 * 1024) {
	    info->textureOffset = 0;
	    info->textureSize = 0;
	}

				/* Reserve space for textures */
	info->textureOffset = (info->FbMapSize - info->textureSize +
			       RADEON_BUFFER_ALIGN) &
			      ~(CARD32)RADEON_BUFFER_ALIGN;

				/* Reserve space for the shared depth buffer */
	info->depthOffset = (info->textureOffset - bufferSize +
			     RADEON_BUFFER_ALIGN) &
			    ~(CARD32)RADEON_BUFFER_ALIGN;
	info->depthPitch = pScrn->displayWidth;

				/* Reserve space for the shared back buffer */
	info->backOffset = (info->depthOffset - bufferSize +
			    RADEON_BUFFER_ALIGN) &
			   ~(CARD32)RADEON_BUFFER_ALIGN;
	info->backPitch = pScrn->displayWidth;

	scanlines = info->backOffset / width_bytes - 1;
	if (scanlines > 8191) scanlines = 8191;

	MemBox.x1 = 0;
	MemBox.y1 = 0;
	MemBox.x2 = pScrn->displayWidth;
	MemBox.y2 = scanlines;

	if (!xf86InitFBManager(pScreen, &MemBox)) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Memory manager initialization to (%d,%d) (%d,%d) failed\n",
		       MemBox.x1, MemBox.y1, MemBox.x2, MemBox.y2);
	    return FALSE;
	} else {
	    int width, height;

	    xf86DrvMsg(scrnIndex, X_INFO,
		       "Memory manager initialized to (%d,%d) (%d,%d)\n",
		       MemBox.x1, MemBox.y1, MemBox.x2, MemBox.y2);
	    if ((fbarea = xf86AllocateOffscreenArea(pScreen,
						    pScrn->displayWidth,
						    2, 0, NULL, NULL, NULL))) {
		xf86DrvMsg(scrnIndex, X_INFO,
			   "Reserved area from (%d,%d) to (%d,%d)\n",
			   fbarea->box.x1, fbarea->box.y1,
			   fbarea->box.x2, fbarea->box.y2);
	    } else {
		xf86DrvMsg(scrnIndex, X_ERROR, "Unable to reserve area\n");
	    }
	    if (xf86QueryLargestOffscreenArea(pScreen, &width,
					      &height, 0, 0, 0)) {
		xf86DrvMsg(scrnIndex, X_INFO,
			   "Largest offscreen area available: %d x %d\n",
			   width, height);
	    }
	}

	xf86DrvMsg(scrnIndex, X_INFO,
		   "Reserved back buffer at offset 0x%x\n",
		   info->backOffset);
	xf86DrvMsg(scrnIndex, X_INFO,
		   "Reserved depth buffer at offset 0x%x\n",
		   info->depthOffset);
	xf86DrvMsg(scrnIndex, X_INFO,
		   "Reserved %d kb for textures at offset 0x%x\n",
		   info->textureSize/1024, info->textureOffset);

	info->frontPitchOffset = (((info->frontPitch * cpp / 64) << 22) |
				  (info->frontOffset >> 10));

	info->backPitchOffset = (((info->backPitch * cpp / 64) << 22) |
				 (info->backOffset >> 10));

	info->depthPitchOffset = (((info->depthPitch * cpp / 64) << 22) |
				  (info->depthOffset >> 10));
    }
    else
#endif
    {
	MemBox.x1 = 0;
	MemBox.y1 = 0;
	MemBox.x2 = pScrn->displayWidth;
	y2        = (info->FbMapSize
		     / (pScrn->displayWidth *
			info->CurrentLayout.pixel_bytes));
				/* The acceleration engine uses 14 bit
				   signed coordinates, so we can't have any
				   drawable caches beyond this region. */
	if (y2 > 8191) y2 = 8191;
	MemBox.y2 = y2;

	if (!xf86InitFBManager(pScreen, &MemBox)) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Memory manager initialization to (%d,%d) (%d,%d) failed\n",
		       MemBox.x1, MemBox.y1, MemBox.x2, MemBox.y2);
	    return FALSE;
	} else {
	    int       width, height;
	    FBAreaPtr fbarea;

	    xf86DrvMsg(scrnIndex, X_INFO,
		       "Memory manager initialized to (%d,%d) (%d,%d)\n",
		       MemBox.x1, MemBox.y1, MemBox.x2, MemBox.y2);
	    if ((fbarea = xf86AllocateOffscreenArea(pScreen, pScrn->displayWidth,
						    2, 0, NULL, NULL, NULL))) {
		xf86DrvMsg(scrnIndex, X_INFO,
			   "Reserved area from (%d,%d) to (%d,%d)\n",
			   fbarea->box.x1, fbarea->box.y1,
			   fbarea->box.x2, fbarea->box.y2);
	    } else {
		xf86DrvMsg(scrnIndex, X_ERROR, "Unable to reserve area\n");
	    }
	    if (xf86QueryLargestOffscreenArea(pScreen, &width, &height,
					      0, 0, 0)) {
		xf86DrvMsg(scrnIndex, X_INFO,
			   "Largest offscreen area available: %d x %d\n",
			   width, height);
	    }
	}
    }

				/* Backing store setup */
    miInitializeBackingStore(pScreen);
    xf86SetBackingStore(pScreen);

				/* Set Silken Mouse */
    xf86SetSilkenMouse(pScreen);

				/* Acceleration setup */
    if (!xf86ReturnOptValBool(info->Options, OPTION_NOACCEL, FALSE)) {
	if (RADEONAccelInit(pScreen)) {
	    xf86DrvMsg(scrnIndex, X_INFO, "Acceleration enabled\n");
	    info->accelOn = TRUE;
	} else {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Acceleration initialization failed\n");
	    xf86DrvMsg(scrnIndex, X_INFO, "Acceleration disabled\n");
	    info->accelOn = FALSE;
	}
    } else {
	xf86DrvMsg(scrnIndex, X_INFO, "Acceleration disabled\n");
	info->accelOn = FALSE;
    }

				/* Cursor setup */
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

				/* Hardware cursor setup */
    if (!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE)) {
	if (RADEONCursorInit(pScreen)) {
	    int width, height;

	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Using hardware cursor (scanline %d)\n",
		       info->cursor_start / pScrn->displayWidth);
	    if (xf86QueryLargestOffscreenArea(pScreen, &width, &height,
					      0, 0, 0)) {
		xf86DrvMsg(scrnIndex, X_INFO,
			   "Largest offscreen area available: %d x %d\n",
			   width, height);
	    }
	} else {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Hardware cursor initialization failed\n");
	    xf86DrvMsg(scrnIndex, X_INFO, "Using software cursor\n");
	}
    } else {
	xf86DrvMsg(scrnIndex, X_INFO, "Using software cursor\n");
    }

				/* Colormap setup */
    if (!miCreateDefColormap(pScreen)) return FALSE;
    if (!xf86HandleColormaps(pScreen, 256, info->dac6bits ? 6 : 8,
			     (info->FBDev ? fbdevHWLoadPalette :
			     RADEONLoadPalette), NULL,
			     CMAP_PALETTED_TRUECOLOR
			     | CMAP_RELOAD_ON_MODE_SWITCH
#if 0 /* This option messes up text mode! (eich@suse.de) */
			     | CMAP_LOAD_EVEN_IF_OFFSCREEN
#endif
			     )) return FALSE;

				/* DPMS setup */
#ifdef ENABLE_FLAT_PANEL
    if (!info->HasPanelRegs || info->CRTOnly)
#endif
	xf86DPMSInit(pScreen, RADEONDisplayPowerManagementSet, 0);

    RADEONInitVideo(pScreen);

				/* Provide SaveScreen */
    pScreen->SaveScreen  = RADEONSaveScreen;

				/* Wrap CloseScreen */
    info->CloseScreen    = pScreen->CloseScreen;
    pScreen->CloseScreen = RADEONCloseScreen;

				/* Note unused options */
    if (serverGeneration == 1)
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);

#ifdef XF86DRI
				/* DRI finalization */
    if (info->directRenderingEnabled) {
				/* Now that mi, cfb, drm and others have
				   done their thing, complete the DRI
				   setup. */
	info->directRenderingEnabled = RADEONDRIFinishScreenInit(pScreen);
    }
    if (info->directRenderingEnabled) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Direct rendering enabled\n");
    } else {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Direct rendering disabled\n");
    }
#endif

    info->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = RADEONBlockHandler;

    return TRUE;
}

/* Write common registers (initialized to 0). */
static void RADEONRestoreCommonRegisters(ScrnInfoPtr pScrn,
					 RADEONSavePtr restore)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTREG(RADEON_OVR_CLR,              restore->ovr_clr);
    OUTREG(RADEON_OVR_WID_LEFT_RIGHT,   restore->ovr_wid_left_right);
    OUTREG(RADEON_OVR_WID_TOP_BOTTOM,   restore->ovr_wid_top_bottom);
    OUTREG(RADEON_OV0_SCALE_CNTL,       restore->ov0_scale_cntl);
    OUTREG(RADEON_MPP_TB_CONFIG,        restore->mpp_tb_config );
    OUTREG(RADEON_MPP_GP_CONFIG,        restore->mpp_gp_config );
    OUTREG(RADEON_SUBPIC_CNTL,          restore->subpic_cntl);
    OUTREG(RADEON_VIPH_CONTROL,         restore->viph_control);
    OUTREG(RADEON_I2C_CNTL_1,           restore->i2c_cntl_1);
    OUTREG(RADEON_GEN_INT_CNTL,         restore->gen_int_cntl);
    OUTREG(RADEON_CAP0_TRIG_CNTL,       restore->cap0_trig_cntl);
    OUTREG(RADEON_CAP1_TRIG_CNTL,       restore->cap1_trig_cntl);
    OUTREG(RADEON_BUS_CNTL,             restore->bus_cntl);
}

/* Write CRTC registers. */
static void RADEONRestoreCrtcRegisters(ScrnInfoPtr pScrn,
				       RADEONSavePtr restore)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTREG(RADEON_CRTC_GEN_CNTL,        restore->crtc_gen_cntl);

    OUTREGP(RADEON_CRTC_EXT_CNTL, restore->crtc_ext_cntl,
	    RADEON_CRTC_VSYNC_DIS |
	    RADEON_CRTC_HSYNC_DIS |
	    RADEON_CRTC_DISPLAY_DIS);

    OUTREGP(RADEON_DAC_CNTL, restore->dac_cntl,
	    RADEON_DAC_RANGE_CNTL |
	    RADEON_DAC_BLANKING);

    OUTREG(RADEON_CRTC_H_TOTAL_DISP,    restore->crtc_h_total_disp);
    OUTREG(RADEON_CRTC_H_SYNC_STRT_WID, restore->crtc_h_sync_strt_wid);
    OUTREG(RADEON_CRTC_V_TOTAL_DISP,    restore->crtc_v_total_disp);
    OUTREG(RADEON_CRTC_V_SYNC_STRT_WID, restore->crtc_v_sync_strt_wid);
    OUTREG(RADEON_CRTC_OFFSET,          restore->crtc_offset);
    OUTREG(RADEON_CRTC_OFFSET_CNTL,     restore->crtc_offset_cntl);
    OUTREG(RADEON_CRTC_PITCH,           restore->crtc_pitch);
}

#ifdef ENABLE_FLAT_PANEL
/* Note: Radeon flat panel support has been disabled for now */
/* Write flat panel registers */
static void RADEONRestoreFPRegisters(ScrnInfoPtr pScrn, RADEONSavePtr restore)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    CARD32        tmp;

    OUTREG(RADEON_CRTC2_GEN_CNTL,       restore->crtc2_gen_cntl);
    OUTREG(RADEON_FP_CRTC_H_TOTAL_DISP, restore->fp_crtc_h_total_disp);
    OUTREG(RADEON_FP_CRTC_V_TOTAL_DISP, restore->fp_crtc_v_total_disp);
    OUTREG(RADEON_FP_GEN_CNTL,          restore->fp_gen_cntl);
    OUTREG(RADEON_FP_H_SYNC_STRT_WID,   restore->fp_h_sync_strt_wid);
    OUTREG(RADEON_FP_HORZ_STRETCH,      restore->fp_horz_stretch);
    OUTREG(RADEON_FP_PANEL_CNTL,        restore->fp_panel_cntl);
    OUTREG(RADEON_FP_V_SYNC_STRT_WID,   restore->fp_v_sync_strt_wid);
    OUTREG(RADEON_FP_VERT_STRETCH,      restore->fp_vert_stretch);
    OUTREG(RADEON_TMDS_CRC,             restore->tmds_crc);

    tmp = INREG(RADEON_LVDS_GEN_CNTL);
    if ((tmp & (RADEON_LVDS_ON | RADEON_LVDS_BLON)) ==
	(restore->lvds_gen_cntl & (RADEON_LVDS_ON | RADEON_LVDS_BLON))) {
	OUTREG(RADEON_LVDS_GEN_CNTL, restore->lvds_gen_cntl);
    } else {
	if (restore->lvds_gen_cntl & (RADEON_LVDS_ON | RADEON_LVDS_BLON)) {
	    OUTREG(RADEON_LVDS_GEN_CNTL,
		   restore->lvds_gen_cntl & ~RADEON_LVDS_BLON);
	    usleep(RADEONPTR(pScrn)->PanelPwrDly * 1000);
	    OUTREG(RADEON_LVDS_GEN_CNTL, restore->lvds_gen_cntl);
	} else {
	    OUTREG(RADEON_LVDS_GEN_CNTL,
		   restore->lvds_gen_cntl | RADEON_LVDS_BLON);
	    usleep(RADEONPTR(pScrn)->PanelPwrDly * 1000);
	    OUTREG(RADEON_LVDS_GEN_CNTL, restore->lvds_gen_cntl);
	}
    }
}
#endif

#if RADEON_ATOMIC_UPDATE
static void RADEONPLLWaitForReadUpdateComplete(ScrnInfoPtr pScrn)
{
    while (INPLL(pScrn, RADEON_PPLL_REF_DIV) & RADEON_PPLL_ATOMIC_UPDATE_R);
}

static void RADEONPLLWriteUpdate(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTPLLP(pScrn, RADEON_PPLL_REF_DIV, RADEON_PPLL_ATOMIC_UPDATE_W, 0xffff);
}
#endif

/* Write PLL registers. */
static void RADEONRestorePLLRegisters(ScrnInfoPtr pScrn, RADEONSavePtr restore)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

#if !RADEON_ATOMIC_UPDATE
    while ( (INREG(RADEON_CLOCK_CNTL_INDEX) & RADEON_PLL_DIV_SEL) !=
						RADEON_PLL_DIV_SEL) {
#endif
	OUTREGP(RADEON_CLOCK_CNTL_INDEX, RADEON_PLL_DIV_SEL, 0xffff);
#if !RADEON_ATOMIC_UPDATE
    }
#endif

#if RADEON_ATOMIC_UPDATE
    OUTPLLP(pScrn,
	    RADEON_PPLL_CNTL,
	    RADEON_PPLL_RESET
	    | RADEON_PPLL_ATOMIC_UPDATE_EN
	    | RADEON_PPLL_VGA_ATOMIC_UPDATE_EN,
	    0xffff);
#else
    OUTPLLP(pScrn,
	    RADEON_PPLL_CNTL,
	    RADEON_PPLL_RESET,
	    0xffff);
#endif

#if RADEON_ATOMIC_UPDATE
    RADEONPLLWaitForReadUpdateComplete(pScrn);
#endif
    while ( (INPLL(pScrn, RADEON_PPLL_REF_DIV) & RADEON_PPLL_REF_DIV_MASK) !=
			(restore->ppll_ref_div & RADEON_PPLL_REF_DIV_MASK)) {
	OUTPLLP(pScrn, RADEON_PPLL_REF_DIV,
			restore->ppll_ref_div, ~RADEON_PPLL_REF_DIV_MASK);
    }
#if RADEON_ATOMIC_UPDATE
    RADEONPLLWriteUpdate(pScrn);
#endif

#if RADEON_ATOMIC_UPDATE
    RADEONPLLWaitForReadUpdateComplete(pScrn);
#endif
    while ( (INPLL(pScrn, RADEON_PPLL_DIV_3) & RADEON_PPLL_FB3_DIV_MASK) !=
			(restore->ppll_div_3 & RADEON_PPLL_FB3_DIV_MASK)) {
	OUTPLLP(pScrn, RADEON_PPLL_DIV_3,
			restore->ppll_div_3, ~RADEON_PPLL_FB3_DIV_MASK);
    }
#if RADEON_ATOMIC_UPDATE
    RADEONPLLWriteUpdate(pScrn);
#endif

#if RADEON_ATOMIC_UPDATE
    RADEONPLLWaitForReadUpdateComplete(pScrn);
#endif
    while ( (INPLL(pScrn, RADEON_PPLL_DIV_3) & RADEON_PPLL_POST3_DIV_MASK) !=
			(restore->ppll_div_3 & RADEON_PPLL_POST3_DIV_MASK)) {
	OUTPLLP(pScrn, RADEON_PPLL_DIV_3,
			restore->ppll_div_3, ~RADEON_PPLL_POST3_DIV_MASK);
    }
#if RADEON_ATOMIC_UPDATE
    RADEONPLLWriteUpdate(pScrn);
#endif

#if RADEON_ATOMIC_UPDATE
    RADEONPLLWaitForReadUpdateComplete(pScrn);
#endif
    OUTPLL(RADEON_HTOTAL_CNTL, restore->htotal_cntl);
#if RADEON_ATOMIC_UPDATE
    RADEONPLLWriteUpdate(pScrn);
#endif

    OUTPLLP(pScrn, RADEON_PPLL_CNTL, 0, ~RADEON_PPLL_RESET);

    RADEONTRACE(("Wrote: 0x%08x 0x%08x 0x%08x (0x%08x)\n",
	       restore->ppll_ref_div,
	       restore->ppll_div_3,
	       restore->htotal_cntl,
	       INPLL(pScrn, RADEON_PPLL_CNTL)));
    RADEONTRACE(("Wrote: rd=%d, fd=%d, pd=%d\n",
	       restore->ppll_ref_div & RADEON_PPLL_REF_DIV_MASK,
	       restore->ppll_div_3 & RADEON_PPLL_FB3_DIV_MASK,
	       (restore->ppll_div_3 & RADEON_PPLL_POST3_DIV_MASK) >> 16));
}

/* Write DDA registers. */
static void RADEONRestoreDDARegisters(ScrnInfoPtr pScrn, RADEONSavePtr restore)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    OUTREG(RADEON_DDA_CONFIG, restore->dda_config);
    OUTREG(RADEON_DDA_ON_OFF, restore->dda_on_off);
}

/* Write palette data. */
static void RADEONRestorePalette(ScrnInfoPtr pScrn, RADEONSavePtr restore)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           i;

    if (!restore->palette_valid) return;

#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    /* Select palette 0 (main CRTC) if using FP-enabled chip */
    if (info->HasPanelRegs) PAL_SELECT(0);
#endif

    OUTPAL_START(0);
    for (i = 0; i < 256; i++) {
	RADEONWaitForFifo(pScrn, 32); /* delay */
	OUTPAL_NEXT_CARD32(restore->palette[i]);
    }
}

/* Write out state to define a new video mode.  */
static void RADEONRestoreMode(ScrnInfoPtr pScrn, RADEONSavePtr restore)
{
#ifdef ENABLE_FLAT_PANEL
    RADEONInfoPtr info = RADEONPTR(pScrn);
#endif

    RADEONTRACE(("RADEONRestoreMode(%p)\n", restore));
    RADEONRestoreCommonRegisters(pScrn, restore);
    RADEONRestoreCrtcRegisters(pScrn, restore);
#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    if (info->HasPanelRegs)
	RADEONRestoreFPRegisters(pScrn, restore);
    if (!info->HasPanelRegs || info->CRTOnly)
	RADEONRestorePLLRegisters(pScrn, restore);
#else
    RADEONRestorePLLRegisters(pScrn, restore);
#endif
    RADEONRestoreDDARegisters(pScrn, restore);
    RADEONRestorePalette(pScrn, restore);
}

/* Read common registers. */
static void RADEONSaveCommonRegisters(ScrnInfoPtr pScrn, RADEONSavePtr save)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    save->ovr_clr            = INREG(RADEON_OVR_CLR);
    save->ovr_wid_left_right = INREG(RADEON_OVR_WID_LEFT_RIGHT);
    save->ovr_wid_top_bottom = INREG(RADEON_OVR_WID_TOP_BOTTOM);
    save->ov0_scale_cntl     = INREG(RADEON_OV0_SCALE_CNTL);
    save->mpp_tb_config      = INREG(RADEON_MPP_TB_CONFIG);
    save->mpp_gp_config      = INREG(RADEON_MPP_GP_CONFIG);
    save->subpic_cntl        = INREG(RADEON_SUBPIC_CNTL);
    save->viph_control       = INREG(RADEON_VIPH_CONTROL);
    save->i2c_cntl_1         = INREG(RADEON_I2C_CNTL_1);
    save->gen_int_cntl       = INREG(RADEON_GEN_INT_CNTL);
    save->cap0_trig_cntl     = INREG(RADEON_CAP0_TRIG_CNTL);
    save->cap1_trig_cntl     = INREG(RADEON_CAP1_TRIG_CNTL);
    save->bus_cntl           = INREG(RADEON_BUS_CNTL);
}

/* Read CRTC registers. */
static void RADEONSaveCrtcRegisters(ScrnInfoPtr pScrn, RADEONSavePtr save)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    save->crtc_gen_cntl        = INREG(RADEON_CRTC_GEN_CNTL);
    save->crtc_ext_cntl        = INREG(RADEON_CRTC_EXT_CNTL);
    save->dac_cntl             = INREG(RADEON_DAC_CNTL);
    save->crtc_h_total_disp    = INREG(RADEON_CRTC_H_TOTAL_DISP);
    save->crtc_h_sync_strt_wid = INREG(RADEON_CRTC_H_SYNC_STRT_WID);
    save->crtc_v_total_disp    = INREG(RADEON_CRTC_V_TOTAL_DISP);
    save->crtc_v_sync_strt_wid = INREG(RADEON_CRTC_V_SYNC_STRT_WID);
    save->crtc_offset          = INREG(RADEON_CRTC_OFFSET);
    save->crtc_offset_cntl     = INREG(RADEON_CRTC_OFFSET_CNTL);
    save->crtc_pitch           = INREG(RADEON_CRTC_PITCH);
}

#ifdef ENABLE_FLAT_PANEL
/* Note: Radeon flat panel support has been disabled for now */
/* Read flat panel registers */
static void RADEONSaveFPRegisters(ScrnInfoPtr pScrn, RADEONSavePtr save)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    save->crtc2_gen_cntl       = INREG(RADEON_CRTC2_GEN_CNTL);
    save->fp_crtc_h_total_disp = INREG(RADEON_FP_CRTC_H_TOTAL_DISP);
    save->fp_crtc_v_total_disp = INREG(RADEON_FP_CRTC_V_TOTAL_DISP);
    save->fp_gen_cntl          = INREG(RADEON_FP_GEN_CNTL);
    save->fp_h_sync_strt_wid   = INREG(RADEON_FP_H_SYNC_STRT_WID);
    save->fp_horz_stretch      = INREG(RADEON_FP_HORZ_STRETCH);
    save->fp_panel_cntl        = INREG(RADEON_FP_PANEL_CNTL);
    save->fp_v_sync_strt_wid   = INREG(RADEON_FP_V_SYNC_STRT_WID);
    save->fp_vert_stretch      = INREG(RADEON_FP_VERT_STRETCH);
    save->lvds_gen_cntl        = INREG(RADEON_LVDS_GEN_CNTL);
    save->tmds_crc             = INREG(RADEON_TMDS_CRC);
}
#endif

/* Read PLL registers. */
static void RADEONSavePLLRegisters(ScrnInfoPtr pScrn, RADEONSavePtr save)
{
    save->ppll_ref_div         = INPLL(pScrn, RADEON_PPLL_REF_DIV);
    save->ppll_div_3           = INPLL(pScrn, RADEON_PPLL_DIV_3);
    save->htotal_cntl          = INPLL(pScrn, RADEON_HTOTAL_CNTL);

    RADEONTRACE(("Read: 0x%08x 0x%08x 0x%08x\n",
	       save->ppll_ref_div,
	       save->ppll_div_3,
	       save->htotal_cntl));
    RADEONTRACE(("Read: rd=%d, fd=%d, pd=%d\n",
	       save->ppll_ref_div & RADEON_PPLL_REF_DIV_MASK,
	       save->ppll_div_3 & RADEON_PPLL_FB3_DIV_MASK,
	       (save->ppll_div_3 & RADEON_PPLL_POST3_DIV_MASK) >> 16));
}

/* Read DDA registers. */
static void RADEONSaveDDARegisters(ScrnInfoPtr pScrn, RADEONSavePtr save)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    save->dda_config           = INREG(RADEON_DDA_CONFIG);
    save->dda_on_off           = INREG(RADEON_DDA_ON_OFF);
}

/* Read palette data. */
static void RADEONSavePalette(ScrnInfoPtr pScrn, RADEONSavePtr save)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           i;

#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    /* Select palette 0 (main CRTC) if using FP-enabled chip */
    if (info->HasPanelRegs) PAL_SELECT(0);
#endif

    INPAL_START(0);
    for (i = 0; i < 256; i++) save->palette[i] = INPAL_NEXT();
    save->palette_valid = TRUE;
}

/* Save state that defines current video mode. */
static void RADEONSaveMode(ScrnInfoPtr pScrn, RADEONSavePtr save)
{
    RADEONTRACE(("RADEONSaveMode(%p)\n", save));

    RADEONSaveCommonRegisters(pScrn, save);
    RADEONSaveCrtcRegisters(pScrn, save);
#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    if (RADEONPTR(pScrn)->HasPanelRegs)
	RADEONSaveFPRegisters(pScrn, save);
#endif
    RADEONSavePLLRegisters(pScrn, save);
    RADEONSaveDDARegisters(pScrn, save);
    RADEONSavePalette(pScrn, save);

    RADEONTRACE(("RADEONSaveMode returns %p\n", save));
}

/* Save everything needed to restore the original VC state. */
static void RADEONSave(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    RADEONSavePtr save        = &info->SavedReg;
    vgaHWPtr      hwp         = VGAHWPTR(pScrn);

    RADEONTRACE(("RADEONSave\n"));
    if (info->FBDev) {
	fbdevHWSave(pScrn);
	return;
    }
    vgaHWUnlock(hwp);
    vgaHWSave(pScrn, &hwp->SavedReg, VGA_SR_ALL); /* save mode, fonts, cmap */
    vgaHWLock(hwp);

    RADEONSaveMode(pScrn, save);

    save->dp_datatype      = INREG(RADEON_DP_DATATYPE);
    save->rbbm_soft_reset  = INREG(RADEON_RBBM_SOFT_RESET);
    save->clock_cntl_index = INREG(RADEON_CLOCK_CNTL_INDEX);
    save->amcgpio_en_reg   = INREG(RADEON_AMCGPIO_EN_REG);
    save->amcgpio_mask     = INREG(RADEON_AMCGPIO_MASK);
}

/* Restore the original (text) mode. */
static void RADEONRestore(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    RADEONSavePtr restore     = &info->SavedReg;
    vgaHWPtr      hwp         = VGAHWPTR(pScrn);

    RADEONTRACE(("RADEONRestore\n"));
    if (info->FBDev) {
	fbdevHWRestore(pScrn);
	return;
    }

    RADEONBlank(pScrn);

    OUTREG(RADEON_AMCGPIO_MASK,     restore->amcgpio_mask);
    OUTREG(RADEON_AMCGPIO_EN_REG,   restore->amcgpio_en_reg);
    OUTREG(RADEON_CLOCK_CNTL_INDEX, restore->clock_cntl_index);
    OUTREG(RADEON_RBBM_SOFT_RESET,  restore->rbbm_soft_reset);
    OUTREG(RADEON_DP_DATATYPE,      restore->dp_datatype);

    RADEONRestoreMode(pScrn, restore);
    vgaHWUnlock(hwp);
    vgaHWRestore(pScrn, &hwp->SavedReg, VGA_SR_MODE | VGA_SR_FONTS );
    vgaHWLock(hwp);

#if 0
    RADEONWaitForVerticalSync(pScrn);
#endif
    RADEONUnblank(pScrn);
}

/* Define common registers for requested video mode. */
static void RADEONInitCommonRegisters(RADEONSavePtr save, RADEONInfoPtr info)
{
    save->ovr_clr            = 0;
    save->ovr_wid_left_right = 0;
    save->ovr_wid_top_bottom = 0;
    save->ov0_scale_cntl     = 0;
    save->mpp_tb_config      = 0;
    save->mpp_gp_config      = 0;
    save->subpic_cntl        = 0;
    save->viph_control       = 0;
    save->i2c_cntl_1         = 0;
    save->rbbm_soft_reset    = 0;
    save->cap0_trig_cntl     = 0;
    save->cap1_trig_cntl     = 0;
    save->bus_cntl           = info->BusCntl;
    /*
     * If bursts are enabled, turn on discards
     * Radeon doesn't have write bursts
     */
    if (save->bus_cntl & (RADEON_BUS_READ_BURST))
	save->bus_cntl |= RADEON_BUS_RD_DISCARD_EN;
}

/* Define CRTC registers for requested video mode. */
static Bool RADEONInitCrtcRegisters(ScrnInfoPtr pScrn, RADEONSavePtr save,
				  DisplayModePtr mode, RADEONInfoPtr info)
{
    int    format;
    int    hsync_start;
    int    hsync_wid;
    int    hsync_fudge;
    int    vsync_wid;
    int    bytpp;
    int    hsync_fudge_default[] = { 0x00, 0x12, 0x09, 0x09, 0x06, 0x05 };
#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    int    hsync_fudge_fp[]      = { 0x12, 0x11, 0x09, 0x09, 0x05, 0x05 };
    int    hsync_fudge_fp_crt[]  = { 0x12, 0x10, 0x08, 0x08, 0x04, 0x04 };
#endif

    switch (info->CurrentLayout.pixel_code) {
    case 4:  format = 1; bytpp = 0; break;
    case 8:  format = 2; bytpp = 1; break;
    case 15: format = 3; bytpp = 2; break;      /*  555 */
    case 16: format = 4; bytpp = 2; break;      /*  565 */
    case 24: format = 5; bytpp = 3; break;      /*  RGB */
    case 32: format = 6; bytpp = 4; break;      /* xRGB */
    default:
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Unsupported pixel depth (%d)\n", info->CurrentLayout.bitsPerPixel);
	return FALSE;
    }
    RADEONTRACE(("Format = %d (%d bytes per pixel)\n", format, bytpp));

#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    if (info->HasPanelRegs)
	if (info->CRTOnly) hsync_fudge = hsync_fudge_fp_crt[format-1];
	else               hsync_fudge = hsync_fudge_fp[format-1];
    else                   hsync_fudge = hsync_fudge_default[format-1];
#else
    hsync_fudge = hsync_fudge_default[format-1];
#endif

    save->crtc_gen_cntl = (RADEON_CRTC_EXT_DISP_EN
			  | RADEON_CRTC_EN
			  | (format << 8)
			  | ((mode->Flags & V_DBLSCAN)
			     ? RADEON_CRTC_DBL_SCAN_EN
			     : 0)
			  | ((mode->Flags & V_INTERLACE)
			     ? RADEON_CRTC_INTERLACE_EN
			     : 0));

    if (info->Chipset == PCI_CHIP_RADEON_VE) {
	save->crtc_ext_cntl = RADEON_VGA_ATI_LINEAR | RADEON_CRTC_CRT_ON;
    } else {
	save->crtc_ext_cntl = RADEON_VGA_ATI_LINEAR | RADEON_XCRT_CNT_EN;
    }
    save->crtc_ext_cntl = RADEON_VGA_ATI_LINEAR | RADEON_XCRT_CNT_EN;
    save->dac_cntl      = (RADEON_DAC_MASK_ALL
			   | RADEON_DAC_VGA_ADR_EN
			   | (info->dac6bits ? 0 : RADEON_DAC_8BIT_EN));

    save->crtc_h_total_disp = ((((mode->CrtcHTotal / 8) - 1) & 0xffff)
			      | (((mode->CrtcHDisplay / 8) - 1) << 16));

    hsync_wid = (mode->CrtcHSyncEnd - mode->CrtcHSyncStart) / 8;
    if (!hsync_wid)       hsync_wid = 1;
    if (hsync_wid > 0x3f) hsync_wid = 0x3f;

    hsync_start = mode->CrtcHSyncStart - 8 + hsync_fudge;

    save->crtc_h_sync_strt_wid = ((hsync_start & 0x1fff)
				 | (hsync_wid << 16)
				 | ((mode->Flags & V_NHSYNC)
				    ? RADEON_CRTC_H_SYNC_POL
				    : 0));

#if 1
				/* This works for double scan mode. */
    save->crtc_v_total_disp = (((mode->CrtcVTotal - 1) & 0xffff)
			      | ((mode->CrtcVDisplay - 1) << 16));
#else
				/* This is what cce/nbmode.c example code
				   does -- is this correct? */
    save->crtc_v_total_disp = (((mode->CrtcVTotal - 1) & 0xffff)
			      | ((mode->CrtcVDisplay
				  * ((mode->Flags & V_DBLSCAN) ? 2 : 1) - 1)
				 << 16));
#endif

    vsync_wid = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;
    if (!vsync_wid)       vsync_wid = 1;
    if (vsync_wid > 0x1f) vsync_wid = 0x1f;

    save->crtc_v_sync_strt_wid = (((mode->CrtcVSyncStart - 1) & 0xfff)
				 | (vsync_wid << 16)
				 | ((mode->Flags & V_NVSYNC)
				    ? RADEON_CRTC_V_SYNC_POL
				    : 0));
    save->crtc_offset      = 0;
    save->crtc_offset_cntl = 0;

    save->crtc_pitch  = ((pScrn->displayWidth * pScrn->bitsPerPixel) +
			 ((pScrn->bitsPerPixel * 8) -1)) /
			 (pScrn->bitsPerPixel * 8);
    save->crtc_pitch |= save->crtc_pitch << 16;

    RADEONTRACE(("Pitch = %d bytes (virtualX = %d, displayWidth = %d)\n",
		 save->crtc_pitch, pScrn->virtualX,
		 info->CurrentLayout.displayWidth));
    return TRUE;
}

#ifdef ENABLE_FLAT_PANEL
/* Note: Radeon flat panel support has been disabled for now */
/* Define CRTC registers for requested video mode. */
static void RADEONInitFPRegisters(ScrnInfoPtr pScrn, RADEONSavePtr orig,
				  RADEONSavePtr save, DisplayModePtr mode,
				  RADEONInfoPtr info)
{
    int   xres = mode->CrtcHDisplay;
    int   yres = mode->CrtcVDisplay;
    float Hratio, Vratio;

    if (info->CRTOnly) {
	save->crtc_ext_cntl  |= RADEON_CRTC_CRT_ON;
	save->crtc2_gen_cntl  = 0;
	save->fp_gen_cntl     = orig->fp_gen_cntl;
	save->fp_gen_cntl    &= ~(RADEON_FP_FPON |
				  RADEON_FP_CRTC_USE_SHADOW_VEND |
				  RADEON_FP_CRTC_HORZ_DIV2_EN |
				  RADEON_FP_CRTC_HOR_CRT_DIV2_DIS |
				  RADEON_FP_USE_SHADOW_EN);
	save->fp_gen_cntl    |= (RADEON_FP_SEL_CRTC2 |
				 RADEON_FP_CRTC_DONT_SHADOW_VPAR);
	save->fp_panel_cntl   = orig->fp_panel_cntl & ~RADEON_FP_DIGON;
	save->lvds_gen_cntl   = orig->lvds_gen_cntl & ~(RADEON_LVDS_ON |
							RADEON_LVDS_BLON);
	return;
    }

    if (xres > info->PanelXRes) xres = info->PanelXRes;
    if (yres > info->PanelYRes) yres = info->PanelYRes;

    Hratio = (float)xres/(float)info->PanelXRes;
    Vratio = (float)yres/(float)info->PanelYRes;

    save->fp_horz_stretch =
	(((((int)(Hratio * RADEON_HORZ_STRETCH_RATIO_MAX + 0.5))
	   & RADEON_HORZ_STRETCH_RATIO_MASK)
	  << RADEON_HORZ_STRETCH_RATIO_SHIFT) |
	 (orig->fp_horz_stretch & (RADEON_HORZ_PANEL_SIZE |
				   RADEON_HORZ_FP_LOOP_STRETCH |
				   RADEON_HORZ_STRETCH_RESERVED)));
    save->fp_horz_stretch &= ~RADEON_HORZ_AUTO_RATIO_FIX_EN;
    if (Hratio == 1.0) save->fp_horz_stretch &= ~(RADEON_HORZ_STRETCH_BLEND |
						  RADEON_HORZ_STRETCH_ENABLE);
    else               save->fp_horz_stretch |=  (RADEON_HORZ_STRETCH_BLEND |
						  RADEON_HORZ_STRETCH_ENABLE);

    save->fp_vert_stretch =
	(((((int)(Vratio * RADEON_VERT_STRETCH_RATIO_MAX + 0.5))
	   & RADEON_VERT_STRETCH_RATIO_MASK)
	  << RADEON_VERT_STRETCH_RATIO_SHIFT) |
	 (orig->fp_vert_stretch & (RADEON_VERT_PANEL_SIZE |
				   RADEON_VERT_STRETCH_RESERVED)));
    save->fp_vert_stretch &= ~RADEON_VERT_AUTO_RATIO_EN;
    if (Vratio == 1.0) save->fp_vert_stretch &= ~(RADEON_VERT_STRETCH_ENABLE |
						  RADEON_VERT_STRETCH_BLEND);
    else               save->fp_vert_stretch |=  (RADEON_VERT_STRETCH_ENABLE |
						  RADEON_VERT_STRETCH_BLEND);

    save->fp_gen_cntl = (orig->fp_gen_cntl & ~(RADEON_FP_SEL_CRTC2 |
					       RADEON_FP_CRTC_USE_SHADOW_VEND |
					       RADEON_FP_CRTC_HORZ_DIV2_EN |
					       RADEON_FP_CRTC_HOR_CRT_DIV2_DIS |
					       RADEON_FP_USE_SHADOW_EN));
    if (orig->fp_gen_cntl & RADEON_FP_DETECT_SENSE) {
	save->fp_gen_cntl |= (RADEON_FP_CRTC_DONT_SHADOW_VPAR |
			      RADEON_FP_TDMS_EN);
    }

    save->fp_panel_cntl        = orig->fp_panel_cntl;
    save->lvds_gen_cntl        = orig->lvds_gen_cntl;

    save->tmds_crc             = orig->tmds_crc;

    /* Disable CRT output by disabling CRT output and setting the CRT
       DAC to use CRTC2, which we set to 0's.  In the future, we will
       want to use the dual CRTC capabilities of the RADEON to allow both
       the flat panel and external CRT to either simultaneously display
       the same image or display two different images. */
    save->crtc_ext_cntl  &= ~RADEON_CRTC_CRT_ON;
    save->dac_cntl       |= RADEON_DAC_CRT_SEL_CRTC2;
    save->crtc2_gen_cntl  = 0;

    /* WARNING: Be careful about turning on the flat panel */
#if 1
    save->lvds_gen_cntl  |= (RADEON_LVDS_ON | RADEON_LVDS_BLON);
#else
    save->fp_panel_cntl  |= (RADEON_FP_DIGON | RADEON_FP_BLON);
    save->fp_gen_cntl    |= (RADEON_FP_FPON);
#endif

    save->fp_crtc_h_total_disp = save->crtc_h_total_disp;
    save->fp_crtc_v_total_disp = save->crtc_v_total_disp;
    save->fp_h_sync_strt_wid   = save->crtc_h_sync_strt_wid;
    save->fp_v_sync_strt_wid   = save->crtc_v_sync_strt_wid;
}
#endif

/* Define PLL registers for requested video mode. */
static void RADEONInitPLLRegisters(RADEONSavePtr save, RADEONPLLPtr pll,
				   double dot_clock)
{
    unsigned long freq = dot_clock * 100;
    struct {
	int divider;
	int bitvalue;
    } *post_div,
      post_divs[]   = {
				/* From RAGE 128 VR/RAGE 128 GL Register
				   Reference Manual (Technical Reference
				   Manual P/N RRG-G04100-C Rev. 0.04), page
				   3-17 (PLL_DIV_[3:0]).  */
	{  1, 0 },              /* VCLK_SRC                 */
	{  2, 1 },              /* VCLK_SRC/2               */
	{  4, 2 },              /* VCLK_SRC/4               */
	{  8, 3 },              /* VCLK_SRC/8               */
	{  3, 4 },              /* VCLK_SRC/3               */
	{ 16, 5 },              /* VCLK_SRC/16              */
	{  6, 6 },              /* VCLK_SRC/6               */
	{ 12, 7 },              /* VCLK_SRC/12              */
	{  0, 0 }
    };

    if (freq > pll->max_pll_freq)      freq = pll->max_pll_freq;
    if (freq * 12 < pll->min_pll_freq) freq = pll->min_pll_freq / 12;

    for (post_div = &post_divs[0]; post_div->divider; ++post_div) {
	save->pll_output_freq = post_div->divider * freq;
	if (save->pll_output_freq >= pll->min_pll_freq
	    && save->pll_output_freq <= pll->max_pll_freq) break;
    }

    save->dot_clock_freq = freq;
    save->feedback_div   = RADEONDiv(pll->reference_div
				     * save->pll_output_freq,
				     pll->reference_freq);
    save->post_div       = post_div->divider;

    RADEONTRACE(("dc=%d, of=%d, fd=%d, pd=%d\n",
	       save->dot_clock_freq,
	       save->pll_output_freq,
	       save->feedback_div,
	       save->post_div));

    save->ppll_ref_div   = pll->reference_div;
    save->ppll_div_3     = (save->feedback_div | (post_div->bitvalue << 16));
    save->htotal_cntl    = 0;
}

/* Define DDA registers for requested video mode. */
static Bool RADEONInitDDARegisters(ScrnInfoPtr pScrn, RADEONSavePtr save,
				   RADEONPLLPtr pll, RADEONInfoPtr info)
{
    int         DisplayFifoWidth = 128;
    int         DisplayFifoDepth = 32;
    int         XclkFreq;
    int         VclkFreq;
    int         XclksPerTransfer;
    int         XclksPerTransferPrecise;
    int         UseablePrecision;
    int         Roff;
    int         Ron;

    XclkFreq = pll->xclk;

    VclkFreq = RADEONDiv(pll->reference_freq * save->feedback_div,
			 pll->reference_div * save->post_div);

    XclksPerTransfer = RADEONDiv(XclkFreq * DisplayFifoWidth,
				 VclkFreq *
				 (info->CurrentLayout.pixel_bytes * 8));

    UseablePrecision = RADEONMinBits(XclksPerTransfer) + 1;

    XclksPerTransferPrecise = RADEONDiv((XclkFreq * DisplayFifoWidth)
					<< (11 - UseablePrecision),
					VclkFreq *
					(info->CurrentLayout.pixel_bytes * 8));

    Roff  = XclksPerTransferPrecise * (DisplayFifoDepth - 4);

    Ron   = (4 * info->ram->MB
	     + 3 * MAX(info->ram->Trcd - 2, 0)
	     + 2 * info->ram->Trp
	     + info->ram->Twr
	     + info->ram->CL
	     + info->ram->Tr2w
	     + XclksPerTransfer) << (11 - UseablePrecision);

    if (Ron + info->ram->Rloop >= Roff) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "(Ron = %d) + (Rloop = %d) >= (Roff = %d)\n",
		   Ron, info->ram->Rloop, Roff);
	return FALSE;
    }

    save->dda_config = (XclksPerTransferPrecise
			| (UseablePrecision << 16)
			| (info->ram->Rloop << 20));

    save->dda_on_off = (Ron << 16) | Roff;

    RADEONTRACE(("XclkFreq = %d; VclkFreq = %d; per = %d, %d (useable = %d)\n",
		 XclkFreq,
		 VclkFreq,
		 XclksPerTransfer,
		 XclksPerTransferPrecise,
		 UseablePrecision));
    RADEONTRACE(("Roff = %d, Ron = %d, Rloop = %d\n",
		 Roff, Ron, info->ram->Rloop));

    return TRUE;
}


/* Define initial palette for requested video mode.  This doesn't do
   anything for XFree86 4.0. */
static void RADEONInitPalette(RADEONSavePtr save)
{
    save->palette_valid = FALSE;
}

/* Define registers for a requested video mode. */
static Bool RADEONInit(ScrnInfoPtr pScrn, DisplayModePtr mode,
		       RADEONSavePtr save)
{
    RADEONInfoPtr info      = RADEONPTR(pScrn);
    double        dot_clock = mode->Clock/1000.0;

#if RADEON_DEBUG
    ErrorF("%-12.12s %7.2f  %4d %4d %4d %4d  %4d %4d %4d %4d (%d,%d)",
	   mode->name,
	   dot_clock,

	   mode->HDisplay,
	   mode->HSyncStart,
	   mode->HSyncEnd,
	   mode->HTotal,

	   mode->VDisplay,
	   mode->VSyncStart,
	   mode->VSyncEnd,
	   mode->VTotal,
	   pScrn->depth,
	   pScrn->bitsPerPixel);
    if (mode->Flags & V_DBLSCAN)   ErrorF(" D");
    if (mode->Flags & V_INTERLACE) ErrorF(" I");
    if (mode->Flags & V_PHSYNC)    ErrorF(" +H");
    if (mode->Flags & V_NHSYNC)    ErrorF(" -H");
    if (mode->Flags & V_PVSYNC)    ErrorF(" +V");
    if (mode->Flags & V_NVSYNC)    ErrorF(" -V");
    ErrorF("\n");
    ErrorF("%-12.12s %7.2f  %4d %4d %4d %4d  %4d %4d %4d %4d (%d,%d)",
	   mode->name,
	   dot_clock,

	   mode->CrtcHDisplay,
	   mode->CrtcHSyncStart,
	   mode->CrtcHSyncEnd,
	   mode->CrtcHTotal,

	   mode->CrtcVDisplay,
	   mode->CrtcVSyncStart,
	   mode->CrtcVSyncEnd,
	   mode->CrtcVTotal,
	   pScrn->depth,
	   pScrn->bitsPerPixel);
    if (mode->Flags & V_DBLSCAN)   ErrorF(" D");
    if (mode->Flags & V_INTERLACE) ErrorF(" I");
    if (mode->Flags & V_PHSYNC)    ErrorF(" +H");
    if (mode->Flags & V_NHSYNC)    ErrorF(" -H");
    if (mode->Flags & V_PVSYNC)    ErrorF(" +V");
    if (mode->Flags & V_NVSYNC)    ErrorF(" -V");
    ErrorF("\n");
#endif

    info->Flags = mode->Flags;

    RADEONInitCommonRegisters(save, info);
    if (!RADEONInitCrtcRegisters(pScrn, save, mode, info)) return FALSE;
#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    if (info->HasPanelRegs)
	RADEONInitFPRegisters(pScrn, &info->SavedReg, save, mode, info);
#endif
    RADEONInitPLLRegisters(save, &info->pll, dot_clock);
    if (!RADEONInitDDARegisters(pScrn, save, &info->pll, info))
	return FALSE;
    if (!info->PaletteSavedOnVT) RADEONInitPalette(save);

    RADEONTRACE(("RADEONInit returns %p\n", save));
    return TRUE;
}

/* Initialize a new mode. */
static Bool RADEONModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    RADEONInfoPtr info      = RADEONPTR(pScrn);

    if (!RADEONInit(pScrn, mode, &info->ModeReg)) return FALSE;
				/* FIXME?  DRILock/DRIUnlock here? */
    pScrn->vtSema = TRUE;
    RADEONBlank(pScrn);
    RADEONRestoreMode(pScrn, &info->ModeReg);
    RADEONUnblank(pScrn);

    info->CurrentLayout.mode = mode;

    return TRUE;
}

static Bool RADEONSaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr   pScrn = xf86Screens[pScreen->myNum];
    Bool unblank;

    unblank = xf86IsUnblank(mode);
    if (unblank)
	SetTimeSinceLastInputEvent();

    if ((pScrn != NULL) && pScrn->vtSema) {
	if (unblank)
	    RADEONUnblank(pScrn);
	else
	    RADEONBlank(pScrn);
    }
    return TRUE;
}

Bool RADEONSwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
    return RADEONModeInit(xf86Screens[scrnIndex], mode);
}

/* Used to disallow modes that are not supported by the hardware. */
int RADEONValidMode(int scrnIndex, DisplayModePtr mode,
		    Bool verbose, int flag)
{
#ifdef ENABLE_FLAT_PANEL
    /* Note: Radeon flat panel support has been disabled for now */
    ScrnInfoPtr   pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr info  = RADEONPTR(pScrn);

    if (info->HasPanelRegs) {
	if (mode->Flags & V_INTERLACE) return MODE_NO_INTERLACE;
	if (mode->Flags & V_DBLSCAN)   return MODE_NO_DBLESCAN;
    }

    if (info->HasPanelRegs && !info->CRTOnly && info->VBIOS) {
	int i;
	for (i = info->FPBIOSstart+64; RADEON_BIOS16(i) != 0; i += 2) {
	    int j = RADEON_BIOS16(i);

	    if (mode->CrtcHDisplay == RADEON_BIOS16(j) &&
		mode->CrtcVDisplay == RADEON_BIOS16(j+2)) {
		/* Assume we are using expanded mode */
		if (RADEON_BIOS16(j+5)) j  = RADEON_BIOS16(j+5);
		else                    j += 9;

		mode->Clock = (CARD32)RADEON_BIOS16(j) * 10;

		mode->HDisplay   = mode->CrtcHDisplay   =
		    ((RADEON_BIOS16(j+10) & 0x01ff)+1)*8;
		mode->HSyncStart = mode->CrtcHSyncStart =
		    ((RADEON_BIOS16(j+12) & 0x01ff)+1)*8;
		mode->HSyncEnd   = mode->CrtcHSyncEnd   =
		    mode->CrtcHSyncStart + (RADEON_BIOS8(j+14) & 0x1f);
		mode->HTotal     = mode->CrtcHTotal     =
		    ((RADEON_BIOS16(j+8)  & 0x01ff)+1)*8;

		mode->VDisplay   = mode->CrtcVDisplay   =
		    (RADEON_BIOS16(j+17) & 0x07ff)+1;
		mode->VSyncStart = mode->CrtcVSyncStart =
		    (RADEON_BIOS16(j+19) & 0x07ff)+1;
		mode->VSyncEnd   = mode->CrtcVSyncEnd   =
		    mode->CrtcVSyncStart + ((RADEON_BIOS16(j+19) >> 11)&0x1f);
		mode->VTotal     = mode->CrtcVTotal     =
		    (RADEON_BIOS16(j+15) & 0x07ff)+1;

		return MODE_OK;
	    }
	}
	return MODE_NOMODE;
    }
#endif

    return MODE_OK;
}

/* Adjust viewport into virtual desktop such that (0,0) in viewport space
   is (x,y) in virtual space. */
void RADEONAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr   pScrn       = xf86Screens[scrnIndex];
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           Base;

    Base = y * info->CurrentLayout.displayWidth + x;

    switch (info->CurrentLayout.pixel_code) {
    case 15:
    case 16: Base *= 2; break;
    case 24: Base *= 3; break;
    case 32: Base *= 4; break;
    }

    Base &= ~7;                 /* 3 lower bits are always 0 */

    OUTREG(RADEON_CRTC_OFFSET, Base);
}

/* Called when VT switching back to the X server.  Reinitialize the video
   mode. */
Bool RADEONEnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr   pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr info  = RADEONPTR(pScrn);

    RADEONTRACE(("RADEONEnterVT\n"));
#ifdef XF86DRI
    if (RADEONPTR(pScrn)->directRenderingEnabled) {
	RADEONCP_START(pScrn, info);
	DRIUnlock(pScrn->pScreen);
    }
#endif
    if (!RADEONModeInit(pScrn, pScrn->currentMode)) return FALSE;
    if (info->accelOn)
	RADEONEngineRestore(pScrn);

    info->PaletteSavedOnVT = FALSE;
    RADEONAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

    return TRUE;
}

/* Called when VT switching away from the X server.  Restore the original
   text mode. */
void RADEONLeaveVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr info  = RADEONPTR(pScrn);
    RADEONSavePtr save  = &info->ModeReg;

    RADEONTRACE(("RADEONLeaveVT\n"));
#ifdef XF86DRI
    if (RADEONPTR(pScrn)->directRenderingEnabled) {
	DRILock(pScrn->pScreen, 0);
	RADEONCP_STOP(pScrn, info);
    }
#endif
    RADEONSavePalette(pScrn, save);
    info->PaletteSavedOnVT = TRUE;
    RADEONRestore(pScrn);
}

static Bool
RADEONEnterVTFBDev(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    RADEONSavePtr restore = &info->SavedReg;
    fbdevHWEnterVT(scrnIndex,flags);
    RADEONRestorePalette(pScrn,restore);
    if (info->accelOn)
	RADEONEngineRestore(pScrn);
    return TRUE;
}

static void RADEONLeaveVTFBDev(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    RADEONSavePtr save = &info->SavedReg;
    RADEONSavePalette(pScrn,save);
    fbdevHWLeaveVT(scrnIndex,flags);
}

/* Called at the end of each server generation.  Restore the original text
   mode, unmap video memory, and unwrap and call the saved CloseScreen
   function.  */
static Bool RADEONCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr info  = RADEONPTR(pScrn);

    RADEONTRACE(("RADEONCloseScreen\n"));

#ifdef XF86DRI
				/* Disable direct rendering */
    if (info->directRenderingEnabled) {
	RADEONDRICloseScreen(pScreen);
	info->directRenderingEnabled = FALSE;
    }
#endif

    if (pScrn->vtSema) {
	RADEONRestore(pScrn);
	RADEONUnmapMem(pScrn);
    }

    if (info->accel)             XAADestroyInfoRec(info->accel);
    info->accel                  = NULL;

    if (info->scratch_save)      xfree(info->scratch_save);
    info->scratch_save           = NULL;

    if (info->cursor)            xf86DestroyCursorInfoRec(info->cursor);
    info->cursor                 = NULL;

    if (info->DGAModes)          xfree(info->DGAModes);
    info->DGAModes               = NULL;

    pScrn->vtSema = FALSE;

    pScreen->BlockHandler = info->BlockHandler;
    pScreen->CloseScreen = info->CloseScreen;
    return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}

void RADEONFreeScreen(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];

    RADEONTRACE(("RADEONFreeScreen\n"));
    if (xf86LoaderCheckSymbol("vgaHWFreeHWRec"))
	vgaHWFreeHWRec(pScrn);
    RADEONFreeRec(pScrn);
}

/* Sets VESA Display Power Management Signaling (DPMS) Mode.  */
static void RADEONDisplayPowerManagementSet(ScrnInfoPtr pScrn,
					  int PowerManagementMode, int flags)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           mask        = (RADEON_CRTC_DISPLAY_DIS
				 | RADEON_CRTC_HSYNC_DIS
				 | RADEON_CRTC_VSYNC_DIS);

    switch (PowerManagementMode) {
    case DPMSModeOn:
	/* Screen: On; HSync: On, VSync: On */
	OUTREGP(RADEON_CRTC_EXT_CNTL, 0, ~mask);
	break;
    case DPMSModeStandby:
	/* Screen: Off; HSync: Off, VSync: On */
	OUTREGP(RADEON_CRTC_EXT_CNTL,
		RADEON_CRTC_DISPLAY_DIS | RADEON_CRTC_HSYNC_DIS, ~mask);
	break;
    case DPMSModeSuspend:
	/* Screen: Off; HSync: On, VSync: Off */
	OUTREGP(RADEON_CRTC_EXT_CNTL,
		RADEON_CRTC_DISPLAY_DIS | RADEON_CRTC_VSYNC_DIS, ~mask);
	break;
    case DPMSModeOff:
	/* Screen: Off; HSync: Off, VSync: Off */
	OUTREGP(RADEON_CRTC_EXT_CNTL, mask, ~mask);
	break;
    }
}
