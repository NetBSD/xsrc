/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/vgaHW.c,v 1.3.2.1 1998/07/30 06:23:46 hohndel Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 */

#ifdef ISC202
#include <sys/types.h>
#define WIFEXITED(a)  ((a & 0x00ff) == 0)  /* LSB will be 0 */
#define WEXITSTATUS(a) ((a & 0xff00) >> 8)
#define WIFSIGNALED(a) ((a & 0xff00) == 0) /* MSB will be 0 */
#define WTERMSIG(a) (a & 0x00ff)
#else
#if defined(ISC) && !defined(_POSIX_SOURCE)
#define _POSIX_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#undef _POSIX_SOURCE
#else
#if defined(MINIX) || defined(AMOEBA) || (defined(ISC) && defined(_POSIX_SOURCE)) || defined(Lynx)
#include <sys/types.h>
#endif
#include <sys/wait.h>
#endif
#endif

#if !defined(AMOEBA) && !defined(MINIX)
#define _NEED_SYSI86
#endif

#include "X.h"
#include "misc.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_HWlib.h"
#include "vga.h"
#ifdef PC98_EGC
/* I/O port address define for extended EGC */
#define		EGC_READ	0x4a2	/* EGC FGC,EGC,Read Plane  */
#define		EGC_MASK	0x4a8	/* EGC Mask register       */
#define		EGC_ADD		0x4ac	/* EGC Dest/Source address */
#define		EGC_LENGTH	0x4ae	/* EGC Bit length          */
#endif

#if !defined(PC98_PEGC) && !defined(PC98_EGC) && !defined(PC98_MGA)
#if !defined(MONOVGA) && !defined(SCO)
#ifndef SAVE_FONT1
#define SAVE_FONT1
#endif
#endif

#if defined(Lynx) || defined(CSRG_BASED) || defined(MACH386) || defined(linux) || defined(AMOEBA) || defined(MINIX)
#ifndef NEED_SAVED_CMAP
#define NEED_SAVED_CMAP
#endif
#ifndef MONOVGA
#ifndef SAVE_TEXT
#define SAVE_TEXT
#endif
#endif
#ifndef SAVE_FONT2
#define SAVE_FONT2
#endif
#endif

/* bytes per plane to save for text */
#if defined(Lynx) || defined(linux) || defined(MINIX)
#define TEXT_AMOUNT 16384
#else
#define TEXT_AMOUNT 4096
#endif

/* bytes per plane to save for font data */
#define FONT_AMOUNT 16384
#endif /* !defined(PC98_PEGC) && !defined(PC98_EGC) && !defined(PC98_MGA) */

#if defined(CSRG_BASED) || defined(MACH386)
#include <sys/time.h>
#endif

#ifdef MACH386
#define WEXITSTATUS(x) (x.w_retcode)
#define WTERMSIG(x) (x.w_termsig)
#define WSTOPSIG(x) (x.w_stopsig)
#endif

/* This the only where the definition seems to work (out of
 * vga.c/vgaHW.c/vgaCmap.c).
 */
Bool clgd6225Lcd= FALSE;

/* DAC indices for white and black */
#define WHITE_VALUE 0x3F
#define BLACK_VALUE 0x00
#define OVERSCAN_VALUE 0x01

Bool (*vgaBlankScreenFunc)()=vgaBlankScreen;

int vgaRamdacMask = 0x3F;

#define new ((vgaHWPtr)vgaNewVideoState)

unsigned VGA_IOPorts[] = {
#ifdef PC98_PEGC
	0x60,  0x62,  0x6a,  0x7c,  0x7e,  0x80,  0xa0,  0xa2,  0x3c0, 0x3c2,
	0x3c4, 0x3c6, 0x3c7, 0x3c8, 0x3c9, 0x3cc, 0x3ce, 0x3d0, 0x9a0, 0x9a8,
#else
#ifdef PC98_EGC
/* I/O port address define Normal & Hireso mode */
	0xa8,  0xaa,  0xac,  0xae,  0x4a0, 0x4a1, 0x4a2, 0x4a3, 0x4a4, 0x4a5,
	0x4a6, 0x4a7, 0x4a8, 0x4a9, 0x4aa, 0x4ab, 0x4ac, 0x4ad, 0x4ae, 0x4af,
#else
	0x3B4, 0x3B5, 0x3BA, 0x3C0, 0x3C1, 0x3C2, 0x3C4, 0x3C5, 0x3C6, 0x3C7, 
	0x3C8, 0x3C9, 0x3CA, 0x3CB, 0x3CC, 0x3CE, 0x3CF, 0x3D4, 0x3D5, 0x3DA,
#endif
#endif
};
int Num_VGA_IOPorts = (sizeof(VGA_IOPorts)/sizeof(VGA_IOPorts[0]));

