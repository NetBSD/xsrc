/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/sol8_x86/sol8_vid.c,v 1.2 2000/02/08 17:19:24 dawes Exp $ */
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
/* $XConsortium: sol8_vid.c /main/4 1996/02/21 17:54:20 kaleb $ */

#include "X.h"

#define _NEED_SYSI86
#include "xf86.h"
#include "xf86Priv.h"
#undef usleep
#include "xf86_OSlib.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

char *apertureDevName = NULL;

Bool
xf86LinearVidMem()
{

	int	mmapFd;

	apertureDevName = "/dev/xsvc";
	if ((mmapFd = open(apertureDevName, O_RDWR)) < 0) 
	{
		xf86Msg(X_WARNING,
			"xf86LinearVidMem: failed to open %s (%s)\n",
			apertureDevName, strerror(errno));
		xf86Msg(X_WARNING, 
			"xf86LinearVidMem: /dev/xsvc device driver required\n");
		xf86Msg(X_WARNING,
			"xf86LinearVidMem: linear memory access disabled\n");
		return FALSE;
	}
	close(mmapFd);
	return TRUE;

}

pointer
xf86MapVidMem(int ScreenNum, int Flags, unsigned long Base, unsigned long Size)
{
	pointer base;
	int fd;

	/* 
	 * Solaris 2.8 7/26/99
	 * Use /dev/xsvc for everything
	 *
         * DWH - 7/26/99 - Solaris8/dev/xsvc changes
	 */

	if (!apertureDevName)
		if (!xf86LinearVidMem())
			FatalError("xf86MapVidMem: Could not mmap "
				   "linear framebuffer [s=%x,a=%x]\n",
				   Size, Base);
	

	if ((fd = open(apertureDevName, O_RDWR,0)) < 0)
	{
		FatalError("xf86MapVidMem: failed to open %s (%s)\n",
			   apertureDevName, strerror(errno));
	}
	base = mmap((caddr_t)0, Size, PROT_READ|PROT_WRITE,
		     MAP_SHARED, fd, (off_t)Base);
	close(fd);
	if (base == MAP_FAILED)
	{
		FatalError("%s: Could not mmap framebuffer [s=%x,a=%x] (%s)\n",
			   "xf86MapVidMem", Size, Base, strerror(errno));
	}
	return(base);
}

/* ARGSUSED */
void
xf86UnMapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
	munmap(Base, Size);
}

/***************************************************************************/
/* I/O Permissions section                                                 */
/***************************************************************************/

static Bool ExtendedEnabled = FALSE;

void
xf86EnableIO()
{

	if (ExtendedEnabled)
		return;

	if (sysi86(SI86V86, V86SC_IOPL, PS_IOPL) < 0)
	{
		FatalError("%s: Failed to set IOPL for I/O\n",
			   "xf86EnableIOPorts");
	}

	ExtendedEnabled = TRUE;
	return;
}

void
xf86DisableIO()
{
	if(!ExtendedEnabled)
		return;

	sysi86(SI86V86, V86SC_IOPL, 0);

	ExtendedEnabled = FALSE;
	return;
}


/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/

Bool xf86DisableInterrupts()
{
	if (!ExtendedEnabled)
	{
		if (sysi86(SI86V86, V86SC_IOPL, PS_IOPL) < 0)
		{
			return(FALSE);
		}
	}

#ifdef __GNUC__
	__asm__ __volatile__("cli");
#else 
	asm("cli");
#endif /* __GNUC__ */

	if (!ExtendedEnabled)
	{
		sysi86(SI86V86, V86SC_IOPL, 0);
	}
	return(TRUE);
}

void xf86EnableInterrupts()
{
	if (!ExtendedEnabled)
	{
		if (sysi86(SI86V86, V86SC_IOPL, PS_IOPL) < 0)
		{
			return;
		}
	}

#ifdef __GNUC__
	__asm__ __volatile__("sti");
#else 
	asm("sti");
#endif /* __GNUC__ */

	if (!ExtendedEnabled)
	{
		sysi86(SI86V86, V86SC_IOPL, 0);
	}
	return;
}

void
xf86MapReadSideEffects(int ScreenNum, int Flags, pointer Base,
	unsigned long Size)
{
}

Bool
xf86CheckMTRR(int s)
{
	return FALSE;
}
