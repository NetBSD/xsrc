/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/shared/stdResource.c,v 1.15 2000/04/23 19:27:03 tsi Exp $ */

/* Standard resource information code */

#include "X.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Privstr.h"
#include "xf86Pci.h"
#define NEED_OS_RAC_PROTOS
#include "xf86_OSlib.h"
#include "xf86Resources.h"

#ifdef USESTDRES
#define xf86StdBusAccWindowsFromOS xf86BusAccWindowsFromOS
#define xf86StdAccResFromOS xf86AccResFromOS
#define xf86StdPciBusAccWindowsFromOS xf86PciBusAccWindowsFromOS
#define xf86StdIsaBusAccWindowsFromOS xf86IsaBusAccWindowsFromOS

resRange PciAvoid[] = {_PCI_AVOID_PC_STYLE, _END};
#endif

resPtr
xf86StdBusAccWindowsFromOS(void)
{
    /* Fallback is to allow addressing of all memory space */
    resPtr ret = NULL;
    resRange range;

    RANGE(range,0,0xffffffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

    /* Fallback is to allow addressing of all I/O space */
    RANGE(range,0,0xffff,ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86StdPciBusAccWindowsFromOS(void)
{
    /* Fallback is to allow addressing of all memory space */
    resPtr ret = NULL;
    resRange range;

    RANGE(range,0,0xffffffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

    /* Fallback is to allow addressing of all I/O space */
    RANGE(range,0,0xffff,ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86StdIsaBusAccWindowsFromOS(void)
{
    /* Fallback is to allow addressing of all memory space */
    resPtr ret = NULL;
    resRange range;

    RANGE(range,0,0xffffffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);

    /* Fallback is to allow addressing of all I/O space */
    RANGE(range,0,0xffff,ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);
    return ret;
}

resPtr
xf86StdAccResFromOS(resPtr ret)
{
    resRange range;

    /*
     * Fallback is to claim the following areas:
     *
     * 0x00000000 - 0x0009ffff	low 640k host memory
     * 0x000C0000 - 0x000EFFFF  location of VGA and other extensions ROMS
     * 0x000f0000 - 0x000fffff	system BIOS
     * 0x00100000 - 0x3fffffff	low 1G - 1MB host memory
     * 0xfec00000 - 0xfecfffff	default I/O APIC config space
     * 0xfee00000 - 0xfeefffff	default Local APIC config space
     * 0xffe00000 - 0xffffffff	high BIOS area (should this be included?)
     *
     * reference: Intel 440BX AGP specs
     *
     * The two APIC spaces appear to be BX-specific and should be dealt with
     * elsewhere.
     */

    /* Fallback is to claim 0x0 - 0x9ffff and 0x100000 - 0x7fffffff */
    RANGE(range,0,0x9ffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range,0xc0000,0xeffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range,0xf0000,0xfffff,ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range,0x100000,0x3fffffff,ResExcMemBlock | ResBios | ResEstimated);
    ret = xf86AddResToList(ret, &range, -1);
#if 0
    RANGE(range,0xfec00000,0xfecfffff,ResExcMemBlock | ResBios);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range,0xfee00000,0xfeefffff,ResExcMemBlock | ResBios);
    ret = xf86AddResToList(ret, &range, -1);
#endif
    RANGE(range,0xffe00000,0xffffffff,ResExcMemBlock | ResBios);
    ret = xf86AddResToList(ret, &range, -1);

    /* Fallback is to claim well known ports in the 0x0 - 0x3ff range */
    /* Possibly should claim some of them as sparse ranges */

    RANGE(range,0,0x1ff,ResExcIoBlock | ResEstimated);
    ret = xf86AddResToList(ret, &range, -1);
    /* XXX add others */
    return ret;
}
