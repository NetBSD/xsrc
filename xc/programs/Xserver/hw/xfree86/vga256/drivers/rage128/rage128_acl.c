/*
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
 * ATI Rage128 driver for 3.3.x
 *
 * Author:  Alan Hourihane, alanh@fairlite.demon..co.uk
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rage128/rage128_acl.c,v 1.1.2.2 1999/10/12 18:33:28 hohndel Exp $
 */

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga256.h"
#include "vga.h"
#include "miline.h"
#include "xf86xaa.h"

void RAGE128AccelInit();
static void RAGE128Sync();
static void RAGE128SetupForFillRectSolid();
static void RAGE128SubsequentFillRectSolid();
static void RAGE128SetupForScreenToScreenCopy();
static void RAGE128SubsequentScreenToScreenCopy();
static void RAGE128SetupFor8x8PatternColorExpand();
static void RAGE128Subsequent8x8PatternColorExpand();
static void RAGE128SetupForScanlineScreenToScreenColorExpand();
static void RAGE128SubsequentScanlineScreenToScreenColorExpand();

unsigned char scratchBuffer[512];
extern CARD32 IOBase;
int blitxdir, blitydir;
int scanlineWordCount;
int height, width, xcoord, ycoord;
int scanlineheight;
int BPP;

static int ROP[16] = 
    {
        0x00, /* GXclear        */
        0x88, /* Gxand          */
        0x44, /* GXandReverse   */
        0xCC, /* GXcopy         */
        0x22, /* GXandInverted  */
        0xAA, /* GXnoop         */
        0x66, /* GXxor          */
        0xEE, /* GXor           */
        0x11, /* GXnor          */
        0x99, /* GXequiv        */
        0x55, /* GXinvert       */
        0xDD, /* GXorReverse    */
        0x33, /* GXcopyInverted */
        0xBB, /* GXorInverted   */
        0x77, /* GXnand         */
        0xFF  /* GXset          */
    };
    
static int PATROP[16] = 
    {
        0x00, /* GXclear        */
        0xA0, /* Gxand          */
        0x50, /* GXandReverse   */
        0xF0, /* GXcopy         */
        0x0A, /* GXandInverted  */
        0xAA, /* GXnoop         */
        0x5A, /* GXxor          */
        0xFA, /* GXor           */
        0x05, /* GXnor          */
        0xA5, /* Gxequiv        */
        0x55, /* GXinvert       */
        0xF5, /* GXorReverse    */
        0x0F, /* GXcopyInverted */
        0xAF, /* GXorInverted   */
        0x5F, /* GXnand         */
        0xFF  /* GXset          */
    };

#define REPLICATE(x)  				\
	if (vgaBitsPerPixel < 32) {		\
		x |= x << 16;			\
		if (vgaBitsPerPixel < 16)	\
			x |= x << 8;		\
	}

void
RAGE128InitializeAccelerator(void)
{
	outl(IOBase, 0x146C);
	outl(IOBase + 4, 13<<4 | BPP<<8 | 3<<12);

	outl(IOBase, 0x16E0); /* DEFAULT_OFFSET */
	outl(IOBase + 4, 0);
	outl(IOBase, 0x16E4); /* DEFAULT_PITCH */
	outl(IOBase + 4, vga256InfoRec.displayWidth / 8);

	outl(IOBase, 0x1660);
	outl(IOBase + 4, 0);
	outl(IOBase, 0x16E8);
	outl(IOBase + 4, 0x1fff<<16 | 0x1fff);
	outl(IOBase, 0x1640);
	outl(IOBase + 4, 0x0);
	outl(IOBase, 0x1644);
	outl(IOBase + 4, 0x1fff);
	outl(IOBase, 0x1648);
	outl(IOBase + 4, 0x0);
	outl(IOBase, 0x164C);
	outl(IOBase + 4, 0x1fff);

	outl(IOBase, 0x147C);
	outl(IOBase + 4, 0);
	outl(IOBase, 0x16CC);
	outl(IOBase + 4, 0xffffffff);
}

