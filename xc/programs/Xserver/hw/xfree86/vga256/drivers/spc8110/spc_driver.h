/*
 * Copyright 1997,1998 by Thomas Mueller
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Mueller not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Mueller makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS MUELLER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS MUELLER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/spc8110/spc_driver.h,v 1.1.2.1 1998/10/18 20:42:41 hohndel Exp $ */


/*
 * EPSON SPC8110 vga256 driver
 */

/* device capabilities */

typedef struct {
    int LCD;		/* panel or simultaneous display	*/
    int CRT;		/* CRT only				*/
    int PCI;		/* PCI (1) or VLB/486LB (0)		*/
    int MClk;		/* MClk value in KHz			*/
    int pll2Num;	/* MClk numerator for PLL2		*/
    int pll2Den;	/* MClk denominator for PLL2		*/
    int PWidth;		/* Panel size				*/
    int PHeight;
#define TEST_SAMPLE	1
    int Quirk;		/* various flags			*/
    unsigned long BASE;	/* linear base address			*/
    Bool HWCursor;	/* are we using HW Cursor ?		*/
    unsigned char FIFO;	/* AUX[0x3c]				*/
} SPC8110cap;

#define spcQuirk	spc8110.Quirk
#define spcLCD		spc8110.LCD
#define spcCRT		spc8110.CRT
#define spcPCI		spc8110.PCI
#define spcMClk		spc8110.MClk
#define spcBASE		spc8110.BASE
#define spcPWidth	spc8110.PWidth
#define spcPHeight	spc8110.PHeight
#define spcHWCursor	spc8110.HWCursor
#define spcFIFO		spc8110.FIFO
#define spcPll2Num	spc8110.pll2Num
#define spcPll2Den	spc8110.pll2Den

extern SPC8110cap	spc8110;

#define CMD_MOVE	0x03
#define DEST_LINEAR	0x60
#define SRC_LINEAR	0x40
#define DIR_NEGATIVE	0x80

#define DOWN_RIGHT	((CMD_MOVE) << 8)
#define UP_LEFT		((CMD_MOVE | DIR_NEGATIVE) << 8)

#ifndef PCI_VENDOR_EPSON
#define PCI_VENDOR_EPSON	0x10F4
#endif
#ifndef PCI_CHIP_SPC8110
#define PCI_CHIP_SPC8110	0x8110
#endif

