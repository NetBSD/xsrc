/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3sav_accel.c,v 1.1.2.1 1999/07/30 11:21:31 hohndel Exp $ */

/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */

/*
 * The accel file for the ViRGE driver.  
 * 
 * Created 20/03/97 by Sebastien Marineau
 * Revision: 
 *
 * Note: we use a few macros to query the state of the coprocessor. 
 * WaitIdle() waits until the GE is idle.
 * WaitIdleEmpty() waits until the GE is idle and its FIFO is empty.
 * WaitCommandEmpty() waits until the command FIFO is empty. The command FIFO
 *       is what handles direct framebuffer writes. We should call this 
 *       before starting any GE functions to make sure that there are no
 *       framebuffer writes left in the FIFO. 
 */

#include <math.h>
#include "xf86.h"
#include "xf86_OSlib.h"
#include "vga256.h"
#include "vga.h"
#include "xf86xaa.h"
#include "xf86Priv.h"
#include "regs3sav.h"
#include "s3sav_driver.h"
#include "s3sav_rop.h"
#include "s3bci.h"
#include "miline.h"

extern S3VPRIV s3vPriv;

/* Globals used in driver */
extern pointer s3vMmioMem;
static int s3DummyTransferArea;
static int s3LineHWClipSet = 0;

#define BCI_REPEAT 0
static unsigned int * bci_base;
static unsigned int   s3SavedBciCmd = 0;
static unsigned int   s3SavedGbdOffset = 0;
static unsigned int   s3SavedGbd = 0;
static unsigned int   s3SavedPbdOffset = 0;
static unsigned int   s3SavedPbd = 0;
static unsigned int   s3SavedSbdOffset = 0;
static unsigned int   s3SavedSbd = 0;
static unsigned int   s3SavedFgColor = 0;
static unsigned int   s3SavedBgColor = 0;

/* These are variables which hold cached values for some virge registers.
 * The important thing to remember is that these registers must be always be 
 * set using the "caching" version of the macros.
 */
static unsigned int s3vCached_CMD_SET;
static unsigned int s3vCached_CLIP_LR;
static unsigned int s3vCached_CLIP_TB;
static unsigned int s3vCached_MONO_PATTERN0;
static unsigned int s3vCached_MONO_PATTERN1;
static unsigned int s3vCached_PAT_FGCLR;
static unsigned int s3vCached_PAT_BGCLR;
static unsigned int s3vCached_RWIDTH_HEIGHT;

/* Temporary to see if caching works */
static int s3vCacheHit = 0, s3vCacheMiss = 0;

/* Forward declaration of fucntions used in the driver */
void S3SAVAccelSync();
void S3SAVAccelInit();
void S3SAVSetupForScreenToScreenCopy();
void S3SAVSubsequentScreenToScreenCopy();
void S3SAVSetupForFillRectSolid();
void S3SAVSubsequentFillRectSolid();
void SavageSubsequentBresenhamLine();
void S3SAVSetupForScreenToScreenColorExpand();
void S3SAVSubsequentScreenToScreenColorExpand();
void S3SAVSetupForCPUToScreenColorExpand();
void S3SAVSubsequentCPUToScreenColorExpand();
void S3SAVSetupFor8x8PatternColorExpand();
void S3SAVSubsequent8x8PatternColorExpand();
void S3SAVSetupForFill8x8Pattern();
void S3SAVSubsequentFill8x8Pattern();
void S3SAVSubsequentTwoPointLine();
void S3SAVSetClippingRectangle();
void S3SAVSubsequentFillTrapezoidSolid();
Bool S3SAVROPHasSrc();
Bool S3SAVROPHasDst();
#include "s3bitmap.c"


/* Acceleration init function, sets up pointers to our accelerated functions */

