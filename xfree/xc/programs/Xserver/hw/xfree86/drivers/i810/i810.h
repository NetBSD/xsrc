
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/i810/i810.h,v 1.29 2002/01/14 18:43:51 dawes Exp $ */

/*
 * Authors:
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 */

/* #define I830DEBUG */

#ifndef _I810_H_
#define _I810_H_

#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "i810_reg.h"
#include "xaa.h"
#include "xf86Cursor.h"
#include "xf86xv.h"
#include "xf86int10.h"
#include "vgaHW.h"

#ifdef XF86DRI
#include "xf86drm.h"
#include "sarea.h"
#define _XF86DRI_SERVER_
#include "xf86dri.h"
#include "dri.h"
#include "GL/glxint.h"
#include "i810_dri.h"
#include "i830_dri.h"
#endif


#define I810_VERSION 4000
#define I810_NAME "I810"
#define I810_DRIVER_NAME "i810"
#define I810_MAJOR_VERSION 1
#define I810_MINOR_VERSION 1
#define I810_PATCHLEVEL 0

#ifdef __GNUC__
#define PFX __FILE__,__LINE__,__FUNCTION__
#define FUNCTION_NAME __FUNCTION__
#else
#define PFX __FILE__,__LINE__,""
#define FUNCTION_NAME ""
#endif

#ifdef I830DEBUG
#define MARKER() fprintf(stderr,"\n### %s:%d: >>> %s <<< ###\n\n",__FILE__,__LINE__,__FUNCTION__)
#define DPRINTF DPRINTF_stub
#else	/* #ifdef I830DEBUG */
#define MARKER()
/* this is a real ugle hack to get the compiler to optimize the debugging statements into oblivion */
#define DPRINTF if(0) DPRINTF_stub
#endif	/* #ifdef I830DEBUG */

#define KB(x) ((x) * 1024)
#define MB(x) ((x) * KB(1024))

/* I830 Video BIOS support */

/* This code is Heavily based upon the VESA driver written by:
 * Paulo C~^Aésar Pereira de Andrade <pcpa@conectiva.com.br> */

typedef struct _VBEInfoBlock VBEInfoBlock;
typedef struct _ModeInfoBlock ModeInfoBlock;
typedef struct _CRTCInfoBlock CRTCInfoBlock;
typedef struct _VBT_header VBTHeader;
typedef struct _BIOSDataBlock BIOSDataBlock;

#define FARP(p)        (((unsigned)(p & 0xffff0000) >> 12) | (p & 0xffff))

typedef struct _VESARec
{
   xf86Int10InfoPtr pInt;
   EntityInfoPtr pEnt;
   CARD16 major, minor;
   VBEInfoBlock *vbeInfo;
   GDevPtr device;
   pciVideoPtr pciInfo;
   PCITAG pciTag;
   CARD16 maxBytesPerScanline;
   int mapPhys, mapOff, mapSize;  /* video memory */
   void *base, *VGAbase;
   CARD8 *state, *pstate; /* SVGA state */
   int statePage, stateSize, stateMode;
   int page;
   CARD8 *block;
   int pix24bpp;
   CARD32 *pal, *savedPal;
   CARD8 *fonts;
   xf86MonPtr monitor;
   Bool shadowFB, primary;
   CARD8 *shadowPtr;
   CARD32 windowAoffset;
} VESARec, *VESAPtr;

typedef struct _ModeInfoData 
{
   int mode;
   ModeInfoBlock *data;
   CRTCInfoBlock *block;
} ModeInfoData;

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(a)
#endif

/*
 * INT 0
 */
struct _VBEInfoBlock
{
   /* VESA 1.2 fields */
   CARD8 VESASignature[4];        /* VESA */
   CARD16 VESAVersion;            /* Higher byte major, lower byte minor */
   /*CARD32*/char *OEMStringPtr;  /* Pointer to OEM string */
   CARD8 Capabilities[4];         /* Capabilities of the video environment */

   /*CARD32*/CARD16 *VideoModePtr;    /* pointer to supported Super VGA modes */

   CARD16 TotalMemory;            /* Number of 64kb memory blocks on board */
   /* if not VESA 2, 236 scratch bytes follow (256 bytes total size) */

