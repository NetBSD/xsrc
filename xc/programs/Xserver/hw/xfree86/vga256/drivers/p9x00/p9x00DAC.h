/*
 * Copyright 1996-1997  Joerg Knura (knura@imst.de)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * JOERG KNURA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00DAC.h,v 1.1.2.1 1998/08/25 10:54:14 hohndel Exp $ */

#include "Xmd.h"
#include "xf86.h"
#include "xf86cursor.h"
#include "p9x00Driver.h"

#define DAC_WRITE_ADDR           0x00
#define DAC_RAMDAC_DATA          0x01
#define DAC_PIXEL_MASK           0x02
#define DAC_READ_ADDR            0x03


#define IBMRGB_INDEX_LOW            0x04
#define IBMRGB_INDEX_HIGH           0x05
#define IBMRGB_INDEX_DATA           0x06
#define IBMRGB_INDEX_CONTROL        0x07

#define IBMRGB_rev		0x00
#define IBMRGB_id		0x01
#define IBMRGB_misc_clock	0x02
#define IBMRGB_sync		0x03
#define IBMRGB_hsync_pos	0x04
#define IBMRGB_pwr_mgmt		0x05
#define IBMRGB_dac_op		0x06
#define IBMRGB_pal_ctrl		0x07
#define IBMRGB_sysclk		0x08  /* not RGB525 */
#define IBMRGB_pix_fmt		0x0a
#define IBMRGB_8bpp		0x0b
#define IBMRGB_16bpp		0x0c
#define IBMRGB_24bpp		0x0d
#define IBMRGB_32bpp		0x0e
#define IBMRGB_pll_ctrl1	0x10
#define IBMRGB_pll_ctrl2	0x11
#define IBMRGB_pll_ref_div_fix	0x14
#define IBMRGB_sysclk_ref_div	0x15  /* not RGB525 */
#define IBMRGB_sysclk_vco_div	0x16  /* not RGB525 */
#define IBMRGB_f0		0x20
#define IBMRGB_m0		0x20
#define IBMRGB_n0		0x21
#define IBMRGB_curs		0x30
#define IBMRGB_curs_xl		0x31
#define IBMRGB_curs_xh		0x32
#define IBMRGB_curs_yl		0x33
#define IBMRGB_curs_yh		0x34
#define IBMRGB_curs_hot_x	0x35
#define IBMRGB_curs_hot_y	0x36
#define IBMRGB_curs_col1_r	0x40
#define IBMRGB_curs_col1_g	0x41
#define IBMRGB_curs_col1_b	0x42
#define IBMRGB_curs_col2_r	0x43
#define IBMRGB_curs_col2_g	0x44
#define IBMRGB_curs_col2_b	0x45
#define IBMRGB_curs_col3_r	0x46
#define IBMRGB_curs_col3_g	0x47
#define IBMRGB_curs_col3_b	0x48
#define IBMRGB_border_col_r	0x60
#define IBMRGB_border_col_g	0x61
#define IBMRGB_border_col_b	0x62
#define IBMRGB_misc1		0x70
#define IBMRGB_misc2		0x71
#define IBMRGB_misc3		0x72
#define IBMRGB_misc4		0x73  /* not RGB525 */
#define IBMRGB_dac_sense	0x82
#define IBMRGB_misr_r		0x84
#define IBMRGB_misr_g		0x86
#define IBMRGB_misr_b		0x88
#define IBMRGB_pll_vco_div_in	0x8e
#define IBMRGB_pll_ref_div_in	0x8f
#define IBMRGB_vram_mask_0	0x90
#define IBMRGB_vram_mask_1	0x91
#define IBMRGB_vram_mask_2	0x92
#define IBMRGB_vram_mask_3	0x93
#define IBMRGB_curs_array	0x100


#define BT_CURS_WR_ADDR 	0x04 
#define BT_CURS_DATA    	0x05 
#define BT_COMMAND_REG_0	0x06 
#define BT_CURS_RD_ADDR 	0x07 
#define BT_COMMAND_REG_1	0x08 
#define BT_COMMAND_REG_2	0x09 
#define BT_STATUS_REG   	0x0A
#define BT_COMMAND_REG_3	0x0A
#define BT_CURS_RAM_DATA	0x0B
#define BT_CURS_X_LOW   	0x0C
#define BT_CURS_X_HIGH  	0x0D
#define BT_CURS_Y_LOW   	0x0E
#define BT_CURS_Y_HIGH  	0x0F

/* bt485 Command Register 0 Bits */
#define BT_CR0_POWERDOWN        0x01  /* Powerdown the RAMDAC */
#define BT_CR0_8BIT             0x02  /* 8 bit operation as opposed to 6 bit */
#define BT_CR0_SYNC_R           0x04  /* Sync on Red */
#define BT_CR0_SYNC_G           0x08  /* Sync on Green */
#define BT_CR0_SYNC_B           0x10  /* Sync on Blue */
#define BT_CR0_ENABLE_CR3       0x80

/* bt485 Command Register 1 Bits */
#define BT_CR1_BP24             0x00  /* 24 bits/pixel */
#define BT_CR1_BP16             0x20  /* 16 bits/pixel */
#define BT_CR1_BP8              0x40  /* 8 bits/pixel */
#define BT_CR1_BP4              0x60  /* 4 bits/pixel */
#define BT_CR1_BYPASS_PAL       0x10  /* TrueColor palette bypass enable */
#define BT_CR1_565RGB           0x08  /* Use 5:6:5 for the 16 bit color */
#define BT_CR1_555RGB           0x00  /* Use 5:5:5 for the 16 bit color */
#define BT_CR1_16B_11MUX        0x04  /* Use 1:1 mux for 16 bit color */
#define BT_CR1_16B_21MUX        0x00  /* Use 2:1 mux for 16 bit color */

/* bt485 Command Register 2 Bits */
#define BT_CR2_PORTSEL_NONMASK  0x20
#define BT_CR2_PCLK1            0x10  /* CLKSEL - select PCLK1 */
#define BT_CR2_PCLK0            0x00  /* CLKSEL - select PCLK0 */
#define BT_CR2_INTERLACED       0x08  /* Display mode is interlaced */
#define BT_CR2_CURSOR_DISABLE   0x00  /* Disabled cursor */
#define BT_CR2_CURSOR_ENABLE    0x03  /* Two color X Windows cursor */

/* bt485 Command Register 3 Bits */
#define BT_CR3_2X_CLOCKMUL      0x08  /* 2x Clock Multiplier */
#define BT_CR3_1X_CLOCKMUL      0x00  /* 1x Clock Multiplier */
#define BT_CR3_64SQ_CURSOR      0x04  /* 64x64x2 cursor */
#define BT_CR3_32SQ_CURSOR      0x00  /* 32x32x2 cursor */

void p9x00write_LUT_regs(int start, int n, CARD8 *lut);
void p9x00read_LUT_regs(int start, int n, CARD8 *lut);

void ibm525SetPort(vgap9x00Ptr state, Bool SetPort);
void ibm525SetClock(vgap9x00Ptr state, Bool SetClock);

void ibm525InitCursor(void);
