/******************************************************************************\

				   Copyright (c) 1999 by Silicon Motion, Inc.
							   All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright notice
and this permission notice appear in supporting documentation, and that the name
of Silicon Motion, Inc. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Silicon Motion, Inc. and its suppliers make no representations about the
suitability of this software for any purpose.  It is provided "as is" without
express or implied warranty.

SILICON MOTION, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SILICON MOTION, INC. AND/OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
\******************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/smi/smi_accel.c,v 1.1.2.2 1999/12/11 17:43:20 hohndel Exp $ */

#include <math.h>
#include "xf86.h"
#include "xf86_OSlib.h"
#include "vga256.h"
#include "vga.h"
#include "xf86xaa.h"
#include "xf86Priv.h"
#include "regsmi.h"
#include "smi_driver.h"
#include "smi_rop.h"

#define MESSAGE(x)	if (xf86Verbose) ErrorF x;

extern SMIPRIV smiPriv;
extern pointer smiMmioMem;
unsigned int smiBltCommand;

/* Forward declaration of fucntions used in the driver */
void SMIAccelSync();
void SMIAccelInit();
void SMISetupForScreenToScreenCopy();
void SMISubsequentScreenToScreenCopy();
void SMISetupForFillRectSolid();
void SMISubsequentFillRectSolid();
void SMISetupForCPUToScreenColorExpand();
void SMISubsequentCPUToScreenColorExpand();
void SMISetupFor8x8PatternColorExpand();
void SMISubsequent8x8PatternColorExpand();
void SMISetupForFill8x8Pattern();
void SMISubsequentFill8x8Pattern();

/* Acceleration init function, sets up pointers to our accelerated functions */

void 
SMIAccelInit() 
{
    /* General acceleration flags */
    xf86AccelInfoRec.Flags = BACKGROUND_OPERATIONS
    					   | HARDWARE_PATTERN_BIT_ORDER_MSBFIRST
    					   | HARDWARE_PATTERN_SCREEN_ORIGIN
    					   | HARDWARE_PATTERN_PROGRAMMED_BITS;

	xf86AccelInfoRec.Sync = SMIAccelSync;

	/* ScreenToScreen copies */
	xf86GCInfoRec.CopyAreaFlags = NO_TRANSPARENCY
								| NO_PLANEMASK;
	xf86AccelInfoRec.SetupForScreenToScreenCopy =
    		SMISetupForScreenToScreenCopy;
	xf86AccelInfoRec.SubsequentScreenToScreenCopy =
			SMISubsequentScreenToScreenCopy;
	if (smiPriv.chip == SMI_820 && vgaBitsPerPixel == 8)
	{
		xf86GCInfoRec.CopyAreaFlags |= GXCOPY_ONLY;
	}

	/* Filled rectangles */
	xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;
	xf86AccelInfoRec.SetupForFillRectSolid =
			SMISetupForFillRectSolid;
	xf86AccelInfoRec.SubsequentFillRectSolid =
			SMISubsequentFillRectSolid;
	if (smiPriv.chip == SMI_820 && vgaBitsPerPixel == 8)
	{
		xf86GCInfoRec.PolyFillRectSolidFlags |= GXCOPY_ONLY;
	}

	/* CPU to screen color expansion */
	xf86AccelInfoRec.ColorExpandFlags = SCANLINE_PAD_DWORD
									  | CPU_TRANSFER_PAD_DWORD
									  | BIT_ORDER_IN_BYTE_MSBFIRST
									  | NO_PLANEMASK;
	if (IS_NEWMMIO(smiPriv.chip))
	{
		xf86AccelInfoRec.CPUToScreenColorExpandBase  = (void *)
				((char *) smiMmioMem + 0x6000);
		xf86AccelInfoRec.CPUToScreenColorExpandRange = 0x2000;
	}
	else
	{
		xf86AccelInfoRec.CPUToScreenColorExpandBase  = smiMmioMem;
		xf86AccelInfoRec.CPUToScreenColorExpandRange = 0x8000;
	}
	if (smiPriv.chip == SMI_820 && vgaBitsPerPixel == 8)
	{
		xf86AccelInfoRec.ColorExpandFlags |= GXCOPY_ONLY;
	}

	xf86AccelInfoRec.SetupForCPUToScreenColorExpand =
			SMISetupForCPUToScreenColorExpand;
	xf86AccelInfoRec.SubsequentCPUToScreenColorExpand =
			SMISubsequentCPUToScreenColorExpand;
 
	/* These are the 8x8 pattern fills using color expansion */
	xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
			SMISetupFor8x8PatternColorExpand;
	xf86AccelInfoRec.Subsequent8x8PatternColorExpand =
			SMISubsequent8x8PatternColorExpand;

	/* These are the 8x8 color pattern fills */
	if (vgaBitsPerPixel <= 16)
	{
		xf86AccelInfoRec.SetupForFill8x8Pattern =
				SMISetupForFill8x8Pattern;
		xf86AccelInfoRec.SubsequentFill8x8Pattern =
				SMISubsequentFill8x8Pattern;
	}

    /*
     * In 8- or 16-bpp we can have a pixel cache (we could in 24-bpp, but I am
     * not sure about the alignment issues).
     */

	if (vgaBitsPerPixel <= 16)
	{
		unsigned int cacheStart, cacheEnd;
		cacheStart = vga256InfoRec.virtualY * vga256InfoRec.displayWidth
				   * vgaBitsPerPixel / 8;
		cacheEnd   = vga256InfoRec.videoRam * 1024
				   - smiPriv.MemReserved
				   - xf86AccelInfoRec.ScratchBufferSize;

		if (cacheEnd > cacheStart)
		{
			xf86InitPixmapCache(&vga256InfoRec, cacheStart, cacheEnd);
			xf86AccelInfoRec.Flags |= PIXMAP_CACHE;
			MESSAGE(("Pixelmap cache: 0x%08X-0x%08X\n", cacheStart, cacheEnd));
		}
	}

	MESSAGE(("Initialized acceleration for %dx%d in %d-bpp\n",
			vga256InfoRec.displayWidth, vga256InfoRec.virtualY,
			vgaBitsPerPixel));
}

