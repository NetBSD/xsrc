/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsd_video.c,v 3.40 2000/12/13 07:02:45 herrb Exp $ */
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

#ifdef HAS_MTRR_SUPPORT
#ifndef __NetBSD__
#include <sys/memrange.h>
#else
#include "memrange.h"
#endif
#define X_MTRR_ID "XFree86"
#endif

#ifdef __alpha__
#include <sys/sysctl.h>
#endif

#ifdef __arm32__
#include "machine/devmap.h"
struct memAccess
{
    int ioctl;
    struct map_info memInfo;
    pointer regionVirtBase;
    Bool Checked;
    Bool OK;
};

static pointer xf86MapInfoMap();
static void xf86MapInfoUnmap();
static struct memAccess *checkMapInfo();
extern int vgaPhysLinearBase;

/* A memAccess structure is needed for each possible region */ 
struct memAccess vgaMemInfo = { CONSOLE_GET_MEM_INFO, NULL, NULL, 
				    FALSE, FALSE };
struct memAccess linearMemInfo = { CONSOLE_GET_LINEAR_INFO, NULL, NULL, 
				       FALSE, FALSE };
struct memAccess ioMemInfo = { CONSOLE_GET_IO_INFO, NULL, NULL,
				   FALSE, FALSE };
#endif /* __arm32__ */

#if defined(__NetBSD__) && !defined(MAP_FILE)
#define MAP_FLAGS MAP_SHARED
#else
#define MAP_FLAGS (MAP_FILE | MAP_SHARED)
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif

#ifdef __OpenBSD__
#define SYSCTL_MSG "\tCheck that you have set 'machdep.allowaperture=1'\n"\
		   "\tin /etc/sysctl.conf and reboot your machine\n" \
		   "\trefer to xf86(4) for details\n"
#define SECURELEVEL_MSG \
                "A card in your system needs more than the aperture driver\n" \
                "\tYou need to rebuild a kernel with \"Option INSECURE\"\n" \
                "\tand set securelevel=-1 in /etc/rc.securelevel\n"
#endif

#ifdef __alpha__

extern unsigned long dense_base(void);

static unsigned long
memory_base(void)
{
    static unsigned long base = 0;

    if (base == 0) {
	size_t len = sizeof(base);
	int error;
	if ((error = sysctlbyname("hw.chipset.memory", &base, &len,
				  0, 0)) < 0)
	    FatalError("xf86MapVidMem: can't find memory\n");
    }

    return base;
}

static int
has_bwx(void)
{
    static int bwx = 0;
    size_t len = sizeof(bwx);
    int error;
    if ((error = sysctlbyname("hw.chipset.bwx", &bwx, &len, 0, 0)) < 0)
	return FALSE;
    else
	return bwx;
}

#define BUS_BASE	dense_base()
#define BUS_BASE_BWX	memory_base()

#else

#define BUS_BASE	0L
#define BUS_BASE_BWX	0L

#endif

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

static Bool useDevMem = FALSE;
static int  devMemFd = -1;

#ifdef HAS_APERTURE_DRV
#define DEV_APERTURE "/dev/xf86"
#endif
#define DEV_MEM "/dev/mem"

static pointer mapVidMem(int, unsigned long, unsigned long, int);
static void unmapVidMem(int, pointer, unsigned long);
#ifdef __alpha__
static pointer mapVidMemSparse(int, unsigned long, unsigned long, int);
static void unmapVidMemSparse(int, pointer, unsigned long);
#endif
#ifdef __powerpc__
static pointer ppcMapVidMem(int, unsigned long, unsigned long);
static void ppcUnmapVidMem(int, pointer, unsigned long);
#endif
#ifdef HAS_MTRR_SUPPORT
static pointer setWC(int, unsigned long, unsigned long, Bool, MessageType);
static void undoWC(int, pointer);
static Bool cleanMTRR(void);
#endif

#if !defined(__powerpc__)
/*
 * Check if /dev/mem can be mmap'd.  If it can't print a warning when
 * "warn" is TRUE.
 */
static void
checkDevMem(Bool warn)
{
	static Bool devMemChecked = FALSE;
	int fd;
	pointer base;

	if (devMemChecked)
	    return;
	devMemChecked = TRUE;

	if ((fd = open(DEV_MEM, O_RDWR)) >= 0)
	{
	    /* Try to map a page at the VGA address */
	    base = mmap((caddr_t)0, 4096, PROT_READ|PROT_WRITE,
				 MAP_FLAGS, fd, (off_t)0xA0000 + BUS_BASE);
	
	    if (base != MAP_FAILED)
	    {
		munmap((caddr_t)base, 4096);
		devMemFd = fd;
		useDevMem = TRUE;
		return;
	    } else {
		/* This should not happen */
		if (warn)
		{
		    xf86Msg(X_WARNING, "checkDevMem: failed to mmap %s (%s)\n",
			    DEV_MEM, strerror(errno));
		}
		useDevMem = FALSE;
		return;
	    }
	}
#ifndef HAS_APERTURE_DRV
	if (warn)
	{ 
	    xf86Msg(X_WARNING, "checkDevMem: failed to open %s (%s)\n",
		    DEV_MEM, strerror(errno));
	    xf86ErrorF("\tlinear framebuffer access unavailable\n");
	} 
	useDevMem = FALSE;
	return;
#else
	/* Failed to open /dev/mem, try the aperture driver */
	if ((fd = open(DEV_APERTURE, O_RDWR)) >= 0)
	{
	    /* Try to map a page at the VGA address */
	    base = mmap((caddr_t)0, 4096, PROT_READ|PROT_WRITE,
			     MAP_FLAGS, fd, (off_t)0xA0000);
	
	    if (base != MAP_FAILED)
	    {
		munmap((caddr_t)base, 4096);
		devMemFd = fd;
		useDevMem = TRUE;
		xf86Msg(X_INFO, "checkDevMem: using aperture driver %s\n",
		        DEV_APERTURE);
		return;
	    } else {

		if (warn)
		{
		    xf86Msg(X_WARNING, "checkDevMem: failed to mmap %s (%s)\n",
			    DEV_APERTURE, strerror(errno));
		}
	    }
	} else {
	    if (warn)
	    {
#ifndef __OpenBSD__
		xf86Msg(X_WARNING, "checkDevMem: failed to open %s and %s\n"
			"\t(%s)\n", DEV_MEM, DEV_APERTURE, strerror(errno));
#else /* __OpenBSD__ */
		xf86Msg(X_WARNING, "checkDevMem: failed to open %s and %s\n"
			"\t(%s)\n%s", DEV_MEM, DEV_APERTURE, strerror(errno),
			SYSCTL_MSG);
#endif /* __OpenBSD__ */
	    }
	}
	
	if (warn)
	{
	    xf86ErrorF("\tlinear framebuffer access unavailable\n");
	}
	useDevMem = FALSE;
	return;

#endif
}
#endif /* !__powerpc__ */

