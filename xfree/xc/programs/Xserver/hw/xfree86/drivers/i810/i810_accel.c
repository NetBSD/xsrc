
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
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/i810/i810_accel.c,v 1.12 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Authors:
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 */

#include "xf86_ansic.h"
#include "xf86.h"

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

static void I810SetupForMono8x8PatternFill(ScrnInfoPtr pScrn, 
					   int pattx, int patty,
					   int fg, int bg, int rop,
					   unsigned int planemask);
static void I810SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, 
						 int pattx, int patty,
						 int x, int y, int w, int h);

static void I810SetupForScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
							   int fg, int bg,
							   int rop,
							   unsigned int mask);

static void I810SubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
							     int x, int y, 
							     int w, int h,
							     int skipleft );

static void I810SubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno);


/* The following function sets up the supported acceleration. Call it
 * from the FbInit() function in the SVGA driver, or before ScreenInit
 * in a monolithic server.
 */
Bool
I810AccelInit( ScreenPtr pScreen ) 
{
   XAAInfoRecPtr infoPtr;
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I810AccelInit\n");

   pI810->AccelInfoRec = infoPtr = XAACreateInfoRec();
   if (!infoPtr) return FALSE;

   pI810->bufferOffset = 0;
   infoPtr->Flags = LINEAR_FRAMEBUFFER | OFFSCREEN_PIXMAPS;
   /* There is a bit blt bug in 24 bpp.  This is a problem, but
      at least without the pixmap cache we can pass the test suite */
   if(pScrn->depth != 24)
      infoPtr->Flags |= PIXMAP_CACHE;

   /* Sync
    */
   infoPtr->Sync = I810Sync;

   /* Solid filled rectangles 
    */
   {
      infoPtr->SolidFillFlags = NO_PLANEMASK;
      infoPtr->SetupForSolidFill = I810SetupForSolidFill;
      infoPtr->SubsequentSolidFillRect = I810SubsequentSolidFillRect;
   }

   /* Screen to screen copy 
    *   - the transparency op hangs the blit engine, disable for now.
    */
   {
      infoPtr->ScreenToScreenCopyFlags = ( 0
					   | NO_PLANEMASK 
					   | NO_TRANSPARENCY 
					   | 0 );

      infoPtr->SetupForScreenToScreenCopy = I810SetupForScreenToScreenCopy;
      infoPtr->SubsequentScreenToScreenCopy = I810SubsequentScreenToScreenCopy;
   }


   /* 8x8 pattern fills 
    */
   {
      infoPtr->SetupForMono8x8PatternFill = I810SetupForMono8x8PatternFill;
      infoPtr->SubsequentMono8x8PatternFillRect =
	 I810SubsequentMono8x8PatternFillRect;
      
      infoPtr->Mono8x8PatternFillFlags = ( HARDWARE_PATTERN_PROGRAMMED_BITS |
					   HARDWARE_PATTERN_SCREEN_ORIGIN |
					   BIT_ORDER_IN_BYTE_MSBFIRST |
					   NO_PLANEMASK |
					   0 );
   }

   /* 8x8 color fills - not considered useful for XAA.
    */

   /* Scanline color expansion - Use the same scheme as the 3.3 driver.  
    *
    */
   if(pI810->Scratch.Size != 0) {
      int i;
      int width = ((pScrn->displayWidth + 31) & ~31) / 8; 
      int nr_buffers = pI810->Scratch.Size / width;
      unsigned char *ptr = pI810->FbBase + pI810->Scratch.Start;
      
      pI810->NumScanlineColorExpandBuffers = nr_buffers;
      pI810->ScanlineColorExpandBuffers = (unsigned char **) 
	 xnfcalloc( nr_buffers,  sizeof (unsigned char *) );
       
      for (i = 0 ; i < nr_buffers ; i++, ptr += width)
	 pI810->ScanlineColorExpandBuffers[i] = ptr;
       
      infoPtr->ScanlineCPUToScreenColorExpandFillFlags = (
	 NO_PLANEMASK | 
	 ROP_NEEDS_SOURCE |
	 BIT_ORDER_IN_BYTE_MSBFIRST | 
	 0);

      infoPtr->ScanlineColorExpandBuffers = (unsigned char **) 
	 xnfcalloc( 1,  sizeof (unsigned char *) );
      infoPtr->NumScanlineColorExpandBuffers = 1;

      infoPtr->ScanlineColorExpandBuffers[0] = 
	 pI810->ScanlineColorExpandBuffers[0];
      pI810->nextColorExpandBuf = 0;

      infoPtr->SetupForScanlineCPUToScreenColorExpandFill =
	 I810SetupForScanlineCPUToScreenColorExpandFill;

      infoPtr->SubsequentScanlineCPUToScreenColorExpandFill =
	 I810SubsequentScanlineCPUToScreenColorExpandFill;
       
      infoPtr->SubsequentColorExpandScanline = 
	 I810SubsequentColorExpandScanline;	  
   }

   /* Possible todo: Image writes w/ non-GXCOPY rop.
    */

   I810SelectBuffer(pScrn, I810_FRONT);

   return XAAInit(pScreen, infoPtr);
}

