/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/i128/i128init.c,v 3.6.2.10 2000/01/08 02:06:33 robin Exp $ */
/*
 * Copyright 1995 by Robin Cutshaw <robin@XFree86.Org>
 * Copyright 1998 by Number Nine Visual Technology, Inc.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Robin Cutshaw not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Robin Cutshaw and Number Nine make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ROBIN CUTSHAW AND NUMBER NINE DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL ROBIN CUTSHAW OR NUMBER NINE BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XConsortium: i128init.c /main/5 1996/10/19 17:52:17 kaleb $ */


#include "i128.h"
#include "i128reg.h"
#include "Ti302X.h"
#include "IBMRGB.h"


i128Registers iR;

static int i128Initialized = 0;
static int i128FontsSaved = 0;
static int i128LUTSaved = 0;
static Bool LUTInited = FALSE;
static LUTENTRY oldlut[256];
#define VGA_SAVE_COUNT 512*1024
static unsigned char vgamem[VGA_SAVE_COUNT];    /* vga text memory */
int i128InitCursorFlag = TRUE;
int i128HDisplay;

extern struct i128mem i128mem;
extern struct i128io i128io;
extern int i128Weight;
extern int i128DisplayWidth;
extern int i128DisplayOffset;
extern int i128DeviceType;
extern int i128MemoryType;
extern int i128RamdacType;
extern int i128DACSyncOnGreen;
extern int i128FlatPanel;
extern Bool i128Doublescan;