void 
S3SAVAccelInit() 
{
    unsigned long cobIndex;	/* command overflow buffer size index */
    unsigned long cobSize;	/* command overfluw buffer size */
    unsigned long cobOffset;	/* command overflow buffer offset */

/* Set-up our GE command primitive */
    
    if (vgaBitsPerPixel == 8) {
      s3vPriv.PlaneMask = 0xFF;
      }
    else if (vgaBitsPerPixel == 16) {
      s3vPriv.PlaneMask = 0xFFFF;
      }
    else if (vgaBitsPerPixel == 24) {
      s3vPriv.PlaneMask = 0xFFFFFF;
      }
    else if (vgaBitsPerPixel == 32) {
      s3vPriv.PlaneMask = 0xFFFFFFFF;
      }


    /* General acceleration flags */

    xf86AccelInfoRec.Flags = PIXMAP_CACHE |
         BACKGROUND_OPERATIONS |
         COP_FRAMEBUFFER_CONCURRENCY | 
         NO_SYNC_AFTER_CPU_COLOR_EXPAND |
         HARDWARE_PATTERN_BIT_ORDER_MSBFIRST |  
         HARDWARE_PATTERN_PROGRAMMED_BITS |
         HARDWARE_PATTERN_SCREEN_ORIGIN;
    xf86AccelInfoRec.Flags = PIXMAP_CACHE |
         BACKGROUND_OPERATIONS |
         NO_SYNC_AFTER_CPU_COLOR_EXPAND |
         HARDWARE_PATTERN_SCREEN_ORIGIN |
         HARDWARE_PATTERN_TRANSPARENCY;

     xf86AccelInfoRec.Sync = S3SAVAccelSync;


    /* ScreenToScreen copies */

#if 1
    xf86AccelInfoRec.SetupForScreenToScreenCopy =
        S3SAVSetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
        S3SAVSubsequentScreenToScreenCopy;
    xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK;
#endif


    /* Filled rectangles */

#if 1
    xf86AccelInfoRec.SetupForFillRectSolid = 
        S3SAVSetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = 
        S3SAVSubsequentFillRectSolid;
    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;
#endif

    /* ImageWrite no sync required */

#if 1
    xf86AccelInfoRec.ImageWrite =
        SavageImageWrite;
#endif

    /* Bresenham lines */

#if 0
    xf86AccelInfoRec.SubsequentBresenhamLine =
        SavageSubsequentBresenhamLine;
#endif

    xf86AccelInfoRec.ColorExpandFlags = SCANLINE_PAD_DWORD |
					CPU_TRANSFER_PAD_DWORD | 
					VIDEO_SOURCE_GRANULARITY_PIXEL |
					LEFT_EDGE_CLIPPING;
    /* WriteBitmap color expand */

#if 1
    xf86AccelInfoRec.WriteBitmap =
        SavageWriteBitmapCPUToScreenColorExpand;
#endif

#if 1
    xf86AccelInfoRec.SetupForScreenToScreenColorExpand =
             S3SAVSetupForScreenToScreenColorExpand;
    xf86AccelInfoRec.SubsequentScreenToScreenColorExpand =
             S3SAVSubsequentCPUToScreenColorExpand;
#endif

#if 0
    xf86AccelInfoRec.SetupForCPUToScreenColorExpand =
             S3SAVSetupForCPUToScreenColorExpand;
    xf86AccelInfoRec.SubsequentCPUToScreenColorExpand =
             S3SAVSubsequentCPUToScreenColorExpand;
    xf86AccelInfoRec.CPUToScreenColorExpandBase = s3vPriv.BciMem;
             (void *) &IMG_TRANS;
    xf86AccelInfoRec.CPUToScreenColorExpandRange = 128 * 1024;
#endif

 
    /* These are the 8x8 pattern fills using color expansion */

#if 0
    xf86AccelInfoRec.SetupFor8x8PatternColorExpand = 
            S3SAVSetupFor8x8PatternColorExpand;
    xf86AccelInfoRec.Subsequent8x8PatternColorExpand = 
            S3SAVSubsequent8x8PatternColorExpand;  
#endif


    /* These are the 8x8 color pattern fills */

#if 0
    xf86AccelInfoRec.SetupForFill8x8Pattern = 
            S3SAVSetupForFill8x8Pattern;
    xf86AccelInfoRec.SubsequentFill8x8Pattern = 
            S3SAVSubsequentFill8x8Pattern; 
#endif


    /* These are the accelerated line functions */
    /* They are only semi-functionnal and do not work fully yet */

/*    xf86AccelInfoRec.SubsequentTwoPointLine = 
      S3SAVSubsequentTwoPointLine;
    xf86AccelInfoRec.SetClippingRectangle = 
            S3SAVSetClippingRectangle;
    xf86AccelInfoRec.SubsequentFillTrapezoidSolid = 
            S3SAVSubsequentFillTrapezoidSolid;   */


    /*
     * Finally, we set up the video memory space available to the pixmap
     * cache. In this case, all memory from the end of the virtual screen
     * to the end of the command overflow buffer can be used. If you haven't
     * enabled the PIXMAP_CACHE flag, then these lines can be omitted.
     */

     if( s3vPriv.chip == S3_SAVAGE4 ) {
	 cobIndex = 2 /*4*/;
	 cobSize = 0x8000 << cobIndex;
     }
     else {
         cobIndex = 7;
	 cobSize = 0x400 << cobIndex;
     }

     /* The command overflow buffer must be placed at the end of RAM. */
     cobOffset = (vga256InfoRec.videoRam << 10) - cobSize;

     xf86InitPixmapCache(&vga256InfoRec, vga256InfoRec.virtualY *
	vga256InfoRec.displayWidth * vga256InfoRec.bitsPerPixel / 8,
	cobOffset);

     /* And these are screen parameters used to setup the GE */

     s3vPriv.Width = vga256InfoRec.displayWidth;
     s3vPriv.Bpp = vgaBitsPerPixel / 8;
     s3vPriv.Bpl = s3vPriv.Width * s3vPriv.Bpp;
     s3vPriv.ScissB = (vga256InfoRec.videoRam * 1024 - 4096 - cobSize) / s3vPriv.Bpl;
     if (s3vPriv.ScissB > 2047)
         s3vPriv.ScissB = 2047;

#define S3_IN32(off) (*(unsigned int*)(((char*)s3vMmioMem)+(off)))
#define S3_OUT32(off, val) (*(unsigned int*)(((char*)s3vMmioMem)+(off)) = val)
     /* Setup plane masks */
     S3_OUT32(0x8128, ~0); /* enable all write planes */
     S3_OUT32(0x812C, ~0); /* enable all read planes */
     S3_OUT32(0x8136, 0x27);
     S3_OUT32(0x8136, 0x07);

     {
	 unsigned long cobWater;

	 /* Sav4 doc shows the smaller value in the upper DWORD. */
	 /* However, that's not what the NT driver does. */
	 if( s3vPriv.chip == S3_SAVAGE4 ) {
	     cobWater = (cobSize >> 7) * 0x10001 - 0x01400300;
	 }
	 else {
	     cobWater = (cobSize >> 2) * 0x10001 - 0x01C00EC0;
	 }

	 /* Disable BCI */
	 S3_OUT32(0x48C18, S3_IN32(0x48C18) & 0x3FF0);
	 /* Disable shadow status update */
	 S3_OUT32(0x48C0C, 0);
	 /* Setup BCI command overflow buffer */
	 S3_OUT32(0x48C14, (cobOffset >> 11) | (cobIndex << 29));
	 /* Command overflow buffer HI/LOW watermarks */
	 S3_OUT32(0x48C10, cobWater);
	 /* Enable BCI and command overflow buffer */
	 S3_OUT32(0x48C18, S3_IN32(0x48C18) | 0x0C);
     }

     {
         unsigned int gbd = 1 | BCI_BD_BW_DISABLE;

         BCI_BD_SET_BPP(gbd, vgaBitsPerPixel);
         BCI_BD_SET_STRIDE(gbd, s3vPriv.Width);
         /* Use and set global bitmap descriptor */
         s3SavedGbd = gbd & ~1;
         outb(vgaCRIndex, 0x50);
         outb(vgaCRReg, inb(vgaCRIndex) | 0xC1);
         S3_OUT32(0x8168, 0);
         S3_OUT32(0x816C, gbd);
         S3_OUT32(0x8170, 0);
         S3_OUT32(0x8174, gbd);
         S3_OUT32(0x8178, 0);
         S3_OUT32(0x817C, gbd);
     }
} 


