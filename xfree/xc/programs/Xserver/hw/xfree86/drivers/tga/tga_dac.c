/*
 * Copyright 1997,1998 by Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/tga/tga_dac.c,v 1.11 2000/10/20 12:57:26 alanh Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "BT.h"
#include "tga_regs.h"
#include "tga.h"

static void ICS1562ClockSelect(ScrnInfoPtr pScrn, int freq);
static void ICS9110ClockSelect(ScrnInfoPtr pScrn, int freq);
extern void ICS1562_CalcClockBits(long f, unsigned char *bits);

static void
ICS1562ClockSelect(ScrnInfoPtr pScrn, int freq)
{
  TGAPtr pTga = TGAPTR(pScrn);
  unsigned char pll_bits[7];
  unsigned long temp;
  int i, j;

  /* There lies an ICS1562 Clock Generator. */
  ICS1562_CalcClockBits(freq, pll_bits);

  switch (pTga->Chipset) {
  case PCI_CHIP_DEC21030:
    /*
     * For the DEC 21030 TGA:
     * This requires the 55 clock bits be written in a serial manner to
     * bit 0 of the CLOCK register and on the 56th bit set the hold flag.
     */
    for (i = 0;i <= 6; i++) {
	for (j = 0; j <= 7; j++) {
	    temp = (pll_bits[i] >> (7-j)) & 1;
	    if (i == 6 && j == 7)
	    	temp |= 2;
	    TGA_WRITE_REG(temp, TGA_CLOCK_REG);
	}
    }
    break;

  case PCI_CHIP_TGA2:
    /*
     * For the DEC TGA2:
     * This requires the 55 clock bits be written in a serial manner to
     * bit 0 of the CLOCK register and on the 56th bit set the hold flag.
     */
#if 0
    /* ?? FIXME FIXME FIXME ?? */
    for (i = 0;i <= 6; i++) {
	for (j = 0; j <= 7; j++) {
	    temp = (pll_bits[i] >> (7-j)) & 1;
	    if (i == 6 && j == 7)
	    	temp |= 2;
	    TGA_WRITE_REG(temp, TGA_CLOCK_REG);
	}
    }
#endif
    break;
  }
}

struct monitor_data crystal_table = 
{
/*  Option 5 Monitor Info 75.00 Mhz                    */
768,                         /* rows                         */
1024,                        /* columns                      */
75,                          /* 74 Mhz                       */
70,                          /* refresh rate                 */
768,                         /* v scanlines                  */
3,                           /* v front porch                */
6,                           /* v sync                       */
29,                          /* v back porch                 */
1024,                        /* h pixels                     */
24,                          /* h front porch                */
136,                         /* h sync                       */
144,                         /* h back porch                 */
/* 75.00 MHz AV9110 clock serial load information         */
0x6e,                           /* 0:6  VCO frequency divider  N         */
0x15,                           /* 7:13 Reference frequency divide  M   */
0x0,                            /* 14 VCO pre-scale divide V (0=div.by 1,1=by 8)
*/
0x1,                            /* 15:16 CLK/X output divide X          */
0x1,                            /* 17:18 VCO output divide R            */
1,                              /* 19 CLK Output enable. */
1,                              /* 20 CLK/X Output enable */
0,                              /* reserved, should be set to 0         */
0,                              /* Reference clock select on CLK 1=ref  */
1,                              /* reserved, should be set to 1         */
/* 75.00 MHz IBM561 PLL setup data */
0x93,                           /* VCO Div: PFR=2, M=0x54 */
0x8                             /* REF: N=0x8 */
};

/* ICS av9110 is only used on TGA2 */

void
write_av9110(ScrnInfoPtr pScrn, unsigned int *temp)
{
    TGAPtr pTga = TGAPTR(pScrn);

    /* the following is based on write_av9110() from the
       TRU64 kernel TGA driver */

    TGA2_WRITE_CLOCK_REG(0x0, 0xf800);
    TGA2_WRITE_CLOCK_REG(0x0, 0xf000);

    TGA2_WRITE_CLOCK_REG(temp[0], 0x0000);
    TGA2_WRITE_CLOCK_REG(temp[1], 0x0000);
    TGA2_WRITE_CLOCK_REG(temp[2], 0x0000);
    TGA2_WRITE_CLOCK_REG(temp[3], 0x0000);
    TGA2_WRITE_CLOCK_REG(temp[4], 0x0000);
    TGA2_WRITE_CLOCK_REG(temp[5], 0x0000);

    TGA2_WRITE_CLOCK_REG(0x0, 0xf800);
}

