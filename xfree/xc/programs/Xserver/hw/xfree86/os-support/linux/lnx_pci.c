/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/lnx_pci.c,v 3.5 2000/10/17 16:53:20 tsi Exp $ */

#include <stdio.h>
#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#define XF86_OS_PRIVS
#include "xf86_OSproc.h"
#include "xf86Pci.h"

Bool
xf86GetPciSizeFromOS(PCITAG tag, int index, int* bits)
{
    FILE *file;
    char c[0x100];
    char *res;
    int bus, devfn, dev, fn;
    unsigned int size[7];
    unsigned int num;
    int Size;

    if (index > 7)
	return FALSE;
    
    if (!(file = fopen("/proc/bus/pci/devices","r")))
	return FALSE;
    do {
	res = fgets(c,0xff,file);
	if (res) {
	    num = sscanf(res,"%02x%02x\t%*04x%*04x\t%*x"
			     "\t%*x\t%*x\t%*x\t%*x\t%*x\t%*x\t%*x"
			     "\t%x\t%x\t%x\t%x\t%x\t%x\t%x",
			 &bus,&devfn,&size[0],&size[1],&size[2],&size[3],
			 &size[4],&size[5],&size[6]);
	    if (num != 9) {  /* apparantly not 2.3 style */ 
		fclose(file);
		return FALSE;
	    }
	    dev = devfn >> 3;
	    fn = devfn & 0x7;
	    if (tag == pciTag(bus,dev,fn)) {
		*bits = 0;
		if (size[index] != 0) {
		    Size = size[index] - 1;
		    while (Size & 0x01) {
			Size = Size >> 1;
			(*bits)++;
		    }
		}
		fclose(file);
		return TRUE;
	    }
	}
    } while (res);

    fclose(file);
    return FALSE;
}
