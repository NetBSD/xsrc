/**********************************************************************
Copyright 1998, 1999 by Precision Insight, Inc., Cedar Park, Texas.

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and
its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Precision Insight not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  Precision Insight
and its suppliers make no representations about the suitability of this
software for any purpose.  It is provided "as is" without express or 
implied warranty.

PRECISION INSIGHT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**********************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/neomagic/neo_bank.c,v 1.2 2000/04/04 19:25:12 dawes Exp $ */

/*
 * The original Precision Insight driver for
 * XFree86 v.3.3 has been sponsored by Red Hat.
 *
 * Authors:
 *   Jens Owen (jens@precisioninsight.com)
 *   Kevin E. Martin (kevin@precisioninsight.com)
 *
 * Port to Xfree86 v.4.0
 *   1998, 1999 by Egbert Eich (Egbert.Eich@Physik.TU-Darmstadt.DE)
 */

#define PSZ 8

/* All drivers should typically include these */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

/* Everything using inb/outb, etc needs "compiler.h" */
#include "compiler.h"

/* Driver specific headers */
#include "neo.h"

int
NEOSetReadWrite(ScreenPtr pScreen, int bank)
{ 
    unsigned char tmp;
    
    outb(0x3CE, 0x11);
    tmp = inb(0x3CF);
    outw(0x3CE, (( tmp & 0xFC ) << 8 ) | 0x11);
    outw(0x3CE, ((((bank << 2) & 0xFF) << 8) | 0x15));
    return 0;
}

int
NEOSetWrite(ScreenPtr pScreen, int bank)
{
    unsigned char tmp;
    
    outb(0x3CE, 0x11);
    tmp = inb(0x3CF);
    outw(0x3CE, ((( tmp & 0xFC ) | 0x01 ) << 8 ) | 0x11);
    outw(0x3CE, ((((bank << 2) & 0xFF) << 8) | 0x16));
    return 0;
}


int
NEOSetRead(ScreenPtr pScreen, int bank)
{
    unsigned char tmp;
    
    outb(0x3CE, 0x11);
    tmp = inb(0x3CF);
    outw(0x3CE, ((( tmp & 0xFC ) | 0x01 ) << 8 ) | 0x11);
    outw(0x3CE, ((((bank << 2) & 0xFF) << 8) | 0x15));
    return 0;
}