#if NeedFunctionPrototypes
void
saveI128state(void)
#else
void
saveI128state()
#endif
{
	/* iobase is filled in during the device probe (as well as config 1&2)*/
	if ((i128io.id&0x7) > 0) {

                /*xf86EnableIOPorts(i128InfoRec.scrnIndex);*/
		iR.vga_ctl = inl(iR.iobase + 0x30);

                /*xf86DisableIOPorts(i128InfoRec.scrnIndex);*/
	}

	if (i128RamdacType == TI3025_DAC) {
		iR.i128_base_g[INDEX_TI] =
			i128mem.rbase_g[INDEX_TI]; /*  0x0018  */
		iR.i128_base_g[DATA_TI]  =
			i128mem.rbase_g[DATA_TI];  /*  0x001C  */
	} else if ((i128RamdacType == IBM526_DAC) ||
		   (i128RamdacType == IBM528_DAC) ||
		   (i128RamdacType == SILVER_HAMMER_DAC)) {
		iR.i128_base_g[IDXL_I] =
			i128mem.rbase_g[IDXL_I];   /*  0x0010  */
		iR.i128_base_g[IDXH_I] =
			i128mem.rbase_g[IDXH_I];   /*  0x0014  */
		iR.i128_base_g[DATA_I] =
			i128mem.rbase_g[DATA_I];   /*  0x0018  */
		iR.i128_base_g[IDXCTL_I] =
			i128mem.rbase_g[IDXCTL_I]; /*  0x001C  */
	}

	iR.i128_base_g[INT_VCNT] = i128mem.rbase_g[INT_VCNT]; /*  0x0020  */
	iR.i128_base_g[INT_HCNT] = i128mem.rbase_g[INT_HCNT]; /*  0x0024  */
	iR.i128_base_g[DB_ADR]   = i128mem.rbase_g[DB_ADR];   /*  0x0028  */
	iR.i128_base_g[DB_PTCH]  = i128mem.rbase_g[DB_PTCH];  /*  0x002C  */
	iR.i128_base_g[CRT_HAC]  = i128mem.rbase_g[CRT_HAC];  /*  0x0030  */
	iR.i128_base_g[CRT_HBL]  = i128mem.rbase_g[CRT_HBL];  /*  0x0034  */
	iR.i128_base_g[CRT_HFP]  = i128mem.rbase_g[CRT_HFP];  /*  0x0038  */
	iR.i128_base_g[CRT_HS]   = i128mem.rbase_g[CRT_HS];   /*  0x003C  */
	iR.i128_base_g[CRT_VAC]  = i128mem.rbase_g[CRT_VAC];  /*  0x0040  */
	iR.i128_base_g[CRT_VBL]  = i128mem.rbase_g[CRT_VBL];  /*  0x0044  */
	iR.i128_base_g[CRT_VFP]  = i128mem.rbase_g[CRT_VFP];  /*  0x0048  */
	iR.i128_base_g[CRT_VS]   = i128mem.rbase_g[CRT_VS];   /*  0x004C  */
	iR.i128_base_g[CRT_BORD] = i128mem.rbase_g[CRT_BORD]; /*  0x0050  */
	iR.i128_base_g[CRT_ZOOM] = i128mem.rbase_g[CRT_ZOOM]; /*  0x0054  */
	iR.i128_base_g[CRT_1CON] = i128mem.rbase_g[CRT_1CON]; /*  0x0058  */
	iR.i128_base_g[CRT_2CON] = i128mem.rbase_g[CRT_2CON]; /*  0x005C  */

	iR.i128_base_w[MW0_CTRL] = i128mem.rbase_w[MW0_CTRL]; /*  0x0000  */
	iR.i128_base_w[MW0_SZ]   = i128mem.rbase_w[MW0_SZ];   /*  0x0008  */
	iR.i128_base_w[MW0_PGE]  = i128mem.rbase_w[MW0_PGE];  /*  0x000C  */
	iR.i128_base_w[MW0_ORG]  = i128mem.rbase_w[MW0_ORG];  /*  0x0010  */
	iR.i128_base_w[MW0_MSRC] = i128mem.rbase_w[MW0_MSRC]; /*  0x0018  */
	iR.i128_base_w[MW0_WKEY] = i128mem.rbase_w[MW0_WKEY]; /*  0x001C  */
	iR.i128_base_w[MW0_KDAT] = i128mem.rbase_w[MW0_KDAT]; /*  0x0020  */
	iR.i128_base_w[MW0_MASK] = i128mem.rbase_w[MW0_MASK]; /*  0x0024  */

	if (i128RamdacType == TI3025_DAC) {
		i128mem.rbase_g[INDEX_TI] = TI_CURS_CONTROL;		MB;
		iR.Ti302X[TI_CURS_CONTROL] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_TRUE_COLOR_CONTROL;	MB;
		iR.Ti302X[TI_TRUE_COLOR_CONTROL] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_VGA_SWITCH_CONTROL;	MB;
		iR.Ti302X[TI_VGA_SWITCH_CONTROL] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_MUX_CONTROL_1;		MB;
		iR.Ti302X[TI_MUX_CONTROL_1] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_MUX_CONTROL_2;		MB;
		iR.Ti302X[TI_MUX_CONTROL_2] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_INPUT_CLOCK_SELECT;	MB;
		iR.Ti302X[TI_INPUT_CLOCK_SELECT] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_OUTPUT_CLOCK_SELECT;	MB;
		iR.Ti302X[TI_OUTPUT_CLOCK_SELECT] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_PALETTE_PAGE;		MB;
		iR.Ti302X[TI_PALETTE_PAGE] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_GENERAL_CONTROL;		MB;
		iR.Ti302X[TI_GENERAL_CONTROL] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_MISC_CONTROL;		MB;
		iR.Ti302X[TI_MISC_CONTROL] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_AUXILIARY_CONTROL;	MB;
		iR.Ti302X[TI_AUXILIARY_CONTROL] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_GENERAL_IO_CONTROL;	MB;
		iR.Ti302X[TI_GENERAL_IO_CONTROL] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_GENERAL_IO_DATA;		MB;
		iR.Ti302X[TI_GENERAL_IO_DATA] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_MCLK_DCLK_CONTROL;	MB;
		iR.Ti302X[TI_MCLK_DCLK_CONTROL] = i128mem.rbase_g[DATA_TI];
		i128mem.rbase_g[INDEX_TI] = TI_COLOR_KEY_CONTROL;	MB;
		iR.Ti302X[TI_COLOR_KEY_CONTROL] = i128mem.rbase_g[DATA_TI];

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x00;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_PIXEL_CLOCK_PLL_DATA;	MB;
		iR.Ti3025[0] = i128mem.rbase_g[DATA_TI];

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x01;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_PIXEL_CLOCK_PLL_DATA;	MB;
		iR.Ti3025[1] = i128mem.rbase_g[DATA_TI];

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x02;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_PIXEL_CLOCK_PLL_DATA;	MB;
		iR.Ti3025[2] = i128mem.rbase_g[DATA_TI];

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x00;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;		MB;
		iR.Ti3025[3] = i128mem.rbase_g[DATA_TI];

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x01;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;		MB;
		iR.Ti3025[4] = i128mem.rbase_g[DATA_TI];

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x02;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;		MB;
		iR.Ti3025[5] = i128mem.rbase_g[DATA_TI];

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x00;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_LOOP_CLOCK_PLL_DATA;	MB;
		iR.Ti3025[6] = i128mem.rbase_g[DATA_TI];

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x01;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_LOOP_CLOCK_PLL_DATA;	MB;
		iR.Ti3025[7] = i128mem.rbase_g[DATA_TI];

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x02;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_LOOP_CLOCK_PLL_DATA;	MB;
		iR.Ti3025[8] = i128mem.rbase_g[DATA_TI];
	} else if ((i128RamdacType == IBM526_DAC) ||
		   (i128RamdacType == IBM528_DAC) ||
		   (i128RamdacType == SILVER_HAMMER_DAC)) {
		CARD32 i;

		for (i=0; i<0x100; i++) {
			i128mem.rbase_g[IDXL_I] = i;			MB;
			iR.IBMRGB[i] = i128mem.rbase_g[DATA_I];
		}
	}

}