/*
 * With Intel, the version in common_hw/SlowBcopy.s is used.
 * This avoids port I/O during the copy (which causes problems with
 * some hardware).
 */
#ifdef __alpha__
#define slowbcopy_tobus(src,dst,count) SlowBCopyToBus(src,dst,count)
#define slowbcopy_frombus(src,dst,count) SlowBCopyFromBus(src,dst,count)
#else /* __alpha__ */
#define slowbcopy_tobus(src,dst,count) SlowBcopy(src,dst,count)
#define slowbcopy_frombus(src,dst,count) SlowBcopy(src,dst,count)
#endif /* __alpha__ */

/*
 * vgaBlankScreen -- blank the screen.
 */

Bool vgaBlankScreen(ScreenPtr ptr, Bool on)
{

#if !defined(PC98_EGC) && !defined(PC98_PEGC)
  unsigned char scrn;

  outb(0x3C4,1);
  scrn = inb(0x3C5);

  if(on) {
    scrn &= 0xDF;			/* enable screen */
  }else {
    scrn |= 0x20;			/* blank screen */
  }

  (*vgaSaveScreenFunc)(SS_START);
  outw(0x3C4, (scrn << 8) | 0x01); /* change mode */
  (*vgaSaveScreenFunc)(SS_FINISH);
#else
  if(on) 
    outb(0xa2, 0xd);
  else
    outb(0xa2, 0xc);
#endif    
  return TRUE;
}

/*
 * vgaSaveScreen -- blank the screen.
 */

Bool
vgaSaveScreen(pScreen, on)
     ScreenPtr pScreen;
     Bool  on;
{
   if (on)
      SetTimeSinceLastInputEvent();

   if (xf86VTSema) {
     (*vgaBlankScreenFunc)(pScreen,on);
   }
   return (TRUE);
}

/*
 * vgaHWSaveScreen
 *      perform a sequencer reset.
 */

void
vgaHWSaveScreen(start)
    Bool start;
{
#if !defined(PC98_PEGC) && !defined(PC98_EGC)
  if (start == SS_START)
    outw(0x3C4, 0x0100);        /* synchronous reset */
  else
    outw(0x3C4, 0x0300);        /* end reset */
#endif
}

/*
 * vgaHWRestore --
 *      restore a video mode
 */

