/*
 * Copyright © 1999 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/tinyx/txmap.c,v 1.3 2005/01/31 04:32:48 dawes Exp $ */
/*
 * Copyright (c) 2004, 2005 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tinyx.h"

#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#ifdef HAS_MTRR
#include <asm/mtrr.h>
#endif

#include <sys/ioctl.h>

void *
KdMapDevice (CARD32 addr, CARD32 size)
{
#ifdef WINDOWS
    void    *a;
    void    *d;

    d = VirtualAlloc (NULL, size, MEM_RESERVE, PAGE_NOACCESS);
    if (!d)
	return NULL;
    DRAW_DEBUG ((DEBUG_S3INIT, "Virtual address of 0x%x is 0x%x", addr, d));
    a = VirtualCopyAddr (addr);
    DRAW_DEBUG ((DEBUG_S3INIT, "Translated address is 0x%x", a));
    if (!VirtualCopy (d, a, size, 
		      PAGE_READWRITE|PAGE_NOCACHE|PAGE_PHYSICAL))
    {
	DRAW_DEBUG ((DEBUG_FAILURE, "VirtualCopy failed %d",
		    GetLastError ()));
	return NULL;
    }
    DRAW_DEBUG ((DEBUG_S3INIT, "Device mapped successfully"));
    return d;
#endif
#ifdef linux
    void    *a;
    int	    fd;

#ifdef __arm__
    fd = open ("/dev/mem", O_RDWR|O_SYNC);
#else
    fd = open ("/dev/mem", O_RDWR);
#endif
    if (fd < 0)
	FatalError ("KdMapDevice: failed to open /dev/mem (%s)\n",
		    strerror (errno));
    
    a = mmap ((caddr_t) 0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr);
    close (fd);
    if ((long) a == -1)
	FatalError ("KdMapDevice: failed to map frame buffer (%s)\n",
		    strerror (errno));
    return a;
#endif
#ifdef VXWORKS
    return (void *) addr;
#endif
}

void
KdUnmapDevice (void *addr, CARD32 size)
{
#ifdef WINDOWS
    VirtualFree (addr, size, MEM_DECOMMIT);
    VirtualFree (addr, 0, MEM_RELEASE);
#endif
#ifdef linux
    munmap (addr, size);
#endif
#ifdef VXWORKS
    ;
#endif
}

#if defined(HAS_MTRR) && \
    (defined(MTRRIOC_ADD_ENTRY) || defined(MTRRIOC_DEL_ENTRY))
static int  mtrr;
#endif

void
KdSetMappedMode (CARD32 addr, CARD32 size, int mode)
{
#if defined(HAS_MTRR) && defined(MTRRIOC_ADD_ENTRY)
    struct mtrr_sentry  sentry;
    unsigned long    	base, bound;
    unsigned int	type;

    if (addr < 0x100000)
	return;
    if (!mtrr)
	mtrr = open ("/proc/mtrr", 2);
    if (mtrr > 0)
    {
	base = addr & ~((1<<22)-1);
	bound = ((addr + size) + ((1<<22) - 1)) & ~((1<<22) - 1);
	switch (mode) {
	case KD_MAPPED_MODE_REGISTERS:
	    type = MTRR_TYPE_UNCACHABLE;
	    break;
	case KD_MAPPED_MODE_FRAMEBUFFER:
	    type = MTRR_TYPE_WRCOMB;
	    break;
	default:
	    type = MTRR_TYPE_UNCACHABLE;
	    break;
	}
	sentry.base = base;
	sentry.size = bound - base;
	sentry.type = type;
	
	if (ioctl (mtrr, MTRRIOC_ADD_ENTRY, &sentry) < 0)
	    ErrorF ("MTRRIOC_ADD_ENTRY failed 0x%lx 0x%lx %d (errno %d)\n",
		    base, bound - base, type, errno);
    }
#endif
}

void
KdResetMappedMode (CARD32 addr, CARD32 size, int mode)
{
#if defined(HAS_MTRR) && defined(MTRRIOC_DEL_ENTRY)
    struct mtrr_sentry  sentry;
    unsigned long    	base, bound;
    unsigned int	type;

    if (addr < 0x100000)
	return;
    if (!mtrr)
	mtrr = open ("/proc/mtrr", 2);
    if (mtrr > 0)
    {
	base = addr & ~((1<<22)-1);
	bound = ((addr + size) + ((1<<22) - 1)) & ~((1<<22) - 1);
	switch (mode) {
	case KD_MAPPED_MODE_REGISTERS:
	    type = MTRR_TYPE_UNCACHABLE;
	    break;
	case KD_MAPPED_MODE_FRAMEBUFFER:
	    type = MTRR_TYPE_WRCOMB;
	    break;
	default:
	    type = MTRR_TYPE_UNCACHABLE;
	    break;
	}
	sentry.base = base;
	sentry.size = bound - base;
	sentry.type = type;
	
	if (ioctl (mtrr, MTRRIOC_DEL_ENTRY, &sentry) < 0)
	    ErrorF ("MTRRIOC_DEL_ENTRY failed 0x%lx 0x%lx %d (errno %d)\n",
		    base, bound - base, type, errno);
    }
#endif
}
