/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/r128/r128.h,v 1.1.2.1 1999/10/11 21:13:49 hohndel Exp $ */
/**************************************************************************

Copyright 1999 ATI Technologies Inc. and Precision Insight, Inc.,
                                         Cedar Park, Texas. 
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Rickard E. Faith <faith@precisioninsight.com>
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *
 * $PI: xc/programs/Xserver/hw/xfree86/vga256/drivers/r128/r128.h,v 1.13 1999/10/08 08:28:46 faith Exp $
 */

#ifndef _R128_H_
#define _R128_H_

#define R128_DEBUG   0		/* Turn on debugging output                */
#define R128_TIMEOUT 2000000	/* Fall out of wait loops after this count */


#if R128_DEBUG
#define R128DEBUG(x) R128VERBOSE(x)
#else
#define R128DEBUG(x)
#endif

#define R128FATAL(x)                                                          \
    do {                                                                      \
        if (vga256InfoRec.chipset)                                            \
	    ErrorF("%s %s: %s: Fatal Error\n",                                \
		   XCONFIG_PROBED, vga256InfoRec.name, vga256InfoRec.chipset);\
	else                                                                  \
	    ErrorF("%s %s: r128: Fatal Error\n",                              \
		   XCONFIG_PROBED, vga256InfoRec.name);                       \
	FatalError x;                                                         \
    } while (0)

#define R128ERROR(x)                                                          \
    do {                                                                      \
        if (vga256InfoRec.chipset)                                            \
	    ErrorF("%s %s: %s: ",                                             \
		   XCONFIG_PROBED, vga256InfoRec.name, vga256InfoRec.chipset);\
	else                                                                  \
	    ErrorF("%s %s: r128: ", XCONFIG_PROBED, vga256InfoRec.name);      \
	ErrorF x;                                                             \
    } while (0)

#define R128VERBOSE(x) do { if (xf86Verbose) R128ERROR(x); } while (0)



/* Other macros */
#define R128_ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))
#define R128_ALIGN(x,bytes) (((x) + ((bytes) - 1)) & ~((bytes) - 1))

typedef struct {
    CARD16        reference_freq;
    CARD16        reference_div;
    CARD32        min_pll_freq;
    CARD32        max_pll_freq;
    CARD16        xclk;
} R128PLLRec, *R128PLLPtr;

typedef struct {
    unsigned int  Chipset;
    pciTagRec     PciTag;
    
    unsigned long LinearAddr;	/* Frame buffer physical address           */
    unsigned long MMIOAddr;	/* MMIO region physical address            */
    unsigned long IOBase;	/* I/O register base address               */

    unsigned char *MMIO;	/* Map of MMIO region                      */

    CARD32        MemCntl;
    int           Flags;	/* Saved copy of mode flags                */
    
    R128PLLRec    pll;

				/* Saved data from vga256InfoRec */
    int           pixel_bytes;
    int           pixel_depth;
    int           virtual_x;
    int           virtual_y;
    unsigned long video_ram;
    unsigned long cursor_start;
    unsigned long cursor_end;

				/* Computed values for Rage 128 */
    int           pitch;
    int           datatype;
    CARD32        dp_gui_master_cntl;

				/* Saved values for ScreenToScreenCopy */
    int           xdir;
    int           ydir;

				/* ScanlineScreenToScreenColorExpand support */
    unsigned char scratch_buffer[512];
    int           scanline_x;
    int           scanline_y;
    int           scanline_h_w;
    int           scanline_words;
} R128InfoRec, *R128InfoPtr;

extern int         INPLL(int addr);
extern R128InfoPtr R128PTR(void);
extern void        R128AccelInit(void);
extern void        R128CursorInit(void);
extern void        R128WaitForVerticalSync(void);

#endif
