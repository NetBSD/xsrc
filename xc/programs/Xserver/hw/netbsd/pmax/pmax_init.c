/*
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andy Doran <ad@NetBSD.org>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* Um, we don't need all these... */
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/proc.h>

#include <machine/tc_machdep.h>
#include <machine/fbio.h>
#include <machine/fbvar.h>
#include <machine/pmioctl.h>
#include <dev/dec/lk201.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "X.h"
#include "Xproto.h"

#include "scrnintstr.h"
#include "servermd.h"
#include "input.h"

extern Bool	pmaxScreenInit();
extern int	pmaxMouseProc();
extern int	pmaxKeybdProc();

int pmaxScreenPrivateIndex;

static	PixmapFormatRec formats[] = {
	{1, 1, BITMAP_SCANLINE_PAD},     /* 1 bit deep */
	{8, 8, BITMAP_SCANLINE_PAD},     /* 8-bit deep */
};

void
InitOutput(screenInfo, argc, argv)
    ScreenInfo *screenInfo;
    int argc;
    char **argv;
{
    int i, fd, imageByteOrder, bitmapScanlineUnit, bitmapScanlinePad;
    int bitmapBitOrder, numPixmapFormats;
    struct fbinfo fb;

    screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    if ((fd = open("/dev/fb0", O_RDWR | O_NDELAY, 0)) < 0)
	ErrorF("couldn't open /dev/fb0\n");

    /* Damn DEC ioctls() aren't enough. Using the Sun ones too... */
    if (ioctl(fd, FBIOGTYPE, &fb) < 0) {
	ErrorF("FBIOGTYPE ioctl failed: %s\n", strerror(errno));
	exit(1);
    }
    
    /* XXX horrible... */
    close(fd);
    screenInfo->numPixmapFormats = (fb.fb_depth == 8 ? 2 : 1);

    for (i = 0; i < screenInfo->numPixmapFormats; i++) {
	screenInfo->formats[i].depth = formats[i].depth;
	screenInfo->formats[i].bitsPerPixel = formats[i].bitsPerPixel;
	screenInfo->formats[i].scanlinePad = formats[i].scanlinePad;
    }

    pmaxScreenPrivateIndex = AllocateScreenPrivateIndex();
    AddScreen(pmaxScreenInit, argc, argv);
}

void
InitInput(argc, argv)
    int argc;
    char *argv[];
{
    DeviceIntPtr p, k;
    
    p = AddInputDevice(pmaxMouseProc, TRUE);
    k = AddInputDevice(pmaxKeybdProc, TRUE);

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
}
