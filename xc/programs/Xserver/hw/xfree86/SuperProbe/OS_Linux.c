/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/OS_Linux.c,v 3.6 1996/08/20 12:26:09 dawes Exp $ */
/*
 * (c) Copyright 1993,1994 by Orest Zborowski <orestz@eskimo.com>
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
 */

/* $XConsortium: OS_Linux.c /main/4 1995/11/13 11:12:42 kaleb $ */

#include "Probe.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/kd.h>
#include <sys/vt.h>
#include <sys/mman.h>

#ifdef __alpha__
extern unsigned long _bus_base(void) __attribute__((const));
#define BUS_BASE _bus_base()
#else
#define BUS_BASE 0
#endif

static int VT_fd = -1;
static int VT_num = -1;
static int VT_cur = -1;
static int BIOS_fd = -1;

/*
 * OpenVideo --
 *
 * Enable access to the installed video hardware.  For Linux, open a new
 * VT, and disable IO protection, since we may need to get at extended
 * registers (full 16-bit decoding).
 */
int OpenVideo()
{
	int fd;
	char fn[20];
	struct vt_stat vts;

	if (geteuid() != 0)
	{
		fprintf(stderr,
			"%s: Must be run as root or installed suid-root\n",
			MyName);
		return(-1);
	}

	if ((fd = open("/dev/console", O_WRONLY, 0)) < 0)
	{
		fprintf(stderr, "%s: Cannot open /dev/console\n", MyName);
		return(-1);
	}
	/*
	 * The Linux kernel doesn't keep track of the VT to return
	 * to, so we have to maintain that ourselves. (Hmm... would
	 * it work like a stack [chain of vt parents])?
	 */
	if (ioctl(fd, VT_GETSTATE, &vts) == 0)
	{
		VT_cur = vts.v_active;
	}
	else
	{
		VT_cur = -1;
	}
	if ((ioctl(fd, VT_OPENQRY, &VT_num) < 0) || (VT_num == -1))
	{
		fprintf(stderr, "%s: No free VTs\n", MyName);
		return(-1);
	}
	close(fd);
	sprintf(fn, "/dev/tty%d", VT_num);
	if ((VT_fd = open(fn, O_RDWR|O_NDELAY, 0)) < 0)
	{
		fprintf(stderr, "%s: Could not open VT %s\n", MyName, fn);
		return(-1);
	}
	/*
	 * Hmmm... I thought WAITACTIVE didn't activate, only
	 * waited for the vt to activate.  Am I wrong in that
	 * assumption?
	 */
	if (ioctl(VT_fd, VT_ACTIVATE, VT_num) != 0)
	{
		fprintf(stderr, "%s: VT_ACTIVATE failed!\n", MyName);
		return(-1);
	}
	if (ioctl(VT_fd, VT_WAITACTIVE, VT_num) != 0)
	{
		fprintf(stderr, "%s: VT_WAITACTIVE failed!\n", MyName);
		return(-1);
	}
	/*
	 * Give the other VT time to release.
	 */
	sleep(1);
	/*
	 * Get IOPL so we can get at all the I/O ports.
	 */
	iopl(3);
	return(VT_fd);
}

/*
 * CloseVideo --
 *
 * Disable access to the video hardware.  For Linux, close the VT, and
 * re-enable IO protection.
 */
void CloseVideo()
{
	int fd;

	iopl(0);
	if (VT_fd > 0)
	{
		close(VT_fd);
		if (VT_cur >= 0)
		{
			if ((fd = open("/dev/console", O_WRONLY, 0)) >= 0)
			{
				ioctl(fd, VT_ACTIVATE, VT_cur);
				close(fd);
			}
		}
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
	int fd;
	Byte *base;

	if ((fd = open("/dev/mem", O_RDWR)) < 0)
	{
		fprintf(stderr, "%s: Failed to open /dev/mem\n", MyName);
		return((Byte *)0);
	}
	base = (Byte *)mmap((caddr_t)0, 0x10000, PROT_READ|PROT_WRITE,
			    MAP_SHARED, fd, (off_t)0xA0000 + BUS_BASE);
	close(fd);
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
	munmap((caddr_t)base, 0x10000);
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

	if (BIOS_fd == -1)
	{
		if ((BIOS_fd = open("/dev/mem", O_RDONLY, 0)) < 0)
		{
			fprintf(stderr, "%s: cannot open /dev/mem\n", MyName);
			return(-1);
		}
	}

#ifdef __alpha__
	if ((myoffset = ((off_t)Base & 0x7FFF)) != 0)
		Base = (Byte *)((off_t)Base & 0xF8000);

	mysize = myoffset + Len;
	mybase = (unsigned char *)mmap((caddr_t)0, mysize, PROT_READ,
				       MAP_SHARED, BIOS_fd,
				       (off_t)Base + BUS_BASE);

	if (mybase == (unsigned char *)-1UL) {
		fprintf(stderr, "%s: Failed to mmap /dev/mem (%d)\n",
			MyName, errno);
		return(-1);
	}

	if (myoffset != (off_t)0)
	{
		/*
	 	 * Sanity check...
	 	 */
		tmp = *(Word *)mybase;
		if (tmp != (Word)0xAA55)
		{
			fprintf(stderr,
				"%s: BIOS sanity check failed, addr=%lx\n",
				MyName, (long)Base);
			return(-1);
		}
	}

	memcpy(Buffer, &mybase[myoffset], Len);

	munmap((caddr_t)mybase, mysize);

#else /* __alpha__ */

	if ((off_t)((off_t)Base & 0x7FFF) != (off_t)0)
	{
		/*
	 	 * Sanity check...
	 	 */
		(void)lseek(BIOS_fd, (off_t)((off_t)Base & 0xF8000), SEEK_SET);
		(void)read(BIOS_fd, &tmp, 2);
		if (tmp != (Word)0xAA55)
		{
			fprintf(stderr,
				"%s: BIOS sanity check failed, addr=%x\n",
				MyName, (int)Base);
			return(-1);
		}
	}
	if (lseek(BIOS_fd, (off_t)Base, SEEK_SET) < 0)
	{
		fprintf(stderr, "%s: BIOS seek failed\n", MyName);
		return(-1);
	}
	if (read(BIOS_fd, Buffer, Len) != Len)
	{
		fprintf(stderr, "%s: BIOS read failed\n", MyName);
		return(-1);
	}
#endif /* __alpha__ */

	return(Len);
}

/*
 * EnableIOPort --
 *
 * Enable access to 'NumPorts' IO ports listed in array 'Ports'.  For Linux,
 * we've disabled IO protections so this is a no-op.
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
	usleep(Delay * 1000);
}