void
vgaHWRestore(restore)
     vgaHWPtr restore;
{
  int i,tmp;

#if !defined(PC98_PEGC) && !defined(PC98_EGC)
  tmp = inb(vgaIOBase + 0x0A);		/* Reset flip-flop */
  outb(0x3C0, 0x00);			/* Enables pallete access */
#endif

  /*
   * This here is a workaround a bug in the kd-driver. We MUST explicitely
   * restore the font we got, when we entered graphics mode.
   * The bug was seen on ESIX, and ISC 2.0.2 when using a monochrome
   * monitor. 
   *
   * BTW, also GIO_FONT seems to have a bug, so we cannot use it, to get
   * a font.
   */
  
  vgaSaveScreen(NULL, FALSE);
  if (xf86bpp > 1) {
#if defined(SAVE_TEXT) || defined(SAVE_FONT1) || defined(SAVE_FONT2)
  if(restore->FontInfo1 || restore->FontInfo2 || restore->TextInfo) {
    /*
     * here we switch temporary to 16 color-plane-mode, to simply
     * copy the font-info and saved text
     *
     * BUGALLERT: The vga's segment-select register MUST be set appropriate !
     */
    tmp = inb(vgaIOBase + 0x0A); /* reset flip-flop */
    outb(0x3C0,0x30); outb(0x3C0, 0x01); /* graphics mode */
    if (xf86bpp == 4) {
      outw(0x3CE,0x0003); /* GJA - don't rotate, write unmodified */
      outw(0x3CE,0xFF08); /* GJA - write all bits in a byte */
      outw(0x3CE,0x0001); /* GJA - all planes come from CPU */
    }
#ifdef SAVE_FONT1
    if (restore->FontInfo1) {
      outw(0x3C4, 0x0402);    /* write to plane 2 */
      outw(0x3C4, 0x0604);    /* enable plane graphics */
      outw(0x3CE, 0x0204);    /* read plane 2 */
      outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
      outw(0x3CE, 0x0506);    /* set graphics */
      slowbcopy_tobus(restore->FontInfo1, vgaBase, FONT_AMOUNT);
    }
#endif
#ifdef SAVE_FONT2
    if (restore->FontInfo2) {
      outw(0x3C4, 0x0802);    /* write to plane 3 */
      outw(0x3C4, 0x0604);    /* enable plane graphics */
      outw(0x3CE, 0x0304);    /* read plane 3 */
      outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
      outw(0x3CE, 0x0506);    /* set graphics */
      slowbcopy_tobus(restore->FontInfo2, vgaBase, FONT_AMOUNT);
    }
#endif
#ifdef SAVE_TEXT
    if (restore->TextInfo) {
      outw(0x3C4, 0x0102);    /* write to plane 0 */
      outw(0x3C4, 0x0604);    /* enable plane graphics */
      outw(0x3CE, 0x0004);    /* read plane 0 */
      outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
      outw(0x3CE, 0x0506);    /* set graphics */
      slowbcopy_tobus(restore->TextInfo, vgaBase, TEXT_AMOUNT);
      outw(0x3C4, 0x0202);    /* write to plane 1 */
      outw(0x3C4, 0x0604);    /* enable plane graphics */
      outw(0x3CE, 0x0104);    /* read plane 1 */
      outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
      outw(0x3CE, 0x0506);    /* set graphics */
      slowbcopy_tobus((char *)restore->TextInfo + TEXT_AMOUNT, vgaBase, TEXT_AMOUNT);
    }
#endif
  }
#endif /* defined(SAVE_TEXT) || defined(SAVE_FONT1) || defined(SAVE_FONT2) */
  }
  vgaSaveScreen(NULL, TRUE);

#if !defined(PC98_PEGC) && !defined(PC98_EGC)

  tmp = inb(vgaIOBase + 0x0A);			/* Reset flip-flop */
  outb(0x3C0, 0x00);				/* Enables pallete access */

  /* Don't change the clock bits when using an external clock program */
  if (restore->NoClock < 0)
  {
    tmp = inb(0x3CC);
    restore->MiscOutReg = (restore->MiscOutReg & 0xF3) | (tmp & 0x0C);
  }
  if (vgaIOBase == 0x3B0)
    restore->MiscOutReg &= 0xFE;
  else
    restore->MiscOutReg |= 0x01;

  outb(0x3C2, restore->MiscOutReg);

  for (i=1; i<6;  i++) outw(0x3C4, (restore->Sequencer[i] << 8) | i);
  
  /* Ensure CRTC registers 0-7 are unlocked by clearing bit 7 or CRTC[17] */

  outw(vgaIOBase + 4, ((restore->CRTC[17] & 0x7F) << 8) | 17);

  for (i=0; i<25; i++) outw(vgaIOBase + 4,(restore->CRTC[i] << 8) | i);

  for (i=0; i<9;  i++) outw(0x3CE, (restore->Graphics[i] << 8) | i);

  for (i=0; i<21; i++) {
    tmp = inb(vgaIOBase + 0x0A);
    outb(0x3C0,i); outb(0x3C0, restore->Attribute[i]);
  }
  
  /* Turn on PAS bit */
  tmp = inb(vgaIOBase + 0x0A);
  outb(0x3C0, 0x20);

#endif /* !defined(PC98_PEGC) && !defined(PC98_EGC) */
}

/*
 * vgaHWSave --
 *      save the current video mode
 */

