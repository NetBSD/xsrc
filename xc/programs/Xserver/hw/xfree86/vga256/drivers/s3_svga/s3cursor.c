/*
 * $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3cursor.c,v 1.1.2.4 1998/10/22 04:31:07 hohndel Exp $
*
*/

/* Written by Mark Vojkovich  (mvojkovi@ucsd.edu) */

#define PSZ 8

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "input.h"
#include "cursorstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "windowstr.h"

#include "compiler.h"
#include "xf86.h"
#include "mipointer.h"
#include "xf86Priv.h"
#include "xf86_Option.h"
#include "xf86_OSlib.h"
#include "vga256.h"
#include "vga.h"
#include "xf86cursor.h"
#include "xf86xaa.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "s3.h"
#include "s3reg.h"

extern void s3IBMRGBShowCursor();
extern void s3IBMRGBHideCursor();
extern void s3IBMRGBSetCursorPosition();
extern void s3IBMRGBSetCursorColors();
extern void s3IBMRGBLoadCursorImage();

extern void s3BtShowCursor();
extern void s3BtHideCursor();
extern void s3BtSetCursorPosition();
extern void s3BtSetCursorColors();
extern void s3BtLoadCursorImage();

extern void s3TiShowCursor();
extern void s3TiHideCursor();
extern void s3TiSetCursorPosition();
extern void s3TiSetCursorColors();
extern void s3TiLoadCursorImage();

extern void s3Ti3026ShowCursor();
extern void s3Ti3026HideCursor();
extern void s3Ti3026SetCursorPosition();
extern void s3Ti3026SetCursorColors();
extern void s3Ti3026LoadCursorImage();

static void s3ShowCursor();
void s3HideCursor(); /* this is needed in EnterLeave for DGA... */
static void s3SetCursorPosition();
static void s3SetCursorColors();
static void s3LoadCursorImage();

extern vgaHWCursorRec vgaHWCursor;

extern Bool XAACursorInit();
extern void XAARestoreCursor();
extern void XAAWarpCursor();
extern void XAAQueryBestSize();

static unsigned int CursorAddress;