void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
#if defined(__powerpc__)
	pVidMem->linearSupported = TRUE;
#else
	checkDevMem(TRUE);
	pVidMem->linearSupported = useDevMem;
#endif
#if defined(__alpha__)
	if (has_bwx()) {
	    xf86Msg(X_INFO,"Machine type has 8/16 bit access\n");
	    pVidMem->mapMem = mapVidMem;
	    pVidMem->unmapMem = unmapVidMem;
	} else {
	    xf86Msg(X_INFO,"Machine needs sparse mapping\n");
	    pVidMem->mapMem = mapVidMemSparse;
	    pVidMem->unmapMem = unmapVidMemSparse;
	}
#elif defined(__arm32__)
	pVidMem->mapMem = armMapVidMem;
	pVidMem->unmapVidMem = armUnmapVidMem;
#elif defined(__powerpc__)
	pVidMem->mapMem = ppcMapVidMem;
	pVidMem->unmapMem = ppcUnmapVidMem;
#else
	pVidMem->mapMem = mapVidMem;
	pVidMem->unmapMem = unmapVidMem;
#endif

#ifdef HAS_MTRR_SUPPORT
	if (useDevMem) {
		if (cleanMTRR()) {
			pVidMem->setWC = setWC;
			pVidMem->undoWC = undoWC;
		}
	}
#endif
	pVidMem->initialised = TRUE;
}

#if !defined(__powerpc__)
static pointer
mapVidMem(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
	pointer base;

	checkDevMem(FALSE);

#ifdef __alpha__
	Base = Base & ((1L<<32) - 1);
#endif

	if (useDevMem)
	{
	    if (devMemFd < 0) 
	    {
		FatalError("xf86MapVidMem: failed to open %s (%s)\n",
			   DEV_MEM, strerror(errno));
	    }
	    base = mmap((caddr_t)0, Size, PROT_READ|PROT_WRITE,
				 MAP_FLAGS, devMemFd, (off_t)Base + BUS_BASE_BWX);
	    if (base == MAP_FAILED)
	    {
		FatalError("%s: could not mmap %s [s=%x,a=%x] (%s)\n",
			   "xf86MapVidMem", DEV_MEM, Size, Base, 
			   strerror(errno));
	    }
	    return(base);
	}
		
	/* else, mmap /dev/vga */
	if ((unsigned long)Base < 0xA0000 || (unsigned long)Base >= 0xC0000)
	{
		FatalError("%s: Address 0x%x outside allowable range\n",
			   "xf86MapVidMem", Base);
	}
	base = mmap(0, Size, PROT_READ|PROT_WRITE, MAP_FLAGS,
			     xf86Info.screenFd,
#if defined(__alpha__)
			     (unsigned long)Base + BUS_BASE
#elif defined(__mips__)
			     (unsigned long)Base
#else
			     (unsigned long)Base - 0xA0000
#endif
	    );
	if (base == MAP_FAILED)
	{
	    FatalError("xf86MapVidMem: Could not mmap /dev/vga (%s)\n",
		       strerror(errno));
	}
	return(base);
}

static void
unmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
	munmap((caddr_t)Base, Size);
}

/*
 * Read BIOS via mmap()ing DEV_MEM
 */

int
xf86ReadBIOS(unsigned long Base, unsigned long Offset, unsigned char *Buf,
	     int Len)
{
	unsigned char *ptr;
	int psize;
	int mlen;

	checkDevMem(TRUE);
	if (devMemFd == -1) {
	    return(-1);
	}

	psize = xf86getpagesize();
	Offset += Base & (psize - 1);
	Base &= ~(psize - 1);
	mlen = (Offset + Len + psize - 1) & ~(psize - 1);
	ptr = (unsigned char *)mmap((caddr_t)0, mlen, PROT_READ,
					MAP_SHARED, devMemFd, (off_t)Base+BUS_BASE);
	if ((long)ptr == -1)
	{
		xf86Msg(X_WARNING, 
			"xf86ReadBIOS: %s mmap[s=%x,a=%x,o=%x] failed (%s)\n",
			DEV_MEM, Len, Base, Offset, strerror(errno));
#ifdef __OpenBSD__
		if (Base < 0xa0000) {
		    xf86Msg(X_WARNING, SECURELEVEL_MSG);
		} 
#endif
		return(-1);
	}
#ifdef DEBUG
	ErrorF("xf86ReadBIOS: BIOS at 0x%08x has signature 0x%04x\n",
		Base, ptr[0] | (ptr[1] << 8));
#endif
	(void)memcpy(Buf, (void *)(ptr + Offset), Len);
	(void)munmap((caddr_t)ptr, mlen);
	xf86MsgVerb(X_INFO, 3, "xf86ReadBIOS(%x, %x, Buf, %x)"
		"-> %02x %02x %02x %02x...\n",
		Base, Offset, Len, Buf[0], Buf[1], Buf[2], Buf[3]);
	return(Len);
}

