/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/arm_video.c,v 1.1 2002/08/06 13:08:38 herrb Exp $ */
/*
 * Copyright 1992 by Rich Murphey <Rich@Rice.edu>
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Rich Murphey and David Wexelblat 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Rich Murphey and
 * David Wexelblat make no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * RICH MURPHEY AND DAVID WEXELBLAT DISCLAIM ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL RICH MURPHEY OR DAVID WEXELBLAT BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * The ARM32 code here carries the following copyright:
 *
 * Copyright 1997
 * Digital Equipment Corporation. All rights reserved.
 * This software is furnished under license and may be used and copied only in 
 * accordance with the following terms and conditions.  Subject to these
 * conditions, you may download, copy, install, use, modify and distribute
 * this software in source and/or binary form. No title or ownership is
 * transferred hereby.
 *
 * 1) Any source code used, modified or distributed must reproduce and retain
 *    this copyright notice and list of conditions as they appear in the
 *    source file.
 *
 * 2) No right is granted to use any trade name, trademark, or logo of Digital 
 *    Equipment Corporation. Neither the "Digital Equipment Corporation"
 *    name nor any trademark or logo of Digital Equipment Corporation may be
 *    used to endorse or promote products derived from this software without
 *    the prior written permission of Digital Equipment Corporation.
 *
 * 3) This software is provided "AS-IS" and any express or implied warranties,
 *    including but not limited to, any implied warranties of merchantability,
 *    fitness for a particular purpose, or non-infringement are disclaimed.
 *    In no event shall DIGITAL be liable for any damages whatsoever, and in
 *    particular, DIGITAL shall not be liable for special, indirect,
 *    consequential, or incidental damages or damages for lost profits, loss
 *    of revenue or loss of use, whether such damages arise in contract, 
 *    negligence, tort, under statute, in equity, at law or otherwise, even
 *    if advised of the possibility of such damage. 
 *
 */

/* $XConsortium: bsd_video.c /main/10 1996/10/25 11:37:57 kaleb $ */

#include "X.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86OSpriv.h"

#define MAP_FLAGS MAP_SHARED

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif

#define BUS_BASE	0x80000000L
#define BUS_BASE_BWX	0x80000000L


/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

static Bool     useDevMem = FALSE;
static int      devMemFd = -1;

#define DEV_MEM "/dev/mem"

static pointer  mapVidMem(int, unsigned long, unsigned long, int);
static void     unmapVidMem(int, pointer, unsigned long);

/*
 * Check if /dev/mem can be mmap'd.  If it can't print a warning when
 * "warn" is TRUE.
 */
static void
checkDevMem(Bool warn)
{
	static Bool     devMemChecked = FALSE;
	int             fd;
	pointer         base;

	if (devMemChecked)
		return;
	devMemChecked = TRUE;

	if ((fd = open(DEV_MEM, O_RDWR)) >= 0) {
		/* Try to map a page at the VGA address */
		base = mmap((caddr_t) 0, 4096, PROT_READ | PROT_WRITE,
			    MAP_FLAGS, fd, (off_t) 0xA0000 + BUS_BASE);

		if (base != MAP_FAILED) {
			munmap((caddr_t) base, 4096);
			devMemFd = fd;
			useDevMem = TRUE;
			return;
		} else {
			/* This should not happen */
			if (warn) {
				xf86Msg(X_WARNING, "checkDevMem: failed to mmap %s (%s)\n",
					DEV_MEM, strerror(errno));
			}
			useDevMem = FALSE;
			return;
		}
	}
	if (warn) {
		xf86Msg(X_WARNING, "checkDevMem: failed to open %s (%s)\n",
			DEV_MEM, strerror(errno));
	}
	useDevMem = FALSE;
	return;
}

void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{

	checkDevMem(TRUE);
	pVidMem->linearSupported = useDevMem;
	pVidMem->mapMem = mapVidMem;
	pVidMem->unmapMem = unmapVidMem;

	pVidMem->initialised = TRUE;
}

static pointer         VidMemBase;

