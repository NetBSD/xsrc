/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_type.h,v 1.24 2001/05/09 00:39:51 mvojkovi Exp $ */

#ifndef __NV_STRUCT_H__
#define __NV_STRUCT_H__

#include "riva_hw.h"
#include "colormapst.h"
#include "vgaHW.h"
#include "xaa.h"
#include "xf86Cursor.h"
#include "xf86int10.h"

#define SetBitField(value,from,to) SetBF(to, GetBF(value,from))
#define SetBit(n) (1<<(n))
#define Set8Bits(value) ((value)&0xff)

#define MAX_CURS            32

typedef RIVA_HW_STATE* NVRegPtr;

typedef struct {
    Bool        isHwCursor;
    int         CursorMaxWidth;
    int         CursorMaxHeight;
    int         CursorFlags;
    int         CursorOffscreenMemSize;
    Bool        (*UseHWCursor)(ScreenPtr, CursorPtr);
    void        (*LoadCursorImage)(ScrnInfoPtr, unsigned char*);
    void        (*ShowCursor)(ScrnInfoPtr);
    void        (*HideCursor)(ScrnInfoPtr);
    void        (*SetCursorPosition)(ScrnInfoPtr, int, int);
    void        (*SetCursorColors)(ScrnInfoPtr, int, int);
    long        maxPixelClock;
    void        (*LoadPalette)(ScrnInfoPtr, int, int*, LOCO*, VisualPtr);
    void        (*PreInit)(ScrnInfoPtr);
    void        (*Save)(ScrnInfoPtr, vgaRegPtr, NVRegPtr, Bool);
    void        (*Restore)(ScrnInfoPtr, vgaRegPtr, NVRegPtr, Bool);
    Bool        (*ModeInit)(ScrnInfoPtr, DisplayModePtr);
} NVRamdacRec, *NVRamdacPtr;

typedef struct {
    int bitsPerPixel;
    int depth;
    int displayWidth;
    rgb weight;
    DisplayModePtr mode;
} NVFBLayout;

typedef struct {
    RIVA_HW_INST        riva;
    RIVA_HW_STATE       SavedReg;
    RIVA_HW_STATE       ModeReg;
    EntityInfoPtr       pEnt;
    pciVideoPtr         PciInfo;
    PCITAG              PciTag;
    xf86AccessRec       Access;
    int                 Chipset;
    int                 ChipRev;
    Bool                Primary;
    CARD32              IOAddress;
    unsigned long       FbAddress;
    int                 FbBaseReg;
    unsigned char *     IOBase;
    unsigned char *     FbBase;
    unsigned char *     FbStart;
    long                FbMapSize;
    long                FbUsableSize;
    NVRamdacRec         Dac;
    Bool                NoAccel;
    Bool                HWCursor;
    Bool                ShowCache;
    Bool                ShadowFB;
    unsigned char *     ShadowPtr;
    int                 ShadowPitch;
    int                 MinClock;
    int                 MaxClock;
    XAAInfoRecPtr       AccelInfoRec;
    xf86CursorInfoPtr   CursorInfoRec;
    DGAModePtr          DGAModes;
    int                 numDGAModes;
    Bool                DGAactive;
    int                 DGAViewportStatus;
    void                (*PreInit)(ScrnInfoPtr pScrn);
    void                (*Save)(ScrnInfoPtr, vgaRegPtr, NVRegPtr, Bool);
    void                (*Restore)(ScrnInfoPtr, vgaRegPtr, NVRegPtr, Bool);
    Bool                (*ModeInit)(ScrnInfoPtr, DisplayModePtr);
    void		(*PointerMoved)(int index, int x, int y);
    ScreenBlockHandlerProcPtr BlockHandler;
    CloseScreenProcPtr  CloseScreen;
    Bool                FBDev;
    /* Color expansion */
    Bool                useFifo;
    unsigned char       *expandBuffer;
    unsigned char       *expandFifo;
    int                 expandWidth;
    int                 expandRows;
    CARD32		FgColor;
    CARD32		BgColor;
    int			Rotate;
    NVFBLayout		CurrentLayout;
    GCPtr		CurrentGC;
#ifdef NV_USE_FB
    DrawablePtr		CurrentDrawable;
#endif    
    /* Cursor */
    unsigned short      curFg, curBg;
    unsigned int        curImage[MAX_CURS*2];
    /* Misc flags */
    unsigned int        opaqueMonochrome;
    int                 currentRop;
    /* I2C / DDC */
    unsigned int        (*ddc1Read)(ScrnInfoPtr);
    void                (*DDC1SetSpeed)(ScrnInfoPtr, xf86ddcSpeed);
    Bool                (*i2cInit)(ScrnInfoPtr);
    I2CBusPtr           I2C;
    xf86Int10InfoPtr    pInt;
    void		(*VideoTimerCallback)(ScrnInfoPtr, Time);
    XF86VideoAdaptorPtr	overlayAdaptor;
    int			videoKey;
    Bool		FlatPanel;
    OptionInfoPtr	Options;
} NVRec, *NVPtr;

#define NVPTR(p) ((NVPtr)((p)->driverPrivate))

void NVRefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void NVRefreshArea8(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void NVRefreshArea16(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void NVRefreshArea32(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void NVPointerMoved(int index, int x, int y);

#define NV_CHIP_RIVA128    ((PCI_VENDOR_NVIDIA_SGS << 16)| PCI_CHIP_RIVA128)
#define NV_CHIP_TNT        ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_TNT)
#define NV_CHIP_TNT2       ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_TNT2)
#define NV_CHIP_TNT2_A     ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_TNT2_A)
#define NV_CHIP_TNT2_B     ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_TNT2_B)
#define NV_CHIP_UTNT2      ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_UTNT2)
#define NV_CHIP_VTNT2      ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_VTNT2)
#define NV_CHIP_UVTNT2     ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_UVTNT2)
#define NV_CHIP_ITNT2      ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_ITNT2)
#define NV_CHIP_GEFORCE256 ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_GEFORCE256)
#define NV_CHIP_GEFORCEDDR ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_GEFORCEDDR)
#define NV_CHIP_QUADRO     ((PCI_VENDOR_NVIDIA     << 16)| PCI_CHIP_QUADRO)
#define NV_CHIP_GEFORCE2MX      ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE2MX)
#define NV_CHIP_GEFORCE2MXDDR    ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE2MXDDR)
#define NV_CHIP_QUADRO2MXR      ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_QUADRO2MXR)
#define NV_CHIP_GEFORCE2GO      ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE2GO)
#define NV_CHIP_GEFORCE2GTS     ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE2GTS)
#define NV_CHIP_GEFORCE2GTS_1   ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE2GTS_1)
#define NV_CHIP_GEFORCE2ULTRA   ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE2ULTRA)
#define NV_CHIP_QUADRO2PRO      ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_QUADRO2PRO)
#define NV_CHIP_GEFORCE3     ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE3)
#define NV_CHIP_GEFORCE3_1   ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE3_1)
#define NV_CHIP_GEFORCE3_2   ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE3_2)
#define NV_CHIP_GEFORCE3_3   ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_GEFORCE3_3)

#endif /* __NV_STRUCT_H__ */
