/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/r128/r128_driver.c,v 1.1.2.2 1999/11/18 15:37:33 hohndel Exp $ */
/**************************************************************************

Copyright 1999 ATI Technologies Inc. and Precision Insight, Inc.,
                                         Cedar Park, Texas. 
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Rickard E. Faith <faith@precisioninsight.com>
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *
 * Credits:
 *
 *   Thanks to Alan Hourihane <alanh@fairlite.demon..co.uk> and SuSE for
 *   providing source code to their 3.3.x Rage 128 driver.  Portions of
 *   this file are based on the initialization code for that driver.
 *
 * References:
 *
 *   RAGE 128 VR/ RAGE 128 GL Register Reference Manual (Technical
 *   Reference Manual P/N RRG-G04100-C Rev. 0.04), ATI Technologies: April
 *   1999.
 *
 *   RAGE 128 Software Development Manual (Technical Reference Manual P/N
 *   SDK-G04000 Rev. 0.01), ATI Technologies: June 1999.
 *
 * $PI: xc/programs/Xserver/hw/xfree86/vga256/drivers/r128/r128_driver.c,v 1.17 1999/10/21 20:47:32 faith Exp $

				/* X and server generic header files */
#include "X.h"
#include "input.h"
#include "screenint.h"

				/* XFree86-specific header files */
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"
#include "xf86cursor.h"

				/* For PCI probing, etc. */
#include "xf86_PCI.h"
#include "vgaPCI.h"

				/* For XF86Config 'Option' flags */
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

				/* DGA includes */
#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

				/* Driver data structures */
#include "r128.h"
#include "r128_reg.h"

				/* From Xserver/cfb/cfb.h for
                                   R128StoreColors */
extern int cfbExpandDirectColors(ColormapPtr pmap, int ndef,
				 xColorItem *indefs, xColorItem *outdefs);


#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct {
    vgaHWRec   vga;		/* For vgaHWInit, which isn't actually used.
				   This may be removed for XFree86 4.0. */

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

				/* Other registers to save for VT switches */
    CARD32     dp_datatype;
    CARD32     gen_reset_cntl;
    CARD32     clock_cntl_index;

				/* Crtc registers */
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

				/* Computed values for PLL */
    int        dot_clock_freq;
    int        pll_output_freq;
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
} R128SaveRec, *R128SavePtr;

				/* Forward definitions for driver functions */
static Bool R128Probe(void);
static char *R128Ident(int n);
static void R128EnterLeave(Bool enter);
static Bool R128Init(DisplayModePtr mode);
static void R128Adjust(int x, int y);
static int  R128ValidMode(DisplayModePtr mode, Bool verbose, int flag);
static void *R128Save(void *s);
static void R128Restore(void *r);
static void R128SaveScreen(int mode);
static void R128FbInit(void);
static void R128DisplayPowerManagementSet(int PowerManagementMode);
static void R128RestorePrivate(R128SavePtr restore, int blank);


				/* Define driver                       */
vgaVideoChipRec R128 = {
				/* Function pointers                   */
    R128Probe,			/* ChipProbe                           */
    R128Ident,			/* ChipIden                            */
    R128EnterLeave,		/* ChipEnterLeave                      */
    R128Init,			/* ChipInit                            */
    R128ValidMode,		/* ChipValidMode                       */
    R128Save,			/* ChipSave                            */
    R128Restore,		/* ChipRestore                         */
    R128Adjust,			/* ChipAdjust                          */
    R128SaveScreen,		/* ChipSaveScreen                      */
    (void (*)())NoopDDA,	/* ChipGetMode                         */
    R128FbInit,			/* ChipFbInit                          */
    (void (*)())NoopDDA,	/* ChipSetRead                         */
    (void (*)())NoopDDA,	/* ChipSetWrite                        */
    (void (*)())NoopDDA,	/* ChipSetReadWrite                    */

				/* Banked VGA access                   */
    0x10000,			/* ChipMapSize (memory mapped window)  */
    0x10000,			/* ChipSegmentSize (video memory bank) */
    16,				/* ChipSegmentShift (right shift)      */
    0xFFFF,			/* ChipSegmentMask                     */
    0x00000,			/* ChipReadBottom                      */
    0x10000,			/* ChipReadTop                         */
    0x00000,			/* ChipWriteBottom                     */
    0x10000,			/* ChipWriteTop                        */
    FALSE,			/* ChipUse2Banks (separate r/w banks?) */
    VGA_NO_DIVIDE_VERT,		/* ChipInterlaceType                   */
    {{0,}},			/* ChipOptionFlags                     */
    64,				/* ChipRounding                        */

				/* Linear framebuffer access           */
    TRUE,			/* ChipUseLinearAddressing             */
    0,				/* ChipLinearBase                      */
    0,				/* ChipLinearSize                      */

				/* Miscellaneous setup                 */
    TRUE,			/* ChipHas16bpp                        */
    TRUE,			/* ChipHas24bpp                        */
    TRUE,			/* ChipHas32bpp                        */
    NULL,			/* ChipBuiltinModes                    */
    1,				/* ChipClockMulFactor                  */
    1				/* ChipClockDivFactor                  */
};

#define newVS ((R128SavePtr)vgaNewVideoState)

/* Don't use globals (for easier porting to XFree86 4.0) */
R128InfoPtr R128PTR(void)
{
    static R128InfoRec info;

    return &info;
}

/* Read PLL information */
int INPLL(int addr)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    OUTREG8(R128_CLOCK_CNTL_INDEX, addr & 0x1f);
    return INREG(R128_CLOCK_CNTL_DATA);
}

#if 0
/* Read PAL information (only used for debugging). */
static int INPAL(int idx)
{
    unsigned char *R128MMIO = R128PTR(NULL)->MMIO;
    
    OUTREG(R128_PALETTE_INDEX, idx << 16);
    return INREG(R128_PALETTE_DATA);
}
#endif