void 
RAGE128AccelInit(void) 
{

  xf86AccelInfoRec.Flags = 	BACKGROUND_OPERATIONS | 
				PIXMAP_CACHE | 
				COP_FRAMEBUFFER_CONCURRENCY |
				HARDWARE_PATTERN_PROGRAMMED_BITS |
                             	HARDWARE_PATTERN_SCREEN_ORIGIN |
                             	HARDWARE_PATTERN_MONO_TRANSPARENCY;

  xf86AccelInfoRec.Sync = RAGE128Sync;

  /* Accelerated filled rectangles */
  xf86GCInfoRec.PolyFillRectSolidFlags = 0;
  xf86AccelInfoRec.SetupForFillRectSolid = RAGE128SetupForFillRectSolid;
  xf86AccelInfoRec.SubsequentFillRectSolid = RAGE128SubsequentFillRectSolid;

  /* Accelerated screen-screen bitblts */
  xf86GCInfoRec.CopyAreaFlags = NO_TRANSPARENCY;
  xf86AccelInfoRec.SetupForScreenToScreenCopy =
    RAGE128SetupForScreenToScreenCopy;
  xf86AccelInfoRec.SubsequentScreenToScreenCopy =
    RAGE128SubsequentScreenToScreenCopy;

  xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
    			RAGE128SetupFor8x8PatternColorExpand;
  xf86AccelInfoRec.Subsequent8x8PatternColorExpand =
    			RAGE128Subsequent8x8PatternColorExpand;
 

  xf86AccelInfoRec.ColorExpandFlags = BIT_ORDER_IN_BYTE_LSBFIRST;

  xf86AccelInfoRec.ScratchBufferAddr = 1;
  xf86AccelInfoRec.ScratchBufferSize = 512;
  xf86AccelInfoRec.ScratchBufferBase = (void*)scratchBuffer;
  xf86AccelInfoRec.PingPongBuffers = 1;

  xf86AccelInfoRec.SetupForScanlineScreenToScreenColorExpand =
    RAGE128SetupForScanlineScreenToScreenColorExpand;
  xf86AccelInfoRec.SubsequentScanlineScreenToScreenColorExpand =
    RAGE128SubsequentScanlineScreenToScreenColorExpand;

  switch (vga256InfoRec.bitsPerPixel) {
	case 8:
		BPP = 2;
		break;
	case 16:
		if (vga256InfoRec.depth == 15)
			BPP = 3;
		else
			BPP = 4;
		break;
	case 24:
		BPP = 5;
		break;
	case 32:
		BPP = 6;
		break;
   }
    RAGE128InitializeAccelerator();

  /* Pixmap cache setup */
  xf86AccelInfoRec.PixmapCacheMemoryStart =
    vga256InfoRec.virtualY * vga256InfoRec.displayWidth
    * vga256InfoRec.bitsPerPixel / 8;
  xf86AccelInfoRec.PixmapCacheMemoryEnd =
   vga256InfoRec.videoRam * 1024 - 1024; 
}

static void
RAGE128WaitForFifo(int entries)
{
	outl(IOBase, 0x1740);
	while ( (inl(IOBase + 4) & 0xFFF) < entries);
}

static void
RAGE128Sync()
{
    int count = 0, timeout = 0;
    CARD32 busy;

    RAGE128WaitForFifo(64);
    outl(IOBase, 0x1740);
    for (;;) {
	busy = inl(IOBase + 4) & 0x80000000;
	if (busy != 0x80000000) return;
	count++;
	if (count == 10000000) {
		ErrorF("RAGE128: GUI Engine time-out.\n");
		count = 9990000;
		timeout++;
		if (timeout == 8) {
			/* Reset BitBLT Engine */
			outl(IOBase + 4, 0);
			return;
		}
	}	
    }
}

static void 
RAGE128SetupForFillRectSolid(int color, int rop, unsigned int planemask)
{
    outl(IOBase, 0x146C);
    outl(IOBase + 4, 3<<28 | 13<<4 | BPP<<8 | PATROP[rop]<<16 | 3<<12);
    REPLICATE(color);
    outl(IOBase, 0x147C);
    outl(IOBase + 4, color);
    REPLICATE(planemask);
    outl(IOBase, 0x16CC);
    outl(IOBase + 4, planemask);
}

static void 
RAGE128SubsequentFillRectSolid(int x, int y, int w, int h)
{
    outl(IOBase, 0x1438);
    outl(IOBase + 4, (y<<16) | x);
    outl(IOBase, 0x143C);
    outl(IOBase + 4, (h<<16) | w);
}

static void 
RAGE128SetupForScreenToScreenCopy(int xdir, int ydir, int rop, unsigned int planemask,
                              int transparency_color)
{
    int direction = 0;
    blitxdir = xdir;
    blitydir = ydir;

    if (xdir >= 0)
	direction |= 1;
    if (ydir >= 0)
	direction |= 2;

    outl(IOBase, 0x146C);
    outl(IOBase + 4, 3<<28 | 15<<4 | BPP<<8 | ROP[rop]<<16 | 3<<12 | 2<<24);

    REPLICATE(planemask);
    outl(IOBase, 0x16CC);
    outl(IOBase + 4, planemask);
    outl(IOBase, 0x16C0);
    outl(IOBase + 4, direction);
    outl(IOBase, 0x15C0);
    outl(IOBase + 4, 0);
}


static void 
RAGE128SubsequentScreenToScreenCopy(int x1, int y1, int x2, int y2, int w, int h)
{
    if (blitydir < 0) {
        y1 = y1 + h - 1;
	y2 = y2 + h - 1;
    }
    if (blitxdir < 0) {
	x1 = x1 + w - 1;
	x2 = x2 + w - 1;
    }
    outl(IOBase, 0x1434);
    outl(IOBase + 4, (y1<<16) | x1);
    outl(IOBase, 0x1438);
    outl(IOBase + 4, (y2<<16) | x2);
    outl(IOBase, 0x143C);
    outl(IOBase + 4, (h<<16) | w);
}