#if NeedFunctionPrototypes
void
restoreI128state(void)
#else
void
restoreI128state()
#endif
{
	int i;

	if (i128RamdacType == TI3025_DAC) {
		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x00;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_PIXEL_CLOCK_PLL_DATA;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti3025[0];		MB;

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x01;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_PIXEL_CLOCK_PLL_DATA;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti3025[1];		MB;

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x02;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_PIXEL_CLOCK_PLL_DATA;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti3025[2];		MB;

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x00;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti3025[3];		MB;

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x01;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti3025[4];		MB;

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x02;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti3025[5];		MB;

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x00;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_LOOP_CLOCK_PLL_DATA;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti3025[6];		MB;

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x01;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_LOOP_CLOCK_PLL_DATA;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti3025[7];		MB;

		i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = 0x02;			MB;
		i128mem.rbase_g[INDEX_TI] = TI_LOOP_CLOCK_PLL_DATA;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti3025[8];		MB;

		i128mem.rbase_g[INDEX_TI] = TI_CURS_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_CURS_CONTROL];	MB;
		i128mem.rbase_g[INDEX_TI] = TI_TRUE_COLOR_CONTROL;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_TRUE_COLOR_CONTROL]; MB;
		i128mem.rbase_g[INDEX_TI] = TI_VGA_SWITCH_CONTROL;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_VGA_SWITCH_CONTROL]; MB;
		i128mem.rbase_g[INDEX_TI] = TI_MUX_CONTROL_1;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_MUX_CONTROL_1];	MB;
		i128mem.rbase_g[INDEX_TI] = TI_MUX_CONTROL_2;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_MUX_CONTROL_2];	MB;
		i128mem.rbase_g[INDEX_TI] = TI_INPUT_CLOCK_SELECT;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_INPUT_CLOCK_SELECT]; MB;
		i128mem.rbase_g[INDEX_TI] = TI_OUTPUT_CLOCK_SELECT;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_OUTPUT_CLOCK_SELECT];MB;
		i128mem.rbase_g[INDEX_TI] = TI_PALETTE_PAGE;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_PALETTE_PAGE];	MB;
		i128mem.rbase_g[INDEX_TI] = TI_GENERAL_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_GENERAL_CONTROL]; MB;
		i128mem.rbase_g[INDEX_TI] = TI_MISC_CONTROL;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_MISC_CONTROL];	MB;
		i128mem.rbase_g[INDEX_TI] = TI_AUXILIARY_CONTROL;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_AUXILIARY_CONTROL]; MB;
		i128mem.rbase_g[INDEX_TI] = TI_GENERAL_IO_CONTROL;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_GENERAL_IO_CONTROL]; MB;
		i128mem.rbase_g[INDEX_TI] = TI_GENERAL_IO_DATA;		MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_GENERAL_IO_DATA]; MB;
		i128mem.rbase_g[INDEX_TI] = TI_MCLK_DCLK_CONTROL;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_MCLK_DCLK_CONTROL]; MB;
		i128mem.rbase_g[INDEX_TI] = TI_COLOR_KEY_CONTROL;	MB;
		i128mem.rbase_g[DATA_TI] = iR.Ti302X[TI_COLOR_KEY_CONTROL]; MB;
	} else if ((i128RamdacType == IBM526_DAC) ||
		   (i128RamdacType == IBM528_DAC) ||
		   (i128RamdacType == SILVER_HAMMER_DAC)) {
		CARD32 i;

		for (i=0; i<0x100; i++) {
			if ((i == IBMRGB_sysclk_vco_div) ||
			    (i == IBMRGB_sysclk_ref_div))
				continue;
			i128mem.rbase_g[IDXL_I] = i;			MB;
			i128mem.rbase_g[DATA_I] = iR.IBMRGB[i];		MB;
		}

   		i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_ref_div;	MB;
   		i128mem.rbase_g[DATA_I] =
			iR.IBMRGB[IBMRGB_sysclk_ref_div];		MB;
   		i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_vco_div;	MB;
   		i128mem.rbase_g[DATA_I] =
			iR.IBMRGB[IBMRGB_sysclk_vco_div];		MB;
		usleep(50000);
	}

        /*xf86EnableIOPorts(i128InfoRec.scrnIndex);*/

	if (i128MemoryType == I128_MEMORY_SGRAM) {
		outl(iR.iobase + 0x24, iR.sgram & 0x7FFFFFFF);
		outl(iR.iobase + 0x24, iR.sgram | 0x80000000);
	}

	/* iobase is filled in during the device probe (as well as config 1&2)*/
	if (((i128io.id&0x7) > 0) || (i128DeviceType == I128_DEVICE_ID3)
			          || (i128DeviceType == I128_DEVICE_ID4)) {
		int i;
		unsigned char *vidmem = (unsigned char *)i128mem.mw0_ad;

		for (i=0; i<VGA_SAVE_COUNT; i++)
			vidmem[i] = vgamem[i];
		outl(iR.iobase + 0x30, iR.vga_ctl);
	}

	/* restore the LUT */

	i128mem.rbase_g[PEL_MASK] = 0xff;				MB;
	i128mem.rbase_g[WR_ADR] = 0x00;					MB;

	for (i=0; i<256; i++) {
	   i128mem.rbase_g[PAL_DAT] = oldlut[i].r;			MB;
	   i128mem.rbase_g[PAL_DAT] = oldlut[i].g;			MB;
	   i128mem.rbase_g[PAL_DAT] = oldlut[i].b;			MB;
	}

	i128mem.rbase_w[MW0_CTRL] = iR.i128_base_w[MW0_CTRL]; /*  0x0000  */
	i128mem.rbase_w[MW0_SZ]   = iR.i128_base_w[MW0_SZ];   /*  0x0008  */
	i128mem.rbase_w[MW0_PGE]  = iR.i128_base_w[MW0_PGE];  /*  0x000C  */
	i128mem.rbase_w[MW0_ORG]  = iR.i128_base_w[MW0_ORG];  /*  0x0010  */
	i128mem.rbase_w[MW0_MSRC] = iR.i128_base_w[MW0_MSRC]; /*  0x0018  */
	i128mem.rbase_w[MW0_WKEY] = iR.i128_base_w[MW0_WKEY]; /*  0x001C  */
	i128mem.rbase_w[MW0_KDAT] = iR.i128_base_w[MW0_KDAT]; /*  0x0020  */
	i128mem.rbase_w[MW0_MASK] = iR.i128_base_w[MW0_MASK]; /*  0x0024  */
									MB;

	if (i128RamdacType == TI3025_DAC) {
		i128mem.rbase_g[INDEX_TI] =
			iR.i128_base_g[INDEX_TI]; /* 0x0018 */		MB;
		i128mem.rbase_g[DATA_TI]  =
			iR.i128_base_g[DATA_TI];  /* 0x001C */		MB;
	} else if ((i128RamdacType == IBM526_DAC) ||
		   (i128RamdacType == IBM528_DAC) ||
		   (i128RamdacType == SILVER_HAMMER_DAC)) {
		i128mem.rbase_g[IDXL_I] =
			iR.i128_base_g[IDXL_I];   /* 0x0010 */		MB;
		i128mem.rbase_g[IDXH_I] =
			iR.i128_base_g[IDXH_I];   /* 0x0014 */		MB;
		i128mem.rbase_g[DATA_I] =
			iR.i128_base_g[DATA_I];   /* 0x0018 */		MB;
		i128mem.rbase_g[IDXCTL_I] =
			iR.i128_base_g[IDXCTL_I]; /* 0x001C */		MB;
	}

	i128mem.rbase_g[INT_VCNT] = iR.i128_base_g[INT_VCNT]; /*  0x0020  */
	i128mem.rbase_g[INT_HCNT] = iR.i128_base_g[INT_HCNT]; /*  0x0024  */
	i128mem.rbase_g[DB_ADR]   = iR.i128_base_g[DB_ADR];   /*  0x0028  */
	i128mem.rbase_g[DB_PTCH]  = iR.i128_base_g[DB_PTCH];  /*  0x002C  */
	i128mem.rbase_g[CRT_HAC]  = iR.i128_base_g[CRT_HAC];  /*  0x0030  */
	i128mem.rbase_g[CRT_HBL]  = iR.i128_base_g[CRT_HBL];  /*  0x0034  */
	i128mem.rbase_g[CRT_HFP]  = iR.i128_base_g[CRT_HFP];  /*  0x0038  */
	i128mem.rbase_g[CRT_HS]   = iR.i128_base_g[CRT_HS];   /*  0x003C  */
	i128mem.rbase_g[CRT_VAC]  = iR.i128_base_g[CRT_VAC];  /*  0x0040  */
	i128mem.rbase_g[CRT_VBL]  = iR.i128_base_g[CRT_VBL];  /*  0x0044  */
	i128mem.rbase_g[CRT_VFP]  = iR.i128_base_g[CRT_VFP];  /*  0x0048  */
	i128mem.rbase_g[CRT_VS]   = iR.i128_base_g[CRT_VS];   /*  0x004C  */
	i128mem.rbase_g[CRT_BORD] = iR.i128_base_g[CRT_BORD]; /*  0x0050  */
	i128mem.rbase_g[CRT_ZOOM] = iR.i128_base_g[CRT_ZOOM]; /*  0x0054  */
	i128mem.rbase_g[CRT_1CON] = iR.i128_base_g[CRT_1CON]; /*  0x0058  */
	i128mem.rbase_g[CRT_2CON] = iR.i128_base_g[CRT_2CON]; /*  0x005C  */
									MB;

	outl(iR.iobase + 0x20, iR.config2);
	outl(iR.iobase + 0x1C, iR.config1);

	/*xf86DisableIOPorts(i128InfoRec.scrnIndex);*/
}