/* Wait for vertical sync. */
void R128WaitForVerticalSync(void)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    int           i;

    OUTREG(R128_GEN_INT_STATUS, R128_VSYNC_INT_AK);
    for (i = 0; i < R128_TIMEOUT; i++) {
	if (INREG(R128_GEN_INT_STATUS) & R128_VSYNC_INT) break;
    }
}

/* Compute log base 2 of val. */
static int R128MinBits(int val)
{
    int bits;

    if (!val) return 1;
    for (bits = 0; val; val >>= 1, ++bits);
    return bits;
}

/* Compute n/d with rounding. */
static int R128Div(int n, int d)
{
    return (n + (d / 2)) / d;
}

/* Read PLL parameters from BIOS block.  Default to typical values if there
   is no BIOS. */
static void R128GetPLLParameters(CARD32 BIOSbase, R128PLLPtr pll)
{
    CARD16        bios_header;
    CARD16        pll_info_block;
    CARD8         tmp[64];

    xf86ReadBIOS(BIOSbase, 0, tmp, sizeof(tmp));
    if (tmp[0] != 0x55 || tmp[1] != 0xaa) {
	R128ERROR(("Video BIOS not detected, using defaults!\n"));
				/* These probably aren't going to work for
                                   the card you are using.  Specifically,
                                   reference freq can be 29.50MHz,
                                   28.63MHz, or 14.32MHz.  YMMV. */
	pll->reference_freq = 2950;
	pll->reference_div  = 65;
	pll->min_pll_freq   = 12500;
	pll->max_pll_freq   = 25000;
	pll->xclk           = 10300;
    } else {
	xf86ReadBIOS(BIOSbase, 0x48, (CARD8 *)&bios_header,
		     sizeof(bios_header));
	xf86ReadBIOS(BIOSbase, bios_header + 0x30,
		     (CARD8 *)&pll_info_block, sizeof(pll_info_block));
	R128DEBUG(("Header at 0x%04x; PLL Information at 0x%04x\n",
		   bios_header, pll_info_block));

	xf86ReadBIOS(BIOSbase, pll_info_block + 0x0e,
		     (CARD8 *)&pll->reference_freq,
		     sizeof(pll->reference_freq));
	xf86ReadBIOS(BIOSbase, pll_info_block + 0x10,
		     (CARD8 *)&pll->reference_div, sizeof(pll->reference_div));
	xf86ReadBIOS(BIOSbase, pll_info_block + 0x12,
		     (CARD8 *)&pll->min_pll_freq, sizeof(pll->min_pll_freq));
	xf86ReadBIOS(BIOSbase, pll_info_block + 0x16,
		     (CARD8 *)&pll->max_pll_freq, sizeof(pll->max_pll_freq));
	xf86ReadBIOS(BIOSbase, pll_info_block + 0x08,
		     (CARD8 *)&pll->xclk, sizeof(pll->xclk));
    }
    
    R128VERBOSE(("PLL parameters: rf=%d rd=%d min=%d max=%d; xclk=%d\n",
		 pll->reference_freq,
		 pll->reference_div,
		 pll->min_pll_freq,
		 pll->max_pll_freq,
		 pll->xclk));
}

/* Return the string name for supported chipset 'n'; NULL otherwise. */
static char *R128Ident(int n)
{
    static char *chipsets[] = { "r128" };

    if (n >= R128_ARRAY_SIZE(chipsets)) return NULL;
    else                                return chipsets[n];
}

