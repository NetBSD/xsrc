/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/tdfx/tdfx_dri.h,v 1.3 2000/02/15 07:13:43 martin Exp $ */

#ifndef _TDFX_DRI_
#define _TDFX_DRI_

#include <xf86drm.h>

typedef struct {
  drmHandle regs;
  drmSize regsSize;
  drmAddress regsMap;
  int deviceID;
  int width;
  int height;
  int mem;
  int cpp;
  int stride;
  int fifoOffset;
  int fifoSize;
  int fbOffset;
  int backOffset;
  int depthOffset;
  int textureOffset;
  int textureSize;
} TDFXDRIRec, *TDFXDRIPtr;

#endif
