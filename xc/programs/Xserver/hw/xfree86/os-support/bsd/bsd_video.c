/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsd_video.c,v 3.14.2.1 1997/05/03 09:47:11 dawes Exp $ */
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
/* $XConsortium: bsd_video.c /main/10 1996/10/25 11:37:57 kaleb $ */

#include "X.h"
#include "input.h"
#include "scrnintstr.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"

#if defined(__NetBSD__) && !defined(MAP_FILE)
#define MAP_FILE 0
#endif

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

#ifndef __mips__
#define _386BSD_MMAP_BUG
#endif

#ifdef _386BSD_MMAP_BUG
/*
 * Bug prevents multiple mappings, so just map a fixed region between 0xA0000
 * and 0xBFFFF, and return a pointer to the requested Base.
 */
static int MemMapped = FALSE;
static pointer MappedPointer = NULL;
static int MapCount = 0;
#define MAP_BASE 0xA0000
#define MAP_SIZE 0x20000
#endif

static Bool devMemChecked = FALSE;
static Bool useDevMem = FALSE;
static int  devMemFd = -1;

#if 0
static struct xf86memMap {
  int offset;
  int memSize;
} xf86memMaps[MAXSCREENS];
#endif

#ifdef HAS_APERTURE_DRV
#define DEV_APERTURE "/dev/xf86"
#endif
#define DEV_MEM "/dev/mem"

/*
 * Check if /dev/mem can be mmap'd.  If it can't print a warning when
 * "warn" is TRUE.
 */
static void checkDevMem(warn)
Bool warn;
{
	int fd;
	pointer base;

	devMemChecked = TRUE;
	if ((fd = open(DEV_MEM, O_RDWR)) >= 0)
	{
	    /* Try to map a page at the VGA address */
	    base = (pointer)mmap((caddr_t)0, 4096, PROT_READ|PROT_WRITE,
				 MAP_FILE, fd, (off_t)0xA0000);
	
	    if (base != (pointer)-1)
	    {
		munmap((caddr_t)base, 4096);
		devMemFd = fd;
		useDevMem = TRUE;
		return;
	    } else {
		/* This should not happen */
		if (warn)
		{
		    ErrorF("%s checkDevMem: warning: failed to mmap %s (%s)\n",
			   XCONFIG_PROBED, DEV_MEM, strerror(errno));
		}
		useDevMem = FALSE;
		return;
	    }
	}
#ifndef HAS_APERTURE_DRV
	if (warn)
	{ 
	    ErrorF("%s checkDevMem: warning: failed to open %s (%s)\n",
		   XCONFIG_PROBED, DEV_MEM, strerror(errno));
	    ErrorF("\tlinear framebuffer access unavailable\n");
	} 
	useDevMem = FALSE;
	return;
#else
	/* Failed to open /dev/mem, try the aperture driver */
	if ((fd = open(DEV_APERTURE, O_RDWR)) >= 0)
	{
	    /* Try to map a page at the VGA address */
	    base = (pointer)mmap((caddr_t)0, 4096, PROT_READ|PROT_WRITE,
			     MAP_FILE, fd, (off_t)0xA0000);
	
	    if (base != (pointer)-1)
	    {
		munmap((caddr_t)base, 4096);
		devMemFd = fd;
		useDevMem = TRUE;
		ErrorF("%s checkDevMem: using aperture driver %s\n",
		       XCONFIG_PROBED, DEV_APERTURE);
		return;
	    } else {

		if (warn)
		{
		    ErrorF("%s checkDevMem: warning: failed to mmap %s (%s)\n",
			   XCONFIG_PROBED, DEV_APERTURE, strerror(errno));
		}
	    }
	} else {
	    if (warn)
	    {
		ErrorF("%s checkDevMem: warning: failed to open %s and %s\n\t(%s)\n",
		   XCONFIG_PROBED, DEV_MEM, DEV_APERTURE, strerror(errno));
	    }
	}
	
	if (warn)
	{
	    ErrorF("\tlinear framebuffer access unavailable\n");
	}
	useDevMem = FALSE;
	return;

#endif
}


