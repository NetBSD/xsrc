/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/hwcursor.h,v 1.1.2.2 1998/08/07 06:40:20 hohndel Exp $ */
/*
 * file hwcursor.h
 */

#ifndef __HWCURSOR_H__
#define __HWCURSOR_H__

/*
 * functions prototypes
 */

void RenditionHWCursorPreInit(ScrnInfoPtr pScreenInfo);
Bool RenditionHWCursorInit(int scrnIndex, ScreenPtr pScreen);
void RenditionHWCursorRelease (ScrnInfoPtr pScreenInfo);

#define HC_SIZE  (64*64*2)/8  /* 1024 */

/* end of __HWCURSOR_H__ */
#endif 

/*
 * end of file hwcursor.h
 */

