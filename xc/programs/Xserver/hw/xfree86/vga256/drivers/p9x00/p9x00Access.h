/*
 * Copyright 1996-1997  Joerg Knura (knura@imst.de)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * JOERG KNURA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00Access.h,v 1.1.2.2 1998/10/21 09:27:35 hohndel Exp $ */

#ifndef P9X00ACCESS_H
#define P9X00ACCESS_H

#include "p9x00Includes.h"
#include "p9x00Probe.h"

typedef struct {
  void  (*enable)(void);
  void  (*disable)(void);
  void  (*write_cfg)(CARD8,CARD8,CARD8);
  void  (*write_icd)(CARD8);
  void  (*write_dac)(CARD8,CARD8);
  CARD8 (*read_dac)(CARD8);
} p9x00function_type;

#ifndef P9X00ACCESS_C

  extern pointer p9x00LinearBase;
  extern p9x00function_type p9x00function;
  
#else /* P9X00ACCESS_C */

  pointer p9x00LinearBase;
  p9x00function_type p9x00function;
  
#endif /* P9X00ACCESS_C */                     


#define P9X00_ENABLE   p9x00function.enable
#define P9X00_DISABLE  p9x00function.disable
#define P9X00_WRITE    p9x00function.write_cfg
#define P9X00_WRITEDAC p9x00function.write_dac
#define P9X00_WRITEICD p9x00function.write_icd
#define P9X00_READDAC  p9x00function.read_dac

#define P9X00_MAPALL p9x00LinearBase=          \
                     xf86MapVidMem (           \
                       vga256InfoRec.scrnIndex,\
                       LINEAR_REGION,          \
                       (pointer)P9X_CFG_BASE,  \
                       P9X_CFG_ALL             \
                     )

#define P9X00_UNMAPALL xf86UnMapVidMem (         \
                         vga256InfoRec.scrnIndex,\
                         LINEAR_REGION,          \
                         p9x00LinearBase,  	 \
                         P9X_CFG_ALL             \
                       )

#define P9X00_ENABLEPORTS xf86EnableIOPorts (       \
                            vga256InfoRec.scrnIndex \
                          )
                       
#define P9X00_DISABLEPORTS xf86DisableIOPorts (      \
                             vga256InfoRec.scrnIndex \
                           )

static void p9000enable(void);
static void p9000disable(void);
static void p9100enable(void);
static void p9100disable(void);
static void p9000write_dac(CARD8,CARD8);
static CARD8 p9000read_dac(CARD8);
static void p9100write_dac(CARD8,CARD8);
static CARD8 p9100read_dac(CARD8);
static void p9000write_icd(CARD8);
static void p9100write_icd(CARD8);
static void p9100writepci(CARD8,CARD8,CARD8);
static void p9000Init_DACRegs(void);

void p9100writevl(CARD8,CARD8,CARD8);
CARD8 p9100readvl(CARD8);
void p9x00Init_Access(void);

#define P9X_SET_PIXCLK   1
#define P9X_SET_MEMCLK   2
#define P9X_SET_VRAMPORT 3
#define P9X_SET_VGAPORT  4

#endif /* P9X00ACCESS_H */
