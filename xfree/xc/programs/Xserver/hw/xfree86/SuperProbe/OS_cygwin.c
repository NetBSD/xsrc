/*
 * (c) Copyright 1998,1999 by Sebastien Marineau <sebastien@qnx.com>
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
 * $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/OS_cygwin.c,v 1.1 2000/08/23 20:56:50 dawes Exp $
 */

#include "Probe.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <windows.h>
#include <sys/mman.h>
#include <sys/cygwin.h>

static int VT_fd = -1;
static int BIOS_fd = -1;

/*
 * OpenVideo --
 *
 * Enable access to the installed video hardware.
 */
int OpenVideo()
{
	int fd;
	char fn[20];

	if (geteuid() != 0) {
		fprintf(stderr, "%s: Must be run as root\n", MyName);
		return(-1);
	}

	if ((fd = open("/dev/conin", O_WRONLY, 0)) < 0) {
		fprintf(stderr, "%s: Cannot open /dev/conin\n", MyName);
		return(-1);
	}

	return fd;
}

/*
 * CloseVideo --
 *
 * Disable access to the video hardware.
 */
void CloseVideo()
{
	int fd;

	if (VT_fd > 0) {
		close(VT_fd);
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

Byte *MapMem(address, size)
	unsigned long address;
	unsigned long size;
{
	int fd;
	Byte *base;

	if ((fd = open("/dev/zero", O_RDWR)) < 0) {
		fprintf(stderr, "%s: Failed to open /dev/zero\n", MyName);
		return((Byte *)0);
	}

	base = (Byte *)mmap((void *)0, size, PROT_READ|PROT_WRITE,
			    MAP_SHARED, fd, (off_t)address);
	close(fd);

	if ((long)base == -1) {
        fprintf(stderr, "%s: Failed to mmap framebuffer\n", MyName);
		return((Byte *)0);
	}

	return base;
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
	munmap((void *)base, size);
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
	Word tmp;
	Byte *Base = Bios_Base + Offset;
	Byte *mybase;
	off_t myoffset;
	int mysize;

	if (BIOS_fd == -1) {
		if ((BIOS_fd = open("/dev/mem", O_RDONLY, 0)) < 0) {
			fprintf(stderr, "%s: cannot open /dev/mem\n", MyName);
			return(-1);
		}
	}

	if ((off_t)((off_t)Base & 0x7FFF) != (off_t)0) {
		/*
	 	 * Sanity check...
	 	 */
		(void)lseek(BIOS_fd, (off_t)((off_t)Base & 0xF8000), SEEK_SET);
		(void)read(BIOS_fd, &tmp, 2);
		if (tmp != (Word)0xAA55) {
			fprintf(stderr, "%s: BIOS sanity check failed, addr=%x\n",
				MyName, (int)Base);
			return(-1);
		}
	}

	if (lseek(BIOS_fd, (off_t)Base, SEEK_SET) < 0) {
		fprintf(stderr, "%s: BIOS seek failed\n", MyName);
		return(-1);
	}

	if (read(BIOS_fd, Buffer, Len) != Len) {
		fprintf(stderr, "%s: BIOS read failed\n", MyName);
		return(-1);
	}

	return Len;
}

/*
 * EnableIOPort --
 *
 * Enable access to 'NumPorts' IO ports listed in array 'Ports'.
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
 * Disable access to 'NumPorts' IO ports listed in array  'Ports'.
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
	usleep(Delay * 1000);
}
