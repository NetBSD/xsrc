/*
 *      Copyright 2001  Ani Joshi <ajoshi@unixbox.com>
 * 
 *      XFree86 4.x driver for S3 chipsets
 * 
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation and
 * that the name of Ani Joshi not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Ani Joshi makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *                 
 * ANI JOSHI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ANI JOSHI BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/s3/s3_bios.c,v 1.2 2001/07/11 07:45:35 alanh Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "compiler.h"

#include "s3.h"


static unsigned char *find_bios_string(int BIOSbase, char *match1, char *match2)
{
	static unsigned char bios[BIOS_BSIZE];
	static int init=0;
	int i, j, l1, l2;

	if (!init) {
		init = 1;
		if (xf86ReadBIOS(BIOSbase, 0, bios, BIOS_BSIZE) != BIOS_BSIZE)
			return NULL;
		if ((bios[0] != 0x55) || (bios[1] != 0xaa))
			return NULL;
	}
	if (match1 == NULL)
		return NULL;

	l1 = strlen(match1);
	if (match2 != NULL)
		l2 = strlen(match2);
	else
		l2 = 0;

	for (i=0; i<BIOS_BSIZE; i++)
		if (bios[i] == match1[0] && !memcmp(&bios[i], match1, l1)) {
			if (match2 == NULL)
				return &bios[i+l1];
			else
				for(j=i+l1; (j<BIOS_BSIZE-l2) && bios[j]; j++)
					if (bios[j] == match2[0] &&
					    !memcmp(&bios[j], match2, l2))
						return &bios[j+l2];
		}

	return NULL;
}


int S3GetRefClock(ScrnInfoPtr pScrn)
{
	int RefClock = 16000;	/* default */

	if (find_bios_string(BIOS_BASE, "Number Nine Visual Technology",
					"Motion 771") != NULL)
		RefClock = 16000;

	return RefClock;
}
