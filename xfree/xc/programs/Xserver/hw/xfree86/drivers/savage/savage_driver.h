/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/savage/savage_driver.h,v 1.3 2000/12/07 20:26:22 dawes Exp $ */

#ifndef SAVAGE_VGAHWMMIO_H
#define SAVAGE_VGAHWMMIO_H

#include "vgaHW.h"
#include "xf86.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "xf86Pci.h"
#include "xf86PciInfo.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "xf86Cursor.h"
#include "mipointer.h"
#include "micmap.h"
#include "cfb.h"
#include "cfb16.h"
#include "cfb32.h"
#include "xf86cmap.h"
#include "vbe.h"
#include "xaa.h"

#include "savage_regs.h"

#define VGAIN8(addr) MMIO_IN8(psav->MapBase+0x8000, addr)
#define VGAIN16(addr) MMIO_IN16(psav->MapBase+0x8000, addr)
#define VGAIN(addr) MMIO_IN32(psav->MapBase+0x8000, addr)

#define VGAOUT8(addr,val) MMIO_OUT8(psav->MapBase+0x8000, addr, val)
#define VGAOUT16(addr,val) MMIO_OUT16(psav->MapBase+0x8000, addr, val)
#define VGAOUT(addr,val) MMIO_OUT32(psav->MapBase+0x8000, addr, val)

#define INREG(addr) MMIO_IN32(psav->MapBase, addr)
#define OUTREG(addr,val) MMIO_OUT32(psav->MapBase, addr, val)
#define INREG16(addr) MMIO_IN16(psav->MapBase, addr)
#define OUTREG16(addr,val) MMIO_OUT16(psav->MapBase, addr, val)


typedef struct _S3VMODEENTRY {
   unsigned short Width;
   unsigned short Height;
   unsigned short VesaMode;
   unsigned char RefreshCount;
   unsigned char * RefreshRate;
} SavageModeEntry, *SavageModeEntryPtr;


typedef struct _S3VMODETABLE {
   unsigned short NumModes;
   SavageModeEntry Modes[1];
} SavageModeTableRec, *SavageModeTablePtr;


typedef struct {
    unsigned int mode, refresh;
    unsigned char SR08, SR0A, SR0F;
    unsigned char SR10, SR11, SR12, SR13, SR15, SR18, SR29;
    unsigned char SR54[8];
    unsigned char Clock;
    unsigned char CR31, CR32, CR33, CR34, CR36, CR3A, CR3B, CR3C;
    unsigned char CR40, CR41, CR42, CR43, CR45;
    unsigned char CR50, CR51, CR53, CR55, CR58, CR5B, CR5D, CR5E;
    unsigned char CR60, CR63, CR65, CR66, CR67, CR68, CR69, CR6D, CR6F;
    unsigned char CR86, CR88;
    unsigned char CR90, CR91, CRB0;
    unsigned char ColorStack[8];
    unsigned int  STREAMS[22];	/* yuck, streams regs */
    unsigned int  MMPR0, MMPR1, MMPR2, MMPR3;
} SavageRegRec, *SavageRegPtr;