void S3CursorInit()
{
    s3CursorBytes = 0;

    XAACursorInfoRec.MaxWidth = 64;
    XAACursorInfoRec.MaxHeight = 64;

    if(DAC_IS_BT485_SERIES) {
    	XAACursorInfoRec.Flags = USE_HARDWARE_CURSOR |
				HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
				HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
				HARDWARE_CURSOR_PROGRAMMED_ORIGIN |
				HARDWARE_CURSOR_AND_SOURCE_WITH_MASK | 	 
				HARDWARE_CURSOR_CHAR_BIT_FORMAT |
				HARDWARE_CURSOR_PROGRAMMED_BITS;
	XAACursorInfoRec.SetCursorColors = s3BtSetCursorColors;
	XAACursorInfoRec.SetCursorPosition = s3BtSetCursorPosition;
	XAACursorInfoRec.LoadCursorImage = s3BtLoadCursorImage;
	XAACursorInfoRec.HideCursor = s3BtHideCursor;
	XAACursorInfoRec.ShowCursor = s3BtShowCursor; 
      	ErrorF("%s %s: Using Bt485/att20c505 hardware cursor.\n", 
			XCONFIG_PROBED, vga256InfoRec.name);    
     }
     else if(DAC_IS_IBMRGB) {
    	XAACursorInfoRec.Flags = USE_HARDWARE_CURSOR |
				HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
				HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE |
				HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
				HARDWARE_CURSOR_PROGRAMMED_ORIGIN |		
				HARDWARE_CURSOR_CHAR_BIT_FORMAT |
				HARDWARE_CURSOR_PROGRAMMED_BITS;
	XAACursorInfoRec.SetCursorColors = s3IBMRGBSetCursorColors;
	XAACursorInfoRec.SetCursorPosition = s3IBMRGBSetCursorPosition;
	XAACursorInfoRec.LoadCursorImage = s3IBMRGBLoadCursorImage;
	XAACursorInfoRec.HideCursor = s3IBMRGBHideCursor;
	XAACursorInfoRec.ShowCursor = s3IBMRGBShowCursor;
      	ErrorF("%s %s: Using IBM RGB_52x hardware cursor.\n", 
			XCONFIG_PROBED, vga256InfoRec.name);    
    } 
    else if(DAC_IS_TI3020_SERIES) {
    	XAACursorInfoRec.Flags = USE_HARDWARE_CURSOR |
				HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
				HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
				HARDWARE_CURSOR_PROGRAMMED_ORIGIN |
				HARDWARE_CURSOR_AND_SOURCE_WITH_MASK | 	 
				HARDWARE_CURSOR_CHAR_BIT_FORMAT |
				HARDWARE_CURSOR_PROGRAMMED_BITS;
	XAACursorInfoRec.SetCursorColors = s3TiSetCursorColors;
	XAACursorInfoRec.SetCursorPosition = s3TiSetCursorPosition;
	XAACursorInfoRec.LoadCursorImage = s3TiLoadCursorImage;
	XAACursorInfoRec.HideCursor = s3TiHideCursor;
	XAACursorInfoRec.ShowCursor = s3TiShowCursor;
      	ErrorF("%s %s: Using Ti3020/3025 hardware cursor.\n", 
			XCONFIG_PROBED, vga256InfoRec.name);    
    } else if(DAC_IS_TI3026 || DAC_IS_TI3030) {
    	XAACursorInfoRec.Flags = USE_HARDWARE_CURSOR |
				HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
				HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
				HARDWARE_CURSOR_PROGRAMMED_ORIGIN |
				HARDWARE_CURSOR_AND_SOURCE_WITH_MASK | 	 
				HARDWARE_CURSOR_CHAR_BIT_FORMAT |
				HARDWARE_CURSOR_PROGRAMMED_BITS;
	XAACursorInfoRec.SetCursorColors = s3Ti3026SetCursorColors;
	XAACursorInfoRec.SetCursorPosition = s3Ti3026SetCursorPosition;
	XAACursorInfoRec.LoadCursorImage = s3Ti3026LoadCursorImage;
	XAACursorInfoRec.HideCursor = s3Ti3026HideCursor;
	XAACursorInfoRec.ShowCursor = s3Ti3026ShowCursor;
      	ErrorF("%s %s: Using Ti3026/3030 hardware cursor.\n", 
			XCONFIG_PROBED, vga256InfoRec.name);    
    } else {	/* generic S3 cursor */
        CursorAddress =  (vga256InfoRec.videoRam - 1) << 10;

	if(CursorAddress < (vga256InfoRec.virtualY * s3BppDisplayWidth)) {
      	    ErrorF("%s %s: Not enough video memory left for hardware cursor "
		"storage... Disabling.\n", XCONFIG_PROBED, vga256InfoRec.name);
    	   OFLG_SET(OPTION_SW_CURSOR, &vga256InfoRec.options); 
 	   return;
	}

        XAACursorInfoRec.CursorDataX = 
		(CursorAddress % s3BppDisplayWidth) / s3Bpp;
        XAACursorInfoRec.CursorDataY = CursorAddress / s3BppDisplayWidth;
        s3CursorBytes = 1024;
        CursorAddress >>= 10;


    	XAACursorInfoRec.Flags = USE_HARDWARE_CURSOR |
				HARDWARE_CURSOR_PROGRAMMED_BITS |
				HARDWARE_CURSOR_PROGRAMMED_ORIGIN |		
				HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
				HARDWARE_CURSOR_SHORT_BIT_FORMAT;
	XAACursorInfoRec.LoadCursorImage = s3LoadCursorImage;
	XAACursorInfoRec.SetCursorColors = s3SetCursorColors;
	XAACursorInfoRec.SetCursorPosition = s3SetCursorPosition;
	XAACursorInfoRec.HideCursor = s3HideCursor;
	XAACursorInfoRec.ShowCursor = s3ShowCursor;
	XAACursorInfoRec.GetInstalledColormaps = vgaGetInstalledColormaps;

      	ErrorF("%s %s: Using built-in S3 hardware cursor.\n", 
			XCONFIG_PROBED, vga256InfoRec.name);    
    }

    if(XAACursorInfoRec.Flags & USE_HARDWARE_CURSOR) {
	vgaHWCursor.Init = XAACursorInit;
	vgaHWCursor.Initialized = TRUE;
	vgaHWCursor.Restore = XAARestoreCursor;
	vgaHWCursor.Warp = XAAWarpCursor;
	vgaHWCursor.QueryBestSize = XAAQueryBestSize;
    }
}