/* Return TRUE if chipset is present; FALSE otherwise. */
static Bool R128Probe(void)
{
    R128InfoPtr   info    = R128PTR();
    pciConfigPtr  pcr     = NULL;
    unsigned char *R128MMIO;
    int           i;

				/* Setup VGA IO ports.  We just need enough
                                   to save VC console state -- all other
                                   setup is done using extended
                                   memory-mapped registers. */
    xf86ClearIOPortList(vga256InfoRec.scrnIndex);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);

				/* Check chipset from XF86Config file. */
    if (vga256InfoRec.chipset) {
	char *chipset;
	for (i = 0; (chipset = R128Ident(i)); i++)
	    if (!StrCaseCmp(vga256InfoRec.chipset, chipset)) break;
	if (!chipset) return FALSE;
    }

    info->Chipset = -1;
    if (!vgaPCIInfo || !vgaPCIInfo->AllCards) return FALSE; /* No PCI info */
    
    for (i = 0; info->Chipset == -1 && (pcr = vgaPCIInfo->AllCards[i++]);) {
	if (pcr->_vendor == PCI_VENDOR_ATI
	    && (pcr->_command & PCI_CMD_IO_ENABLE)
	    && (pcr->_command & PCI_CMD_MEM_ENABLE)) {
	    
	    int id = pcr->_device;
	    
				/* Allow override in XF86Config file */
	    if (vga256InfoRec.chipID) {
		R128ERROR(("Chipset override, using ChipID value (0x%04X)"
			   " instead of PCI value (0x%04X)\n",
			   vga256InfoRec.chipID, pcr->_device));
		id = vga256InfoRec.chipID;
	    }
	    
	    switch (id) {
	    case PCI_CHIP_RAGE128RE: /* Rage 128 */
	    case PCI_CHIP_RAGE128RF:
	    case PCI_CHIP_RAGE128RK:
	    case PCI_CHIP_RAGE128RL:
		vga256InfoRec.chipset = R128Ident(0);
		info->Chipset         = id;
		info->LinearAddr      = pcr->_base0   & 0xfc000000;
		info->IOBase          = pcr->_base1   & 0xffffff00;
		info->MMIOAddr        = pcr->_base2   & 0xffffc000;
		break;
	    }
	}
    }

    if (!pcr || info->Chipset == -1) {
	if (vga256InfoRec.chipset) R128ERROR(("Unknown chipset\n"));
	return FALSE;
    }

				/* Rage 128 present. */
    
    info->PciTag = pcibusTag(pcr->_bus, pcr->_cardnum, pcr->_func);

    info->pixel_depth = (vga256InfoRec.depth == 24
			 ? vga256InfoRec.bitsPerPixel
			 : vga256InfoRec.depth);
    info->pixel_bytes = vga256InfoRec.bitsPerPixel / 8;

    R128DEBUG(("Pixel depth = %d bits stored in %d bytes\n",
	       info->pixel_depth, info->pixel_bytes));

    R128DEBUG(("FB 0x%08x; IO 0x%08x; MMIO 0x%08x; BIOS 0x%08x\n",
	       info->LinearAddr, info->IOBase, info->MMIOAddr));
    R128DEBUG(("PCI Tag = bus %d card %d func %d\n",
	       pcr->_bus, pcr->_cardnum, pcr->_func));

				/* Allow override in XF86Config file */
    if (vga256InfoRec.MemBase) {
	if (vga256InfoRec.MemBase != info->LinearAddr) {
	    R128ERROR(("Linear address override, using 0x%08x"
		       " instead of probed value (0x%08x)\n",
		       vga256InfoRec.MemBase, info->LinearAddr));
	}
	info->LinearAddr = vga256InfoRec.MemBase;
    }

				/* Allow override in XF86Config file */
    if (vga256InfoRec.IObase) {
	if (vga256InfoRec.IObase != info->MMIOAddr) {
	    R128ERROR(("MMIO address override, using 0x%08x"
		       " instead of probed value (0x%08x)\n",
		       vga256InfoRec.IObase, info->MMIOAddr));
	}
	info->MMIOAddr = vga256InfoRec.IObase;
    }


				/* Obtain information from registers */
    R128MMIO = info->MMIO = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
					  (pointer)info->MMIOAddr, 0x80000);
    if (!info->MMIO) R128FATAL(("Can't memory map IO registers\n"));
    
    R128DEBUG(("MMIO registers at 0x%08lx\n", info->MMIOAddr));


				/* Allow override in XF86Config file */
    if (!vga256InfoRec.videoRam) {
	vga256InfoRec.videoRam = INREG(R128_CONFIG_MEMSIZE) / 1024;
    }

				/* Force the linear frame buffer size to be
				   a multiple of 1MB.  This fixes a problem
				   with 8bpp rendering, the cause of which
				   has not yet been determined. */
    vga256InfoRec.videoRam &= ~(1024 - 1);

    info->MemCntl = INREG(R128_MEM_CNTL);

    xf86UnMapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION, info->MMIO, 0x80000);
    R128MMIO = info->MMIO = NULL;

    R128DEBUG(("%d kB RAM present on card\n", vga256InfoRec.videoRam));
    R128DEBUG(("MemCntl = 0x%08x\n", info->MemCntl));


    
				/* Read PLL parameters from BIOS */
    R128GetPLLParameters(vga256InfoRec.BIOSbase, &info->pll);



				/* Fill in additional information */
    vga256InfoRec.bankedMono = FALSE;
    vga256InfoRec.maxClock   = info->pll.max_pll_freq * 10;
#ifdef XFreeXDGA
    vga256InfoRec.directMode = XF86DGADirectPresent;
#endif
#ifdef DPMSExtension
    vga256InfoRec.DPMSSet    = R128DisplayPowerManagementSet;
#endif

    
				/* Allow options that impact this chipset */
    OFLG_SET(OPTION_NOACCEL,   &R128.ChipOptionFlags);
    OFLG_SET(OPTION_SW_CURSOR, &R128.ChipOptionFlags);
    OFLG_SET(OPTION_HW_CURSOR, &R128.ChipOptionFlags);
    OFLG_SET(OPTION_DAC_6_BIT, &R128.ChipOptionFlags);
    OFLG_SET(OPTION_DAC_8_BIT, &R128.ChipOptionFlags);
    
				/* Allow overspecification of options */
    OFLG_SET(OPTION_LINEAR,    &R128.ChipOptionFlags);
    OFLG_SET(OPTION_MMIO,      &R128.ChipOptionFlags);
    


				/* Forbid mutually exclusive options */
    if (OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)
	&& OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
	R128FATAL(("Incompatible options: \"sw_cursor\" and \"hw_cursor\"\n"));
    if (OFLG_ISSET(OPTION_DAC_6_BIT, &vga256InfoRec.options)
	&& OFLG_ISSET(OPTION_DAC_8_BIT, &vga256InfoRec.options))
	R128FATAL(("Incompatible options: \"dac_6_bit\" and \"dac_8_bit\"\n"));

				/* Default to 8-bit DAC if "dac_6_bit" not
                                   present or screen depth greater than
                                   8bpp */
    if (!OFLG_ISSET(OPTION_DAC_6_BIT, &vga256InfoRec.options)
	|| info->pixel_bytes > 1)
	OFLG_SET(OPTION_DAC_8_BIT, &vga256InfoRec.options);

				/* No 6-bit DAC option for deep screens */
    if (OFLG_ISSET(OPTION_DAC_6_BIT, &vga256InfoRec.options)
	&& info->pixel_bytes > 1){
	OFLG_CLR(OPTION_DAC_6_BIT, &vga256InfoRec.options);
	R128VERBOSE(("Cannot use \"dac_6_bit\" in %d bpp mode\n",
		     info->pixel_depth));
    }

				/* Avoid clock probing */
    OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);

    return TRUE;
}

/* Save everything needed to restore the original VC state. */
static R128SavePtr R128SaveAll(void)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    R128SavePtr   save = vgaHWSave(NULL, sizeof(*save));

    R128Save(save);
    
    save->dp_datatype      = INREG(R128_DP_DATATYPE);
    save->gen_reset_cntl   = INREG(R128_GEN_RESET_CNTL);
    save->clock_cntl_index = INREG(R128_CLOCK_CNTL_INDEX);
    
    return save;
}

