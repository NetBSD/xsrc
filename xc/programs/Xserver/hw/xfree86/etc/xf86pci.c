/* $XFree86: xc/programs/Xserver/hw/xfree86/etc/xf86pci.c,v 3.9 1996/10/16 14:41:31 dawes Exp $ */
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
/* $XConsortium: xf86pci.c /main/3 1995/11/12 20:17:58 kaleb $ */

#include <stdio.h>
#include <sys/types.h>
#if defined(SVR4)
#ifdef sun
#define __EXTENSIONS__
#endif
#include <sys/proc.h>
#include <sys/tss.h>
#ifdef NCR
#define __STDC
#include <sys/sysi86.h>
#undef __STDC
#else
#include <sys/sysi86.h>
#endif
#ifndef sun
#include <sys/seg.h>
#endif
#include <sys/v86.h>
#ifdef sun
#include <sys/psw.h>
#endif
#endif
#if defined(__FreeBSD__) || defined(__386BSD__)
#include <sys/file.h>
#include <machine/console.h>
#define GCCUSESGAS
#endif
#if defined(__NetBSD__)
#include <sys/param.h>
#ifndef NetBSD1_1
#include <sys/file.h>
#else
#include <machine/sysarch.h>
#endif
#define GCCUSESGAS
#endif
#if defined(__OpenBSD__)
#include <machine/sysarch.h>
#endif
#if defined(__bsdi__)
#include <sys/file.h>
#include <sys/ioctl.h>
#include <i386/isa/pcconsioctl.h>
#define GCCUSESGAS
#endif
#if defined(SCO)
#  include <sys/console.h>
#  include <sys/param.h>
#  include <sys/immu.h>
#  include <sys/region.h>
#  include <sys/proc.h>
#  include <sys/tss.h>
#  include <sys/sysi86.h>
#  include <sys/v86.h>
#endif
#if defined(Lynx_22)
#  define GCCUSESGAS
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

void outb(short port, char val) {
     __asm__ __volatile__(OUTB_GCC : :"a" (val), "d" (port)); }
void outl(short port, unsigned long val) {
     __asm__ __volatile__(OUTL_GCC : :"a" (val), "d" (port)); }
unsigned char inb(short port) { unsigned char ret;
     __asm__ __volatile__(INB_GCC : "=a" (ret) : "d" (port)); return ret; }
unsigned long inl(short port) { unsigned long ret;
     __asm__ __volatile__(INL_GCC : "=a" (ret) : "d" (port)); return ret; }

#else  /* __GNUC__ */

#if defined(__STDC__) && (__STDC__ == 1)
# ifndef NCR
#  define asm __asm
# endif
#endif

#ifdef SVR4
# ifndef __USLC__
#  define __USLC__
# endif
#endif

#ifndef SCO325
# include <sys/inline.h>
#else
# include "../common/scoasm.h"
#endif

#endif /* __GNUC__ */
#endif /* __WATCOMC__ */

#include "xf86_PCI.h"

void xf86ClearIOPortList(int);
void xf86AddIOPorts(int, int, unsigned *);
void xf86EnableIOPorts(int);
void xf86DisableIOPorts(int);

extern void identify_card(struct pci_config_reg *);
extern void print_i128(struct pci_config_reg *);
extern void print_pcibridge(struct pci_config_reg *);

#define MAX_DEV_PER_VENDOR 16
#define NF ((void (*)())NULL)

