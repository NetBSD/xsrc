/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsdResource.c,v 1.2 2000/11/06 19:24:08 dawes Exp $ */

/* Resource information code */

#include "X.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Privstr.h"
#include "xf86Pci.h"
#include "xf86Resources.h"
#define NEED_OS_RAC_PROTOS
#include "xf86_OSlib.h"

#ifdef __alpha__

#include <sys/sysctl.h>

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
     * Only allow the upper half of the pci memory range to be used
     * for allocation. The lower half includes magic regions for DMA.
     * XXX this is not right for XP1000's and similar where we use the 
     * region 0x40000000-0xbfffffff for DMA but this only matters if
     * the bios screws up the pci region mappings.
     */
    RANGE(range,0x80000000,0xffffffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

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

#elif defined(__powerpc__)

resRange PciAvoid[] = {_PCI_AVOID_PC_STYLE, _END};

resPtr
xf86BusAccWindowsFromOS(void)
{
    resPtr ret = NULL;
    resRange range;

    RANGE(range, 0, 0xffffffff, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

    RANGE(range, 0, 0x0000ffff, ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86PciBusAccWindowsFromOS(void)
{
    resPtr ret = NULL;
    resRange range;

    RANGE(range, 0, 0xffffffff, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

    RANGE(range, 0, 0x0000ffff, ResExcIoBlock);
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
