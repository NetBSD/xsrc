/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/vga/vgaAsm.h,v 3.3 1996/02/04 09:14:53 dawes Exp $ */





/* $XConsortium: vgaAsm.h /main/5 1995/11/13 11:43:05 kaleb $ */

/* Definitions for VGA bank assembler routines */

#ifdef CSRG_BASED
#define VGABASE CONST(0xFF000000)
#else
#define VGABASE CONST(0xF0000000)
#endif