static void
ICS9110ClockSelect(ScrnInfoPtr pScrn, int freq)
{
    unsigned int temp, temp1[6];
    struct monitor_data *c_table;

    /* There lies an ICS9110 Clock Generator. */
    /* ICS9110_CalcClockBits(freq, pll_bits); */

    c_table = &crystal_table;

    /* the following is based on munge_ics() from the
       TRU64 kernel TGA driver */

    temp = (unsigned int)(c_table->vco_div |
			  (c_table->ref_div << 7) |
			  (c_table->vco_pre << 14) |
			  (c_table->clk_div << 15) |
			  (c_table->vco_out_div << 17) |
			  (c_table->clk_out_en << 19) |
			  (c_table->clk_out_enX << 20) |
			  (c_table->res0 << 21) |
			  (c_table->clk_sel << 22) |
			  (c_table->res1 << 23));

    temp1[0] = (temp & 0x00000001)         | ((temp & 0x00000002) << 7) |
      ((temp & 0x00000004) << 14) | ((temp & 0x00000008) << 21);

    temp1[1] = ((temp & 0x00000010) >> 4)  | ((temp & 0x00000020) << 3) |
      ((temp & 0x00000040) << 10) | ((temp & 0x00000080) << 17);

    temp1[2] = ((temp & 0x00000100) >> 8)  | ((temp & 0x00000200) >> 1) |
      ((temp & 0x00000400) << 6)  | ((temp & 0x00000800) << 13);

    temp1[3] = ((temp & 0x00001000) >> 12) | ((temp & 0x00002000) >> 5) |
      ((temp & 0x00004000) << 2)  | ((temp & 0x00008000) << 9);

    temp1[4] = ((temp & 0x00010000) >> 16) | ((temp & 0x00020000) >> 9) |
      ((temp & 0x00040000) >> 2)  | ((temp & 0x00080000) << 5);

    temp1[5] = ((temp & 0x00100000) >> 20) | ((temp & 0x00200000) >> 13) |
      ((temp & 0x00400000) >> 6)  | ((temp & 0x00800000) << 1);

    write_av9110(pScrn, temp1);

}

void
Ibm561Init(TGAPtr pTga)
{
    unsigned char *Ibm561 = pTga->Ibm561modeReg;
    int i, j;
 
/* ?? FIXME FIXME FIXME FIXME */

    /* Command registers */
    Ibm561[0] = 0x40;  Ibm561[1] = 0x08;
    Ibm561[2] = (pTga->SyncOnGreen ? 0x80 : 0x00);
	
    /* Read mask */
    Ibm561[3] = 0xff;  Ibm561[4] = 0xff;  Ibm561[5] = 0xff;  Ibm561[6] = 0x0f;

    /* Blink mask */
    Ibm561[7] = 0x00;  Ibm561[8] = 0x00;  Ibm561[9] = 0x00; Ibm561[10] = 0x00;

    /* Window attributes */
    for (i = 0, j=11; i < 16; i++) {
        Ibm561[j++] = 0x00;  Ibm561[j++] = 0x01;  Ibm561[j++] = 0x80;
    }
}

void
Bt463Init(TGAPtr pTga)
{
    unsigned char *Bt463 = pTga->Bt463modeReg;
    int i, j;

    /* Command registers */
    Bt463[0] = 0x40;  Bt463[1] = 0x08;
    Bt463[2] = (pTga->SyncOnGreen ? 0x80 : 0x00);
	
    /* Read mask */
    Bt463[3] = 0xff;  Bt463[4] = 0xff;  Bt463[5] = 0xff;  Bt463[6] = 0x0f;

    /* Blink mask */
    Bt463[7] = 0x00;  Bt463[8] = 0x00;  Bt463[9] = 0x00; Bt463[10] = 0x00;

    /* Window attributes */
    for (i = 0, j=11; i < 16; i++) {
        Bt463[j++] = 0x00;  Bt463[j++] = 0x01;  Bt463[j++] = 0x80;
    }
}