pointer xf86MapVidMem(ScreenNum, Region, Base, Size)
int ScreenNum;
int Region;
pointer Base;
unsigned long Size;
{
	pointer base;

	if (!devMemChecked)
		checkDevMem(FALSE);

	if (useDevMem)
	{
	    if (devMemFd < 0) 
	    {
		FatalError("xf86MapVidMem: failed to open %s (%s)\n",
			   DEV_MEM, strerror(errno));
	    }
	    base = (pointer)mmap((caddr_t)0, Size, PROT_READ|PROT_WRITE,
				 MAP_FILE, devMemFd,
				 (off_t)(unsigned long) Base);
	    if (base == (pointer)-1)
	    {
		FatalError("%s: could not mmap %s [s=%x,a=%x] (%s)\n",
			   "xf86MapVidMem", DEV_MEM, Size, Base, 
			   strerror(errno));
	    }
#if 0
	    xf86memMaps[ScreenNum].offset = (int) Base;
	    xf86memMaps[ScreenNum].memSize = Size;
#endif
	    return(base);
	}
		
	/* else, mmap /dev/vga */
#ifdef _386BSD_MMAP_BUG
	if ((unsigned long)Base < MAP_BASE ||
	    (unsigned long)Base >= MAP_BASE + MAP_SIZE)
	{
		FatalError("%s: Address 0x%x outside allowable range\n",
			   "xf86MapVidMem", Base);
	}
	if ((unsigned long)Base + Size > MAP_BASE + MAP_SIZE)
	{
		FatalError("%s: Size 0x%x too large (Base = 0x%x)\n",
			   "xf86MapVidMem", Size, Base);
	}
	if (!MemMapped)
	{
		base = (pointer)mmap(0, MAP_SIZE, PROT_READ|PROT_WRITE,
				     MAP_FILE, xf86Info.screenFd, 0);
		if (base == (pointer)-1)
		{
		    FatalError("xf86MapVidMem: Could not mmap /dev/vga (%s)\n",
			       strerror(errno));
		}
		MappedPointer = base;
		MemMapped = TRUE;
	}
	MapCount++;
	return((pointer)((unsigned long)MappedPointer +
			 ((unsigned long)Base - MAP_BASE)));

#else
#ifndef PC98
	if ((unsigned long)Base < 0xA0000 || (unsigned long)Base >= 0xC0000)
#else
	if ((unsigned long)Base < 0xA0000 || (unsigned long)Base >= 0xE8000)
#endif
	{
		FatalError("%s: Address 0x%x outside allowable range\n",
			   "xf86MapVidMem", Base);
	}
	base = (pointer)mmap(0, Size, PROT_READ|PROT_WRITE, MAP_FILE,
			     xf86Info.screenFd,
#ifdef __mips__
			     (unsigned long)Base);
#else
			     (unsigned long)Base - 0xA0000);
#endif
	if (base == (pointer)-1)
	{
	    FatalError("xf86MapVidMem: Could not mmap /dev/vga (%s)\n",
		       strerror(errno));
	}
#if 0
	xf86memMaps[ScreenNum].offset = (int) Base;
	xf86memMaps[ScreenNum].memSize = Size;
	return(base);
#endif
#endif
}

#if 0
void xf86GetVidMemData(ScreenNum, Base, Size)
int ScreenNum;
int *Base;
int *Size;      
{              
   *Base = xf86memMaps[ScreenNum].offset;
   *Size = xf86memMaps[ScreenNum].memSize;
}
#endif
                             
void xf86UnMapVidMem(ScreenNum, Region, Base, Size)
int ScreenNum;
int Region;
pointer Base;
unsigned long Size;
{
	if (useDevMem)
	{
		munmap((caddr_t)Base, Size);
		return;
	}

#ifdef _386BSD_MMAP_BUG
	if (MapCount == 0 || MappedPointer == NULL)
		return;

	if (--MapCount == 0)
	{
		munmap((caddr_t)MappedPointer, MAP_SIZE);
		MemMapped = FALSE;
	}
#else
	munmap((caddr_t)Base, Size);
#endif
}