void *
vgaHWSave(save, size)
     vgaHWPtr save;
     int          size;
{
  int           i,tmp;
  Bool	        first_time = FALSE;  /* Should be static? */

  if (save == NULL) {
    tmp = size;
    if (tmp < 0)
      tmp = -size;
    if (tmp < sizeof(vgaHWRec))
      tmp = sizeof(vgaHWRec);
    save = (vgaHWPtr)xcalloc(1,tmp);
    /*
     * Here we are, when we first save the videostate. This means we came here
     * to save the original Text mode. Because some drivers may depend
     * on NoClock we set it here to a resonable value.
     */
    first_time = TRUE;
  }

#if !defined(PC98_PEGC) && !defined(PC98_EGC)
  save->MiscOutReg = inb(0x3CC);
#ifdef PC98
  save->MiscOutReg |= 0x01;
#endif
  vgaIOBase = (save->MiscOutReg & 0x01) ? 0x3D0 : 0x3B0;

  tmp = inb(vgaIOBase + 0x0A); /* reset flip-flop */
  outb(0x3C0, 0x00);

  for (i=0; i<25; i++) { outb(vgaIOBase + 4,i);
			 save->CRTC[i] = inb(vgaIOBase + 5); }

  for (i=0; i<21; i++) {
    tmp = inb(vgaIOBase + 0x0A);
    outb(0x3C0,i);
    save->Attribute[i] = inb(0x3C1);
  }

  for (i=0; i<9;  i++) { outb(0x3CE,i); save->Graphics[i]  = inb(0x3CF); }

  for (i=0; i<6;  i++) { outb(0x3C4,i); save->Sequencer[i]   = inb(0x3C5); }

#endif /* !defined(PC98_PEGC) && !defined(PC98_EGC) */

  vgaSaveScreen(NULL, FALSE);
  
#if !defined(PC98_PEGC) && !defined(PC98_EGC)
  /* XXXX Still not sure if this is needed.  It isn't done in the Restore */
  outb(0x3C2, save->MiscOutReg | 0x01);		/* shift to colour emulation */
  /* Since forced to colour mode, must use 0x3Dx instead of (vgaIOBase + x) */

  if (xf86bpp > 1) {
#if defined(SAVE_TEXT) || defined(SAVE_FONT1) || defined(SAVE_FONT2)
  /*
   * get the character sets, and text screen if required
   */
  if (((save->Attribute[0x10] & 0x01) == 0) && (size >= 0)) {
#ifdef SAVE_FONT1
    if (save->FontInfo1 == NULL) {
      save->FontInfo1 = (pointer)xalloc(FONT_AMOUNT);
      /*
       * Here we switch temporary to 16 color-plane-mode, to simply
       * copy the font-info
       *
       * BUGALLERT: The vga's segment-select register MUST be set appropriate !
       */
      tmp = inb(0x3D0 + 0x0A); /* reset flip-flop */
      outb(0x3C0,0x30); outb(0x3C0, 0x01); /* graphics mode */
      outw(0x3C4, 0x0402);    /* write to plane 2 */
      outw(0x3C4, 0x0604);    /* enable plane graphics */
      outw(0x3CE, 0x0204);    /* read plane 2 */
      outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
      outw(0x3CE, 0x0506);    /* set graphics */
      slowbcopy_frombus(vgaBase, save->FontInfo1, FONT_AMOUNT);
    }
#endif /* SAVE_FONT1 */
#ifdef SAVE_FONT2
    if (save->FontInfo2 == NULL) {
      save->FontInfo2 = (pointer)xalloc(FONT_AMOUNT);
      tmp = inb(0x3D0 + 0x0A); /* reset flip-flop */
      outb(0x3C0,0x30); outb(0x3C0, 0x01); /* graphics mode */
      outw(0x3C4, 0x0802);    /* write to plane 3 */
      outw(0x3C4, 0x0604);    /* enable plane graphics */
      outw(0x3CE, 0x0304);    /* read plane 3 */
      outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
      outw(0x3CE, 0x0506);    /* set graphics */
      slowbcopy_frombus(vgaBase, save->FontInfo2, FONT_AMOUNT);
    }
#endif /* SAVE_FONT2 */
#ifdef SAVE_TEXT
    if (save->TextInfo == NULL) {
      save->TextInfo = (pointer)xalloc(2 * TEXT_AMOUNT);
      tmp = inb(0x3D0 + 0x0A); /* reset flip-flop */
      outb(0x3C0,0x30); outb(0x3C0, 0x01); /* graphics mode */
      /*
       * This is a quick hack to save the text screen for system that don't
       * restore it automatically.
       */
      outw(0x3C4, 0x0102);    /* write to plane 0 */
      outw(0x3C4, 0x0604);    /* enable plane graphics */
      outw(0x3CE, 0x0004);    /* read plane 0 */
      outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
      outw(0x3CE, 0x0506);    /* set graphics */
      slowbcopy_frombus(vgaBase, save->TextInfo, TEXT_AMOUNT);
      outw(0x3C4, 0x0202);    /* write to plane 1 */
      outw(0x3C4, 0x0604);    /* enable plane graphics */
      outw(0x3CE, 0x0104);    /* read plane 1 */
      outw(0x3CE, 0x0005);    /* write mode 0, read mode 0 */
      outw(0x3CE, 0x0506);    /* set graphics */
      slowbcopy_frombus(vgaBase, (char *)save->TextInfo + TEXT_AMOUNT, TEXT_AMOUNT);
    }
#endif /* SAVE_TEXT */
  }
#endif /* defined(SAVE_TEXT) || defined(SAVE_FONT1) || defined(SAVE_FONT2) */
  }

  outb(0x3C2, save->MiscOutReg);		/* back to original setting */
#endif /* !defined(PC98_PEGC) && !defined(PC98_EGC) */
  
  vgaSaveScreen(NULL, TRUE);

#if !defined(PC98_PEGC) && !defined(PC98_EGC)
  /* Turn on PAS bit */
  tmp = inb(vgaIOBase + 0x0A);
  outb(0x3C0, 0x20);
#endif /* !defined(PC98_PEGC) && !defined(PC98_EGC) */
  
  return ((void *) save);
}
