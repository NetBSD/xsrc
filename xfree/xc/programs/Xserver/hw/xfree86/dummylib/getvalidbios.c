/* $XFree86: xc/programs/Xserver/hw/xfree86/dummylib/getvalidbios.c,v 1.2 2000/12/07 15:43:45 tsi Exp $ */

#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"

/*
 * Utility functions required by libxf86_os. 
 */

memType
getValidBIOSBase(PCITAG tag, int *num)
{
    return 0;
}
