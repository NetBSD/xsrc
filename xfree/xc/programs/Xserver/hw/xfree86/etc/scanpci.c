/* Copyright 2000 by Egbert Eich 
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
/* $XFree86: xc/programs/Xserver/hw/xfree86/etc/scanpci.c,v 3.80 2000/06/20 19:38:04 eich Exp $ */

#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSproc.h"
#include "xf86Pci.h"
#include "xf86ScanPci.h"
#include "xf86PciInfo.h"
#include "dummylib.h"

#include <stdarg.h>
#include <stdlib.h>
#ifdef __linux__
/* to get getopt on Linux */
#ifndef __USE_POSIX2
#define __USE_POSIX2
#endif
#endif
#include <unistd.h>

#if defined(ISC) || defined(Lynx)
extern char *optarg;
extern int optind, opterr;
#endif

void usage(void);
void identify_card(pciConfigPtr pcr, int verbose);
void print_default_class(pciConfigPtr pcr);
void print_bridge_pci_class(pciConfigPtr pcr);
void print_bridge_class(pciConfigPtr pcr);
void print_mach64(pciConfigPtr pcr);
void print_i128(pciConfigPtr pcr);
void print_pcibridge(pciConfigPtr pcr);

typedef struct {
    unsigned int Vendor;
    struct {
	int DeviceID;
	void(*func)(pciConfigPtr);
    } Device[MAX_DEV_PER_VENDOR];
} pciVendorDevFuncInfo;

pciVendorDevFuncInfo vendorDeviceFuncInfo[] = {
    { 0x1002, {
	{ 0x4354, print_mach64 },
	{ 0x4358, print_mach64 },
	{ 0x4554, print_mach64 },
	{ 0x4742, print_mach64 },
	{ 0x4744, print_mach64 },
	{ 0x4749, print_mach64 },
	{ 0x474C, print_mach64 },
	{ 0x474D, print_mach64 },
	{ 0x474E, print_mach64 },
	{ 0x474F, print_mach64 },
	{ 0x4750, print_mach64 },
	{ 0x4751, print_mach64 },
	{ 0x4752, print_mach64 },
	{ 0x4753, print_mach64 },
	{ 0x4754, print_mach64 },
	{ 0x4755, print_mach64 },
	{ 0x4756, print_mach64 },
	{ 0x4757, print_mach64 },
	{ 0x4758, print_mach64 },
	{ 0x475A, print_mach64 },
	{ 0x4C42, print_mach64 },
	{ 0x4C44, print_mach64 },
	{ 0x4C47, print_mach64 },
	{ 0x4C49, print_mach64 },
	{ 0x4C4D, print_mach64 },
	{ 0x4C4E, print_mach64 },
	{ 0x4C50, print_mach64 },
	{ 0x4C52, print_mach64 },
	{ 0x4C53, print_mach64 },
	{ 0x5654, print_mach64 },
	{ 0x5655, print_mach64 },
	{ 0x5656, print_mach64 },
	{ 0x0000,  NULL } } },
    { 0x105D, {
	{ 0x2309, print_i128 },
	{ 0x2339, print_i128 },
	{ 0x493D, print_i128 },
	{ 0x5348, print_i128 },
	{ 0x0000, NULL } } },
    { 0x1011, {
	{ 0x0001, print_pcibridge},
	{ 0x0000, NULL } } },
    { 0x0000, {
	{ 0x0000, NULL } } }
};

void
usage(void)
{
    printf("Usage: scanpci [-v12OfV]\n");
    printf("           -v print config space\n");
    printf("           -1 config type 1\n");
    printf("           -2 config type 2\n");
    printf("           -O use OS config support\n");    
    printf("           -f force config type\n");
    printf("           -V set message verbosity level\n");
}

