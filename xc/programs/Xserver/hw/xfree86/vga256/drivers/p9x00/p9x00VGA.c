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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00VGA.c,v 1.1.2.3 1998/10/31 14:41:02 hohndel Exp $ */

#include "X.h"
#include "input.h"

#include "xf86.h"
#include "xf86_OSlib.h"

#include "misc.h"
#include "vga.h"
#include "vgaBank.h"

#include "p9x00VGARegs.h"
#include "p9x00Access.h"

/* Virtual Terminal / VGA values to be saved and restored */
static short vga_dac_mask = 0xff;

pointer w5x86Base;
int vgaIOBase;
static unsigned char saved_misc;
static unsigned w5x86_IOPorts[]=
   {
   SEQ_INDEX_REG, SEQ_PORT, MISC_OUT_REG, MISC_IN_REG,
   GRA_I, GRA_D, CRT_IC, CRT_DC, IS1_RC, ATT_IW, ATT_R,
   VGA_BANK_REG
   };
static unsigned Num_w5x86_IOPorts=sizeof(w5x86_IOPorts)/sizeof(w5x86_IOPorts[0]);   
extern ScrnInfoRec vga256InfoRec;
static void w5x86SlowCopy(
#if NeedFunctionPrototypes
   unsigned char *, unsigned char *, unsigned
#endif
);

static void w5x86VGADelay(
#if NeedFunctionPrototypes
   void
#endif
);

/*
 * w5x86VGASlowCopy --
 *     Copiess memory from one location to another
 */
void w5x86VGASlowCopy(unsigned char *dest, unsigned char *src, unsigned bytes)
{
  while (bytes-- > 0)
    {
      *(dest++) = *(src++);
    }
}

/*
 * w5x86VGADelay --
 *     Pauses for a very short time.
 */
void w5x86VGADelay()
{
  int ctr;
  for (ctr = 0; ctr <= 10; ctr++)
    outb(0x80, 0);  /* This is the POST register.  The length of this
		     * delay is dependant only on ISA bus speed */
}

/*
 * this is used to put the vga chip into a 16 color graphics mode, which
 * makes the font saving and restoration MUCH easier
 */
/* BIOS mode 10h - 640x350x16 */
static const unsigned char g640x350x16_regs[60] = {
    0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x40,0x00,0x00, 
    0x00,0x00,0x00,0x00,0x83,0x85,0x5D,0x28,0x0F,0x63,0xBA,0xE3, 
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 
    0x0C,0x0D,0x0E,0x0F,0x01,0x00,0x0F,0x00,0x00, 
    0x00,0x0F,0x00,0x20,0x00,0x00,0x05,0x0F,0xFF, 
    0x03,0x01,0x0F,0x00,0x06, 
    0xA3
};


static unsigned char text_regs[200] ;  /* holds the textmode register values */
static unsigned char font_buf[FONT_SIZE] ; /* hold the first font buffer     */
#ifdef Lynx
static unsigned char font_buf2[FONT_SIZE]; /* hold the second font buffer     */
static unsigned char text_buf[2*TEXT_SIZE];  /* hold the text buffer */
#endif

int CRT_I = CRT_IC ;	/* bases for color */
int CRT_D = CRT_DC ;
int IS1_R = IS1_RC ;

int w5x86VGASaveRegs(unsigned char *regs)
{
  int i;

  /* save VGA registers */
  for (i = 0; i < CRT_C; i++) {
       outb(CRT_I, i); 
       regs[CRT+i] = inb(CRT_D); 
  }
  for (i = 0; i < ATT_C; i++) {
       inb(IS1_R);
       w5x86VGADelay();
       outb(ATT_IW, i); 
       w5x86VGADelay();
       regs[ATT_+i] = inb(ATT_R); 
       w5x86VGADelay();
  }
  for (i = 0; i < GRA_C; i++) {
       outb(GRA_I, i); 
       regs[GRA+i] = inb(GRA_D); 
  }
  for (i = 0; i < SEQ_C; i++) {
       outb(SEQ_INDEX_REG, i); 
       regs[SEQ+i] = inb(SEQ_PORT); 
  }
  /* This save and restore is used for switching into
   * 640x400x16 and does not conflict with the one
   * done in the vendor specific code */
  regs[MIS] = inb(MISC_IN_REG); 
#ifdef DEBUG
  ErrorF("w5x86VGASaveRegs: debug info: regs[MIS]=inb(MISC_IN_REG)=0x%X\n" ,
	 (int)regs[MIS]);
#endif
  return CRT_C + ATT_C + GRA_C + SEQ_C + 1 + i;
}

