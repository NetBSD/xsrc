/* $XFree86: xc/programs/Xserver/hw/xfree86/SuperProbe/OS_GNUmach.c,v 1.1 1998/08/16 10:25:38 dawes Exp $ */
/*
 * (c) Copyright 1993,1994 by Robert V. Baron 
 *			<Robert.Baron@ernst.mach.cs.cmu.edu>
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
 * ROBERT V. BARON  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Robert V. Baron shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Robert V. Baron.
 *
 */
/* $XConsortium: OS_Mach.c /main/5 1996/02/21 17:11:12 kaleb $ */
/* GNU/Hurd port by UCHIYAMA Yasushi */
#define _GNU_SOURCE
#define MACH3
#include<hurd.h>
#include<device/device.h>

#include "Probe.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

/* These can be Mach 2.5 or 3.0 headers */
#include <mach.h>
#include <mach_error.h>
#include <mach/message.h>

static device_t device_master = MACH_PORT_NULL;
static int iopl_dev = MACH_PORT_NULL;
static memory_object_t iopl_mem = MACH_PORT_NULL;
static vm_address_t mapped_address;

/*
 * OpenVideo --
 *
 * Enable access to the installed video hardware.  For Mach, we disable
 * IO protection, since this is currently the only way to access any
 * IO registers.
 */
int screen_addr;

int OpenVideo()
{
  int ret;
#define C /*Bios_Base*/ 0xc0000
#define S 0x40000
  if(KERN_SUCCESS != get_privileged_ports(0,&device_master)){
    fprintf(stderr,"Failed get_privileged_ports\n");
    return -1;
  }
  if (KERN_SUCCESS != device_open(device_master,D_READ|D_WRITE,"iopl",&iopl_dev)){
    fprintf(stderr, "Failed to device_open iopl\n");
    return -1;
  }
  if(KERN_SUCCESS != device_map(iopl_dev,VM_PROT_READ|VM_PROT_WRITE, C , S ,&iopl_mem,0)){
    fprintf(stderr, "Failed to device_map\n");
    return -1;
  }
  if(KERN_SUCCESS != vm_map(mach_task_self(),
			    &mapped_address,S,
			    0,1,iopl_mem,C,0,VM_PROT_READ|VM_PROT_WRITE,
			    VM_PROT_READ|VM_PROT_WRITE,VM_INHERIT_NONE)){
    fprintf(stderr,"Failed to vm_map\n");
    return -1;
  }

  screen_addr = (int)mapped_address;

  return TRUE;

}

/*
 * CloseVideo --
 *
 * Disable access to the video hardware.  For Mach, we re-enable
 * IO protection.
 */
void CloseVideo()
{
	if (KERN_SUCCESS != vm_deallocate(mach_task_self(), screen_addr, S)) {
		fprintf(stderr, "Failed vmdeallocate %x\n", S);
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
        return((Byte *)0);
}

Byte *MapMem(address,size)
	unsigned long address;
	unsigned long size;
{
	return((Byte *)0);
}

/*
 * UnMapVGA --
 *
 * Unmap the VGA memory window.
 */
void UnMapVGA(base)
Byte *base;
{
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
	Word tmp;
	Byte *Base;

	Base = (Byte *)(screen_addr + Bios_Base + Offset - C);
#ifdef DEBUG
	{int i;
		fprintf(stderr, "ReadBIOS(Offset %x, Buffer %x, Len %x) .. ",
			Offset, Buffer, Len);
		for (i=0;i<Len;i++)
			fprintf(stderr," [%c](%x)|", *(Base+i), *(Base+i));
		fprintf(stderr,"\n");
	}
#endif
	bcopy(Base, Buffer, Len);
	return (Len);
}

/*
 * EnableIOPort --
 *
 * Enable access to 'NumPorts' IO ports listed in array 'Ports'.  For Mach, 
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
 * Disable access to 'NumPorts' IO ports listed in array  'Ports'.  For Mach, 
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
#ifdef MACH3
	struct trial {
		mach_msg_header_t	h;
		mach_msg_type_t	t;
		int		d;
	} msg_rcv;
	mach_port_t		my_port;
#else
	/* This does Mach 2.5 IPC */	
	struct trial {
		msg_header_t	h;
		msg_type_t	t;
		int		d;
	} msg_rcv;
	port_t		my_port;
#endif
	kern_return_t	error;
	int		ret;

	if ((error = mach_port_allocate(mach_task_self(),MACH_PORT_RIGHT_PORT_SET,&my_port)) != KERN_SUCCESS) {
		printf("ShortSleep: mach_port_allocate failed with %d: %s\n", 
			error, mach_error_string(error));
		return;
	}
	
#ifdef MACH3
	msg_rcv.h.msgh_size = sizeof(msg_rcv);
	if ((ret = mach_msg_receive(&msg_rcv.h, MACH_RCV_TIMEOUT,	Delay)) != MACH_RCV_TIMED_OUT) {
#else
	if ((ret = mach_msg_receive(&msg_rcv.h, RCV_TIMEOUT,	Delay)) != RCV_TIMED_OUT) {
	msg_rcv.h.msg_local_port = my_port;
#endif
		mach_error("ShortSleep: mach_msg_receive returned ", ret);
		return;
	}

	if ((error = mach_port_deallocate(mach_task_self(), my_port)) != KERN_SUCCESS) {
		printf("ShortSleep: port_deallocate failed with %d: %s\n", 
			error, mach_error_string(error));
		return;
	}
}