/* The sync function for the GE */

void
SMIAccelSync()
{
	WaitIdleEmpty(); 
}

void
SMISetupForScreenToScreenCopy(int xdir, int ydir, int rop, unsigned planemask,
							  int transparency_color)
{
	smiBltCommand = smiAlu[rop]
				  | SMI_BITBLT
				  | SMI_STARTENGINE;
}

void
SMISubsequentScreenToScreenCopy(int x1, int y1, int x2, int y2, int w, int h)
{
	if ((y2 < y1) || ((y2 == y1) && (x2 < x1)))
	{
		smiBltCommand &= ~SMI_RIGHTTOLEFT;
	}
	else
	{
		smiBltCommand |= SMI_RIGHTTOLEFT;
		x1 += w - 1;
		y1 += h - 1;
		x2 += w - 1;
		y2 += h - 1;
	}

	if (vgaBitsPerPixel == 24)
	{
		x1 *= 3;
		x2 *= 3;
		w  *= 3;
		if (smiPriv.chip == SMI_910)
		{
			y1 *= 3;
			y2 *= 3;
		}
		if (smiBltCommand & SMI_RIGHTTOLEFT)
		{
			x1 += 2;
			x2 += 2;
		}
	}

	WaitQueue(4);
	smiPriv.ptrDPR->dpr00 = (x1 << 16) + (y1 & 0xFFFF);
	smiPriv.ptrDPR->dpr04 = (x2 << 16) + (y2 & 0xFFFF);
	smiPriv.ptrDPR->dpr08 = (w  << 16) + (h  & 0xFFFF);
	smiPriv.ptrDPR->dpr0C = smiBltCommand;
}

void 
SMISetupForFillRectSolid(int color, int rop, unsigned planemask)
{
	WaitQueue(3);
	smiPriv.ptrDPR->dpr14 = color;
	smiPriv.ptrDPR->dpr34 = 0xFFFFFFFF;
	smiPriv.ptrDPR->dpr38 = 0xFFFFFFFF;

	smiBltCommand = smiAlu_sp[rop]
				  | SMI_BITBLT
				  | SMI_STARTENGINE;
}
    
void 
SMISubsequentFillRectSolid(int x, int y, int w, int h)
{
	if (vgaBitsPerPixel == 24)
	{
		x *= 3;
		w *= 3;
		if (smiPriv.chip == SMI_910)
		{
			y *= 3;
		}
	}

	WaitQueue(3);
	smiPriv.ptrDPR->dpr04 = (x << 16) + (y & 0xFFFF);
	smiPriv.ptrDPR->dpr08 = (w << 16) + (h & 0xFFFF);
	smiPriv.ptrDPR->dpr0C = smiBltCommand;
}

