/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/i810.h,v 1.1.2.2 1999/11/18 19:06:16 hohndel Exp $ */
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
 * $PI$
 */

#ifndef _I810_H
#define _I810_H

/* Function prototypes */
extern void           I810StoreColors();
extern void           I810AccelFinishInit();
extern void           I810PrintErrorState();
extern void           I810UnlockFrameBuffer( void );
extern unsigned int   I810CalcWatermark( double freq, int local_mem_freq );
extern Bool           I810CharacterizeSystemRam();

/* Globals */
extern unsigned char *I810MMIOBase;
extern unsigned long  I810CursorPhysical;
extern int            I810CursorOffset;
extern int            I810Chipset;


/* Very simple memory management. 
 */
typedef struct {
   unsigned long Start;
   unsigned long End;
   unsigned long Size;
} I810MemRange;

extern int I810AllocHigh( I810MemRange *result, I810MemRange *pool, int size );
extern int I810AllocLow( I810MemRange *result, I810MemRange *pool, int size );
extern unsigned long I810LocalToPhysical( unsigned long local );

/* 
 */
typedef struct {
   int base_reg;
   int tail_mask;
   I810MemRange mem;
   char *virtual_start;
   int head;
   int tail;
   int space;
} I810RingBuffer;

extern I810RingBuffer I810LpRing;

extern int I810FrameBufferLocked;
extern int I810LmFreqSel;

extern I810MemRange I810SysMem, I810DcacheMem, *I810DisplayPtr;
extern I810MemRange I810Mprotect;
extern I810MemRange I810Cursor;

extern int I810MMIOAddr;

/* To remove all debugging, make sure I810_DEBUG is defined as a
 * preprocessor symbol, and equal to zero.  
 */
#define I810_DEBUG 0   
#ifndef I810_DEBUG
#warning "Debugging enabled - expect reduced performance"
extern int I810_DEBUG;
#endif

#define DEBUG_VERBOSE_ACCEL  0x1
#define DEBUG_VERBOSE_SYNC   0x2
#define DEBUG_VERBOSE_VGA    0x4
#define DEBUG_VERBOSE_RING   0x8
#define DEBUG_VERBOSE_OUTREG 0x10
#define DEBUG_VERBOSE_MEMORY 0x20
#define DEBUG_ALWAYS_SYNC    0x40




#define OUT_RING(n) {					\
   if (I810_DEBUG & DEBUG_VERBOSE_RING)			\
      fprintf(stderr, "OUT_RING %x: %x\n", outring, n);	\
   *(volatile unsigned int *)(virt + outring) = n;	\
   outring += 4;					\
   outring &= ringmask;					\
}

#define ADVANCE_LP_RING() {				\
    I810LpRing.tail = outring;				\
    OUTREG(I810LpRing.base_reg + RING_TAIL, outring);	\
}

#define BEGIN_LP_RING(n)					\
   unsigned int outring, ringmask;				\
   volatile char *virt;						\
   if (n>2 && (I810_DEBUG&DEBUG_ALWAYS_SYNC)) I810Sync();	\
   if (I810LpRing.space < n*4) I810WaitLpRing(n*4);		\
   I810LpRing.space -= n*4;					\
   if (I810_DEBUG & DEBUG_VERBOSE_RING)				\
      fprintf(stderr, "BEGIN_LP_RING %d\n",  n);		\
   outring = I810LpRing.tail;					\
   ringmask = I810LpRing.tail_mask;				\
   virt = I810LpRing.virtual_start;			


/* Memory mapped register access macros */
#define INREG8(addr)        *(volatile CARD8  *)(I810MMIOBase + (addr))
#define INREG16(addr)       *(volatile CARD16 *)(I810MMIOBase + (addr))
#define INREG(addr)         *(volatile CARD32 *)(I810MMIOBase + (addr))

#define OUTREG8(addr, val) do {				\
   *(volatile CARD8 *)(I810MMIOBase  + (addr)) = (val);	\
   if (I810_DEBUG&DEBUG_VERBOSE_OUTREG)			\
     fprintf(stderr, "OUTREG8(%p, %x)\n", addr, val);	\
} while (0)

#define OUTREG16(addr, val) do {			\
   *(volatile CARD16 *)(I810MMIOBase + (addr)) = (val);	\
   if (I810_DEBUG&DEBUG_VERBOSE_OUTREG)			\
     fprintf(stderr, "OUTREG16(%p, %x)\n", addr, val);	\
} while (0)

#define OUTREG(addr, val) do {				\
   *(volatile CARD32 *)(I810MMIOBase + (addr)) = (val);	\
   if (I810_DEBUG&DEBUG_VERBOSE_OUTREG)			\
     fprintf(stderr, "OUTREG(%p, %x)\n", addr, val);	\
} while (0)


#define OUTREG_(addr, val)   *(volatile CARD32 *)(I810MMIOBase + (addr)) = (val)


#ifndef PCI_VENDOR_INTEL
#define PCI_VENDOR_INTEL           0x8086
#endif

#ifndef PCI_CHIP_I810
#define PCI_CHIP_I810              0x7121
#define PCI_CHIP_I810_DC100        0x7123
#define PCI_CHIP_I810_E            0x7125 
#define PCI_CHIP_I810_BRIDGE       0x7120
#define PCI_CHIP_I810_DC100_BRIDGE 0x7122
#define PCI_CHIP_I810_E_BRIDGE     0x7124
#endif

#endif
