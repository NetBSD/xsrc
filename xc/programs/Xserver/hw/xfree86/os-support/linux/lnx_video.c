/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/lnx_video.c,v 3.9 1996/08/20 12:29:40 dawes Exp $ */
/*
 * Copyright 1992 by Orest Zborowski <obz@Kodak.com>
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Orest Zborowski and David Wexelblat
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.  Orest Zborowski
 * and David Wexelblat make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * OREST ZBOROWSKI AND DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL OREST ZBOROWSKI OR DAVID WEXELBLAT BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: lnx_video.c /main/5 1995/12/17 08:30:27 kaleb $ */

#include "X.h"
#include "input.h"
#include "scrnintstr.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#ifdef __alpha__
extern unsigned long _bus_base(void) __attribute__((const));
#define BUS_BASE _bus_base()
#else
#define BUS_BASE 0
#endif

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

/*
 * Unfortunatly mmap without MAP_FIXED only works the first time :-(
 * This is now fixed in pl13 ALPHA, but still seems to have problems.
 */
#undef ONLY_MMAP_FIXED_WORKS

#ifdef ONLY_MMAP_FIXED_WORKS
static pointer AllocAddress[MAXSCREENS][NUM_REGIONS];
#endif

#if 0
static struct xf86memMap {
  int offset;
  int memSize;
} xf86memMaps[MAXSCREENS];
#endif

pointer xf86MapVidMem(ScreenNum, Region, Base, Size)
int ScreenNum;
int Region;
pointer Base;
unsigned long Size;
{
	pointer base;
      	int fd;

#ifdef ONLY_MMAP_FIXED_WORKS
	AllocAddress[ScreenNum][Region] = (pointer)xalloc(Size + 0x1000);
	if (AllocAddress[ScreenNum][Region] == NULL)
	{
		FatalError("xf86MapVidMem: can't alloc framebuffer space\n");
	}
	base = (pointer)(((unsigned int)AllocAddress[ScreenNum][Region]
			  & ~0xFFF) + 0x1000);
	if ((fd = open("/dev/mem", O_RDWR)) < 0)
	{
		FatalError("xf86MapVidMem: failed to open /dev/mem (%s)\n",
			   strerror(errno));
	}
	base = (pointer)mmap((caddr_t)base, Size, PROT_READ|PROT_WRITE,
			     MAP_FIXED|MAP_SHARED, fd, (off_t)Base);
#else
	if ((fd = open("/dev/mem", O_RDWR)) < 0)
	{
		FatalError("xf86MapVidMem: failed to open /dev/mem (%s)\n",
			   strerror(errno));
	}
	/* This requirers linux-0.99.pl10 or above */
	base = (pointer)mmap((caddr_t)0, Size, PROT_READ|PROT_WRITE,
			     MAP_SHARED, fd, (off_t)Base + BUS_BASE);
#endif
	close(fd);
	if ((long)base == -1)
	{
		FatalError("xf86MapVidMem: Could not mmap framebuffer (%s)\n",
			   strerror(errno));
	}
#if 0
	xf86memMaps[ScreenNum].offset = (int) Base;
	xf86memMaps[ScreenNum].memSize = Size;
#endif
	return base;
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
    munmap((caddr_t)Base, Size);
#ifdef ONLY_MMAP_FIXED_WORKS
    xfree(AllocAddress[ScreenNum][Region]);
#endif
}

Bool xf86LinearVidMem()
{
	return(TRUE);
}

/***************************************************************************/
/* I/O Permissions section                                                 */
/***************************************************************************/

/*
 * Linux handles regular (<= 0x3ff) ports with the TSS I/O bitmap, and
 * extended ports with the iopl() system call.
 *
 * For testing, it's useful to enable only the ports we need, but for
 * production purposes, it's faster to enable all ports.
 */
#define ALWAYS_USE_EXTENDED

#ifdef ALWAYS_USE_EXTENDED

static Bool ScreenEnabled[MAXSCREENS];
static Bool ExtendedEnabled = FALSE;
static Bool InitDone = FALSE;

void xf86ClearIOPortList(ScreenNum)
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

void xf86AddIOPorts(ScreenNum, NumPorts, Ports)
int ScreenNum;
int NumPorts;
unsigned *Ports;
{
	return;
}

void xf86EnableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;

	ScreenEnabled[ScreenNum] = TRUE;

	if (ExtendedEnabled)
		return;

	if (iopl(3))
		FatalError("%s: Failed to set IOPL for I/O\n",
			   "xf86EnableIOPorts");
	ExtendedEnabled = TRUE;

	return;
}

void xf86DisableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;

	ScreenEnabled[ScreenNum] = FALSE;

	if (!ExtendedEnabled)
		return;

	for (i = 0; i < MAXSCREENS; i++)
		if (ScreenEnabled[i])
			return;

	iopl(0);
	ExtendedEnabled = FALSE;

	return;
}

#else /* !ALWAYS_USE_EXTENDED */