#endif /* !__powerpc__ */

#ifdef __arm32__

/* XXX This needs to be updated for the ND */

/*
** Find out whether the console driver provides memory mapping information 
** for the specified region and return the map_info pointer. Print a warning if required.
*/
static struct memAccess *
checkMapInfo(Bool warn, int Region)
{
    struct memAccess *memAccP;
        
    switch (Region)
    {
	case VGA_REGION:
	    memAccP = &vgaMemInfo;
	    break;
	    	    
	case LINEAR_REGION:
	    memAccP = &linearMemInfo;
	    break;
	    
	case MMIO_REGION:
	    memAccP = &ioMemInfo;
	    break;
	
	default:
	    return NULL;
	    break;
    }
    
    if(!memAccP->Checked)
    {	
	if(ioctl(xf86Info.screenFd, memAccP->ioctl, &(memAccP->memInfo)) == -1)
	{
	    if(warn)
	    {
		xf86Msg(X_WARNING,
		 "checkMapInfo: failed to get map info for region %d\n\t(%s)\n",
		       Region, strerror(errno));
	    }
	}
	else
	{
	    if(memAccP->memInfo.u.map_info_mmap.map_offset 
	       != MAP_INFO_UNKNOWN)
		memAccP->OK = TRUE;
	}
	memAccP->Checked = TRUE;
    }
    if (memAccP->OK)
    {
	return memAccP;
    }
    else
    {
	return NULL;
    }
}

static pointer
xf86MapInfoMap(struct memAccess *memInfoP, pointer Base, unsigned long Size)
{
    struct map_info *mapInfoP = &(memInfoP->memInfo);

    if (mapInfoP->u.map_info_mmap.map_size == MAP_INFO_UNKNOWN)
    {	
	Size = (unsigned long)Base + Size;
    }
    else
    {
	Size = mapInfoP->u.map_info_mmap.map_size;
    }
    
    switch(mapInfoP->method)
    {
	case MAP_MMAP:
	    /* Need to remap if size is unknown because we may not have
	       mapped the whole region initially */
	    if(memInfoP->regionVirtBase == NULL ||
	       mapInfoP->u.map_info_mmap.map_size == MAP_INFO_UNKNOWN)
	    {
		if((memInfoP->regionVirtBase = 
		    mmap((caddr_t)0,
			 Size,
			 PROT_READ|PROT_WRITE,
			 MAP_SHARED,
			 xf86Info.screenFd,
			 (unsigned long)mapInfoP->u.map_info_mmap.map_offset))
		   == (pointer)-1)
		{
		    FatalError("xf86MapInfoMap: Failed to map memory at 0x%x\n\t%s\n", 
			       mapInfoP->u.map_info_mmap.map_offset, strerror(errno));
		}
		if(mapInfoP->u.map_info_mmap.internal_offset > 0)
		    memInfoP->regionVirtBase += 
			mapInfoP->u.map_info_mmap.internal_offset;
	    }
	    break;
	    
	default:
	    FatalError("xf86MapInfoMap: Unsuported mapping method\n");
	    break;
    }
	    
    return (pointer)((int)memInfoP->regionVirtBase + (int)Base);
}

static void
xf86MapInfoUnmap(struct memAccess *memInfoP, unsigned long Size)
{
    struct map_info *mapInfoP = &(memInfoP->memInfo);
    
    switch(mapInfoP->method)
    {
	case MAP_MMAP:
	    if(memInfoP->regionVirtBase != NULL)
	    {
		if(mapInfoP->u.map_info_mmap.map_size != MAP_INFO_UNKNOWN)
		    Size = mapInfoP->u.map_info_mmap.map_size;
		munmap((caddr_t)memInfoP->regionVirtBase, Size);
		memInfoP->regionVirtBase = NULL;
	    }
	    break;
	 default:
	    FatalError("xf86MapInfoMap: Unsuported mapping method\n");
	    break;
    }
}

static pointer
armMapVidMem(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
	struct memAccess *memInfoP;
	
	if((memInfoP = checkMapInfo(FALSE, Region)) != NULL)
	{
	    /*
	     ** xf86 passes in a physical address offset from the start
	     ** of physical memory, but xf86MapInfoMap expects an 
	     ** offset from the start of the specified region - it gets 
	     ** the physical address of the region from the display driver.
	     */
	    switch(Region)
	    {
	        case LINEAR_REGION:
		    if (vgaPhysLinearBase)
		    {
			Base -= vgaPhysLinearBase;
		    }
		    break;
		case VGA_REGION:
		    Base -= 0xA0000;
		    break;
	    }
	    
	    base = xf86MapInfoMap(memInfoP, Base, Size);
	    return (base);
	}
	return mapVidMem(ScreenNum, Base, Size, flags);
}

static void
armUnmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
        struct memAccess *memInfoP;
	
	if((memInfoP = checkMapInfo(FALSE, Region)) != NULL)
	{
	    xf86MapInfoUnmap(memInfoP, Base, Size);
	}
	unmapVidMem(ScreenNum, Base, Size);
}
#endif /* __arm32__ */

#if defined(__powerpc__)

volatile unsigned char *ioBase = MAP_FAILED;

