/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/i810_wmark.c,v 1.1.2.2 1999/11/18 19:06:18 hohndel Exp $ */
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
 */

#include "vga.h"
#include "i810.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"


struct wm_info {
   double freq;
   unsigned int wm;
};


struct wm_info i810_wm_8_100[] = {
   { 0, 0x22002000 },
   { 25, 0x22002000 },
   { 31, 0x22003000 },
   { 35, 0x22003000 },
   { 36, 0x22003000 },
   { 40, 0x22003000 },
   { 43, 0x22004000 },
   { 45, 0x22004000 },
   { 49, 0x22004000 },
   { 50, 0x22004000 },
   { 56, 0x22004000 },
   { 65, 0x22005000 },
   { 75, 0x22005000 },
   { 78, 0x22006000 },
/*     { 80, 0x2220c000 }, */
   { 94, 0x22007000 },
   { 96, 0x22107000 },
   { 99, 0x22107000 },
   { 108, 0x22107000 },
   { 129, 0x22107000 },
   { 132, 0x22109000 },
   { 135, 0x22109000 },
   { 157, 0x2210b000 },
   { 162, 0x2210b000 },
   { 189, 0x2220e000 },
   { 195, 0x2220e000 },
   { 202, 0x2220e000 },
};

struct wm_info i810_wm_16_100[] = {
   { 0, 0x22004000 },
   { 25, 0x22004000 },
   { 28, 0x22004000 },
   { 31, 0x22005000 },
   { 33, 0x22005000 },
   { 35, 0x22004000 },
   { 36, 0x22005000 },
   { 40, 0x22006000 },
   { 43, 0x22006000 },
   { 45, 0x22007000 },
   { 49, 0x22007000 },
   { 50, 0x22007000 },
   { 56, 0x22108000 },
   { 65, 0x22109000 },
   { 75, 0x2210a000 },
   { 78, 0x2210b000 },
   { 80, 0x22210000 },
   { 94, 0x2220e000 },
   { 96, 0x22210000 },
   { 99, 0x22210000 },
   { 108, 0x22210000 },
   { 121, 0x22210000 },
   { 129, 0x22210000 },
   { 132, 0x22314000 },
   { 135, 0x22314000 },
   { 157, 0x22415000 },
   { 162, 0x22416000 },
   { 189, 0x22416000 },
   { 195, 0x22416000 },
   { 202, 0x22416000 }
};


struct wm_info i810_wm_24_100[] = {
   { 0,  0x22006000 },
   { 25, 0x22006000 },
   { 28, 0x22006000 },
   { 31, 0x22007000 },
   { 35, 0x22008000 },
   { 40, 0x22008000 },
   { 40, 0x22108000 },
   { 49, 0x2210b000 },
   { 65, 0x2220d000 },
   { 75, 0x2220f000 },
   { 78, 0x22210000 },
   { 80, 0x22415000 },
   { 96, 0x22415000 },
   { 99, 0x22415000 },
   { 108, 0x22415000 },
   { 121, 0x22415000 },
   { 129, 0x22419000 },
   { 132, 0x22519000 },
   { 135, 0x2251d000 },
   { 157, 0x2251d000 },
   { 162, 0x4441d000 },
   { 189, 0x4441d000 },
   { 195, 0x4441d000 },
   { 202, 0x4441d000 },
};

struct wm_info i810_wm_32_100[] = {
   { 0, 0x2210b000 },
   { 60, 0x22415000 },		/* 0x314000 works too */
   { 80, 0x22419000 }           /* 0x518000 works too */
};


struct wm_info i810_wm_8_133[] = {
   { 0, 0x0070c000 },
   { 15, 0x0070c000 },
   { 25, 0x22002000 },
   { 28, 0x22002000 },
   { 31, 0x22003000 },
   { 35, 0x22003000 },
   { 36, 0x22003000 },
   { 40, 0x22003000 },
   { 43, 0x22004000 },
   { 45, 0x22004000 },
   { 49, 0x22004000 },
   { 50, 0x22004000 },
   { 56, 0x22004000 },
   { 65, 0x22005000 },
   { 75, 0x22005000 },
   { 78, 0x22006000 },
   { 80, 0x2220c000 },
   { 94, 0x22007000 },
   { 96, 0x22107000 },
   { 99, 0x22107000 },
   { 108, 0x22107000 },
   { 121, 0x2220c000 },
   { 129, 0x22107000 },
   { 132, 0x22109000 },
   { 135, 0x22109000 },
   { 157, 0x2210b000 },
   { 162, 0x2210b000 },
   { 189, 0x2220e000 },
   { 195, 0x2220e000 },
   { 202, 0x2220e000 }
};


