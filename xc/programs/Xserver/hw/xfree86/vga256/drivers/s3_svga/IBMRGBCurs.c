/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/IBMRGBCurs.c,v 1.1.2.1 1998/02/09 13:57:42 robin Exp $ */
/*
 *
 * Copyright 1995 The XFree86 Project, Inc.
 *
 */

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
#include "mipointer.h"
#define S3_SERVER
#include "IBMRGB.h"

#ifndef __GNUC__
# define __inline__ /**/
#endif

/*
 * Convert the cursor from server-format to hardware-format.  
 * The IBM RGB52x has one array of two-bit-pixels. The MSB contains
 * the transparency information (mask) and the LSB is the color bit.
 * The pixel order within a byte can be selected and is MSB-to-LSB
 * or "00112233".
 */

void 
s3IBMRGBShowCursor()
{
   unsigned char tmp;

   UNLOCK_SYS_REGS;

   /* turn on external cursor */
   outb(vgaCRIndex, 0x55);
   tmp = (inb(vgaCRReg) & 0xDF) | 0x20;
   outb(vgaCRReg, tmp);

   /* Enable IBMRGB */
   outb(vgaCRIndex, 0x45);
   tmp = inb(vgaCRReg) & ~0x20;
   outb(vgaCRReg, tmp);
   
   /* Enable cursor - X11 mode */
   s3OutIBMRGBIndReg(IBMRGB_curs, 0, 0x27);

   LOCK_SYS_REGS;
}

void
s3IBMRGBHideCursor()
{
   UNLOCK_SYS_REGS;

   /*
    * Don't need to undo the S3 registers here; they will be undone when
    * the mode is restored from save registers.  If it is done here, it
    * causes the cursor to flash each time it is loaded, so don't do that.
    */

   /* Disable cursor */
   s3OutIBMRGBIndReg(IBMRGB_curs, ~3, 0x00);

   LOCK_SYS_REGS;
}

void
s3IBMRGBSetCursorPosition(x, y, xorigin, yorigin)
     int   x, y, xorigin, yorigin;
{
   unsigned char tmp;
   
   x -= xorigin;
   y -= yorigin;

   if (vga256InfoRec.modes->Flags & V_DBLSCAN)
      y <<= 1;

   if (vga256InfoRec.modes->Flags & V_INTERLACE)
      y >>= 1;

   UNLOCK_SYS_REGS;

   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg) & 0xFC;
   outb(vgaCRReg, tmp | 0x01);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_xl);     outb(IBMRGB_INDEX_DATA, x);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_xh);     outb(IBMRGB_INDEX_DATA, x>>8);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_yl);     outb(IBMRGB_INDEX_DATA, y);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_yh);     outb(IBMRGB_INDEX_DATA, y>>8);
   outb(vgaCRReg, tmp);

   LOCK_SYS_REGS;
}

void
s3IBMRGBSetCursorColors(bg, fg)
   int bg, fg;
{
   unsigned char tmp;
   UNLOCK_SYS_REGS;

   /* The IBM RGB52x cursor is always 8 bits so shift 8, not 10 */

   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg) & 0xFC;
   outb(vgaCRReg, tmp | 0x01);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_col1_r); 
   outb(IBMRGB_INDEX_DATA, (bg & 0x00FF0000) >> 16);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_col1_g); 
   outb(IBMRGB_INDEX_DATA, (bg & 0x0000FF00) >> 8);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_col1_b); 
   outb(IBMRGB_INDEX_DATA, (bg & 0x000000FF));
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_col2_r); 
   outb(IBMRGB_INDEX_DATA, (fg & 0x00FF0000) >> 16);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_col2_g); 
   outb(IBMRGB_INDEX_DATA, (fg & 0x0000FF00) >> 8);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_col2_b); 
   outb(IBMRGB_INDEX_DATA, (fg & 0x000000FF));
   outb(vgaCRReg, tmp);

   LOCK_SYS_REGS;
}

void 
s3IBMRGBLoadCursorImage(bits, xorigin, yorigin)
   unsigned char *bits;
   int xorigin, yorigin;
{
   unsigned char tmp, tmp2;
   register int i;

   UNLOCK_SYS_REGS;

   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg) & 0xFC;
   outb(vgaCRReg, tmp | 0x01);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_hot_x);  outb(IBMRGB_INDEX_DATA, 0);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_hot_y);  outb(IBMRGB_INDEX_DATA, 0);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_xl);     outb(IBMRGB_INDEX_DATA, 0xff);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_xh);     outb(IBMRGB_INDEX_DATA, 0x7f);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_yl);     outb(IBMRGB_INDEX_DATA, 0xff);
   outb(IBMRGB_INDEX_LOW,IBMRGB_curs_yh);     outb(IBMRGB_INDEX_DATA, 0x7f);

   tmp2 = inb(IBMRGB_INDEX_CONTROL) & 0xfe;
   outb(IBMRGB_INDEX_CONTROL, tmp2 | 1);  /* enable auto-increment */
   
   outb(IBMRGB_INDEX_HIGH, (unsigned char) (IBMRGB_curs_array >> 8));
   outb(IBMRGB_INDEX_LOW,  (unsigned char) IBMRGB_curs_array);

   for (i = 0; i < 1024; i++)
      outb(IBMRGB_INDEX_DATA, *bits++);

   outb(IBMRGB_INDEX_HIGH, 0);
   outb(IBMRGB_INDEX_CONTROL, tmp2);  /* disable auto-increment */
   outb(vgaCRIndex, 0x55);
   outb(vgaCRReg, tmp);

   LOCK_SYS_REGS;

}