#if NeedFunctionPrototypes
void
i128CleanUp(void)
#else
void
i128CleanUp()
#endif
{
	restoreI128state();
	/*xf86DisableIOPorts(i128InfoRec.scrnIndex);*/
}


#if NeedFunctionPrototypes
Bool
i128Init(DisplayModePtr mode)
#else
Bool
i128Init(mode)
	DisplayModePtr mode;
#endif
{
	int pitch_multiplier, iclock;
	Bool ret;
	CARD32 tmp;
	int doubled = 1;

	if (mode->Flags & V_DBLSCAN)
		doubled = 2;

	i128HDisplay = mode->HDisplay;

	/* config 1 and 2 were saved in Probe()
	 * we reset here again in case there was a VT switch
	 */

	xf86EnableIOPorts(i128InfoRec.scrnIndex);
	outl(iR.iobase + 0x1C, i128io.config1);
	outl(iR.iobase + 0x20, i128io.config2);
	/*xf86DisableIOPorts(i128InfoRec.scrnIndex);*/

	if (!i128Initialized)
		saveI128state();

	if (i128InfoRec.bitsPerPixel == 32)		pitch_multiplier = 4;
	else if (i128InfoRec.bitsPerPixel == 16)	pitch_multiplier = 2;
	else						pitch_multiplier = 1;

	if (i128RamdacType == TI3025_DAC)
		iclock = 4;
	else if (i128RamdacType == IBM528_DAC)
		iclock = 128 / i128InfoRec.bitsPerPixel;
	else if (i128RamdacType == SILVER_HAMMER_DAC)
		iclock = 64 / i128InfoRec.bitsPerPixel;
	else if ((i128MemoryType == I128_MEMORY_DRAM) ||
		 (i128MemoryType == I128_MEMORY_SGRAM))
		iclock = 32 / i128InfoRec.bitsPerPixel; /* IBM526 DAC 32b bus */
	else
		iclock = 64 / i128InfoRec.bitsPerPixel; /* IBM524/526 DAC */

	i128mem.rbase_g[INT_VCNT] = 0x00;
	i128mem.rbase_g[INT_HCNT] = 0x00;
	i128mem.rbase_g[DB_ADR] = i128DisplayOffset;
	i128mem.rbase_g[DB_PTCH] = i128DisplayWidth * pitch_multiplier;
	i128mem.rbase_g[CRT_HAC] = mode->HDisplay/iclock;
	i128mem.rbase_g[CRT_HBL] = (mode->HTotal - mode->HDisplay)/iclock;
	i128mem.rbase_g[CRT_HFP] = (mode->HSyncStart - mode->HDisplay)/iclock;
	i128mem.rbase_g[CRT_HS] = (mode->HSyncEnd - mode->HSyncStart)/iclock;
	i128mem.rbase_g[CRT_VAC] = mode->VDisplay * doubled;
	i128mem.rbase_g[CRT_VBL] = (mode->VTotal - mode->VDisplay) * doubled;
	i128mem.rbase_g[CRT_VFP] = (mode->VSyncStart - mode->VDisplay)* doubled;
	i128mem.rbase_g[CRT_VS] = (mode->VSyncEnd - mode->VSyncStart) * doubled;
	i128mem.rbase_g[CRT_BORD] = 0x00;
	tmp = 0x00000070;
	if (i128DeviceType == I128_DEVICE_ID3)
		tmp |= 0x00000100;
	if ((i128DeviceType == I128_DEVICE_ID4) && i128FlatPanel)
		tmp |= 0x00000100;	/* Turn on digital flat panel support */
	if (i128DACSyncOnGreen || (mode->Flags & V_CSYNC))
		tmp |= 0x00000004;
	i128mem.rbase_g[CRT_1CON] = tmp;
	if ((i128MemoryType == I128_MEMORY_DRAM) ||
	    (i128MemoryType == I128_MEMORY_SGRAM))
		tmp = 0x20000100;
	else if (i128MemoryType == I128_MEMORY_WRAM)
		tmp = 0x00040100;
	else {
		tmp = 0x00040101;
		if (i128InfoRec.videoRam == 2048)
			tmp |= 0x00000002;
		if ((i128DisplayWidth & (i128DisplayWidth-1)) ||
		    ((i128DisplayWidth * i128InfoRec.bitsPerPixel) > 32768L))
			tmp |= 0x01000000;  /* split transfer */
	}
	i128mem.rbase_g[CRT_2CON] = tmp;
        if (mode->Flags & V_DBLSCAN)
		i128Doublescan = TRUE;
        else
		i128Doublescan = FALSE;
	i128mem.rbase_g[CRT_ZOOM] = (i128Doublescan ? 0x00000001 : 0x00000000);

	i128mem.rbase_w[MW0_CTRL] = 0x00000000;
	switch (i128InfoRec.videoRam) {
		case 2048:
			i128mem.rbase_w[MW0_SZ]   = 0x00000009;
			break;
		case 8192:
			i128mem.rbase_w[MW0_SZ]   = 0x0000000B;
			break;
		case 8192+4096:
			/* no break */
		case 16384:
			i128mem.rbase_w[MW0_SZ]   = 0x0000000C;
			break;
		case 16384+4096:
			/* no break */
		case 16384+8192:
			/* no break */
		case 16384+8192+4096:
			/* no break */
		case 32768:
			i128mem.rbase_w[MW0_SZ]   = 0x0000000D;
			break;
		case 4096:
			/* no break */
		default:
			i128mem.rbase_w[MW0_SZ]   = 0x0000000A;/* default 4MB */
			break;
	}
	i128mem.rbase_w[MW0_PGE]  = 0x00000000;
	i128mem.rbase_w[MW0_ORG]  = 0x00000000;
	i128mem.rbase_w[MW0_MSRC] = 0x00000000;
	i128mem.rbase_w[MW0_WKEY] = 0x00000000;
	i128mem.rbase_w[MW0_KDAT] = 0x00000000;
	i128mem.rbase_w[MW0_MASK] = 0xFFFFFFFF;
									MB;

	if ((i128io.id&0x7) > 0 || i128DeviceType == I128_DEVICE_ID3
			        || i128DeviceType == I128_DEVICE_ID4) {

        	/*xf86EnableIOPorts(i128InfoRec.scrnIndex);*/

	   	i128io.vga_ctl &= 0x0000FF00;
   		i128io.vga_ctl |= 0x00000082;
                if (i128FlatPanel && (mode->Flags & V_DBLSCAN))
		   i128io.vga_ctl |= 0x00000020;  /* Stretch horizontally */
   		outl(iR.iobase + 0x30, i128io.vga_ctl);

                if (i128DeviceType == I128_DEVICE_ID4) {
                        outl(iR.iobase + 0x24, 0x211BF030);
			usleep(5000);
			outl(iR.iobase + 0x24, 0xA11BF030);
		} else if (i128MemoryType == I128_MEMORY_SGRAM) {
			outl(iR.iobase + 0x24, 0x21089030);
			usleep(5000);
			outl(iR.iobase + 0x24, 0xA1089030);
		}

        	/*xf86DisableIOPorts(i128InfoRec.scrnIndex);*/

		if (!i128FontsSaved) {
			int i;
			unsigned char *vidmem = (unsigned char *)i128mem.mw0_ad;

			for (i=0; i<VGA_SAVE_COUNT; i++)
				vgamem[i] = vidmem[i];
			i128FontsSaved = 1;
		}

	}

	if (i128RamdacType == TI3025_DAC)
		ret = i128ProgramTi3025(mode->SynthClock);
	else if (i128RamdacType == SILVER_HAMMER_DAC)
		ret = i128ProgramSilverHammerDAC(mode->SynthClock,
				                 mode->Flags, mode->HSkew);
	else
		ret = i128ProgramIBMRGB(mode->SynthClock, mode->Flags);

	i128InitCursorFlag = TRUE;
	i128Initialized = 1;

	return(ret);
}


