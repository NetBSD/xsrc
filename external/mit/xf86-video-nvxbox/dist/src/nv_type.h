/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv_type.h,v 1.29 2001/12/07 00:09:56 mvojkovi Exp $ */

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

void NVXRefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void NVXRefreshArea8(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void NVXRefreshArea16(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void NVXRefreshArea32(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void NVXPointerMoved(int index, int x, int y);

int RivaGetConfig(NVPtr);

#define PCI_CHIP_XBOX                  0x02A0
#define NV_CHIP_XBOX    ((PCI_VENDOR_NVIDIA  << 16) | PCI_CHIP_XBOX)

#endif /* __NV_STRUCT_H__ */
