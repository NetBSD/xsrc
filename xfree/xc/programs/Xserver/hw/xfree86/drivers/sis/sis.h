/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
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
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *           Mike Chapman <mike@paranoia.com>, 
 *           Juanjo Santamarta <santamarta@ctv.es>, 
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp> 
 *           David Thomas <davtom@dream.org.uk>. 
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis.h,v 1.19 2000/12/02 01:16:17 dawes Exp $ */

#ifndef _SIS_H
#define _SIS_H_


#include "xf86Pci.h"
#include "xf86Cursor.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xaa.h"
#include "vgaHW.h"

#ifdef XF86DRI
#include "xf86drm.h"
#include "sarea.h"
#define _XF86DRI_SERVER_
#include "xf86dri.h"
#include "dri.h"
#include "GL/glxint.h"
#include "sis_dri.h"
#endif

#include "xf86xv.h"

#define SIS_NAME                "SIS"
#define SIS_DRIVER_NAME         "sis"
#define SIS_MAJOR_VERSION       0
#define SIS_MINOR_VERSION       6
#define SIS_PATCHLEVEL          0
#define SIS_CURRENT_VERSION     ((SIS_MAJOR_VERSION << 16) | \
                                (SIS_MINOR_VERSION << 8) | SIS_PATCHLEVEL )

#define UMA                     0x00000001
#define MMIOMODE                0x00000001
#define LFBQMODE                0x00000002
#define AGPQMODE                0x00000004

#define BIOS_BASE               0xC0000
#define BIOS_SIZE               0x10000

#define CRT2_DEFAULT            0x00000001
#define CRT2_LCD                0x00000010
#define CRT2_TV                 0x00000020
#define CRT2_VGA                0x00000040
#define CRT2_ENABLE             0x00000070
#define LCD_800x600             0x00000100
#define LCD_1024x768            0x00000200
#define LCD_1280x1024           0x00000400
#define LCD_TYPE                0x00000700
#define TV_NTSC                 0x00001000
#define TV_PAL                  0x00002000
#define TV_HIVISION             0x00004000
#define TV_TYPE                 0x00007000
#define TV_AVIDEO               0x00010000
#define TV_SVIDEO               0x00020000
#define TV_SCART                0x00040000
#define TV_INTERFACE            0x00070000
#define VB_301                  0x00100000
#define VB_302                  0x00200000
#define VB_303			0x00400000
#define VB_LVDS                 0x01000000
#define VB_CHRONTEL             0x02000000
#define SINGLE_MODE             0x00000000
#define SIMU_MODE               0x10000000
#define MM_MODE                 0x20000000
#define DISPLAY_MODE            0x30000000

#define MASK_DISPTYPE_CRT2     0x04         /* Connect LCD */
#define MASK_DISPTYPE_LCD      0x02         /* Connect LCD */
#define MASK_DISPTYPE_TV       0x01         /* Connect TV */
#define MASK_DISPTYPE_DISP2    (MASK_DISPTYPE_LCD | MASK_DISPTYPE_TV | MASK_DISPTYPE_CRT2)

#ifdef  DEBUG
#define PDEBUG(p)       p
#else
#define PDEBUG(p)
#endif

typedef struct {
        unsigned char sisRegs3C4[0x50];
        unsigned char sisRegs3D4[0x40];
        unsigned char sisRegs3C2;
        unsigned char VBPart1[0x29];
        unsigned char VBPart2[0x46];
        unsigned char VBPart3[0x3F];
        unsigned char VBPart4[0x1C];
	unsigned short ch7005[0x11];
} SISRegRec, *SISRegPtr;

#define SISPTR(p)       ((SISPtr)((p)->driverPrivate))
#define XAAPTR(p)       ((XAAInfoRecPtr)(SISPTR(p)->AccelInfoPtr))

