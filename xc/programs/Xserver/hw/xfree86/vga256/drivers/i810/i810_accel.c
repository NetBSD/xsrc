/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/i810_accel.c,v 1.1.2.2 1999/11/18 19:06:16 hohndel Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 *   Based on i740 driver by
 *       Kevin E. Martin <kevin@precisioninsight.com>
 *
 *
 * $PI$
 */

#include <math.h>
#include <stdio.h>
#include <sys/mman.h>
#include <signal.h>

#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#include "vgaPCI.h"

#include "xf86xaa.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "i810.h"
#include "i810_reg.h"


static unsigned int i810Rop[16] = {
    0x00, /* GXclear      */
    0x88, /* GXand        */
    0x44, /* GXandReverse */
    0xCC, /* GXcopy       */
    0x22, /* GXandInvert  */
    0xAA, /* GXnoop       */
    0x66, /* GXxor        */
    0xEE, /* GXor         */
    0x11, /* GXnor        */
    0x99, /* GXequiv      */
    0x55, /* GXinvert     */
    0xDD, /* GXorReverse  */
    0x33, /* GXcopyInvert */
    0xBB, /* GXorInverted */
    0x77, /* GXnand       */
    0xFF  /* GXset        */
};

static unsigned int i810PatternRop[16] = {
    0x00, /* GXclear      */
    0xA0, /* GXand        */
    0x50, /* GXandReverse */
    0xF0, /* GXcopy       */
    0x0A, /* GXandInvert  */
    0xAA, /* GXnoop       */
    0x5A, /* GXxor        */
    0xFA, /* GXor         */
    0x05, /* GXnor        */
    0xA5, /* GXequiv      */
    0x55, /* GXinvert     */
    0xF5, /* GXorReverse  */
    0x0F, /* GXcopyInvert */
    0xAF, /* GXorInverted */
    0x5F, /* GXnand       */
    0xFF  /* GXset        */
};

static void I810Sync();
static void I810LockFrameBuffer();
static void I810SetupForFillRectSolid();
static void I810SubsequentFillRectSolid();
static void I810SetupForScreenToScreenCopy();
static void I810SubsequentScreenToScreenCopy();
static void I810SetupFor8x8PatternColorExpand();
static void I810Subsequent8x8PatternColorExpand();
static void I810SetupForScanlineScreenToScreenColorExpand();
static void I810SubsequentScanlineScreenToScreenColorExpand();
static void I810SetupForCPUToScreenColorExpand();
static void I810SubsequentCPUToScreenColorExpand();


typedef struct {
   unsigned int BR[20]; 
} BlitCommand;

static BlitCommand bltcmd;


/* KW: Use mprotect() to avoid repeated sync()'ing that is
 * characteristic of xaa.  With this defined, 'x11perf -hseg10' goes
 * from .0026msec to .0009msec, but '-line10' goes up from .0017 to
 * .0027.  Have added a heuristic to minimize the damage in the worst
 * case while preserving the benefits in the all-accelerated cases;
 * -line10 is now around .0019.
 *
 * It seems in xfree-4.0 these problems are solved and xaa has correct
 * syncing behaviour - so this code can be thrown away when migrating
 * to that server.  The mods to scanline color expansion may still be
 * useful. 
 */
#define SYNC_BY_SIGNAL (!(I810_DEBUG&DEBUG_ALWAYS_SYNC))

/*
 * The following function sets up the supported acceleration. Call it
 * from the FbInit() function in the SVGA driver, or before ScreenInit
 * in a monolithic server.
 */