Bool xf86LinearVidMem()
{
	/*
	 * Call checkDevMem even if already called by xf86MapVidMem() so that
	 * a warning about no linear fb is printed.
	 */
	if (!useDevMem)
		checkDevMem(TRUE);

	return(useDevMem);
}

#ifdef USE_I386_IOPL
/***************************************************************************/
/* I/O Permissions section                                                 */
/***************************************************************************/

static Bool ScreenEnabled[MAXSCREENS];
static Bool ExtendedEnabled = FALSE;
static Bool InitDone = FALSE;

void
xf86ClearIOPortList(ScreenNum)
int ScreenNum;
{
	if (!InitDone)
	{
		int i;
		for (i = 0; i < MAXSCREENS; i++)
			ScreenEnabled[i] = FALSE;
		InitDone = TRUE;
	}
	return;
}

void
xf86AddIOPorts(ScreenNum, NumPorts, Ports)
int ScreenNum;
int NumPorts;
unsigned *Ports;
{
	return;
}

void
xf86EnableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;

	ScreenEnabled[ScreenNum] = TRUE;

	if (ExtendedEnabled)
		return;

	if (i386_iopl(TRUE) < 0)
	{
		FatalError("%s: Failed to set IOPL for extended I/O\n",
			   "xf86EnableIOPorts");
	}
	ExtendedEnabled = TRUE;

	return;
}
	
void
xf86DisableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;

	ScreenEnabled[ScreenNum] = FALSE;

	if (!ExtendedEnabled)
		return;

	for (i = 0; i < MAXSCREENS; i++)
		if (ScreenEnabled[i])
			return;

	i386_iopl(TRUE);
	ExtendedEnabled = FALSE;

	return;
}


void xf86DisableIOPrivs()
{
	if (ExtendedEnabled)
		i386_iopl(FALSE);
	return;
}

#endif /* USE_I386_IOPL */


#ifdef USE_ARC_MMAP

static Bool ScreenEnabled[MAXSCREENS];
static Bool ExtendedEnabled = FALSE;
static Bool InitDone = FALSE;

void
xf86ClearIOPortList(ScreenNum)
int ScreenNum;
{
	if (!InitDone)
	{
		int i;
		for (i = 0; i < MAXSCREENS; i++)
			ScreenEnabled[i] = FALSE;
		InitDone = TRUE;
	}
	return;
}

void
xf86AddIOPorts(ScreenNum, NumPorts, Ports)
int ScreenNum;
int NumPorts;
unsigned *Ports;
{
	return;
}

void
xf86EnableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;
	int fd;
	pointer base;

	ScreenEnabled[ScreenNum] = TRUE;

	if (ExtendedEnabled)
		return;

	if ((fd = open("/dev/ttyC0", O_RDWR)) >= 0) {
		/* Try to map a page at the pccons I/O space */
		base = (pointer)mmap((caddr_t)0, 65536, PROT_READ|PROT_WRITE,
				MAP_FILE, fd, (off_t)0x0000);

		if (base != (pointer)-1) {
			IOPortBase = base;
		}
		else {
			ErrorF("EnableIOPorts: failed to mmap %s (%s)\n",
				"/dev/ttyC0", strerror(errno));
		}
	}
	else {
		ErrorF("EnableIOPorts: failed to open %s (%s)\n",
			"/dev/ttyC0", strerror(errno));
	}
	
	ExtendedEnabled = TRUE;

	return;
}

void
xf86DisableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;

	ScreenEnabled[ScreenNum] = FALSE;

	return;
}

void xf86DisableIOPrivs()
{
}

#endif /* USE_ARC_MMAP */

/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/

Bool xf86DisableInterrupts()
{

#if !defined(__mips__)
#ifdef __GNUC__
	__asm__ __volatile__("cli");
#else 
	asm("cli");
#endif /* __GNUC__ */
#endif /* __mips__ */

	return(TRUE);
}

void xf86EnableInterrupts()
{

#if !defined(__mips__)
#ifdef __GNUC__
	__asm__ __volatile__("sti");
#else 
	asm("sti");
#endif /* __GNUC__ */
#endif /* __mips__ */

	return;
}
