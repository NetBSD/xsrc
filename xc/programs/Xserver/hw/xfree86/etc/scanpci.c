/* $XConsortium: scanpci.c /main/25 1996/10/27 11:48:40 kaleb $ */
/*
 *  name:             scanpci.c
 *
 *  purpose:          This program will scan for and print details of
 *                    devices on the PCI bus.

 *  author:           Robin Cutshaw (robin@xfree86.org)
 *
 *  supported O/S's:  SVR4, UnixWare, SCO, Solaris,
 *                    FreeBSD, NetBSD, 386BSD, BSDI BSD/386,
 *                    Linux, Mach/386, ISC, DGUX
 *                    DOS (WATCOM 9.5 compiler)
 *
 *  compiling:        [g]cc scanpci.c -o scanpci
 *                    for SVR4 (not Solaris), UnixWare use:
 *                        [g]cc -DSVR4 scanpci.c -o scanpci
 *                    for DOS, watcom 9.5:
 *                        wcc386p -zq -omaxet -7 -4s -s -w3 -d2 name.c
 *                        and link with PharLap or other dos extender for exe
 *
 * case Intel DG/ux:  gcc -DDGUX scanpci.c -o scanpci (with gcc-DG-2.7.2.88)
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/etc/scanpci.c,v 3.34.2.35 1999/11/26 15:24:08 hohndel Exp $ */

/*
 * Copyright 1995 by Robin Cutshaw <robin@XFree86.Org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the above listed copyright holder(s)
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.  The above listed
 * copyright holder(s) make(s) no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM(S) ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#if !defined(DGUX)
#if defined(__SVR4)
#if !defined(SVR4)
#define SVR4
#endif
#endif
#endif

#if defined(DGUX)
#define DG_NO_SYSI86 1
#endif

#ifdef __EMX__
#define INCL_DOSFILEMGR
#include <os2.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#if defined(SVR4) && !defined(DGUX)
#if defined(sun)
#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif
#endif
#include <sys/proc.h>
#include <sys/tss.h>
#if defined(NCR)
#define __STDC
#include <sys/sysi86.h>
#undef __STDC
#else
#include <sys/sysi86.h>
#endif
#if defined(__SUNPRO_C) || defined(sun) || defined(__sun)
#include <sys/psw.h>
#else
#include <sys/seg.h>
#endif
#if defined(sun) && defined (i386) && defined (SVR4)	/* Solaris? */
# if !defined(V86SC_IOPL)				/* Solaris 7? */
#include <sys/v86.h>					/* Nope */
# else
/* Do nothing what so ever */				/* Yup */
# endif /* V86SC_IOPL */
#else
#  include <sys/v86.h>					/* Not solaris */
#endif /* sun/i386/svr4 */
#endif
#if defined(__FreeBSD__) || defined(__386BSD__)
#include <sys/file.h>
#include <machine/console.h>
#ifndef GCCUSESGAS
#define GCCUSESGAS
#endif
#endif
#if defined(__NetBSD__)
#include <sys/param.h>
#include <sys/file.h>
#include <machine/sysarch.h>
#ifndef GCCUSESGAS
#define GCCUSESGAS
#endif
#endif
#if defined(__bsdi__)
#include <sys/file.h>
#include <sys/ioctl.h>
#include <i386/isa/pcconsioctl.h>
#ifndef GCCUSESGAS
#define GCCUSESGAS
#endif
#endif
#if defined(DGUX)
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/param.h>
#include <sys/kd.h>
#include <sys/sysi86.h>  /* Definition of SI86IOPL for DG/ux */
#if defined(DG_NO_SYSI86)
#define SI86IOPL 112
#endif
#endif /* DGUX */
#if defined(SCO) || defined(ISC)
#ifndef ISC
#include <sys/console.h>
#endif
#include <sys/param.h>
#include <sys/immu.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/tss.h>
#include <sys/sysi86.h>
#include <sys/v86.h>
#endif
#if defined(Lynx_22)
#ifndef GCCUSESGAS
#define GCCUSESGAS
#endif
#endif


#if defined(__WATCOMC__)

#include <stdlib.h>
void outl(unsigned port, unsigned data);
#pragma aux outl =  "out    dx, eax" parm [dx] [eax];
void outb(unsigned port, unsigned data);
#pragma aux outb = "out    dx, al" parm [dx] [eax];
unsigned inl(unsigned port);
#pragma aux inl = "in     eax, dx" parm [dx];
unsigned inb(unsigned port);
#pragma aux inb = "xor    eax,eax" "in     al, dx" parm [dx];

#else /* __WATCOMC__ */

#if defined(__GNUC__)

#if !defined(__alpha__) && !defined(__powerpc__) && !defined(__sparc__)
#if defined(GCCUSESGAS)
#define OUTB_GCC "outb %0,%1"
#define OUTL_GCC "outl %0,%1"
#define INB_GCC  "inb %1,%0"
#define INL_GCC  "inl %1,%0"
#else
#define OUTB_GCC "out%B0 (%1)"
#define OUTL_GCC "out%L0 (%1)"
#define INB_GCC "in%B0 (%1)"
#define INL_GCC "in%L0 (%1)"
#endif /* GCCUSESGAS */

static void outb(unsigned short port, unsigned char val) {
     __asm__ __volatile__(OUTB_GCC : :"a" (val), "d" (port)); }
static void outl(unsigned short port, unsigned long val) {
     __asm__ __volatile__(OUTL_GCC : :"a" (val), "d" (port)); }
static unsigned char inb(unsigned short port) { unsigned char ret;
     __asm__ __volatile__(INB_GCC : "=a" (ret) : "d" (port)); return ret; }
static unsigned long inl(unsigned short port) { unsigned long ret;
     __asm__ __volatile__(INL_GCC : "=a" (ret) : "d" (port)); return ret; }

#endif /* !defined(__alpha__) && !defined(__powerpc__) && !defined(__sparc__) */
#else  /* __GNUC__ */

#if defined(__STDC__) && (__STDC__ == 1)
# if !defined(NCR)
#  define asm __asm
# endif
#endif

#if defined(__SUNPRO_C)
/*
 * This section is a gross hack in if you tell anyone that I wrote it,
 * I'll deny it.  :-)
 * The leave/ret instructions are the big hack to leave %eax alone on return.
 */
	unsigned char inb(int port) {
		asm("	movl 8(%esp),%edx");
		asm("	subl %eax,%eax");
		asm("	inb  (%dx)");
		asm("	leave");
		asm("	ret");
	}

	unsigned short inw(int port) {
		asm("	movl 8(%esp),%edx");
		asm("	subl %eax,%eax");
		asm("	inw  (%dx)");
		asm("	leave");
		asm("	ret");
	}

	unsigned long inl(int port) {
		asm("	movl 8(%esp),%edx");
		asm("	inl  (%dx)");
		asm("	leave");
		asm("	ret");
	}

	void outb(int port, unsigned char value) {
		asm("	movl 8(%esp),%edx");
		asm("	movl 12(%esp),%eax");
		asm("	outb (%dx)");
	}

	void outw(int port, unsigned short value) {
		asm("	movl 8(%esp),%edx");
		asm("	movl 12(%esp),%eax");
		asm("	outw (%dx)");
	}

	void outl(int port, unsigned long value) {
		asm("	movl 8(%esp),%edx");
		asm("	movl 12(%esp),%eax");
		asm("	outl (%dx)");
	}
#else

#if defined(SVR4) && !defined(DGUX)
# if !defined(__USLC__)
#  define __USLC__
# endif
#endif

#if defined(DGUX)
# if !defined(_USL)
#  define _USL
# endif
#endif

#ifndef SCO325
# include <sys/inline.h>
#else
# include "scoasm.h"
#endif

#endif /* SUNPRO_C */

#endif /* __GNUC__ */
#endif /* __WATCOMC__ */


#if defined(__alpha__) || defined(__sparc__)
#if defined(linux)
#include <asm/unistd.h>
#if defined(__sparc__)
#if !defined(__NR_pciconfig_read)
#define __NR_pciconfig_read  148
#define __NR_pciconfig_write 149
#endif
#endif
#define BUS(tag) (((tag)>>16)&0xff)
#define DFN(tag) (((tag)>>8)&0xff)
int pciconfig_read(
          unsigned char bus,
          unsigned char dfn,
          unsigned char off,
          unsigned char len,
          void * buf)
{
  return syscall(__NR_pciconfig_read, bus, dfn, off, len, buf);
}
int pciconfig_write(
          unsigned char bus,
          unsigned char dfn,
          unsigned char off,
          unsigned char len,
          void * buf)
{
  return syscall(__NR_pciconfig_write, bus, dfn, off, len, buf);
}
#else
Generate compiler error - scanpci unsupported on non-linux alpha and sparc platforms
#endif /* linux */
#endif /* __alpha__ || __sparc__ */
#if defined(Lynx) && defined(__powerpc__)
/* let's mimick the Linux Alpha stuff for LynxOS so we don't have
 * to change too much code
 */
#include <smem.h>

unsigned char *pciConfBase;

static __inline__ unsigned long
swapl(unsigned long val)
{
	unsigned char *p = (unsigned char *)&val;
	return ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | (p[0] << 0));
}


#define BUS(tag) (((tag)>>16)&0xff)
#define DFN(tag) (((tag)>>8)&0xff)

#define PCIBIOS_DEVICE_NOT_FOUND	0x86
#define PCIBIOS_SUCCESSFUL		0x00