void
I810AccelInit() 
{
   I810MemRange scratch, pix_cache;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810AccelInit\n");


    if (vgaBitsPerPixel == 32) {
	xf86AccelInfoRec.Flags = 0; /* Disables all acceleration */
	return;
    }

    xf86AccelInfoRec.Flags = (BACKGROUND_OPERATIONS |
			      PIXMAP_CACHE |
			      NO_PLANEMASK |
			      COP_FRAMEBUFFER_CONCURRENCY |
			      HARDWARE_PATTERN_SCREEN_ORIGIN |
			      HARDWARE_PATTERN_BIT_ORDER_MSBFIRST |
			      HARDWARE_PATTERN_MONO_TRANSPARENCY);


    /* Sync - allow fallback to the old code by environment variable.
     * This should go into the options section of the config file.
     */
    if (SYNC_BY_SIGNAL && !getenv("I810_NO_SYNC_BY_SIGNAL"))
       xf86AccelInfoRec.Sync = I810LockFrameBuffer;
    else {
       ErrorF("%s %s: Using old style sync\n",
	      XCONFIG_PROBED, vga256InfoRec.name);
       xf86AccelInfoRec.Sync = I810Sync; 
    }

    /* Solid filled rectangles */
    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;
    xf86AccelInfoRec.SetupForFillRectSolid =
	I810SetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid =
	I810SubsequentFillRectSolid;

    /* Screen to screen copy 
     *   - the transparency op hangs the blit engine, disable for now.
     */
    xf86GCInfoRec.CopyAreaFlags = ( 0
				    | NO_PLANEMASK 
				    | NO_TRANSPARENCY
				    );

    xf86AccelInfoRec.SetupForScreenToScreenCopy =
       I810SetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
       I810SubsequentScreenToScreenCopy;

    /* 8x8 pattern fills */
    xf86AccelInfoRec.ColorExpandFlags = (NO_PLANEMASK |
					 COP_FRAMEBUFFER_CONCURRENCY |
					 SCANLINE_PAD_DWORD |
					 CPU_TRANSFER_PAD_QWORD |
					 BIT_ORDER_IN_BYTE_MSBFIRST);

    xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
       I810SetupFor8x8PatternColorExpand;
    xf86AccelInfoRec.Subsequent8x8PatternColorExpand =
       I810Subsequent8x8PatternColorExpand;

    /* No mmio regions for cpu-to-screen expand.  Using indirect
     * expansion, with a small hack to the xaa code for 64 bit
     * scanline alignment.  The improvement is slight, but probably
     * worthwhile.
     *
     * With the sync_by_signal code, a futher speedup was gained by
     * modifying xaa to use the full scratch buffer before recycling
     * to earlier addresses - no explicit flushing is required until
     * the address wrap-around is detected.  Reducing the calls to
     * I180Sync() is a significant win.
     *
     * Text now seems significantly faster than on my mga-g200 (also
     * xfree-3.3).  
     */

    /* Try for 64K, otherwise try for 16K
     */
    if ( I810AllocHigh( &scratch, &I810SysMem, 64*1024 ) || 
	 I810AllocHigh( &scratch, &I810SysMem, 16*1024 ) )
    {
       xf86AccelInfoRec.PingPongBuffers = (scratch.Size * 8 / 
					   (vga256InfoRec.displayWidth * 
					    vgaBytesPerPixel));

       xf86AccelInfoRec.ScratchBufferAddr = scratch.Start;
       xf86AccelInfoRec.ScratchBufferSize = scratch.Size;

       if (I810_DEBUG & DEBUG_VERBOSE_MEMORY)
	  fprintf(stderr, 
		  "advertising scratch (sz %x) from %x to %x\n", 
		  scratch.Size,
		  scratch.Start, scratch.Start + scratch.Size);

       xf86AccelInfoRec.SetupForScanlineScreenToScreenColorExpand =
	  I810SetupForScanlineScreenToScreenColorExpand;

       xf86AccelInfoRec.SubsequentScanlineScreenToScreenColorExpand =
	  I810SubsequentScanlineScreenToScreenColorExpand;
    }
    else
    {
       /* Without color-expanded text, there will be too many
	* fallbacks for sync-by-signal to be effective.
	*/
       ErrorF("%s %s: Using old style sync\n",
	      XCONFIG_PROBED, vga256InfoRec.name);
       xf86AccelInfoRec.Sync = I810Sync; 
    }


    /* Pixmap cache.
     */    
    pix_cache.Size = (256 * vga256InfoRec.displayWidth * PSZ / 8);

    if (pix_cache.Size > I810SysMem.Size)
       pix_cache.Size = I810SysMem.Size;
    
    if (I810AllocLow( &pix_cache, I810DisplayPtr, pix_cache.Size ))
    {
       xf86AccelInfoRec.PixmapCacheMemoryStart = pix_cache.Start;
       xf86AccelInfoRec.PixmapCacheMemoryEnd = pix_cache.End;
       I810Mprotect.End = pix_cache.End;
       I810Mprotect.Size = I810Mprotect.End - I810Mprotect.Start;
    }
   
    if (I810_DEBUG & DEBUG_VERBOSE_MEMORY) {
       fprintf(stderr, "advertising pixcache from %x to %x\n", 
	       xf86AccelInfoRec.PixmapCacheMemoryStart,
	       xf86AccelInfoRec.PixmapCacheMemoryEnd);
       fprintf(stderr, "mprotect region from %x to %x\n", 
	       I810Mprotect.Start, I810Mprotect.End);
    }

}