int
main(int argc, char *argv[])
{
    pciConfigPtr *pcrpp = NULL;
    int Verbose = 0;
    int i = 0;
    int force = 0;
    char c;
    
    xf86Info.pciFlags = PCIProbe1;

    while ((c = getopt(argc, argv, "?v12OfV:")) != -1) 
	switch(c) {
	case 'v':
	    Verbose = 1;
	    break;
	case '1':
	    xf86Info.pciFlags = PCIProbe1;
	    break;
	case '2':
	    xf86Info.pciFlags = PCIProbe2;
	    break;
	case 'O':
	    xf86Info.pciFlags = PCIOsConfig;
	    break;
	case 'f':
	    force = 1;
	    break;
	case 'V':
	    xf86Verbose = atoi(optarg);
	    break;
	case '?':
	default:
	    usage();
	    exit (1);
	    break;
	}

    if (force)
	switch (xf86Info.pciFlags) {
	case PCIProbe1:
	    xf86Info.pciFlags = PCIForceConfig1;
	    break;
	case PCIProbe2:
	    xf86Info.pciFlags = PCIForceConfig2;
	    break;
	default:
	    break;
	}
    	    
    xf86EnableIO();
    pcrpp = xf86scanpci(0);  
    xf86DisableIO();

    if (!pcrpp) {
	printf("No PCI devices found\n");
	exit (1);
    }
    
    while (pcrpp[i])
	identify_card(pcrpp[i++],Verbose);

    exit(0);
}

