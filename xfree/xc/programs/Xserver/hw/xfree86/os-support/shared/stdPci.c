/* $XFree86: $ */

#include "X.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Privstr.h"
#include "xf86Pci.h"
#define NEED_OS_RAC_PROTOS
#include "xf86_OSlib.h"

#ifndef HAVE_PCI_SIZE_FUNC
#define xf86StdGetPciSizeFromOS xf86GetPciSizeFromOS
#endif

Bool
xf86StdGetPciSizeFromOS(PCITAG tag, int index, int* bits)
{
    return FALSE;
}