void 
I810AccelFinishInit()
{
   if (I810LpRing.mem.Start)
      I810LpRing.virtual_start = 
	 (char *)vgaLinearBase+I810LpRing.mem.Start;

   if (xf86AccelInfoRec.CPUToScreenColorExpandBase) {
      unsigned int tmp = 
	 ((unsigned int)xf86AccelInfoRec.CPUToScreenColorExpandBase +
	  (unsigned int)vgaLinearBase);
      xf86AccelInfoRec.CPUToScreenColorExpandBase = (unsigned int *)tmp;
   }
}

int I810FrameBufferLocked = 0;

/* Adapt John Carmack's mga register protection code to do the same
 * trick to sync the ring buffer when the rest of the xserver decides
 * to do a software fallback (or a frame buffer read).  This is a big
 * win because XAA calls sync() very often, just in case there is a
 * fallback, and it is not a cheap operation.
 *
 * Doing this precludes the same trick from working in glx.so, but
 * it shouldn't be needed as everything is serialized by the ring
 * buffer anyway.  
 */
void 
I810UnlockFrameBuffer( void ) 
{   
   I810Sync();

   mprotect( (void *)((unsigned int)vgaLinearBase + I810Mprotect.Start), 
	     I810Mprotect.Size, 
	     PROT_READ | PROT_WRITE );
   I810FrameBufferLocked = 0;
}

static void 
I810FrameBufferAccessSignalHandler( int signal ) 
{
   if (I810FrameBufferLocked) {
      /* Someone has tried to access the frame buffer, 
       * so make sure the drawing engine has finished.
       */
      I810UnlockFrameBuffer();
   } else {
      /* This is a real segfault...  
       */
      FatalError("Signal 11");
   }
}

static void 
I810LockFrameBuffer( void ) 
{
   static int count = 0;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810LockFrameBuffer\n");

   /* Sometimes, xaa even calls sync when it has done *no* acellerated
    * rendering - trap these cases by checking for an empty ring buffer,
    * which only occurs if the last operation was a sync.
    *
    * This is actually a useful heuristic as it tells us when a
    * fallback has occurred, without the expense of 3 system calls.
    */
   if (I810LpRing.head == I810LpRing.tail) {
      if (count < 10) count = 1;      
      return;
   }

   if (!I810FrameBufferLocked) {

      /* Sync the first few times, as this is quicker for mixed
       * blit/fallback code.
       */
      if (++count < 10) {
	 I810Sync();
	 return;
      }
      
      /* Cause a SIGSEGV if the X server tries to read or write the
       * framebuffer.
       */
      mprotect( (void *)((unsigned int)vgaLinearBase + I810Mprotect.Start), 
		I810Mprotect.Size, PROT_NONE );
      signal( SIGSEGV, I810FrameBufferAccessSignalHandler );
      I810FrameBufferLocked = 1;
      count = 0;
   }
}




static void
I810WaitLpRing( int n )
{
   int j = 0;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810WaitLpRing %d\n", n);

   while (I810LpRing.space < n) 
   {
      I810LpRing.head = INREG(I810LpRing.base_reg + RING_HEAD) & HEAD_ADDR;
      I810LpRing.space = I810LpRing.head - (I810LpRing.tail + 8);

      if (I810LpRing.space < 0) 
	 I810LpRing.space += I810LpRing.mem.Size;

      if (++j > 5000000) { 
	 I810PrintErrorState(); 
	 fprintf(stderr, "space: %d wanted %d\n", I810LpRing.space, n );
	 FatalError("lockup\n"); 
      }
   }
}

static void
I810Sync() 
{
   if (I810_DEBUG & (DEBUG_VERBOSE_ACCEL|DEBUG_VERBOSE_SYNC))
      fprintf(stderr, "I810Sync\n");
   
   /* Catch the sync-after-sync case.
    */
   if (I810LpRing.head == I810LpRing.tail && I810LpRing.space) 
      return;

   /* Send a flush instruction and then wait till the ring is empty.
    * This is stronger than waiting for the blitter to finish as it also
    * flushes the internal graphics caches.
    */
   {
      BEGIN_LP_RING(2);   
      OUT_RING( INST_PARSER_CLIENT | INST_OP_FLUSH );
      OUT_RING( 0 );		/* pad to quadword */
      ADVANCE_LP_RING();
   }

   I810WaitLpRing( I810LpRing.mem.Size - 8 );	
   I810LpRing.space = I810LpRing.mem.Size - 8;			
}