/* Restore the original VC state. */
static void R128RestoreAll(R128SavePtr restore)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    vgaProtect(TRUE);		/* Blank screen */
    OUTREG(R128_CLOCK_CNTL_INDEX, restore->clock_cntl_index);
    OUTREG(R128_GEN_RESET_CNTL,   restore->gen_reset_cntl);
    OUTREG(R128_DP_DATATYPE,      restore->dp_datatype);
    R128RestorePrivate(restore, 0);
    vgaHWRestore(&restore->vga);
    R128WaitForVerticalSync();
    vgaProtect(FALSE);		/* Turn on screen */
}

/* Called at server startup and shutdown, and when VC switching.  On first
   call, save all of the state necessary to restore the VC. */
static void R128EnterLeave(Bool enter)
{
    static R128SavePtr save         = NULL;
    
#ifdef XFreeXDGA
    if ((vga256InfoRec.directMode & XF86DGADirectGraphics) && !enter) {
        if (XAACursorInfoRec.Flags & USE_HARDWARE_CURSOR) 
            XAACursorInfoRec.HideCursor();
	return;
    }
#endif
    
    if (enter) {		/* Enter */
	xf86EnableIOPorts(vga256InfoRec.scrnIndex);
	if (!save) save = R128SaveAll();
    } else {			/* Leave */
	if (save) {
	    R128RestoreAll(save);
	    xfree(save);
	    save = NULL;
	} else {
	    R128ERROR(("Leave without enter\n"));
	}
	xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
    return;
}

/* Write common registers (initialized to 0). */
static void R128RestoreCommonRegisters(R128SavePtr restore)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    OUTREG(R128_OVR_CLR,              restore->ovr_clr);
    OUTREG(R128_OVR_WID_LEFT_RIGHT,   restore->ovr_wid_left_right);
    OUTREG(R128_OVR_WID_TOP_BOTTOM,   restore->ovr_wid_top_bottom);
    OUTREG(R128_OV0_SCALE_CNTL,       restore->ov0_scale_cntl);
    OUTREG(R128_MPP_TB_CONFIG,        restore->mpp_tb_config );
    OUTREG(R128_MPP_GP_CONFIG,        restore->mpp_gp_config );
    OUTREG(R128_SUBPIC_CNTL,          restore->subpic_cntl);
    OUTREG(R128_VIPH_CONTROL,         restore->viph_control);
    OUTREG(R128_I2C_CNTL_1,           restore->i2c_cntl_1);
    OUTREG(R128_GEN_INT_CNTL,         restore->gen_int_cntl);
    OUTREG(R128_CAP0_TRIG_CNTL,       restore->cap0_trig_cntl);
    OUTREG(R128_CAP1_TRIG_CNTL,       restore->cap1_trig_cntl);
}

/* Write CRTC registers. */
static void R128RestoreCrtcRegisters(R128SavePtr restore)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    OUTREG(R128_CRTC_GEN_CNTL,        restore->crtc_gen_cntl);

    OUTREGP(R128_CRTC_EXT_CNTL, restore->crtc_ext_cntl,
	    R128_CRTC_VSYNC_DIS | R128_CRTC_HSYNC_DIS | R128_CRTC_DISPLAY_DIS);
    
    OUTREGP(R128_DAC_CNTL, restore->dac_cntl,
	    R128_DAC_RANGE_CNTL | R128_DAC_BLANKING);

    OUTREG(R128_CRTC_H_TOTAL_DISP,    restore->crtc_h_total_disp);
    OUTREG(R128_CRTC_H_SYNC_STRT_WID, restore->crtc_h_sync_strt_wid);
    OUTREG(R128_CRTC_V_TOTAL_DISP,    restore->crtc_v_total_disp);
    OUTREG(R128_CRTC_V_SYNC_STRT_WID, restore->crtc_v_sync_strt_wid);
    OUTREG(R128_CRTC_OFFSET,          restore->crtc_offset);
    OUTREG(R128_CRTC_OFFSET_CNTL,     restore->crtc_offset_cntl);
    OUTREG(R128_CRTC_PITCH,           restore->crtc_pitch);
}

static void R128PLLWaitForReadUpdateComplete(void)
{
    while (INPLL(R128_PPLL_REF_DIV) & R128_PPLL_ATOMIC_UPDATE_R);
}

static void R128PLLWriteUpdate(void)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    OUTPLLP(R128_PPLL_REF_DIV, R128_PPLL_ATOMIC_UPDATE_W, 0xffff);
}


/* Write PLL registers. */
static void R128RestorePLLRegisters(R128SavePtr restore)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    OUTREGP(R128_CLOCK_CNTL_INDEX, R128_PLL_DIV_SEL, 0xffff);
    
    OUTPLLP(R128_PPLL_CNTL,
	    R128_PPLL_RESET
	    | R128_PPLL_ATOMIC_UPDATE_EN
	    | R128_PPLL_VGA_ATOMIC_UPDATE_EN,
	    0xffff);

    R128PLLWaitForReadUpdateComplete();
    OUTPLLP(R128_PPLL_REF_DIV, restore->ppll_ref_div, ~R128_PPLL_REF_DIV_MASK);
    R128PLLWriteUpdate();
    
    R128PLLWaitForReadUpdateComplete();
    OUTPLLP(R128_PPLL_DIV_3, restore->ppll_div_3, ~R128_PPLL_FB3_DIV_MASK);
    R128PLLWriteUpdate();
    OUTPLLP(R128_PPLL_DIV_3, restore->ppll_div_3, ~R128_PPLL_POST3_DIV_MASK);
    R128PLLWriteUpdate();
    
    R128PLLWaitForReadUpdateComplete();
    OUTPLL(R128_HTOTAL_CNTL, restore->htotal_cntl);
    R128PLLWriteUpdate();

    OUTPLLP(R128_PPLL_CNTL, 0, ~R128_PPLL_RESET);
    
    R128DEBUG(("Wrote: 0x%08x 0x%08x 0x%08x (0x%08x)\n",
	       restore->ppll_ref_div,
	       restore->ppll_div_3,
	       restore->htotal_cntl,
	       INPLL(R128_PPLL_CNTL)));
    R128DEBUG(("Wrote: rd=%d, fd=%d, pd=%d\n",
	       restore->ppll_ref_div & R128_PPLL_REF_DIV_MASK,
	       restore->ppll_div_3 & R128_PPLL_FB3_DIV_MASK,
	       (restore->ppll_div_3 & R128_PPLL_POST3_DIV_MASK) >> 16));
}

