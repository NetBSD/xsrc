/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/glint.h,v 1.37 2000/11/14 17:32:59 dawes Exp $ */
/*
 * Copyright 1997,1998 by Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *           Dirk Hohndel, <hohndel@suse.de>
 *	     Stefan Dirsch, <sndirsch@suse.de>
 *
 * this work is sponsored by S.u.S.E. GmbH, Fuerth, Elsa GmbH, Aachen and
 * Siemens Nixdorf Informationssysteme
 */
#ifndef _GLINT_H_
#define _GLINT_H_

#include "xaa.h"
#include "xf86RamDac.h"
#include "xf86cmap.h"
#include "xf86i2c.h"
#include "xf86DDC.h"
#ifdef XF86DRI
#include "xf86drm.h"
#include "sarea.h"
#define _XF86DRI_SERVER_
#include "xf86dri.h"
#include "dri.h"
#include "GL/glxint.h"
#include "glint_dripriv.h"
#endif

#define GLINT_MAX_MX_DEVICES 2
#define GLINT_VGA_MMIO_OFF 0x6000

#define VERSION 4000
#define GLINT_NAME "GLINT"
#define GLINT_DRIVER_NAME "glint"
#define GLINT_MAJOR_VERSION 1
#define GLINT_MINOR_VERSION 0
#define GLINT_PATCHLEVEL 0

typedef struct {
	CARD32 glintRegs[0x2000];
	CARD32 glintSecondRegs[0x2000];
	CARD32 DacRegs[0x100];  /* used by internal DACs */
	CARD8 cmap[0x300];
} GLINTRegRec, *GLINTRegPtr;

#define GLINTPTR(p)	((GLINTPtr)((p)->driverPrivate))

typedef struct {
    pciVideoPtr		PciInfo;
    pciVideoPtr		MXPciInfo[GLINT_MAX_MX_DEVICES];
    int			numMXDevices;
    PCITAG		PciTag;
    EntityInfoPtr	pEnt;
    RamDacHelperRecPtr	RamDac;
    int			MemClock;
    int			Chipset;
    int                 ChipRev;
    int			HwBpp;
    int			BppShift;
    int			pprod;
    int			ForeGroundColor;
    int			BackGroundColor;
    int			bppalign;
    int			startxdom;
    int			startxsub;
    int			starty;
    int			count;
    int			dy;
    int			x;
    int			y;
    int			w;
    int			h;
    int			dxdom;
    int			dwords;
    int			cpuheight;
    int			cpucount;
    int			planemask;
    int			realMXWidth;
    CARD32		IOAddress;
    unsigned long	FbAddress;
    int                 irq;
    unsigned char *     IOBase;
    unsigned char *     IOBaseVGA;
    unsigned char *	FbBase;
    long		FbMapSize;
    DGAModePtr		DGAModes;
    int			numDGAModes;
    Bool		DGAactive;
    int			DGAViewportStatus;
    Bool		DoubleBuffer;
    Bool		NoAccel;
    Bool		FBDev;
    Bool		ShadowFB;
    Bool		WriteBitmap;
    unsigned char *	ShadowPtr;
    int			ShadowPitch;
    Bool		Dac6Bit;
    Bool		HWCursor;
    Bool		ClippingOn;
    Bool		UsePCIRetry;
    Bool		UseBlockWrite;
    Bool		UseFireGL3000;
    Bool		VGAcore;
    int			MXFbSize;
    CARD32		rasterizerMode;
    int			MinClock;
    int			MaxClock;
    int			RefClock;
    GLINTRegRec		SavedReg;
    GLINTRegRec		ModeReg;
    CARD32		AccelFlags;
    CARD32		ROP;
    CARD32		FrameBufferReadMode;
    CARD32		BltScanDirection;
    CARD32		TexMapFormat;
    CARD32		PixelWidth;
    RamDacRecPtr	RamDacRec;
    xf86CursorInfoPtr	CursorInfoRec;
    XAAInfoRecPtr	AccelInfoRec;
    CloseScreenProcPtr	CloseScreen;
    GCPtr		CurrentGC;
    DrawablePtr		CurrentDrawable;
    I2CBusPtr		DDCBus, VSBus;
    CARD8*		XAAScanlineColorExpandBuffers[2];
    CARD32		RasterizerSwap;
    int			PM3_Config2D;
    int			PM3_Render2D;
    int			PM3_AreaStippleMode;
    int			PM3_VideoControl;
    Bool		PM3_UseGamma;
    pciVideoPtr		PM3_GammaPciInfo;
    PCITAG		PM3_GammaPciTag;
    CARD32		PM3_GammaIOAddress;
    unsigned char *     PM3_GammaIOBase;
#ifdef XF86DRI
    Bool		directRenderingEnabled;
    DRIInfoPtr		pDRIInfo;
    int			drmSubFD;
    drmBufMapPtr        drmBufs;         /* Map of DMA buffers */
    int			numVisualConfigs;
    __GLXvisualConfig*	pVisualConfigs;
    GLINTConfigPrivPtr	pVisualConfigsPriv;
    GLINTRegRec		DRContextRegs;
#endif
} GLINTRec, *GLINTPtr;

