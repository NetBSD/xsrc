/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_const.h,v 1.6 2001/12/07 00:09:55 mvojkovi Exp $ */

#ifndef __NV_CONST_H__
#define __NV_CONST_H__

#define NV_VERSION 4000
#define NV_NAME "NVXBOX"
#define NV_DRIVER_NAME "nvxbox"
#define NV_MAJOR_VERSION 1
#define NV_MINOR_VERSION 0
#define NV_PATCHLEVEL 0

#define DEBUG_PRINT 1

#ifdef DEBUG_PRINT
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

#endif /* __NV_CONST_H__ */
          