/* Write DDA registers. */
static void R128RestoreDDARegisters(R128SavePtr restore)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    OUTREG(R128_DDA_CONFIG, restore->dda_config);
    OUTREG(R128_DDA_ON_OFF, restore->dda_on_off);
}

/* Write palette data. */
static void R128RestorePalette(R128SavePtr restore)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    int           i;

    if (!restore->palette_valid) return;

    OUTPAL_START(0);
    for (i = 0; i < 256; i++) OUTPAL_NEXT_CARD32(restore->palette[i]);
}

/* Write out state to define a new video mode.  Only blank if blank != 0 so
   that this routine can also be used by the R128RestoreAll routine. */
static void R128RestorePrivate(R128SavePtr restore, int blank)
{
    if (blank) vgaProtect(TRUE);		/* Blank screen */
    R128RestoreCommonRegisters(restore);
    R128RestoreCrtcRegisters(restore);
    R128RestorePLLRegisters(restore);
    R128RestoreDDARegisters(restore);
    R128RestorePalette(restore);
    if (blank) vgaProtect(FALSE);		/* Turn on screen */
}

/* Write out state to define a new video mode. */
static void R128Restore(void *r)
{
    R128SavePtr restore = (R128SavePtr)r;
    
    R128DEBUG(("R128Restore(%p)\n", restore));
    
    R128RestorePrivate(restore, 1);
}

/* Read common registers. */
static void R128SaveCommonRegisters(R128SavePtr save)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    save->ovr_clr            = INREG(R128_OVR_CLR);
    save->ovr_wid_left_right = INREG(R128_OVR_WID_LEFT_RIGHT);
    save->ovr_wid_top_bottom = INREG(R128_OVR_WID_TOP_BOTTOM);
    save->ov0_scale_cntl     = INREG(R128_OV0_SCALE_CNTL);
    save->mpp_tb_config      = INREG(R128_MPP_TB_CONFIG);
    save->mpp_gp_config      = INREG(R128_MPP_GP_CONFIG);
    save->subpic_cntl        = INREG(R128_SUBPIC_CNTL);
    save->viph_control       = INREG(R128_VIPH_CONTROL);
    save->i2c_cntl_1         = INREG(R128_I2C_CNTL_1);
    save->gen_int_cntl       = INREG(R128_GEN_INT_CNTL);
    save->cap0_trig_cntl     = INREG(R128_CAP0_TRIG_CNTL);
    save->cap1_trig_cntl     = INREG(R128_CAP1_TRIG_CNTL);
}

/* Read CRTC registers. */
static void R128SaveCrtcRegisters(R128SavePtr save)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    save->crtc_gen_cntl        = INREG(R128_CRTC_GEN_CNTL);
    save->crtc_ext_cntl        = INREG(R128_CRTC_EXT_CNTL);
    save->dac_cntl             = INREG(R128_DAC_CNTL);
    save->crtc_h_total_disp    = INREG(R128_CRTC_H_TOTAL_DISP);
    save->crtc_h_sync_strt_wid = INREG(R128_CRTC_H_SYNC_STRT_WID);
    save->crtc_v_total_disp    = INREG(R128_CRTC_V_TOTAL_DISP);
    save->crtc_v_sync_strt_wid = INREG(R128_CRTC_V_SYNC_STRT_WID);
    save->crtc_offset          = INREG(R128_CRTC_OFFSET);
    save->crtc_offset_cntl     = INREG(R128_CRTC_OFFSET_CNTL);
    save->crtc_pitch           = INREG(R128_CRTC_PITCH);
}

/* Read PLL registers. */
static void R128SavePLLRegisters(R128SavePtr save)
{
    save->ppll_ref_div         = INPLL(R128_PPLL_REF_DIV);
    save->ppll_div_3           = INPLL(R128_PPLL_DIV_3);
    save->htotal_cntl          = INPLL(R128_HTOTAL_CNTL);

    R128DEBUG(("Read: 0x%08x 0x%08x 0x%08x\n",
	       save->ppll_ref_div,
	       save->ppll_div_3,
	       save->htotal_cntl));
    R128DEBUG(("Read: rd=%d, fd=%d, pd=%d\n",
	       save->ppll_ref_div & R128_PPLL_REF_DIV_MASK,
	       save->ppll_div_3 & R128_PPLL_FB3_DIV_MASK,
	       (save->ppll_div_3 & R128_PPLL_POST3_DIV_MASK) >> 16));
}

/* Read DDA registers. */
static void R128SaveDDARegisters(R128SavePtr save)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    
    save->dda_config           = INREG(R128_DDA_CONFIG);
    save->dda_on_off           = INREG(R128_DDA_ON_OFF);
}

/* Read palette data. */
static void R128SavePalette(R128SavePtr save)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    int           i;
    
    INPAL_START(0);
    for (i = 0; i < 256; i++) save->palette[i] = INPAL_NEXT();
    save->palette_valid = TRUE;
}

/* Save state that defines current video mode. */
static void *R128Save(void *s)
{
    R128SavePtr save = s;
    
    R128DEBUG(("R128Save(%p)\n", save));

    if (!save) {
	R128EnterLeave(ENTER);	/* Save vga state */
	save = (void *)xcalloc(1, sizeof(*save));
    }
    R128SaveCommonRegisters(save);
    R128SaveCrtcRegisters(save);
    R128SavePLLRegisters(save);
    R128SaveDDARegisters(save);
    R128SavePalette(save);
    
    R128DEBUG(("R128Save returns %p\n", save));
    return save;
}

/* Define common registers for requested video mode. */
static void R128InitCommonRegisters(R128SavePtr save, DisplayModePtr mode)
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
    save->gen_int_cntl       = 0;
    save->cap0_trig_cntl     = 0;
    save->cap1_trig_cntl     = 0;
}