typedef struct {
    ScrnInfoPtr         pScrn;
    pciVideoPtr         PciInfo;
    PCITAG              PciTag;
    EntityInfoPtr       pEnt;
    int                 Chipset;
    int                 ChipRev;
    unsigned long       FbAddress;              /* VRAM physical address */
    unsigned char *     FbBase;                 /* VRAM linear address */
    CARD32              IOAddress;              /* MMIO physical address */
    unsigned char *     IOBase;                 /* MMIO linear address */
#ifdef __alpha__
    unsigned char *     IOBaseDense;            /* MMIO for Alpha platform */
#endif
    CARD16              RelIO;                  /* Relocate IO Base */
    unsigned char *     BIOS;
    int                 MemClock;
    int                 BusWidth;
    int                 MinClock;
    int                 MaxClock;
    int                 Flags;                  /* HW config flags */
    long                FbMapSize;
    DGAModePtr          DGAModes;
    int                 numDGAModes;
    Bool                DGAactive;
    int                 DGAViewportStatus;
    Bool                NoAccel;
    Bool                HWCursor;
    Bool                UsePCIRetry;
    Bool                TurboQueue;
    int			ForceCRT2Type;	
    Bool                ValidWidth;
    Bool                FastVram;
    int                 VBFlags;
    short               scrnOffset;
    short               DstColor;
    int                 Xdirection;
    int                 Ydirection;
    int                 sisPatternReg[4];
    int                 ROPReg;
    int                 CommandReg;
    int                 MaxCMDQueueLen;
    int                 CurCMDQueueLen;
    int                 MinCMDQueueLen;
    int                 DstX;
    int                 DstY;
    unsigned char *     XAAScanlineColorExpandBuffers[2];
    CARD32              AccelFlags;
    Bool                ClipEnabled;
    Bool                DoColorExpand;
    SISRegRec           SavedReg;
    SISRegRec           ModeReg;
    xf86CursorInfoPtr   CursorInfoPtr;
    XAAInfoRecPtr       AccelInfoPtr;
    CloseScreenProcPtr  CloseScreen;
    unsigned int        (*ddc1Read)(ScrnInfoPtr);
    Bool                (*ModeInit)(ScrnInfoPtr pScrn, DisplayModePtr mode);
    void                (*SiSSave)(ScrnInfoPtr pScrn, SISRegPtr sisreg);
    void                (*SiSSave2)(ScrnInfoPtr pScrn, SISRegPtr sisreg);
    void                (*SiSSaveLVDS)(ScrnInfoPtr pScrn, SISRegPtr sisreg);
    void                (*SiSSaveChrontel)(ScrnInfoPtr pScrn, SISRegPtr sisreg);
    void                (*SiSRestore)(ScrnInfoPtr pScrn, SISRegPtr sisreg);
    void                (*SiSRestore2)(ScrnInfoPtr pScrn, SISRegPtr sisreg);
    void                (*SiSRestoreLVDS)(ScrnInfoPtr pScrn, SISRegPtr sisreg);
    void                (*SiSRestoreChrontel)(ScrnInfoPtr pScrn, SISRegPtr sisreg);
    void                (*SetThreshold)(ScrnInfoPtr pScrn, DisplayModePtr mode,
                                unsigned short *Low, unsigned short *High);
    void                (*LoadCRT2Palette)(ScrnInfoPtr pScrn, int numColors,
                                int *indicies, LOCO *colors, VisualPtr pVisual);

  int *cmdQueueLenPtr;
  unsigned long agpHandle;
  CARD32 agpAddr;
  unsigned char *agpBase;
  unsigned int agpSize;
  CARD32 agpCmdBufAddr;
  unsigned char *agpCmdBufBase;
  unsigned int agpCmdBufSize;
  unsigned int agpCmdBufFree;
  Bool irqEnabled;
  int irq;
  int ColorExpandRingHead;
  int ColorExpandRingTail;      
  int PerColorExpandBufferSize; 
  int ColorExpandBufferNumber;
  int ColorExpandBufferCountMask;
  unsigned char *ColorExpandBufferAddr[32];     
  int ColorExpandBufferScreenOffset[32];
  int ImageWriteBufferSize;
  unsigned char *ImageWriteBufferAddr;
  
#ifdef XF86DRI
  Bool directRenderingEnabled;
  DRIInfoPtr pDRIInfo;
  int drmSubFD;
  int numVisualConfigs;
  __GLXvisualConfig* pVisualConfigs;
  SISConfigPrivPtr pVisualConfigsPriv;
  SISRegRec DRContextRegs;
#endif

  XF86VideoAdaptorPtr adaptor;
  ScreenBlockHandlerProcPtr BlockHandler;
} SISRec, *SISPtr;

/* Prototypes */

void    SiSOptions(ScrnInfoPtr pScrn);
void    SISVGAPreInit(ScrnInfoPtr pScrn);
void    SISLCDPreInit(ScrnInfoPtr pScrn);
void    SISTVPreInit(ScrnInfoPtr pScrn);
void    SISCRT2PreInit(ScrnInfoPtr pScrn);
OptionInfoPtr SISAvailableOptions(int chipid, int busid);

void    SISDACPreInit(ScrnInfoPtr pScrn);
void    SISLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indicies,
                                        LOCO *colors, VisualPtr pVisual);

int compute_vclk(int Clock, int *out_n, int *out_dn, int *out_div,
                                        int *out_sbit, int *out_scale);
void SiSCalcClock(ScrnInfoPtr pScrn, int clock, int max_VLD,
                                        unsigned int *vclk);
unsigned int SiSddc1Read(ScrnInfoPtr pScrn);
Bool SiSAccelInit(ScreenPtr pScreen);
Bool SiS530AccelInit(ScreenPtr pScreen);
Bool SiS300AccelInit(ScreenPtr pScreen);
int  SiSMclk(SISPtr pSIS);
int sisMemBandWidth(ScrnInfoPtr pScrn);
int sis300MemBandWidth(ScrnInfoPtr pScrn);
Bool SiSHWCursorInit(ScreenPtr pScreen);
void SiSIODump(ScrnInfoPtr pScreen);
void SiSInitializeAccelerator(ScrnInfoPtr pScrn);
void SiSSetup(ScrnInfoPtr pScrn);
void SiSPreSetMode(ScrnInfoPtr pScrn);

extern void     SetReg1(unsigned short, unsigned short, unsigned short);
extern unsigned short GetReg1(unsigned short, unsigned short);

extern Bool     SISSwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
extern void     SISAdjustFrame(int scrnIndex, int x, int y, int flags);

#ifdef XF86DRI
extern void FillPrivateDRI(SISPtr pSIS, SISDRIPtr pSISDRI);
extern Bool SISDRIScreenInit(ScreenPtr pScreen);
extern void SISDRICloseScreen(ScreenPtr pScreen);
Bool SISDRIFinishScreenInit(ScreenPtr pScreen);
#endif

#endif
