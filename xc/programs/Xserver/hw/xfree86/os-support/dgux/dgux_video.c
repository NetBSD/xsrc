/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/dgux/dgux_video.c,v 1.1.2.4 1999/08/03 09:41:44 hohndel Exp $ */
/*
 * INTEL DG/UX RELEASE 4.20 MU04
 * Copyright 1997-1999 Takis Psarogiannakopoulos Cambridge,UK
 * <takis@dpmms.cam.ac.uk>
 *
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 * XFREE86 PROJECT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE.
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FITNESS, IN NO EVENT SHALL XCONSORTIUM BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "X.h"
#include "input.h"
#include "scrnintstr.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#define DG_NO_SYSI86 1

/***************************************************************************/
/* SET_IOPL() and RESET_IOPL() section for ix86 DG/ux 4.20MU04             */
/***************************************************************************/

#include <sys/sysi86.h>

#if defined(DG_NO_SYSI86)
#define SI86IOPL 112  /* Definition of SI86IOPL for DG/ux */


      asm("sysi86:_sysi86:pushl %ebp");
      asm("movl %esp,%ebp");
      asm("pushl 12(%ebp)");
      asm("pushl 8(%ebp)");
      asm("pushl 4(%ebp)");
      asm("movl $50,%eax");
      asm("lcall $7,$0");
      asm("addl $12,%esp");
      asm("leave");
      asm("ret");

#endif /* NO_SYSI86 */

#define SET_IOPL() sysi86(SI86IOPL ,3)
#define RESET_IOPL() sysi86(SI86IOPL,0)

/***************************************************************************/
/* DG/ux Video Memory Mapping part                                         */
/***************************************************************************/

#undef HAS_SVR3_MMAPDRV /* ix86 DG/ux is a typical SVR4 without SVR3_MMAPDRV */

Bool xf86LinearVidMem()
{
   return(TRUE);
}



pointer AllocAddress[MAXSCREENS][NUM_REGIONS];
#ifndef SVR4
static int mmapFd = -2;
#endif



#if 0
/* For DGA support */
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

#if defined(DGUX)
        if ((fd = open(DEV_MEM, O_RDWR)) < 0)
        {
                FatalError("xf86MapVidMem: failed to open %s (%s)\n",
                           DEV_MEM, strerror(errno));
        }
        base = (pointer)mmap((caddr_t)0, Size, PROT_READ|PROT_WRITE,
                             MAP_SHARED, fd, (off_t)Base);
        close(fd);
        if ((long)base == -1)
        {
                FatalError("%s: Could not mmap framebuffer [s=%x,a=%x] (%s)\n",
                           "xf86MapVidMem", Size, Base, strerror(errno));
        }
#else /* HAS SVR#_MMAPDRV */
#ifdef HAS_SVR3_MMAPDRV
        if (mmapFd == -2)
        {
                mmapFd = open("/dev/mmap", O_RDWR);
        }
#endif
        if (mmapFd >= 0)
        {
                /* To force the MMAP driver to provide the address */
                base = (pointer)0;
        }
        else
        {
            AllocAddress[ScreenNum][Region] = (pointer)xalloc(Size + 0x1000);
            if (AllocAddress[ScreenNum][Region] == (pointer)0)
            {
                FatalError("xf86MapVidMem: can't alloc framebuffer space\n");
                /* NOTREACHED */
            }
            base = (pointer)(((unsigned int)AllocAddress[ScreenNum][Region]
                              & ~0xFFF) + 0x1000);
        }


#ifdef HAS_SVR3_MMAPDRV
        if(mmapFd >= 0)
        {
            if((base = (pointer)ioctl(mmapFd, MAP,
                           &(MapDSC[ScreenNum][Region]))) == (pointer)-1)
            {
                FatalError("%s: Could not mmap framebuffer [s=%x,a=%x] (%s)\n",
                           "xf86MapVidMem", Size, Base, strerror(errno));
                /* NOTREACHED */
            }

#if 0
/* inserted for DGA support */
            xf86memMaps[ScreenNum].offset = (int) Base;
            xf86memMaps[ScreenNum].memSize = Size;
#endif
            return((pointer)base);
        }
#endif
#endif /* DGUX */
#if 0
        xf86memMaps[ScreenNum].offset = (int) Base;
        xf86memMaps[ScreenNum].memSize = Size;
#endif
        return((pointer)base);
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
	munmap(Base, Size);
}





/* NULL for DG/ux  */
void xf86MapDisplay(ScreenNum, Region)
int ScreenNum;
int Region;
{
	return;
}




/* NULL for DG/ux */
void xf86UnMapDisplay(ScreenNum, Region)
int ScreenNum;
int Region;
{
	return;
}

/***************************************************************************/
/* I/O Permissions section                                                 */
/***************************************************************************/

#define ALWAYS_USE_EXTENDED
#ifdef ALWAYS_USE_EXTENDED

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

	if (SET_IOPL() < 0)
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

	RESET_IOPL();
	ExtendedEnabled = FALSE;

	return;
}