/* Define CRTC registers for requested video mode. */
static void R128InitCrtcRegisters(R128SavePtr save, DisplayModePtr mode,
				  R128InfoPtr info)
{
    int    format;
    int    hsync_start;
    int    hsync_wid;
    int    hsync_fudge;
    int    vsync_wid;
    int    bytpp;
    
    switch (info->pixel_depth) {
    case 4:  format = 1; bytpp = 0; hsync_fudge =  0; break;
    case 8:  format = 2; bytpp = 1; hsync_fudge = 18; break;
    case 15: format = 3; bytpp = 2; hsync_fudge =  9; break;	/*  555 */
    case 16: format = 4; bytpp = 2; hsync_fudge =  9; break;	/*  565 */
    case 24: format = 5; bytpp = 3; hsync_fudge =  6; break;	/*  RGB */
    case 32: format = 6; bytpp = 4; hsync_fudge =  5; break;	/* xRGB */
    default:
	R128FATAL(("Unsupported pixel depth (%d)\n", info->pixel_depth));
    }
    
    
    save->crtc_gen_cntl = (R128_CRTC_EXT_DISP_EN
			  | R128_CRTC_EN
			  | (format << 8)
			  | ((mode->Flags & V_DBLSCAN)
			     ? R128_CRTC_DBL_SCAN_EN
			     : 0)
			  | ((mode->Flags & V_INTERLACE)
			     ? R128_CRTC_INTERLACE_EN
			     : 0));

    save->crtc_ext_cntl = R128_VGA_ATI_LINEAR | R128_XCRT_CNT_EN;
    save->dac_cntl      = (R128_DAC_MASK_ALL
			  | R128_DAC_VGA_ADR_EN
			  | (OFLG_ISSET(OPTION_DAC_8_BIT,
					&vga256InfoRec.options)
			     ? R128_DAC_8BIT_EN
			     : 0));
    
    save->crtc_h_total_disp = ((((mode->CrtcHTotal / 8) - 1) & 0xffff)
			      | (((mode->CrtcHDisplay / 8) - 1) << 16));
    
    hsync_wid = (mode->CrtcHSyncEnd - mode->CrtcHSyncStart) / 8;
    if (!hsync_wid)       hsync_wid = 1;
    if (hsync_wid > 0x3f) hsync_wid = 0x3f;
    
    hsync_start = mode->CrtcHSyncStart - 8 + hsync_fudge;
    
    save->crtc_h_sync_strt_wid = ((hsync_start & 0xfff)
				 | (hsync_wid << 16)
				 | ((mode->Flags & V_NHSYNC)
				    ? R128_CRTC_H_SYNC_POL
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
				    ? R128_CRTC_V_SYNC_POL
				    : 0));
    save->crtc_offset      = 0;
    save->crtc_offset_cntl = 0;
    save->crtc_pitch       = vga256InfoRec.displayWidth / 8;

    R128DEBUG(("Pitch = %d\n", save->crtc_pitch));
}

