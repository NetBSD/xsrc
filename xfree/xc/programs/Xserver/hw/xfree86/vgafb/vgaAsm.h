/* $XFree86: xc/programs/Xserver/hw/xfree86/vgafb/vgaAsm.h,v 1.1.2.1 1997/07/16 10:36:46 hohndel Exp $ */





/* $XConsortium: vgaAsm.h /main/6 1996/02/21 18:10:00 kaleb $ */

/* Definitions for VGA bank assembler routines */

#ifdef CSRG_BASED
#define VGABASE CONST(0xFF000000)
#else
#define VGABASE CONST(0xF0000000)
#endif

