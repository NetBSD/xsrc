/*
 * $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nv1accel.c,v 1.1.2.2 1998/12/22 16:33:18 hohndel Exp $
 *
 * Copyright 1996-1997  David J. McKay
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
 * DAVID J. MCKAY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>

#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#include "xf86xaa.h"
#include "miline.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "nvuser.h"
#include "nvreg.h"

#ifdef DEBUG
#define CHECK()
#else
#define CHECK()
#endif

/* These are the subchannels where the various objects are */

#define ROP_SUBCHANNEL           0
#define BLIT_SUBCHANNEL          1
#define PATTERN_SUBCHANNEL       2
#define RECT_SUBCHANNEL          3
#define CLIP_SUBCHANNEL          4
#define COLOUR_EXPAND_SUBCHANNEL 5
#define GLYPH_SUBCHANNEL         5
#define LINE_SUBCHANNEL          6
#define TRI3D_SUBCHANNEL         6
#define LIN_SUBCHANNEL           7

extern void * pfb;

/* This is the pointer to the channel */
static NvChannel *chan0=NULL;
/* Pointers to fixed subchannels */
static NvSubChannel *ropChan=NULL;
static NvSubChannel *rectChan=NULL;
static NvSubChannel *blitChan=NULL;
static NvSubChannel *clipChan=NULL;
static NvSubChannel *colourExpandChan=NULL;

/*#define NV_ENABLE_LINES*/

/* Lines are disables at the moment. I have been unable to get the
 * lines produced by the NV1 hardware to match the software routines.
 * The NV1 draws lines a la windows, which is different to what X wants
 * Even fiddling with the zero bias does not get it correct. Will need a
 * lot of work to get it to work
 */
#ifdef NV_ENABLE_LINES
static NvSubChannel *lineChan=NULL;
static NvSubChannel *linChan=NULL;
#endif

/* Holds number of free slots in fifo. Means we don't have to re-read
 * the fifo count every time we want to write to the chip.  Should do something useful if chip
 * is busy (like keep the cursor updated). Calling sched_yield() is nice for the other apps running,
 * but it really makes the server sluggish.
 */
int freeSlots=0;

#define WaitForSlots(n,words) \
  while((freeSlots-=((words)*4))<0){\
    freeSlots=chan0->subChannel[n].control.free;\
  }

static int currentRop=-1;

static void NVSetRop(int rop)
{
    static int ropTrans[16] = {
        0x0,  /* GXclear */
        0x88, /* Gxand */
        0x44, /* GXandReverse */
        0xcc, /* GXcopy */
        0x22, /* GXandInverted */
        0xaa, /* GXnoop */
        0x66, /* GXxor */
        0xee, /* GXor */
        0x11, /* GXnor */
        0x99, /* GXequiv */
        0x55, /* GXinvert */
        0xdd, /* GXorReverse */
        0x33, /* GXcopyInverted */
        0xbb, /* GXorInverted */
        0x77, /* GXnand */
        0xff  /* GXset */
    };
    currentRop=rop;
    WaitForSlots(ROP_SUBCHANNEL,1);
    ropChan->method.ropSolid.setRop=ropTrans[rop];
    CHECK();
}

/* I haven't figured out how Patterns work yet, so this code is
 * not used at present
 */
#if 0
static NVSetPatternRop(int rop)
{
    static int ropTrans[16]={
        0x00, /* GXclear */
        0xa0, /* Gxand */
        0x0a, /* GXandReverse */
        0xf0, /* GXcopy */
        0x30, /* GXandInverted */
        0xaa, /* GXnoop */
        0x3a, /* GXxor */
        0xfa, /* GXor */
        0x03, /* GXnor */
        0xa0, /* Gxequiv */
        0x0f, /* GXinvert */
        0xaf, /* GXorReverse */
        0x33, /* GXcopyInverted */
        0xbb, /* GXorInverted */
        0xf3, /* GXnand */
        0xff  /* GXset */
    };
    currentRop=rop+16; /* +16 is important */
    WaitForSlots(ROP_SUBCHANNEL,1);
    ropChan->method.ropSolid.setRop=ropTrans[rop];
    CHECK();

}