int
I810WaitLpRing( ScrnInfoPtr pScrn, int n, int timeout_millis )
{
   I810Ptr pI810 = I810PTR(pScrn);
   I810RingBuffer *ring = &(pI810->LpRing);
   int iters = 0;
   int start = 0;
   int now = 0;
   int last_head = 0;
   int first = 0;

   /* If your system hasn't moved the head pointer in 2 seconds, I'm going to
    * call it crashed.
    */
   if (timeout_millis == 0)
      timeout_millis = 2000;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL) {
      ErrorF( "I810WaitLpRing %d\n", n);
      first = GetTimeInMillis();
   }

   while (ring->space < n) 
   {
      int i;

      ring->head = INREG(LP_RING + RING_HEAD) & HEAD_ADDR;
      ring->space = ring->head - (ring->tail+8);

      if (ring->space < 0) 
	 ring->space += ring->mem.Size;
      
      iters++;
      now = GetTimeInMillis();
      if ( start == 0 || now < start || ring->head != last_head) {
	 if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
	    if (now > start) 
	       ErrorF( "space: %d wanted %d\n", ring->space, n ); 
	 start = now;
	 last_head = ring->head;
      } else if ( now - start > timeout_millis ) { 
	 I810PrintErrorState( pScrn ); 
	 ErrorF( "space: %d wanted %d\n", ring->space, n );
#ifdef XF86DRI
	 if(pI810->directRenderingEnabled) {
	    DRIUnlock(screenInfo.screens[pScrn->scrnIndex]);
	    DRICloseScreen(screenInfo.screens[pScrn->scrnIndex]);
	 }
#endif
 	 pI810->AccelInfoRec = NULL; /* Stops recursive behavior */
	 FatalError("lockup\n"); 
      }

      for (i = 0 ; i < 2000 ; i++)
	 ;
   }

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
   {
      now = GetTimeInMillis();
      if (now - first) {
	 ErrorF("Elapsed %d ms\n", now - first);
	 ErrorF( "space: %d wanted %d\n", ring->space, n );
      }
   }

   return iters;
}

void
I810Sync( ScrnInfoPtr pScrn ) 
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & (DEBUG_VERBOSE_ACCEL|DEBUG_VERBOSE_SYNC))
      ErrorF( "I810Sync\n");
   
#ifdef XF86DRI
   /* VT switching tries to do this.  
    */
   if (!pI810->LockHeld && pI810->directRenderingEnabled) {
      return;   
   }
#endif

   /* Send a flush instruction and then wait till the ring is empty.
    * This is stronger than waiting for the blitter to finish as it also
    * flushes the internal graphics caches.
    */
   {
      BEGIN_LP_RING(2);   
      OUT_RING( INST_PARSER_CLIENT | INST_OP_FLUSH | INST_FLUSH_MAP_CACHE );
      OUT_RING( 0 );		/* pad to quadword */
      ADVANCE_LP_RING();
   }

   I810WaitLpRing( pScrn, pI810->LpRing.mem.Size - 8, 0 );	

   pI810->LpRing.space = pI810->LpRing.mem.Size - 8;			
   pI810->nextColorExpandBuf = 0;
}

