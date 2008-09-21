/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_proto.h,v 1.6 2001/03/28 01:17:43 mvojkovi Exp $ */

#ifndef __NV_PROTO_H__
#define __NV_PROTO_H__

/* in nv_driver.c */
Bool    NVXSwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
void    NVXAdjustFrame(int scrnIndex, int x, int y, int flags);

/* in nv_dac.c */
void    NVXRamdacInit(ScrnInfoPtr pScrn);
Bool    NVXDACInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
void    NVXDACSave(ScrnInfoPtr pScrn, vgaRegPtr vgaReg,
                  NVRegPtr nvReg, Bool saveFonts);
void    NVXDACRestore(ScrnInfoPtr pScrn, vgaRegPtr vgaReg,
                     NVRegPtr nvReg, Bool restoreFonts);
void    NVXDACLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
                         LOCO *colors, VisualPtr pVisual );

/* in nv_video.c */
void NVXInitVideo(ScreenPtr);

/* in nv_setup.c */
void    NVX1Setup(ScrnInfoPtr pScrn);
void    NVX3Setup(ScrnInfoPtr pScrn);
void    NVX4Setup(ScrnInfoPtr pScrn);
void    NVX10Setup(ScrnInfoPtr pScrn);
void    NVX20Setup(ScrnInfoPtr pScrn);

/* in nv_cursor.c */
Bool    NVXCursorInit(ScreenPtr pScreen);

/* in nv_xaa.c */
Bool    NVXAccelInit(ScreenPtr pScreen);
void    NVXSync(ScrnInfoPtr pScrn);
void    NVXResetGraphics(ScrnInfoPtr pScrn);

/* in nv_dga.c */
Bool    NVXDGAInit(ScreenPtr pScreen);

#endif /* __NV_PROTO_H__ */