static pointer
ppcMapVidMem(int ScreenNum, unsigned long Base, unsigned long Size)
{
	int fd = xf86Info.screenFd;
	pointer base;

	fprintf(stderr, "mapVidMem %lx, %lx, fd = %d\n", Base, Size, fd);

	base = mmap(0, Size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, Base);
	if (base == MAP_FAILED)
		FatalError("%s: could not mmap screen [s=%x,a=%x] (%s)\n",
			   "xf86MapVidMem", Size, Base, strerror(errno));

	return base;
}

static void
ppcUnmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
	munmap(Base, Size);
}

int
xf86ReadBIOS(unsigned long Base, unsigned long Offset, unsigned char *Buf,
	     int Len)
{
	int rv;
	int kmem;

 	kmem = open("/dev/kmem", 2);
 	if (kmem == -1) {
 		FatalError("xf86ReadBIOS: open /dev/kmem\n");
 	}


	fprintf(stderr, "xf86ReadBIOS() %lx %lx, %x\n", Base, Offset, Len);

	if (Base < 0x80000000) {
		fprintf(stderr, "No VGA\n");
		return 0;
	}


	lseek(kmem, Base + Offset, 0);
	rv = read(kmem, Buf, Len);
	close(kmem);

	return rv;
}


#endif /* __powerpc__ */

#ifdef USE_I386_IOPL
/***************************************************************************/
/* I/O Permissions section                                                 */
/***************************************************************************/

static Bool ExtendedEnabled = FALSE;

void
xf86EnableIO()
{
	if (ExtendedEnabled)
		return;

	if (i386_iopl(TRUE) < 0)
	{
#ifndef __OpenBSD__
		FatalError("%s: Failed to set IOPL for extended I/O\n",
			   "xf86EnableIO");
#else
		FatalError("%s: Failed to set IOPL for extended I/O\n%s",
			   "xf86EnableIO", SYSCTL_MSG);
#endif
	}
	ExtendedEnabled = TRUE;

	return;
}
	
void
xf86DisableIO()
{
	if (!ExtendedEnabled)
		return;

	i386_iopl(FALSE);
	ExtendedEnabled = FALSE;

	return;
}

#endif /* USE_I386_IOPL */

#ifdef USE_DEV_IO
static int IoFd = -1;

void
xf86EnableIO()
{
	if (IoFd >= 0)
		return;

	if ((IoFd = open("/dev/io", O_RDWR)) == -1)
	{
		FatalError("xf86EnableIO: "
				"Failed to open /dev/io for extended I/O\n");
	}
	return;
}

void
xf86DisableIO()
{
	if (IoFd < 0)
		return;

	close(IoFd);
	IoFd = -1;
	return;
}

#endif

#if defined(USE_ARC_MMAP) || defined(__arm32__)

void
xf86EnableIO()
{
	int fd;
	pointer base;

	if (ExtendedEnabled)
		return;

	if ((fd = open("/dev/ttyC0", O_RDWR)) >= 0) {
		/* Try to map a page at the pccons I/O space */
		base = (pointer)mmap((caddr_t)0, 65536, PROT_READ|PROT_WRITE,
				MAP_FLAGS, fd, (off_t)0x0000);

		if (base != (pointer)-1) {
			IOPortBase = base;
		}
		else {
			FatalError("EnableIO: failed to mmap %s (%s)\n",
				"/dev/ttyC0", strerror(errno));
		}
	}
	else {
		FatalError("EnableIO: failed to open %s (%s)\n",
			"/dev/ttyC0", strerror(errno));
	}
	
	ExtendedEnabled = TRUE;

	return;
}

void
xf86DisableIO()
{
	return;
}

#endif /* USE_ARC_MMAP */

#if defined(__FreeBSD__) && defined(__alpha__)

extern int ioperm(unsigned long from, unsigned long num, int on);

void
xf86EnableIO()
{
	ioperm(0, 65536, TRUE);
	return;
}

void
xf86DisableIO()
{
	return;
}

#endif /* __FreeBSD__ && __alpha__ */

/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/

Bool
xf86DisableInterrupts()
{

#if !defined(__mips__) && !defined(__arm32__) && !defined(__alpha__) && \
    !defined(__powerpc__)
#ifdef __GNUC__
	__asm__ __volatile__("cli");
#else 
	asm("cli");
#endif /* __GNUC__ */
#endif /* __mips__ */

	return(TRUE);
}

void
xf86EnableInterrupts()
{

#if !defined(__mips__) && !defined(__arm32__) && !defined(__alpha__) && \
    !defined(__powerpc__)
#ifdef __GNUC__
	__asm__ __volatile__("sti");
#else 
	asm("sti");
#endif /* __GNUC__ */
#endif /* __mips__ */

	return;
}


#ifdef __NetBSD__
/***************************************************************************/
/* Set TV output mode                                                      */
/***************************************************************************/
void
xf86SetTVOut(int mode)
{    
    switch (xf86Info.consType)
    {
#ifdef PCCONS_SUPPORT
	case PCCONS:{

	    if (ioctl (xf86Info.consoleFd, CONSOLE_X_TV_ON, &mode) < 0)
	    {
		xf86Msg(X_WARNING,
		    "xf86SetTVOut: Could not set console to TV output, %s\n",
		    strerror(errno));
	    }
	}
	break;
#endif /* PCCONS_SUPPORT */

	default:
	    FatalError("Xf86SetTVOut: Unsupported console\n");
	    break; 
    }
    return;
}