/* The sync function for the GE */
void
S3SAVAccelSync()
{
#if 0
    BCI_GET_PTR;

    WaitQueue(3);
    BCI_SEND(BCI_CMD_NOP | BCI_CMD_CLIP_NEW);
    BCI_SEND(BCI_CLIP_TL(0, 0));
    BCI_SEND(BCI_CLIP_BR(s3vPriv.ScissB, s3vPriv.Width));
#endif

    WaitCommandEmpty();
    WaitIdleEmpty();
}


/* This next function performs a reset of the graphics engine and 
 * fills in some GE registers with default values.                  
 */

void
S3SAVGEReset(int from_timeout, int line, char *file)
{
    unsigned char tmp;
    int r;
    int32  fifo_control, miu_control, streams_timeout, misc_timeout;

    if (from_timeout) {
      static int n=0;
      if (n++ < 10 || xf86Verbose > 1)
	ErrorF("\tS3VGEReset called from %s line %d\n",file,line);
    }
    else
      WaitIdleEmpty();


    if (from_timeout) {
      /* reset will trash these registers, so save them */
      fifo_control    = ((mmtr)s3vMmioMem)->memport_regs.regs.fifo_control;
      miu_control     = ((mmtr)s3vMmioMem)->memport_regs.regs.miu_control;
      streams_timeout = ((mmtr)s3vMmioMem)->memport_regs.regs.streams_timeout;
      misc_timeout    = ((mmtr)s3vMmioMem)->memport_regs.regs.misc_timeout;
    }

    outb(vgaCRIndex, 0x66);
    tmp = inb(vgaCRReg);

    usleep(10000);
    for (r=1; r<10; r++) {  /* try multiple times to avoid lockup of ViRGE/MX */
      outb(vgaCRReg, tmp | 0x02);
      usleep(10000);
      outb(vgaCRReg, tmp & ~0x02);
      usleep(10000);

      if (!from_timeout) 
	WaitIdleEmpty();
      SETB_DEST_SRC_STR(s3vPriv.Bpl, s3vPriv.Bpl); 

      usleep(10000);
      if ((ALT_STATUS_WORD0 & s3vPriv.StatusMask) != s3vPriv.Status2DI) {
        usleep(10000);
	ErrorF("restarting S3 graphics engine reset %2d ...\n",r);
      }
      else
	break;
    }

    if (from_timeout) {
      /* restore trashed registers */
      ((mmtr)s3vMmioMem)->memport_regs.regs.fifo_control    = fifo_control;
      ((mmtr)s3vMmioMem)->memport_regs.regs.miu_control     = miu_control;
      ((mmtr)s3vMmioMem)->memport_regs.regs.streams_timeout = streams_timeout;
      ((mmtr)s3vMmioMem)->memport_regs.regs.misc_timeout    = misc_timeout;
    }

    SETB_SRC_BASE(0);
    SETB_DEST_BASE(0);   

    /* Now write some default rgisters and reset cached values */
    s3vCached_CLIP_LR = -1;
    s3vCached_CLIP_TB = -1;
    CACHE_SETB_CLIP_L_R(0, s3vPriv.Width);
    CACHE_SETB_CLIP_T_B(0, s3vPriv.ScissB);
    s3vCached_MONO_PATTERN0 = 0;
    s3vCached_MONO_PATTERN1 = 0;
    CACHE_SETB_MONO_PAT0(~0);
    CACHE_SETB_MONO_PAT1(~0);   

    s3vCached_RWIDTH_HEIGHT = -1;
    s3vCached_PAT_FGCLR = -1;
    s3vCached_PAT_BGCLR = -1;
    s3vCached_CMD_SET = -1;
    if (xf86Verbose > 1)
        ErrorF("ViRGE register cache hits: %d misses: %d\n",
            s3vCacheHit, s3vCacheMiss);    
    s3vCacheHit = 0; s3vCacheMiss = 0;

#if 0
     /* Disable BCI */
     S3_OUT32(0x48C18, S3_IN32(0x48C18) & 0x3FF7);
     /* Disable shadow status update */
     S3_OUT32(0x48C0C, 0);

     if( s3vPriv.chip == S3_SAVAGE4 ) {
	 /* Setup BCI command overflow buffer, reserve 1k for cursor */
	 S3_OUT32(0x48C14, (0x800000 - 0x7FFFE - 0x400) >> 11 | 0x80000000);
	 /* Enable BCI and command overflow buffer */
	 S3_OUT32(0x48C18, S3_IN32(0x48C18) | 0x0C);
	 /* Command overflow buffer HI/LOW watermarks */
	 S3_OUT32(0x48C10, 0x03C10391);
     }
     else {
	 /* Setup BCI command overflow buffer, reserve 1k for cursor */
	 S3_OUT32(0x48C14, (0x800000 - 0x1FFFE - 0x400) >> 11 | 0xE0000000);
	 /* Enable BCI and command overflow buffer */
	 S3_OUT32(0x48C18, S3_IN32(0x48C18) | 0x0C);
	 /* Command overflow buffer HI/LOW watermarks */
	 S3_OUT32(0x48C10, 0x78207220);
     }

     {
         unsigned int gbd = 1 | BCI_BD_BW_DISABLE;

         BCI_BD_SET_BPP(gbd, vgaBitsPerPixel);
         BCI_BD_SET_STRIDE(gbd, s3vPriv.Width);
         /* Use and set global bitmap descriptor */
         outb(vgaCRIndex, 0x50);
         outb(vgaCRReg, inb(vgaCRIndex) | 0xC1);
         S3_OUT32(0x8168, 0);
         S3_OUT32(0x816C, gbd);
         S3_OUT32(0x8170, 0);
         S3_OUT32(0x8174, gbd);
         S3_OUT32(0x8178, 0);
         S3_OUT32(0x817C, gbd);
     }
#endif
}