   /* VESA 2 fields */
   CARD16 OemSoftwareRev;     /* VBE implementation Software revision */
   /*CARD32*/char *OemVendorNamePtr;  /* Pointer to Vendor Name String */
   /*CARD32*/char *OemProductNamePtr; /* Pointer to Product Name String */
   /*CARD32*/char *OemProductRevPtr;  /* Pointer to Product Revision String */
   CARD8 Reserved[222];       /* Reserved for VBE implementation */
   CARD8 OemData[256];            /* Data Area for OEM Strings */
} __attribute__((packed));

/*
 * INT 1
 */
struct _ModeInfoBlock {
   CARD16 ModeAttributes;     /* mode attributes */
   CARD8 WinAAttributes;      /* window A attributes */
   CARD8 WinBAttributes;      /* window B attributes */
   CARD16 WinGranularity;     /* window granularity */
   CARD16 WinSize;            /* window size */
   CARD16 WinASegment;            /* window A start segment */
   CARD16 WinBSegment;            /* window B start segment */
   CARD32 WinFuncPtr;         /* real mode pointer to window function */
   CARD16 BytesPerScanline;       /* bytes per scanline */

   /* Mandatory information for VBE 1.2 and above */
   CARD16 XResolution;            /* horizontal resolution in pixels or characters */
   CARD16 YResolution;            /* vertical resolution in pixels or characters */
   CARD8 XCharSize;           /* character cell width in pixels */
   CARD8 YCharSize;           /* character cell height in pixels */
   CARD8 NumberOfPlanes;      /* number of memory planes */
   CARD8 BitsPerPixel;            /* bits per pixel */
   CARD8 NumberOfBanks;       /* number of banks */
   CARD8 MemoryModel;         /* memory model type */
   CARD8 BankSize;            /* bank size in KB */
   CARD8 NumberOfImages;      /* number of images */
   CARD8 Reserved;    /* 1 */     /* reserved for page function */

   /* Direct color fields (required for direct/6 and YUV/7 memory models) */
   CARD8 RedMaskSize;         /* size of direct color red mask in bits */
   CARD8 RedFieldPosition;        /* bit position of lsb of red mask */
   CARD8 GreenMaskSize;       /* size of direct color green mask in bits */
   CARD8 GreenFieldPosition;      /* bit position of lsb of green mask */
   CARD8 BlueMaskSize;            /* size of direct color blue mask in bits */
   CARD8 BlueFieldPosition;       /* bit position of lsb of blue mask */
   CARD8 RsvdMaskSize;            /* size of direct color reserved mask in bits */
   CARD8 RsvdFieldPosition;       /* bit position of lsb of reserved mask */
   CARD8 DirectColorModeInfo;     /* direct color mode attributes */

   /* Mandatory information for VBE 2.0 and above */
   CARD32 PhysBasePtr;            /* physical address for flat memory frame buffer */
   CARD32 Reserved32; /* 0 */     /* Reserved - always set to 0 */
   CARD16 Reserved16; /* 0 */     /* Reserved - always set to 0 */

   /* Mandatory information for VBE 3.0 and above */
   CARD16 LinBytesPerScanLine;        /* bytes per scan line for linear modes */
   CARD8 BnkNumberOfImagePages;   /* number of images for banked modes */
   CARD8 LinNumberOfImagePages;   /* number of images for linear modes */
   CARD8 LinRedMaskSize;      /* size of direct color red mask (linear modes) */
   CARD8 LinRedFieldPosition;     /* bit position of lsb of red mask (linear modes) */
   CARD8 LinGreenMaskSize;        /* size of direct color green mask (linear modes) */
   CARD8 LinGreenFieldPosition;   /* bit position of lsb of green mask (linear modes) */
   CARD8 LinBlueMaskSize;     /* size of direct color blue mask (linear modes) */
   CARD8 LinBlueFieldPosition;        /* bit position of lsb of blue mask (linear modes) */
   CARD8 LinRsvdMaskSize;     /* size of direct color reserved mask (linear modes) */
   CARD8 LinRsvdFieldPosition;        /* bit position of lsb of reserved mask (linear modes) */
   CARD32 MaxPixelClock;      /* maximum pixel clock (in Hz) for graphics mode */
   CARD8 Reserved2[189];      /* remainder of ModeInfoBlock */
} __attribute__((packed));

