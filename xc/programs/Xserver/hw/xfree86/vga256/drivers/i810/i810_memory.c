/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/i810_memory.c,v 1.1.2.2 1999/11/18 19:06:17 hohndel Exp $ */
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


#include "X.h"
#include "input.h"
#include "screenint.h"
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"
#include "xf86cursor.h"
#include "xf86_PCI.h"
#include "vgaPCI.h"

extern vgaPCIInformation *vgaPCIInfo;
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "i810.h"
#include "i810_reg.h"

#include <agpgart.h>

/* DcachePtr will be set up, but not used. 
 */
I810MemRange I810DcacheMem = { 0, 0, 0 };
I810MemRange I810SysMem = { 0, 0, 0 };
I810MemRange *I810DisplayPtr = &I810SysMem;


#define gtt_None         0
#define gtt_Local        1
#define gtt_Kernel       2


/* Used in fallback GTT:
 */
static int I810GttType = 0;
static unsigned long I810Physical = 0;
static unsigned long I810GttSize = 0;
static unsigned long I810GttPhysical = 0;
static unsigned long I810ScratchPageLocal = 0;


/* Used to determine which watermarks to use:
 */
int I810LmFreqSel = 0;


/* Communications with the kernel module:
 */
static int    gartfd;
static struct gart_info gartinf;
static struct gart_pge_info gart_pg_inf;

static int I810AllocateGARTMemory( void ) 
{
   int size = 4 * 1024 * 1024;
   int i, j, pages = size / 4096; 
   struct stat sb;

   /* We can only fallback if there is no agpgart module active -
    * otherwise the fallback to system ram screws up the other gart
    * client(s). 
    */
   if (stat("/dev/agpgart", &sb) != 0) {
      ErrorF("%s %s: Stat failed on /dev/agpgart: %s\n", 
	     XCONFIG_PROBED, vga256InfoRec.name, 
	     sys_errlist[errno]);

      /* Fallback - we'll manage the GTT ourselves.
       */
      return -1;
   }

   gartfd = open("/dev/agpgart", O_RDWR);
   if (gartfd == -1) {	
      ErrorF("%s %s: unable to open /dev/agpgart: %s\n", 
	     XCONFIG_PROBED, vga256InfoRec.name, 
	     sys_errlist[errno]);
      FatalError("Aborting");
   }

   if (ioctl(gartfd, GARTIOCINFO, &gartinf) != 0) {
      ErrorF("%s %s: error doing ioctl(GARTIOCINFO): %s\n", 
	     XCONFIG_PROBED, vga256InfoRec.name, 
	     sys_errlist[errno]);
      FatalError("Aborting");
   }


   /* Dcache - half the speed of normal ram, so not really useful for
    * a 2d server.  Don't bother reporting its presence.  
    */
   if (gartinf.num_dcache_slots) {
      I810DcacheMem.Start = gartinf.num_of_slots * 4096;
      I810DcacheMem.Size = gartinf.num_dcache_slots * 4096;
      I810DcacheMem.End = I810DcacheMem.Start + I810DcacheMem.Size;
   }
   

   /* Treat the gart like video memory - we assume we own all that is
    * there, so ignore EBUSY errors.  Don't try to remove it on
    * failure, either.
    */
   for (i = 0; i < pages; i++) 
      if (ioctl(gartfd, GARTIOCINSERT, &i) != 0) {
	 if (errno != EBUSY) 
	 {	
	    perror("gart insert");
	    ErrorF("%s %s: GART: allocation of %d pages failed at page %d\n", 
		   XCONFIG_PROBED, vga256InfoRec.name, pages, i);
	    FatalError("Aborting");
	 }	
      } 

   ErrorF("%s %s: GART: allocated %dK system ram\n",
	  XCONFIG_PROBED, vga256InfoRec.name, pages * 4);
   
   I810SysMem.Start = 0;
   I810SysMem.End = pages * 4096;
   I810SysMem.Size = pages * 4096;
   I810GttType = gtt_Kernel;

   vga256InfoRec.videoRam = I810SysMem.Size / 1024;

   return 0;
}


static void I810SetupFallbackGTT()
{
   unsigned int off, pte;

   OUTREG(PGETBL_CTL, (I810GttPhysical | PGETBL_ENABLED));

   /* - load GTT entries via the MMIO aperture 
    * - Use a dedicated scratch page.
    */
   for (off = 0, pte = 0 ; 
	pte < (I810GttSize * 1024) ;
	off += 4096, pte += 4) 
   {
      if (off < I810DcacheMem.End) {
	 OUTREG_(PTE_BASE + pte, off | PTE_LOCAL | PTE_VALID);
      } else if (off < I810SysMem.End) {
	 OUTREG_(PTE_BASE + pte,
		 (I810Physical + off - I810DcacheMem.End)
		 | PTE_MAIN_UNCACHED | PTE_VALID);
      } else {
	 OUTREG_(PTE_BASE + pte,
		 (I810Physical + I810ScratchPageLocal)
		 | PTE_MAIN_UNCACHED | PTE_VALID);
      }
   }
}