void
xf86SetRGBOut()
{    
    switch (xf86Info.consType)
    {
#ifdef PCCONS_SUPPORT
	case PCCONS:{
	    
	    if (ioctl (xf86Info.consoleFd, CONSOLE_X_TV_OFF, 0) < 0)
	    {
		xf86Msg(X_WARNING,
		    "xf86SetTVOut: Could not set console to RGB output, %s\n",
		    strerror(errno));
	    }
	}
	break;
#endif /* PCCONS_SUPPORT */

	default:
	    FatalError("Xf86SetTVOut: Unsupported console\n");
	    break; 
    }
    return;
}
#endif


#if 0
/*
 * XXX This is here for reference.  It needs to be handled differently for the
 * ND.
 */
#if defined(USE_ARC_MMAP) || defined(__arm32__)

#ifdef USE_ARM32_MMAP
#define	DEV_MEM_IOBASE	0x43000000
#endif

static Bool ScreenEnabled[MAXSCREENS];
static Bool ExtendedEnabled = FALSE;
static Bool InitDone = FALSE;

void
xf86EnableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;
	int fd;
	pointer base;

#ifdef __arm32__
	struct memAccess *memInfoP;
	int *Size;
#endif

	ScreenEnabled[ScreenNum] = TRUE;

	if (ExtendedEnabled)
		return;

#ifdef USE_ARC_MMAP
	if ((fd = open("/dev/ttyC0", O_RDWR)) >= 0) {
		/* Try to map a page at the pccons I/O space */
		base = (pointer)mmap((caddr_t)0, 65536, PROT_READ|PROT_WRITE,
				MAP_FLAGS, fd, (off_t)0x0000);

		if (base != (pointer)-1) {
			IOPortBase = base;
		}
		else {
			xf86Msg(X_ERROR,
				"EnableIOPorts: failed to mmap %s (%s)\n",
				"/dev/ttyC0", strerror(errno));
		}
	}
	else {
		xf86Msg(X_ERROR, "EnableIOPorts: failed to open %s (%s)\n",
			"/dev/ttyC0", strerror(errno));
	}
#endif

#ifdef __arm32__
	IOPortBase = (unsigned int)-1;

	if((memInfoP = checkMapInfo(TRUE, MMIO_REGION)) != NULL)
	{
	    /* 
	     * xf86MapInfoMap maps an offset from the start of video IO
	     * space (e.g. 0x3B0), but IOPortBase is expected to map to
	     * physical address 0x000, so subtract the start of video I/O
	     * space from the result.  This is safe for now becase we
	     * actually mmap the start of the page, then the start of video
	     * I/O space is added as an internal offset.
	     */
	    IOPortBase = (unsigned int)xf86MapInfoMap(memInfoP,
						      (caddr_t)0x0, 0L) 
		- memInfoP->memInfo.u.map_info_mmap.internal_offset;
	    ExtendedEnabled = TRUE;
	    return;
	}
#ifdef USE_ARM32_MMAP
	checkDevMem(TRUE);

	if (devMemFd >= 0 && useDevMem)
	{
	    base = (pointer)mmap((caddr_t)0, 0x400, PROT_READ|PROT_WRITE,
				 MAP_FLAGS, devMemFd, (off_t)DEV_MEM_IOBASE);

	    if (base != (pointer)-1)
		IOPortBase = (unsigned int)base;
	}

        if (IOPortBase == (unsigned int)-1)
	{	
	    FatalError("xf86EnableIOPorts: failed to open mem device or map IO base. \n\
Make sure you have the Aperture Driver installed, or a kernel built with the INSECURE option\n");
	}
#else
	/* We don't have the IOBASE, so we can't map the address */
	FatalError("xf86EnableIOPorts: failed to open mem device or map IO base. \n\
Try building the server with USE_ARM32_MMAP defined\n");
#endif
#endif
	
	ExtendedEnabled = TRUE;

	return;
}

void
xf86DisableIOPorts(ScreenNum)
int ScreenNum;
{
	int i;
#ifdef __arm32__
        struct memAccess *memInfoP;
#endif

	ScreenEnabled[ScreenNum] = FALSE;

#ifdef __arm32__
	if((memInfoP = checkMapInfo(FALSE, MMIO_REGION)) != NULL)
	{
	    xf86MapInfoUnmap(memInfoP, 0);
	}
#endif

#ifdef USE_ARM32_MMAP
	if (!ExtendedEnabled)
	return;

	for (i = 0; i < MAXSCREENS; i++)
		if (ScreenEnabled[i])
			return;

	munmap((caddr_t)IOPortBase, 0x400);
	IOPortBase = (unsigned int)-1;
	ExtendedEnabled = FALSE;
#endif

	return;
}

#endif /* USE_ARC_MMAP || USE_ARM32_MMAP */
#endif


#ifdef HAS_MTRR_SUPPORT
/* memory range (MTRR) support for FreeBSD */

/*
 * This code is experimental.  Some parts may be overkill, and other parts
 * may be incomplete.
 */

/*
 * getAllRanges returns the full list of memory ranges with attributes set.
 */

static struct mem_range_desc *
getAllRanges(int *nmr)
{
	struct mem_range_desc *mrd;
	struct mem_range_op mro;

	/*
	 * Find how many ranges there are.  If this fails, then the kernel
	 * probably doesn't have MTRR support.
	 */
	mro.mo_arg[0] = 0;
	if (ioctl(devMemFd, MEMRANGE_GET, &mro))
		return NULL;
	*nmr = mro.mo_arg[0];
	mrd = xnfalloc(*nmr * sizeof(struct mem_range_desc));
	mro.mo_arg[0] = *nmr;
	mro.mo_desc = mrd;
	if (ioctl(devMemFd, MEMRANGE_GET, &mro)) {
		xfree(mrd);
		return NULL;
	}
	return mrd;
}

/*
 * cleanMTRR removes any memory attribute that may be left by a previous
 * X server.  Normally there won't be any, but this takes care of the
 * case where a server crashed without being able finish cleaning up.
 */