static void
I810SetupForFillRectSolid(color, rop, planemask)
    int color, rop;
    unsigned planemask;
{
   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810SetupForFillRectSolid color: %x rop: %x mask: %x\n", 
	      color, rop, planemask);

   /* Color blit, p166 */
   bltcmd.BR[13] = (BR13_SOLID_PATTERN | 
		    (i810PatternRop[rop] << 16) |
		    (vga256InfoRec.displayWidth * vgaBytesPerPixel));
   bltcmd.BR[16] = color;
}


static void
I810SubsequentFillRectSolid(x, y, w, h)
    int x, y, w, h;
{
   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810SubsequentFillRectSolid %d,%d %dx%d\n",
	      x,y,w,h);

   {   
      BEGIN_LP_RING(6);
   
      OUT_RING( BR00_BITBLT_CLIENT | BR00_OP_COLOR_BLT | 0x3 );
      OUT_RING( bltcmd.BR[13] );
      OUT_RING( (h << 16) | (w * vgaBytesPerPixel));
      OUT_RING( (y * vga256InfoRec.displayWidth + x) * vgaBytesPerPixel);

      OUT_RING( bltcmd.BR[16]);
      OUT_RING( 0 );		/* pad to quadword */

      ADVANCE_LP_RING();
   }
}

static void
I810SetupForScreenToScreenCopy(xdir, ydir, rop, planemask, transparency_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int transparency_color;
{
   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810SetupForScreenToScreenCopy %d %d %x %x %d\n",
	      xdir, ydir, rop, planemask, transparency_color);

   bltcmd.BR[13] = (vga256InfoRec.displayWidth * vgaBytesPerPixel);
   
   if (ydir == -1)
      bltcmd.BR[13] = (- bltcmd.BR[13]) & 0xFFFF;
   if (xdir == -1)
      bltcmd.BR[13] |= BR13_RIGHT_TO_LEFT;

   bltcmd.BR[13] |= i810Rop[rop] << 16;
   bltcmd.BR[18] = 0;
}

static void
I810SubsequentScreenToScreenCopy(x1, y1, x2, y2, w, h)
    int x1, y1, x2, y2, w, h;
{
   int src, dst;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810SubsequentScreenToScreenCopy %d,%d - %d,%d %dx%d\n",
	      x1,y1,x2,y2,w,h);

   if (bltcmd.BR[13] & BR13_PITCH_SIGN_BIT) {
      src = (y1 + h - 1) * vga256InfoRec.displayWidth * vgaBytesPerPixel;
      dst = (y2 + h - 1) * vga256InfoRec.displayWidth * vgaBytesPerPixel;
   } else {
      src = y1 * vga256InfoRec.displayWidth * vgaBytesPerPixel;
      dst = y2 * vga256InfoRec.displayWidth * vgaBytesPerPixel;
   }

   if (bltcmd.BR[13] & BR13_RIGHT_TO_LEFT) {
      src += (x1 + w - 1) * vgaBytesPerPixel + vgaBytesPerPixel - 1;
      dst += (x2 + w - 1) * vgaBytesPerPixel + vgaBytesPerPixel - 1;
   } else {
      src += x1 * vgaBytesPerPixel;
      dst += x2 * vgaBytesPerPixel;
   }


   /* SRC_COPY_BLT, p169 */
   {
      BEGIN_LP_RING(6);
      OUT_RING( BR00_BITBLT_CLIENT | BR00_OP_SRC_COPY_BLT | 0x4 );
      OUT_RING( bltcmd.BR[13] );

      OUT_RING( (h << 16) | (w * vgaBytesPerPixel));
      OUT_RING( dst );

      OUT_RING( bltcmd.BR[13] & 0xFFFF );
      OUT_RING( src );
      ADVANCE_LP_RING();
   }

}

