/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_proto.h,v 1.3 1999/11/12 02:12:40 mvojkovi Exp $ */

#ifndef __NV_PROTO_H__
#define __NV_PROTO_H__

/* in nv_driver.c */
Bool    NVSwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
void    NVAdjustFrame(int scrnIndex, int x, int y, int flags);

/* in nv_dac.c */
void    NVRamdacInit(ScrnInfoPtr pScrn);
Bool    NVDACInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
void    NVDACSave(ScrnInfoPtr pScrn, vgaRegPtr vgaReg,
                  NVRegPtr nvReg, Bool saveFonts);
void    NVDACRestore(ScrnInfoPtr pScrn, vgaRegPtr vgaReg,
                     NVRegPtr nvReg, Bool restoreFonts);
void    NVDACLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
                         LOCO *colors, VisualPtr pVisual );

/* in nv_setup.c */
void    RivaEnterLeave(ScrnInfoPtr pScrn, Bool enter);
void    NV1Setup(ScrnInfoPtr pScrn);
void    NV3Setup(ScrnInfoPtr pScrn);
void    NV4Setup(ScrnInfoPtr pScrn);
void    NV10Setup(ScrnInfoPtr pScrn);

/* in nv_cursor.c */
Bool    NVCursorInit(ScreenPtr pScreen);

/* in nv_xaa.c */
Bool    NVAccelInit(ScreenPtr pScreen);
void    NVSync(ScrnInfoPtr pScrn);

/* in nv_dga.c */
Bool    NVDGAInit(ScreenPtr pScreen);

#endif /* __NV_PROTO_H__ */