struct pci_vendor_device {
    unsigned short vendor_id;
    char *vendorname;
    struct pci_device {
        unsigned short device_id;
        char *devicename;
	void (*print_func)(struct pci_config_reg *);
    } device[MAX_DEV_PER_VENDOR];
} pvd[] = {
        { 0x1000, "NCR", {
                            { 0x0001, "53C810", NF },
                            { 0x0002, "53C820", NF },
                            { 0x0003, "53C825", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1002, "ATI", {
                            { 0x4158, "Mach32", NF },
                            { 0x4758, "Mach64", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x100C, "Tseng Labs", {
                            { 0x3202, "ET4000w32p rev A", NF },
                            { 0x3207, "ET4000w32p rev D", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x100E, "Diamond", {
                            { 0x9001, "Viper/PCI", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1011, "Digital", {
                            { 0x0001, "DC21050 PCI-PCI Bridge",print_pcibridge},
                            { 0x0002, "DC21040 10Mb/s Ethernet", NF },
                            { 0x0009, "DC21140 10/100 Mb/s Ethernet", NF },
                            { 0x000F, "DEFPA (FDDI PCI)", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1013, "Cirrus Logic", {
                            { 0x00A4, "unknown", NF },
                            { 0x00A8, "5434", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x101A, "NCR", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1025, "ALI", {
                            { 0x1435, "M1435", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x102B, "Matrox", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1039, "SIS", {
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1042, "SMC", {
                            { 0x1000, "37C665", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1045, "Opti", {
                            { 0xC822, "82C822", NF },
                            { 0xC621, "82C621", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x104B, "BusLogic", {
                            { 0x0140, "946C 01", NF },
                            { 0x1040, "946C 10", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x105D, "Number Nine", {
                            { 0x2309, "Imagine-128", print_i128 },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x1060, "UMC", {
                            { 0x0101, "UM8673F", NF },
                            { 0x8881, "UM8881F", NF },
                            { 0x8886, "UM8886F", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x5333, "S3", {
                            { 0x8811, "Trio64", NF },
                            { 0x88B0, "928", NF },
                            { 0x88C0, "864-0", NF },
                            { 0x88C1, "864-1", NF },
                            { 0x88D0, "964", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x8086, "Intel", {
                            { 0x0482, "82375EB pci-eisa bridge", NF },
                            { 0x0483, "82424ZX cache dram controller", NF },
                            { 0x0484, "82378IB pci-isa bridge", NF },
                            { 0x04A3, "82434LX pci cache mem controller", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x9004, "Adaptec", {
                            { 0x7178, "2940", NF },
                            { 0x0000, (char *)NULL, NF } } },
        { 0x0000, (char *)NULL, {
                            { 0x0000, (char *)NULL, NF } } }
};


main(int argc, unsigned char *argv[])
{
    extern struct pci_config_reg *pci_devp[];
    int idx = 0;

    if (argc != 1) {
	printf("Usage: %s\n");
	exit(1);
    }
#ifndef MSDOS
    if (getuid()) {
	printf("This program must be run as root\n");
	exit(1);
    }
#endif

    xf86scanpci(0);

    while ((idx < MAX_PCI_DEVICES) && pci_devp[idx])
        identify_card(pci_devp[idx++]);
}


void
identify_card(struct pci_config_reg *pcr)
{

    int i = 0, j, foundit = 0;

    if (pcr->_configtype == 1)
        printf("\npci bus 0x%x cardnum 0x%02x, vendor 0x%04x device 0x%04x\n",
            pcr->_pcibuses[pcr->_pcibusidx], pcr->_cardnum, pcr->_vendor,
            pcr->_device);
    else
	printf("\npci bus 0x%x slot at 0x%04x, vendor 0x%04x device 0x%04x\n",
	    pcr->_pcibuses[pcr->_pcibusidx], pcr->_ioaddr, pcr->_vendor,
            pcr->_device);

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
	if (pvd[i].device[j].print_func != (void (*)())NULL) {
            pvd[i].device[j].print_func(pcr);
	    return;
	}
    }

    if (pcr->_status_command)
        printf("  STATUS    0x%04x  COMMAND 0x%04x\n",
            pcr->_status, pcr->_command);
    if (pcr->_class_revision)
        printf("  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
            pcr->_base_class, pcr->_sub_class, pcr->_prog_if,
            pcr->_rev_id);
    if (pcr->_bist_header_latency_cache)
        printf("  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
            pcr->_bist, pcr->_header_type, pcr->_latency_timer,
	    pcr->_cache_line_size);
    if (pcr->_base0)
        printf("  BASE0     0x%08x  addr 0x%08x  %s\n",
            pcr->_base0, pcr->_base0 & (pcr->_base0 & 0x1 ?
	    0xFFFFFFFC : 0xFFFFFFF0), pcr->_base0 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base1)
        printf("  BASE1     0x%08x  addr 0x%08x  %s\n",
            pcr->_base1, pcr->_base1 & (pcr->_base1 & 0x1 ?
	    0xFFFFFFFC : 0xFFFFFFF0), pcr->_base1 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base2)
        printf("  BASE2     0x%08x  addr 0x%08x  %s\n",
            pcr->_base2, pcr->_base2 & (pcr->_base2 & 0x1 ?
	    0xFFFFFFFC : 0xFFFFFFF0), pcr->_base2 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base3)
        printf("  BASE3     0x%08x  addr 0x%08x  %s\n",
            pcr->_base3, pcr->_base3 & (pcr->_base3 & 0x1 ?
	    0xFFFFFFFC : 0xFFFFFFF0), pcr->_base3 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base4)
        printf("  BASE4     0x%08x  addr 0x%08x  %s\n",
            pcr->_base4, pcr->_base4 & (pcr->_base4 & 0x1 ?
	    0xFFFFFFFC : 0xFFFFFFF0), pcr->_base4 & 0x1 ? "I/O" : "MEM");
    if (pcr->_base5)
        printf("  BASE5     0x%08x  addr 0x%08x  %s\n",
            pcr->_base5, pcr->_base5 & (pcr->_base5 & 0x1 ?
	    0xFFFFFFFC : 0xFFFFFFF0), pcr->_base5 & 0x1 ? "I/O" : "MEM");
    if (pcr->_baserom)
        printf("  BASEROM   0x%08x  addr 0x%08x  %sdecode-enabled\n",
            pcr->_baserom, pcr->_baserom & 0xFFFF8000,
            pcr->_baserom & 0x1 ? "" : "not-");
    if (pcr->_max_min_ipin_iline)
        printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
            pcr->_max_lat, pcr->_min_gnt, pcr->_int_pin, pcr->_int_line);
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
    printf("  PRIBUS 0x%02x SECBUS 0x%02x SUBBUS 0x%02x SECLT 0x%02x\n",
           pcr->_primary_bus_number, pcr->_secondary_bus_number,
	   pcr->_subordinate_bus_number, pcr->_secondary_latency_timer);
    printf("  IOBASE: 0x%02x00 IOLIM 0x%02x00 SECSTATUS 0x%04x\n",
	pcr->_io_base, pcr->_io_limit, pcr->_secondary_status);
    printf("  NOPREFETCH MEMBASE: 0x%08x MEMLIM 0x%08x\n",
	pcr->_mem_base, pcr->_mem_limit);
    printf("  PREFETCH MEMBASE: 0x%08x MEMLIM 0x%08x\n",
	pcr->_prefetch_mem_base, pcr->_prefetch_mem_limit);
    printf("  RBASE_E   0x%08x  addr 0x%08x  %sdecode-enabled\n",
        pcr->_baserom, pcr->_baserom & 0xFFFF8000,
        pcr->_baserom & 0x1 ? "" : "not-");
    if (pcr->_max_min_ipin_iline)
        printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
            pcr->_max_lat, pcr->_min_gnt, pcr->_int_pin, pcr->_int_line);
}

static int io_fd;

void
xf86ClearIOPortList(int dummy) {}
void
xf86AddIOPorts(int dummy1, int dummy2, unsigned *dummy3) {}

void
xf86EnableIOPorts(int dummy)
{
#if defined(SVR4) || defined(SCO)
#if defined(SI86IOPL)
    sysi86(SI86IOPL, 3);
#else
    sysi86(SI86V86, V86SC_IOPL, PS_IOPL);
#endif
#endif
#ifdef linux
    iopl(3);
#endif
#if defined(__FreeBSD__) || defined(__386BSD__) || defined(__bsdi__)
    if ((io_fd = open("/dev/console", O_RDWR, 0)) < 0) {
        perror("/dev/console");
        exit(1);
    }
#if defined(__FreeBSD__) || defined(__386BSD__)
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
}


void
xf86DisableIOPorts(int dummy)
{
#if defined(SVR4) || defined(SCO)
#if defined(SI86IOPL)
    sysi86(SI86IOPL, 0);
#else
    sysi86(SI86V86, V86SC_IOPL, 0);
#endif
#endif
#ifdef linux
    iopl(0);
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__386BSD__)
    if (ioctl(io_fd, KDDISABIO, 0) < 0) {
        perror("ioctl(KDDISABIO)");
	close(io_fd);
        exit(1);
    }
    close(io_fd);
#endif
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
}

/* These are to allow libxf86_hw.a use Xalloc(), Xfree() */

unsigned long *
Xalloc(unsigned long amount)
{
	return (unsigned long *)malloc(amount);
}

void
Xfree(void *ptr)
{
	free(ptr);
	return;
}