#endif

/*
 * Due to the fact that the SetupForFillRectSolid() is also used
 * to setup for lines we have to record the colour in a static
 * variable and write it out every time we do a drawing operation.
 * This costs some performance and is IMHO wrong anyway. Why should
 * you call a routine for filled rectangles when you are drawing lines?
 * Will get round to "fixing" this if I ever get lines on the NV1 to work
 * correctly
 */

#if 1
static int currentColor;
#endif

static void NVSetupForFillRectSolid(int color,int rop,unsigned planemask)
{
    if (currentRop!=rop)
    {
        NVSetRop(rop);
    }
#if 1
    currentColor=color;
#else
    WaitForSlots(RECT_SUBCHANNEL,1);
    rectChan->method.renderSolidRectangle.color=color;
#endif

    CHECK();
}

static void NVSubsequentFillRectSolid(int x,int y,int w,int h)
{

#if 1
    WaitForSlots(RECT_SUBCHANNEL,3);
    rectChan->method.renderSolidRectangle.color=currentColor;
#else
    WaitForSlots(RECT_SUBCHANNEL,2);
#endif

    rectChan->method.renderSolidRectangle.rectangle[0].yx=PACK_UINT16(y,x);
    rectChan->method.renderSolidRectangle.rectangle[0].heightWidth=
    PACK_UINT16(h,w);
    CHECK();
}




static int clippingOn=0;

static void NVSetClippingRectangle(int x1,int y1,int x2,int y2)
{
    int height,width;

    width=x2-x1+1;height=y2-y1+1;
    WaitForSlots(CLIP_SUBCHANNEL,2);
    clipChan->method.clip.setRectangle.yx=PACK_INT16(y1,x1);
    clipChan->method.clip.setRectangle.heightWidth=PACK_UINT16(height,width);
    clippingOn=1;
    CHECK();
}


#define NVResetClippingRectangle() \
  { NVSetClippingRectangle(0,0,MAX_INT16,MAX_INT16);\
  clippingOn=0;}

#ifdef NV_ENABLE_LINES
static void NVSubsequentTwoPointLine(int x1,int y1,int x2,int y2,int bias)
{
    NvRenderSolidLine *line;

    line = (bias&0x0100) ?  &(linChan->method.line) : &(lineChan->method.line);
    /* We should really check appropriate subchannel here */
    WaitForSlots(LINE_SUBCHANNEL,3);
    line->color=currentColor;
    line->line[0].y0_x0=PACK_INT16(y1,x1);
    line->line[0].y1_x1=PACK_INT16(y2,x2);
    /* Reset clipping rectangle to normal */
    if (clippingOn)
    {
        NVResetClippingRectangle();
    }
}

#endif

static void NVSetupForScreenToScreenCopy(int xdir,int ydir,int rop,
                                         unsigned planemask,
                                         int transparency_color)
{
    if (rop!=currentRop)
    {
        NVSetRop(rop);
    }

    /* When transparency is implemented, will have to flip object */
}

static void NVSubsequentScreenToScreenCopy(int x1,int y1,
                                           int x2,int y2,int w,int h)
{
    WaitForSlots(BLIT_SUBCHANNEL,3);
    blitChan->method.blit.yxIn=PACK_UINT16(y1,x1);
    blitChan->method.blit.yxOut=PACK_UINT16(y2,x2);
    blitChan->method.blit.heightWidth=PACK_UINT16(h,w);
    CHECK();
}


/* How much date to transfer */
static int scanlineWordCount;
static unsigned char scratchBuffer[512];
static int colourExpandMask=0;