int w5x86VGASetRegs(const unsigned char *regs)
{
  int i;
  int tmp;

  /* update misc output register */
  usleep(20000) ;
  /* This save and restore is used for switching into
   * 640x400x16 and does not conflict with the one
   * done in the vendor specific code */
  outb(MISC_OUT_REG, regs[MIS]);         
#ifdef DEBUG
  ErrorF("w5x86VGASetRegs: debug info: outb(MISC_OUT_REG, regs[MIS]) regs[MIS]=0x%X\n",
	 (int)regs[MIS]);
#endif

  usleep(30000);  /* Wait at least 10 ms for the clock to change */

  /* synchronous reset on */
  outb(SEQ_INDEX_REG, 0x00); 
  outb(SEQ_PORT, 0x01);               
 
  /* write sequencer registers */
  outb(SEQ_INDEX_REG, 1);
  outb(SEQ_PORT, regs[SEQ+1]|0x20);
  for (i = 2; i < SEQ_C; i++) {       
      outb(SEQ_INDEX_REG, i); 
      outb(SEQ_PORT, regs[SEQ+i]); 
  }

  /* synchronous reset off */
  outb(SEQ_INDEX_REG, 0x00); 
  outb(SEQ_PORT, 0x03);              

  outb(CRT_I, 0x11);            
  tmp = inb(CRT_D)&0x7F;
  outb(CRT_D, tmp);   

  /* write CRT registers */
  for (i = 0; i < CRT_C; i++) {       
      outb(CRT_I, i); 
      outb(CRT_D, regs[CRT+i]); 
  }

  /* write graphics controller registers */
  for (i = 0; i < GRA_C; i++) {       
      outb(GRA_I, i); 
      outb(GRA_D, regs[GRA+i]); 
  }
   
  /* write attribute controller registers */
  for (i = 0; i < ATT_C; i++) {       
      inb(IS1_R);   /* reset flip-flop */
      w5x86VGADelay();
      outb(ATT_IW, i);
      w5x86VGADelay();
      outb(ATT_IW, regs[ATT_+i]);
      w5x86VGADelay();
  }

  return 0;
}

/* 
 * w5x86Save --
 *    Restores the LUT and other parts of the VT's state for a virtual terminal
 */
void w5x86Save()
{
  w5x86Base = xf86MapVidMem(vga256InfoRec.scrnIndex,VGA_REGION,(pointer)0xA0000,0x10000);
  P9X00_ENABLEPORTS;
  /* Save old values */
  /*w5x86ReadLUT(vga_lut);*/
  /* vga_dac_mask = inb(BT_PIXEL_MASK);
     should be done from main EnterLeaveVT,
     because of different DACs */
  w5x86VGASaveRegs(text_regs);
  w5x86VGASetRegs(g640x350x16_regs);
#ifndef Lynx
  outb(GRA_I, 0x04);
  outb(GRA_D, 0x02);
  w5x86VGASlowCopy(font_buf, w5x86Base, FONT_SIZE);
#else
  /* see vgaHW.c: SAVE_FONT1, SAVE_FONT2, SAVE_TEXT */
  outw(0x3C4, 0x0402);    /* write to plane 2 */
  outw(0x3C4, 0x0604);    /* enable plane graphics */
  outw(0x3CE, 0x0204);    /* read plane 2 */
  outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
  outw(0x3CE, 0x0506);    /* set graphics */
  w5x86VGASlowCopy(font_buf, w5x86Base, FONT_SIZE);

  outw(0x3C4, 0x0802);    /* write to plane 3 */
  outw(0x3C4, 0x0604);    /* enable plane graphics */
  outw(0x3CE, 0x0304);    /* read plane 3 */
  outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
  outw(0x3CE, 0x0506);    /* set graphics */
  w5x86VGASlowCopy(font_buf2, w5x86Base, FONT_SIZE);

  outw(0x3C4, 0x0102);    /* write to plane 0 */
  outw(0x3C4, 0x0604);    /* enable plane graphics */
  outw(0x3CE, 0x0004);    /* read plane 0 */
  outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
  outw(0x3CE, 0x0506);    /* set graphics */
  w5x86VGASlowCopy(text_buf, w5x86Base, TEXT_SIZE);
  outw(0x3C4, 0x0202);    /* write to plane 1 */
  outw(0x3C4, 0x0604);    /* enable plane graphics */
  outw(0x3CE, 0x0104);    /* read plane 1 */
  outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
  outw(0x3CE, 0x0506);    /* set graphics */
  w5x86VGASlowCopy(text_buf + TEXT_SIZE, w5x86Base, TEXT_SIZE);
#endif
  w5x86VGASetRegs(text_regs);
  P9X00_DISABLEPORTS;
} 

