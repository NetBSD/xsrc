/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/apm/apm_accel.c,v 1.1.2.5 1999/10/13 14:44:23 hohndel Exp $ */


/*
  Created 1997-06-08 by Henrik Harmsen (hch@cd.chalmers.se or Henrik.Harmsen@erv.ericsson.se)

  Does (for 8, 16 and 32 bpp modes):
    - Filled rectangles
    - Screen-screen bitblts
    - Host-screen color expand bitblts (text acceleration)
    - Line drawing
    
  See apm_driver.c for more info.
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
#include "apm.h"


/* Defines */
#define MAXLOOP 1000000


/* Exported functions */
void ApmAccelInit(void);


/* Local functions */
static void ApmSync(void);
static void ApmSync6422(void);
static void ApmSetupForFillRectSolid(int color, int rop, unsigned int planemask);
static void ApmSubsequentFillRectSolid(int x, int y, int w, int h);
static void ApmSetupForScreenToScreenCopy(int xdir, int ydir, int rop, unsigned int planemask,
                                          int transparency_color);
static void ApmSubsequentScreenToScreenCopy(int x1, int y1, int x2, int y2, int w, int h);
#if 0
static void ApmSetupForCPUToScreenColorExpand(int bg, int fg, int rop, unsigned int planemask);
static void ApmSubsequentCPUToScreenColorExpand(int x, int y, int w, int h, int skipleft);
#endif
static void ApmSetupForScreenToScreenColorExpand(int bg, int fg, int rop,
                                                 unsigned int planemask);
static void ApmSubsequentScreenToScreenColorExpand(int srcx, int srcy, int x, 
                                                   int y, int w, int h);
static void ApmSubsequentBresenhamLine(int x1, int y1, int octant, int err, int e1, int e2, int length);
static void ApmSubsequentBresenhamLine6422(int x1, int y1, int octant, int err, int e1, int e2, int length);
static void ApmSetClippingRectangle(int x1, int y1, int x2, int y2);

static void Dump(void* start, u32 len);


/* Statics */
static int blitxdir, blitydir;
static u32 apmBitsPerPixel_DEC;
static u32 apmScreenWidth_DEC;
static int apmTransparency;
static int apmClip = FALSE;


/* Translation from X ROP's to APM ROP's. */
static unsigned char apmROP[] = {
  0,
  0x88,
  0x44,
  0xCC,
  0x22,
  0xAA,
  0x66,
  0xEE,
  0x11,
  0x99,
  0x55,
  0xDD,
  0x33,
  0xBB,
  0x77,
  0xFF
};


/* Globals */
int apmMMIO_Init = FALSE;
volatile u8* apmRegBase = NULL;


/* Inline functions */
static __inline__ void
WaitForFifo(int slots)
{
  volatile int i;

  for(i = 0; i < MAXLOOP; i++) { 
    if ((STATUS() & STATUS_FIFO) >= slots)
      break;
  }
  if (i == MAXLOOP)
    FatalError("Hung in WaitForFifo()\n");
}

static __inline__ void
ApmCheckMMIO_InitFast(void)
{
  if (!apmMMIO_Init)
    ApmCheckMMIO_Init();
}


/*********************************************************************************************/