/* Work out the real top of physical memory (not just what it says in
 * /proc/meminfo).  Figure out the position and size of the
 * preallocated (stolen) video ram segment.  For now, this is all the
 * memory we will use.
 */
Bool I810CharacterizeSystemRam( pciConfigPtr pcr )
{
   pciTagRec tag = pcibusTag(pcr->_bus, pcr->_cardnum, pcr->_func);
   unsigned long mb = 0;
   unsigned long foo;
   unsigned long whtcfg_pamr_drp;
   unsigned long smram_miscc;
   int i;

   /* Need to do read longs, because read word returns rubbish...
    */
   whtcfg_pamr_drp = pciReadLong( tag, WHTCFG_PAMR_DRP );
   smram_miscc = pciReadLong( tag, SMRAM_MISCC );

   /* Need this for choosing watermarks.
    */
   if ((whtcfg_pamr_drp & LM_FREQ_MASK) == LM_FREQ_133)
      I810LmFreqSel = 133;
   else
      I810LmFreqSel = 100;


   if (I810AllocateGARTMemory() == 0) 
      return TRUE;
   else if (getenv("I810_UNSUPPORTED_GTT_FALLBACK"))
      ErrorF("%s %s: No kernel GTT support detected - trying to fall back\n",
	     XCONFIG_PROBED, vga256InfoRec.name);
   else
      FatalError("Couldn't get memory from gart module,\n"
		 "and I810_UNSUPPORTED_GTT_FALLBACK not set.");


   /* Fallback is useful for debugging, or if someone is unable to
    * compile the kernel module (eg. a 1.2.x kernel, or a non-linux
    * operating system).  However there are real drawbacks - there has
    * been little thought given to synchronization between two X
    * servers running on the same hardware, non-X users of the gart
    * (eg svga, fbdev, ggi), and when direct-rendering 3d clients
    * become available the same problems will arise there, too.
    *
    * Additionally, the X server is unable to allocate system memory,
    * so must cram everything into whatever 'stolen' memory was
    * reserved by the chipset at startup.  This is sneaky - the memory
    * isn't guarenteed to be present, and is not guarenteed to be
    * stable, either.
    *
    * Thus this should never be considered anything but a stopgap
    * measure, or tool for special circumstances where a kernel module
    * is unavailable.  
    */
   if ((smram_miscc & GFX_MEM_WIN_SIZE) == GFX_MEM_WIN_32M) 
      I810GttSize = 32;
   else
      I810GttSize = 64;

   ErrorF("%s %s: GTT window size: %ld mb\n", 
	  XCONFIG_PROBED, vga256InfoRec.name, I810GttSize);
   
   for ( i = 0 ; i < 2 ; i++ ) {
      char drp;
      int row = 0;

      switch (i) {
      case 0: drp = whtcfg_pamr_drp >> SYS_DRAM_ROW_0_SHIFT; break;
      case 1: drp = whtcfg_pamr_drp >> SYS_DRAM_ROW_1_SHIFT; break;
      }

      switch (drp & DRAM_MASK) {
      case 0x0: row = DRAM_VALUE_0; break;
      case 0x1: row = DRAM_VALUE_1; break;
	 /* no 0x2 value defined  */
      case 0x3: row = DRAM_VALUE_3; break;
      case 0x4: row = DRAM_VALUE_4; break;
      case 0x5: row = DRAM_VALUE_5; break;
      case 0x6: row = DRAM_VALUE_6; break;
      case 0x7: row = DRAM_VALUE_7; break;
      case 0x8: row = DRAM_VALUE_8; break;
      case 0x9: row = DRAM_VALUE_9; break;
      case 0xa: row = DRAM_VALUE_A; break;
      case 0xb: row = DRAM_VALUE_B; break;
      case 0xc: row = DRAM_VALUE_C; break;
      case 0xd: row = DRAM_VALUE_D; break;
      case 0xe: row = DRAM_VALUE_E; break;
      case 0xf: row = DRAM_VALUE_F; break;
      default:
	 FatalError("%s %s: Unrecognized system dram row size\n",
		    XCONFIG_PROBED, vga256InfoRec.name);
	 break;
      }

      mb += row;

      ErrorF("%s %s: System dram row %d, size %d mb\n", 
	     XCONFIG_PROBED, vga256InfoRec.name, i, row );
   }

   ErrorF("%s %s: Installed mainboard ram: %d mb\n", 
	  XCONFIG_PROBED, vga256InfoRec.name, mb);

   mb *= 1024*1024;

   /* Take into account memory reserved for TSEG, whatever that is.
    */
   switch (smram_miscc & USMM) {
   case USMM_TSEG_512K: mb -= 512 * 1024; break;
   case USMM_TSEG_1M: mb -= 1024 * 1024; break;
   default: break;
   }

   switch (smram_miscc & GMS) {
   case GMS_DISABLE: 
      ErrorF("i810 is disabled\n");
      return 0;
   case GMS_ENABLE_BARE:
      ErrorF("\nNo system ram reserved for i810, and no kernel GTT\n");
      return 0;
   case GMS_ENABLE_512K:
      I810SysMem.End = 512 * 1024;
      I810Physical = mb - 512 * 1024;
      ErrorF("%s %s: Only 512k system ram available for i810\n",
	     XCONFIG_PROBED, vga256InfoRec.name);
      break;
   case GMS_ENABLE_1M:
      I810SysMem.End = 1024 * 1024;
      I810Physical = mb - 1024 * 1024;
      ErrorF("%s %s: Only 1024k system ram available for i810\n",
	     XCONFIG_PROBED, vga256InfoRec.name);
      break;
   }

   /* Reserve space for the GTT and scratch page.
    */
   I810SysMem.End -= I810GttSize * 1024;
   I810GttPhysical = I810Physical + I810SysMem.End;
   I810SysMem.End -= 4096;
   I810ScratchPageLocal = I810SysMem.End;
   I810SysMem.Size = I810SysMem.End - I810SysMem.Start;


   /* Breifly map IO registers to virtual address space. */
   I810MMIOBase = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
				(pointer)(I810MMIOAddr), 0x80000);

   if (!I810MMIOBase) 
      FatalError("Couldn't map MMIO region");

   /* Dcache is too slow for normal use, but it's a way to get a
    * fullsized framebuffer in the fallback mode.  
    */
   if ((INREG8(DRAM_ROW_TYPE) & DRAM_ROW_0) == DRAM_ROW_0_SDRAM)
   {
      ErrorF("%s %s: Detected 4MB dedicated video ram\n",
	     XCONFIG_PROBED, vga256InfoRec.name);

      I810DcacheMem.Start = 0;
      I810DcacheMem.End = 4 * 1024 * 1024;
      I810DcacheMem.Size = I810DcacheMem.End;      
      I810SysMem.Start += I810DcacheMem.Size;
      I810SysMem.End += I810DcacheMem.Size;	       
      I810DisplayPtr = &I810DcacheMem;
   }

   vga256InfoRec.videoRam = (I810SysMem.End - I810DcacheMem.Start) / 1024;
   I810GttType = gtt_Local;
   
   I810SetupFallbackGTT();

   /* Unmap them again. */
   xf86UnMapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
		   (pointer)(I810MMIOAddr), 0x80000);
   return TRUE;
}