/* 
 * w5x86Restore --
 *    Restores the LUT and other things for a virtual terminal
 */
void w5x86Restore()
{
  int tmp;
  
  P9X00_ENABLEPORTS;
  w5x86VGASetRegs(g640x350x16_regs); /* put us in a 16 color graphics mode */

  /* disable Set/Reset register */
  outb(GRA_I, 0x01);
  outb(GRA_D, 0x00);
  
#ifndef Lynx
  /* restore font data from plane 2 */
  outb(GRA_I, 0x02);
  outb(GRA_D, 0x04);
  w5x86VGASlowCopy(w5x86Base, font_buf, FONT_SIZE);
#else
  outw(0x3C4, 0x0402);    /* write to plane 2 */
  outw(0x3C4, 0x0604);    /* enable plane graphics */
  outw(0x3CE, 0x0204);    /* read plane 2 */
  outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
  outw(0x3CE, 0x0506);    /* set graphics */
  w5x86VGASlowCopy(w5x86Base, font_buf, FONT_SIZE);

  outw(0x3C4, 0x0802);    /* write to plane 3 */
  outw(0x3C4, 0x0604);    /* enable plane graphics */
  outw(0x3CE, 0x0304);    /* read plane 3 */
  outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
  outw(0x3CE, 0x0506);    /* set graphics */
  w5x86VGASlowCopy(w5x86Base, font_buf2, FONT_SIZE);

  outw(0x3C4, 0x0102);    /* write to plane 0 */
  outw(0x3C4, 0x0604);    /* enable plane graphics */
  outw(0x3CE, 0x0004);    /* read plane 0 */
  outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
  outw(0x3CE, 0x0506);    /* set graphics */
  w5x86VGASlowCopy(w5x86Base, text_buf, TEXT_SIZE);
  outw(0x3C4, 0x0202);    /* write to plane 1 */
  outw(0x3C4, 0x0604);    /* enable plane graphics */
  outw(0x3CE, 0x0104);    /* read plane 1 */
  outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
  outw(0x3CE, 0x0506);    /* set graphics */
  w5x86VGASlowCopy(w5x86Base, text_buf + TEXT_SIZE, TEXT_SIZE);
#endif

  w5x86VGASetRegs(text_regs);	/* assumes they have already been saved */
  outb(SEQ_INDEX_REG, 0x01);			/* turn screen on */
  tmp = inb(SEQ_PORT)&0xDF;
  outb(SEQ_PORT, tmp);
  outb(ATT_IW, 0x20); 				/* enable display ????? */

  /* Restore old values */
  /*w5x86WriteLUT(vga_lut);*/
  /* outb(BT_PIXEL_MASK,vga_dac_mask);
     Should be done from main EnterLeave,
     because of differnt DACs */
 P9X00_DISABLEPORTS;
 xf86UnMapVidMem(vga256InfoRec.scrnIndex,VGA_REGION,w5x86Base,0x10000);
}

void w5x86Lock(void)
   {
   P9X00_ENABLEPORTS;
   outb(SEQ_INDEX_REG,SEQ_MISC_INDEX);
   outb(SEQ_PORT,saved_misc);
   P9X00_DISABLEPORTS;
   }
   
void w5x86Unlock(void)
   {
   unsigned char tmp;

   P9X00_ENABLEPORTS;
   outb(SEQ_INDEX_REG,SEQ_MISC_INDEX);
   saved_misc=inb(SEQ_PORT);
   outb(SEQ_PORT,saved_misc);
   outb(SEQ_PORT,saved_misc);
   tmp = inb(SEQ_PORT);
   outb(SEQ_PORT, tmp & ~0x20);
   P9X00_DISABLEPORTS;
   }