void
identify_card(pciConfigPtr pcr, int verbose)
{

    int i, j; 
    int foundit = 0;
    int foundvendor = 0;

    SymTabRec *pvnd;
    pciVendorDeviceInfo *pvd;
    pciVendorDevFuncInfo *vdf = vendorDeviceFuncInfo;
    pciVendorCardInfo *pvc;
    
    xf86SetupScanPci(&pvnd,&pvd,&pvc);
    
    printf("\npci bus 0x%x cardnum 0x%02x function 0x%04x: vendor 0x%04x device 0x%04x\n",
	   pcr->busnum, pcr->devnum, pcr->funcnum,
	   pcr->pci_vendor, pcr->pci_device);
    
    for (i = 0;  pvnd[i].name;  i++) {
	if (pvnd[i].token == pcr->pci_vendor) {
	    printf(" %s ", pvnd[i].name);
	    break;
	}
    }

    for (i = 0; pvd[i].VendorID && pvd[i].VendorID != pcr->pci_vendor; i++)
	;
    if (pvd[i].VendorID) {
	for (j = 0;  pvd[i].Device[j].DeviceName;  j++) {
	    if (pvd[i].Device[j].DeviceID == pcr->pci_device) {
		printf("%s", pvd[i].Device[j].DeviceName);
		foundit = 1;
		break;
	    }
	}
    }

    if (!foundit)
	printf(" Device unknown\n");
    else {
	printf("\n");
	if (verbose) {
	    for (i = 0;  vdf[i].Vendor;  i++) {
		if (vdf[i].Vendor == pcr->pci_vendor) {
		    for (j = 0;  vdf[i].Device[j].DeviceID;  j++) {
			if (vdf[i].Device[j].DeviceID == pcr->pci_device) {
			    vdf[i].Device[j].func(pcr);
			    return;
			}
		    }
		    break;
		}
	    }
	}
    }

    if (verbose && !(pcr->pci_header_type & 0x7f) &&
	(pcr->pci_subsys_vendor != 0 || pcr->pci_subsys_card != 0)) {
        foundit = 0;
        foundvendor = 0;
	printf(" CardVendor 0x%04x card 0x%04x",
	       pcr->pci_subsys_vendor, pcr->pci_subsys_card);
	for (i = 0;  pvnd[i].name;  i++) {
	    if (pvnd[i].token == pcr->pci_subsys_vendor) {
	        printf(" (%s", pvnd[i].name);
		foundvendor = 1;
	        break;
	    }
        }

        for (i = 0; pvc[i].VendorID && pvc[i].VendorID != pcr->pci_subsys_vendor; i++)
	    ;
        if (pvc[i].VendorID) {
	    for (j = 0;  pvc[i].Device[j].CardName;  j++) {
	        if (pvc[i].Device[j].SubsystemID == pcr->pci_subsys_card) {
		    printf(" %s)", pvc[i].Device[j].CardName);
		    foundit = 1;
		    break;
	        }
	    }
        }
        if (!foundit) {
	    if (!foundvendor)
		printf(" (");
	    else
		printf(", ");
	    printf("Card unknown)");
	}
	printf("\n");
    }

    if (verbose) {
	if (pcr->pci_status_command)
	    printf("  STATUS    0x%04x  COMMAND 0x%04x\n",
		   pcr->pci_status, pcr->pci_command);
	if (pcr->pci_class_revision)
	    printf("  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
		   pcr->pci_base_class, pcr->pci_sub_class, pcr->pci_prog_if,
		   pcr->pci_rev_id);
	switch (pcr->pci_base_class) {
	case PCI_CLASS_BRIDGE:
	    switch (pcr->pci_sub_class) {
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
print_default_class(pciConfigPtr pcr)
{
    if (pcr->pci_bist_header_latency_cache)
	printf("  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
	       pcr->pci_bist, pcr->pci_header_type, pcr->pci_latency_timer,
	       pcr->pci_cache_line_size);
    if (pcr->pci_base0)
	printf("  BASE0     0x%08x  addr 0x%08x  %s%s%s\n",
	      (int)pcr->pci_base0,(int)(pcr->pci_base0
					& (pcr->pci_base0 & 0x1 ?
					   0xFFFFFFFC : 0xFFFFFFF0)), 
	       pcr->pci_base0 & 0x1 ? "I/O" : "MEM",
	       ((pcr->pci_base0 & 0x9) == 0x8) ? " PREFETCHABLE" :"",
	       ((pcr->pci_base0 & 0x7) == 0x4) ? " 64BIT" : "");
    if (pcr->pci_base1)
	printf("  BASE1     0x%08x  addr 0x%08x  %s%s%s\n",
	       (int)pcr->pci_base1, (int)(pcr->pci_base1
					  & (pcr->pci_base1 & 0x1 ?
					     0xFFFFFFFC : 0xFFFFFFF0)), 
	       pcr->pci_base1 & 0x1 ? "I/O" : "MEM",
	       ((pcr->pci_base1 & 0x9) == 0x8) ? " PREFETCHABLE" :"",
	       ((pcr->pci_base1 & 0x7) == 0x4) ? " 64BIT" : "");
    if (pcr->pci_base2)
	printf("  BASE2     0x%08x  addr 0x%08x  %s%s%s\n",
	       (int)pcr->pci_base2, (int)(pcr->pci_base2
					  & (pcr->pci_base2 & 0x1 ?
					     0xFFFFFFFC : 0xFFFFFFF0)), 
	       pcr->pci_base2 & 0x1 ? "I/O" : "MEM",
	       ((pcr->pci_base2 & 0x9) == 0x8) ? " PREFETCHABLE" :"",
	       ((pcr->pci_base2 & 0x7) == 0x4) ? " 64BIT" : "");
    if (pcr->pci_base3)
	printf("  BASE3     0x%08x  addr 0x%08x  %s%s%s\n",
	       (int)pcr->pci_base3, (int)(pcr->pci_base3
					  & (pcr->pci_base3 & 0x1 ?
					     0xFFFFFFFC : 0xFFFFFFF0)), 
	       pcr->pci_base3 & 0x1 ? "I/O" : "MEM",
	       ((pcr->pci_base3 & 0x9) == 0x8) ? " PREFETCHABLE" :"",
	       ((pcr->pci_base3 & 0x7) == 0x4) ? " 64BIT" : "");
    if (pcr->pci_base4)
	printf("  BASE4     0x%08x  addr 0x%08x  %s%s%s\n",
	       (int)pcr->pci_base4, (int)(pcr->pci_base4
					  & (pcr->pci_base4 & 0x1 ?
					   0xFFFFFFFC : 0xFFFFFFF0)), 
	       pcr->pci_base4 & 0x1 ? "I/O" : "MEM",
	       ((pcr->pci_base4 & 0x9) == 0x8) ? " PREFETCHABLE" :"",
	       ((pcr->pci_base4 & 0x7) == 0x4) ? " 64BIT" : "");
    if (pcr->pci_base5)
	printf("  BASE5     0x%08x  addr 0x%08x  %s%s%s\n",
	       (int)pcr->pci_base5, (int)(pcr->pci_base5
					  & (pcr->pci_base5 & 0x1 ?
					     0xFFFFFFFC : 0xFFFFFFF0)), 
	       pcr->pci_base5 & 0x1 ? "I/O" : "MEM",
	       ((pcr->pci_base5 & 0x9) == 0x8) ? " PREFETCHABLE" :"",
	       ((pcr->pci_base5 & 0x7) == 0x4) ? " 64BIT" : "");
    if (pcr->pci_baserom)
	printf("  BASEROM   0x%08x  addr 0x%08x  %sdecode-enabled\n",
	       (int)pcr->pci_baserom, (int)(pcr->pci_baserom & 0xFFFF8000),
	       pcr->pci_baserom & 0x1 ? "" : "not-");
    if (pcr->pci_max_min_ipin_iline)
	printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
	       pcr->pci_max_lat, pcr->pci_min_gnt, 
	       pcr->pci_int_pin, pcr->pci_int_line);
    if (pcr->pci_user_config)
	printf("  BYTE_0    0x%02x  BYTE_1  0x%02x  BYTE_2  0x%02x  BYTE_3  0x%02x\n",
	       (int)pcr->pci_user_config_0, (int)pcr->pci_user_config_1, 
	       (int)pcr->pci_user_config_2, (int)pcr->pci_user_config_3);
}

#define PCI_B_FAST_B_B 0x80
#define PCI_B_SB_RESET 0x40
#define PCI_B_M_ABORT  0x20
#define PCI_B_VGA_EN   0x08
#define PCI_B_ISA_EN   0x04
#define PCI_B_P_ERR    0x01

void
print_bridge_pci_class(pciConfigPtr pcr)
{
    if (pcr->pci_bist_header_latency_cache)
        printf("  HEADER    0x%02x  LATENCY 0x%02x\n",
	       pcr->pci_header_type, pcr->pci_latency_timer);
    printf("  PRIBUS    0x%02x  SECBUS 0x%02x  SUBBUS 0x%02x  SECLT 0x%02x\n",
           pcr->pci_primary_bus_number, pcr->pci_secondary_bus_number,
	   pcr->pci_subordinate_bus_number, pcr->pci_secondary_latency_timer);
    printf("  IOBASE    0x%02x  IOLIM 0x%02x  SECSTATUS 0x%04x\n",
	   pcr->pci_io_base << 8, (pcr->pci_io_limit << 8) | 0xfff,
	   pcr->pci_secondary_status);
    printf("  NOPREFETCH_MEMBASE 0x%08x  MEMLIM 0x%08x\n",
	   pcr->pci_mem_base << 16, (pcr->pci_mem_limit << 16) | 0xfffff);
    printf("  PREFETCH_MEMBASE   0x%08x  MEMLIM 0x%08x\n",
	   pcr->pci_prefetch_mem_base << 16,
	   (pcr->pci_prefetch_mem_limit << 16) | 0xfffff);
    printf("  %sFAST_B2B %sSEC_BUS_RST %sM_ABRT %sVGA_EN %sISA_EN"
	   " %sPERR_EN\n",
	   (pcr->pci_bridge_control & PCI_B_FAST_B_B) ? "" : "NO_",
	   (pcr->pci_bridge_control & PCI_B_SB_RESET) ? "" : "NO_",
	   (pcr->pci_bridge_control & PCI_B_M_ABORT) ? "" : "NO_",
	   (pcr->pci_bridge_control & PCI_B_VGA_EN) ? "" : "NO_",
	   (pcr->pci_bridge_control & PCI_B_ISA_EN) ? "" : "NO_",
	   (pcr->pci_bridge_control & PCI_B_P_ERR) ? "" : "NO_");
}

void
print_bridge_class(pciConfigPtr pcr)
{
    if (pcr->pci_bist_header_latency_cache)
        printf("  HEADER    0x%02x  LATENCY 0x%02x\n",
	       pcr->pci_header_type, pcr->pci_latency_timer);
}

void
print_mach64(pciConfigPtr pcr)
{
    CARD32 sparse_io = 0;

    printf(" CardVendor 0x%04x card 0x%04x\n",
	pcr->pci_subsys_vendor, pcr->pci_subsys_card);
    if (pcr->pci_status_command)
        printf("  STATUS    0x%04x  COMMAND 0x%04x\n",
            pcr->pci_status, pcr->pci_command);
    if (pcr->pci_class_revision)
        printf("  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
            pcr->pci_base_class, pcr->pci_sub_class, pcr->pci_prog_if, pcr->pci_rev_id);
    if (pcr->pci_bist_header_latency_cache)
        printf("  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
            pcr->pci_bist, pcr->pci_header_type, pcr->pci_latency_timer,
            pcr->pci_cache_line_size);
    if (pcr->pci_base0)
        printf("  APBASE    0x%08x  addr 0x%08x\n",
            (int)pcr->pci_base0, (int)(pcr->pci_base0
				       & (pcr->pci_base0 & 0x1 ?
					  0xFFFFFFFC : 0xFFFFFFF0)));
    if (pcr->pci_base1)
        printf("  BLOCKIO   0x%08x  addr 0x%08x\n",
            (int)pcr->pci_base1, (int)(pcr->pci_base1
				       & (pcr->pci_base1 & 0x1 ?
					  0xFFFFFFFC : 0xFFFFFFF0)));
    if (pcr->pci_base2)
        printf("  REGBASE   0x%08x  addr 0x%08x\n",
            (int)pcr->pci_base2, (int)(pcr->pci_base2
				       & (pcr->pci_base2 & 0x1 ?
					  0xFFFFFFFC : 0xFFFFFFF0)));
    if (pcr->pci_baserom)
        printf("  BASEROM   0x%08x  addr 0x%08x  %sdecode-enabled\n",
	       (int)pcr->pci_baserom, (int)(pcr->pci_baserom & 0xFFFF8000),
				    pcr->pci_baserom & 0x1 ? "" : "not-");
    if (pcr->pci_max_min_ipin_iline)
        printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
            pcr->pci_max_lat, pcr->pci_min_gnt, pcr->pci_int_pin, pcr->pci_int_line);
    switch (pcr->pci_user_config_0 & 0x03) {
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
	    (int)sparse_io, pcr->pci_user_config_0 & 0x04 ? "Block" : "Sparse",
	    pcr->pci_user_config_0 & 0x08 ? "Dis" : "En");
}

void
print_i128(pciConfigPtr pcr)
{
    printf(" CardVendor 0x%04x card 0x%04x\n",
	pcr->pci_subsys_vendor, pcr->pci_subsys_card);
    if (pcr->pci_status_command)
        printf("  STATUS    0x%04x  COMMAND 0x%04x\n",
            pcr->pci_status, pcr->pci_command);
    if (pcr->pci_class_revision)
        printf("  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
            pcr->pci_base_class, pcr->pci_sub_class, pcr->pci_prog_if, pcr->pci_rev_id);
    if (pcr->pci_bist_header_latency_cache)
        printf("  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
            pcr->pci_bist, pcr->pci_header_type, pcr->pci_latency_timer,
            pcr->pci_cache_line_size);
    printf("  MW0_AD    0x%08x  addr 0x%08x  %spre-fetchable\n",
        (int)pcr->pci_base0, (int)(pcr->pci_base0 & 0xFFC00000),
        pcr->pci_base0 & 0x8 ? "" : "not-");
    printf("  MW1_AD    0x%08x  addr 0x%08x  %spre-fetchable\n",
        (int)pcr->pci_base1, (int)(pcr->pci_base1 & 0xFFC00000),
        pcr->pci_base1 & 0x8 ? "" : "not-");
    printf("  XYW_AD(A) 0x%08x  addr 0x%08x\n",
        (int)pcr->pci_base2, (int)(pcr->pci_base2 & 0xFFC00000));
    printf("  XYW_AD(B) 0x%08x  addr 0x%08x\n",
        (int)pcr->pci_base3, (int)(pcr->pci_base3 & 0xFFC00000));
    printf("  RBASE_G   0x%08x  addr 0x%08x\n",
        (int)pcr->pci_base4, (int)(pcr->pci_base4 & 0xFFFF0000));
    printf("  IO        0x%08x  addr 0x%08x\n",
        (int)pcr->pci_base5, (int)(pcr->pci_base5 & 0xFFFFFF00));
    printf("  RBASE_E   0x%08x  addr 0x%08x  %sdecode-enabled\n",
        (int)pcr->pci_baserom, (int)(pcr->pci_baserom & 0xFFFF8000),
        pcr->pci_baserom & 0x1 ? "" : "not-");
    if (pcr->pci_max_min_ipin_iline)
        printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
            pcr->pci_max_lat, pcr->pci_min_gnt, pcr->pci_int_pin, pcr->pci_int_line);
}

void
print_pcibridge(pciConfigPtr pcr)
{
    if (pcr->pci_status_command)
        printf("  STATUS    0x%04x  COMMAND 0x%04x\n",
            pcr->pci_status, pcr->pci_command);
    if (pcr->pci_class_revision)
        printf("  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
            pcr->pci_base_class, pcr->pci_sub_class, pcr->pci_prog_if, pcr->pci_rev_id);
    if (pcr->pci_bist_header_latency_cache)
        printf("  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
            pcr->pci_bist, pcr->pci_header_type, pcr->pci_latency_timer,
            pcr->pci_cache_line_size);
    printf("  PRIBUS    0x%02x  SECBUS 0x%02x  SUBBUS 0x%02x  SECLT 0x%02x\n",
           pcr->pci_primary_bus_number, pcr->pci_secondary_bus_number,
	   pcr->pci_subordinate_bus_number, pcr->pci_secondary_latency_timer);
    printf("  IOBASE    0x%02x  IOLIM 0x%02x  SECSTATUS 0x%04x\n",
	   pcr->pci_io_base << 8, (pcr->pci_io_limit << 8) | 0xfff,
	   pcr->pci_secondary_status);
    printf("  NOPREFETCH_MEMBASE 0x%08x  MEMLIM 0x%08x\n",
	   pcr->pci_mem_base << 16, (pcr->pci_mem_limit << 16) | 0xfffff);
    printf("  PREFETCH_MEMBASE   0x%08x  MEMLIM 0x%08x\n",
	   pcr->pci_prefetch_mem_base << 16,
	   (pcr->pci_prefetch_mem_limit << 16) | 0xfffff);
    printf("  RBASE_E   0x%08x  addr 0x%08x  %sdecode-enabled\n",
        (int)pcr->pci_baserom, (int)(pcr->pci_baserom & 0xFFFF8000),
        pcr->pci_baserom & 0x1 ? "" : "not-");
    if (pcr->pci_max_min_ipin_iline)
        printf("  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
            pcr->pci_max_lat, pcr->pci_min_gnt, pcr->pci_int_pin, pcr->pci_int_line);
}

#include "xf86getpagesize.c"