int pciconfig_read(
          unsigned char bus,
          unsigned char dev,
          unsigned char offset,
          int len,		/* unused, alway 4 */
          unsigned long *val)
{
	unsigned long _val;
	unsigned long *ptr;

	dev >>= 3;
	if (bus || dev >= 16) {
		*val = 0xFFFFFFFF;
		return PCIBIOS_DEVICE_NOT_FOUND;
	} else {
		ptr = (unsigned long *)(pciConfBase + ((1<<dev) | offset));
		_val = swapl(*ptr);
	}
	*val = _val;
	return PCIBIOS_SUCCESSFUL;
}

int pciconfig_write(
          unsigned char bus,
          unsigned char dev,
          unsigned char offset,
          int len,		/* unused, alway 4 */
          unsigned long val)
{
	unsigned long _val;
	unsigned long *ptr;

	dev >>= 3;
	_val = swapl(val);
	if (bus || dev >= 16) {
		return PCIBIOS_DEVICE_NOT_FOUND;
	} else {
		ptr = (unsigned long *)(pciConfBase + ((1<<dev) | offset));
		*ptr = _val;
	}
	return PCIBIOS_SUCCESSFUL;
}
#endif

#if !defined(__powerpc__) && !defined(__sparc__)
struct pci_config_reg {
    /* start of official PCI config space header */
    union {
        unsigned long device_vendor;
	struct {
	    unsigned short vendor;
	    unsigned short device;
	} dv;
    } dv_id;
#define _device_vendor dv_id.device_vendor
#define _vendor dv_id.dv.vendor
#define _device dv_id.dv.device
    union {
        unsigned long status_command;
	struct {
	    unsigned short command;
	    unsigned short status;
	} sc;
    } stat_cmd;
#define _status_command stat_cmd.status_command
#define _command stat_cmd.sc.command
#define _status  stat_cmd.sc.status
    union {
        unsigned long class_revision;
	struct {
	    unsigned char rev_id;
	    unsigned char prog_if;
	    unsigned char sub_class;
	    unsigned char base_class;
	} cr;
    } class_rev;
#define _class_revision class_rev.class_revision
#define _rev_id     class_rev.cr.rev_id
#define _prog_if    class_rev.cr.prog_if
#define _sub_class  class_rev.cr.sub_class
#define _base_class class_rev.cr.base_class
    union {
        unsigned long bist_header_latency_cache;
	struct {
	    unsigned char cache_line_size;
	    unsigned char latency_timer;
	    unsigned char header_type;
	    unsigned char bist;
	} bhlc;
    } bhlc;
#define _bist_header_latency_cache bhlc.bist_header_latency_cache
#define _cache_line_size bhlc.bhlc.cache_line_size
#define _latency_timer   bhlc.bhlc.latency_timer
#define _header_type     bhlc.bhlc.header_type
#define _bist            bhlc.bhlc.bist
    union {
	struct {
	    unsigned long dv_base0;
	    unsigned long dv_base1;
	    unsigned long dv_base2;
	    unsigned long dv_base3;
	    unsigned long dv_base4;
	    unsigned long dv_base5;
	} dv;
	struct {
	    unsigned long bg_rsrvd[2];
	    unsigned char primary_bus_number;
	    unsigned char secondary_bus_number;
	    unsigned char subordinate_bus_number;
	    unsigned char secondary_latency_timer;
	    unsigned char io_base;
	    unsigned char io_limit;
	    unsigned short secondary_status;
	    unsigned short mem_base;
	    unsigned short mem_limit;
	    unsigned short prefetch_mem_base;
	    unsigned short prefetch_mem_limit;
	} bg;
    } bc;
#define	_base0				bc.dv.dv_base0
#define	_base1				bc.dv.dv_base1
#define	_base2				bc.dv.dv_base2
#define	_base3				bc.dv.dv_base3
#define	_base4				bc.dv.dv_base4
#define	_base5				bc.dv.dv_base5
#define	_primary_bus_number		bc.bg.primary_bus_number
#define	_secondary_bus_number		bc.bg.secondary_bus_number
#define	_subordinate_bus_number		bc.bg.subordinate_bus_number
#define	_secondary_latency_timer	bc.bg.secondary_latency_timer
#define _io_base			bc.bg.io_base
#define _io_limit			bc.bg.io_limit
#define _secondary_status		bc.bg.secondary_status
#define _mem_base			bc.bg.mem_base
#define _mem_limit			bc.bg.mem_limit
#define _prefetch_mem_base		bc.bg.prefetch_mem_base
#define _prefetch_mem_limit		bc.bg.prefetch_mem_limit
    unsigned long rsvd1;
    union {     /* Offset 0x2c - 0x2f */
        unsigned long subsys_card_vendor;
        unsigned long rsvd2;
        struct {
            unsigned short subsys_vendor;
            unsigned short subsys_card;
        } ssys;
    } ssys_id;
#define _subsys_card_vendor		ssys_id.subsys_card_vendor
#define _subsys_vendor			ssys_id.ssys.subsys_vendor
#define _subsys_card			ssys_id.ssys.subsys_card
    unsigned long _baserom;
    unsigned long rsvd3;
    unsigned long rsvd4;
    union {
	union {
	    unsigned long max_min_ipin_iline;
	    struct {
		unsigned char int_line;
		unsigned char int_pin;
		unsigned char min_gnt;
		unsigned char max_lat;
	    } mmii;
	} mmii;
	struct {
	    unsigned char res1;
	    unsigned char res2;
	    unsigned char bridge_control;
	    unsigned char res3;
	} bctrl;
    } bm;
#define _max_min_ipin_iline bm.mmii.max_min_ipin_iline
#define _int_line bm.mmii.mmii.int_line
#define _int_pin  bm.mmii.mmii.int_pin
#define _min_gnt  bm.mmii.mmii.min_gnt
#define _max_lat  bm.mmii.mmii.max_lat
#define _b_ctrl   bm.bctrl.bridge_control
    /* I don't know how accurate or standard this is (DHD) */
    union {
	unsigned long user_config;
	struct {
	    unsigned char user_config_0;
	    unsigned char user_config_1;
	    unsigned char user_config_2;
	    unsigned char user_config_3;
	} uc;
    } uc;
#define _user_config uc.user_config
#define _user_config_0 uc.uc.user_config_0
#define _user_config_1 uc.uc.user_config_1
#define _user_config_2 uc.uc.user_config_2
#define _user_config_3 uc.uc.user_config_3
    /* end of official PCI config space header */
    unsigned long _pcibusidx;
    unsigned long _pcinumbus;
    unsigned long _pcibuses[16];
    unsigned short _configtype;   /* config type found                   */
    unsigned short _ioaddr;       /* config type 1 - private I/O addr    */
    unsigned long _cardnum;       /* config type 2 - private card number */
};
#else
/* ppc and sparc are big endian, swapping bytes is not quite enough
 * to interpret the PCI config registers...
 */
struct pci_config_reg {
    /* start of official PCI config space header */
    union {
        unsigned long device_vendor;
	struct {
	    unsigned short device;
	    unsigned short vendor;
	} dv;
    } dv_id;
#define _device_vendor dv_id.device_vendor
#define _vendor dv_id.dv.vendor
#define _device dv_id.dv.device
    union {
        unsigned long status_command;
	struct {
	    unsigned short status;
	    unsigned short command;
	} sc;
    } stat_cmd;
#define _status_command stat_cmd.status_command
#define _command stat_cmd.sc.command
#define _status  stat_cmd.sc.status
    union {
        unsigned long class_revision;
	struct {
	    unsigned char base_class;
	    unsigned char sub_class;
	    unsigned char prog_if;
	    unsigned char rev_id;
	} cr;
    } class_rev;
#define _class_revision class_rev.class_revision
#define _rev_id     class_rev.cr.rev_id
#define _prog_if    class_rev.cr.prog_if
#define _sub_class  class_rev.cr.sub_class
#define _base_class class_rev.cr.base_class
    union {
        unsigned long bist_header_latency_cache;
	struct {
	    unsigned char bist;
	    unsigned char header_type;
	    unsigned char latency_timer;
	    unsigned char cache_line_size;
	} bhlc;
    } bhlc;
#define _bist_header_latency_cache bhlc.bist_header_latency_cache
#define _cache_line_size bhlc.bhlc.cache_line_size
#define _latency_timer   bhlc.bhlc.latency_timer
#define _header_type     bhlc.bhlc.header_type
#define _bist            bhlc.bhlc.bist
    union {
	struct {
	    unsigned long dv_base0;
	    unsigned long dv_base1;
	    unsigned long dv_base2;
	    unsigned long dv_base3;
	    unsigned long dv_base4;
	    unsigned long dv_base5;
	} dv;
/* ?? */
	struct {
	    unsigned long bg_rsrvd[2];

	    unsigned char secondary_latency_timer;
	    unsigned char subordinate_bus_number;
	    unsigned char secondary_bus_number;
	    unsigned char primary_bus_number;

	    unsigned short secondary_status;
	    unsigned char io_limit;
	    unsigned char io_base;

	    unsigned short mem_limit;
	    unsigned short mem_base;

