/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i740/i740_cmap.c,v 1.1.2.1 1999/04/15 11:40:33 hohndel Exp $ */
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
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *
 * $PI: i740_cmap.c,v 1.2 1999/02/18 20:50:59 martin Exp martin $
 */

#include "X.h"
#include "Xproto.h"
#include "windowstr.h"
#include "compiler.h"

#include "xf86.h"
#include "vga.h"

#ifdef XFreeXDGA
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

/* Modified from vgaStoreColors to use gamma correction */

void
I740StoreColors(pmap, ndef, pdefs)
    ColormapPtr	pmap;
    int		ndef;
    xColorItem	        *pdefs;
{
    int		i;
    unsigned char *cmap, *tmp;
    xColorItem	directDefs[256];
    Bool          new_overscan = FALSE;
    unsigned char overscan = ((vgaHWPtr)vgaNewVideoState)->Attribute[OVERSCAN];
    unsigned char tmp_overscan;
    extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];

    if (vgaCheckColorMap(pmap))
        return;

    if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
        ndef = cfbExpandDirectColors (pmap, ndef, pdefs, directDefs);
        pdefs = directDefs;
    }

    for(i = 0; i < ndef; i++) {
        if (pdefs[i].pixel == overscan) {
	    new_overscan = TRUE;
	}
        cmap = &((vgaHWPtr)vgaNewVideoState)->DAC[pdefs[i].pixel*3];
        cmap[0] = xf86rGammaMap[pdefs[i].red   >> 8];
        cmap[1] = xf86gGammaMap[pdefs[i].green >> 8];
        cmap[2] = xf86bGammaMap[pdefs[i].blue  >> 8];
#ifndef PC98_EGC
#ifndef PC98_NEC480
	if (!vgaDAC8BitComponents) {
            cmap[0] >>= 2;
            cmap[1] >>= 2;
            cmap[2] >>= 2;
        }
#endif /* PC98_NEC480 */
#else
        cmap[0] >>= 4;
        cmap[1] >>= 4;
        cmap[2] >>= 4;
#endif /* PC98_EGC */

        if (xf86VTSema
#ifdef XFreeXDGA
	    || ((vga256InfoRec.directMode & XF86DGADirectGraphics)
	        && !(vga256InfoRec.directMode & XF86DGADirectColormap))
	    || (vga256InfoRec.directMode & XF86DGAHasColormap)
#endif
	    ) {
#if !defined(PC98_EGC) && !defined(PC98_NEC480)
	    outb(0x3C8, pdefs[i].pixel);
	    DACDelay;
	    outb(0x3C9, cmap[0]);
	    DACDelay;
	    outb(0x3C9, cmap[1]);
	    DACDelay;
	    outb(0x3C9, cmap[2]);
	    DACDelay;
#else
	    /* also, PC9821Ne */
	    outb(0xa8, pdefs[i].pixel);
	    outb(0xac, cmap[0]);
	    outb(0xaa, cmap[1]);
	    outb(0xae, cmap[2]);
#endif /* PC98_EGC */
	}
    }	
    if (new_overscan) {
	new_overscan = FALSE;
        for(i = 0; i < ndef; i++) {
            if (pdefs[i].pixel == overscan) {
	        if ((pdefs[i].red != 0) || 
	            (pdefs[i].green != 0) || 
	            (pdefs[i].blue != 0)) {
	            new_overscan = TRUE;
		    tmp_overscan = overscan;
        	    tmp = &((vgaHWPtr)vgaNewVideoState)->DAC[pdefs[i].pixel*3];
	        }
	        break;
	    }
        }
        if (new_overscan) {
            /*
             * Find a black pixel, or the nearest match.
             */
            for (i=255; i >= 0; i--) {
                cmap = &((vgaHWPtr)vgaNewVideoState)->DAC[i*3];
	        if ((cmap[0] == 0) && (cmap[1] == 0) && (cmap[2] == 0)) {
	            overscan = i;
	            break;
	        } else {
		    /* Better way to find optimal overscan color, KEM */
		    if ((cmap[0]+cmap[1]+cmap[2]) < (tmp[0]+tmp[1]+tmp[2])) {
		        tmp = cmap;
		        tmp_overscan = i;
	            }
	        }
	    }
	    if (i < 0) {
	        overscan = tmp_overscan;
	    }
	    ((vgaHWPtr)vgaNewVideoState)->Attribute[OVERSCAN] = overscan;
            if (xf86VTSema
#ifdef XFreeXDGA
	        || ((vga256InfoRec.directMode & XF86DGADirectGraphics)
	            && !(vga256InfoRec.directMode & XF86DGADirectColormap))
	        || (vga256InfoRec.directMode&XF86DGAHasColormap)
#endif
		) {
#ifndef PC98_EGC
	        (void)inb(vgaIOBase + 0x0A);
	        outb(0x3C0, OVERSCAN);
	        outb(0x3C0, overscan);
	        (void)inb(vgaIOBase + 0x0A);
	        outb(0x3C0, 0x20);
#endif
	    }
        }
    }
}