/*
 * INT2
 */
#define CRTC_DBLSCAN   (1<<0)
#define CRTC_INTERLACE (1<<1)
#define CRTC_NHSYNC    (1<<2)
#define CRTC_NVSYNC    (1<<3)

struct _CRTCInfoBlock 
{
   CARD16 HorizontalTotal;        /* Horizontal total in pixels */
   CARD16 HorizontalSyncStart;        /* Horizontal sync start in pixels */
   CARD16 HorizontalSyncEnd;      /* Horizontal sync end in pixels */
   CARD16 VerticalTotal;      /* Vertical total in lines */
   CARD16 VerticalSyncStart;      /* Vertical sync start in lines */
   CARD16 VerticalSyncEnd;        /* Vertical sync end in lines */
   CARD8 Flags;           /* Flags (Interlaced, Double Scan etc) */
   CARD32 PixelClock;         /* Pixel clock in units of Hz */
   CARD16 RefreshRate;            /* Refresh rate in units of 0.01 Hz */
   CARD8 Reserved[40];            /* remainder of ModeInfoBlock */
} __attribute__((packed));

/* CRTCInfoBlock is in the VESA 3.0 specs */
typedef struct _VESApmi 
{
   int seg_tbl;
   int tbl_off;
   int tbl_len;
} VESApmi;

/* Functions */
const OptionInfoRec *I830BIOSAvailableOptions(int chipid, int busid);
ModeInfoBlock *I830VESAGetModeInfo(ScrnInfoPtr pScrn, int mode);
void I830BIOSProbeDDC(ScrnInfoPtr pScrn, int index);
void I830VESAFreeModeInfo(ModeInfoBlock *block);
Bool I830BIOSPreInit(ScrnInfoPtr pScrn, int flags);
void I830BIOSSaveRegisters(ScrnInfoPtr pScrn);
void I830BIOSSetRegisters(ScrnInfoPtr pScrn, int mode);
Bool I830VESAGetVBEMode(ScrnInfoPtr pScrn, int *mode);
Bool I830VESASetVBEMode(ScrnInfoPtr pScrn, int mode, CRTCInfoBlock *block);
Bool I830VESASaveRestore(ScrnInfoPtr pScrn, int function);
Bool I830VESASetGetLogicalScanlineLength(ScrnInfoPtr pScrn, int command,
                 int width, int *pixels, int *bytes,
                 int *max);
int I830VESASetGetDACPaletteFormat(ScrnInfoPtr pScrn, int bits);
CARD32 *I830VESASetGetPaletteData(ScrnInfoPtr pScrn, Bool set, int first,
                 int num, CARD32 *data, Bool secondary,
                 Bool wait_retrace);
Bool I830BIOSInitializeFirstMode(int scrnIndex);
Bool I830BIOSScreenInit(int scrnIndex, ScreenPtr pScreen,
                 int argc, char **argv);
Bool I830VESASetDisplayStart(ScrnInfoPtr pScrn, int x, int y,
                 Bool wait_retrace);
void I830BIOSAdjustFrame(int scrnIndex, int x, int y, int flags);
void I830BIOSFreeScreen(int scrnIndex, int flags);
void I830BIOSLeaveVT(int scrnIndex, int flags);
Bool I830BIOSEnterVT(int scrnIndex, int flags);
Bool I830BIOSSwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
Bool I830BIOSSaveScreen(ScreenPtr pScreen, Bool unblack);
Bool I830BIOSCloseScreen(int scrnIndex, ScreenPtr pScreen);

#define    SCANWID_SET     0
#define SCANWID_GET        1
#define SCANWID_SET_BYTES  2
#define SCANWID_GET_MAX        3
#define I830VESASetLogicalScanline(pScrn, width)   \
  I830VESASetGetLogicalScanlineLength(pScrn, SCANWID_SET, \
                 width, NULL, NULL, NULL)