#if NeedFunctionPrototypes
static void
InitLUT(void)
#else
static void
InitLUT()
#endif
{
   short i, j;

   i128mem.rbase_g[PEL_MASK] = 0xff;					MB;

   if (!i128LUTSaved) {
   	i128mem.rbase_g[RD_ADR] = 0x00;					MB;
   	for (i=0; i<256; i++) {
   	   oldlut[i].r = i128mem.rbase_g[PAL_DAT];			MB;
   	   oldlut[i].g = i128mem.rbase_g[PAL_DAT];			MB;
   	   oldlut[i].b = i128mem.rbase_g[PAL_DAT];			MB;
   	}
	i128LUTSaved = 1;
   }
      
   i128mem.rbase_g[WR_ADR] = 0x00;					MB;

   for (i=0; i<256; i++) {
      i128mem.rbase_g[PAL_DAT] = 0x00;					MB;
      i128mem.rbase_g[PAL_DAT] = 0x00;					MB;
      i128mem.rbase_g[PAL_DAT] = 0x00;					MB;
   }

   if (i128InfoRec.bitsPerPixel > 8) {
      int r,g,b;
      int mr,mg,mb;
      int nr=5, ng=5, nb=5;
      extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
      extern LUTENTRY currenti128dac[];

      if (!LUTInited) {
	 if (i128Weight == RGB32_888) {
	    for(i=0; i<256; i++) {
	       currenti128dac[i].r = xf86rGammaMap[i];
	       currenti128dac[i].g = xf86gGammaMap[i];
	       currenti128dac[i].b = xf86bGammaMap[i];
	    }
	 } else {
	    if (i128Weight == RGB16_565) ng = 6;
	    mr = (1<<nr)-1;
	    mg = (1<<ng)-1;
	    mb = (1<<nb)-1;
	    
	    for(i=0; i<256; i++) {
	       r = (i >> (6-nr)) & mr;
	       g = (i >> (6-ng)) & mg;
	       b = (i >> (6-nb)) & mb;
	       currenti128dac[i].r = xf86rGammaMap[(r*255+mr/2)/mr];
	       currenti128dac[i].g = xf86gGammaMap[(g*255+mg/2)/mg];
	       currenti128dac[i].b = xf86bGammaMap[(b*255+mb/2)/mb];
	    }
	 }
      }

      i128mem.rbase_g[WR_ADR] = 0x00;					MB;
      for(i=0; i<256; i++) {
	 i128mem.rbase_g[PAL_DAT] = currenti128dac[i].r;		MB;
	 i128mem.rbase_g[PAL_DAT] = currenti128dac[i].g;		MB;
	 i128mem.rbase_g[PAL_DAT] = currenti128dac[i].b;		MB;
      }
   }
   LUTInited = TRUE;
}


#if NeedFunctionPrototypes
void
i128InitEnvironment(void)
#else
void
i128InitEnvironment()
#endif
{
   InitLUT();
}