typedef struct _Savage {
    SavageRegRec	SavedReg;
    SavageRegRec	ModeReg;
    xf86CursorInfoPtr	CursorInfoRec;
    Bool		ModeStructInit;
    Bool		NeedSTREAMS;
    Bool		STREAMSRunning;
    int			Bpp, Bpl, ScissB;
    unsigned		PlaneMask;

    int			videoRambytes;
    int			videoRamKbytes;
    int			MemOffScreen;
    CARD32		CursorKByte;

    /* These are physical addresses. */
    unsigned long	FrameBufferBase;
    unsigned long	MmioBase;

    /* These are linear addresses. */
    unsigned char*	MapBase;
    unsigned char*	BciMem;
    unsigned char*	MapBaseDense;
    unsigned char*	FBBase;
    unsigned char*	FBStart;

    Bool		PrimaryVidMapped;
    int			dacSpeedBpp;
    int			minClock, maxClock;
    int			HorizScaleFactor;
    int			MCLK, REFCLK, LCDclk;
    double		refclk_fact;
    int			GEResetCnt;

    /* Here are all the Options */

    Bool		ShowCache;
    Bool		pci_burst;
    Bool		NoPCIRetry;
    Bool		fifo_conservative;
    Bool		fifo_moderate;
    Bool		fifo_aggressive;
    Bool		slow_edodram;
    Bool		slow_dram;
    Bool		fast_dram;
    Bool		fpm_vram;
    Bool		early_ras_precharge;
    Bool		hwcursor;
    Bool		NoAccel;
    Bool		shadowFB;
    Bool		UseBIOS;
    int			rotate;

    CloseScreenProcPtr	CloseScreen;
    pciVideoPtr		PciInfo;
    PCITAG		PciTag;
    int			Chipset;
    int			ChipId;
    int			ChipRev;
    vbeInfoPtr		pVbe;
    int			EntityIndex;

    /* The various Savage wait handlers. */
    int			(*myWaitQueue)(struct _Savage *, int);
    int			(*myWaitIdle)(struct _Savage *);
    int			(*myWaitIdleEmpty)(struct _Savage *);
    int			(*myWaitCommandEmpty)(struct _Savage *);

    /* Support for shadowFB and rotation */
    unsigned char *	ShadowPtr;
    int			ShadowPitch;
    void		(*PointerMoved)(int index, int x, int y);

    /* Support for XAA acceleration */
    XAAInfoRecPtr	AccelInfoRec;
    xRectangle		Rect;
    unsigned int	SavedBciCmd;
    unsigned int	SavedFgColor;
    unsigned int	SavedBgColor;
    unsigned int	SavedGbdOffset;
    unsigned int	SavedGbd;
    unsigned int	SavedSbdOffset;
    unsigned int	SavedSbd;

    /* Support for Int10 processing */
    xf86Int10InfoPtr	pInt10;
    SavageModeTablePtr	ModeTable;

    /* Support for the Savage command overflow buffer. */
    unsigned long	cobIndex;	/* size index */
    unsigned long	cobSize;	/* size in bytes */
    unsigned long	cobOffset;	/* offset in frame buffer */

} SavageRec, *SavagePtr;

/* Shortcuts.  These depend on a local symbol "psav". */

#define WaitIdle()		psav->myWaitIdle(psav)
#define WaitIdleEmpty()		psav->myWaitIdleEmpty(psav)
#define	WaitQueue(k)		psav->myWaitQueue(psav,k)
#define WaitCommandEmpty()	psav->myWaitCommandEmpty(psav)

#define SAVPTR(p)	((SavagePtr)((p)->driverPrivate))

/* Prototypes. */

extern void SavageCommonCalcClock(long freq, int min_m, int min_n1,
			int max_n1, int min_n2, int max_n2,
			long freq_min, long freq_max,
			unsigned char *mdiv, unsigned char *ndiv);
void SavageAdjustFrame(int scrnIndex, int y, int x, int flags);
Bool SavageSwitchMode(int scrnIndex, DisplayModePtr mode, int flags);

/* In savage_cursor.c. */

Bool SavageHWCursorInit(ScreenPtr pScreen);

/* In savage_accel.c. */

Bool SavageInitAccel(ScreenPtr);
void SavageInitialize2DEngine(ScrnInfoPtr);
void SavageSetGBD(ScrnInfoPtr);

/* In savage_shadow.c */

void SavagePointerMoved(int index, int x, int y);
void SavageRefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void SavageRefreshArea8(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void SavageRefreshArea16(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void SavageRefreshArea24(ScrnInfoPtr pScrn, int num, BoxPtr pbox);
void SavageRefreshArea32(ScrnInfoPtr pScrn, int num, BoxPtr pbox);

/* In savage_vbe.c */

void SavageSetTextMode( SavagePtr psav );
void SavageSetVESAMode( SavagePtr psav, int n, int Refresh );
void SavageFreeBIOSModeTable( SavagePtr psav, SavageModeTablePtr* ppTable );
SavageModeTablePtr SavageGetBIOSModeTable( SavagePtr psav, int iDepth );

unsigned short SavageGetBIOSModes( 
    SavagePtr psav,
    int iDepth,
    SavageModeEntryPtr s3vModeTable );


#endif /* SAVAGE_VGAHWMMIO_H */

