/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86ExtInit.h,v 3.5 1996/02/18 03:42:48 dawes Exp $ */





/* $XConsortium: xf86ExtInit.h /main/6 1996/02/21 17:38:17 kaleb $ */

/* Hack to avoid multiple versions of dixfonts in vga{2,16}misc.o */
#ifndef LBX
extern void LbxFreeFontTag(
#if NeedFunctionPrototypes
	void
#endif
	);
void LbxFreeFontTag() {}
#endif