void 
ApmAccelInit(void) 
{
  xf86AccelInfoRec.Flags = BACKGROUND_OPERATIONS | PIXMAP_CACHE | 
    NO_PLANEMASK | COP_FRAMEBUFFER_CONCURRENCY | HARDWARE_CLIP_LINE;

  xf86AccelInfoRec.Sync = ApmSync;

  /* Accelerated filled rectangles */
  xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;
  xf86AccelInfoRec.SetupForFillRectSolid = ApmSetupForFillRectSolid;
  xf86AccelInfoRec.SubsequentFillRectSolid = ApmSubsequentFillRectSolid;
#if 0
  /* The Alliance chips are too much demanding. Need to check after each write
   * if the chip is ok to have the next one. If not the machine ends crashed...
   */
  /* Accelerated CPU to screen color expansion */
  xf86AccelInfoRec.SetupForCPUToScreenColorExpand = ApmSetupForCPUToScreenColorExpand;
  xf86AccelInfoRec.SubsequentCPUToScreenColorExpand = ApmSubsequentCPUToScreenColorExpand;
  xf86AccelInfoRec.CPUToScreenColorExpandRange = 30*1024;
  xf86AccelInfoRec.ColorExpandFlags = VIDEO_SOURCE_GRANULARITY_PIXEL |
    NO_PLANEMASK | SCANLINE_PAD_DWORD | CPU_TRANSFER_PAD_QWORD
    | BIT_ORDER_IN_BYTE_MSBFIRST | LEFT_EDGE_CLIPPING |
    LEFT_EDGE_CLIPPING_NEGATIVE_X;
#endif


#if 0
  /* Since this code is not used yet, I cannot test or implement it */
  /* Accelerated screen to screen color expansion */
  xf86AccelInfoRec.SetupForScreenToScreenColorExpand =
    ApmSetupForScreenToScreenColorExpand;
  xf86AccelInfoRec.SubsequentScreenToScreenColorExpand =
    ApmSubsequentScreenToScreenColorExpand;
#endif

  /* Accelerated screen-screen bitblts */
  xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK | NO_TRANSPARENCY;
  xf86AccelInfoRec.SetupForScreenToScreenCopy =
    ApmSetupForScreenToScreenCopy;
  xf86AccelInfoRec.SubsequentScreenToScreenCopy =
    ApmSubsequentScreenToScreenCopy;

  /* Accelerated Line drawing */
  xf86AccelInfoRec.SubsequentBresenhamLine = ApmSubsequentBresenhamLine;
  xf86AccelInfoRec.SetClippingRectangle = ApmSetClippingRectangle;
  xf86AccelInfoRec.ErrorTermBits = 15;

  /* Pixmap cache setup */
  xf86AccelInfoRec.PixmapCacheMemoryStart =
    vga256InfoRec.virtualY * vga256InfoRec.displayWidth
    * vga256InfoRec.bitsPerPixel / 8;
  xf86AccelInfoRec.PixmapCacheMemoryEnd =
    vga256InfoRec.videoRam * 1024 - 1024;

#if 0
  if (apmChip == AP6422)
  {
    xf86AccelInfoRec.Sync = ApmSync6422;
    xf86AccelInfoRec.SetupForCPUToScreenColorExpand = NULL;
    xf86AccelInfoRec.SubsequentCPUToScreenColorExpand = NULL;
    xf86AccelInfoRec.SubsequentBresenhamLine = ApmSubsequentBresenhamLine6422;
  }
#endif

}

static void 
ApmSetupForFillRectSolid(int color, int rop, unsigned int planemask)
{
  ApmCheckMMIO_InitFast();
  WaitForFifo(3);
  SETCLIP_CTRL(0);
  SETFOREGROUNDCOLOR(color);
  SETROP(apmROP[rop]);
}

static void 
ApmSubsequentFillRectSolid(int x, int y, int w, int h)
{
  u32 c;

  WaitForFifo(3);
  SETDESTXY(x,y);
  SETWIDTHHEIGHT(w,h);
  SETDEC(DEC_START | DEC_OP_RECT | apmScreenWidth_DEC | apmBitsPerPixel_DEC);
}

static void 
ApmSetupForScreenToScreenCopy(int xdir, int ydir, int rop, unsigned int planemask,
                              int transparency_color)
{
  ApmCheckMMIO_InitFast();
  blitxdir = xdir;
  blitydir = ydir;
  WaitForFifo(2);
  SETCLIP_CTRL(0);
  SETROP(apmROP[rop]);
}


static void 
ApmSubsequentScreenToScreenCopy(int x1, int y1, int x2, int y2, int w, int h)
{
  u32 c = 0;
  u32 sx, dx, sy, dy;

  WaitForFifo(4);

  if (blitxdir < 0)
  {
    c |= DEC_DIR_X_NEG;
    sx = x1+w-1;
    dx = x2+w-1;
  }
  else
  {
    c |= DEC_DIR_X_POS;
    sx = x1;
    dx = x2;
  }

  if (blitydir < 0)
  {
    c |= DEC_DIR_Y_NEG;
    sy = y1+h-1;
    dy = y2+h-1;
  }
  else
  {
    c |= DEC_DIR_Y_POS;
    sy = y1;
    dy = y2;
  }

  SETSOURCEXY(sx,sy);
  SETDESTXY(dx,dy);
  SETWIDTHHEIGHT(w,h);

  SETDEC(DEC_START | DEC_OP_BLT | c | apmScreenWidth_DEC | apmBitsPerPixel_DEC);

}