void 
I810SetupForSolidFill(ScrnInfoPtr pScrn, int color, int rop,
		      unsigned int planemask)
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I810SetupForFillRectSolid color: %x rop: %x mask: %x\n", 
	      color, rop, planemask);

   /* Color blit, p166 */
   pI810->BR[13] = (BR13_SOLID_PATTERN | 
		    (i810PatternRop[rop] << 16) |
		    (pScrn->displayWidth * pI810->cpp));
   pI810->BR[16] = color;
}


void 
I810SubsequentSolidFillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h)
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I810SubsequentFillRectSolid %d,%d %dx%d\n",
	      x,y,w,h);

   {   
      BEGIN_LP_RING(6);
   
      OUT_RING( BR00_BITBLT_CLIENT | BR00_OP_COLOR_BLT | 0x3 );
      OUT_RING( pI810->BR[13] );
      OUT_RING( (h << 16) | (w * pI810->cpp));
      OUT_RING( pI810->bufferOffset + 
		(y * pScrn->displayWidth + x) * pI810->cpp);

      OUT_RING( pI810->BR[16]);
      OUT_RING( 0 );		/* pad to quadword */

      ADVANCE_LP_RING();
   }
}

void
I810SetupForScreenToScreenCopy(ScrnInfoPtr pScrn, int xdir, int ydir, int rop,
			       unsigned int planemask, int transparency_color)
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I810SetupForScreenToScreenCopy %d %d %x %x %d\n",
	      xdir, ydir, rop, planemask, transparency_color);

   pI810->BR[13] = (pScrn->displayWidth * pI810->cpp);
   
   if (ydir == -1)
      pI810->BR[13] = (- pI810->BR[13]) & 0xFFFF;
   if (xdir == -1)
      pI810->BR[13] |= BR13_RIGHT_TO_LEFT;

   pI810->BR[13] |= i810Rop[rop] << 16;

   pI810->BR[18] = 0;
}

void
I810SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int x1, int y1, 
				 int x2, int y2, int w, int h) 
{
   I810Ptr pI810 = I810PTR(pScrn);
   int src, dst;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I810SubsequentScreenToScreenCopy %d,%d - %d,%d %dx%d\n",
	      x1,y1,x2,y2,w,h);

   if (pI810->BR[13] & BR13_PITCH_SIGN_BIT) {
      src = (y1 + h - 1) * pScrn->displayWidth * pI810->cpp;
      dst = (y2 + h - 1) * pScrn->displayWidth * pI810->cpp;
   } else {
      src = y1 * pScrn->displayWidth * pI810->cpp;
      dst = y2 * pScrn->displayWidth * pI810->cpp;
   }

   if (pI810->BR[13] & BR13_RIGHT_TO_LEFT) {
      src += (x1 + w - 1) * pI810->cpp + pI810->cpp - 1;
      dst += (x2 + w - 1) * pI810->cpp + pI810->cpp - 1;
   } else {
      src += x1 * pI810->cpp;
      dst += x2 * pI810->cpp;
   }


   /* SRC_COPY_BLT, p169 */
   {
      BEGIN_LP_RING(6);
      OUT_RING( BR00_BITBLT_CLIENT | BR00_OP_SRC_COPY_BLT | 0x4 );
      OUT_RING( pI810->BR[13] );

      OUT_RING( (h << 16) | (w * pI810->cpp));
      OUT_RING( pI810->bufferOffset + dst );

      OUT_RING( pI810->BR[13] & 0xFFFF );
      OUT_RING( pI810->bufferOffset + src );
      ADVANCE_LP_RING();
   }
}

static void
I810SetupForMono8x8PatternFill( ScrnInfoPtr pScrn, int pattx, int patty, 
				int fg, int bg, int rop, 
				unsigned int planemask) 
{
  I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I810SetupFor8x8PatternColorExpand\n");

    /* FULL_MONO_PAT_BLT, p176 */
    pI810->BR[0] = (BR00_BITBLT_CLIENT | BR00_OP_MONO_PAT_BLT | 0x9 );
    pI810->BR[18] = bg;
    pI810->BR[19] = fg;
    pI810->BR[13] = (pScrn->displayWidth * pI810->cpp);
    pI810->BR[13] |= i810PatternRop[rop] << 16;
    if (bg == -1) pI810->BR[13] |= BR13_MONO_TRANSPCY;
}

