/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3TiCursor.c,v 1.1.2.1 1998/02/07 10:05:37 hohndel Exp $ */
/*
 * Copyright 1994 by Robin Cutshaw <robin@XFree86.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Robin Cutshaw not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Robin Cutshaw makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ROBIN CUTSHAW DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ROBIN CUTSHAW BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define NEED_EVENTS
#include <X.h>
#include "Xproto.h"
#include <misc.h>
#include <input.h>
#include <cursorstr.h>
#include <regionstr.h>
#include <scrnintstr.h>
#include <servermd.h>
#include <windowstr.h>
#include "xf86.h"
#include "inputstr.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "s3.h"
#include "s3reg.h"
#define S3_SERVER
#include "Ti302X.h"
#include "mipointer.h"


/*
 * TI ViewPoint 3020/3025 support - Robin Cutshaw
 *
 * The Ti3020 has 8 direct command registers and indirect registers
 * 0x00-0x3F and 0xFF.  The low-order two bits of the direct register
 * address follow normal VGA DAC addressing conventions (which 
 * for some reason aren't in numeric order, so we remap them through 
 * an array).  The S3 provides access to the high-order bit via
 * the low-order bit of CR55.  The indirect registers are accessed
 * through the direct index and data registers.  See s3Ti3020.h for
 * details.
 *
 * The Ti3025 is both Ti3020 and Bt485 compatable.  The mode of
 * operation is set via RS4 using S3 register 0x5C and the 3020
 * cursor control register.  The 3025 also has a built in PLL
 * clock generator.
 */

static Bool s3In3020mode = FALSE;             /* starts in Bt485 mode */

void s3set3020mode()
{
   unsigned char tmp, tmp1, tmp2;

   outb(vgaCRIndex, 0x5C);
   tmp = inb(vgaCRReg);
   outb(vgaCRReg, tmp & 0xDF);           /* clear RS4 - use 3020 mode */

   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg) & 0xFC;
   outb(vgaCRReg, tmp | 0x01);  /* toggle to upper 4 direct registers */
   tmp1 = inb(TI_INDEX_REG);
   outb(TI_INDEX_REG, TI_CURS_CONTROL);
   tmp2 = inb(TI_DATA_REG);
   outb(TI_DATA_REG, tmp2 & 0x7F);      /* clear TI_PLANAR_ACCESS bit */

   outb(TI_INDEX_REG, tmp1);
   outb(vgaCRIndex, 0x55);
   outb(vgaCRReg, tmp);
   s3In3020mode = TRUE;
}

void s3set485mode()
{
   unsigned char tmp, tmp1;

   outb(vgaCRIndex, 0x5C);
   tmp = inb(vgaCRReg);
   outb(vgaCRReg, tmp & 0xDF);           /* clear RS4 - use 3020 mode */

   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg) & 0xFC;
   outb(vgaCRReg, tmp | 0x01);  /* toggle to upper 4 direct registers */
   outb(TI_INDEX_REG, TI_CURS_CONTROL);
   tmp1 = inb(TI_DATA_REG);
   outb(TI_DATA_REG, tmp1 | 0x80);        /* set TI_PLANAR_ACCESS bit */

   outb(vgaCRIndex, 0x5C);
   outb(vgaCRReg, tmp | 0x20);              /* set RS4 - use 485 mode */

   outb(vgaCRIndex, 0x55);
   outb(vgaCRReg, tmp);
   s3In3020mode = FALSE;
}

/*
 * s3OutTiIndReg() and s3InTiIndReg() are used to access the indirect
 * 3020 registers only.
 */

#ifdef __STDC__
void s3OutTiIndReg(unsigned char reg, unsigned char mask, unsigned char data)
#else
void s3OutTiIndReg(reg, mask, data)
unsigned char reg;
unsigned char mask;
unsigned char data;
#endif
{
   unsigned char tmp, tmp1, tmp2 = 0x00;

   /* High 2 bits of reg in CR55 bits 0-1 (1 is cleared for the TI ramdac) */
   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg) & 0xFC;
   outb(vgaCRReg, tmp | 0x01);  /* toggle to upper 4 direct registers */
   tmp1 = inb(TI_INDEX_REG);
   outb(TI_INDEX_REG, reg);

   /* Have to map the low two bits to the correct DAC register */
   if (mask != 0x00)
      tmp2 = inb(TI_DATA_REG) & mask;
   outb(TI_DATA_REG, tmp2 | data);
   
   /* Now clear 2 high-order bits so that other things work */
   outb(TI_INDEX_REG, tmp1);  /* just in case anyone relies on this */
   outb(vgaCRReg, tmp);
}

#ifdef __STDC__
unsigned char s3InTiIndReg(unsigned char reg)
#else
unsigned char s3InTiIndReg(reg)
unsigned char reg;
#endif
{
   unsigned char tmp, tmp1, ret;

   /* High 2 bits of reg in CR55 bits 0-1 (1 is cleared for the TI ramdac) */
   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg) & 0xFC;
   outb(vgaCRReg, tmp | 0x01);  /* toggle to upper 4 direct registers */
   tmp1 = inb(TI_INDEX_REG);
   outb(TI_INDEX_REG, reg);

   /* Have to map the low two bits to the correct DAC register */
   ret = inb(TI_DATA_REG);

   /* Now clear 2 high-order bits so that other things work */
   outb(TI_INDEX_REG, tmp1);  /* just in case anyone relies on this */
   outb(vgaCRReg, tmp);

   return(ret);
}