static Bool
cleanMTRR()
{
	struct mem_range_desc *mrd;
	struct mem_range_op mro;
	int nmr, i;

	/* This shouldn't happen */
	if (devMemFd < 0)
		return FALSE;

	if (!(mrd = getAllRanges(&nmr)))
		return FALSE;

	for (i = 0; i < nmr; i++) {
		if (strcmp(mrd[i].mr_owner, X_MTRR_ID) == 0 &&
		    (mrd[i].mr_flags & MDF_ACTIVE)) {
#ifdef DEBUG
			ErrorF("Clean for (0x%lx,0x%lx)\n",
				(unsigned long)mrd[i].mr_base,
				(unsigned long)rd[i].mr_len);
#endif
			if (mrd[i].mr_flags & MDF_FIXACTIVE) {
				mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
				mrd[i].mr_flags = MDF_UNCACHEABLE;
			} else {
				mro.mo_arg[0] = MEMRANGE_SET_REMOVE;
			}
			mro.mo_desc = mrd + i;
			ioctl(devMemFd, MEMRANGE_SET, &mro);
		}
	}
#ifdef DEBUG
	sleep(10);
#endif
	xfree(mrd);
	return TRUE;
}

typedef struct x_RangeRec {
	struct mem_range_desc	mrd;
	Bool			wasWC;
	struct x_RangeRec *	next;
} RangeRec, *RangePtr;

static void
freeRangeList(RangePtr range)
{
	RangePtr rp;

	while (range) {
		rp = range;
		range = rp->next;
		xfree(rp);
	}
}

static RangePtr
dupRangeList(RangePtr list)
{
	RangePtr new = NULL, rp, p;

	rp = list;
	while (rp) {
		p = xnfalloc(sizeof(RangeRec));
		*p = *rp;
		p->next = new;
		new = p;
		rp = rp->next;
	}
	return new;
}

static RangePtr
sortRangeList(RangePtr list)
{
	RangePtr rp1, rp2, copy, sorted = NULL, minp, prev, minprev;
	unsigned long minBase;

	/* Sort by base address */
	rp1 = copy = dupRangeList(list);
	while (rp1) {
		minBase = rp1->mrd.mr_base;
		minp = rp1;
		minprev = NULL;
		prev = rp1;
		rp2 = rp1->next;
		while (rp2) {
			if (rp2->mrd.mr_base < minBase) {
				minBase = rp2->mrd.mr_base;
				minp = rp2;
				minprev = prev;
			}
			prev = rp2;
			rp2 = rp2->next;
		}
		if (minprev) {
			minprev->next = minp->next;
			rp1 = copy;
		} else {
			rp1 = minp->next;
		}
		minp->next = sorted;
		sorted = minp;
	}
	return sorted;
}

/*
 * findRanges returns a list of ranges that overlap the specified range.
 */

static void
findRanges(unsigned long base, unsigned long size, RangePtr *ucp, RangePtr *wcp)
{
	struct mem_range_desc *mrd;
	int nmr, i;
	RangePtr rp, *p;
	
	if (!(mrd = getAllRanges(&nmr)))
		return;

	for (i = 0; i < nmr; i++) {
		if ((mrd[i].mr_flags & MDF_ACTIVE) &&
		    mrd[i].mr_base < base + size &&
		    mrd[i].mr_base + mrd[i].mr_len > base) {
			if (mrd[i].mr_flags & MDF_WRITECOMBINE)
				p = wcp;
			else if (mrd[i].mr_flags & MDF_UNCACHEABLE)
				p = ucp;
			else
				continue;
			rp = xnfalloc(sizeof(RangeRec));
			rp->mrd = mrd[i];
			rp->next = *p;
			*p = rp;
		}
	}
	xfree(mrd);
}

/*
 * This checks if the existing overlapping ranges fully cover the requested
 * range.  Is this overkill?
 */

static Bool
fullCoverage(unsigned long base, unsigned long size, RangePtr overlap)
{
	RangePtr rp1, sorted = NULL;
	unsigned long end;

	sorted = sortRangeList(overlap);
	/* Look for gaps */
	rp1 = sorted;
	end = base + size;
	while (rp1) {
		if (rp1->mrd.mr_base > base) {
			freeRangeList(sorted);
			return FALSE;
		} else {
			base = rp1->mrd.mr_base + rp1->mrd.mr_len;
		}
		if (base >= end) {
			freeRangeList(sorted);
			return TRUE;
		}
		rp1 = rp1->next;
	}
	freeRangeList(sorted);
	return FALSE;
}

static pointer
addWC(int screenNum, unsigned long base, unsigned long size, MessageType from)
{
	RangePtr uc = NULL, wc = NULL, retlist = NULL;
	struct mem_range_desc mrd;
	struct mem_range_op mro;

	findRanges(base, size, &uc, &wc);

	/* See of the full range is already WC */
	if (!uc && fullCoverage(base, size, wc)) {
		xf86DrvMsg(screenNum, from, 
		   "Write-combining range (0x%lx,0x%lx) was already set\n",
		    base, size);
		return NULL;
	}

	/* Otherwise, try to add the new range */
	mrd.mr_base = base;
	mrd.mr_len = size;
	strcpy(mrd.mr_owner, X_MTRR_ID);
	mrd.mr_flags = MDF_WRITECOMBINE;
	mro.mo_desc = &mrd;
	mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
	if (ioctl(devMemFd, MEMRANGE_SET, &mro)) {
		xf86DrvMsg(screenNum, X_WARNING,
			   "Failed to set write-combining range "
			   "(0x%lx,0x%lx)\n", base, size);
		return NULL;
	} else {
		xf86DrvMsg(screenNum, from,
			   "Write-combining range (0x%lx,0x%lx)\n", base, size);
		retlist = xnfalloc(sizeof(RangeRec));
		retlist->mrd = mrd;
		retlist->wasWC = FALSE;
		retlist->next = NULL;
		return retlist;
	}
}