#define I830VESASetLogicalScanlineBytes(pScrn, width)  \
  I830VESASetGetLogicalScanlineLength(pScrn, width, SCANWID_SET_BYTES, \
                 NULL, NULL, NULL)
#define I830VESAGetLogicalScanline(pScrn, pixels, bytes, max)  \
  I830VESASetGetLogicalScanlineLength(pScrn, SCANWID_GET, NULL, \
                 pixels, bytes, max)
#define I830VESAGetMaxLogicalScanline(pScrn, pixels, bytes, max)   \
  I830VESASetGetLogicalScanlineLength(pScrn, SCANWID_GET_MAX, \
                 NULL, pixels, bytes, max)

#define MODE_QUERY 0
#define MODE_SAVE  1
#define MODE_RESTORE   2

#define SET_SAVED_MODE     1
#define SET_CURRENT_MODE   2

/* HWMC Surfaces */
#define I810_MAX_SURFACES 7
#define I810_MAX_SUBPICTURES 2
#define I810_TOTAL_SURFACES 9

/* Globals */

typedef struct _I810Rec *I810Ptr;

typedef void  (*I810WriteIndexedByteFunc)(I810Ptr pI810, int addr, 
					  CARD8 index, CARD8 value);
typedef CARD8 (*I810ReadIndexedByteFunc)(I810Ptr pI810, int addr, CARD8 index);
typedef void  (*I810WriteByteFunc)(I810Ptr pI810, int addr, CARD8 value);
typedef CARD8 (*I810ReadByteFunc)(I810Ptr pI810, int addr);


extern void I810SetTiledMemory(ScrnInfoPtr pScrn, 
			       int nr, 
			       unsigned start,
			       unsigned pitch,
			       unsigned size);

typedef enum {
   OPTION_NOACCEL,
   OPTION_SW_CURSOR,
   OPTION_COLOR_KEY,
   OPTION_CACHE_LINES,
   OPTION_DAC_6BIT,
   OPTION_DRI,
   OPTION_NO_DDC,
   OPTION_STRETCH,
   OPTION_CENTER,
   OPTION_XVMC_SURFACES
} I810Opts;

/* Linear region allocated in framebuffer.
 */
typedef struct {
   unsigned long Start;
   unsigned long End;
   unsigned long Size;
} I810MemRange;

typedef struct {
   int tail_mask;
   I810MemRange mem;
   unsigned char *virtual_start;
   int head;
   int tail;
   int space;
} I810RingBuffer;

typedef struct {
   unsigned char DisplayControl;
   unsigned char PixelPipeCfg0;
   unsigned char PixelPipeCfg1;
   unsigned char PixelPipeCfg2;
   unsigned short VideoClk2_M;
   unsigned short VideoClk2_N;
   unsigned char VideoClk2_DivisorSel;
   unsigned char AddressMapping;
   unsigned char IOControl;
   unsigned char BitBLTControl;
   unsigned char ExtVertTotal;
   unsigned char ExtVertDispEnd;
   unsigned char ExtVertSyncStart;
   unsigned char ExtVertBlankStart;
   unsigned char ExtHorizTotal;
   unsigned char ExtHorizBlank;
   unsigned char ExtOffset;
   unsigned char InterlaceControl;
   unsigned int  LMI_FIFO_Watermark;

   unsigned int  LprbTail;
   unsigned int  LprbHead;
   unsigned int  LprbStart;
   unsigned int  LprbLen;

   unsigned int Fence[8];

   unsigned short OverlayActiveStart;
   unsigned short OverlayActiveEnd;


} I810RegRec, *I810RegPtr;