/* Define PLL registers for requested video mode. */
static void R128InitPLLRegisters(R128SavePtr save, DisplayModePtr mode,
				 R128PLLPtr pll, double dot_clock)
{
    int freq        = dot_clock * 100;
    struct {
	int divider;
	int bitvalue;
    } *post_div,
      post_divs[]   = {
				/* From RAGE 128 VR/RAGE 128 GL Register
				   Reference Manual (Technical Reference
				   Manual P/N RRG-G04100-C Rev. 0.04), page
				   3-17 (PLL_DIV_[3:0]).  */
	{  1, 0 },		/* VCLK_SRC                 */
	{  2, 1 },		/* VCLK_SRC/2               */
	{  4, 2 },		/* VCLK_SRC/4               */
	{  8, 3 },		/* VCLK_SRC/8               */
	
	{  3, 4 },		/* VCLK_SRC/3               */
				/* bitvalue = 5 is reserved */
	{  6, 6 },		/* VCLK_SRC/6               */
	{ 12, 7 },		/* VCLK_SRC/12              */
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
    save->feedback_div   = R128Div(pll->reference_div * save->pll_output_freq,
				   pll->reference_freq);
    save->post_div       = post_div->divider;
    
    R128DEBUG(("dc=%d, of=%d, fd=%d, pd=%d\n",
	       save->dot_clock_freq,
	       save->pll_output_freq,
	       save->feedback_div,
	       save->post_div));
    
    save->ppll_ref_div   = pll->reference_div;
    save->ppll_div_3     = (save->feedback_div | (post_div->bitvalue << 16));
    save->htotal_cntl    = 0;
}

/* Define DDA registers for requested video mode. */
static void R128InitDDARegisters(R128SavePtr save, DisplayModePtr mode,
				 R128PLLPtr pll, R128InfoPtr info)
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
    int         offset;
    struct {			/* All values in XCLKS    */ 
	int  ML;		/* Memory Read Latency    */
	int  MB;		/* Memory Burst Length    */
	int  Trcd;		/* RAS to CAS delay       */
	int  Trp;		/* RAS percentage         */
	int  Twr;		/* Write Recovery         */
	int  CL;		/* CAS Latency            */
	int  Tr2w;		/* Read to Write Delay    */
	int  Rloop;		/* Loop Latency           */
	int  Rloop_fudge;	/* Add to ML to get Rloop */
	char *name;
    }           ms[] = {	/* Memory Specifications

				   From RAGE 128 Software Development
				   Manual (Technical Reference Manual P/N
				   SDK-G04000 Rev 0.01), page 3-21.  */
	{ 4, 4, 3, 3, 1, 3, 1, 16, 12, "128-bit SDR SGRAM 1:1" },
	{ 4, 8, 3, 3, 1, 3, 1, 17, 13, "64-bit SDR SGRAM 1:1" },
	{ 4, 4, 1, 2, 1, 2, 1, 16, 12, "64-bit SDR SGRAM 2:1" },
	{ 4, 4, 3, 3, 2, 3, 1, 16, 12, "64-bit DDR SGRAM" },
    };

    switch (info->MemCntl & 0x3) {
    case 0:			/* SDR SGRAM 1:1 */
	switch (info->Chipset) {
	case PCI_CHIP_RAGE128RE: 
	case PCI_CHIP_RAGE128RF: offset = 0; break; /* 128-bit SDR SGRAM 1:1 */
	case PCI_CHIP_RAGE128RK:
	case PCI_CHIP_RAGE128RL:
	default:                 offset = 1; break; /*  64-bit SDR SGRAM 1:1 */
	}
    case 1:                      offset = 2; break; /*  64-bit SDR SGRAM 2:1 */
    case 2:                      offset = 3; break; /*  64-bit DDR SGRAM     */
    default:                     offset = 1; break; /*  64-bit SDR SGRAM 1:1 */
    }

    XclkFreq = pll->xclk;
    
#if 1
				/* This is the way the manual does it. */
    VclkFreq = R128Div(pll->reference_freq * save->feedback_div,
		       pll->reference_div * save->post_div);
#else
				/* This turns out to be approximately the
                                   same value. */
    VclkFreq = vga256InfoRec.clock[mode->Clock]/10;
#endif
    
    R128DEBUG(("%f %d\n", vga256InfoRec.clock[mode->Clock]/1000.0, VclkFreq));
		
    XclksPerTransfer = R128Div(XclkFreq * DisplayFifoWidth,
			       VclkFreq * (info->pixel_bytes * 8));
    
    UseablePrecision = R128MinBits(XclksPerTransfer) + 1;
    
    XclksPerTransferPrecise = R128Div((XclkFreq * DisplayFifoWidth)
				      << (11 - UseablePrecision),
				      VclkFreq * (info->pixel_bytes * 8));
    
    Roff  = XclksPerTransferPrecise * (DisplayFifoDepth - 4);
    
    Ron   = (4 * ms[offset].MB
	     + 3 * MAX(ms[offset].Trcd - 2, 0)
	     + 2 * ms[offset].Trp
	     + ms[offset].Twr
	     + ms[offset].CL
	     + ms[offset].Tr2w
	     + XclksPerTransfer) << (11 - UseablePrecision);
    
    if (Ron + ms[offset].Rloop >= Roff)
	R128FATAL(("(Ron = %d) + (Rloop = %d) >= (Roff = %d)\n",
		   Ron, ms[offset].Rloop, Roff));
    
    save->dda_config = (XclksPerTransferPrecise
			| (UseablePrecision << 16)
			| (ms[offset].Rloop << 20));
    
    save->dda_on_off = (Ron << 16) | Roff;
    
    R128DEBUG(("XclkFreq = %d; VclkFreq = %d; per = %d, %d (useable = %d)\n",
	       XclkFreq,
	       VclkFreq,
	       XclksPerTransfer,
	       XclksPerTransferPrecise,
	       UseablePrecision));
    R128DEBUG(("Roff = %d, Ron = %d, Rloop = %d\n",
	       Roff, Ron, ms[offset].Rloop));
}


/* Define initial palette for requested video mode. */
static void R128InitPalette(R128SavePtr save, R128InfoPtr info)
{
    extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
    int                  i;
    
    save->palette_valid = FALSE;
    if (info->pixel_depth > 8) {
	for (i = 0; i < 256; i++)
	    save->palette[i] = ((xf86rGammaMap[i] << 16)
				| (xf86gGammaMap[i] << 8)
				|  xf86bGammaMap[i]);
	save->palette_valid = TRUE;
    }
}

/* Define registers for a requested video mode. */
static Bool R128Init(DisplayModePtr mode)
{
    R128InfoPtr info      = R128PTR();
    double      dot_clock = vga256InfoRec.clock[mode->Clock]/1000.0;

#if R128_DEBUG
    if (xf86Verbose) {
	ErrorF("%-15.15s %7.2f  %4d %4d %4d %4d  %4d %4d %4d %4d (%d,%d)",
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
	       info->pixel_depth,
	       info->pixel_bytes * 8);
	if (mode->Flags & V_DBLSCAN)   ErrorF(" D");
	if (mode->Flags & V_INTERLACE) ErrorF(" I");
	if (mode->Flags & V_PHSYNC)    ErrorF(" +H");
	if (mode->Flags & V_NHSYNC)    ErrorF(" -H");
	if (mode->Flags & V_PVSYNC)    ErrorF(" +V");
	if (mode->Flags & V_NVSYNC)    ErrorF(" -V");
	ErrorF("\n");
    }
#endif

    if (!vgaHWInit(mode, sizeof(R128SaveRec))) return FALSE;

    info->Flags = mode->Flags;
    
    R128InitCommonRegisters(newVS, mode);
    R128InitCrtcRegisters(newVS, mode, info);
    R128InitPLLRegisters(newVS, mode, &info->pll, dot_clock);
    R128InitDDARegisters(newVS, mode, &info->pll, info);
    R128InitPalette(newVS, info);
    
    R128DEBUG(("R128Init returns %p\n", newVS));
    return TRUE;
}

/* This function gets called before and after a synchronous reset is
   performed on the SVGA chipset during a mode-changing operation.  The
   Rage 128 currently doesn't require any register resets, so this is just
   a stub. */
static void R128SaveScreen(int mode)
{
    if (mode == SS_START) {
	return;
    } else {
	return;
    }
}