void
SMISetupForCPUToScreenColorExpand(int bg, int fg, int rop, unsigned planemask)
{
	if (bg == -1)
	{
		WaitQueue(4);
		smiPriv.ptrDPR->dpr00 = 0;
		smiPriv.ptrDPR->dpr14 = fg;
		smiPriv.ptrDPR->dpr18 = ~fg;
		smiPriv.ptrDPR->dpr20 = fg;

		smiBltCommand = smiAlu[rop]
					  | SMI_BLTWRITE
					  | SMI_TRANSPARENT
					  | SMI_MONOSOURCE
					  | SMI_STARTENGINE;
	}
	else
	{
		WaitQueue(3);
		smiPriv.ptrDPR->dpr00 = 0;
		smiPriv.ptrDPR->dpr14 = fg;
		smiPriv.ptrDPR->dpr18 = bg;

		smiBltCommand = smiAlu[rop]
					  | SMI_BLTWRITE
					  | SMI_MONOSOURCE
					  | SMI_STARTENGINE;
	}
}

void
SMISubsequentCPUToScreenColorExpand(int x, int y, int w, int h, int skipleft)
{
	if (vgaBitsPerPixel == 24)
	{
		x *= 3;
		w *= 3;
		if (smiPriv.chip == SMI_910)
		{
			y *= 3;
		}
	}

	WaitQueue(3);
	smiPriv.ptrDPR->dpr04 = (x << 16) + (y & 0xFFFF);
	smiPriv.ptrDPR->dpr08 = (w << 16) + (h & 0xFFFF);
	smiPriv.ptrDPR->dpr0C = smiBltCommand;
}

void
SMISetupFor8x8PatternColorExpand(unsigned patternx, unsigned patterny, int bg,
								 int fg, int rop, unsigned planemask)
{
	WaitQueue(4);
	smiPriv.ptrDPR->dpr14 = fg;
	smiPriv.ptrDPR->dpr18 = bg;
	smiPriv.ptrDPR->dpr34 = patternx;
	smiPriv.ptrDPR->dpr38 = patterny;

	smiBltCommand = smiAlu_sp[rop]
				  | SMI_BITBLT
				  | SMI_MONOPATTERN
				  | SMI_STARTENGINE;
}

void
SMISubsequent8x8PatternColorExpand(unsigned patternx, unsigned patterny, int x,
								   int y, int w, int h)
{
	if (vgaBitsPerPixel == 24)
	{
		x *= 3;
		w *= 3;
		if (smiPriv.chip == SMI_910)
		{
			y *= 3;
		}
	}

	WaitQueue(3);
	smiPriv.ptrDPR->dpr04 = (x << 16) + (y & 0xFFFF);
	smiPriv.ptrDPR->dpr08 = (w << 16) + (h & 0xFFFF);
	smiPriv.ptrDPR->dpr0C = smiBltCommand;
}

void
SMISetupForFill8x8Pattern(unsigned patternx, unsigned patterny, int rop,
						  unsigned planemask, int trans_col)
{
	char *patloc = (char *) xf86AccelInfoRec.FramebufferBase
				 + patternx * vgaBitsPerPixel / 8
				 + patterny * vga256InfoRec.displayWidth;

	WaitQueue(1);
	smiPriv.ptrDPR->dpr0C = SMI_BITBLT
						  | SMI_COLORPATTERN;
	MemToBus((char *) xf86AccelInfoRec.CPUToScreenColorExpandBase, patloc,
			vgaBitsPerPixel * 8);

	smiBltCommand = smiAlu_sp[rop]
				  | SMI_BITBLT
				  | SMI_COLORPATTERN
				  | SMI_STARTENGINE;
}

void SMISubsequentFill8x8Pattern(unsigned patternx, unsigned patterny, int x,
								 int y, int w, int h)
{
	WaitQueue(3);
	smiPriv.ptrDPR->dpr04 = (x << 16) + (y & 0xFFFF);
	smiPriv.ptrDPR->dpr08 = (w << 16) + (h & 0xFFFF);
	smiPriv.ptrDPR->dpr0C = smiBltCommand;
}

void
SMIGEReset(int from_timeout, int line, char *file)
{
	unsigned char tmp;
	int r;
	int32 fifo_control, miu_control, streams_timeout, misc_timeout;

	if (from_timeout)
	{
		static int n = 0;
		if (n++ < 10 || xf86Verbose > 1)
		{
			ErrorF("\tSMIGEReset called from %s line %d\n", file, line);
		}
	}
	else
	{
		WaitIdleEmpty();
	}

	outb(0x3C4, 0x15);
	tmp = inb(0x3C5);
	outb(0x3C5, tmp | 0x30);

	WaitIdleEmpty();

	outb(0x3C4, 0x15);
	outb(0x3C5, tmp);
}