typedef struct
{
   unsigned int  LprbTail;
   unsigned int  LprbHead;
   unsigned int  LprbStart;
   unsigned int  LprbLen;

   unsigned int Fence[8];

   unsigned int VideoClk_VGA0_M1M2N;
   unsigned int VideoClk_VGA1_M1M2N;
   unsigned int VideoClk_PostDiv;
   unsigned int fpa_0;
   unsigned int fpa_1;
   unsigned int dpll_a;
   unsigned int dpll_b;

   unsigned int  LMI_FIFO_Watermark;
   unsigned int  LMI_FIFO_Watermark2;

   unsigned int HTotal_A;
   unsigned int HBlank_A;
   unsigned int HSync_A;
   unsigned int VTotal_A;
   unsigned int VBlank_A;
   unsigned int VSync_A;
   unsigned int Bclrpat_A;
   unsigned int PipeASRC;

   unsigned int HTotal_B;
   unsigned int HBlank_B;
   unsigned int HSync_B;
   unsigned int VTotal_B;
   unsigned int VBlank_B;
   unsigned int VSync_B;
   unsigned int Bclrpat_B;
   unsigned int PipeBSRC;

   unsigned int PipeAConf;
   unsigned int PipeBConf;
   unsigned int DspACntr;
   unsigned int DspAStride;
   unsigned int DspABase;
   unsigned int DspBCntr;
   unsigned int DspBStride;
   unsigned int DspBBase;

   unsigned int adpa;
   unsigned int dv0a;
   unsigned int dv0b;
} I830RegRec, *I830RegPtr;

typedef struct _I810Rec {
   unsigned char *MMIOBase;
   unsigned char *FbBase;
   long FbMapSize;
   long DepthOffset;
   long BackOffset;
   int cpp;
   int MaxClock;

   unsigned int bufferOffset;	/* for I810SelectBuffer */
   Bool DoneFrontAlloc;
   BoxRec FbMemBox;
   I810MemRange FrontBuffer;
   I810MemRange BackBuffer;   
   I810MemRange DepthBuffer;
   I810MemRange TexMem;
   I810MemRange Scratch;
   I810MemRange BufferMem;
   I810MemRange ContextMem;
   I810MemRange MC; 

   int auxPitch;
   int auxPitchBits;

   int CursorOffset;
   unsigned long CursorPhysical;
   unsigned long CursorStart;
   unsigned long OverlayPhysical;
   unsigned long OverlayStart;
   int colorKey;
   int surfaceAllocation[I810_TOTAL_SURFACES];
   int numSurfaces;

   DGAModePtr DGAModes;
   int numDGAModes;
   Bool DGAactive;
   int DGAViewportStatus;

   int Chipset;
   unsigned long LinearAddr;
   unsigned long MMIOAddr;
   EntityInfoPtr pEnt;
   pciVideoPtr PciInfo;
   PCITAG PciTag;

   I810RingBuffer LpRing;
   unsigned int BR[20]; 

   int  LmFreqSel;

   int VramKey;
   unsigned long VramOffset;
   int DcacheKey;
   unsigned long DcacheOffset;
   int HwcursKey; 
   unsigned long HwcursOffset;

   int GttBound;
   
   I810MemRange DcacheMem;
   I810MemRange SysMem;

   I810MemRange SavedDcacheMem;
   I810MemRange SavedSysMem;

   unsigned char **ScanlineColorExpandBuffers;
   int NumScanlineColorExpandBuffers;
   int nextColorExpandBuf;

   I810RegRec SavedReg;
   I810RegRec ModeReg;

   I830RegRec i830_SavedReg;
   I830RegRec i830_ModeReg;

   XAAInfoRecPtr AccelInfoRec;
   xf86CursorInfoPtr CursorInfoRec;
   CloseScreenProcPtr CloseScreen;
   ScreenBlockHandlerProcPtr BlockHandler;
 
   I810WriteIndexedByteFunc writeControl;
   I810ReadIndexedByteFunc readControl;
   I810WriteByteFunc writeStandard;
   I810ReadByteFunc readStandard;

   Bool directRenderingEnabled;	/* false if XF86DRI not defined. */

#ifdef XF86DRI
   int LockHeld;
   DRIInfoPtr pDRIInfo;
   int drmSubFD;
   int numVisualConfigs;
   __GLXvisualConfig* pVisualConfigs;
   I810ConfigPrivPtr pVisualConfigsPriv;
   unsigned long dcacheHandle;
   unsigned long backHandle;
   unsigned long zHandle;
   unsigned long cursorHandle;
   unsigned long xvmcHandle;
   unsigned long sysmemHandle;
   Bool agpAcquired;
   drmHandle buffer_map;
   drmHandle ring_map;
   drmHandle overlay_map;
   drmHandle mc_map;
   drmHandle xvmcContext;
#endif
   Bool agpAcquired2d;

   XF86VideoAdaptorPtr adaptor;
   OptionInfoPtr Options;

   /* Stolen memory support */
   unsigned long StolenSize;
   Bool StolenOnly;

   /* Video BIOS support */
   VESAPtr vesa;

   int configured_device;
} I810Rec;