static void
I810SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, int pattx, int patty, 
				     int x, int y, int w, int h) 
{
   I810Ptr pI810 = I810PTR(pScrn);
   int addr = pI810->bufferOffset + (y * pScrn->displayWidth + x) * pI810->cpp;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I810Subsequent8x8PatternColorExpand\n");

   {
      BEGIN_LP_RING( 12 );
      OUT_RING( pI810->BR[0] | ((y << 5) & BR00_PAT_VERT_ALIGN));
      OUT_RING( pI810->BR[13]);
      OUT_RING( (h << 16) | (w * pI810->cpp));
      OUT_RING( addr );
      OUT_RING( pI810->BR[13] & 0xFFFF ); /* src pitch */
      OUT_RING( addr );                   /* src addr */
      OUT_RING( 0 );		          /* transparency color */
      OUT_RING( pI810->BR[18] );          /* bg */
      OUT_RING( pI810->BR[19] );          /* fg */
      OUT_RING( pattx );                  /* pattern data */
      OUT_RING( patty ); 
      OUT_RING( 0 );
      ADVANCE_LP_RING();
   }
}

static void 
I810GetNextScanlineColorExpandBuffer( ScrnInfoPtr pScrn )
{
   I810Ptr pI810 = I810PTR(pScrn);
   XAAInfoRecPtr infoPtr = pI810->AccelInfoRec;

   if (pI810->nextColorExpandBuf == pI810->NumScanlineColorExpandBuffers) 
      I810Sync( pScrn );
   
   infoPtr->ScanlineColorExpandBuffers[0] = 
      pI810->ScanlineColorExpandBuffers[pI810->nextColorExpandBuf];

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "using color expand buffer %d\n",
	      pI810->nextColorExpandBuf);

   pI810->nextColorExpandBuf++;
}

static void
I810SetupForScanlineCPUToScreenColorExpandFill( ScrnInfoPtr pScrn, 
						int fg, int bg, int rop,
						unsigned int planemask )
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( 
	      "I810SetupForScanlineScreenToScreenColorExpand %d %d %x %x\n",
	      fg,bg,rop,planemask);

   pI810->BR[13] = (pScrn->displayWidth * pI810->cpp);
   pI810->BR[13] |= i810Rop[rop] << 16;
   pI810->BR[13] |= (1<<27);
   if (bg == -1) pI810->BR[13] |= BR13_MONO_TRANSPCY;

   pI810->BR[18] = bg;
   pI810->BR[19] = fg;

   I810GetNextScanlineColorExpandBuffer( pScrn );
}


static void 
I810SubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
						 int x, int y, 
						 int w, int h,
						 int skipleft )
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( 
	      "I810SubsequentScanlineCPUToScreenColorExpandFill "
	      "%d,%d %dx%x %d\n", 
	      x,y,w,h,skipleft);
   
   pI810->BR[0] = BR00_BITBLT_CLIENT | BR00_OP_MONO_SRC_COPY_BLT | 0x06; 
   pI810->BR[9] = (pI810->bufferOffset + 
		   (y * pScrn->displayWidth + x) * pI810->cpp);
   pI810->BR[14] = ( (1 << 16) | (w * pI810->cpp));
   pI810->BR[11] = ((w+31)/32)-1; 
}


static void 
I810SubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno)
{
   I810Ptr pI810 = I810PTR(pScrn);

   pI810->BR[12] = (pI810->AccelInfoRec->ScanlineColorExpandBuffers[0] - 
		    pI810->FbBase);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I810SubsequentColorExpandScanline %d (addr %x)\n",
	      bufno,
	      pI810->BR[12]);
   

   {
      BEGIN_LP_RING( 8 );
      OUT_RING( pI810->BR[0]);
      OUT_RING( pI810->BR[13]);
      OUT_RING( pI810->BR[14] );
      OUT_RING( pI810->BR[9] );
      OUT_RING( pI810->BR[11] );
      OUT_RING( pI810->BR[12] ); /* srcaddr */
      OUT_RING( pI810->BR[18]);
      OUT_RING( pI810->BR[19]);    
      ADVANCE_LP_RING();
   }

   /* Advance to next scanline.
    */
   pI810->BR[9] += pScrn->displayWidth * pI810->cpp;
   I810GetNextScanlineColorExpandBuffer( pScrn );
}


