/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/Epson.c,v 1.1.2.2 1998/10/22 04:31:00 hohndel Exp $ */
/*
 * (c) Copyright 1998 by Thomas Mueller <tmueller@sysgo.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * THOMAS MUELLER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Thomas Mueller shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Thomas Mueller.
 *
 */

#include "Probe.h"

static Word Ports[] = {0x3CD, 0x3DE, 0x3DF, 0x3D0, 0x3D1, 0x3D2, 0x3D3,
		       ATR_IDX, ATR_REG_R, SEQ_IDX, SEQ_REG};
#define NUMPORTS (sizeof(Ports)/sizeof(Word))

static int MemProbe_Epson __STDCARGS((int));

Chip_Descriptor Epson_Descriptor = {
	"Epson",
	Probe_Epson,
	Ports,
	NUMPORTS,
	FALSE,
	FALSE,
	FALSE,
	MemProbe_Epson,
};

Bool Probe_Epson(Chipset)
int *Chipset;
{
	Bool result = FALSE;
	Byte old, val;
	
	EnableIOPorts(NUMPORTS, Ports);
        old = inp(0x3de);
        outp(0x3de, 0x08);
        val = inp(0x3df);
	if ((val & 0xe0) == 0xe0) {
		outp(0x3de, 0x0f);	/* secondary revision code */
		val = inp(0x3df);

		if ((val & 0xf8) == 0xa0) {
			*Chipset = CHIP_EPSON_8110;
			result = TRUE;
		}
	}
	outp(0x3de, old);

	DisableIOPorts(NUMPORTS, Ports);
      	return result;
}



static int MemProbe_Epson(Chipset)
int Chipset;
{
	int Mem = 0;

	EnableIOPorts(NUMPORTS, Ports);

	outp(0x3de, 3);
	if (inp(0x3df) & 0x40)
		Mem = 512;
	else
		Mem = 1024;

	DisableIOPorts(NUMPORTS, Ports);

	return(Mem);
}