#if 0
static void 
ApmSetupForCPUToScreenColorExpand(int bg, int fg, int rop, unsigned int planemask)
{
  ApmCheckMMIO_InitFast();
  WaitForFifo(3);
  if (bg == -1)
  {
    SETFOREGROUNDCOLOR(fg);
    SETBACKGROUNDCOLOR(fg+1);
    apmTransparency = TRUE;
  }
  else
  {
    SETFOREGROUNDCOLOR(fg);
    SETBACKGROUNDCOLOR(bg);
    apmTransparency = FALSE;
  }
  SETROP(apmROP[rop]);
}

static void 
ApmSubsequentCPUToScreenColorExpand(int x, int y, int w, int h, int skipleft)
{
  u32 c;
  WaitForFifo(7);

  SETCLIP_LEFTTOP(x+skipleft, y);
  SETCLIP_RIGHTBOT(x+w-1, y+h-1);
  SETCLIP_CTRL(0x01);
  SETSOURCEX(0); /* According to manual, it just has to be zero */
  SETDESTXY(x, y);
  SETWIDTHHEIGHT((w + 31) & ~31, h);

  c = DEC_OP_HOSTBLT_HOST2SCREEN | DEC_SOURCE_LINEAR | DEC_SOURCE_CONTIG | DEC_SOURCE_MONOCHROME;

  if (apmTransparency)
    c |= DEC_SOURCE_TRANSPARENCY;

  SETDEC(DEC_START | c | apmScreenWidth_DEC | apmBitsPerPixel_DEC);
}
#endif


static void 
ApmSetupForScreenToScreenColorExpand(int bg, int fg, int rop,
                                     unsigned int planemask)
{
  /* To be written... */
}

static void 
ApmSubsequentScreenToScreenColorExpand(int srcx, int srcy, int x, 
                                       int y, int w, int h)
{
  /* To be written... */
}

static void 
ApmSubsequentBresenhamLine(int x1, int y1, int octant, int err, int e1, int e2, int length)
{
  u32 c = 0;

  WaitForFifo(5);
  SETDESTXY(x1,y1);
  SETWIDTH(length);
  SETDDA_ERRORTERM(err);
  SETDDA_ADSTEP(e1,e2);

  if (octant & XDECREASING)
    c |= DEC_DIR_X_NEG;
  else
    c |= DEC_DIR_X_POS;

  if (octant & YDECREASING)
    c |= DEC_DIR_Y_NEG;
  else
    c |= DEC_DIR_Y_POS;
    
  if (octant & YMAJOR)
    c |= DEC_MAJORAXIS_Y;
  else
    c |= DEC_MAJORAXIS_X;

  SETDEC(DEC_START | DEC_OP_VECT_ENDP | c | apmScreenWidth_DEC | apmBitsPerPixel_DEC);

  if (apmClip)
  {
    WaitForFifo(1);
    apmClip = FALSE;
    SETCLIP_CTRL(0);
  }    
}

static void 
ApmSetClippingRectangle(int x1, int y1, int x2, int y2)
{
  WaitForFifo(3);
  SETCLIP_LEFTTOP(x1,y1);
  SETCLIP_RIGHTBOT(x2,y2);
  SETCLIP_CTRL(0x01);
  apmClip = TRUE;
}

/* This function is a f*cking kludge since I could not get MMIO to
   work if I initialized in one of the functions in apm_driver.c (like
   preferrably ApmFbInit()... */