void 
I810EmitFlush( ScrnInfoPtr pScrn )
{
   I810Ptr pI810 = I810PTR(pScrn);
   
   BEGIN_LP_RING(2);
   OUT_RING( INST_PARSER_CLIENT | INST_OP_FLUSH | INST_FLUSH_MAP_CACHE);
   OUT_RING( 0 );
   ADVANCE_LP_RING();
}


void 
I810SelectBuffer(ScrnInfoPtr pScrn, int buffer)
{
   I810Ptr pI810 = I810PTR(pScrn);

   switch (buffer) {
   case I810_BACK:
      pI810->bufferOffset = pI810->BackBuffer.Start;
      break;
   case I810_DEPTH:
      pI810->bufferOffset = pI810->DepthBuffer.Start;
      break;
   default:
   case I810_FRONT:
      pI810->bufferOffset = pI810->FrontBuffer.Start;
      break;
   }

   if (I810_DEBUG&DEBUG_VERBOSE_ACCEL)
      ErrorF("I810SelectBuffer %d --> offset %x\n", 
	     buffer, pI810->bufferOffset);
}


void 
I810RefreshRing(ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR(pScrn);
      
   pI810->LpRing.head = INREG(LP_RING + RING_HEAD) & HEAD_ADDR;
   pI810->LpRing.tail = INREG(LP_RING + RING_TAIL);
   pI810->LpRing.space = pI810->LpRing.head - (pI810->LpRing.tail+8);
   if (pI810->LpRing.space < 0) 
      pI810->LpRing.space += pI810->LpRing.mem.Size;

   pI810->AccelInfoRec->NeedToSync = TRUE;
}


/* Emit on gaining VT?
 */
void 
I810EmitInvarientState(ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR(pScrn);
   BEGIN_LP_RING( 10 );

   OUT_RING( INST_PARSER_CLIENT | INST_OP_FLUSH | INST_FLUSH_MAP_CACHE );
   OUT_RING( GFX_CMD_CONTEXT_SEL | CS_UPDATE_USE | CS_USE_CTX0 );
   OUT_RING( INST_PARSER_CLIENT | INST_OP_FLUSH | INST_FLUSH_MAP_CACHE);
   OUT_RING( 0 );


   OUT_RING( GFX_OP_COLOR_CHROMA_KEY );
   OUT_RING( CC1_UPDATE_KILL_WRITE | 
	     CC1_DISABLE_KILL_WRITE | 
	     CC1_UPDATE_COLOR_IDX |
	     CC1_UPDATE_CHROMA_LOW |
	     CC1_UPDATE_CHROMA_HI |
             0);
   OUT_RING( 0 );
   OUT_RING( 0 );

/*     OUT_RING( CMD_OP_Z_BUFFER_INFO ); */
/*     OUT_RING( pI810->DepthBuffer.Start | pI810->auxPitchBits); */

   ADVANCE_LP_RING();      
}

/* I830 Accel Functions */

static void I830SetupForMono8x8PatternFill(ScrnInfoPtr pScrn, 
					   int pattx, int patty,
					   int fg, int bg, int rop,
					   unsigned int planemask);
static void I830SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, 
						 int pattx, int patty,
						 int x, int y, int w, int h);

static void I830SetupForScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
							   int fg, int bg,
							   int rop,
							   unsigned int mask);

static void I830SubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
							     int x, int y, 
							     int w, int h,
							     int skipleft );

static void I830SubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno);


/* The following function sets up the supported acceleration. Call it
 * from the FbInit() function in the SVGA driver, or before ScreenInit
 * in a monolithic server.
 */
