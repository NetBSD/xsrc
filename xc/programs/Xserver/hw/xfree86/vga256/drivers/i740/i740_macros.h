/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i740/i740_macros.h,v 1.1.2.1 1999/04/15 11:40:35 hohndel Exp $ */
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
 * $PI: i740_macros.h,v 1.6 1999/02/18 20:50:59 martin Exp martin $
 */

#define	WAIT_ENGINE_IDLE() {						\
	    outb(XRX, BITBLT_CNTL);					\
	    while (inb(XRX+1) & BITBLT_STATUS)				\
		outb(XRX, BITBLT_CNTL);					\
	}

#define WAIT_BLT_IDLE() {						\
	    while (INREG(BITBLT_CONTROL) & BLTR_STATUS);		\
	}

#define WAIT_LP_FIFO(n) {						\
	    while (INREG8(LP_FIFO_COUNT) > 15-(n));			\
	}

#define WAIT_HP_FIFO(n) {						\
	    while (INREG8(HP_FIFO_COUNT) > 15-(n));			\
	}