struct wm_info i810_wm_16_133[] = {
   { 0, 0x22004000 },
   { 25, 0x22004000 },
   { 28, 0x22004000 },
   { 31, 0x22005000 },
   { 33, 0x22005000 },
   { 35, 0x22004000 },
   { 36, 0x22005000 },
   { 40, 0x22004000 },
   { 40, 0x22006000 },
   { 43, 0x22006000 },
   { 45, 0x22007000 },
   { 49, 0x22006000 },
   { 49, 0x22007000 },
   { 50, 0x22007000 },
   { 56, 0x22108000 },
   { 65, 0x22109000 },
   { 75, 0x2210a000 },
   { 78, 0x2210b000 },
   { 80, 0x22210000 },
   { 94, 0x2220e000 },
   { 96, 0x22210000 },
   { 99, 0x22210000 },
   { 108, 0x22210000 },
   { 121, 0x22210000 },
   { 129, 0x22210000 },
   { 132, 0x22314000 },
   { 135, 0x22314000 },
   { 157, 0x22415000 },
   { 162, 0x22416000 },
   { 189, 0x22416000 },
   { 195, 0x22416000 },
   { 202, 0x22416000 },
};

struct wm_info i810_wm_24_133[] = {
   { 0, 0x408000 },
   { 15, 0x408000 },
   { 19, 0x408000 },
   { 25, 0x20c000 },
   { 25, 0x40a000 },
   { 25, 0x22006000 },
   { 28, 0x22005000 },
   { 28, 0x22006000 },
   { 31, 0x22007000 },
   { 33, 0x22006000 },
   { 35, 0x22008000 },
   { 36, 0x22107000 },
   { 40, 0x22008000 },
   { 40, 0x22108000 },
   { 43, 0x22008000 },
   { 45, 0x2210a000 },
   { 49, 0x22009000 },
   { 49, 0x2210b000 },
   { 50, 0x2210a000 },
   { 56, 0x2210b000 },
   { 65, 0x2220d000 },
   { 75, 0x2220f000 },
   { 78, 0x22210000 },
   { 80, 0x22415000 },
   { 94, 0x22212000 },
   { 96, 0x22415000 },
   { 99, 0x22415000 },
   { 108, 0x22415000 },
   { 121, 0x22415000 },
   { 129, 0x22419000 },
   { 132, 0x22515000 },
   { 135, 0x2251c000 },
   { 157, 0x2251d000 },
   { 162, 0x44419000 },
   { 189, 0x44419000 },
   { 195, 0x44419000 },
   { 202, 0x44419000 },
};


struct wm_info i810_wm_32_133[] = {
   { 0, 0x2210b000 },
   { 60, 0x22415000 },	
   { 80, 0x22419000 }   
};


#define Elements(x) (sizeof(x)/sizeof(*x))

/*
 * I810CalcFIFO --
 *
 * Calculate burst length and FIFO watermark.
 */
unsigned int I810CalcWatermark( double freq, int local_mem_freq )
{
   struct wm_info *tab;
   int nr;
   int i;

   if (local_mem_freq == 100) {
      switch(vgaBitsPerPixel) {
      case 8:
	 tab = i810_wm_8_100;
	 nr = Elements(i810_wm_8_100);
	 break;
      case 16:
	 tab = i810_wm_16_100;
	 nr = Elements(i810_wm_16_100);
	 break;
      case 24:
	 tab = i810_wm_24_100;
	 nr = Elements(i810_wm_24_100);
	 break;
      case 32:
	 tab = i810_wm_32_100;
	 nr = Elements(i810_wm_32_100);
	 break;
      }
   } else {
      switch(vgaBitsPerPixel) {
      case 8:
	 tab = i810_wm_8_133;
	 nr = Elements(i810_wm_8_133);
	 break;
      case 16:
	 tab = i810_wm_16_133;
	 nr = Elements(i810_wm_16_133);
	 break;
      case 24:
	 tab = i810_wm_24_133;
	 nr = Elements(i810_wm_24_133);
	 break;
      case 32:
	 tab = i810_wm_32_133;
	 nr = Elements(i810_wm_32_133);
	 break;
      }
   }

   for (i = 0 ; i < nr && tab[i].freq < freq ; i++);
   
   if (i == nr)
      i--;

   ErrorF("%s %s: chose watermark 0x%x: (tab.freq %.1f)\n",
	  XCONFIG_PROBED, vga256InfoRec.name, tab[i].wm, tab[i].freq);

   /* None of these values (sourced from intel) have watermarks for
    * the dcache memory.  Fake it for now by using the same watermark
    * for both...  
    *
    * Update: this is probably because dcache isn't real useful as
    * framebuffer memory, so intel's drivers don't need watermarks
    * for that memory because they never use it to feed the ramdacs.
    * We do use it in the fallback mode, so keep the watermarks for
    * now.
    */
   if (I810DisplayPtr == &I810DcacheMem)
      return (tab[i].wm & ~0xffffff) | ((tab[i].wm>>12) & 0xfff);
   else
      return tab[i].wm;
}