static void NVSetupForScanlineScreenToScreenColorExpand(int x,int y,int w,
                                                        int h,int bg,int fg,
                                                        int rop,
                                                        unsigned planemask)
{
    if (currentRop!=rop)
    {
        NVSetRop(rop);
    }
    WaitForSlots(COLOUR_EXPAND_SUBCHANNEL,5);
    if (bg==-1)
    {
        colourExpandChan->method.imageMonochromeFromCpu.color0=0;
    }
    else
    {
        colourExpandChan->method.imageMonochromeFromCpu.color0=bg|colourExpandMask;
    }
    colourExpandChan->method.imageMonochromeFromCpu.color1=fg|colourExpandMask;
    colourExpandChan->method.imageMonochromeFromCpu.point=PACK_UINT16(y,x);
    colourExpandChan->method.imageMonochromeFromCpu.size=PACK_UINT16(h,w);
    colourExpandChan->method.imageMonochromeFromCpu.sizeIn=
    PACK_UINT16(h,(w+31)&(~31));
    scanlineWordCount = (w + 31) >> 5;
    CHECK();
}

static void NVSubsequentScanlineScreenToScreenColorExpand(int srcAddr)
{
    unsigned long *ptr = (unsigned long*)scratchBuffer;
    int count = scanlineWordCount;
    int i=0;

    /* This rather simple algorithm seems to perform better than
     * the more complex variants with loop unrolling that I have tried
     */
    for (i=0;i<count;i++)
    {
        WaitForSlots(COLOUR_EXPAND_SUBCHANNEL,1);
        colourExpandChan->method.imageMonochromeFromCpu.monochrome[i%32]=(*(ptr++));
    }
    CHECK();
}
/*
 * Subchannel initialization.
 */
static void SetupSubChans(void)
{
    /* Map subchannels */
    ropChan=&(chan0->subChannel[ROP_SUBCHANNEL]);
    rectChan=&(chan0->subChannel[RECT_SUBCHANNEL]);
    blitChan=&(chan0->subChannel[BLIT_SUBCHANNEL]);
    clipChan=&(chan0->subChannel[CLIP_SUBCHANNEL]);

#ifdef NV_ENABLE_LINES
    lineChan=&(chan0->subChannel[LINE_SUBCHANNEL]);
    linChan=&(chan0->subChannel[LIN_SUBCHANNEL]);
#endif

    /* Bung the appropriate objects into the subchannels */
    WaitForSlots(ROP_SUBCHANNEL,1);
    ropChan->control.object=ROP_OBJECT_ID;
    WaitForSlots(BLIT_SUBCHANNEL,1);
    blitChan->control.object=BLIT_OBJECT_ID;
    WaitForSlots(RECT_SUBCHANNEL,1);
    rectChan->control.object=RECT_OBJECT_ID;
    WaitForSlots(CLIP_SUBCHANNEL,1);
    clipChan->control.object=CLIP_OBJECT_ID;
    colourExpandChan=&(chan0->subChannel[COLOUR_EXPAND_SUBCHANNEL]);
    WaitForSlots(COLOUR_EXPAND_SUBCHANNEL,1);
    colourExpandChan->control.object=COLOUR_EXPAND_OBJECT_ID;

#ifdef NV_ENABLE_LINES
    lineChan->control.object=LINE_OBJECT_ID;
    WaitForSlots(LIN_SUBCHANNEL,1);
    linChan->control.object=LIN_OBJECT_ID;
#endif
    CHECK();
}


/* These should really be in a separate file */
void NV1Sync(void);

int NV1SetupGraphicsEngine(int width,int height,int bpp);

