/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/shared/xf86Axp.h,v 1.2 2000/11/06 21:57:11 dawes Exp $ */

#ifndef _XF86_AXP_H_
#define _XF86_AXP_H_

typedef enum {
  SYS_NONE,
  TSUNAMI,
  LCA,
  APECS,
  T2,
  T2_GAMMA,
  CIA,
  MCPCIA,
  JENSEN,
  POLARIS,
  PYXIS,
  PYXIS_CIA,
  IRONGATE
} axpDevice;
  
typedef struct 
 { char* sysName; 
   char* sysVari; 
   char* cpu; 
   axpDevice sys; }
AXP;

typedef struct {
  axpDevice id;
  unsigned long hae_thresh;
  unsigned long hae_mask;
  unsigned long size;
} axpParams;

extern axpParams xf86AXPParams[];

#endif