/*
 * Convert the cursor from server-format to hardware-format.  The Ti3020
 * has two planes, plane 0 selects cursor color 0 or 1 and plane 1
 * selects transparent or display cursor.  The bits of these planes
 * are packed together so that one byte has 4 pixels. The organization
 * looks like:
 *             Byte 0x000 - 0x00F    top scan line, left to right
 *                  0x010 - 0x01F
 *                    .       .
 *                  0x3F0 - 0x3FF    bottom scan line
 *
 *             Byte/bit map - D7D6,D5D4,D3D2,D1D0  four pixels, two planes each
 *             Pixel/bit map - P1P0  (plane 1) == 1 maps to cursor color
 *                                   (plane 1) == 0 maps to transparent
 *                                   (plane 0) maps to cursor colors 0 and 1
 */


void 
s3TiShowCursor()
{
   unsigned char tmp;

   UNLOCK_SYS_REGS;

   /* turn on external cursor */
   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg) & 0xDF;
   outb(vgaCRReg, tmp | 0x20);

   /* Enable Ti3020 */
   outb(vgaCRIndex, 0x45);
   tmp = inb(vgaCRReg) & 0xDF;
   outb(vgaCRReg, tmp | 0x20);
   
   /* Enable cursor - sprite enable, X11 mode */
   s3OutTiIndReg(TI_CURS_CONTROL,
		 (unsigned char )~TI_CURS_CTRL_MASK,
	         TI_CURS_SPRITE_ENABLE | TI_CURS_X_WINDOW_MODE);

   LOCK_SYS_REGS;
}

void
s3TiHideCursor()
{
   UNLOCK_SYS_REGS;

   /*
    * Don't need to undo the S3 registers here; they will be undone when
    * the mode is restored from save registers.  If it is done here, it
    * causes the cursor to flash each time it is loaded, so don't do that.
    */

   /* Disable cursor */
   s3OutTiIndReg(TI_CURS_CONTROL,
		 (unsigned char )~TI_CURS_CTRL_MASK, 0x00);

   LOCK_SYS_REGS;
}

void
s3TiSetCursorPosition(x, y, xorigin, yorigin)
     int   x, y, xorigin, yorigin;
{   
   UNLOCK_SYS_REGS;

   outb(TI_INDEX_REG, TI_SPRITE_ORIGIN_X); /* offset into cursor data */
   outb(TI_DATA_REG, xorigin);
   outb(TI_INDEX_REG, TI_SPRITE_ORIGIN_Y);
   outb(TI_DATA_REG, yorigin);

   s3OutTiIndReg(TI_CURS_X_LOW, 0x00, x & 0xFF);
   s3OutTiIndReg(TI_CURS_X_HIGH, 0x00, (x >> 8) & 0x0F);
   s3OutTiIndReg(TI_CURS_Y_LOW, 0x00, y & 0xFF);
   s3OutTiIndReg(TI_CURS_Y_HIGH, 0x00, (y >> 8) & 0x0F);

   LOCK_SYS_REGS;
}

void
s3TiSetCursorColors(bg, fg)
   int bg, fg;
{
   UNLOCK_SYS_REGS;

   /* The TI 3020 cursor is always 8 bits so shift 8, not 10 */

   /* Background color */
   s3OutTiIndReg(TI_CURSOR_COLOR_0_RED,   0, (bg & 0x00FF0000) >> 16);
   s3OutTiIndReg(TI_CURSOR_COLOR_0_GREEN, 0, (bg & 0x0000FF00) >> 8);
   s3OutTiIndReg(TI_CURSOR_COLOR_0_BLUE,  0, (bg & 0x000000FF));

   /* Foreground color */
   s3OutTiIndReg(TI_CURSOR_COLOR_1_RED,   0, (fg & 0x00FF0000) >> 16);
   s3OutTiIndReg(TI_CURSOR_COLOR_1_GREEN, 0, (fg & 0x0000FF00) >> 8);
   s3OutTiIndReg(TI_CURSOR_COLOR_1_BLUE,  0, (fg & 0x000000FF));

   LOCK_SYS_REGS;
}

void 
s3TiLoadCursorImage(bits, xorigin, yorigin)
   unsigned char *bits;
   int xorigin, yorigin;
{
   register int   i;
   unsigned char tmp, tmp1;
   register unsigned char *mask = bits + 1;

   UNLOCK_SYS_REGS;

   /* The hardware cursor is not supported in interlaced mode */

   /* High 2 bits of reg in CR55 bits 0-1 (1 is cleared for the TI ramdac) */
   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg) & 0xFC;
   outb(vgaCRReg, tmp | 0x01); /* toggle to the high four direct registers */
   tmp1 = inb(TI_INDEX_REG);

   outb(TI_INDEX_REG, TI_CURS_RAM_ADDR_LOW); /* must be first */
   outb(TI_DATA_REG, 0x00);
   outb(TI_INDEX_REG, TI_CURS_RAM_ADDR_HIGH);
   outb(TI_DATA_REG, 0x00);
   outb(TI_INDEX_REG, TI_CURS_RAM_DATA);

   for (i = 0; i < 512; i++, mask+=2)
      outb(TI_DATA_REG, *mask);
   for (i = 0; i < 512; i++, bits+=2)
      outb(TI_DATA_REG, *bits);

   outb(TI_INDEX_REG, tmp1);

   outb(vgaCRIndex, 0x55);
   outb(vgaCRReg, tmp);

   LOCK_SYS_REGS;
}