void
ApmCheckMMIO_Init(void)
{
  if (!apmMMIO_Init)
  {
    apmMMIO_Init = TRUE;
    wrinx(0x3C4, 0x1b, 0x24); /* Enable memory mapping */

    apmRegBase = (u8*)vgaLinearBase + APM.ChipLinearSize - 2*1024;

    if (apmChip == AP6422)
      ApmSync6422();
    else
      ApmSync();

    switch(vga256InfoRec.bitsPerPixel)
    {
      case 8:
           apmBitsPerPixel_DEC = DEC_BITDEPTH_8;
           break;
      case 16:
           apmBitsPerPixel_DEC = DEC_BITDEPTH_16;
           break;
      case 24:
           apmBitsPerPixel_DEC = DEC_BITDEPTH_24;
           break;
      case 32:
           apmBitsPerPixel_DEC = DEC_BITDEPTH_32;
           break;
      default:
           ErrorF("Cannot set up drawing engine control for bpp = %d\n", 
                  vga256InfoRec.bitsPerPixel);
           break;
    }

    if (apmChip == AP6422)
      apmBitsPerPixel_DEC = DEC_BITDEPTH_COMPAT;
    
    switch(vga256InfoRec.displayWidth)
    {
      case 640:
           apmScreenWidth_DEC = DEC_WIDTH_640;
           break;
      case 800:
           apmScreenWidth_DEC = DEC_WIDTH_800;
           break;
      case 1024:
           apmScreenWidth_DEC = DEC_WIDTH_1024;
           break;
      case 1152:
           apmScreenWidth_DEC = DEC_WIDTH_1152;
           break;
      case 1280:
           apmScreenWidth_DEC = DEC_WIDTH_1280;
           break;
      case 1600:
           apmScreenWidth_DEC = DEC_WIDTH_1600;
           break;
      default:
           ErrorF("Cannot set up drawing engine control for screen width = %d\n", vga256InfoRec.displayWidth);
           break;
    }


    SETBYTEMASK(0xff); 

    SETROP(ROP_S);     

#if 0
    xf86AccelInfoRec.CPUToScreenColorExpandBase = (pointer)((u8*)vgaLinearBase + 
      APM.ChipLinearSize - 32*1024);
#endif

    /*Dump(apmRegBase + 0xe8, 8);*/

  }
}

static void 
ApmSync(void) 
{
  volatile u32 i, stat;

  for(i = 0; i < MAXLOOP; i++) { 
    stat = STATUS();
    if ((!(stat & (STATUS_HOSTBLTBUSY | STATUS_ENGINEBUSY))) &&
        ((stat & STATUS_FIFO) >= 8))
      break;
  }
  if (i == MAXLOOP)
    FatalError("Hung in ApmSync()\n");
}


static void
Dump(void* start, u32 len)
{
  u8* i;
  int c = 0;
  ErrorF("Memory Dump. Start 0x%x length %d\n", (u32)start, len);
  for (i = (u8*)start; i < ((u8*)start+len); i++)
  {
    ErrorF("%02x ", *i);
    if (c++ % 25 == 24) 
      ErrorF("\n");
  }
  ErrorF("\n");
}

/************** AP6422 code ***************/

static void 
ApmSubsequentBresenhamLine6422(int x1, int y1, int octant, int err, int e1, int e2, int length)
{
  u32 c = 0;

  WaitForFifo(1);
  SETDESTXY(x1,y1);
  WaitForFifo(4);
  SETWIDTH(length);
  SETDDA_ERRORTERM(err);
  SETDDA_ADSTEP(e1,e2);

  if (octant & XDECREASING)
    c |= DEC_DIR_X_NEG;
  else
    c |= DEC_DIR_X_POS;

  if (octant & YDECREASING)
    c |= DEC_DIR_Y_NEG;
  else
    c |= DEC_DIR_Y_POS;
    
  if (octant & YMAJOR)
    c |= DEC_MAJORAXIS_Y;
  else
    c |= DEC_MAJORAXIS_X;

  SETDEC(DEC_START | DEC_OP_VECT_ENDP | c | apmScreenWidth_DEC | apmBitsPerPixel_DEC);

  if (apmClip)
  {
    WaitForFifo(1);
    apmClip = FALSE;
    SETCLIP_CTRL(0);
  }    
}


static void 
ApmSync6422(void) 
{
  volatile u32 i, stat, j;

  /* This is a kludge. Somehow we can't trust the status register. Don't
     know why... We shouldn't be forced to read the status reg and get
     a correct value more than once... */
  for (j = 0; j < 2; j++)
  {
    for(i = 0; i < MAXLOOP; i++) { 
      stat = STATUS();
      if ((!(stat & (STATUS_HOSTBLTBUSY | STATUS_ENGINEBUSY))) &&
          ((stat & STATUS_FIFO) >= 4))
        break;
    }
  }
  if (i == MAXLOOP)
    FatalError("Hung in ApmSync6422()\n");
}