static void RAGE128SetupForCPUToScreenColorExpand(bg, fg, rop, planemask)
    int bg, fg, rop;
    unsigned planemask;
{
    outl(IOBase, 0x146C);
    outl(IOBase + 4, 12<<4 | BPP<<8 | 1<<14 | ROP[rop]<<16 | 7<<28 | 3<<24);
    REPLICATE(planemask);
    outl(IOBase, 0x16CC);
    outl(IOBase + 4, planemask);
    REPLICATE(fg);
    outl(IOBase, 0x15D8);
    outl(IOBase + 4, fg);
    REPLICATE(bg);
    outl(IOBase, 0x15DC);
    outl(IOBase + 4, bg);
}

static void RAGE128SubsequentCPUToScreenColorExpand(x, y, w, h, skipleft)
    int x, y, w, h, skipleft;
{
    outl(IOBase, 0x1438);
    outl(IOBase + 4, (y<<16) | x);
    outl(IOBase, 0x143C);
    outl(IOBase + 4, (h<<16) | w);
}

static void 
RAGE128SetupFor8x8PatternColorExpand(patternx, patterny, bg, fg,
                                            rop, planemask)
    unsigned patternx, patterny, planemask;
    int bg, fg, rop;
{
    outl(IOBase, 0x146C);
    if (bg != -1)
    	outl(IOBase + 4, 3<<28 | 1<<14 | BPP<<8 | PATROP[rop]<<16 | 2<<12);
    else 
    	outl(IOBase + 4, 3<<28 | 1<<14 | 1<<4 | BPP<<8 | PATROP[rop]<<16 | 2<<12);
    REPLICATE(planemask);
    outl(IOBase, 0x16CC);
    outl(IOBase + 4, planemask);
    REPLICATE(fg);
    outl(IOBase, 0x147C);
    outl(IOBase + 4, fg);
    REPLICATE(bg);
    outl(IOBase, 0x1478);
    outl(IOBase + 4, bg);
    outl(IOBase, 0x1480);
    outl(IOBase + 4, patternx);
    outl(IOBase, 0x1484);
    outl(IOBase + 4, patterny);
}

static void 
RAGE128Subsequent8x8PatternColorExpand(patternx, patterny, x, y, w, h)
    unsigned patternx, patterny;
    int x, y, w, h;
{
    outl(IOBase, 0x1438);
    outl(IOBase + 4, (y<<16) | x);
    outl(IOBase, 0x143C);
    outl(IOBase + 4, (h<<16) | w);
}

static void RAGE128SetupForScanlineScreenToScreenColorExpand(int x,int y,int w,
                                                        int h,int bg,int fg,
                                                        int rop,
                                                        unsigned planemask)
{
    scanlineWordCount = (w + 31) >> 5;
    height = h; width = w; xcoord = x; ycoord = y;
    scanlineheight = 0;

    outl(IOBase, 0x146C);
    if (bg != -1) 
    	outl(IOBase + 4, 12<<4 | BPP<<8 | 1<<14 | ROP[rop]<<16 | 3<<28 | 3<<24);
    else
    	outl(IOBase + 4, 12<<4 | 1<<12 | BPP<<8 | 1<<14 | ROP[rop]<<16 | 3<<28 | 3<<24);

#if 0
    outl(IOBase, 0x16EC);
    outl(IOBase + 4, y<<16 | x);
    outl(IOBase, 0x16F0);
    outl(IOBase + 4, (y+h-1)<<16 | (x+w-1));
#endif

    REPLICATE(planemask);
    outl(IOBase, 0x16CC);
    outl(IOBase + 4, planemask);
    REPLICATE(fg);
    outl(IOBase, 0x15D8);
    outl(IOBase + 4, fg);
    REPLICATE(bg);
    outl(IOBase, 0x15DC);
    outl(IOBase + 4, bg);
}

static void RAGE128SubsequentScanlineScreenToScreenColorExpand(int srcAddr)
{
    CARD32 *ptr = (CARD32*)scratchBuffer;
    int count = scanlineWordCount;
    int i=0;

    outl(IOBase, 0x1438);
    outl(IOBase + 4, (ycoord+scanlineheight<<16) | xcoord);
    outl(IOBase, 0x143C);
    outl(IOBase + 4, (1<<16) | width);

    for(i=0;i<count-1;i++) {
        outl(IOBase, 0x17C0);
        outl(IOBase + 4, (*(ptr++)));
    }
    outl(IOBase, 0x17E0);
    outl(IOBase + 4, (*(ptr)));
    scanlineheight++;
}