#define VerticalRetraceWait() \
{ \
   outb(vgaCRIndex, 0x17); \
   if ( inb(vgaCRReg) & 0x80 ) { \
       while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x00) ; \
       while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x08) ; \
       }\
}

static void 
s3LoadCursorImage(bits, xorigin, yorigin)
   unsigned char *bits;
   int xorigin, yorigin;
{
   unsigned char cr45;

   if (!xf86VTSema)
      return;

   UNLOCK_SYS_REGS;

   WaitIdle();

   /* Wait for vertical retrace */
   VerticalRetraceWait();

   /* turn cursor off */
   outb(vgaCRIndex, 0x45);
   cr45 = inb(vgaCRReg);
   outb(vgaCRReg, cr45 & 0xFE);

   /* move cursor off-screen */
   outb(vgaCRIndex, 0x46);
   outb(vgaCRReg, 0xff);
   outb(vgaCRIndex, 0x47);
   outb(vgaCRReg, 0x7f);
   outb(vgaCRIndex, 0x49);
   outb(vgaCRReg, 0xff);
   outb(vgaCRIndex, 0x4e);
   outb(vgaCRReg, 0x3f);
   outb(vgaCRIndex, 0x4f);
   outb(vgaCRReg, 0x3f);
   outb(vgaCRIndex, 0x48);
   outb(vgaCRReg, 0x7f);

   if (xorigin == 0 && yorigin == 0)
       xf86AccelInfoRec.ImageWrite(
           XAACursorInfoRec.CursorDataX,
           XAACursorInfoRec.CursorDataY,
           (XAACursorInfoRec.MaxWidth * XAACursorInfoRec.MaxHeight * 2) / xf86bpp,
           1,
           bits,
           0, GXcopy, ~0
       );
   else
       /*
        * XXX
        * Must simulate programmable origin by uploading pattern
        * "skewed" (horizontally and/or vertically).
        */
      	ErrorF("%s %s: s3cursor:  x/y origin %d %d  != 0\n", 
			XCONFIG_PROBED, vga256InfoRec.name,xorigin,yorigin);
       ;

   WaitIdle();

   /* Wait for vertical retrace */
   VerticalRetraceWait();

   /* turn cursor on */
   outb(vgaCRIndex, 0x45);
   outb(vgaCRReg, cr45);

   LOCK_SYS_REGS;
}

 
void 
s3ShowCursor()
{
   unsigned char tmp;

   UNLOCK_SYS_REGS;

   outb(vgaCRIndex, 0x55);
   tmp = inb(vgaCRReg);
   outb(vgaCRReg, tmp | 0x10);

   outb(vgaCRIndex, 0x4c);
   outb(vgaCRReg, CursorAddress >> 8);
   outb(vgaCRIndex, 0x4d);
   outb(vgaCRReg, CursorAddress);

   outb(vgaCRIndex, 0x45);
   tmp = inb(vgaCRReg);
   outb(vgaCRReg, tmp | 0x01);

   LOCK_SYS_REGS;
}


void
s3HideCursor()
{
   unsigned char tmp;

   UNLOCK_SYS_REGS;

   outb(vgaCRIndex, 0x45);
   tmp = inb(vgaCRReg);
   outb(vgaCRReg, tmp & ~0x01);

   LOCK_SYS_REGS;
}