	    unsigned short prefetch_mem_limit;
	    unsigned short prefetch_mem_base;
	} bg;
    } bc;
#define	_base0				bc.dv.dv_base0
#define	_base1				bc.dv.dv_base1
#define	_base2				bc.dv.dv_base2
#define	_base3				bc.dv.dv_base3
#define	_base4				bc.dv.dv_base4
#define	_base5				bc.dv.dv_base5
#define	_primary_bus_number		bc.bg.primary_bus_number
#define	_secondary_bus_number		bc.bg.secondary_bus_number
#define	_subordinate_bus_number		bc.bg.subordinate_bus_number
#define	_secondary_latency_timer	bc.bg.secondary_latency_timer
#define _io_base			bc.bg.io_base
#define _io_limit			bc.bg.io_limit
#define _secondary_status		bc.bg.secondary_status
#define _mem_base			bc.bg.mem_base
#define _mem_limit			bc.bg.mem_limit
#define _prefetch_mem_base		bc.bg.prefetch_mem_base
#define _prefetch_mem_limit		bc.bg.prefetch_mem_limit
    unsigned long rsvd1;
    union {     /* Offset 0x2c - 0x2f */
        unsigned long subsys_card_vendor;
        unsigned long rsvd2;
        struct {
            unsigned short subsys_card;
            unsigned short subsys_vendor;
        } ssys;
    } ssys_id;
#define _subsys_card_vendor		ssys_id.subsys_card_vendor
#define _subsys_vendor			ssys_id.ssys.subsys_vendor
#define _subsys_card			ssys_id.ssys.subsys_card
    unsigned long _baserom;
    unsigned long rsvd3;
    unsigned long rsvd4;
    union {
	union {
	    unsigned long max_min_ipin_iline;
	    struct {
		unsigned char max_lat;
		unsigned char min_gnt;
		unsigned char int_pin;
		unsigned char int_line;
	    } mmii;
	} mmii;
	struct {
	    unsigned char res1;
	    unsigned char bridge_control;
	    unsigned char res2;
	    unsigned char res3;
	} bctrl;
    } bm;
#define _max_min_ipin_iline bm.mmii.max_min_ipin_iline
#define _int_line bm.mmii.mmii.int_line
#define _int_pin  bm.mmii.mmii.int_pin
#define _min_gnt  bm.mmii.mmii.min_gnt
#define _max_lat  bm.mmii.mmii.max_lat
#define _b_ctrl   bm.bctrl.bridge_control
    /* I don't know how accurate or standard this is (DHD) */
    union {
	unsigned long user_config;
	struct {
	    unsigned char user_config_3;
	    unsigned char user_config_2;
	    unsigned char user_config_1;
	    unsigned char user_config_0;
	} uc;
    } uc;
#define _user_config uc.user_config
#define _user_config_0 uc.uc.user_config_0
#define _user_config_1 uc.uc.user_config_1
#define _user_config_2 uc.uc.user_config_2
#define _user_config_3 uc.uc.user_config_3
    /* end of official PCI config space header */
    unsigned long _pcibusidx;
    unsigned long _pcinumbus;
    unsigned long _pcibuses[16];
    unsigned short _ioaddr;       /* config type 1 - private I/O addr    */
    unsigned short _configtype;   /* config type found                   */
    unsigned long _cardnum;       /* config type 2 - private card number */
};
#endif

extern void identify_card(struct pci_config_reg *, int);
extern void print_default_class(struct pci_config_reg *pcr);
extern void print_bridge_pci_class(struct pci_config_reg *pcr);
extern void print_bridge_class(struct pci_config_reg *pcr);
extern void print_i128(struct pci_config_reg *);
extern void print_mach64(struct pci_config_reg *);
extern void print_pcibridge(struct pci_config_reg *);
extern void enable_os_io();
extern void disable_os_io();

#define MAX_DEV_PER_VENDOR_CFG1 64
#define MAX_DEV_PER_VENDOR_CFG2 16
#define MAX_PCI_DEVICES         64
#define NF ((void (*)())NULL)
#define PCI_MULTIFUNC_DEV	0x80
#if defined(__alpha__) || defined(__powerpc__) || defined(__sparc__)
#define PCI_ID_REG              0x00
#define PCI_CMD_STAT_REG        0x04
#define PCI_CLASS_REG           0x08
#define PCI_HEADER_MISC         0x0C
#define PCI_MAP_REG_START       0x10
#define PCI_MAP_ROM_REG         0x30
#define PCI_INTERRUPT_REG       0x3C
#define PCI_REG_USERCONFIG      0x40
#endif

