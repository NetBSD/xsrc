/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/misc/xf86_IlHack.c,v 3.3 1996/02/04 09:10:13 dawes Exp $ */
/*
 * This file is an incredible crock to get the normally-inline functions
 * built into the server so that things can be debugged properly.
 *
 * Note: this doesn't work when using a compiler other than GCC.
 */
/* $XConsortium: xf86_IlHack.c /main/3 1995/11/13 06:06:00 kaleb $ */


#define static /**/
#define __inline__ /**/
#undef NO_INLINE
#include "compiler.h"
