/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsd_video.c,v 3.12 1996/10/03 08:38:21 dawes Exp $ */
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
/* $XConsortium: bsd_video.c /main/6 1995/12/28 17:16:32 kaleb $ */

#include "X.h"
#include "input.h"
#include "scrnintstr.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

static struct grfinfo grfInfo;
static int grfInfoChecked = FALSE;

unsigned int IOPortBase;

/*
 * Check if /dev/mem can be mmap'd.  If it can't print a warning when
 * "warn" is TRUE.
 */
static void getGrfInfo()
{
	if (!grfInfoChecked)
	{
	    if (ioctl(xf86Info.screenFd, GRFIOCGINFO, &grfInfo) == -1)
	    {
	        FatalError("unable to get frame buffer attributes\n");
	        return;
	    }
	    grfInfoChecked = TRUE;
#if 0
	    ErrorF("FrameBuffer Attributes:\n"
	           "   Phys addr: 0x%08x\n"
	           "   Size     : 0x%08x\n"
	           "   Reg addr : 0x%08x\n"
	           "   Reg size : 0x%08x\n"
	           "   colors   : 0x%08x\n"
	           "   planes   : 0x%08x\n"
	           "   with     : 0x%08x\n"
	           "   height   : 0x%08x\n"
	           "   bank size: 0x%08x\n", grfInfo.gd_fbaddr,
	           grfInfo.gd_fbsize, grfInfo.gd_regaddr, grfInfo.gd_regsize,
	           grfInfo.gd_colors, grfInfo.gd_planes, grfInfo.gd_fbwidth,
	           grfInfo.gd_fbheight, grfInfo.gd_bank_size);
#endif
	}
	return;
}

pointer xf86MapVidMem(ScreenNum, Region, Base, Size)
int ScreenNum;
int Region;
pointer Base;
unsigned long Size;
{
	pointer base;

	if (!grfInfoChecked)
		getGrfInfo();

	base = (pointer)mmap((caddr_t)0, Size, PROT_READ|PROT_WRITE,
			      MAP_FILE, xf86Info.screenFd,
			      (off_t)(unsigned long)Base);
	if (base == (pointer)-1)
	{
		FatalError("%s: could not mmap [s=%x,a=%x] (%s)\n",
			   "xf86MapVidMem", Size, Base, 
			   strerror(errno));
	}
	return(base);
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

pointer xf86GetLinearAddress()
{
	if (!grfInfoChecked)
		getGrfInfo();
	return(grfInfo.gd_linbase);
}
                             
void xf86UnMapVidMem(ScreenNum, Region, Base, Size)
int ScreenNum;
int Region;
pointer Base;
unsigned long Size;
{
	munmap((caddr_t)Base, Size);
}

Bool xf86LinearVidMem()
{
	return(TRUE);
}

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
	pointer base;

	ScreenEnabled[ScreenNum] = TRUE;

	if (ExtendedEnabled)
		return;

	ExtendedEnabled = TRUE;

	if (!grfInfoChecked)
		getGrfInfo();

	base = (pointer)mmap((caddr_t)0, grfInfo.gd_regsize, PROT_READ|PROT_WRITE,
			      MAP_FILE, xf86Info.screenFd, (off_t)0);
	if (base == (pointer)-1)
	{
		FatalError("%s: could not mmap [s=%x,a=%x] (%s)\n",
			   "xf86EnableIOPorts", grfInfo.gd_regsize, 0,
			   strerror(errno));
	}
	IOPortBase = (unsigned int)base;

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
	return;
}

/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/

Bool xf86DisableInterrupts()
{
	return(FALSE);
}

void xf86EnableInterrupts()
{
	return;
}