void NVAccelInit(void)
{
    int i;
    int ret;

#ifdef DEBUG
    if (getenv("NV_NOACCEL")) return;
#endif
    ret = NV1SetupGraphicsEngine(vga256InfoRec.virtualX,
                              vga256InfoRec.virtualY,
                              vgaBitsPerPixel);
    if (!ret)
    {
        ErrorF("Failed to init graphics engine - no acceleration\n");
    }
    switch (vgaBitsPerPixel)
    {
        case 8:
        case 32:
            colourExpandMask=0xff000000;
            break;
        case 15:
        case 16:
            colourExpandMask=0xffff8000;
            break;
    }
    CHECK();

    chan0=NvOpenChannel();
    if (chan0==NULL) return;
    SetupSubChans();
    /*
     * Set up default values.
     */
    NVSetRop(3);
    NVResetClippingRectangle();

    /* There are still some problems with delayed syncing */
    xf86AccelInfoRec.Flags = BACKGROUND_OPERATIONS/*| DELAYED_SYNC*/;
    xf86AccelInfoRec.Sync = NV1Sync;
    /*
     * Install the low-level functions for drawing solid filled rectangles.
     */
    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK | NO_TRANSPARENCY;
    xf86AccelInfoRec.SetupForFillRectSolid = NVSetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = NVSubsequentFillRectSolid;

    xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK | NO_TRANSPARENCY;
    xf86AccelInfoRec.SetupForScreenToScreenCopy = NVSetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy = NVSubsequentScreenToScreenCopy;
#ifdef DEBUG
    if (getenv("NV_NOCOLOUREXPAND")==NULL)
#endif
    /* Colour Expansion */
    xf86AccelInfoRec.Flags|=NO_SYNC_AFTER_CPU_COLOR_EXPAND |
                            COP_FRAMEBUFFER_CONCURRENCY;

    xf86AccelInfoRec.ColorExpandFlags = /*NO_TRANSPARENCY | */
                                        NO_PLANEMASK |
                                        SCANLINE_PAD_DWORD |
                                        CPU_TRANSFER_PAD_DWORD |
                                        BIT_ORDER_IN_BYTE_LSBFIRST |
                                        VIDEO_SOURCE_GRANULARITY_DWORD;

    xf86AccelInfoRec.ScratchBufferAddr = 1;
    xf86AccelInfoRec.ScratchBufferSize = 1024;
    xf86AccelInfoRec.ScratchBufferBase = (void*)scratchBuffer;
    xf86AccelInfoRec.PingPongBuffers = 1;

    xf86AccelInfoRec.SetupForScanlineScreenToScreenColorExpand =
    NVSetupForScanlineScreenToScreenColorExpand;
    xf86AccelInfoRec.SubsequentScanlineScreenToScreenColorExpand =
    NVSubsequentScanlineScreenToScreenColorExpand;
#ifdef NV_ENABLE_LINES
#ifdef DEBUG
    if (getenv("NV_NOLINES")==NULL)
#endif
    {
        extern int nvMiLineZeroBias;


#ifdef DEBUG
        ErrorF("NV_ZEROBIAS is %s\n",getenv("NV_ZEROBIAS"));

        if (getenv("NV_ZEROBIAS")!=NULL)
        {
            nvMiLineZeroBias=atoi((char*)getenv("NV_ZEROBIAS"));
            ErrorF("Setting bias to %d\n",nvMiLineZeroBias);
        }
#endif

        /* Lines and lins */
        xf86AccelInfoRec.Flags|=HARDWARE_CLIP_LINE|
                                USE_TWO_POINT_LINE|
                                TWO_POINT_LINE_NOT_LAST;
        xf86AccelInfoRec.SubsequentTwoPointLine = NVSubsequentTwoPointLine;
        xf86AccelInfoRec.SetClippingRectangle =  NVSetClippingRectangle;
    }
#endif
    /*
     * Finally, we set up the video memory space available to the pixmap
     * cache. In this case, all memory from the end of the virtual screen
     * to the end of video memory minus 13K, can be used.
     */
#ifdef DEBUG
    if (getenv("NV_NOPIXMAPCACHE")==NULL)
#endif
    {
        xf86AccelInfoRec.Flags|= PIXMAP_CACHE;
        xf86InitPixmapCache( &vga256InfoRec,
                             vga256InfoRec.virtualY * vga256InfoRec.displayWidth *
                             vga256InfoRec.bitsPerPixel / 8,
                             (vga256InfoRec.videoRam-65/*(NvKbRamUsedByHW()+1)*/)*1024);
    }
}