static          pointer
mapVidMem(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
	pointer         base;

	checkDevMem(FALSE);

#ifdef DEBUG
	xf86MsgVerb(X_INFO, 3, "mapVidMem %lx, %lx, %x, fd = %d\n", 
		    Base, Size, flags, devMemFd);
#endif

	if (useDevMem) {
		if (devMemFd < 0) {
			FatalError("xf86MapVidMem: failed to open %s (%s)\n",
				   DEV_MEM, strerror(errno));
		}
		base = mmap(NULL, Size, PROT_READ | PROT_WRITE,
			  MAP_FLAGS, devMemFd, (off_t) Base + BUS_BASE_BWX);
		if (base == MAP_FAILED) {
			FatalError("%s: could not mmap %s [s=%x,a=%x] (%s)\n",
				   "xf86MapVidMem", DEV_MEM, Size, Base,
				   strerror(errno));
		}
		/*
		 * arm is evil, and you need a double map to make the are not
		 * cached
		 */
		VidMemBase = mmap(NULL, Size, PROT_READ | PROT_WRITE,
			  MAP_FLAGS, devMemFd, (off_t) Base + BUS_BASE_BWX);
#ifdef DEBUG
		xf86MsgVerb(X_INFO, 3, "mapVidMem returning %p\n", base);
#endif

		return (base);
	}
	/* else, mmap /dev/vga */
	if ((unsigned long) Base < 0xA0000 || (unsigned long) Base >= 0xC0000) {
		FatalError("%s: Address 0x%x outside allowable range\n",
			   "xf86MapVidMem", Base);
	}
	base = mmap(NULL, Size, PROT_READ | PROT_WRITE, MAP_FLAGS,
		    xf86Info.screenFd,
		    (unsigned long) Base + BUS_BASE);
	if (base == MAP_FAILED) {
		FatalError("xf86MapVidMem: Could not mmap /dev/vga (%s)\n",
			   strerror(errno));
	}

	return (base);
}

static void
unmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
	munmap((caddr_t) Base, Size);
	if (VidMemBase != MAP_FAILED)
	    munmap((caddr_t) VidMemBase, Size);
	VidMemBase = MAP_FAILED;
}

/*
 * Read BIOS via mmap()ing DEV_MEM
 */

int
xf86ReadBIOS(unsigned long Base, unsigned long Offset, unsigned char *Buf,
	     int Len)
{
	unsigned char  *ptr;
	int             psize;
	int             mlen;

	checkDevMem(TRUE);
	if (devMemFd == -1) {
		return (-1);
	}
	psize = xf86getpagesize();
	Offset += Base & (psize - 1);
	Base &= ~(psize - 1);
	mlen = (Offset + Len + psize - 1) & ~(psize - 1);
	ptr = mmap(NULL, mlen, PROT_READ,
		   MAP_SHARED, devMemFd, (off_t) Base + BUS_BASE);
	if (ptr == MAP_FAILED) {
		xf86Msg(X_WARNING,
		      "xf86ReadBIOS: %s mmap[s=%x,a=%x,o=%x] failed (%s)\n",
			DEV_MEM, Len, Base, Offset, strerror(errno));
		return (-1);
	}
#ifdef DEBUG
	ErrorF("xf86ReadBIOS: BIOS at 0x%08x has signature 0x%04x\n",
	       Base, ptr[0] | (ptr[1] << 8));
#endif
	(void) memcpy(Buf, ptr + Offset, Len);
	(void) munmap(ptr, mlen);
#ifdef DEBUG
	xf86MsgVerb(X_INFO, 3, "xf86ReadBIOS(%x, %x, Buf, %x)"
		    "-> %02x %02x %02x %02x...\n",
		    Base, Offset, Len, Buf[0], Buf[1], Buf[2], Buf[3]);
#endif
	return (Len);
}

#define	DEV_MEM_IOBASE	0x7c000000	/* PCI I/O space */
#define	DEV_MEM_IOSIZE	0x00010000

static Bool     ExtendedEnabled = FALSE;
unsigned int IOPortBase;
static unsigned int IOPortBase2;

void
xf86EnableIO()
{
	pointer         base;
	pointer		base2;

	IOPortBase = (unsigned int) -1;
	IOPortBase2 = (unsigned int) -1;

	checkDevMem(TRUE);

	if (devMemFd >= 0 && useDevMem) {
		base = (pointer) mmap((caddr_t) 0, DEV_MEM_IOSIZE, PROT_READ | PROT_WRITE,
				MAP_FILE, devMemFd, (off_t) DEV_MEM_IOBASE);

		if (base != (pointer) - 1)
		{
			IOPortBase = (unsigned int) base;

			/* once more fight the arm vm system */
			base2 = (pointer) mmap((caddr_t) 0, DEV_MEM_IOSIZE, PROT_READ | PROT_WRITE,
				MAP_FILE, devMemFd, (off_t) DEV_MEM_IOBASE);
			if (base != (pointer) -1)
			{
			    IOPortBase2 = (unsigned int) base2;
			}
		}
	}
	if (IOPortBase == (unsigned int) -1 || IOPortBase2 == (unsigned int) -1 ) {
		FatalError("xf86EnableIO: failed to open mem device or map IO base. \n"
			"Make sure you have the Aperture Driver installed, "
			   "or a kernel built with the INSECURE option\n");
	}
	ExtendedEnabled = TRUE;

	return;
}

void
xf86DisableIO()
{
	if (!ExtendedEnabled)
		return;

	munmap((caddr_t) IOPortBase, DEV_MEM_IOSIZE);
	IOPortBase = (unsigned int) -1;

	munmap((caddr_t) IOPortBase2, DEV_MEM_IOSIZE);
	IOPortBase2 = (unsigned int) -1;

	ExtendedEnabled = FALSE;
}

Bool
xf86DisableInterrupts()
{
	return (TRUE);
}

void
xf86EnableInterrupts()
{
	return;
}