#define I810PTR(p) ((I810Ptr)((p)->driverPrivate))

#define I810_FRONT 0
#define I810_BACK 1
#define I810_DEPTH 2

extern const char *I810vgahwSymbols[];
extern const char *I810ramdacSymbols[];
extern const char *I810int10Symbols[];
extern const char *I810vbeSymbols[];
extern const char *I810ddcSymbols[];
extern const char *I810fbSymbols[];
extern const char *I810xaaSymbols[];

extern Bool I810DRIScreenInit(ScreenPtr pScreen);
extern void I810DRICloseScreen(ScreenPtr pScreen);
extern Bool I810DRIFinishScreenInit(ScreenPtr pScreen);
extern Bool I810InitDma(ScrnInfoPtr pScrn);
extern Bool I810CleanupDma(ScrnInfoPtr pScrn);

#define I810PTR(p) ((I810Ptr)((p)->driverPrivate))
#define I810REGPTR(p) (&(I810PTR(p)->ModeReg))

extern Bool I810CursorInit(ScreenPtr pScreen);
extern Bool I810AccelInit(ScreenPtr pScreen);
extern void I810SetPIOAccess(I810Ptr pI810);
extern void I810SetMMIOAccess(I810Ptr pI810);
extern unsigned int I810CalcWatermark( ScrnInfoPtr pScrn, double freq, 
				       Bool dcache );
extern void I810PrintErrorState( ScrnInfoPtr pScrn  );
extern int I810WaitLpRing( ScrnInfoPtr pScrn, int n, int timeout_millis );
extern void I810Sync( ScrnInfoPtr pScrn );
extern unsigned long I810LocalToPhysical( ScrnInfoPtr pScrn, 
					  unsigned long local );
extern int I810AllocLow( I810MemRange *result, I810MemRange *pool, int size );
extern int I810AllocHigh( I810MemRange *result, I810MemRange *pool, int size );
extern Bool I810AllocateFront(ScrnInfoPtr pScrn);

extern void I810SetCursorPosition(ScrnInfoPtr pScrn, int x, int y);

extern int I810AllocateGARTMemory( ScrnInfoPtr pScrn );
extern void I810FreeGARTMemory( ScrnInfoPtr pScrn );

extern Bool I810BindGARTMemory( ScrnInfoPtr pScrn );
extern Bool I810UnbindGARTMemory( ScrnInfoPtr pScrn );

extern int I810CheckAvailableMemory(ScrnInfoPtr pScrn);

extern Bool I810SwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
extern void I810AdjustFrame(int scrnIndex, int x, int y, int flags);

extern void I810SetupForScreenToScreenCopy(ScrnInfoPtr pScrn, int xdir, 
					   int ydir, int rop,
					   unsigned int planemask, 
					   int trans_color);
extern void I810SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int srcX, 
					     int srcY, int dstX, int dstY, 
					     int w, int h);
extern void I810SetupForSolidFill(ScrnInfoPtr pScrn, int color, int rop, 
				  unsigned int planemask);
extern void I810SubsequentSolidFillRect(ScrnInfoPtr pScrn, int x, int y, 
					int w, int h);

extern void I810SelectBuffer(ScrnInfoPtr pScrn, int buffer);

extern void I810RefreshRing(ScrnInfoPtr pScrn);
extern void I810EmitFlush(ScrnInfoPtr pScrn);
extern void I810EmitInvarientState(ScrnInfoPtr pScrn);

extern Bool I810DGAInit(ScreenPtr pScreen);

extern void I810InitVideo(ScreenPtr pScreen);
extern void I810InitMC(ScreenPtr pScreen);

extern const OptionInfoRec *I810AvailableOptions(int chipid, int busid);

extern Bool I810MapMem(ScrnInfoPtr pScrn);
extern Bool I810UnmapMem(ScrnInfoPtr pScrn);
extern void I810SetRingRegs(ScrnInfoPtr pScrn);

