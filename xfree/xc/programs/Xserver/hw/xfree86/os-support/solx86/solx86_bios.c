/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/solx86/solx86_bios.c,v 1.6 2000/11/19 16:38:07 tsi Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell and David Wexelblat 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Thomas Roell and
 * David Wexelblat makes no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THOMAS ROELL AND DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL THOMAS ROELL OR DAVID WEXELBLAT BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "X.h"

#define _NEED_SYSI86
#include "xf86.h"
#include "xf86Priv.h"
#undef usleep
#include "xf86_OSlib.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

extern char *apertureDevName;

/*
 * Read BIOS via mmap()ing physical memory.
 */
int
xf86ReadBIOS(unsigned long Base, unsigned long Offset, unsigned char *Buf,
	     int Len)
{
	int fd;
	unsigned char *ptr;
	char	solx86_vtname[20];
	int psize;
	int mlen;

	/*
     	 * Solaris 2.1 x86 SVR4 (10/27/93)
     	 *      The server must treat the virtual terminal device file
     	 *      as the standard SVR4 /dev/pmem. By default, then used VT
     	 *      is considered the "default" file to open.
     	 */
	psize = xf86getpagesize();
	Offset += Base & (psize - 1);
	Base &= ~(psize - 1);
	mlen = (Offset + Len + psize - 1) & ~(psize - 1);
	if (Base >= 0xA0000 && Base + mlen < 0xFFFFF && xf86Info.vtno >= 0)
        	sprintf(solx86_vtname,"/dev/vt%02d",xf86Info.vtno);
	else
	{
		if (!apertureDevName)
			if (!xf86LinearVidMem())
				FatalError("xf86ReadBIOS: Could not mmap "
					   "BIOS [a=%x]\n", Base);
		sprintf(solx86_vtname, apertureDevName);
	}

	if ((fd = open(solx86_vtname, O_RDONLY)) < 0)
    	{
        	xf86Msg(X_WARNING, "xf86ReadBIOS: Failed to open %s (%s)\n",
			solx86_vtname, strerror(errno));
        	return(-1);
	}	
	ptr = (unsigned char *)mmap((caddr_t)0, mlen, PROT_READ,
					MAP_SHARED, fd, (off_t)Base);
	if (ptr == MAP_FAILED)
	{
		xf86Msg(X_WARNING, "xf86ReadBIOS: %s mmap failed "
			"[0x%05x, 0x%04x]\n",
			solx86_vtname, Base, mlen);
		close(fd);
		return(-1);
	}
	(void)memcpy(Buf, (void *)(ptr + Offset), Len);
	(void)munmap((caddr_t)ptr, mlen);
	(void)close(fd);
	return(Len);
}