static pointer
delWC(int screenNum, unsigned long base, unsigned long size, MessageType from)
{
	RangePtr uc = NULL, wc = NULL, retlist = NULL;
	struct mem_range_desc mrd;
	struct mem_range_op mro;

	findRanges(base, size, &uc, &wc);

	/*
	 * See of the full range is already not WC, or if there is full
	 * coverage from UC ranges.
	 */
	if (!wc || fullCoverage(base, size, uc)) {
		xf86DrvMsg(screenNum, from, 
		   "Write-combining range (0x%lx,0x%lx) was already clear\n",
		    base, size);
		return NULL;
	}

	/* Otherwise, try to add the new range */
	mrd.mr_base = base;
	mrd.mr_len = size;
	strcpy(mrd.mr_owner, X_MTRR_ID);
	mrd.mr_flags = MDF_UNCACHEABLE;
	mro.mo_desc = &mrd;
	mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
	if (ioctl(devMemFd, MEMRANGE_SET, &mro)) {
		xf86DrvMsg(screenNum, X_WARNING,
			   "Failed to remove write-combining range "
			   "(0x%lx,0x%lx)\n", base, size);
		/* XXX Should then remove all of the overlapping WC ranges */
		return NULL;
	} else {
		xf86DrvMsg(screenNum, from,
			   "Removed Write-combining range (0x%lx,0x%lx)\n",
			   base, size);
		retlist = xnfalloc(sizeof(RangeRec));
		retlist->mrd = mrd;
		retlist->wasWC = TRUE;
		retlist->next = NULL;
		return retlist;
	}
}

static pointer
setWC(int screenNum, unsigned long base, unsigned long size, Bool enable,
	MessageType from)
{
	if (enable)
		return addWC(screenNum, base, size, from);
	else
		return delWC(screenNum, base, size, from);
}

static void
undoWC(int screenNum, pointer list)
{
	RangePtr rp;
	struct mem_range_op mro;
	Bool failed;

	rp = list;
	while (rp) {
#ifdef DEBUG
		ErrorF("Undo for (0x%lx,0x%lx), %d\n",
			(unsigned long)rp->mrd.mr_base,
			(unsigned long)rp->mrd.mr_len, rp->wasWC);
#endif
		failed = FALSE;
		if (rp->wasWC) {
			mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
			rp->mrd.mr_flags = MDF_WRITECOMBINE;
			strcpy(rp->mrd.mr_owner, "unknown");
		} else {
			mro.mo_arg[0] = MEMRANGE_SET_REMOVE;
		}
		mro.mo_desc = &rp->mrd;

		if (ioctl(devMemFd, MEMRANGE_SET, &mro)) {
			if (!rp->wasWC) {
				mro.mo_arg[0] = MEMRANGE_SET_UPDATE;
				rp->mrd.mr_flags = MDF_UNCACHEABLE;
				strcpy(rp->mrd.mr_owner, "unknown");
				if (ioctl(devMemFd, MEMRANGE_SET, &mro))
					failed = TRUE;
			} else
				failed = TRUE;
		}
		if (failed) {
			xf86DrvMsg(screenNum, X_WARNING,
				"Failed to restore MTRR range (0x%lx,0x%lx)\n",
				(unsigned long)rp->mrd.mr_base,
				(unsigned long)rp->mrd.mr_len);
		}
		rp = rp->next;
	}
}

#endif /* HAS_MTRR_SUPPORT */

#if defined(__FreeBSD__) && defined(__alpha__)

#define vuip    volatile unsigned int *

static unsigned long msb_set = 0;
static pointer memSBase = 0;
static pointer memBase = 0;

extern int readDense8(pointer Base, register unsigned long Offset);
extern int readDense16(pointer Base, register unsigned long Offset);
extern int readDense32(pointer Base, register unsigned long Offset);
extern void
writeDenseNB8(int Value, pointer Base, register unsigned long Offset);
extern void
writeDenseNB16(int Value, pointer Base, register unsigned long Offset);
extern void
writeDenseNB32(int Value, pointer Base, register unsigned long Offset);
extern void
writeDense8(int Value, pointer Base, register unsigned long Offset);
extern void
writeDense16(int Value, pointer Base, register unsigned long Offset);
extern void
writeDense32(int Value, pointer Base, register unsigned long Offset);

static int readSparse8(pointer Base, register unsigned long Offset);
static int readSparse16(pointer Base, register unsigned long Offset);
static int readSparse32(pointer Base, register unsigned long Offset);
static void
writeSparseNB8(int Value, pointer Base, register unsigned long Offset);
static void
writeSparseNB16(int Value, pointer Base, register unsigned long Offset);
static void
writeSparseNB32(int Value, pointer Base, register unsigned long Offset);
static void
writeSparse8(int Value, pointer Base, register unsigned long Offset);
static void
writeSparse16(int Value, pointer Base, register unsigned long Offset);
static void
writeSparse32(int Value, pointer Base, register unsigned long Offset);

#include <machine/sysarch.h>

extern int sysarch(int, char *);

struct parms {
	u_int64_t hae;
};

static int
sethae(u_int64_t hae)
{
	struct parms p;
	p.hae = hae;
	return (sysarch(ALPHA_SETHAE, (char *)&p));
}