/* Defines for PCI data */

#define PCI_VENDOR_TI_CHIP_PERMEDIA2	\
			((PCI_VENDOR_TI << 16) | PCI_CHIP_TI_PERMEDIA2)
#define PCI_VENDOR_TI_CHIP_PERMEDIA	\
			((PCI_VENDOR_TI << 16) | PCI_CHIP_TI_PERMEDIA)
#define PCI_VENDOR_3DLABS_CHIP_PERMEDIA	\
			((PCI_VENDOR_3DLABS << 16) | PCI_CHIP_PERMEDIA)
#define PCI_VENDOR_3DLABS_CHIP_PERMEDIA2	\
			((PCI_VENDOR_3DLABS << 16) | PCI_CHIP_PERMEDIA2)
#define PCI_VENDOR_3DLABS_CHIP_PERMEDIA2V	\
			((PCI_VENDOR_3DLABS << 16) | PCI_CHIP_PERMEDIA2V)
#define PCI_VENDOR_3DLABS_CHIP_PERMEDIA3	\
			((PCI_VENDOR_3DLABS << 16) | PCI_CHIP_PERMEDIA3)
#define PCI_VENDOR_3DLABS_CHIP_300SX	\
			((PCI_VENDOR_3DLABS << 16) | PCI_CHIP_300SX)
#define PCI_VENDOR_3DLABS_CHIP_500TX	\
			((PCI_VENDOR_3DLABS << 16) | PCI_CHIP_500TX)
#define PCI_VENDOR_3DLABS_CHIP_MX	\
			((PCI_VENDOR_3DLABS << 16) | PCI_CHIP_MX)
#define PCI_VENDOR_3DLABS_CHIP_GAMMA	\
			((PCI_VENDOR_3DLABS << 16) | PCI_CHIP_GAMMA)
#define PCI_VENDOR_3DLABS_CHIP_DELTA	\
			((PCI_VENDOR_3DLABS << 16) | PCI_CHIP_DELTA)

/* Prototypes */