/* These are the ScreenToScreen bitblt functions. We support all ROPs, all
 * directions, and a planemask by adjusting the ROP and using the mono pattern
 * registers. There is no support for transparency. 
 */

void 
S3SAVSetupForScreenToScreenCopy(xdir, ydir, rop, planemask,
transparency_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int transparency_color;
{
    BCI_GET_PTR;
    int cmd;

    cmd = BCI_CMD_RECT | BCI_CMD_DEST_GBD | BCI_CMD_SRC_GBD;
    cmd |= s3vAlu[rop];
    if (transparency_color != -1)
        cmd |= BCI_CMD_SEND_COLOR | BCI_CMD_SRC_TRANSPARENT;

    if (xdir == 1 ) cmd |= BCI_CMD_RECT_XP;
    if (ydir == 1 ) cmd |= BCI_CMD_RECT_YP;
    s3SavedBciCmd = cmd;

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(cmd);
    if (transparency_color != -1) {
        BCI_SEND(transparency_color);
	ErrorF("CopyRect transparency\n");
    }
    /*ErrorF("CopyRect command 0x%.8x sent\n", cmd);*/
#endif
    s3SavedBciCmd = cmd;
    s3SavedBgColor = transparency_color;
}

void 
S3SAVSubsequentScreenToScreenCopy(x1, y1, x2, y2, w, h)
int x1, y1, x2, y2, w, h;
{
    BCI_GET_PTR;

    if (!w || !h) return;
    if (!(s3SavedBciCmd & BCI_CMD_RECT_XP)) {
        w --;
        x1 += w;
        x2 += w;
        w ++;
    }
    if (!(s3SavedBciCmd & BCI_CMD_RECT_YP)) {
        h --;
        y1 += h;
        y2 += h;
        h ++;
    }

#if BCI_REPEAT
    WaitQueue(3);
    if (s3SavedBgColor != -1) BCI_SEND(s3SavedBgColor);
    BCI_SEND(BCI_X_Y(x1, y1));
    BCI_SEND(BCI_X_Y(x2, y2));
    BCI_SEND(BCI_W_H(w, h));
#else
    WaitQueue(5);
    BCI_SEND(s3SavedBciCmd);
    if (s3SavedBgColor != -1) BCI_SEND(s3SavedBgColor);
    BCI_SEND(BCI_X_Y(x1, y1));
    BCI_SEND(BCI_X_Y(x2, y2));
    BCI_SEND(BCI_W_H(w, h));
#endif
}


/*
 *
 */ 

void 
S3SAVSetupForFillRectSolid(color, rop, planemask)
int color, rop;
unsigned planemask;
{
    BCI_GET_PTR;
    int cmd;

    cmd = BCI_CMD_RECT | BCI_CMD_SEND_COLOR
        | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_DEST_GBD | BCI_CMD_SRC_SOLID;
    cmd |= s3vAlu[rop];

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(cmd);
    /*ErrorF("FillRect command 0x%.8x sent\n", cmd);*/
    BCI_SEND(color);
#endif
    s3SavedBciCmd = cmd;
    s3SavedFgColor = color;
}
    
    
void 
S3SAVSubsequentFillRectSolid(x, y, w, h)
int x, y, w, h;
{
    BCI_GET_PTR;

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#else
    WaitQueue(4);
    BCI_SEND(s3SavedBciCmd);
    BCI_SEND(s3SavedFgColor);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#endif
}

void
SavageSubsequentBresenhamLine(x1, y1, octant, err, e1, e2, length)
    int x1, y1, octant, err, e1, e2, length;
{
    BCI_GET_PTR;
    unsigned int cmd;

    cmd = BCI_CMD_LINE | BCI_CMD_LINE_LAST_PIXEL;

    WaitQueue(4);
    BCI_SEND(cmd);
    BCI_SEND(BCI_LINE_X_Y(x1, y1));
    BCI_SEND(BCI_LINE_STEPS(e2, e1));
    BCI_SEND(BCI_LINE_MISC(length, (octant & YMAJOR),
			   !(octant & XDECREASING),
			   !(octant & YDECREASING),
			   err));
}

void
S3SAVSetupForScreenToScreenColorExpand(bg, fg, rop, planemask)
int bg, fg, rop;
unsigned planemask;
{
}

void
S3SAVSubsequentScreenToScreenColorExpand(x, y, w, h, skipleft)
int x, y, w, h, skipleft;
{
}

void
S3SAVSetupForCPUToScreenColorExpand(bg, fg, rop, planemask)
int bg, fg, rop;
unsigned planemask;
{
    BCI_GET_PTR;
    int cmd;

    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_SEND_COLOR
        | BCI_CMD_DEST_GBD | BCI_CMD_SRC_MONO;
    cmd |= s3vAlu[rop];

    if (bg != -1)
        cmd |= BCI_CMD_SEND_COLOR;
    else cmd |= BCI_CMD_SRC_TRANSPARENT;

#if BCI_REPEAT    
    WaitQueue(3);
    BCI_SEND(cmd);
    BCI_SEND(fg);
    if(bg != -1) BCI_SEND(bg);
    s3SavedBciCmd = cmd;
#endif
    s3SavedBciCmd = cmd;
    s3SavedFgColor = fg;
    s3SavedBgColor = bg;
}

void
S3SAVSubsequentCPUToScreenColorExpand(x, y, w, h, skipleft)
int x, y, w, h, skipleft;
{
    BCI_GET_PTR;

#if BCI_REPEAT
    WaitQueue(4);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
    BCI_SEND(((w + 31) / 32) * h);
#else
    WaitQueue(7);
    if ( 0 ) {
    BCI_SEND(s3SavedBciCmd);
#if 0
    if(skipleft != 0)  
        BCI_SEND(BCI_CLIP_LR(x + skipleft, s3vPriv.Width)); 
    else
        BCI_SEND(BCI_CLIP_LR(0, s3vPriv.Width));
#endif
    BCI_SEND(s3SavedFgColor);
    if (s3SavedBgColor != -1)  BCI_SEND(s3SavedBgColor);
    else BCI_SEND(0);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
    /*BCI_SEND(((w + 31) / 32) * h);*/
    } else {
        int i, j, count;
	s3SavedBciCmd = 0;
        BCI_SEND(0x4bcc8060);
        BCI_SEND(s3SavedFgColor);
        BCI_SEND(s3SavedBgColor);
	count = (w + 31) / 32;
	for (j = 0; j < 1; j ++) {
	    BCI_SEND(BCI_X_Y(x, y+j));
	    BCI_SEND(BCI_W_H(w, 1));
	    for (i = count; i > 0; i--) {
	        BCI_SEND(0xffffffff);
	    }
	}
    }
#endif

    if(S3SAVROPHasSrc(s3SavedBciCmd<<1)) {
       xf86AccelInfoRec.CPUToScreenColorExpandBase = 
             (void *) bci_ptr;
       xf86AccelInfoRec.ColorExpandFlags &= (~CPU_TRANSFER_BASE_FIXED); 
       }
    else {    /* Fix for XAA bug */
       xf86AccelInfoRec.CPUToScreenColorExpandBase = 
             (void *) &s3DummyTransferArea;
       xf86AccelInfoRec.ColorExpandFlags |= CPU_TRANSFER_BASE_FIXED; 
       }
}


void
S3SAVSetupFor8x8PatternColorExpand(patternx, patterny, bg, fg, rop, planemask)
unsigned patternx, patterny;
int bg, fg, rop;
unsigned planemask;
{
    BCI_GET_PTR;
    int cmd;
    int pat_offset;
    unsigned int bd;

    pat_offset = (int) (patternx * s3vPriv.Bpp + patterny * s3vPriv.Bpl);

    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_SEND_COLOR
        | BCI_CMD_DEST_GBD | BCI_CMD_PAT_SBD_MONO_NEW;
    if (bg != -1) cmd |= BCI_CMD_SEND_COLOR;
    else cmd |= BCI_CMD_PAT_TRANSPARENT;
    /*cmd |= s3vAlu_sp[rop];*/
    cmd |= 0x00F00000; /* ROP3 P */

    bd = BCI_BD_BW_DISABLE;
    BCI_BD_SET_BPP(bd, 1);
    BCI_BD_SET_STRIDE(bd, 8);

#if BCI_REPEAT
    WaitQueue(5);
    BCI_SEND(cmd);
    BCI_SEND(pat_offset);
    BCI_SEND(bd);
    BCI_SEND(fg);
    if (bg != -1) BCI_SEND(bg);
#endif
    s3SavedBciCmd = cmd;
    s3SavedSbdOffset = pat_offset;
    s3SavedSbd = bd;
    s3SavedFgColor = fg;
    s3SavedBgColor = bg;
}


void
S3SAVSubsequent8x8PatternColorExpand(patternx, patterny, x, y, w, h)
unsigned patternx, patterny;
int x, y, w, h;
{
    BCI_GET_PTR;
    unsigned int pattern0, pattern1;

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#else
    if ( 0 ) {
    WaitQueue(7);
    BCI_SEND(s3SavedBciCmd);
    BCI_SEND(s3SavedSbdOffset);
    BCI_SEND(s3SavedSbd);
    BCI_SEND(s3SavedFgColor);
    if (s3SavedBgColor != -1 || 1) BCI_SEND(s3SavedBgColor);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
    } else if ( 1 ) {
    pattern0 = *(unsigned int*) (s3SavedSbdOffset + (char *)xf86AccelInfoRec.FramebufferBase);
    pattern1 = *(unsigned int*) (4 + s3SavedSbdOffset + (char *)xf86AccelInfoRec.FramebufferBase);
    pattern1 = 0x05050505;
    WaitQueue(7);
    BCI_SEND(0x4bf08003);
    BCI_SEND(s3SavedFgColor);
    BCI_SEND(s3SavedBgColor);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
    BCI_SEND(pattern0);
    BCI_SEND(pattern1);
    }
#endif
}


void 
S3SAVSetupForFill8x8Pattern(patternx, patterny, rop, planemask, trans_col)
unsigned patternx, patterny;
int rop; 
unsigned planemask;
int trans_col;
{
    BCI_GET_PTR;
    
    int cmd;
    unsigned int bd;
    int pat_offset;

    pat_offset = (int) (patternx * s3vPriv.Bpp + patterny * s3vPriv.Bpl);

    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_DEST_GBD | BCI_CMD_PAT_SBD_COLOR_NEW;
    if (trans_col != -1)
        cmd |= BCI_CMD_SEND_COLOR | BCI_CMD_PAT_TRANSPARENT;
    cmd |= s3vAlu[rop];

    bd = BCI_BD_BW_DISABLE;
    BCI_BD_SET_BPP(bd, vgaBitsPerPixel);
    BCI_BD_SET_STRIDE(bd, 8);

#if BCI_REPEAT
    WaitQueue(6);
    BCI_SEND(cmd);
    BCI_SEND(pat_offset);
    BCI_SEND(bd);
    if (trans_col != -1) BCI_SEND(trans_col);
#endif
    s3SavedBciCmd = cmd;
    s3SavedSbdOffset = pat_offset;
    s3SavedSbd = bd;
    s3SavedBgColor = trans_col;
}


void S3SAVSubsequentFill8x8Pattern(patternx, patterny, x, y, w, h)
unsigned patternx, patterny;
int x, y, w, h;
{
    BCI_GET_PTR;

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#else
    if ( 0 ) {
    WaitQueue(6);
    BCI_SEND(s3SavedBciCmd);
    BCI_SEND(s3SavedSbdOffset);
    BCI_SEND(s3SavedSbd);
    if (s3SavedBgColor != -1) BCI_SEND(s3SavedBgColor);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
    } else {
        int i;
        WaitQueue(24);
        BCI_SEND(0x4bf00c02);
	BCI_SEND(s3SavedSbdOffset);
	BCI_SEND(s3SavedSbd);
	BCI_SEND(0);
	BCI_SEND(BCI_W_H(8, 8));
	for (i = 8*8*s3vPriv.Bpp/4; i > 0; i--)
	    BCI_SEND(0x00ff00ff);
#if 1
	BCI_SEND(0x4bf00004);
	BCI_SEND(BCI_X_Y(x, y));
	BCI_SEND(BCI_W_H(w, h));
#endif
    }
#endif
}



#if 0
void S3SAVSubsequentTwoPointLine(x1, y1, x2, y2, bias)
int x1, x2, y1, y2, bias;
{
    int cmd;
    int dx, dy;
    int Xdelta, Xstart, Ystart, Ycount;
    Bool Dir = 0;

    dx = x2 - x1;
    dy = y2 - y1;
    WaitQueue(1); 

    ErrorF("TwoPointLine, x1 %d y1 %d x2 %d y2 %d bias %d\n",
        x1, y1, x2, y2, bias);

    if(y1 > y2) {   /* Things are the right way for ViRGE */
        if(dy != 0) Xdelta = -(dx << 20)/ dy;
        else Xdelta = 0;
        if( dx > dy ){ 
            if (dx > 0) Xstart = (x1 << 20) + dx / 2; 
            else Xstart = (x1 << 20) + dx / 2 + ((1 << 20) -1);
            }
        else {
            Xstart = (x1 << 20);
            }
        Ystart = y1;
        Ycount = abs(y2 - y1) + 1;
        if (dx > 0) 
            SETL_LXEND0_END1(x1, (bias & 0x100) ? (x2 - 1) : x2); 
        else 
            SETL_LXEND0_END1(x1, (bias & 0x100) ? (x2 + 1) : x2); 
        if(dx >= 0) Dir = 1;
        }
    else {   /* Things are reversed for ViRGE */
        if(dy != 0) Xdelta = -(dx << 20)/ dy;
        else Xdelta = 0;
        if( dx > dy ){ 
            if (dx > 0) Xstart = (x2 << 20) + dx / 2; 
            else Xstart = (x2 << 20) + dx / 2 + ((1 << 20) -1);
            }
        else {
            Xstart = (x2 << 20);
            }
        Ystart = y2;
        Ycount = abs(y1 - y2) + 1;
        if (dx > 0)
            SETL_LXEND0_END1(x2, (bias & 0x100) ? (x1 - 1) : x1); 
        else 
            SETL_LXEND0_END1(x2, (bias & 0x100) ? (x1 + 1) : x1); 
        if(dx <= 0) Dir = 1;
        }

    cmd |= (CMD_LINE | CMD_AUTOEXEC | MIX_MONO_PATT) ;
    cmd |= (s3SavedRectCmdForLine & (0xff << 17));
    if(s3vLineHWClipSet) cmd |= CMD_HWCLIP ;

    WaitQueue(5);
    SETL_CMD_SET(cmd);
    SETL_LDX(Xdelta);
    SETL_LXSTART(Xstart);
    SETL_LYSTART(Ystart);
    SETL_LYCNT(Ycount | (Dir ? 0x80000000 : 0x00)); 

    if(s3vLineHWClipSet) {
        WaitQueue(2);
        CACHE_SETB_CLIP_LR(0, s3vPriv.Width); 
        CACHE_SETB_CLIP_TB(0, s3vPriv.ScissB); 
        s3vLineHWClipSet = FALSE;
        }
  
    WaitQueue(1);  
    CACHE_SETB_CMD_SET(s3SavedRectCmdForLine); 
}
#endif


void S3SAVSetClippingRectangle(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
    BCI_GET_PTR;
    int cmd;

    return;
    cmd = BCI_CMD_NOP | BCI_CMD_CLIP_NEW;
    WaitQueue(3);
    BCI_SEND(cmd);
    BCI_SEND(BCI_CLIP_TL(y1, x1));
    BCI_SEND(BCI_CLIP_BR(y1 + y2, x1 + x2));
    s3LineHWClipSet = TRUE;
}


/* Trapezoid solid fills. XAA passes the coordinates of the top start
 * and end points, and the slopes of the left and right vertexes. We
 * use this info to generate the bottom points. We use a mixture of
 * floating-point and fixed point logic; the biases are done in fixed
 * point. Essentially, these were determined experimentally. The function
 * passes xtest, but I suspect that it will not match cfb for large polygons.
 *
 * Remaining bug: no planemask support, have to tell XAA somehow.
 */

#if 0
void
S3SAVSubsequentFillTrapezoidSolid(y, h, left, dxl, dyl, el, right, dxr, dyr, er)
int y, h, left, dxl, dyl, el, right, dxr, dyr, er;
{
int l_xdelta, r_xdelta;
double lendx, rendx, dl_delta, dr_delta;
int lbias, rbias;
unsigned int cmd;
double l_sgn = -1.0, r_sgn = -1.0;

    cmd |= (CMD_POLYFILL | CMD_AUTOEXEC | MIX_MONO_PATT) ;
    cmd |= (s3SavedRectCmdForLine & (0xff << 17));
   
    l_xdelta = -(dxl << 20)/ dyl;
    r_xdelta = -(dxr << 20)/ dyr;

    dl_delta = -(double) dxl / (double) dyl;
    dr_delta = -(double) dxr / (double) dyr;
    if (dl_delta < 0.0) l_sgn = 1.0;
    if (dr_delta < 0.0) r_sgn = 1.0;
   
    lendx = l_sgn * ((double) el / (double) dyl) + left + ((h - 1) * dxl) / (double) dyl;
    rendx = r_sgn * ((double) er / (double) dyr) + right + ((h - 1) * dxr) / (double) dyr;

    /* We now have four cases */

    if (fabs(dl_delta) > 1.0) {  /* XMAJOR line */
        if (dxl > 0) { lbias = ((1 << 20) - h); }
        else { lbias = 0; }
        }
    else {
        if (dxl > 0) { lbias = ((1 << 20) - 1) + l_xdelta / 2; }
        else { lbias = 0; }
        }

    if (fabs(dr_delta) > 1.0) {   /* XMAJOR line */
        if (dxr > 0) { rbias = (1 << 20); }
        else { rbias = ((1 << 20) - 1); }
        }
    else {
        if (dxr > 0) { rbias = (1 << 20); }
        else { rbias = ((1 << 20) - 1); }
        }

    WaitQueue(8);
    CACHE_SETP_CMD_SET(cmd);
    SETP_PRDX(r_xdelta);
    SETP_PLDX(l_xdelta);
    SETP_PRXSTART(((int) (rendx * (double) (1 << 20))) + rbias);
    SETP_PLXSTART(((int) (lendx * (double) (1 << 20))) + lbias);

    SETP_PYSTART(y + h - 1);
    SETP_PYCNT((h) | 0x30000000);

    CACHE_SETB_CMD_SET(s3SavedRectCmdForLine);

}
#endif