static unsigned *EnabledPorts[MAXSCREENS];
static int NumEnabledPorts[MAXSCREENS];
static Bool ScreenEnabled[MAXSCREENS];
static Bool ExtendedPorts[MAXSCREENS];
static Bool ExtendedEnabled = FALSE;
static Bool InitDone = FALSE;

void xf86ClearIOPortList(ScreenNum)
int ScreenNum;
{
	if (!InitDone)
	{
		xf86InitPortLists(EnabledPorts, NumEnabledPorts,
				  ScreenEnabled, ExtendedPorts, MAXSCREENS);
		InitDone = TRUE;
		return;
	}
	ExtendedPorts[ScreenNum] = FALSE;
	if (EnabledPorts[ScreenNum] != (unsigned *)NULL)
		xfree(EnabledPorts[ScreenNum]);
	EnabledPorts[ScreenNum] = (unsigned *)NULL;
	NumEnabledPorts[ScreenNum] = 0;
}

void xf86AddIOPorts(ScreenNum, NumPorts, Ports)
int ScreenNum;
int NumPorts;
unsigned *Ports;
{
	int i;

	if (!InitDone)
	{
	    FatalError("xf86AddIOPorts: I/O control lists not initialised\n");
	}
	EnabledPorts[ScreenNum] = (unsigned *)xrealloc(EnabledPorts[ScreenNum],
			(NumEnabledPorts[ScreenNum]+NumPorts)*sizeof(unsigned));
	for (i = 0; i < NumPorts; i++)
	{
		EnabledPorts[ScreenNum][NumEnabledPorts[ScreenNum] + i] =
								Ports[i];
		if (Ports[i] > 0x3FF)
			ExtendedPorts[ScreenNum] = TRUE;
	}
	NumEnabledPorts[ScreenNum] += NumPorts;
}

void xf86EnableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;

	if (ScreenEnabled[ScreenNum])
		return;

	for (i = 0; i < MAXSCREENS; i++)
	{
		if (ExtendedPorts[i] && (ScreenEnabled[i] || i == ScreenNum))
		{
		    if (iopl(3))
		    {
			FatalError("%s: Failed to set IOPL for extended I/O\n",
				   "xf86EnableIOPorts");
		    }
		    ExtendedEnabled = TRUE;
		    break;
		}
	}
	/* Extended I/O was used, but not any more */
	if (ExtendedEnabled && i == MAXSCREENS)
	{
		iopl(0);
		ExtendedEnabled = FALSE;
	}
	/*
	 * Turn on non-extended ports even when using extended I/O
	 * so they are there if extended I/O gets turned off when it's no
	 * longer needed.
	 */
	for (i = 0; i < NumEnabledPorts[ScreenNum]; i++)
	{
		unsigned port = EnabledPorts[ScreenNum][i];

		if (port > 0x3FF)
			continue;

		if (xf86CheckPorts(port, EnabledPorts, NumEnabledPorts,
				   ScreenEnabled, MAXSCREENS))
		{
		    if (ioperm(port, 1, TRUE) < 0)
		    {
			FatalError("%s: Failed to enable I/O port 0x%x (%s)\n",
				   "xf86EnableIOPorts", port, strerror(errno));
		    }
		}
	}
	ScreenEnabled[ScreenNum] = TRUE;
	return;
}

void xf86DisableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;

	if (!ScreenEnabled[ScreenNum])
		return;

	ScreenEnabled[ScreenNum] = FALSE;
	for (i = 0; i < MAXSCREENS; i++)
	{
		if (ScreenEnabled[i] && ExtendedPorts[i])
			break;
	}
	if (ExtendedEnabled && i == MAXSCREENS)
	{
		iopl(0);
		ExtendedEnabled = FALSE;
	}
	for (i = 0; i < NumEnabledPorts[ScreenNum]; i++)
	{
		unsigned port = EnabledPorts[ScreenNum][i];

		if (port > 0x3FF)
			continue;

		if (xf86CheckPorts(port, EnabledPorts, NumEnabledPorts,
				   ScreenEnabled, MAXSCREENS))
		{
			ioperm(port, 1, FALSE);
		}
	}
	return;
}

#endif /* ALWAYS_USE_EXTENDED */

void xf86DisableIOPrivs()
{
	if (ExtendedEnabled)
		iopl(0);
	return;
}

/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/

Bool xf86DisableInterrupts()
{
	if (!ExtendedEnabled)
		if (iopl(3))
			return (FALSE);
#if defined(__alpha__) || defined(__mc68000__)
#else
#ifdef __GNUC__
	__asm__ __volatile__("cli");
#else
	asm("cli");
#endif
#endif
	if (!ExtendedEnabled)
		iopl(0);
	return (TRUE);
}

void xf86EnableInterrupts()
{
	if (!ExtendedEnabled)
		if (iopl(3))
			return;
#if defined(__alpha__) || defined(__mc68000__)
#else
#ifdef __GNUC__
	__asm__ __volatile__("sti");
#else
	asm("sti");
#endif
#endif
	if (!ExtendedEnabled)
		iopl(0);
	return;
}