/* Write color map. */
static void R128StoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
    unsigned char        *R128MMIO    = R128PTR()->MMIO;
    extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
    xColorItem           directDefs[256];
    unsigned char        *cmap;
    int                  i, j;


    if (vgaCheckColorMap(pmap)) return;

    if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
        ndef = cfbExpandDirectColors (pmap, ndef, pdefs, directDefs);
        pdefs = directDefs;
    }

    for (i = 0; i < ndef; i++) {
	cmap    = &((vgaHWPtr)vgaNewVideoState)->DAC[pdefs[i].pixel*3];
        cmap[0] = xf86rGammaMap[pdefs[i].red   >> 8];
        cmap[1] = xf86gGammaMap[pdefs[i].green >> 8];
        cmap[2] = xf86bGammaMap[pdefs[i].blue  >> 8];

	if (!vgaDAC8BitComponents) for (j = 0; j < 3; j++) cmap[j] >>= 2;

	if (xf86VTSema
#ifdef XFreeXDGA
            || ((vga256InfoRec.directMode & XF86DGADirectGraphics)
                && !(vga256InfoRec.directMode & XF86DGADirectColormap))
            || (vga256InfoRec.directMode & XF86DGAHasColormap)
#endif
            ) {
	    OUTPAL(pdefs[i].pixel, cmap[0], cmap[1], cmap[2]);
	}
    }
}

/* Hook to substitute our R128StoreColors into the current pSreen
   structure. */
static Bool R128ScreenInit(ScreenPtr pScreen, pointer pbits, int xsize,
			   int ysize, int dpix, int dpiy, int width)
{
    pScreen->StoreColors = R128StoreColors;
    return TRUE;
}

/* Identify chip, map MMIO registers, initialize hardware cursor and
   acceleration. */
static void R128FbInit(void)
{
    R128InfoPtr  info = R128PTR();

				/* Hook out screen init so that we can
                                   substitute our own StoreColors */
    vgaSetScreenInitHook(R128ScreenInit);

				/* Print out some useful information about
                                   the system. */
    if (xf86Verbose) {
	switch (info->Chipset) {
	case PCI_CHIP_RAGE128RE:
	    R128VERBOSE(("ATI Rage 128 RE (PCI)\n")); /* 312 pin */
	    break;
	case PCI_CHIP_RAGE128RF:
	    R128VERBOSE(("ATI Rage 128 RF (AGP)\n")); /* 312 pin */
	    break;
	case PCI_CHIP_RAGE128RK:
	    R128VERBOSE(("ATI Rage 128 RK (PCI)\n")); /* 256 pin */
	    break;
	case PCI_CHIP_RAGE128RL:
	    R128VERBOSE(("ATI Rage 128 RL (AGP)\n")); /* 256 pin */
	    break;
	}
    }

				/* Map IO registers to virtual address
                                   space. */
    info->MMIO = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
			       (pointer)info->MMIOAddr, 0x80000);
    if (!info->MMIO) R128FATAL(("Can't memory map IO registers\n"));
    R128DEBUG(("Mapped MMIO registers at 0x%08lx to %p\n",
	       info->MMIOAddr, info->MMIO));


    R128.ChipLinearBase          = info->LinearAddr;
    R128.ChipUseLinearAddressing = TRUE;
    R128.ChipLinearSize          = vga256InfoRec.videoRam * 1024;
    
    info->virtual_x              = vga256InfoRec.displayWidth;
    info->virtual_y              = vga256InfoRec.virtualY;
    info->video_ram              = vga256InfoRec.videoRam;
    info->cursor_start           = 0;
    info->cursor_end             = 0;

    R128DEBUG(("Virtual %dx%d\n", info->virtual_x, info->virtual_y));

    if (OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) {
	R128VERBOSE(("Using software cursor\n"));
    } else {
	R128CursorInit();
	R128VERBOSE(("Using hardware cursor\n"));
    }

    if (OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options)) {
	R128VERBOSE(("Acceleration disabled\n"));
    } else {
	R128AccelInit();
	R128VERBOSE(("Acceleration enabled\n"));
    }
}

/* Used to disallow modes that are not supported by the hardware. */
static int R128ValidMode(DisplayModePtr mode, Bool verbose, int flag)
{
    return MODE_OK;
}

/* Adjust viewport into virtual desktop such that (0,0) in viewport space
   is (x,y) in virtual space. */
static void R128Adjust(int x, int y)
{
    R128InfoPtr   info      = R128PTR();
    unsigned char *R128MMIO = info->MMIO;
    int           Base;

    Base = y * info->virtual_x + x;

    switch (info->pixel_depth) {
    case 15:
    case 16: Base *= 2; break;
    case 24: Base *= 3; break;
    case 32: Base *= 4; break;
    }

    Base &= ~7;			/* 3 lower bits are always 0 */

    if (info->pixel_depth == 24)
	Base += 8 * (Base % 3); /* Must be multiple of 8 and 3 */

    OUTREG(R128_CRTC_OFFSET, Base);

#ifdef XFreeXDGA
    if (vga256InfoRec.directMode & XF86DGADirectGraphics)
	R128WaitForVerticalSync();
#endif
}

#ifdef DPMSExtension
/* Sets VESA Display Power Management Signaling (DPMS) Mode.  */
static void R128DisplayPowerManagementSet(int PowerManagementMode)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    int           mask      = (R128_CRTC_DISPLAY_DIS
			       | R128_CRTC_HSYNC_DIS
			       | R128_CRTC_VSYNC_DIS);
    
    if (!xf86VTSema) return;
    
    switch (PowerManagementMode) {
    case DPMSModeOn:
	/* Screen: On; HSync: On, VSync: On */
	OUTREGP(R128_CRTC_EXT_CNTL, 0, ~mask);
	break;
    case DPMSModeStandby:
	/* Screen: Off; HSync: Off, VSync: On */
	OUTREGP(R128_CRTC_EXT_CNTL,
		R128_CRTC_DISPLAY_DIS | R128_CRTC_HSYNC_DIS, ~mask);
	break;
    case DPMSModeSuspend:
	/* Screen: Off; HSync: On, VSync: Off */
	OUTREGP(R128_CRTC_EXT_CNTL,
		R128_CRTC_DISPLAY_DIS | R128_CRTC_VSYNC_DIS, ~mask);
	break;
    case DPMSModeOff:
	/* Screen: Off; HSync: Off, VSync: Off */
	OUTREGP(R128_CRTC_EXT_CNTL, mask, ~mask);
	break;
    }
}
#endif