Bool
I830AccelInit(ScreenPtr pScreen)
{
   XAAInfoRecPtr infoPtr;
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL) ErrorF("I830AccelInit\n");

   pI810->AccelInfoRec = infoPtr = XAACreateInfoRec();
   if (!infoPtr) return FALSE;

   pI810->bufferOffset = 0;
   infoPtr->Flags = LINEAR_FRAMEBUFFER | OFFSCREEN_PIXMAPS | PIXMAP_CACHE;

   /* Use the same sync function as the I810.
    */
   infoPtr->Sync = I810Sync;

   /* Everything else is different enough to justify different functions */
   {
      infoPtr->SolidFillFlags = NO_PLANEMASK;
      infoPtr->SetupForSolidFill = I830SetupForSolidFill;
      infoPtr->SubsequentSolidFillRect = I830SubsequentSolidFillRect;
   }

   {
      infoPtr->ScreenToScreenCopyFlags = (NO_PLANEMASK | NO_TRANSPARENCY);

      infoPtr->SetupForScreenToScreenCopy = I830SetupForScreenToScreenCopy;
      infoPtr->SubsequentScreenToScreenCopy = I830SubsequentScreenToScreenCopy;
   }

   {
      infoPtr->SetupForMono8x8PatternFill = I830SetupForMono8x8PatternFill;
      infoPtr->SubsequentMono8x8PatternFillRect =
	 I830SubsequentMono8x8PatternFillRect;

      infoPtr->Mono8x8PatternFillFlags = (HARDWARE_PATTERN_PROGRAMMED_BITS |
					  HARDWARE_PATTERN_SCREEN_ORIGIN |
					  HARDWARE_PATTERN_PROGRAMMED_ORIGIN |
					  BIT_ORDER_IN_BYTE_MSBFIRST |
					  NO_PLANEMASK);

   }

   if(pI810->Scratch.Size != 0) {
      int i;
      int width = ((pScrn->displayWidth + 31) & ~31) / 8; 
      int nr_buffers = pI810->Scratch.Size / width;
      unsigned char *ptr = pI810->FbBase + pI810->Scratch.Start;
      
      pI810->NumScanlineColorExpandBuffers = nr_buffers;
      pI810->ScanlineColorExpandBuffers = (unsigned char **) 
	 xnfcalloc( nr_buffers,  sizeof (unsigned char *) );
       
      for (i = 0 ; i < nr_buffers ; i++, ptr += width)
	 pI810->ScanlineColorExpandBuffers[i] = ptr;
       
      infoPtr->ScanlineCPUToScreenColorExpandFillFlags = 
	(NO_PLANEMASK |
	 ROP_NEEDS_SOURCE |
	 BIT_ORDER_IN_BYTE_MSBFIRST);

      infoPtr->ScanlineColorExpandBuffers = (unsigned char **) 
	 xnfcalloc( 1,  sizeof (unsigned char *) );
      infoPtr->NumScanlineColorExpandBuffers = 1;

      infoPtr->ScanlineColorExpandBuffers[0] = 
	 pI810->ScanlineColorExpandBuffers[0];
      pI810->nextColorExpandBuf = 0;

      infoPtr->SetupForScanlineCPUToScreenColorExpandFill =
	 I830SetupForScanlineCPUToScreenColorExpandFill;

      infoPtr->SubsequentScanlineCPUToScreenColorExpandFill =
	 I830SubsequentScanlineCPUToScreenColorExpandFill;
       
      infoPtr->SubsequentColorExpandScanline = 
	 I830SubsequentColorExpandScanline;	  
   }

   I810SelectBuffer(pScrn, I810_FRONT);

   return XAAInit(pScreen, infoPtr);
}

void
I830SetupForSolidFill(ScrnInfoPtr pScrn, int color, int rop,
		      unsigned int planemask)
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I830SetupForFillRectSolid color: %x rop: %x mask: %x\n", 
	      color, rop, planemask);

   pI810->BR[13] = ((i810PatternRop[rop] << 16) |
		    (pScrn->displayWidth * pI810->cpp));

   pI810->BR[16] = color;

   switch (pScrn->bitsPerPixel) {
   case 8: break;
   case 16: pI810->BR[13] |= (1<<24); break;
   case 32: pI810->BR[13] |= ((1<<25)|(1<<24)); break;
   }
}

void
I830SubsequentSolidFillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h)
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I830SubsequentFillRectSolid %d,%d %dx%d\n",
	      x,y,w,h);

   {
      BEGIN_LP_RING(6);

      if(pScrn->bitsPerPixel == 32) {
	 OUT_RING(COLOR_BLT_CMD | COLOR_BLT_WRITE_ALPHA | COLOR_BLT_WRITE_RGB);
      } else {
	 OUT_RING(COLOR_BLT_CMD);
      }
      OUT_RING(pI810->BR[13]);
      OUT_RING((h << 16) | (w * pI810->cpp));
      OUT_RING(pI810->bufferOffset + (y * pScrn->displayWidth + x) * 
	       pI810->cpp);
      OUT_RING(pI810->BR[16]);
      OUT_RING(0);

      ADVANCE_LP_RING();
   }
}

