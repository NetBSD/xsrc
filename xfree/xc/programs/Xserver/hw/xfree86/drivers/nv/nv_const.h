/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_const.h,v 1.4 2000/10/24 22:45:08 dawes Exp $ */

#ifndef __NV_CONST_H__
#define __NV_CONST_H__

#define VERSION 4000
#define NV_NAME "NV"
#define NV_DRIVER_NAME "nv"
#define NV_MAJOR_VERSION 1
#define NV_MINOR_VERSION 0
#define NV_PATCHLEVEL 0

#define NV_USE_FB

#ifdef DEBUG_PRINT
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

#endif /* __NV_CONST_H__ */
          
