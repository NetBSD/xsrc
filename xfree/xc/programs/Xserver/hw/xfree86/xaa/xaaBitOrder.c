/* $XFree86: xc/programs/Xserver/hw/xfree86/xaa/xaaBitOrder.c,v 1.6 2000/06/13 02:51:24 mvojkovi Exp $ */

#include "xf86.h"
#include "xf86_ansic.h"
#ifndef RAMDAC_MODULE
#include "xaa.h"
#include "xaalocal.h"
#else
#include "xf86CursorPriv.h"
#endif


CARD32
XAAReverseBitOrder(CARD32 v)
{
 return (((0x01010101 & v) << 7) | ((0x02020202 & v) << 5) | 
         ((0x04040404 & v) << 3) | ((0x08080808 & v) << 1) | 
         ((0x10101010 & v) >> 1) | ((0x20202020 & v) >> 3) | 
         ((0x40404040 & v) >> 5) | ((0x80808080 & v) >> 7));
}
