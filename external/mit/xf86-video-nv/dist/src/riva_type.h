#ifndef __Riva_STRUCT_H__
#define __Riva_STRUCT_H__

#include "riva_hw.h"
#include "colormapst.h"
#include "vgaHW.h"
#ifdef HAVE_XAA_H
#include "xaa.h"
#endif
#include "xf86Cursor.h"
#include "xf86int10.h"


#define RIVA_BITMASK(t,b) (((unsigned)(1U << (((t)-(b)+1)))-1)  << (b))
#define RIVA_MASKEXPAND(mask) RIVA_BITMASK(1?mask,0?mask)
#define RIVA_SetBF(mask,value) ((value) << (0?mask))
#define RIVA_GetBF(var,mask) (((unsigned)((var) & RIVA_MASKEXPAND(mask))) >> (0?mask) )
#define RIVA_SetBitField(value,from,to) RIVA_SetBF(to, RIVA_GetBF(value,from))
#define RIVA_SetBit(n) (1<<(n))
#define RIVA_Set8Bits(value) ((value)&0xff)

typedef RIVA_HW_STATE* RivaRegPtr;

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
    void        (*Save)(ScrnInfoPtr, vgaRegPtr, RivaRegPtr, Bool);
    void        (*Restore)(ScrnInfoPtr, vgaRegPtr, RivaRegPtr, Bool);
    Bool        (*ModeInit)(ScrnInfoPtr, DisplayModePtr);
} RivaRamdacRec, *RivaRamdacPtr;

typedef struct {
    int bitsPerPixel;
    int depth;
    int displayWidth;
    rgb weight;
    DisplayModePtr mode;
} RivaFBLayout;

typedef struct {
    RIVA_HW_INST        riva;
    RIVA_HW_STATE       SavedReg;
    RIVA_HW_STATE       ModeReg;
    EntityInfoPtr       pEnt;
#ifdef XSERVER_LIBPCIACCESS
    struct pci_device  *PciInfo;
#else
    pciVideoPtr         PciInfo;
    PCITAG              PciTag;
    xf86AccessRec       Access;
#endif
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
    RivaRamdacRec         Dac;
    Bool                NoAccel;
    Bool                HWCursor;
    Bool                ShowCache;
    Bool                ShadowFB;
    unsigned char *     ShadowPtr;
    int                 ShadowPitch;
    int                 MinClock;
    int                 MaxClock;
#ifdef HAVE_XAA_H
    XAAInfoRecPtr       AccelInfoRec;
#endif
    xf86CursorInfoPtr   CursorInfoRec;
    DGAModePtr          DGAModes;
    int                 numDGAModes;
    Bool                DGAactive;
    int                 DGAViewportStatus;
    void                (*Save)(ScrnInfoPtr, vgaRegPtr, RivaRegPtr, Bool);
    void                (*Restore)(ScrnInfoPtr, vgaRegPtr, RivaRegPtr, Bool);
    Bool                (*ModeInit)(ScrnInfoPtr, DisplayModePtr);
    void		(*PointerMoved)(SCRN_ARG_TYPE arg, int x, int y);
    CloseScreenProcPtr  CloseScreen;
    Bool                FBDev;
    /* Color expansion */
    unsigned char       *expandBuffer;
    unsigned char       *expandFifo;
    int                 expandWidth;
    int                 expandRows;
    CARD32		FgColor;
    CARD32		BgColor;
    int			Rotate;
    RivaFBLayout		CurrentLayout;
    /* Cursor */
    CARD32              curFg, curBg;
    CARD32              curImage[64];
    /* Misc flags */
    unsigned int        opaqueMonochrome;
    int                 currentRop;
    /* I2C / DDC */
    I2CBusPtr           I2C;
    xf86Int10InfoPtr    pInt;
    OptionInfoPtr	Options;
    unsigned char       DDCBase;
} RivaRec, *RivaPtr;

#define RivaPTR(p) ((RivaPtr)((p)->driverPrivate))

void RivaRefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void RivaRefreshArea8(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void RivaRefreshArea16(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void RivaRefreshArea32(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void RivaPointerMoved(SCRN_ARG_TYPE arg, int x, int y);

int RivaGetConfig(RivaPtr);

#endif /* __Riva_STRUCT_H__ */
