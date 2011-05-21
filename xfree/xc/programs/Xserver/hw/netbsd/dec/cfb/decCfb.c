/* $NetBSD: decCfb.c,v 1.3 2011/05/21 14:59:34 christos Exp $ */

/* XConsortium: sunCfb.c,v 1.15.1.2 95/01/12 18:54:42 kaleb Exp */
/* XFree86: xc/programs/Xserver/hw/sun/sunCfb.c,v 3.2 1995/02/12 02:36:22 dawes Exp */

/*
Copyright (c) 1990  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/*
 * Copyright (c) 1987 by the Regents of the University of California
 * Copyright (c) 1987 by Adam de Boor, UC Berkeley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/****************************************************************/
/* Modified from  sunCG4C.c for X11R3 by Tom Jarmolowski	*/
/****************************************************************/

/* 
 * Copyright 1991, 1992, 1993 Kaleb S. Keithley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  Kaleb S. Keithley makes no 
 * representations about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include "dec.h"
#include "cfb.h"
#include "cfb32.h"

static Bool decCfbScreenInit(ScreenPtr, pointer, int, int, int, int, int, int);

Bool decCFBInit (
    int	    	  screen,    	/* what screen am I going to be */
    ScreenPtr	  pScreen,  	/* The Screen to initialize */
    int	    	  argc,	    	/* The number of the Server's arguments. */
    char    	  **argv)   	/* The arguments themselves. Don't change! */
{
	unsigned char *fb;
	size_t sz;
	int stride;

	fb = decFbs[screen].fb;

	decFbs[screen].EnterLeave = (void (*)())NoopDDA;

    	if (!decScreenAllocate(pScreen))
		return FALSE;

	if (!fb) {
		sz = decFbs[screen].width * decFbs[screen].height *
		    decFbs[screen].depth / 8;
		if ((fb = decMemoryMap (sz, 0, decFbs[screen].fd)) == NULL)
			return FALSE;
	        decFbs[screen].fb = fb;
	}

	switch (decFbs[screen].type) {
	case WSDISPLAY_TYPE_MFB:
	    stride = 2048;
	    break;
	default:
	    stride = decFbs[screen].width;
	    break;
	}

	if (!decCfbScreenInit(pScreen, fb,
	    decFbs[screen].width,
	    decFbs[screen].height,
	    monitorResolution, monitorResolution,
	    stride,
	    decFbs[screen].depth)) {
            ErrorF("decCfbScreenInit failed\n");
            return FALSE;
	}

	decColormapScreenInit(pScreen);
	if (!decScreenInit(pScreen)) {
                ErrorF("decScreenInit failed\n");
		return FALSE;
	}

	(void) decSaveScreen(pScreen, SCREEN_SAVER_OFF);
	return cfbCreateDefColormap(pScreen);
}

Bool
decCfbSetupScreen(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize, int ysize,	/* in pixels */
    int dpix, int dpiy,		/* dots per inch */
    int width,			/* pixel width of frame buffer */
    int	bpp)			/* bits per pixel of root */
{
    switch (bpp) {
    case 32:
	if (!cfb32SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy,
	  width))
	    return FALSE;
	return cfbSetVisualTypes(24, 1 << TrueColor, 8);
    case 8:
	if (!cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy,
	  width))
	    return FALSE;
        if (decFbs[pScreen->myNum].type == WSDISPLAY_TYPE_MFB)
		return (cfbSetVisualTypes(1, 1 << StaticGray, 8));
	return TRUE;
    default:
	ErrorF("decCfbSetupScreen:  unsupported bpp = %d\n", bpp);
	return FALSE;
    }
}

Bool
decCfbFinishScreenInit(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize, int ysize,	/* in pixels */
    int dpix, int dpiy,		/* dots per inch */
    int width,			/* pixel width of frame buffer */
    int bpp)			/* bits per pixel of root */
{
    Bool retval;

    switch (bpp) {
    case 32:
	retval = cfb32FinishScreenInit(pScreen, pbits, xsize, ysize,
		    dpix, dpiy, width);

	/* XXXNJW cfb doesn't provide a way to tweak these, so cheat. */
	pScreen->visuals[0].redMask = 0xff0000;
	pScreen->visuals[0].greenMask = 0xff00;
	pScreen->visuals[0].blueMask = 0xff;

	pScreen->visuals[0].offsetRed = 16;
	pScreen->visuals[0].offsetGreen = 8;
	pScreen->visuals[0].offsetBlue = 0;

	break;
    case 8:
	retval = cfbFinishScreenInit(pScreen, pbits, xsize, ysize,
		    dpix, dpiy, width);
	break;
    default:
	retval = FALSE;
    }

    return retval;
}

static Bool
decCfbScreenInit(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize, int ysize,	/* in pixels */
    int dpix, int dpiy,		/* dots per inch */
    int width,			/* pixel width of frame buffer */
    int bpp)			/* bits per pixel of root */
{
    if (!decCfbSetupScreen(pScreen, pbits, xsize, ysize, dpix,
	dpiy, width, bpp))
	    return FALSE;
    return decCfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix,
	  dpiy, width, bpp);
}
