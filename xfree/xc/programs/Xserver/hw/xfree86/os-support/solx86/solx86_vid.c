/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/solx86/solx86_vid.c,v 3.16 1999/12/13 02:32:19 robin Exp $ */
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
/* $XConsortium: solx86_vid.c /main/4 1996/02/21 17:54:20 kaleb $ */

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
xf86CheckMTRR(int ScreenNum)
{
	return FALSE;
}

Bool
xf86LinearVidMem()
{

	int	mmapFd;

	apertureDevName = "/dev/xsvc";
	if ((mmapFd = open(apertureDevName, O_RDWR)) < 0) 
	{
	    apertureDevName = "/dev/fbs/aperture";
	    if((mmapFd = open(apertureDevName, O_RDWR)) < 0)
	    {
		xf86Msg(X_WARNING,
			"xf86LinearVidMem: failed to open %s (%s)\n",
			apertureDevName, strerror(errno));
		xf86Msg(X_WARNING, "xf86LinearVidMem: either /dev/fbs/aperture, or /dev/xsvc device "
				   "driver required\n");
		xf86Msg(X_WARNING,
			"xf86LinearVidMem: linear memory access disabled\n");
		return FALSE;
	    } 
	}
	close(mmapFd);
	return TRUE;

}

pointer
xf86MapVidMem(int ScreenNum, int Flags, unsigned long Base, unsigned long Size)
{
	pointer base;
	int fd;
	char solx86_vtname[20];

	/* 
	 * Solaris 2.1 x86 SVR4 (10/27/93)
	 *		The server must treat the virtual terminal device file 
	 *		as the standard SVR4 /dev/pmem. 
	 * 
	 * Uning the /dev/vtXX device as /dev/pmem only works for the
	 * A0000-FFFFF region - If we wish you mmap the linear aperture
 	 * it requires a device driver.
 	 * 
	 * So what we'll do is use /dev/vtXX for the A0000-FFFFF stuff, and
	 * try to use the /dev/fbs/aperture or /dev/xsvc driver if the server
	 * tries to mmap anything > FFFFF
	 * its very very unlikely that the server will try to mmap
	 * anything below FFFFF that can't be handled by /dev/vtXX.
	 * 
	 * DWH - 2/23/94
	 * DWH - 1/31/99 (Gee has it really been 5 years?) 
	 */

	if(Base < 0xFFFFF)
		sprintf(solx86_vtname,"/dev/vt%02d",xf86Info.vtno);
	else

	{
		if (!apertureDevName)
			if (!xf86LinearVidMem())
				FatalError("xf86MapVidMem: Could not mmap "
					   "linear framebuffer [s=%x,a=%x]\n",
					   Size, Base);
		
		sprintf(solx86_vtname, apertureDevName);
	}

	if ((fd = open(solx86_vtname, O_RDWR,0)) < 0)
	{
		FatalError("xf86MapVidMem: failed to open %s (%s)\n",
			   solx86_vtname, strerror(errno));
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

