/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/lnxResource.c,v 3.11 2000/10/17 16:53:20 tsi Exp $ */

/* Resource information code */

#include "X.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Privstr.h"
#include "xf86Pci.h"
#include "xf86Resources.h"
#define NEED_OS_RAC_PROTOS
#include "xf86_OSlib.h"
#include "lnx.h"

#ifdef __alpha__
resRange PciAvoid[] = {_PCI_AVOID_PC_STYLE, _END};

resPtr
xf86BusAccWindowsFromOS(void)
{
    resPtr ret = NULL;
    resRange range;

    RANGE(range,0,0xffffffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

    RANGE(range,0,0xffffffff,ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86PciBusAccWindowsFromOS(void)
{
    resPtr ret = NULL;
    resRange range;
    /*
     * On the Alpha the first 16MB of every 128 Mb segment in
     * sparse address space are an image of the ISA bus range
     */
    if (_bus_base_sparse()) {
	RANGE(range,0,0x07ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x09000000,0x0fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x11000000,0x17ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x19000000,0x1fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x21000000,0x27ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x29000000,0x2fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x31000000,0x37ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x39000000,0x3fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x41000000,0x47ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x49000000,0x4fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x51000000,0x57ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x59000000,0x5fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x61000000,0x67ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x69000000,0x6fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x71000000,0x77ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x79000000,0x7fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x81000000,0x87ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x89000000,0x8fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x91000000,0x97ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0x99000000,0x9fffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xA1000000,0xa7ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xA9000000,0xafffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xB1000000,0xb7ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xB9000000,0xbfffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xC1000000,0xc7ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xC9000000,0xcfffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xD1000000,0xd7ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xD9000000,0xdfffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xE1000000,0xe7ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xE9000000,0xefffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xF1000000,0xf7ffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
	RANGE(range,0xF9000000,0xffffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
    } else {
      /* Some drivers choke if a PCI base address is set to 0 */
	RANGE(range,1,0xffffffff,ResExcMemBlock);
	ret = xf86AddResToList(ret, &range, -1);
    }
    RANGE(range,0,0xffffffff,ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86IsaBusAccWindowsFromOS(void)
{
    resPtr ret = NULL;
    resRange range;

    RANGE(range,0,0xffffffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

    RANGE(range,0,0xffffffff,ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86AccResFromOS(resPtr ret)
{
    resRange range;

    /*
     * Fallback is to claim the following areas:
     *
     * 0x000C0000 - 0x000EFFFF  location of VGA and other extensions ROMS
     */

    RANGE(range,0xc0000,0xeffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

    /* Fallback is to claim well known ports in the 0x0 - 0x3ff range */
    /* Possibly should claim some of them as sparse ranges */

    RANGE(range,0,0x1ff,ResExcIoBlock | ResEstimated);
    ret = xf86AddResToList(ret, &range, -1);
    /* XXX add others */
    return ret;
}

#elif defined(__powerpc__) || defined(__sparc__) || defined(__mips__)

 /* XXX this isn't exactly correct but it will get the server working 
  * for now until we get something better.
  */
  
#ifdef __sparc__
resRange PciAvoid[] = {_END};
#else
resRange PciAvoid[] = {_PCI_AVOID_PC_STYLE, _END};
#endif

resPtr
xf86BusAccWindowsFromOS(void)
{
    resPtr ret = NULL;
    resRange range;

    RANGE(range,0,0xffffffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

#ifdef __sparc__
    RANGE(range,0,0x00ffffff,ResExcIoBlock);
#else
    RANGE(range,0,0x0000ffff,ResExcIoBlock);
#endif
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86PciBusAccWindowsFromOS(void)
{
    resPtr ret = NULL;
    resRange range;

    RANGE(range,0,0xffffffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

#ifdef __sparc__
    RANGE(range,0,0x00ffffff,ResExcIoBlock);
#else
    RANGE(range,0,0x0000ffff,ResExcIoBlock);
#endif
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86IsaBusAccWindowsFromOS(void)
{
    resPtr ret = NULL;
    resRange range;

    RANGE(range,0,0xffffffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

#ifdef __sparc__
    RANGE(range,0,0x00ffffff,ResExcIoBlock);
#else
    RANGE(range,0,0x0000ffff,ResExcIoBlock);
#endif
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86AccResFromOS(resPtr ret)
{
    return ret;
}

#else

#error : Put your platform dependent code here!!

#endif