static pointer
mapVidMemSparse(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
    static Bool was_here = FALSE;

    if (!was_here) {
      was_here = TRUE;

      checkDevMem(FALSE);

      xf86WriteMmio8 = writeSparse8;
      xf86WriteMmio16 = writeSparse16;
      xf86WriteMmio32 = writeSparse32;
      xf86WriteMmioNB8 = writeSparseNB8;
      xf86WriteMmioNB16 = writeSparseNB16;
      xf86WriteMmioNB32 = writeSparseNB32;
      xf86ReadMmio8 = readSparse8;
      xf86ReadMmio16 = readSparse16;
      xf86ReadMmio32 = readSparse32;
	
      memBase = mmap((caddr_t)0, 0x100000000,
		     PROT_READ | PROT_WRITE,
		     MAP_SHARED, devMemFd,
		     (off_t) dense_base());
      memSBase = mmap((caddr_t)0, 0x100000000,
		      PROT_READ | PROT_WRITE,
		      MAP_SHARED, devMemFd,
		      (off_t) memory_base());
      
      if (memSBase == MAP_FAILED || memBase == MAP_FAILED)	{
	FatalError("xf86MapVidMem: Could not mmap framebuffer (%s)\n",
		   strerror(errno));
      }
    }
    return (pointer)((unsigned long)memBase + Base);
}

static void
unmapVidMemSparse(int ScreenNum, pointer Base, unsigned long Size)
{
}

static int
readSparse8(pointer Base, register unsigned long Offset)
{
    register unsigned long result, shift;
    register unsigned long msb;

    Offset += (unsigned long)Base - (unsigned long)memBase;
    shift = (Offset & 0x3) << 3;
      if (Offset >= (1UL << 24)) {
        msb = Offset & 0xf8000000UL;
        Offset -= msb;
	if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
	}
      }

    result = *(vuip) ((unsigned long)memSBase + (Offset << 5));
    result >>= shift;
    return 0xffUL & result;
}

static int
readSparse16(pointer Base, register unsigned long Offset)
{
    register unsigned long result, shift;
    register unsigned long msb;

    Offset += (unsigned long)Base - (unsigned long)memBase;
    shift = (Offset & 0x2) << 3;
    if (Offset >= (1UL << 24)) {
        msb = Offset & 0xf8000000UL;
        Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    result = *(vuip)((unsigned long)memSBase+(Offset<<5)+(1<<(5-2)));
    result >>= shift;
    return 0xffffUL & result;
}

static int
readSparse32(pointer Base, register unsigned long Offset)
{
    return *(vuip)((unsigned long)Base+(Offset));
}

static void
writeSparse8(int Value, pointer Base, register unsigned long Offset)
{
    register unsigned long msb;
    register unsigned int b = Value & 0xffU;

    Offset += (unsigned long)Base - (unsigned long)memBase;
    if (Offset >= (1UL << 24)) {
      msb = Offset & 0xf8000000;
      Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    *(vuip) ((unsigned long)memSBase + (Offset << 5)) = b * 0x01010101;
    mem_barrier();
}

static void
writeSparse16(int Value, pointer Base, register unsigned long Offset)
{
    register unsigned long msb;
    register unsigned int w = Value & 0xffffU;

    Offset += (unsigned long)Base - (unsigned long)memBase;
    if (Offset >= (1UL << 24)) {
      msb = Offset & 0xf8000000;
      Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    *(vuip)((unsigned long)memSBase+(Offset<<5)+(1<<(5-2))) =
      w * 0x00010001;
    mem_barrier();

}

static void
writeSparse32(int Value, pointer Base, register unsigned long Offset)
{
    *(vuip)((unsigned long)Base + (Offset)) = Value;
    mem_barrier();
    return;
}

static void
writeSparseNB8(int Value, pointer Base, register unsigned long Offset)
{
    register unsigned long msb;
    register unsigned int b = Value & 0xffU;

    Offset += (unsigned long)Base - (unsigned long)memBase;
    if (Offset >= (1UL << 24)) {
      msb = Offset & 0xf8000000;
      Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    *(vuip) ((unsigned long)memSBase + (Offset << 5)) = b * 0x01010101;
}

static void
writeSparseNB16(int Value, pointer Base, register unsigned long Offset)
{
    register unsigned long msb;
    register unsigned int w = Value & 0xffffU;

    Offset += (unsigned long)Base - (unsigned long)memBase;
    if (Offset >= (1UL << 24)) {
      msb = Offset & 0xf8000000;
      Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    *(vuip)((unsigned long)memSBase+(Offset<<5)+(1<<(5-2))) =
      w * 0x00010001;
}

static void
writeSparseNB32(int Value, pointer Base, register unsigned long Offset)
{
    *(vuip)((unsigned long)Base + (Offset)) = Value;
    return;
}

void (*xf86WriteMmio8)(int Value, pointer Base, unsigned long Offset) 
     = writeDense8;
void (*xf86WriteMmio16)(int Value, pointer Base, unsigned long Offset)
     = writeDense16;
void (*xf86WriteMmio32)(int Value, pointer Base, unsigned long Offset)
     = writeDense32;
void (*xf86WriteMmioNB8)(int Value, pointer Base, unsigned long Offset) 
     = writeDenseNB8;
void (*xf86WriteMmioNB16)(int Value, pointer Base, unsigned long Offset)
     = writeDenseNB16;
void (*xf86WriteMmioNB32)(int Value, pointer Base, unsigned long Offset)
     = writeDenseNB32;
int  (*xf86ReadMmio8)(pointer Base, unsigned long Offset) 
     = readDense8;
int  (*xf86ReadMmio16)(pointer Base, unsigned long Offset)
     = readDense16;
int  (*xf86ReadMmio32)(pointer Base, unsigned long Offset)
     = readDense32;

#endif /* __FreeBSD__ && __alpha__ */
