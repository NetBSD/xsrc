/*
 * (c) Copyright 1998 by Sebastien Marineau <sebastien@qnx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * OREST ZBOROWSKI BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Orest Zborowski shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Orest Zborowski.
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/OS_QNX.c,v 1.1.2.2 1999/07/23 13:42:30 hohndel Exp $
 */

#include "Probe.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/console.h>
#include <sys/mman.h>
#include <i86.h>

static int CONS_fd = -1;
static int PHMEM_fd = -1;

/*
 * OpenVideo --
 *
 * Enable access to the installed video hardware. On QNX, only allow the
 * user to run from a text console, so that we do not mess any Photon 
 * drivers.
 */
int OpenVideo()
{
	int fd;

	if (geteuid() != 0)
	{
		fprintf(stderr,
			"%s: Must be run as root\n",
			MyName);
		return(-1);
	}

/*	if ((fd = open("/dev/con1", O_WRONLY, 0)) < 0)
	{
		fprintf(stderr, "%s: Cannot open /dev/console\n", MyName);
		return(-1);
	}*/
	CONS_fd = fd;
	return (1);
}

/*
 * CloseVideo --
 *
 * Disable access to the video hardware.
 */
void CloseVideo()
{

	if(CONS_fd > 0) {
		close(CONS_fd);
		CONS_fd = -1;
		}
}
/*
 * MapVGA --
 *
 * Map the VGA memory window (0xA0000-0xAFFFF) as read/write memory for
 * the process for use in probing memory.
 */
Byte *MapVGA()
{
	return( MapMem(0xA0000,0x10000) );
}

/* 
 * MapMem() is used to map a chunk of physical memory. We use the
 * shm_open ("Physical"...) and the mmap mechanism.
 *
 */
Byte *MapMem(address, size)
	unsigned long address;
	unsigned long size;
{
	int fd;
	Byte *base;

	if(PHMEM_fd < 0) {	
		if ((fd = shm_open("Physical", O_RDWR, 0777)) < 0) {
			fprintf(stderr, "%s: Failed to open /dev/mem\n", MyName);
			return((Byte *)0);
			}
		PHMEM_fd = fd;
		}
	base = (Byte *)mmap((caddr_t)0, size, PROT_READ|PROT_WRITE,
			    MAP_SHARED, PHMEM_fd, (off_t)address);
	/* close(fd); */
	if ((long)base == -1)
	{
                fprintf(stderr, "%s: Failed to mmap framebuffer\n", MyName);
		return((Byte *)0);
	}
	return(base);
}

/*
 * UnMapVGA --
 *
 * Unmap the VGA memory window.
 */
void UnMapVGA(base)
	Byte *base;
{
	UnMapMem(base,0x10000);
	return;
}

void UnMapMem(base,size)
	Byte *base;
	unsigned long size;
{
	munmap((caddr_t)base, size);
	return;
}

/*
 * ReadBIOS --
 *
 * Read 'Len' bytes from the video BIOS at address 'Bios_Base'+'Offset' into
 * buffer 'Buffer'.
 */
int ReadBIOS(Offset, Buffer, Len)
unsigned Offset;
Byte *Buffer;
int Len;
{
	Byte *VirtBase;

	if (PHMEM_fd == -1)
	{
		if ((PHMEM_fd = shm_open("Physical", O_RDWR, 0777)) < 0)
		{
			fprintf(stderr, "%s: cannot open Physical memory\n", MyName);
			return(-1);
		}
	}

	/* Use mmap to map BIOS region. Note the restrictions on 
	 * mmap alignement of offset variable (which must be on a page
	 * boundary).
	 */
	VirtBase = (Byte *) mmap(0, (size_t)((Offset & 0x7fff) + Len), PROT_READ, 
		MAP_SHARED, PHMEM_fd, 
		(off_t) (Bios_Base + (Offset & 0xffff8000)));
	if(VirtBase == -1) {
		fprintf(stderr,
			"%s: Could not mmap BIOS memory space, errno=%i\n",
			MyName, errno);
		return(-1);
		}

	/* So now we have our mapping to the BIOS region */
	/* Do a sanity check on what we have just mapped */	
	if (((off_t)((off_t)Offset & 0x7FFF) != (off_t)0) &&
		(VirtBase[0] != 0x55) &&
		(VirtBase[1] != 0xaa)) {
		fprintf(stderr,
			"%s: BIOS sanity check failed, addr=%x\n",
			MyName, (int)Bios_Base + Offset);
		munmap(VirtBase, (Offset & 0x7fff) + Len);
		return(-1);
		}
	memcpy(Buffer, VirtBase + (Offset & 0x7fff), Len);
	munmap(VirtBase, (Offset & 0x7fff) + Len);
	return(Len);
}

/*
 * EnableIOPort --
 *
 * Enable access to 'NumPorts' IO ports listed in array 'Ports'. The
 * app is already compiled as Privity level 1 (-T1) so that IO port acces
 * is already granted. 
 */
/*ARGSUSED*/
int EnableIOPorts(NumPorts, Ports)
CONST int NumPorts;
CONST Word *Ports;
{
	return(0);
}

/*
 * DisableIOPort --
 *
 * Disable access to 'NumPorts' IO ports listed in array  'Ports'.  For Linux,
 * we've disabled IO protections so this is a no-op.
 */
/*ARGSUSED*/
int DisableIOPorts(NumPorts, Port)
CONST int NumPorts;
CONST Word *Port;
{
	return(0);
}

/*
 * ShortSleep --
 *
 * Sleep for the number of milliseconds specified in 'Delay'.
 */
void ShortSleep(Delay)
int Delay;
{
	delay(Delay);
}