void
I830SetupForScreenToScreenCopy(ScrnInfoPtr pScrn, int xdir, int ydir, int rop,
			       unsigned int planemask, int transparency_color)
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I830SetupForScreenToScreenCopy %d %d %x %x %d\n",
	      xdir, ydir, rop, planemask, transparency_color);

   pI810->BR[13] = (pScrn->displayWidth * pI810->cpp);
   pI810->BR[13] |= i810Rop[rop] << 16;

   switch (pScrn->bitsPerPixel) {
   case 8: break;
   case 16: pI810->BR[13] |= (1<<24); break;
   case 32: pI810->BR[13] |= ((1<<25)|(1<<24)); break;
   }

}

void
I830SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int src_x1, int src_y1, 
				 int dst_x1, int dst_y1, int w, int h) 
{
   I810Ptr pI810 = I810PTR(pScrn);
   int dst_x2, dst_y2;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I830SubsequentScreenToScreenCopy %d,%d - %d,%d %dx%d\n",
	     src_x1,src_y1,dst_x1,dst_y1,w,h);

   dst_x2 = dst_x1 + w;
   dst_y2 = dst_y1 + h;

   {
      BEGIN_LP_RING(8);

      if(pScrn->bitsPerPixel == 32) {
	 OUT_RING(XY_SRC_COPY_BLT_CMD | XY_SRC_COPY_BLT_WRITE_ALPHA | 
		  XY_SRC_COPY_BLT_WRITE_RGB);
      } else {
	 OUT_RING(XY_SRC_COPY_BLT_CMD);
      }
      OUT_RING(pI810->BR[13]);
      OUT_RING((dst_y1 << 16) | (dst_x1 & 0xffff));
      OUT_RING((dst_y2 << 16) | (dst_x2 & 0xffff));
      OUT_RING(pI810->bufferOffset);
      OUT_RING((src_y1 << 16) | (src_x1 & 0xffff));
      OUT_RING(pI810->BR[13] & 0xFFFF);
      OUT_RING(pI810->bufferOffset);

      ADVANCE_LP_RING();
   }
}

static void
I830SetupForMono8x8PatternFill(ScrnInfoPtr pScrn, int pattx, int patty,
			       int fg, int bg, int rop,
			       unsigned int planemask)
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I830SetupFor8x8PatternColorExpand\n");

   pI810->BR[16] = pattx;
   pI810->BR[17] = patty;
   pI810->BR[18] = bg;
   pI810->BR[19] = fg;

   pI810->BR[13] = (pScrn->displayWidth * pI810->cpp); /* In bytes */
   pI810->BR[13] |= i810PatternRop[rop] << 16;
   if (bg == -1) pI810->BR[13] |= (1<<28);

   switch (pScrn->bitsPerPixel) {
   case 8: break;
   case 16: pI810->BR[13] |= (1<<24); break;
   case 32: pI810->BR[13] |= ((1<<25)|(1<<24)); break;
   }
   
}

static void
I830SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, int pattx, int patty, 
				     int x, int y, int w, int h) 
{
   I810Ptr pI810 = I810PTR(pScrn);
   int x1, x2, y1, y2;

   x1 = x;
   x2 = x + w;
   y1 = y;
   y2 = y + h;

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I830Subsequent8x8PatternColorExpand\n");

   {
      BEGIN_LP_RING( 10 );

      if(pScrn->bitsPerPixel == 32) {
	 OUT_RING(XY_MONO_PAT_BLT_CMD | XY_MONO_PAT_BLT_WRITE_ALPHA | 
		  XY_MONO_PAT_BLT_WRITE_RGB | 
		  ((patty << 8) & XY_MONO_PAT_VERT_SEED) |
		  ((pattx << 12) & XY_MONO_PAT_HORT_SEED));
      } else {
	 OUT_RING(XY_MONO_PAT_BLT_CMD |
		  ((patty << 8) & XY_MONO_PAT_VERT_SEED) |
		  ((pattx << 12) & XY_MONO_PAT_HORT_SEED));
      }
      OUT_RING(pI810->BR[13]);
      OUT_RING((y1 << 16) | x1);
      OUT_RING((y2 << 16) | x2);
      OUT_RING(pI810->bufferOffset);
      OUT_RING(pI810->BR[18]);          /* bg */
      OUT_RING(pI810->BR[19]);          /* fg */
      OUT_RING(pI810->BR[16]);		/* pattern data */
      OUT_RING(pI810->BR[17]);
      OUT_RING(0);
      ADVANCE_LP_RING();
   }
}