static void
I810SetupFor8x8PatternColorExpand(patternx, patterny, bg, fg, 
				  rop, planemask)
    unsigned int patternx, patterny;
    int bg, fg, rop;
    unsigned int planemask;
{
   char *paddr = ((char *)vgaLinearBase + 
		  (patternx >> 3) +
		  patterny * vga256InfoRec.displayWidth * vgaBytesPerPixel);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810SetupFor8x8PatternColorExpand\n");

    /* FULL_MONO_PAT_BLT, p176 */
    bltcmd.BR[0] = (BR00_BITBLT_CLIENT | BR00_OP_MONO_PAT_BLT | 0x9 );
    bltcmd.BR[18] = bg;
    bltcmd.BR[19] = fg;
    bltcmd.BR[13] = (vga256InfoRec.displayWidth * vgaBytesPerPixel);
    bltcmd.BR[13] |= i810PatternRop[rop] << 16;
    if (bg == -1) bltcmd.BR[13] |= BR13_MONO_TRANSPCY;

    /* Have to actually pluck the data out of uncached memory ourselves -
     * it would be far more convenient to have the server pass this data
     * as parameters.
     */
    bltcmd.BR[15] = *(int *)paddr;
    bltcmd.BR[16] = *(int *)(paddr + 4);
}

static void
I810Subsequent8x8PatternColorExpand(patternx, patterny, x, y, w, h)
    unsigned int patternx, patterny;
    int x, y, w, h;
{
    int addr = (y * vga256InfoRec.displayWidth + x) * vgaBytesPerPixel;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810Subsequent8x8PatternColorExpand\n");

   {
      BEGIN_LP_RING( 12 );
      OUT_RING( bltcmd.BR[0] | ((y << 5) & BR00_PAT_VERT_ALIGN));
      OUT_RING( bltcmd.BR[13]);
      OUT_RING( (h << 16) | (w * vgaBytesPerPixel));
      OUT_RING( addr );
      OUT_RING( bltcmd.BR[13] & 0xFFFF ); /* src pitch */
      OUT_RING( addr );                   /* src addr */
      OUT_RING( 0 );		          /* transparency color */
      OUT_RING( bltcmd.BR[18] );          /* bg */
      OUT_RING( bltcmd.BR[19] );          /* fg */
      OUT_RING( bltcmd.BR[15] );          /* pattern data */
      OUT_RING( bltcmd.BR[16] ); 
      OUT_RING( 0 );
      ADVANCE_LP_RING();
   }
}


static void
I810SetupForScanlineScreenToScreenColorExpand(x, y, w, h, bg, fg, rop, 
					      planemask)
    int x, y, w, h;
    int bg, fg, rop;
    unsigned int planemask;
{
   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, 
	      "I810SetupForScanlineScreenToScreenColorExpand %d,%d %dx%d\n",
	      x,y,w,h);

   /* MONO_SRC_COPY_BLT, p172 */

   bltcmd.BR[0] = ( BR00_BITBLT_CLIENT | 
		    BR00_OP_MONO_SRC_COPY_BLT |
		    0x6 );

   bltcmd.BR[13] = (vga256InfoRec.displayWidth * vgaBytesPerPixel);
   bltcmd.BR[13] |= i810Rop[rop] << 16;
   bltcmd.BR[13] |= (1<<27);
   if (bg == -1) bltcmd.BR[13] |= BR13_MONO_TRANSPCY;

   bltcmd.BR[14] = ( (1 << 16) | (w * vgaBytesPerPixel));
   bltcmd.BR[9] = (y * vga256InfoRec.displayWidth + x) * vgaBytesPerPixel;
   bltcmd.BR[18] = bg;
   bltcmd.BR[19] = fg;

   bltcmd.BR[11] = ((w+31)/32)-1; 
}



static void
I810SubsequentScanlineScreenToScreenColorExpand(srcaddr)
   unsigned int srcaddr;
{
   static int last_scraddr;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      fprintf(stderr, "I810SubsequentScanlineScreenToScreenColorExpand %x\n", 
	      srcaddr);

   /* Sync when old addresses start to be reused.  Works best with
    * slight mod to xaa to use the whole of the scratch buffer.
    */
   if (srcaddr < last_scraddr) 
      I810Sync();

   last_scraddr = srcaddr;

   {
      BEGIN_LP_RING( 8 );
      OUT_RING( bltcmd.BR[0] );
      OUT_RING( bltcmd.BR[13]);
      OUT_RING( bltcmd.BR[14] );
      OUT_RING( bltcmd.BR[9] );
      OUT_RING( bltcmd.BR[11] );
      OUT_RING( srcaddr/8 );
      OUT_RING( bltcmd.BR[18]);
      OUT_RING( bltcmd.BR[19]);    
      ADVANCE_LP_RING();
   }

   bltcmd.BR[9] += vga256InfoRec.displayWidth * vgaBytesPerPixel;
}