unsigned long I810LocalToPhysical( unsigned long local )
{
   switch (I810GttType) {
   case gtt_Local:
      if (I810DisplayPtr == &I810SysMem)
	 return I810Physical + local;
      else
	 return I810Physical + local - 4 * 1024 * 1024;
      break;
   case gtt_Kernel:
      gart_pg_inf.index = (local + 4095) / 4096;
      if (ioctl(gartfd, GARTIOCPGINFO, &gart_pg_inf) != 0) {
	 ErrorF("%s %s: error doing ioctl(GARTIOCINFO, %x): %s\n", 
		XCONFIG_PROBED, vga256InfoRec.name, gart_pg_inf.index,
		sys_errlist[errno]);
	 return 0;
      }
      return gart_pg_inf.physical + (local & 4095);
   default:
      return 0;
   }
}

int I810AllocLow( I810MemRange *result, I810MemRange *pool, int size )
{
   if (size > pool->Size) return 0;

   pool->Size -= size;
   result->Size = size;
   result->Start = pool->Start;
   result->End = pool->Start += size;
   return 1;
}

int I810AllocHigh( I810MemRange *result, I810MemRange *pool, int size )
{
   if (size > pool->Size) return 0;

   pool->Size -= size;
   result->Size = size;
   result->End = pool->End;
   result->Start = pool->End -= size;
   return 1;
}