/* I830 specific functions */
extern void I830EmitInvarientState(ScrnInfoPtr pScrn);

extern Bool I830DRIScreenInit(ScreenPtr pScreen);
extern void I830DRICloseScreen(ScreenPtr pScreen);
extern Bool I830DRIFinishScreenInit(ScreenPtr pScreen);
extern Bool I830InitDma(ScrnInfoPtr pScrn);
extern Bool I830CleanupDma(ScrnInfoPtr pScrn);
extern Bool I830AccelInit(ScreenPtr pScreen);
extern void I830SetupForScreenToScreenCopy(ScrnInfoPtr pScrn, int xdir,
										   int ydir, int rop,
										   unsigned int planemask,
										   int trans_color);
extern void I830SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int srcX,
											 int srcY, int dstX, int dstY,
											 int w, int h);
extern void I830SetupForSolidFill(ScrnInfoPtr pScrn, int color, int rop,
								  unsigned int planemask);
extern void I830SubsequentSolidFillRect(ScrnInfoPtr pScrn, int x, int y,
										int w, int h);

/* Debug functions */
extern char *i810_outreg_get_addr_name(unsigned int addr, int size);
extern void i810_outreg_decode_register(unsigned int addr, unsigned int val, int size);
extern void I830PrintAllRegisters(I830RegPtr i810Reg);
extern void I830ReadAllRegisters(I810Ptr pI810, I830RegPtr i810Reg);
void DPRINTF_stub (const char *filename,int line,const char *function,const char *fmt, ...);

/* BIOS debug macro */
#define xf86ExecX86int10_wrapper(pInt, pScrn) do							\
	{																		\
	   if(I810_DEBUG & DEBUG_VERBOSE_BIOS)									\
		 {																	\
			ErrorF("\n\n\n\nExecuting (ax == 0x%x) BIOS call\n", pInt->ax);	\
			ErrorF("Checking Error state before execution\n");				\
			I810PrintErrorState(pScrn);										\
		 }																	\
	   xf86ExecX86int10(pInt);												\
	   if(I810_DEBUG & DEBUG_VERBOSE_BIOS)									\
		 {																	\
			ErrorF("Checking Error state after execution\n");				\
			usleep(50000);													\
			I810PrintErrorState(pScrn);										\
		 }																	\
	} while(0)

#define minb(p) *(volatile CARD8 *)(pI810->MMIOBase + (p))
#define moutb(p,v) *(volatile CARD8 *)(pI810->MMIOBase + (p)) = (v)

#define OUT_RING(n) {					\
   if (I810_DEBUG & DEBUG_VERBOSE_RING)			\
      ErrorF( "OUT_RING %x: %x\n", outring, n);	\
   *(volatile unsigned int *)(virt + outring) = n;	\
   outring += 4;					\
   outring &= ringmask;					\
}

#define ADVANCE_LP_RING() {					\
    pI810->LpRing.tail = outring;					\
    OUTREG(LP_RING + RING_TAIL, outring);	\
}

#define DO_RING_IDLE() do									\
	{														\
	   int _head;											\
	   int _tail;											\
	   int _i;												\
	   do													\
		 {													\
			_head = INREG(LP_RING + RING_HEAD) & HEAD_ADDR;	\
			_tail = INREG(LP_RING + RING_TAIL) & TAIL_ADDR;	\
			for(_i = 0; _i < 65535; _i++);					\
		 }													\
	   while(_head != _tail);								\
	} while(0)

#define BEGIN_LP_RING(n)											\
   unsigned int outring, ringmask;									\
   volatile unsigned char *virt;									\
   if (n>2 && (I810_DEBUG&DEBUG_ALWAYS_SYNC)) DO_RING_IDLE();		\
   if (pI810->LpRing.space < n*4) I810WaitLpRing( pScrn, n*4, 0);	\
   pI810->LpRing.space -= n*4;										\
   if (I810_DEBUG & DEBUG_VERBOSE_RING)								\
	 ErrorF( "BEGIN_LP_RING %d in %s\n", n, FUNCTION_NAME);			\
   outring = pI810->LpRing.tail;									\
   ringmask = pI810->LpRing.tail_mask;								\
   virt = pI810->LpRing.virtual_start;