struct pci_vendor_device {
    unsigned short vendor_id;
    char *vendorname;
    struct pci_device {
        unsigned short device_id;
        char *devicename;
	void (*print_func)(struct pci_config_reg *);
    } device[MAX_DEV_PER_VENDOR_CFG1];
} pvd[] = {
        { 0x0e11, "Compaq", {
                            { 0x3033, "QVision 1280/p", NF },
                            { 0xae10, "Smart-2/P RAID Controller", NF },
                            { 0xae32, "Netellignet 10/100", NF },
                            { 0xae34, "Netellignet 10", NF },
                            { 0xae35, "NetFlex 3", NF },
                            { 0xae40, "Netellignet 10/100 Dual", NF },
                            { 0xae43, "Netellignet 10/100 ProLiant", NF },
                            { 0xb011, "Netellignet 10/100 Integrated", NF },
                            { 0xf130, "ThunderLAN", NF },
                            { 0xf150, "NetFlex 3 BNC", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1000, "NCR", {
                            { 0x0001, "53C810", NF },
                            { 0x0002, "53C820", NF },
                            { 0x0003, "53C825", NF },
                            { 0x0004, "53C815", NF },
                            { 0x0005, "53C810AP", NF },
                            { 0x0006, "53C860", NF },
                            { 0x000B, "53C896", NF },
                            { 0x000C, "53C895", NF },
                            { 0x000D, "53C885", NF },
                            { 0x000F, "53C875", NF },
                            { 0x008F, "53C875J", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1002, "ATI", {
                            { 0x4158, "Mach32", NF },
                            { 0x4354, "Mach64 CT", print_mach64 },
                            { 0x4358, "Mach64 CX", print_mach64 },
                            { 0x4554, "Mach64 ET", print_mach64 },
                            { 0x4742, "Mach64 GB", print_mach64 },
                            { 0x4744, "Mach64 GD", print_mach64 },
                            { 0x4749, "Mach64 GI", print_mach64 },
                            { 0x474C, "Mach64 GL", print_mach64 },
                            { 0x474D, "Mach64 GM", print_mach64 },
                            { 0x474E, "Mach64 GN", print_mach64 },
                            { 0x474F, "Mach64 GO", print_mach64 },
                            { 0x4750, "Mach64 GP", print_mach64 },
                            { 0x4751, "Mach64 GQ", print_mach64 },
                            { 0x4752, "Mach64 GR", print_mach64 },
                            { 0x4753, "Mach64 GS", print_mach64 },
                            { 0x4754, "Mach64 GT", print_mach64 },
                            { 0x4755, "Mach64 GU", print_mach64 },
                            { 0x4756, "Mach64 GV", print_mach64 },
                            { 0x4757, "Mach64 GW", print_mach64 },
                            { 0x4758, "Mach64 GX", print_mach64 },
                            { 0x475A, "Mach64 GZ", print_mach64 },
                            { 0x4C42, "Mach64 LB", print_mach64 },
                            { 0x4C44, "Mach64 LD", print_mach64 },
                            { 0x4C47, "Mach64 LG", print_mach64 },
                            { 0x4C49, "Mach64 LI", print_mach64 },
                            { 0x4C4D, "Mach64 LM", print_mach64 },
                            { 0x4C4E, "Mach64 LN", print_mach64 },
                            { 0x4C50, "Mach64 LP", print_mach64 },
                            { 0x4C52, "Mach64 LR", print_mach64 },
                            { 0x4C53, "Mach64 LS", print_mach64 },
                            { 0x5245, "Rage 128 RE", NF },
                            { 0x5246, "Rage 128 RF", NF },
                            { 0x524B, "Rage 128 RK", NF },
                            { 0x524C, "Rage 128 RL", NF },
                            { 0x5654, "Mach64 VT", print_mach64 },
                            { 0x5655, "Mach64 VU", print_mach64 },
                            { 0x5656, "Mach64 VV", print_mach64 },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1004, "VLSI", {
                            { 0x0005, "82C592-FC1", NF },
                            { 0x0006, "82C593-FC1", NF },
                            { 0x0007, "82C594-AFC2", NF },
                            { 0x0009, "82C597-AFC2", NF },
                            { 0x000C, "82C541 Lynx", NF },
                            { 0x000D, "82C543 Lynx ISA", NF },
                            { 0x0702, "VAS96011", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1005, "Avance Logic", {
                            { 0x2301, "ALG2301", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x100B, "NS", {
                            { 0x0002, "87415", NF },
                            { 0xD001, "87410", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x100C, "Tseng Labs", {
                            { 0x3202, "ET4000w32p rev A", NF },
                            { 0x3205, "ET4000w32p rev B", NF },
                            { 0x3206, "ET4000w32p rev D", NF },
                            { 0x3207, "ET4000w32p rev C", NF },
                            { 0x3208, "ET6000/6100", NF },
                            { 0x4702, "ET6300", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x100E, "Weitek", {
                            { 0x9001, "P9000", NF },
                            { 0x9100, "P9100", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1011, "Digital Equipment Corporation", {
                            { 0x0001, "DC21050 PCI-PCI Bridge",print_pcibridge},
                            { 0x0002, "DC21040 10Mb/s Ethernet", NF },
                            { 0x0004, "TGA", NF },
                            { 0x0009, "DC21140 10/100 Mb/s Ethernet", NF },
                            { 0x000D, "TGA2", NF },
                            { 0x000F, "DEFPA (FDDI PCI)", NF },
                            { 0x0014, "DC21041 10Mb/s Ethernet Plus", NF },
                            { 0x0019, "DC21142 10/100 Mb/s Ethernet", NF },
                            { 0x0021, "DC21052", NF },
                            { 0x0024, "DC21152", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1013, "Cirrus Logic", {
                            { 0x0038, "GD 7548", NF },
                            { 0x00A0, "GD 5430", NF },
                            { 0x00A4, "GD 5434-4", NF },
                            { 0x00A8, "GD 5434-8", NF },
                            { 0x00AC, "GD 5436", NF },
                            { 0x00B8, "GD 5446", NF },
                            { 0x00BC, "GD 5480", NF },
                            { 0x00D0, "GD 5462", NF },
                            { 0x00D4, "GD 5464", NF },
                            { 0x00D6, "GD 5465", NF },
                            { 0x1100, "CL 6729", NF },
                            { 0x1110, "CL 6832", NF },
                            { 0x1200, "GD 7542", NF },
                            { 0x1202, "GD 7543", NF },
                            { 0x1204, "GD 7541", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1014, "IBM", {
                            { 0x000A, "Fire Coral", NF },
                            { 0x0018, "Token Ring", NF },
                            { 0x001D, "82G2675", NF },
                            { 0x0022, "82351 pci-pci bridge", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x101A, "NCR", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x101C, "WD*", {
                            { 0x3296, "WD 7197", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1022, "AMD", {
                            { 0x2000, "79C970 Lance", NF },
                            { 0x2020, "53C974 SCSI", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1023, "Trident", {
                            { 0x9320, "TGUI 9320", NF },
                            { 0x9420, "TGUI 9420", NF },
                            { 0x9440, "TGUI 9440", NF },
                            { 0x9660, "TGUI 9660/9680/9682", NF },
#if 0
                            { 0x9680, "TGUI 9680", NF },
                            { 0x9682, "TGUI 9682", NF },
#endif
                            { 0x9388, "TGUI 9388", NF },
                            { 0x9397, "TGUI 9397", NF },
                            { 0x9750, "TGUI 9750", NF },
                            { 0x9850, "TGUI 9850", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1025, "ALI", {
                            { 0x1435, "M1435", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x102B, "Matrox", {
                            { 0x0518, "MGA-2 Atlas PX2085", NF },
                            { 0x0519, "MGA Millennium", NF },
                            { 0x051a, "MGA Mystique", NF },
                            { 0x051b, "MGA Millennium II", NF },
                            { 0x051f, "MGA Millennium II AGP", NF },
                            { 0x0520, "MGA Millennium G200 PCI", NF },
                            { 0x0521, "MGA Millennium G200 AGP", NF },
                            { 0x0D10, "MGA Impression", NF },
                            { 0x1000, "MGA Productiva G100 PCI", NF },
                            { 0x1001, "MGA Productiva G100 AGP", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x102C, "CT", {
                            { 0x00D8, "65545", NF },
                            { 0x00DC, "65548", NF },
                            { 0x00E0, "65550", NF },
                            { 0x00E4, "65554", NF },
                            { 0x00E5, "65555", NF },
                            { 0x00F4, "68554", NF },
                            { 0x00C0, "69000", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1031, "Miro", {
                            { 0x5601, "ZR36050", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1033, "NEC", {
                            { 0x0046, "PowerVR PCX2", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1036, "FD", {
                            { 0x0000, "TMC-18C30 (36C70)", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1039, "SIS", {
                            { 0x0001, "86C201", NF },
                            { 0x0002, "86C202", NF },
                            { 0x0008, "85C503", NF },
                            { 0x0205, "86C205", NF },
                            { 0x0406, "85C501", NF },
                            { 0x0496, "85C496", NF },
                            { 0x0601, "85C601", NF },
                            { 0x5107, "5107", NF },
                            { 0x5511, "85C5511", NF },
                            { 0x5513, "85C5513", NF },
                            { 0x5571, "5571", NF },
                            { 0x5597, "5597", NF },
                            { 0x6306, "530", NF },
                            { 0x6326, "6326", NF },
                            { 0x7001, "7001", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x103C, "HP", {
                            { 0x1030, "J2585A", NF },
                            { 0x1031, "J2585B", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1042, "SMC/PCTECH", {
                            { 0x1000, "FDC 37C665/RZ1000", NF },
                            { 0x1001, "FDC /RZ1001", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1044, "DPT", {
                            { 0xA400, "SmartCache/Raid", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1045, "Opti", {
                            { 0xC178, "92C178", NF },
                            { 0xC557, "82C557 Viper-M", NF },
                            { 0xC558, "82C558 Viper-M ISA+IDE", NF },
                            { 0xC621, "82C621", NF },
                            { 0xC700, "82C700", NF },
                            { 0xC701, "82C701 FireStar Plus", NF },
                            { 0xC814, "82C814 Firebridge 1", NF },
                            { 0xC822, "82C822", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x104A, "SGS Thomson", {
                            { 0x0008, "STG2000", NF },
                            { 0x0009, "STG1764", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x104B, "BusLogic", {
                            { 0x0140, "946C 01", NF },
                            { 0x1040, "946C 10", NF },
                            { 0x8130, "FlashPoint", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x104C, "Texas Instruments", {
                            { 0x3d04, "3DLabs Permedia", NF },
                            { 0x3d07, "3DLabs Permedia 2", NF },
                            { 0xAC12, "PCI1130", NF },
                            { 0xAC15, "PCI1131", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x104E, "Oak", {
                            { 0x0107, "OTI107", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1050, "Windbond", {
                            { 0x0940, "89C940 NE2000-PCI", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1057, "Motorola", {
                            { 0x0001, "MPC105 Eagle", NF },
                            { 0x0002, "MPC105 Grackle", NF },
                            { 0x4801, "Raven", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x105A, "Promise", {
                            { 0x4D33, "IDE UltraDMA/33", NF },
                            { 0x5300, "DC5030", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x105D, "Number Nine", {
                            { 0x2309, "Imagine-128", print_i128 },
                            { 0x2339, "Imagine-128-II", print_i128 },
                            { 0x493D, "Imagine-128-T2R", print_i128 },
                            { 0x5348, "Imagine-128-T2R4", print_i128 },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1060, "UMC", {
                            { 0x0101, "UM8673F", NF },
                            { 0x673A, "UM8886BF", NF },
                            { 0x886A, "UM8886A", NF },
                            { 0x8881, "UM8881F", NF },
                            { 0x8886, "UM8886F", NF },
                            { 0x8891, "UM8891A", NF },
                            { 0x9017, "UM9017F", NF },
                            { 0xE886, "UM8886N", NF },
                            { 0xE891, "UM8891N", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1061, "X", {
                            { 0x0001, "ITT AGX016", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1066, "PICOP", {
                            { 0x0001, "PT86C52x Vesuvius", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1069, "Mylex", {
                            { 0x0010, "AccelRAID 250", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x106B, "Apple", {
                            { 0x0001, "Bandit", NF },
                            { 0x0002, "Grand Central", NF },
                            { 0x000E, "Hydra", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1074, "Nexgen", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1077, "QLogic", {
                            { 0x1020, "ISP1020", NF },
                            { 0x1022, "ISP1022", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1078, "Cyrix", {
                            { 0x0000, "5510", NF },
                            { 0x0001, "PCI Master", NF },
                            { 0x0002, "5520", NF },
                            { 0x0100, "5530 Kahlua Legacy", NF },
                            { 0x0101, "5530 Kahlua SMI", NF },
                            { 0x0102, "5530 Kahlua IDE", NF },
                            { 0x0103, "5530 Kahlua Audio", NF },
                            { 0x0104, "5530 Kahlua Video", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x107D, "Leadtek", {
                            { 0x0000, "S3 805", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1080, "Contaq", {
                            { 0x0600, "82C599", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1083, "FOREX", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x108D, "Olicom", {
                            { 0x0001, "OC-3136", NF },
                            { 0x0011, "OC-2315", NF },
                            { 0x0012, "OC-2325", NF },
                            { 0x0013, "OC-2183", NF },
                            { 0x0014, "OC-2326", NF },
                            { 0x0021, "OC-6151", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x108E, "Sun", {
                            { 0x1000, "EBUS", NF },
                            { 0x1001, "Happy Meal", NF },
                            { 0x5000, "Simba Bus Module", NF },
                            { 0x8000, "PCI Bus Module", NF },
                            { 0xa000, "Sabre Bus Module", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1095, "CMD", {
                            { 0x0640, "640A", NF },
                            { 0x0643, "643", NF },
                            { 0x0646, "646", NF },
                            { 0x0670, "670", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1098, "Vision", {
                            { 0x0001, "QD 8500", NF },
                            { 0x0002, "QD 8580", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x109E, "Brooktree", {
                            { 0x0350, "Bt848", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10A8, "Sierra", {
                            { 0x0000, "STB Horizon 64", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10AA, "ACC", {
                            { 0x0000, "2056", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10AD, "Winbond", {
                            { 0x0001, "W83769F", NF },
                            { 0x0105, "SL82C105", NF },
                            { 0x0565, "W83C553", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10B3, "Databook", {
                            { 0xB106, "DB87144", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10B7, "3COM", {
                            { 0x5900, "3C590 10bT", NF },
                            { 0x5950, "3C595 100bTX", NF },
                            { 0x5951, "3C595 100bT4", NF },
                            { 0x5952, "3C595 10b-MII", NF },
                            { 0x9000, "3C900 10bTPO", NF },
                            { 0x9001, "3C900 10b Combo", NF },
                            { 0x9050, "3C905 100bTX", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10B8, "SMC", {
                            { 0x0005, "9432 TX", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10B9, "ALI", {
                            { 0x1445, "M1445", NF },
                            { 0x1449, "M1449", NF },
                            { 0x1451, "M1451", NF },
                            { 0x1461, "M1461", NF },
                            { 0x1489, "M1489", NF },
                            { 0x1511, "M1511", NF },
                            { 0x1513, "M1513", NF },
                            { 0x1521, "M1521", NF },
                            { 0x1523, "M1523", NF },
                            { 0x1531, "M1531 Aladdin IV", NF },
                            { 0x1533, "M1533 Aladdin IV", NF },
                            { 0x5215, "M4803", NF },
                            { 0x5219, "M5219", NF },
                            { 0x5229, "M5229 TXpro", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10BA, "Mitsubishi", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10BD, "Surecom", {
                            { 0x0E34, "NE-34PCI Lan", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10C8, "Neomagic", {
                            { 0x0001, "Magicgraph NM2070", NF },
                            { 0x0002, "Magicgraph 128V", NF },
                            { 0x0003, "Magicgraph 128ZV", NF },
                            { 0x0004, "Magicgraph NM2160", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10CD, "Advanced System Products", {
                            { 0x1200, "ABP940", NF },
                            { 0x1300, "ABP940U", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10DC, "CERN", {
                            { 0x0001, "STAR/RD24 SCI-PCI (PMC)", NF },
                            { 0x0002, "STAR/RD24 SCI-PCI (PMC)", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10DE, "NVidia", {
                            { 0x0008, "NV1", NF },
                            { 0x0009, "DAC64", NF },
                            { 0x0020, "Riva TNT", NF },
                            { 0x0028, "Riva TNT2", NF },
                            { 0x0029, "Riva TNT2 Ultra", NF },
                            { 0x002C, "Riva TNT2 Vanta", NF },
                            { 0x002D, "Riva TNT2 Ultra Vanta", NF },
                            { 0x00A0, "Riva Integrated", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10E0, "IMS", {
                            { 0x8849, "8849", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10E1, "Tekram", {
                            { 0x690C, "DC690C", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10E3, "Tundra", {
                            { 0x0000, "CA91C042 Universe", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10E8, "AMCC", {
                            { 0x8043, "Myrinet PCI (M2-PCI-32)", NF },
                            { 0x807D, "S5933 PCI44", NF },
                            { 0x809C, "S5933 Traquair HEPC3", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10EA, "Intergraphics", {
                            { 0x1680, "IGA-1680", NF },
                            { 0x1682, "IGA-1682", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10EC, "Realtek", {
                            { 0x8029, "8029", NF },
                            { 0x8129, "8129", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x10FA, "Truevision", {
                            { 0x000C, "Targa 1000", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1101, "Initio Corp", {
                            { 0x9100, "320 P", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1106, "VIA", {
                            { 0x0505, "VT 82C505", NF },
                            { 0x0561, "VT 82C505", NF },
                            { 0x0576, "VT 82C576 3V", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1119, "Vortex", {
                            { 0x0001, "GDT 6000b", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x111A, "EF", {
                            { 0x0000, "155P-MF1", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1127, "Fore Systems", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x112F, "Imaging Technology", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x113C, "PLX", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1142, "Alliance", {
                            { 0x3210, "ProMotion 6410", NF },
                            { 0x6422, "ProMotion 6422", NF },
                            { 0x6424, "ProMotion AT24", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x114A, "VMIC", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x114F, "DIGI*", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1159, "Mutech", {
                            { 0x0001, "MV1000", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1163, "Rendition", {
                            { 0x0001, "V1000", NF },
                            { 0x2000, "V2100/V2200", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1179, "Toshiba", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1193, "Zeinet", {
                            { 0x0001, "1221", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x11CB, "Specialix", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x11FE, "Control", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x120E, "Cyclades", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x121A, "3Dfx Interactive", {
                            { 0x0001, "Voodoo Graphics", NF },
                            { 0x0003, "Banshee", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1236, "Sigma Designs", {
                            { 0x6401, "REALmagic64/GX (SD 6425)", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1281, "YOKOGAWA", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1292, "TriTech Microelectronics", {
                            { 0xfc02, "Pyramid3D TR25202", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x12D2, "NVidia/SGS-Thomson", {
                            { 0x0018, "Riva128", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1C1C, "Symphony", {
                            { 0x0001, "82C101", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1DE1, "Tekram", {
                            { 0xDC29, "DC290", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x3D3D, "3Dlabs", {
                            { 0x0001, "GLINT 300SX", NF },
                            { 0x0002, "GLINT 500TX", NF },
                            { 0x0003, "GLINT Delta", NF },
                            { 0x0004, "GLINT Permedia", NF },
                            { 0x0006, "GLINT MX", NF },
                            { 0x0007, "GLINT Permedia 2", NF },
                            { 0x0008, "GLINT Gamma", NF },
                            { 0x0009, "GLINT Permedia 2v", NF },
                            { 0x0000, (char *)NULL, NF } } } ,
        { 0x4005, "Avance", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x5333, "S3", {
                            { 0x0551, "Plato/PX", NF },
                            { 0x5631, "ViRGE", NF },
                            { 0x8811, "Trio32/64", NF },
                            { 0x8812, "Aurora64V+", NF },
                            { 0x8814, "Trio64UV+", NF },
                            { 0x883D, "ViRGE/VX", NF },
                            { 0x8880, "868", NF },
                            { 0x88B0, "928", NF },
                            { 0x88C0, "864-0", NF },
                            { 0x88C1, "864-1", NF },
                            { 0x88D0, "964-0", NF },
                            { 0x88D1, "964-1", NF },
                            { 0x88F0, "968", NF },
                            { 0x8901, "Trio64V2/DX or /GX", NF },
                            { 0x8902, "PLATO/PX", NF },
                            { 0x8904, "Trio3D", NF },
                            { 0x8A01, "ViRGE/DX or /GX", NF },
                            { 0x8A10, "ViRGE/GX2", NF },
                            { 0x8A13, "Trio3D/2X", NF },
                            { 0x8A20, "Savage3D", NF },
                            { 0x8A21, "Savage3D S3 (Macrovision Support)", NF },
                            { 0x8A22, "Savage4", NF },
                            { 0x8C01, "ViRGE/MX", NF },
                            { 0x8C02, "ViRGE/MX+", NF },
                            { 0x8C03, "ViRGE/MX+MV", NF },
                            { 0x9102, "Savage2000", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x8086, "Intel", {
                            { 0x0482, "82375EB pci-eisa bridge", NF },
                            { 0x0483, "82424ZX cache dram controller", NF },
                            { 0x0484, "82378IB/ZB pci-isa bridge", NF },
                            { 0x0486, "82430ZX Aries", NF },
                            { 0x04A3, "82434LX/NX pci cache mem controller", NF },
                            { 0x0960, "960RD processor/bridge", NF },
                            { 0x1230, "82371 bus-master IDE controller", NF },
                            { 0x1223, "SAA7116", NF },
                            { 0x1229, "82557/8 10/100MBit network controller",NF},
                            { 0x122D, "82437 Triton", NF },
                            { 0x122E, "82471 Triton", NF },
                            { 0x1230, "82438", NF },
                            { 0x1250, "82439", NF },
                            { 0x7000, "82371 pci-isa bridge", NF },
                            { 0x7010, "82371 bus-master IDE controller", NF },
                            { 0x7100, "82439 TX", NF },
                            { 0x7110, "82371AB PIIX4 ISA", NF },
                            { 0x7111, "82371AB PIIX4 IDE", NF },
                            { 0x7112, "82371AB PIIX4 USB", NF },
                            { 0x7113, "82371AB PIIX4 ACPI", NF },
			    { 0x7180, "82443LX PAC Host", NF },
			    { 0x7181, "82443LX PAC AGP", NF },
			    { 0x7190, "82443BX Host", NF },
			    { 0x7191, "82443BX AGP", NF },
			    { 0x7192, "82443BX Host (no AGP)", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x9004, "Adaptec", {
                            { 0x0010, "2940U2", NF },
                            { 0x1078, "7810", NF },
                            { 0x5078, "7850", NF },
                            { 0x5578, "7855", NF },
                            { 0x6078, "7860", NF },
                            { 0x6178, "2940AU", NF },
                            { 0x7078, "7870", NF },
                            { 0x7178, "2940", NF },
                            { 0x7278, "7872", NF },
                            { 0x7378, "398X", NF },
                            { 0x7478, "2944", NF },
                            { 0x7895, "7895", NF },
                            { 0x8078, "7880", NF },
                            { 0x8178, "2940U/UW", NF },
                            { 0x8278, "3940U/UW", NF },
                            { 0x8378, "389XU", NF },
                            { 0x8478, "2944U", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x9005, "Adaptec", {
                            { 0x001F, "7890/7891", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x907F, "Atronics", {
                            { 0x2015, "IDE-2015PL", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0xEDD8, "ARK Logic", {
                            { 0xA091, "1000PV", NF },
                            { 0xA099, "2000PV", NF },
                            { 0xA0A1, "2000MT", NF },
                            { 0xA0A9, "2000MI", NF },
                            { 0x0000, (char *)NULL, NF } } },
	{ 0x1274, "Ensoniq", {
			    { 0x5000, "AudioPCI", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x0000, (char *)NULL, {
                            { 0x0000, (char *)NULL, NF } } }
};

#if defined(__alpha__) || defined(__sparc__)
#define PCI_EN 0x00000000
#else
#define PCI_EN 0x80000000
#endif

#define	PCI_MODE1_ADDRESS_REG		0xCF8
#define	PCI_MODE1_DATA_REG		0xCFC

#define	PCI_MODE2_ENABLE_REG		0xCF8
#ifdef PC98
#define	PCI_MODE2_FORWARD_REG		0xCF9
#else
#define	PCI_MODE2_FORWARD_REG		0xCFA
#endif


main(int argc, char *argv[])
{
    unsigned long tmplong1, tmplong2, config_cmd;
    unsigned char tmp1, tmp2;
    unsigned int idx;
    struct pci_config_reg pcr;
    int ch, verbose = 0, do_mode1_scan = 0, do_mode2_scan = 0;
    int func, hostbridges=0;

    while((ch = getopt(argc, argv, "v12")) != EOF) {
     	switch((char)ch) {
	case '1':
		do_mode1_scan = 1;
		break;
	case '2':
		do_mode2_scan = 1;
		break;
	case 'v':
		verbose = 1;
		break;
	default :
		printf("Usage: %s [-v12] \n", argv[0]);
		exit(1);
        }
    }

#if !defined(MSDOS)
    if (geteuid()) {
	printf("This program must have root privileges\n");
	exit(1);
    }
#endif

#if defined(DGUX)
    printf("Scanpci for Intel ix86 DG/ux R4.20MU04...MUxx\n\n");
#endif

    enable_os_io();

#if !defined(__alpha__) && !defined(__powerpc__) && !defined(__sparc__)
    pcr._configtype = 0;

    outb(PCI_MODE2_ENABLE_REG, 0x00);
    outb(PCI_MODE2_FORWARD_REG, 0x00);
    tmp1 = inb(PCI_MODE2_ENABLE_REG);
    tmp2 = inb(PCI_MODE2_FORWARD_REG);
    if ((tmp1 == 0x00) && (tmp2 == 0x00)) {
	pcr._configtype = 2;
        printf("PCI says configuration type 2\n");
    } else {
        tmplong1 = inl(PCI_MODE1_ADDRESS_REG);
        outl(PCI_MODE1_ADDRESS_REG, PCI_EN);
        tmplong2 = inl(PCI_MODE1_ADDRESS_REG);
        outl(PCI_MODE1_ADDRESS_REG, tmplong1);
        if (tmplong2 == PCI_EN) {
	    pcr._configtype = 1;
            printf("PCI says configuration type 1\n");
	} else {
            printf("No PCI !\n");
	    disable_os_io();
	    exit(1);
	}
    }
#else
    pcr._configtype = 1;
#endif

    /* Try pci config 1 probe first */

    if ((pcr._configtype == 1) || do_mode1_scan) {
    printf("\nPCI probing configuration type 1\n");

    pcr._ioaddr = 0xFFFF;

    pcr._pcibuses[0] = 0;
    pcr._pcinumbus = 1;
    pcr._pcibusidx = 0;
    idx = 0;

    do {
        printf("\nProbing for devices on PCI bus %d:\n", pcr._pcibusidx);

        for (pcr._cardnum = 0x0; pcr._cardnum < 0x20; pcr._cardnum += 0x1) {
	  func = 0;
	  do { /* loop over the different functions, if present */
#if !defined(__alpha__) && !defined(__powerpc__) && !defined(__sparc__)
	    config_cmd = PCI_EN | (pcr._pcibuses[pcr._pcibusidx]<<16) |
                                  (pcr._cardnum<<11) | (func<<8);

            outl(PCI_MODE1_ADDRESS_REG, config_cmd);         /* ioreg 0 */
            pcr._device_vendor = inl(PCI_MODE1_DATA_REG);
#else
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
		PCI_ID_REG, 4, &pcr._device_vendor);
#endif

            if ((pcr._vendor == 0xFFFF) || (pcr._device == 0xFFFF))
                break;   /* nothing there */

	    printf("\npci bus 0x%x cardnum 0x%02x function 0x%04x: vendor 0x%04x device 0x%04x\n",
	        pcr._pcibuses[pcr._pcibusidx], pcr._cardnum, func,
		pcr._vendor, pcr._device);

#if !defined(__alpha__) && !defined(__powerpc__) && !defined(__sparc__)
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x04);
	    pcr._status_command  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x08);
	    pcr._class_revision  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x0C);
	    pcr._bist_header_latency_cache = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x10);
	    pcr._base0  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x14);
	    pcr._base1  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x18);
	    pcr._base2  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x1C);
	    pcr._base3  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x20);
	    pcr._base4  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x24);
	    pcr._base5  = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x2c);
	    pcr._subsys_card_vendor = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x30);
	    pcr._baserom = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x3C);
	    pcr._max_min_ipin_iline = inl(PCI_MODE1_DATA_REG);
            outl(PCI_MODE1_ADDRESS_REG, config_cmd | 0x40);
	    pcr._user_config = inl(PCI_MODE1_DATA_REG);
#else
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_CMD_STAT_REG, 4, &pcr._status_command);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_CLASS_REG, 4, &pcr._class_revision);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_HEADER_MISC, 4, &pcr._bist_header_latency_cache);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_MAP_REG_START, 4, &pcr._base0);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_MAP_REG_START + 0x04, 4, &pcr._base1);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_MAP_REG_START + 0x08, 4, &pcr._base2);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_MAP_REG_START + 0x0C, 4, &pcr._base3);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_MAP_REG_START + 0x10, 4, &pcr._base4);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_MAP_REG_START + 0x14, 4, &pcr._base5);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_MAP_REG_START + 0x1c, 4, &pcr._subsys_card_vendor);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_MAP_ROM_REG, 4, &pcr._baserom);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_INTERRUPT_REG, 4, &pcr._max_min_ipin_iline);
	    pciconfig_read(pcr._pcibuses[pcr._pcibusidx], (pcr._cardnum<<3)|func,
			PCI_REG_USERCONFIG, 4, &pcr._user_config);
#endif

            /* check for pci-pci bridges */
#define PCI_CLASS_MASK 		0xff000000
#define PCI_SUBCLASS_MASK 	0x00ff0000
#define PCI_CLASS_BRIDGE 	0x06000000
#define PCI_SUBCLASS_BRIDGE_PCI	0x00040000
	    switch(pcr._class_revision & (PCI_CLASS_MASK|PCI_SUBCLASS_MASK)) {
		case PCI_CLASS_BRIDGE|PCI_SUBCLASS_BRIDGE_PCI:
		    if (pcr._secondary_bus_number > 0) {
		        pcr._pcibuses[pcr._pcinumbus++] = pcr._secondary_bus_number;
		    }
			break;
		case PCI_CLASS_BRIDGE:
		    if ( ++hostbridges > 1) {
			pcr._pcibuses[pcr._pcinumbus] = pcr._pcinumbus;
			pcr._pcinumbus++;
		    }
			break;
		default:
			break;
	    }
	    if((func==0) && ((pcr._header_type & PCI_MULTIFUNC_DEV) == 0)) {
	        /* not a multi function device */
		func = 8;
	    } else {
	        func++;
	    }

	    if (idx++ >= MAX_PCI_DEVICES)
	        continue;

	    identify_card(&pcr, verbose);
	  } while( func < 8 );
        }
    } while (++pcr._pcibusidx < pcr._pcinumbus);
    }

#if !defined(__alpha__) && !defined(__powerpc__) && !defined(__sparc__)
    /* Now try pci config 2 probe (deprecated) */

    if ((pcr._configtype == 2) || do_mode2_scan) {
    outb(PCI_MODE2_ENABLE_REG, 0xF1);
    outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */

    printf("\nPCI probing configuration type 2\n");

    pcr._pcibuses[0] = 0;
    pcr._pcinumbus = 1;
    pcr._pcibusidx = 0;
    idx = 0;

    do {
        for (pcr._ioaddr = 0xC000; pcr._ioaddr < 0xD000; pcr._ioaddr += 0x0100){
	    outb(PCI_MODE2_FORWARD_REG, pcr._pcibuses[pcr._pcibusidx]); /* bus 0 for now */
            pcr._device_vendor = inl(pcr._ioaddr);
	    outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */

            if ((pcr._vendor == 0xFFFF) || (pcr._device == 0xFFFF))
                continue;
            if ((pcr._vendor == 0xF0F0) || (pcr._device == 0xF0F0))
                continue;  /* catch ASUS P55TP4XE motherboards */

	    printf("\npci bus 0x%x slot at 0x%04x, vendor 0x%04x device 0x%04x\n",
	        pcr._pcibuses[pcr._pcibusidx], pcr._ioaddr, pcr._vendor,
                pcr._device);

	    outb(PCI_MODE2_FORWARD_REG, pcr._pcibuses[pcr._pcibusidx]); /* bus 0 for now */
            pcr._status_command = inl(pcr._ioaddr + 0x04);
            pcr._class_revision = inl(pcr._ioaddr + 0x08);
            pcr._bist_header_latency_cache = inl(pcr._ioaddr + 0x0C);
            pcr._base0 = inl(pcr._ioaddr + 0x10);
            pcr._base1 = inl(pcr._ioaddr + 0x14);
            pcr._base2 = inl(pcr._ioaddr + 0x18);
            pcr._base3 = inl(pcr._ioaddr + 0x1C);
            pcr._base4 = inl(pcr._ioaddr + 0x20);
            pcr._base5 = inl(pcr._ioaddr + 0x24);
            pcr._subsys_card_vendor = inl(pcr._ioaddr + 0x2c);
            pcr._baserom = inl(pcr._ioaddr + 0x30);
            pcr._max_min_ipin_iline = inl(pcr._ioaddr + 0x3C);
            pcr._user_config = inl(pcr._ioaddr + 0x40);
	    outb(PCI_MODE2_FORWARD_REG, 0x00); /* bus 0 for now */

            /* check for pci-pci bridges (currently we only know Digital) */
            if ((pcr._vendor == 0x1011) && (pcr._device == 0x0001))
                if (pcr._secondary_bus_number > 0)
                    pcr._pcibuses[pcr._pcinumbus++] = pcr._secondary_bus_number;

	    if (idx++ >= MAX_PCI_DEVICES)
	        continue;

	    identify_card(&pcr, verbose);
	}
    } while (++pcr._pcibusidx < pcr._pcinumbus);

    outb(PCI_MODE2_ENABLE_REG, 0x00);
    }

#endif /* __alpha__ */

    disable_os_io();
}


void
identify_card(struct pci_config_reg *pcr, int verbose)
{

	int i = 0, j, foundit = 0;

	while (pvd[i].vendorname != (char *)NULL) {
	    if (pvd[i].vendor_id == pcr->_vendor) {
		j = 0;
		printf(" %s ", pvd[i].vendorname);
		while (pvd[i].device[j].devicename != (char *)NULL) {
		    if (pvd[i].device[j].device_id == pcr->_device) {
	                printf("%s", pvd[i].device[j].devicename);
			foundit = 1;
			break;
		    }
		    j++;
		}
	    }
	    if (foundit)
		break;
	    i++;
	}

	if (!foundit)
		printf(" Device unknown\n");
	else {
	    printf("\n");
	    if (verbose) {
	        if (pvd[i].device[j].print_func != (void (*)())NULL) {
                    pvd[i].device[j].print_func(pcr);
		    return;
	        }
	    }
	}

	if (verbose) {
	    printf(" CardVendor 0x%04x card 0x%04x\n",
		pcr->_subsys_vendor, pcr->_subsys_card);
            if (pcr->_status_command)
                printf("  STATUS    0x%04x  COMMAND 0x%04x\n",
                    pcr->_status, pcr->_command);
            if (pcr->_class_revision)
                printf("  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
                    pcr->_base_class, pcr->_sub_class, pcr->_prog_if,
		    pcr->_rev_id);
	    switch (pcr->_class_revision & PCI_CLASS_MASK) {
	    case PCI_CLASS_BRIDGE:
		switch (pcr->_class_revision & PCI_SUBCLASS_MASK) {
		case PCI_SUBCLASS_BRIDGE_PCI:
		    print_bridge_pci_class(pcr);
		    break;
		default:
		    print_bridge_class(pcr);
		    break;
		}
		break;
	    default:
		print_default_class(pcr);
		break;
	    }
	}
}

void
print_default_class(struct pci_config_reg *pcr)
{
    if (pcr->_bist_header_latency_cache)
	printf("  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
	       pcr->_bist, pcr->_header_type, pcr->_latency_timer,
	       pcr->_cache_line_size);
    if (pcr->_base0)
	printf("  BASE0     0x%08x  addr 0x%08x  %s\n",
	       pcr->_base0, pcr->_base0 & (pcr->_base0 & 0x1 ?
					   0xFFFFFFFC : 0xFFFFFFF0), 
	       pcr->_base0 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base1)
	printf("  BASE1     0x%08x  addr 0x%08x  %s\n",
	       pcr->_base1, pcr->_base1 & (pcr->_base1 & 0x1 ?
					   0xFFFFFFFC : 0xFFFFFFF0), 
	       pcr->_base1 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base2)
	printf("  BASE2     0x%08x  addr 0x%08x  %s\n",
	       pcr->_base2, pcr->_base2 & (pcr->_base2 & 0x1 ?
					   0xFFFFFFFC : 0xFFFFFFF0), 
	       pcr->_base2 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base3)
	printf("  BASE3     0x%08x  addr 0x%08x  %s\n",
	       pcr->_base3, pcr->_base3 & (pcr->_base3 & 0x1 ?
					   0xFFFFFFFC : 0xFFFFFFF0), 
	       pcr->_base3 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base4)
	printf("  BASE4     0x%08x  addr 0x%08x  %s\n",
	       pcr->_base4, pcr->_base4 & (pcr->_base4 & 0x1 ?
					   0xFFFFFFFC : 0xFFFFFFF0), 
	       pcr->_base4 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base5)
	printf("  BASE5     0x%08x  addr 0x%08x  %s\n",
	       pcr->_base5, pcr->_base5 & (pcr->_base5 & 0x1 ?
					   0xFFFFFFFC : 0xFFFFFFF0), 
	       pcr->_base5 & 0x1 ? "I/O" : "MEM");
    if (pcr->_baserom)
	printf("  BASEROM   0x%08x  addr 0x%08x  %sdecode-enabled\n",
	       pcr->_baserom, pcr->_baserom & 0xFFFF8000,
	       pcr->_baserom & 0x1 ? "" : "not-");
    if (pcr->_max_min_ipin_iline)
	printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
	       pcr->_max_lat, pcr->_min_gnt, 
	       pcr->_int_pin, pcr->_int_line);
    if (pcr->_user_config)
	printf("  BYTE_0    0x%02x  BYTE_1  0x%02x  BYTE_2  0x%02x  BYTE_3  0x%02x\n",
	       pcr->_user_config_0, pcr->_user_config_1, 
	       pcr->_user_config_2, pcr->_user_config_3);
}

#define PCI_B_FAST_B_B 0x80
#define PCI_B_SB_RESET 0x40
#define PCI_B_M_ABORT  0x20
#define PCI_B_VGA_EN   0x08
#define PCI_B_ISA_EN   0x04
#define PCI_B_P_ERR    0x01
void
print_bridge_pci_class(struct pci_config_reg *pcr)
{
    if (pcr->_bist_header_latency_cache)
        printf("  HEADER    0x%02x  LATENCY 0x%02x\n",
	       pcr->_header_type, pcr->_latency_timer);
    printf("  PRIBUS    0x%02x  SECBUS 0x%02x  SUBBUS 0x%02x  SECLT 0x%02x\n",
           pcr->_primary_bus_number, pcr->_secondary_bus_number,
	   pcr->_subordinate_bus_number, pcr->_secondary_latency_timer);
    printf("  IOBASE    0x%02x  IOLIM 0x%02x  SECSTATUS 0x%04x\n",
	   pcr->_io_base << 8, (pcr->_io_limit << 8) | 0xfff,
	   pcr->_secondary_status);
    printf("  NOPREFETCH_MEMBASE 0x%08x  MEMLIM 0x%08x\n",
	   pcr->_mem_base << 16, (pcr->_mem_limit << 16) | 0xfffff);
    printf("  PREFETCH_MEMBASE   0x%08x  MEMLIM 0x%08x\n",
	   pcr->_prefetch_mem_base << 16,
	   (pcr->_prefetch_mem_limit << 16) | 0xfffff);
    printf("  %sFAST_B2B %sSEC_BUS_RST %sM_ABRT %sVGA_EN %sISA_EN"
	   " %sPERR_EN\n",
	   (pcr->_b_ctrl & PCI_B_FAST_B_B) ? "" : "NO_",
	   (pcr->_b_ctrl & PCI_B_SB_RESET) ? "" : "NO_",
	   (pcr->_b_ctrl & PCI_B_M_ABORT) ? "" : "NO_",
	   (pcr->_b_ctrl & PCI_B_VGA_EN) ? "" : "NO_",
	   (pcr->_b_ctrl & PCI_B_ISA_EN) ? "" : "NO_",
	   (pcr->_b_ctrl & PCI_B_P_ERR) ? "" : "NO_");
}

void
print_bridge_class(struct pci_config_reg *pcr)
{
    if (pcr->_bist_header_latency_cache)
        printf("  HEADER    0x%02x  LATENCY 0x%02x\n",
	       pcr->_header_type, pcr->_latency_timer);
}


void
print_mach64(struct pci_config_reg *pcr)
{
    unsigned long sparse_io = 0;

    if (pcr->_status_command)
        printf("  STATUS    0x%04x  COMMAND 0x%04x\n",
            pcr->_status, pcr->_command);
    if (pcr->_class_revision)
        printf("  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
            pcr->_base_class, pcr->_sub_class, pcr->_prog_if, pcr->_rev_id);
    if (pcr->_bist_header_latency_cache)
        printf("  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
            pcr->_bist, pcr->_header_type, pcr->_latency_timer,
            pcr->_cache_line_size);
    if (pcr->_base0)
        printf("  APBASE    0x%08x  addr 0x%08x\n",
            pcr->_base0, pcr->_base0 & (pcr->_base0 & 0x1 ?
		0xFFFFFFFC : 0xFFFFFFF0));
    if (pcr->_base1)
        printf("  BLOCKIO   0x%08x  addr 0x%08x\n",
            pcr->_base1, pcr->_base1 & (pcr->_base1 & 0x1 ?
		0xFFFFFFFC : 0xFFFFFFF0));
    if (pcr->_base2)
        printf("  REGBASE   0x%08x  addr 0x%08x\n",
            pcr->_base2, pcr->_base2 & (pcr->_base2 & 0x1 ?
		0xFFFFFFFC : 0xFFFFFFF0));
    if (pcr->_baserom)
        printf("  BASEROM   0x%08x  addr 0x%08x  %sdecode-enabled\n",
            pcr->_baserom, pcr->_baserom & 0xFFFF8000,
            pcr->_baserom & 0x1 ? "" : "not-");
    if (pcr->_max_min_ipin_iline)
        printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
            pcr->_max_lat, pcr->_min_gnt, pcr->_int_pin, pcr->_int_line);
    switch (pcr->_user_config_0 & 0x03) {
    case 0:
	sparse_io = 0x2ec;
	break;
    case 1:
	sparse_io = 0x1cc;
	break;
    case 2:
	sparse_io = 0x1c8;
	break;
    }
    printf("  SPARSEIO  0x%03x    %s IO enabled    %sable 0x46E8\n",
	    sparse_io, pcr->_user_config_0 & 0x04 ? "Block" : "Sparse",
	    pcr->_user_config_0 & 0x08 ? "Dis" : "En");
}

void
print_i128(struct pci_config_reg *pcr)
{
    if (pcr->_status_command)
        printf("  STATUS    0x%04x  COMMAND 0x%04x\n",
            pcr->_status, pcr->_command);
    if (pcr->_class_revision)
        printf("  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
            pcr->_base_class, pcr->_sub_class, pcr->_prog_if, pcr->_rev_id);
    if (pcr->_bist_header_latency_cache)
        printf("  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
            pcr->_bist, pcr->_header_type, pcr->_latency_timer,
            pcr->_cache_line_size);
    printf("  MW0_AD    0x%08x  addr 0x%08x  %spre-fetchable\n",
        pcr->_base0, pcr->_base0 & 0xFFC00000,
        pcr->_base0 & 0x8 ? "" : "not-");
    printf("  MW1_AD    0x%08x  addr 0x%08x  %spre-fetchable\n",
        pcr->_base1, pcr->_base1 & 0xFFC00000,
        pcr->_base1 & 0x8 ? "" : "not-");
    printf("  XYW_AD(A) 0x%08x  addr 0x%08x\n",
        pcr->_base2, pcr->_base2 & 0xFFC00000);
    printf("  XYW_AD(B) 0x%08x  addr 0x%08x\n",
        pcr->_base3, pcr->_base3 & 0xFFC00000);
    printf("  RBASE_G   0x%08x  addr 0x%08x\n",
        pcr->_base4, pcr->_base4 & 0xFFFF0000);
    printf("  IO        0x%08x  addr 0x%08x\n",
        pcr->_base5, pcr->_base5 & 0xFFFFFF00);
    printf("  RBASE_E   0x%08x  addr 0x%08x  %sdecode-enabled\n",
        pcr->_baserom, pcr->_baserom & 0xFFFF8000,
        pcr->_baserom & 0x1 ? "" : "not-");
    if (pcr->_max_min_ipin_iline)
        printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
            pcr->_max_lat, pcr->_min_gnt, pcr->_int_pin, pcr->_int_line);
}

void
print_pcibridge(struct pci_config_reg *pcr)
{
    if (pcr->_status_command)
        printf("  STATUS    0x%04x  COMMAND 0x%04x\n",
            pcr->_status, pcr->_command);
    if (pcr->_class_revision)
        printf("  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
            pcr->_base_class, pcr->_sub_class, pcr->_prog_if, pcr->_rev_id);
    if (pcr->_bist_header_latency_cache)
        printf("  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
            pcr->_bist, pcr->_header_type, pcr->_latency_timer,
            pcr->_cache_line_size);
    printf("  PRIBUS    0x%02x  SECBUS 0x%02x  SUBBUS 0x%02x  SECLT 0x%02x\n",
           pcr->_primary_bus_number, pcr->_secondary_bus_number,
	   pcr->_subordinate_bus_number, pcr->_secondary_latency_timer);
    printf("  IOBASE    0x%02x  IOLIM 0x%02x  SECSTATUS 0x%04x\n",
	   pcr->_io_base << 8, (pcr->_io_limit << 8) | 0xfff,
	   pcr->_secondary_status);
    printf("  NOPREFETCH_MEMBASE 0x%08x  MEMLIM 0x%08x\n",
	   pcr->_mem_base << 16, (pcr->_mem_limit << 16) | 0xfffff);
    printf("  PREFETCH_MEMBASE   0x%08x  MEMLIM 0x%08x\n",
	   pcr->_prefetch_mem_base << 16,
	   (pcr->_prefetch_mem_limit << 16) | 0xfffff);
    printf("  RBASE_E   0x%08x  addr 0x%08x  %sdecode-enabled\n",
        pcr->_baserom, pcr->_baserom & 0xFFFF8000,
        pcr->_baserom & 0x1 ? "" : "not-");
    if (pcr->_max_min_ipin_iline)
        printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
            pcr->_max_lat, pcr->_min_gnt, pcr->_int_pin, pcr->_int_line);
}

static int io_fd;
#ifdef __EMX__
USHORT callgate[3] = {0,0,0};
#endif

void
enable_os_io()
{
#if (defined(SVR4) || defined(SCO) || defined(ISC)) || defined(DGUX)
#if defined(SI86IOPL) || defined(DGUX)
    sysi86(SI86IOPL, 3);
#else
    sysi86(SI86V86, V86SC_IOPL, PS_IOPL);
#endif
#endif
#if defined(linux) && !defined(__sparc__)
    iopl(3);
#endif
#if defined(__FreeBSD__)  || defined(__386BSD__) || defined(__bsdi__)
    if ((io_fd = open("/dev/console", O_RDWR, 0)) < 0) {
        perror("/dev/console");
        exit(1);
    }
#if defined(__FreeBSD__)  || defined(__386BSD__)
    if (ioctl(io_fd, KDENABIO, 0) < 0) {
        perror("ioctl(KDENABIO)");
        exit(1);
    }
#endif
#if defined(__bsdi__)
    if (ioctl(io_fd, PCCONENABIOPL, 0) < 0) {
        perror("ioctl(PCCONENABIOPL)");
        exit(1);
    }
#endif
#endif
#if defined(__NetBSD__)
#if !defined(USE_I386_IOPL)
    if ((io_fd = open("/dev/io", O_RDWR, 0)) < 0) {
	perror("/dev/io");
	exit(1);
    }
#else
    if (i386_iopl(1) < 0) {
	perror("i386_iopl");
	exit(1);
    }
#endif /* USE_I386_IOPL */
#endif /* __NetBSD__ */
#if defined(__OpenBSD__)
    if (i386_iopl(1) < 0) {
	perror("i386_iopl");
	exit(1);
    }
#endif /* __OpenBSD__ */
#if defined(MACH386)
    if ((io_fd = open("/dev/iopl", O_RDWR, 0)) < 0) {
        perror("/dev/iopl");
        exit(1);
    }
#endif
#ifdef __EMX__
    {
	HFILE hfd;
	ULONG dlen,action;
	APIRET rc;
	static char *ioDrvPath = "/dev/fastio$";

	if (DosOpen((PSZ)ioDrvPath, (PHFILE)&hfd, (PULONG)&action,
	   (ULONG)0, FILE_SYSTEM, FILE_OPEN,
	   OPEN_SHARE_DENYNONE|OPEN_FLAGS_NOINHERIT|OPEN_ACCESS_READONLY,
	   (ULONG)0) != 0) {
		fprintf(stderr,"Error opening fastio$ driver...\n");
		fprintf(stderr,"Please install xf86sup.sys in config.sys!\n");
		exit(42);
	}
	callgate[0] = callgate[1] = 0;

/* Get callgate from driver for fast io to ports and other stuff */

	rc = DosDevIOCtl(hfd, (ULONG)0x76, (ULONG)0x64,
		NULL, 0, NULL,
		(ULONG*)&callgate[2], sizeof(USHORT), &dlen);
	if (rc) {
		fprintf(stderr,"xf86-OS/2: EnableIOPorts failed, rc=%d, dlen=%d; emergency exit\n",
			rc,dlen);
		DosClose(hfd);
		exit(42);
	}

/* Calling callgate with function 13 sets IOPL for the program */

	asm volatile ("movl $13,%%ebx;.byte 0xff,0x1d;.long _callgate"
			: /*no outputs */
			: /*no inputs */
			: "eax","ebx","ecx","edx","cc");

        DosClose(hfd);
   }
#endif
#if defined(Lynx) && defined(__powerpc__)
    pciConfBase = (unsigned char *) smem_create("PCI-CONF",
    	    (char *)0x80800000, 64*1024, SM_READ|SM_WRITE);
    if (pciConfBase == (void *) -1)
        exit(1);
#endif
}


void
disable_os_io()
{
#if (defined(SVR4) || defined(SCO) || defined(ISC)) || defined(DGUX)
#if defined(SI86IOPL) || defined(DGUX)
    sysi86(SI86IOPL, 0);
#else
    sysi86(SI86V86, V86SC_IOPL, 0);
#endif
#endif
#if defined(linux) && !defined(__sparc__)
    iopl(0);
#endif
#if defined(__FreeBSD__)  || defined(__386BSD__)
    if (ioctl(io_fd, KDDISABIO, 0) < 0) {
        perror("ioctl(KDDISABIO)");
        close(io_fd);
        exit(1);
    }
    close(io_fd);
#endif
#if defined(__NetBSD__)
#if !defined(USE_I386_IOPL)
    close(io_fd);
#else
    if (i386_iopl(0) < 0) {
        perror("i386_iopl");
        exit(1);
    }
#endif /* NetBSD1_1 */
#endif /* __NetBSD__ */
#if defined(__bsdi__)
    if (ioctl(io_fd, PCCONDISABIOPL, 0) < 0) {
        perror("ioctl(PCCONDISABIOPL)");
        close(io_fd);
        exit(1);
    }
    close(io_fd);
#endif
#if defined(MACH386)
    close(io_fd);
#endif
#if defined(Lynx) && defined(__powerpc__)
    smem_create(NULL, (char *) pciConfBase, 0, SM_DETACH);
    smem_remove("PCI-CONF");
    pciConfBase = NULL;
#endif
}

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
#endif /* DGUX && NO_SYSI86 */