Bool
DEC21030Init(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    TGAPtr pTga = TGAPTR(pScrn);
    TGARegPtr pReg = &pTga->ModeReg;

    if (pTga->RamDac != NULL) { /* this really means 8-bit and BT485 */
        RamDacHWRecPtr pBT = RAMDACHWPTR(pScrn);
	RamDacRegRecPtr ramdacReg = &pBT->ModeReg;

	ramdacReg->DacRegs[BT_COMMAND_REG_0] = 0xA0 |
	    (!pTga->Dac6Bit ? 0x2 : 0x0) | (pTga->SyncOnGreen ? 0x8 : 0x0);
#if 1
	ramdacReg->DacRegs[BT_COMMAND_REG_2] = 0x20;
#else
	ramdacReg->DacRegs[BT_COMMAND_REG_2] = 0x27; /* ?? was 0x20 */
#endif
	ramdacReg->DacRegs[BT_STATUS_REG] = 0x14;
	(*pTga->RamDac->SetBpp)(pScrn, ramdacReg);

    } else {
	switch (pTga->Chipset) {
	case PCI_CHIP_DEC21030: /* always BT463 */
	    Bt463Init(pTga);
	    break;
	case PCI_CHIP_TGA2:	/* always IBM 561 */
	    Ibm561Init(pTga);
	    break;
	}
    }

    pReg->tgaRegs[0x00] = mode->CrtcHDisplay;
    pReg->tgaRegs[0x01] = mode->CrtcHSyncStart - mode->CrtcHDisplay;
    pReg->tgaRegs[0x02] = (mode->CrtcHSyncEnd - mode->CrtcHSyncStart) / 4;
    pReg->tgaRegs[0x03] = (mode->CrtcHTotal - mode->CrtcHSyncEnd) / 4;
    pReg->tgaRegs[0x04] = mode->CrtcVDisplay;
    pReg->tgaRegs[0x05] = mode->CrtcVSyncStart - mode->CrtcVDisplay;
    pReg->tgaRegs[0x06] = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;
    pReg->tgaRegs[0x07] = mode->CrtcVTotal - mode->CrtcVSyncEnd;

    /*
     * We do polarity the Step B way of the 21030 
     * Tell me how I can detect a Step A, and I'll support that too. 
     * But I think that the Step B's are most common 
     */
    if (mode->Flags & V_PHSYNC)
	pReg->tgaRegs[0x08] = 1; /* Horizontal Polarity */
    else
	pReg->tgaRegs[0x08] = 0;

    if (mode->Flags & V_PVSYNC)
	pReg->tgaRegs[0x09] = 1; /* Vertical Polarity */
    else
	pReg->tgaRegs[0x09] = 0;

    pReg->tgaRegs[0x0A] = mode->Clock;

    pReg->tgaRegs[0x10] = (((pReg->tgaRegs[0x00]) / 4) & 0x1FF) |
                ((((pReg->tgaRegs[0x00]) / 4) & 0x600) << 19) |
		(((pReg->tgaRegs[0x01]) / 4) << 9) |
		(pReg->tgaRegs[0x02] << 14) |
		(pReg->tgaRegs[0x03] << 21) |
#if 0
      (1 << 31) | /* ?? */
#endif
		(pReg->tgaRegs[0x08] << 30);
    pReg->tgaRegs[0x11] = pReg->tgaRegs[0x04] |
		(pReg->tgaRegs[0x05] << 11) | 
		(pReg->tgaRegs[0x06] << 16) |
		(pReg->tgaRegs[0x07] << 22) |
		(pReg->tgaRegs[0x09] << 30);

    pReg->tgaRegs[0x12] = 0x01;

    pReg->tgaRegs[0x13] = 0x0000;
    return TRUE;
}

void
DEC21030Save(ScrnInfoPtr pScrn, TGARegPtr tgaReg)
{
    TGAPtr pTga = TGAPTR(pScrn);

    tgaReg->tgaRegs[0x10] = TGA_READ_REG(TGA_HORIZ_REG);
    tgaReg->tgaRegs[0x11] = TGA_READ_REG(TGA_VERT_REG);
    tgaReg->tgaRegs[0x12] = TGA_READ_REG(TGA_VALID_REG);
    tgaReg->tgaRegs[0x13] = TGA_READ_REG(TGA_BASE_ADDR_REG);
    
    return;
}

void
DEC21030Restore(ScrnInfoPtr pScrn, TGARegPtr tgaReg)
{
    TGAPtr pTga = TGAPTR(pScrn);

    TGA_WRITE_REG(0x00, TGA_VALID_REG); /* Disable Video */

    switch (pTga->Chipset) {
    case PCI_CHIP_DEC21030:
        ICS1562ClockSelect(pScrn, tgaReg->tgaRegs[0x0A]);
	break;
    case PCI_CHIP_TGA2:
        ICS9110ClockSelect(pScrn, tgaReg->tgaRegs[0x0A]);
	break;
    }

    TGA_WRITE_REG(tgaReg->tgaRegs[0x10], TGA_HORIZ_REG);
    TGA_WRITE_REG(tgaReg->tgaRegs[0x11], TGA_VERT_REG);
    TGA_WRITE_REG(tgaReg->tgaRegs[0x13], TGA_BASE_ADDR_REG);

    TGA_WRITE_REG(tgaReg->tgaRegs[0x12], TGA_VALID_REG); /* Re-enable Video */

    return;
}
