/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/cirrus/cir.h,v 1.17 2000/12/06 15:35:15 eich Exp $ */

/* (c) Itai Nahshon */

#ifndef CIR_H
#define CIR_H

#include "xf86Cursor.h"
#include "xaa.h"
#include "xf86i2c.h"

/* Card-specific driver information */
#define CIRPTR(p) ((CirPtr)((p)->driverPrivate))
struct lgRec;
struct alpRec;

typedef enum {
  LCD_NONE, LCD_DUAL_MONO, LCD_UNKNOWN, LCD_DSTN, LCD_TFT } LCDType;

typedef struct {
	ScrnInfoPtr			pScrn;
        CARD32              properties;
        pciVideoPtr			PciInfo;
	PCITAG				PciTag;
    union {
	struct lgRec *lg;
	struct alpRec *alp;
    } chip;
        EntityInfoPtr		pEnt;
	int					Chipset;
	int					ChipRev;
	int					Rounding;
	int					BppShift;
	Bool				HasFBitBlt;
	CARD32				IOAddress;
	CARD32				FbAddress;
	unsigned char *		IOBase;
	unsigned char *		FbBase;
	long				FbMapSize;
	long				IoMapSize;
	int					MinClock;
	int   				MaxClock;
	Bool				NoAccel;
	Bool				HWCursor;
	Bool				UseMMIO;
	XAAInfoRecPtr		AccelInfoRec;
	xf86CursorInfoPtr	CursorInfoRec;
#if 0
	DGAInfoPtr			DGAInfo;
#endif
	I2CBusPtr			I2CPtr1;
	I2CBusPtr			I2CPtr2;
	CloseScreenProcPtr	CloseScreen;

	Bool				CursorIsSkewed;
        Bool                    shadowFB;
        int                     rotate;
        int                     ShadowPitch;
        unsigned char *         ShadowPtr;
        void	                (*PointerMoved)(int index, int x, int y);
        int                     pitch;

        unsigned char **        ScanlineColorExpandBuffers;

        LCDType                 lcdType;
        int                     lcdWidth, lcdHeight;
} CirRec, *CirPtr;

/* CirrusClk.c */
extern Bool
CirrusFindClock(int *rfreq, int max_clock, int *num_out, int *den_out);

/* cir_driver.c */
extern SymTabRec CIRChipsets[];
extern PciChipsets CIRPciChipsets[];

extern Bool CirMapMem(CirPtr pCir, int scrnIndex);
extern Bool CirUnmapMem(CirPtr pCir, int scrnIndex);
extern void cirProbeDDC(ScrnInfoPtr pScrn, int index);

/* in cir_shadow.c */
void cirPointerMoved(int index, int x, int y);
void cirRefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void cirRefreshArea8(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void cirRefreshArea16(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void cirRefreshArea24(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void cirRefreshArea32(ScrnInfoPtr pScrn, int num, BoxPtr pbox);

/* properties */
#define HWCUR64 0x1
#define ACCEL_AUTOSTART 0x2

#endif /* CIR_H */
