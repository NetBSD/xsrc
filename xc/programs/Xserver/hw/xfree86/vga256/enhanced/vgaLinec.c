/*
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/enhanced/vgaLinec.c,v 3.0 1996/12/09 11:54:31 dawes Exp $ */

#include "X.h"
#include "misc.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"

#include "vga.h"
#include "cfb.h"

/* vgabres.s */
void fastvga256BresS(
    int            alu,
    unsigned long  and,
    unsigned long  xor,
    unsigned long *addrl,
    int            nlwidth,
    register int   signdx,
    int            signdy,
    int            axis,
    int            x,
    int            y,
    register int   e,
    register int   e1,
    int            e2,
    int            len
)
{
    cfbBresS(alu,and,xor,addrl,nlwidth,signdx,signdy,axis,x,y,e,e1,e2,len);
}

/* vgalineH.s */
int fastvga256HorzS(
    int                     alu,
    unsigned long           and,
    register unsigned long  xor,
    register unsigned long *addrl,
    int                     nlwidth,
    int                     x,
    int                     y,
    int                     len
)
{
    cfbHorzS(alu,and,xor,addrl,nlwidth,x,y,len);
}

/* vgalineV.s */
int fastvga256VertS(
    int            alu,
    unsigned long  and,
    unsigned long  xor,
    unsigned long *addrl,
    int            nlwidth,
    int            x,
    int            y,
    register int   len
)
{
    cfbVertS(alu,and,xor,addrl,nlwidth,x,y,len);
}