static void 
I830GetNextScanlineColorExpandBuffer(ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR(pScrn);
   XAAInfoRecPtr infoPtr = pI810->AccelInfoRec;

   if (pI810->nextColorExpandBuf == pI810->NumScanlineColorExpandBuffers) 
      I810Sync( pScrn );
   
   infoPtr->ScanlineColorExpandBuffers[0] = 
      pI810->ScanlineColorExpandBuffers[pI810->nextColorExpandBuf];

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "using color expand buffer %d\n",
	      pI810->nextColorExpandBuf);

   pI810->nextColorExpandBuf++;
}

static void
I830SetupForScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
					       int fg, int bg, int rop,
					       unsigned int planemask)
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF("I830SetupForScanlineScreenToScreenColorExpand %d %d %x %x\n",
	      fg,bg,rop,planemask);

   /* Fill out register values */
   pI810->BR[13] = (pScrn->displayWidth * pI810->cpp);
   pI810->BR[13] |= i810Rop[rop] << 16;
   if (bg == -1) pI810->BR[13] |= (1<<29);

   switch (pScrn->bitsPerPixel) {
   case 8: break;
   case 16: pI810->BR[13] |= (1<<24); break;
   case 32: pI810->BR[13] |= ((1<<25)|(1<<24)); break;
   }

   pI810->BR[18] = bg;
   pI810->BR[19] = fg;

   I830GetNextScanlineColorExpandBuffer( pScrn );
}

static void 
I830SubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
						 int x, int y, 
						 int w, int h,
						 int skipleft)
{
   I810Ptr pI810 = I810PTR(pScrn);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF("I830SubsequentScanlineCPUToScreenColorExpandFill "
	      "%d,%d %dx%x %d\n", 
	      x,y,w,h,skipleft);

   /* Fill out register values */
   pI810->BR[9] = (pI810->bufferOffset + 
		   (y * pScrn->displayWidth + x) * pI810->cpp);
   pI810->BR[11] = ((1 << 16) | w);
}

static void 
I830SubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno)
{
   I810Ptr pI810 = I810PTR(pScrn);

   pI810->BR[12] = (pI810->AccelInfoRec->ScanlineColorExpandBuffers[0] - 
		    pI810->FbBase);

   if (I810_DEBUG & DEBUG_VERBOSE_ACCEL)
      ErrorF( "I830SubsequentColorExpandScanline %d (addr %x)\n",
	      bufno,
	      pI810->BR[12]);

   {
      BEGIN_LP_RING( 8 );

      if(pScrn->bitsPerPixel == 32) {
	 OUT_RING(XY_MONO_SRC_BLT_CMD | XY_MONO_SRC_BLT_WRITE_ALPHA | 
		  XY_MONO_SRC_BLT_WRITE_RGB);
      } else {
	 OUT_RING(XY_MONO_SRC_BLT_CMD);
      }
      OUT_RING(pI810->BR[13]);
      OUT_RING(0); /* x1 = 0, y1 = 0 */
      OUT_RING(pI810->BR[11]); /* x2 = w, y2 = 1 */
      OUT_RING(pI810->BR[9]);  /* dst addr */
      OUT_RING(pI810->BR[12]); /* src addr */
      OUT_RING( pI810->BR[18]); /* bg */
      OUT_RING( pI810->BR[19]); /* fg */

      ADVANCE_LP_RING();
   }

   /* Advance to next scanline.
    */
   pI810->BR[9] += pScrn->displayWidth * pI810->cpp;
   I830GetNextScanlineColorExpandBuffer( pScrn );
}