/* Memory mapped register access macros */
#define INREG8(addr)        *(volatile CARD8  *)(pI810->MMIOBase + (addr))
#define INREG16(addr)       *(volatile CARD16 *)(pI810->MMIOBase + (addr))
#define INREG(addr)         *(volatile CARD32 *)(pI810->MMIOBase + (addr))

#define OUTREG8(addr, val) do														\
	{																				\
	   *(volatile CARD8 *)(pI810->MMIOBase  + (addr)) = (val);						\
	   if (I810_DEBUG&DEBUG_VERBOSE_OUTREG) {										\
		  char *_name = i810_outreg_get_addr_name(addr, 8);							\
		  ErrorF( "OUTREG8(%s, %x, %x) in %s\n", _name, addr, val, FUNCTION_NAME);	\
		  i810_outreg_decode_register(addr, val, 8);								\
	   }																			\
	} while (0)

#define OUTREG16(addr, val) do														\
	{																				\
	   *(volatile CARD16 *)(pI810->MMIOBase + (addr)) = (val);						\
	   if (I810_DEBUG&DEBUG_VERBOSE_OUTREG) {										\
		  char *_name = i810_outreg_get_addr_name(addr, 16);						\
		  ErrorF( "OUTREG16(%s, %x, %x) in %s\n", _name, addr, val, FUNCTION_NAME);	\
		  i810_outreg_decode_register(addr, val, 16);								\
	   }																			\
	} while (0)

#define OUTREG(addr, val) do														\
	{																				\
	   *(volatile CARD32 *)(pI810->MMIOBase + (addr)) = (val);						\
	   if (I810_DEBUG&DEBUG_VERBOSE_OUTREG) {										\
		  char *_name = i810_outreg_get_addr_name(addr, 32);						\
		  ErrorF( "OUTREG(%s, %x, %x) in %s\n", _name, addr, val, FUNCTION_NAME);	\
		  i810_outreg_decode_register(addr, val, 32);								\
	   }																			\
	} while (0)

/* To remove all debugging, make sure I810_DEBUG is defined as a
 * preprocessor symbol, and equal to zero.  
 */
#define I810_DEBUG 0
#ifndef I810_DEBUG
#warning "Debugging enabled - expect reduced performance"
extern int I810_DEBUG;
#endif

#define DEBUG_VERBOSE_ACCEL  0x1
#define DEBUG_VERBOSE_SYNC   0x2
#define DEBUG_VERBOSE_VGA    0x4
#define DEBUG_VERBOSE_RING   0x8
#define DEBUG_VERBOSE_OUTREG 0x10
#define DEBUG_VERBOSE_MEMORY 0x20
#define DEBUG_VERBOSE_CURSOR 0x40
#define DEBUG_ALWAYS_SYNC    0x80
#define DEBUG_VERBOSE_DRI    0x100
#define DEBUG_VERBOSE_BIOS   0x200

/* Size of the mmio region.
 */
#define I810_REG_SIZE 0x80000


#ifndef PCI_CHIP_I810
#define PCI_CHIP_I810              0x7121
#define PCI_CHIP_I810_DC100        0x7123
#define PCI_CHIP_I810_E            0x7125 
#define PCI_CHIP_I815              0x1132 
#define PCI_CHIP_I810_BRIDGE       0x7120
#define PCI_CHIP_I810_DC100_BRIDGE 0x7122
#define PCI_CHIP_I810_E_BRIDGE     0x7124
#define PCI_CHIP_I815_BRIDGE       0x1130
#endif


#define IS_I810(pI810) (pI810->PciInfo->chipType == PCI_CHIP_I810 ||	\
			pI810->PciInfo->chipType == PCI_CHIP_I810_DC100 || \
			pI810->PciInfo->chipType == PCI_CHIP_I810_E)
#define IS_I815(pI810) (pI810->PciInfo->chipType == PCI_CHIP_I815)
#define IS_I830(pI810) (pI810->PciInfo->chipType == PCI_CHIP_I830_M)


#endif
  