#else /* !ALWAYS_USE_EXTENDED */

#define DISABLED	0
#define NON_EXTENDED	1
#define EXTENDED	2

static unsigned *EnabledPorts[MAXSCREENS];
static int NumEnabledPorts[MAXSCREENS];
static Bool ScreenEnabled[MAXSCREENS];
static Bool ExtendedPorts[MAXSCREENS];
static Bool ExtendedEnabled = FALSE;
static Bool InitDone = FALSE;
static struct kd_disparam OrigParams;

void xf86ClearIOPortList(ScreenNum)
int ScreenNum;
{
	if (!InitDone)
	{
		xf86InitPortLists(EnabledPorts, NumEnabledPorts, ScreenEnabled,
				  ExtendedPorts, MAXSCREENS);
		if (ioctl(xf86Info.consoleFd, KDDISPTYPE, &OrigParams) < 0)
		{
			FatalError("%s: Could not get display parameters\n",
				   "xf86ClearIOPortList");
		}
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
	struct kd_disparam param;
	int i, j;

	if (ScreenEnabled[ScreenNum])
		return;

	for (i = 0; i < MAXSCREENS; i++)
	{
		if (ExtendedPorts[i] && (ScreenEnabled[i] || i == ScreenNum))
		{
		    if (SET_IOPL() < 0)
		    {
			FatalError("%s: Failed to set IOPL for extended I/O\n",
				   "xf86EnableIOPorts");
		    }
		    ExtendedEnabled = TRUE;
		    break;
		}
	}
	if (ExtendedEnabled && i == MAXSCREENS)
	{
		RESET_IOPL();
		ExtendedEnabled = FALSE;
	}
	if (ioctl(xf86Info.consoleFd, KDDISPTYPE, &param) < 0)
	{
		FatalError("%s: Could not get display parameters\n",
			   "xf86EnableIOPorts");
	}
	for (i = 0; i < NumEnabledPorts[ScreenNum]; i++)
	{
		unsigned port = EnabledPorts[ScreenNum][i];

		if (port > 0x3FF)
			continue;

		if (!xf86CheckPorts(port, EnabledPorts, NumEnabledPorts,
				    ScreenEnabled, MAXSCREENS))
		{
			continue;
		}
		for (j=0; j < MKDIOADDR; j++)
		{
			if (param.ioaddr[j] == port)
			{
				break;
			}
		}
		if (j == MKDIOADDR)
		{
			if (ioctl(xf86Info.consoleFd, KDADDIO, port) < 0)
			{
				FatalError("%s: Failed to enable port 0x%x\n",
					   "xf86EnableIOPorts", port);
			}
		}
	}
	if (ioctl(xf86Info.consoleFd, KDENABIO, 0) < 0)
	{
		FatalError("xf86EnableIOPorts: I/O port enable failed (%s)\n",
			   strerror(errno));
	}
	ScreenEnabled[ScreenNum] = TRUE;
	return;
}

void xf86DisableIOPorts(ScreenNum)
int ScreenNum;
{
	struct kd_disparam param;
	int i, j;

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
		RESET_IOPL();
		ExtendedEnabled = FALSE;
	}
	/* Turn off I/O before changing the access list */
	ioctl(xf86Info.consoleFd, KDDISABIO, 0);
	if (ioctl(xf86Info.consoleFd, KDDISPTYPE, &param) < 0)
	{
		ErrorF("%s: Could not get display parameters\n",
		       "xf86DisableIOPorts");
		return;
	}

	for (i=0; i < MKDIOADDR; i++)
	{
		if (param.ioaddr[i] == 0)
		{
			break;
		}
		if (!xf86CheckPorts(param.ioaddr[i], EnabledPorts,
				    NumEnabledPorts, ScreenEnabled, MAXSCREENS))
		{
			continue;
		}
		for (j=0; j < MKDIOADDR; j++)
		{
			if (param.ioaddr[i] == OrigParams.ioaddr[j])
			{
				/*
				 * Port was one of the original ones; don't
				 * touch it.
				 */
				break;
			}
		}
		if (j == MKDIOADDR)
		{
			/*
			 * We added this port, so remove it.
			 */
			ioctl(xf86Info.consoleFd, KDDELIO, param.ioaddr[i]);
		}
	}
	for (i = 0; i < MAXSCREENS; i++)
	{
		if (ScreenEnabled[i])
		{
			ioctl(xf86Info.consoleFd, KDENABIO, 0);
			break;
		}
	}
	return;
}
#endif

void xf86DisableIOPrivs()
{
	if (ExtendedEnabled)
		RESET_IOPL();
	return;
}

/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/


Bool xf86DisableInterrupts()
{

#ifdef __GNUC__
        __asm__ __volatile__("cli");
#else
        asm("cli");
#endif /* __GNUC__ */

        return(TRUE);
}

void xf86EnableInterrupts()
{

#ifdef __GNUC__
        __asm__ __volatile__("sti");
#else
        asm("sti");
#endif /* __GNUC__ */

        return;
}