void Permedia2StoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs);
void Permedia2InstallColormap(ColormapPtr pmap);
void Permedia2UninstallColormap(ColormapPtr pmap);
int  Permedia2ListInstalledColormaps(ScreenPtr pScreen, Colormap *pmaps);
void Permedia2HandleColormaps(ScreenPtr pScreen, ScrnInfoPtr scrnp);
void Permedia2RestoreDACValues(ScrnInfoPtr pScrn);
void Permedia2Restore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
void Permedia2Save(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
Bool Permedia2Init(ScrnInfoPtr pScrn, DisplayModePtr mode);
Bool Permedia2AccelInit(ScreenPtr pScreen);
void Permedia2Sync(ScrnInfoPtr pScrn);
void Permedia2InitializeEngine(ScrnInfoPtr pScrn);
Bool Permedia2HWCursorInit(ScreenPtr pScreen);

void PermediaRestore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
void PermediaSave(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
Bool PermediaInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
Bool PermediaAccelInit(ScreenPtr pScreen);
void PermediaInitializeEngine(ScrnInfoPtr pScrn);
void Permedia2VRestore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
void Permedia2VSave(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
Bool Permedia2VInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
Bool Permedia2vHWCursorInit(ScreenPtr pScreen);

/* pm3_dac.c */
void Permedia3PreInit(ScrnInfoPtr pScrn);
int Permedia3MemorySizeDetect(ScrnInfoPtr pScrn);
void Permedia3Restore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
void Permedia3Save(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
Bool Permedia3Init(ScrnInfoPtr pScrn, DisplayModePtr mode);
/* pm3_accel.c */
/* Not needed, ... is it ever called outside of pm3_accel.c ?
void Permedia3Sync(ScrnInfoPtr pScrn);
*/
Bool Permedia3AccelInit(ScreenPtr pScreen);
void Permedia3InitializeEngine(ScrnInfoPtr pScrn);
/* End of pm3 stuff */

void TXRestore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
void TXSave(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
Bool TXInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
Bool TXAccelInit(ScreenPtr pScreen);
void TXInitializeEngine(ScrnInfoPtr pScrn);

Bool SXAccelInit(ScreenPtr pScreen);

void DualMXRestore(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
void DualMXSave(ScrnInfoPtr pScrn, GLINTRegPtr glintReg);
Bool DualMXInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
Bool DualMXAccelInit(ScreenPtr pScreen);
void DualMXInitializeEngine(ScrnInfoPtr pScrn);

void glintOutIBMRGBIndReg(ScrnInfoPtr pScrn,
		     CARD32 reg, unsigned char mask, unsigned char data);
unsigned char glintInIBMRGBIndReg(ScrnInfoPtr pScrn, CARD32 reg);
void glintIBMWriteAddress(ScrnInfoPtr pScrn, CARD32 index);
void glintIBMReadAddress(ScrnInfoPtr pScrn, CARD32 index);
void glintIBMWriteData(ScrnInfoPtr pScrn, unsigned char data);
Bool glintIBMHWCursorInit(ScreenPtr pScreen);
unsigned char glintIBMReadData(ScrnInfoPtr pScrn);
Bool glintIBM526HWCursorInit(ScreenPtr pScreen);
Bool glintIBM640HWCursorInit(ScreenPtr pScreen);

void glintOutTIIndReg(ScrnInfoPtr pScrn,
		     CARD32 reg, unsigned char mask, unsigned char data);
unsigned char glintInTIIndReg(ScrnInfoPtr pScrn, CARD32 reg);
void DUALglintOutTIIndReg(ScrnInfoPtr pScrn,
		     CARD32 reg, unsigned char mask, unsigned char data);
unsigned char DUALglintInTIIndReg(ScrnInfoPtr pScrn, CARD32 reg);
void glintTIWriteAddress(ScrnInfoPtr pScrn, CARD32 index);
void glintTIReadAddress(ScrnInfoPtr pScrn, CARD32 index);
void glintTIWriteData(ScrnInfoPtr pScrn, unsigned char data);
unsigned char glintTIReadData(ScrnInfoPtr pScrn);
void DUALglintTIWriteAddress(ScrnInfoPtr pScrn, CARD32 index);
void DUALglintTIReadAddress(ScrnInfoPtr pScrn, CARD32 index);
void DUALglintTIWriteData(ScrnInfoPtr pScrn, unsigned char data);
unsigned char DUALglintTIReadData(ScrnInfoPtr pScrn);
Bool glintTIHWCursorInit(ScreenPtr pScreen);

void Permedia2OutIndReg(ScrnInfoPtr pScrn,
		     CARD32, unsigned char mask, unsigned char data);
unsigned char Permedia2InIndReg(ScrnInfoPtr pScrn, CARD32);
void Permedia2WriteAddress(ScrnInfoPtr pScrn, CARD32 index);
void Permedia2ReadAddress(ScrnInfoPtr pScrn, CARD32 index);
void Permedia2WriteData(ScrnInfoPtr pScrn, unsigned char data);
unsigned char Permedia2ReadData(ScrnInfoPtr pScrn);
void TIramdacLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
    			  LOCO *colors, VisualPtr pVisual);
void Permedia2LoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
    			  LOCO *colors, VisualPtr pVisual);
void Permedia2LoadPalette16(ScrnInfoPtr pScrn, int numColors, int *indices,
    			  LOCO *colors, VisualPtr pVisual);
void Permedia2I2CUDelay(I2CBusPtr b, int usec);
void Permedia2I2CPutBits(I2CBusPtr b, int scl, int sda);
void Permedia2I2CGetBits(I2CBusPtr b, int *scl, int *sda);

void Permedia2VideoInit(ScreenPtr pScreen);
void Permedia2VideoUninit(ScrnInfoPtr pScrn);
void Permedia2VideoEnterVT(ScrnInfoPtr pScrn);
void Permedia2VideoLeaveVT(ScrnInfoPtr pScrn);

void Permedia2vOutIndReg(ScrnInfoPtr pScrn,
		   CARD32, unsigned char mask, unsigned char data);
unsigned char Permedia2vInIndReg(ScrnInfoPtr pScrn, CARD32);

Bool GLINTSwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
void GLINTAdjustFrame(int scrnIndex, int x, int y, int flags);

extern int partprodPermedia[];

Bool GLINTDRIScreenInit(ScreenPtr pScreen);
Bool GLINTDRIFinishScreenInit(ScreenPtr pScreen);
void GLINTDRICloseScreen(ScreenPtr pScreen);
Bool GLINTInitGLXVisuals(ScreenPtr pScreen);
void GLINTDRIWakeupHandler(ScreenPtr pScreen);
void GLINTDRIBlockHandler(ScreenPtr pScreen);
void GLINTDRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 index);
void GLINTDRIMoveBuffers(WindowPtr pWin, DDXPointRec ptOldOrg, 
		RegionPtr prgnSrc, CARD32 index);

void GLINT_VERB_WRITE_REG(GLINTPtr, CARD32 v, int r, char *file, int line);
CARD32 GLINT_VERB_READ_REG(GLINTPtr, CARD32 r, char *file, int line);

void GLINTRefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
#endif /* _GLINT_H_ */
