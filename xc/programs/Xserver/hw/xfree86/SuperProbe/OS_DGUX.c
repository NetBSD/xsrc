/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/OS_DGUX.c,v 1.1.2.4 1999/08/03 09:41:41 hohndel Exp $ */
/*
 * INTEL DG/UX RELEASE 4.20 MU04
 * Copyright Takis Psarogiannakopoulos
 * e-mail <takis@pmms.cam.ac.uk>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 * XCONSORTIUM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE.
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FITNESS, IN NO EVENT SHALL XCONSORTIUM BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */


#include "Probe.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/kd.h>
#include <sys/proc.h>
#include <sys/mman.h>

/* Definition of SI86IOPL for DG/ux 
 * According to sysi86 of DG/ux 
 * Unixware
 */

#define DG_NO_SYSI86 1

#include <sys/sysi86.h>
#if defined(DG_NO_SYSI86)
#define SI86IOPL 112
#endif

#define DEV_MEM	"/dev/mem"

int Takis_fd;
static int BIOS_fd = -1;

/*
 * OpenVideo --
 */
int OpenVideo()
{
        int fd;
        char takis[20];

        if (geteuid() != 0)
        {
                fprintf(stderr,
                        "%s: Must be run as root or installed suid-root\n",
                        MyName);
                return(-1);
        }
        if ((fd = open("/dev/console", O_RDWR, 0)) < 0)
        {
                fprintf(stderr, "%s: Cannot open /dev/console\n",
                        MyName);
                return(-1);
        }
        close(fd);
        sprintf(takis, "/dev/console");

        if ((Takis_fd = open(takis, O_RDWR|O_NDELAY, 0)) < 0)
        {
                fprintf(stderr, "%s: Could not open system Console \n",
                        MyName);
                return(-1);
        }
        if (sysi86(SI86IOPL, 3)< 0)
        {
            return(-1);
        }

    return(Takis_fd);
}


/*
 * CloseVideo --
 *
 * Disable access to the video hardware, ie Close system console,
 * and re-enable IO protection.
 */
void CloseVideo()
{
    if (Takis_fd>0)
    {
       sysi86(SI86IOPL, 0);    /* Disable I/O ports */
       close(Takis_fd);
    }
}


/*
 * MapVGA --
 */
Byte *MapVGA()
{
        int fd;
        Byte *base;

        if ((fd = open(DEV_MEM, O_RDWR)) < 0)
        {
                fprintf(stderr, "%s: Failed to open %s\n", MyName, DEV_MEM);
                return((Byte *)0);
        }
        base = (Byte *)mmap((caddr_t)0, 0x10000, PROT_READ|PROT_WRITE,
                            MAP_SHARED, fd, (off_t)0xA0000);
        close(fd);
        if ((long)base == -1)
        {
                fprintf(stderr, "%s: Failed to mmap framebuffer\n", MyName);
                return((Byte *)0);
        }
        return(base);
}


Byte *MapMem(address,size)
	unsigned long address;
	unsigned long size;
{
	return((Byte*)0);
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


void UnMapMem(base,size)
	Byte *base;
	unsigned long size;
{
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
        Byte *Base = Bios_Base + Offset;
        Word *tmp;
        Byte *ptr;
        unsigned long page_offset = (unsigned long)Base & 0xFFF;
        unsigned long mmap_base = (unsigned long)Base & ~0xFFF;
        unsigned long mmap_len = Len + page_offset;

        if (BIOS_fd == -1)
        {
                if ((BIOS_fd = open(DEV_MEM, O_RDONLY, 0)) < 0)
                {
                        fprintf(stderr, "%s: cannot open %s\n",
                                MyName, DEV_MEM);
                        return(-1);
                }
        }
        if ((off_t)((off_t)Base & 0x7FFF) != (off_t)0)
        {
                tmp = (Word *)mmap((caddr_t)0, 0x1000, PROT_READ, MAP_SHARED,
                                   BIOS_fd, (off_t)((off_t)Base & 0xF8000));
                if ((int)tmp == -1)
                {
                        fprintf(stderr, "%s: Failed to mmap() %s, addr=%x\n",
                                MyName, DEV_MEM, (int)Base);
                        return(-1);
                }
                if (*tmp != (Word)0xAA55)
                {
                        fprintf(stderr,
                                "%s: BIOS sanity check failed, addr=%x\n",
                                MyName, (int)Base);
                        return(-1);
                }
                (void)munmap((caddr_t)tmp, 0x1000);
        }
        ptr = (Byte *)mmap((caddr_t)0, mmap_len, PROT_READ, MAP_SHARED,
                           BIOS_fd, (off_t)mmap_base);
        if ((int)ptr == -1)
        {
                fprintf(stderr, "%s: Failed to mmap() %s, addr=%x\n",
                        MyName, DEV_MEM, (int)Base);
                return(-1);
        }
        (void)memcpy((void *)Buffer, (void *)(ptr + page_offset), Len);
        (void)munmap((caddr_t)ptr, mmap_len);
        return(Len);
}


/*
 * EnableIOPort --
 */
/*ADDED FOR INTEL DG/ux */
int EnableIOPorts(NumPorts, Ports)
CONST int NumPorts;
CONST Word *Ports;
{
	return(0);
}

/*
 * DisableIOPort --
 */

/*ADDED FOR INTEL DG/ux */
int DisableIOPorts(NumPorts, Port)
CONST int NumPorts;
CONST Word *Port;
{
	return(0);
}

/*
 * ShortSleep --
 */

/*ADDED FOR INTEL DG/ux */
void ShortSleep(Delay)
int Delay;
{
        usleep(Delay * 1000);
}

/* Added to allow lightweight processes to access I/O directly  */

#if defined(DGUX) && defined(DG_NO_SYSI86)
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
#endif