void
s3SetCursorPosition(x, y, xoff, yoff)
     int x, y, xoff, yoff;
{
   UNLOCK_SYS_REGS;

   if (!S3_TRIOxx_SERIES(s3ChipId)) {
      if (S3_968_SERIES(s3ChipId))
	 x *= (2 * s3Bpp);
      else if (!S3_x64_SERIES(s3ChipId) && !S3_805_I_SERIES(s3ChipId)) 
	 x *= s3Bpp;
      else if (s3Bpp > 2)
	 x *= 2;
   }

   if (vga256InfoRec.modes->Flags & V_DBLSCAN)
	y *= 2; 

   outb(vgaCRIndex, 0x46);
   outb(vgaCRReg, x >> 8);

   outb(vgaCRIndex, 0x47);
   outb(vgaCRReg, x);

   outb(vgaCRIndex, 0x49);
   outb(vgaCRReg, y);

   outb(vgaCRIndex, 0x4e);
   outb(vgaCRReg, xoff);

   outb(vgaCRIndex, 0x4f);
   outb(vgaCRReg, yoff);      

   outb(vgaCRIndex, 0x48);
   outb(vgaCRReg, y >> 8);

   LOCK_SYS_REGS;

}

void
s3SetCursorColors(bg, fg)
   unsigned int bg, fg;
{	 
    unsigned short packfg, packbg;

    switch(s3Bpp) {
	case 1:
	   if(S3_TRIOxx_SERIES(s3ChipId)) {
    	   	outb(vgaCRIndex, 0x45);
    	   	inb(vgaCRReg);		/* reset stack pointer */ 
    	   	outb(vgaCRIndex, 0x4A);
    	   	outb(vgaCRReg, fg);
    	   	outb(vgaCRReg, fg);

    	   	outb(vgaCRIndex, 0x45);
    	   	inb(vgaCRReg);		/* reset stack pointer */
    	   	outb(vgaCRIndex, 0x4B);
    	   	outb(vgaCRReg, bg);
    	   	outb(vgaCRReg, bg);
	   } else {
		outb(vgaCRIndex, 0x0E);
		outb(vgaCRReg, fg);
		outb(vgaCRIndex, 0x0F);
		outb(vgaCRReg, bg);
	   }
	   break;
	case 2:
	   if(vga256InfoRec.depth == 15) {
	      packfg = ((fg & 0x00F80000) >> 19) | ((fg & 0x0000F800) >> 6) |
			((fg & 0x000000F8) <<  7);
	      packbg = ((bg & 0x00F80000) >> 19) | ((bg & 0x0000F800) >> 6) |
			((bg & 0x000000F8) << 7);
	   } else {
	      packfg = ((fg & 0x00F80000) >> 19) | ((fg & 0x0000Fc00) >> 5) |
			((fg & 0x000000F8) <<  8);
	      packbg = ((bg & 0x00F80000) >> 19) | ((bg & 0x0000Fc00) >> 5) |
			((bg & 0x000000F8) << 8);
	   }

    	   outb(vgaCRIndex, 0x45);
    	   inb(vgaCRReg);		/* reset stack pointer */ 
    	   outb(vgaCRIndex, 0x4A);
    	   outb(vgaCRReg, packfg);
    	   outb(vgaCRReg, packfg >> 8);

    	   outb(vgaCRIndex, 0x45);
    	   inb(vgaCRReg);		/* reset stack pointer */
    	   outb(vgaCRIndex, 0x4B);
    	   outb(vgaCRReg, packbg);
    	   outb(vgaCRReg, packbg >> 8);
	   break;
	default:
    	   outb(vgaCRIndex, 0x45);
    	   inb(vgaCRReg);		/* reset stack pointer */ 
    	   outb(vgaCRIndex, 0x4A);
    	   outb(vgaCRReg, (fg & 0x00FF0000) >> 16);
    	   outb(vgaCRReg, (fg & 0x0000FF00) >> 8);
    	   outb(vgaCRReg, (fg & 0x000000FF));

    	   outb(vgaCRIndex, 0x45);
    	   inb(vgaCRReg);		/* reset stack pointer */
    	   outb(vgaCRIndex, 0x4B);
    	   outb(vgaCRReg, (bg & 0x00FF0000) >> 16);
    	   outb(vgaCRReg, (bg & 0x0000FF00) >> 8);
    	   outb(vgaCRReg, (bg & 0x000000FF));
	   break;
    }
}


