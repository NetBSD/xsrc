/* $XConsortium: i128.c /main/13 1996/10/27 11:04:19 kaleb $ */
/*
 * Copyright 1995 by Robin Cutshaw <robin@XFree86.Org>
 * Copyright 1998 by Number Nine Visual Technology, Inc.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Robin Cutshaw not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Robin Cutshaw and Number Nine make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ROBIN CUTSHAW AND NUMBER NINE DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL ROBIN CUTSHAW OR NUMBER NINE BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/i128/i128.c,v 3.22.2.16 2000/09/04 00:42:33 robin Exp $ */

#include "i128.h"
#include "i128reg.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#include "xf86_Config.h"
#include "Ti302X.h"
#include "IBMRGB.h"

#include "xf86xaa.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h" 
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif  


extern char *xf86VisualNames[];
extern int defaultColorVisualClass;
extern Bool xf86ProbeFailed;

static
int i128ValidMode(
#if NeedFunctionPrototypes
    DisplayModePtr,
    Bool,
    int
#endif
); 


ScrnInfoRec i128InfoRec =
{
   FALSE,			/* Bool configured */
   -1,				/* int tmpIndex */
   -1,				/* int scrnIndex */
   i128Probe,			/* Bool (* Probe)() */
   i128Initialize,		/* Bool (* Init)() */
   i128ValidMode,		/* int (* ValidMode)() */
   i128EnterLeaveVT,		/* void (* EnterLeaveVT)() */
   (void (*)())NoopDDA,		/* void (* EnterLeaveMonitor)() */
   (void (*)())NoopDDA,		/* void (* EnterLeaveCursor)() */
   i128AdjustFrame,		/* void (* AdjustFrame)() */
   i128SwitchMode,		/* Bool (* SwitchMode)() */
   i128DPMSSet,			/* void (* DPMSSet)() */
   i128PrintIdent,		/* void (* PrintIdent)() */
   8,				/* int depth */
   {5, 6, 5},			/* xrgb weight */
   8,				/* int bitsPerPixel */
   PseudoColor,			/* int defaultVisual */
   -1, -1,			/* int virtualX,virtualY */
   -1,				/* int displayWidth */
   -1, -1, -1, -1,		/* int frameX0, frameY0, frameX1, frameY1 */
   {0, },			/* OFlagSet options */
   {0, },			/* OFlagSet clockOptions */   
   {0, },              		/* OFlagSet xconfigFlag */
   NULL,			/* char *chipset */
   NULL,			/* char *ramdac */
   {0, 0, 0, 0},		/* int dacSpeeds[MAXDACSPEEDS] */
   0,				/* int dacSpeedBpp */
   0,				/* int clocks */
   {0, },			/* int clock[MAXCLOCKS] */
   0,				/* int maxClock */
   4096,			/* int videoRam */
   0x0,                         /* int BIOSbase */  
   0,				/* unsigned long MemBase */
   240, 180,			/* int width, height */
   0,				/* unsigned long  speedup */
   NULL,			/* DisplayModePtr modes */   
   NULL,			/* DisplayModePtr pModes */   
   NULL,			/* char           *clockprog */
   -1,			        /* int textclock */
   FALSE,			/* Bool           bankedMono */
   "I128",			/* char           *name */
   {0, },			/* xrgb blackColour */
   {0, },			/* xrgb whiteColour */
   i128ValidTokens,		/* int *validTokens */
   I128_PATCHLEVEL,		/* char *patchlevel */
   0,				/* int IObase */
   0,				/* int DACbase */
   0,				/* int COPbase */
   0,				/* int POSbase */
   0,				/* int instance */
   0,				/* int s3Madjust */
   0,				/* int s3Nadjust */
   0,				/* int s3MClk */
   0,				/* int chipID */
   0,				/* int chipRev */
   0,				/* unsigned long VGAbase */
   0,				/* int s3RefClk */
   -1,				/* int s3BlankDelay */
   0,				/* int textClockFreq */
   NULL,                        /* char* DCConfig */
   NULL,                        /* char* DCOptions */
   0,				/* int MemClk */
   0				/* int LCDClk */
#ifdef XFreeXDGA
   ,0,				/* int directMode */
   NULL,			/* Set Vid Page */
   0,				/* unsigned long physBase */
   0				/* int physSize */
#endif
};

static SymTabRec i128DacTable[] = {
   { TI3025_DAC,	"ti3025" },
   { IBM524_DAC,	"ibm524" },
   { IBM526_DAC,	"ibm526" },
   { IBM528_DAC,	"ibm528" },
   { SILVER_HAMMER_DAC,	"SilverHammerDAC" },
   { -1,		"" },
};

extern miPointerScreenFuncRec xf86PointerScreenFuncs;
Bool  (*i128ClockSelectFunc) ();
static Bool ti3025ClockSelect();

ScreenPtr i128savepScreen;
Bool  i128DAC8Bit = FALSE;
Bool  i128DACSyncOnGreen = FALSE;
Bool  i128FlatPanel = FALSE;
int i128DisplayWidth;
int i128DisplayOffset = 0;
int i128Weight;
int i128AdjustCursorXPos = 0;
pointer i128VideoMem = NULL;
struct i128io i128io;
struct i128mem i128mem;
int i128hotX, i128hotY;
Bool i128BlockCursor, i128ReloadCursor;
int i128CursorStartX, i128CursorStartY, i128CursorLines;
int i128DeviceType;
int i128MemoryType = I128_MEMORY_UNKNOWN;
int i128RamdacType = UNKNOWN_DAC;
Bool i128Doublescan = FALSE;

extern Bool xf86Exiting, xf86Resetting;

void (*i128ImageReadFunc)(
#if NeedFunctionPrototypes
    int, int, int, int, char *, int, int, int, unsigned long
#endif
);
void (*i128ImageWriteFunc)(
#if NeedFunctionPrototypes
    int, int, int, int, char *, int, int, int, short, unsigned long
#endif
);
void (*i128ImageFillFunc)(
#if NeedFunctionPrototypes
    int, int, int, int, char *, int, int, int, int, int, short, unsigned long
#endif
);


/*
 * i128PrintIdent -- print identification message
 */
void
i128PrintIdent()
{
  ErrorF("  %s: server for I128 graphics adaptors (Patchlevel %s)\n",
	 i128InfoRec.name, i128InfoRec.patchLevel);
}


/*
 * i128Probe -- find the card on the PCI bus
 */

Bool
i128Probe()
{
   DisplayModePtr pMode, pEnd;
   int i, tx, ty;
   int maxDisplayWidth, maxDisplayHeight;
   unsigned short ioaddr;
   OFlagSet validOptions;
   unsigned char n, m, p, mdc, df;
   float mclk;
   pciConfigPtr pcrp, *pcrpp;
   CARD32 tmpl, tmph, tmp;
   extern i128Registers iR;
   int PitchAlignment;

   pcrpp = xf86scanpci(i128InfoRec.scrnIndex);

   if (!pcrpp)
      return(FALSE);

   i = 0;
   while ((pcrp = pcrpp[i]) != (pciConfigPtr)NULL) {
      if (((pcrp->_device_vendor == I128_DEVICE_ID1) ||
           (pcrp->_device_vendor == I128_DEVICE_ID2) ||
           (pcrp->_device_vendor == I128_DEVICE_ID3) ||
           (pcrp->_device_vendor == I128_DEVICE_ID4)) &&
	  (pcrp->_command & PCI_CMD_IO_ENABLE) &&
	  (pcrp->_command & PCI_CMD_MEM_ENABLE)    )
        break;
      i++;
   }

   if (!pcrp)
      return(FALSE);

   iR.iobase = (unsigned short )pcrp->_base5 & 0xFF00;

   i128DeviceType = pcrp->_device_vendor;

   xf86EnableIOPorts(i128InfoRec.scrnIndex);

   i128io.rbase_g = inl(iR.iobase)        & 0xFFFFFF00;
   i128io.rbase_w = inl(iR.iobase + 0x04) & 0xFFFFFF00;
   i128io.rbase_a = inl(iR.iobase + 0x08) & 0xFFFFFF00;
   i128io.rbase_b = inl(iR.iobase + 0x0C) & 0xFFFFFF00;
   i128io.rbase_i = inl(iR.iobase + 0x10) & 0xFFFFFF00;
   i128io.rbase_e = inl(iR.iobase + 0x14) & 0xFFFF8003;
   i128io.id =      inl(iR.iobase + 0x18) & /* 0x7FFFFFFF */ 0xFFFFFFFF;
   i128io.config1 = inl(iR.iobase + 0x1C) & /* 0xF3333F1F */ 0xFF333F3F;
   i128io.config2 = inl(iR.iobase + 0x20) & 0xC1F70FFF;
   i128io.sgram   = inl(iR.iobase + 0x24) & 0xFFFFFFFF;
   i128io.soft_sw = inl(iR.iobase + 0x28) & 0x0000FFFF;
   i128io.vga_ctl = inl(iR.iobase + 0x30) & 0x0000FFFF;

#ifdef DEBUG
   ErrorF("  PCI Registers\n");
   ErrorF("    MW0_AD    0x%08x  addr 0x%08x  %spre-fetchable\n",
	    pcrp->_base0, pcrp->_base0 & 0xFFC00000,
	    pcrp->_base0 & 0x8 ? "" : "not-");
   ErrorF("    MW1_AD    0x%08x  addr 0x%08x  %spre-fetchable\n",
	    pcrp->_base1, pcrp->_base1 & 0xFFC00000,
	    pcrp->_base1 & 0x8 ? "" : "not-");
   ErrorF("    XYW_AD(A) 0x%08x  addr 0x%08x\n",
	    pcrp->_base2, pcrp->_base2 & 0xFFC00000);
   ErrorF("    XYW_AD(B) 0x%08x  addr 0x%08x\n",
	    pcrp->_base3, pcrp->_base3 & 0xFFC00000);
   ErrorF("    RBASE_G   0x%08x  addr 0x%08x\n",
	    pcrp->_base4, pcrp->_base4 & 0xFFFF0000);
   ErrorF("    IO        0x%08x  addr 0x%08x\n",
	    pcrp->_base5, pcrp->_base5 & 0xFFFFFF00);
   ErrorF("    R1        0x%08x  addr 0x%08x\n",
	    pcrp->rsvd1, pcrp->rsvd1 & 0xFFFFFF00);
   ErrorF("    R2        0x%08x  addr 0x%08x\n",
	    pcrp->rsvd2, pcrp->rsvd2 & 0xFFFFFF00);
   ErrorF("    RBASE_E   0x%08x  addr 0x%08x  %sdecode-enabled\n\n",
	    pcrp->_baserom, pcrp->_baserom & 0xFFFF8000,
	    pcrp->_baserom & 0x1 ? "" : "not-");

   ErrorF("  IO Mapped Registers\n");
   ErrorF("    RBASE_G   0x%08x  addr 0x%08x\n",
	    i128io.rbase_g, i128io.rbase_g & 0xFFFFFF00);
   ErrorF("    RBASE_W   0x%08x  addr 0x%08x\n",
	    i128io.rbase_w, i128io.rbase_w & 0xFFFFFF00);
   ErrorF("    RBASE_A   0x%08x  addr 0x%08x\n",
	    i128io.rbase_a, i128io.rbase_a & 0xFFFFFF00);
   ErrorF("    RBASE_B   0x%08x  addr 0x%08x\n",
	    i128io.rbase_b, i128io.rbase_b & 0xFFFFFF00);
   ErrorF("    RBASE_I   0x%08x  addr 0x%08x\n",
	    i128io.rbase_i, i128io.rbase_i & 0xFFFFFF00);
   ErrorF("    RBASE_E   0x%08x  addr 0x%08x  size 0x%x\n\n",
	    i128io.rbase_e, i128io.rbase_e & 0xFFFF8000, i128io.rbase_e & 0x7);

   ErrorF("  Miscellaneous IO Registers\n");
   ErrorF("    ID        0x%08x\n", i128io.id);
   ErrorF("    CONFIG1   0x%08x\n", i128io.config1);
   ErrorF("    CONFIG2   0x%08x\n", i128io.config2);
   ErrorF("    SGRAM     0x%08x\n", i128io.sgram);
   ErrorF("    SOFT_SW   0x%08x\n", i128io.soft_sw);
   ErrorF("    VGA_CTL   0x%08x\n", i128io.vga_ctl);
#endif

   iR.config1 = i128io.config1;
   iR.config2 = i128io.config2;
   iR.sgram = i128io.sgram;
   if (i128DeviceType == I128_DEVICE_ID4)
	i128io.sgram = 0x211BF030;
   else
	i128io.sgram = 0x21089030;
   /* vga_ctl is saved later */

   /* enable all of the memory mapped windows */

   i128io.config1 &= 0xFF00001F;
   i128io.config1 |= 0x00331F10;
   outl(iR.iobase + 0x1C, i128io.config1);

   if (i128DeviceType == I128_DEVICE_ID4)
		i128MemoryType = I128_MEMORY_SGRAM;
   else if (i128DeviceType == I128_DEVICE_ID3) {
	if ((i128io.config2&6) == 2)
		i128MemoryType = I128_MEMORY_SGRAM;
	else
		i128MemoryType = I128_MEMORY_WRAM;
   } else if (i128DeviceType == I128_DEVICE_ID2) {
   	if (((pcrp->_status_command & 0x03) == 0x03) &&
   	    ((pcrp->rsvd2 >>16) == 0x08))
   	   i128MemoryType = I128_MEMORY_DRAM;
   }

   i128io.config2 &= 0xFF0FFFFF;
   i128io.config2 |= 0x00100000;
   if (i128MemoryType != I128_MEMORY_SGRAM)
   	i128io.config2 |= 0x00400000;
   outl(iR.iobase + 0x20, i128io.config2);


   xf86DisableIOPorts(i128InfoRec.scrnIndex);

   xf86ProbeFailed = FALSE;

   ErrorF("%s %s: I128%s%s revision (%d)\n", 
	  XCONFIG_PROBED, i128InfoRec.name,
	  i128DeviceType == I128_DEVICE_ID2 ? "-II" :
	  i128DeviceType == I128_DEVICE_ID3 ? "-T2R (Rev3D)" :
	  i128DeviceType == I128_DEVICE_ID4 ? "-T2R4 (Rev4)" : "",
	  i128DeviceType != I128_DEVICE_ID3 ? "" :
	   i128MemoryType == I128_MEMORY_SGRAM ? "-SGRAM" : "-WRAM",
	  i128io.id&0x7);

   OFLG_ZERO(&validOptions);
   OFLG_SET(OPTION_SHOWCACHE, &validOptions);
   OFLG_SET(OPTION_DAC_8_BIT, &validOptions);
   OFLG_SET(OPTION_SYNC_ON_GREEN, &validOptions);
   OFLG_SET(OPTION_POWER_SAVER, &validOptions);
   OFLG_SET(OPTION_NOACCEL, &validOptions);
   xf86VerifyOptions(&validOptions, &i128InfoRec);

   if (xf86Verbose)
      ErrorF("%s %s: card type: PCI\n", XCONFIG_PROBED, i128InfoRec.name);

   i128InfoRec.videoRam = 0;

   if (i128DeviceType == I128_DEVICE_ID4) {
      /* Use the subsystem ID to determine the memory size */
      switch ((pcrp->rsvd2>>16) & 0x0007) {
         case 0x00:      /* 4MB card */
	    i128InfoRec.videoRam = 4 * 1024; break;
         case 0x01:      /* 8MB card */
	    i128InfoRec.videoRam = 8 * 1024; break;
         case 0x02:      /* 12MB card */
            i128InfoRec.videoRam = 12 * 1024; break;
         case 0x03:      /* 16MB card */
	    i128InfoRec.videoRam = 16 * 1024; break;
         case 0x04:      /* 20MB card */
	    i128InfoRec.videoRam = 20 * 1024; break;
         case 0x05:      /* 24MB card */
	    i128InfoRec.videoRam = 24 * 1024; break;
         case 0x06:      /* 28MB card */
	    i128InfoRec.videoRam = 28 * 1024; break;
         case 0x07:      /* 32MB card */
	    i128InfoRec.videoRam = 32 * 1024; break;
         default: /* Unknown board... */
            break;
      }
   }

   if (i128DeviceType == I128_DEVICE_ID3) {
      switch ((pcrp->rsvd2>>16)&0xFFF7) {
	 case 0x00:	/* 4MB card, no daughtercard */
	    i128InfoRec.videoRam = 4 * 1024; break;
	 case 0x01:	/* 4MB card, 4MB daughtercard */
	 case 0x04:	/* 8MB card, no daughtercard */
	    i128InfoRec.videoRam = 8 * 1024; break;
	 case 0x02:	/* 4MB card, 8MB daughtercard */
	 case 0x05:	/* 8MB card, 4MB daughtercard */
	    i128InfoRec.videoRam = 12 * 1024; break;
	 case 0x06:	/* 8MB card, 8MB daughtercard */
	    i128InfoRec.videoRam = 16 * 1024; break;
	 case 0x03:	/* 4MB card, 16 daughtercard */
	    i128InfoRec.videoRam = 20 * 1024; break;
	 case 0x07:	/* 8MB card, 16MB daughtercard */
	    i128InfoRec.videoRam = 24 * 1024; break;
	 default:
	    break;
      }
   }

   if (i128InfoRec.videoRam == 0) {
      i128InfoRec.videoRam = 2048;  /* default to 2MB */
      if (i128io.config1 & 0x04)    /* 128 bit mode   */
         i128InfoRec.videoRam <<= 1;
      if (i128io.id & 0x0400)       /* 2 banks VRAM   */
         i128InfoRec.videoRam <<= 1;
   }

   if (xf86Verbose)
      ErrorF("%s %s: videoram:  %dk\n", 
              XCONFIG_GIVEN, i128InfoRec.name, i128InfoRec.videoRam);

   if (xf86bpp < 0)
      xf86bpp = i128InfoRec.depth;

   if (xf86weight.red == 0 || xf86weight.green == 0 || xf86weight.blue == 0)
      xf86weight = i128InfoRec.weight;

   switch (xf86bpp) {
      case 8:
	 i128Weight = RGB8_PSEUDO;
         break;
      case 16:
         if (xf86weight.red==5 && xf86weight.green==5 && xf86weight.blue==5) {
	    i128InfoRec.depth = 15;
	    i128Weight = RGB16_555;
         } else if (xf86weight.red==5 &&
                    xf86weight.green==6 && xf86weight.blue==5) {
	    i128InfoRec.depth = 16;
	    i128Weight = RGB16_565;
         } else {
	    ErrorF(
	     "Invalid color weighting %1d%1d%1d (only 555 and 565 are valid)\n",
	     xf86weight.red,xf86weight.green,xf86weight.blue);
	    return(FALSE);
         }
         i128InfoRec.bitsPerPixel = 16;
         if (i128InfoRec.defaultVisual < 0)
	    i128InfoRec.defaultVisual = TrueColor;
         if (defaultColorVisualClass < 0)
	    defaultColorVisualClass = i128InfoRec.defaultVisual;
         break;
      case 24:
      case 32:
         xf86bpp = 32;
         i128InfoRec.depth = 24;
         i128InfoRec.bitsPerPixel = 32;
	 i128Weight = RGB32_888;
         xf86weight.red =  xf86weight.green = xf86weight.blue = 8;
         if (i128InfoRec.defaultVisual < 0)
	    i128InfoRec.defaultVisual = TrueColor;
         if (defaultColorVisualClass < 0)
	    defaultColorVisualClass = i128InfoRec.defaultVisual;
         break;
      default:
         ErrorF("Invalid value for bpp.  Valid values are 8, 16, 24 and 32.\n");
         return(FALSE);
   }

   if (i128InfoRec.bitsPerPixel > 8 &&
       defaultColorVisualClass >= 0 && defaultColorVisualClass != TrueColor) {
      ErrorF("Invalid default visual type: %d (%s)\n", defaultColorVisualClass,
	     xf86VisualNames[defaultColorVisualClass]);
      return(FALSE);
   }

   maxDisplayWidth = 4096;
   maxDisplayHeight = 4096;

   if (i128InfoRec.virtualX > maxDisplayWidth) {
      ErrorF("%s: Virtual width (%d) is too large.  Maximum is %d\n",
	     i128InfoRec.name, i128InfoRec.virtualX, maxDisplayWidth);
      return (FALSE);
   }
   if (i128InfoRec.virtualY > maxDisplayHeight) {
      ErrorF("%s: Virtual height (%d) is too large.  Maximum is %d\n",
	     i128InfoRec.name, i128InfoRec.virtualY, maxDisplayHeight);
      return (FALSE);
   }
 
   /* Now we can map the rest of the chip into memory */

   i128mem.mw0_ad =  (unsigned char *)xf86MapVidMem(0, 0,
			(pointer)(pcrp->_base0 & 0xFFC00000),
                        i128InfoRec.videoRam * 1024);
   i128VideoMem = (pointer )i128mem.mw0_ad;
   i128InfoRec.MemBase = pcrp->_base0 & 0xFFC00000;
#ifdef TOOMANYMMAPS
   i128mem.mw1_ad =  (unsigned char *)xf86MapVidMem(0, 1,
			(pointer)(pcrp->_base1 & 0xFFC00000),
                        i128InfoRec.videoRam * 1024);
#endif
   i128mem.xyw_ada = (unsigned char *)xf86MapVidMem(0, 2,
			(pointer)(pcrp->_base2 & 0xFFC00000),
                        4 * 1024 * 1024);  /* Never use more than 4MB here */
#if 0 /* #ifdef TOOMANYMMAPS */ /* This is never used */
   i128mem.xyw_adb = (CARD32 *)xf86MapVidMem(0, 3,
			(pointer)(pcrp->_base3 & 0xFFC00000),
                        i128InfoRec.videoRam * 1024);
#endif
   i128mem.rbase_g = (CARD32 *)xf86MapVidMem(0, 4,
			(pointer)(pcrp->_base4 & 0xFFFF0000), 64 * 1024);
   i128mem.rbase_w = i128mem.rbase_g + ( 8 * 1024)/4;
   i128mem.rbase_a = i128mem.rbase_g + (16 * 1024)/4;
   i128mem.rbase_b = i128mem.rbase_g + (24 * 1024)/4;
   i128mem.rbase_i = i128mem.rbase_g + (32 * 1024)/4;

   if (pcrp->_device_vendor == I128_DEVICE_ID1) {
      if (i128io.id & 0x0400)       /* 2 banks VRAM   */
	 i128RamdacType = IBM528_DAC;
      else
	 i128RamdacType = TI3025_DAC;
   } else if (pcrp->_device_vendor == I128_DEVICE_ID2) {
      if (i128io.id & 0x0400)       /* 2 banks VRAM   */
	 i128RamdacType = IBM528_DAC;
      else
	 i128RamdacType = IBM526_DAC;
   } else if (pcrp->_device_vendor == I128_DEVICE_ID3) {
	 i128RamdacType = IBM526_DAC;
   } else if (pcrp->_device_vendor == I128_DEVICE_ID4) {
	 i128RamdacType = SILVER_HAMMER_DAC;
   } else {
            ErrorF("%s: Unknown I128 rev (%x).\n", i128InfoRec.name,
		pcrp->_device_vendor);
            return(FALSE);
   }

   switch(i128RamdacType) {
      case TI3025_DAC:
         /* verify that the ramdac is a TVP3025 */

         i128mem.rbase_g[INDEX_TI] = TI_ID;				MB;
         if ((i128mem.rbase_g[DATA_TI]&0xFF) != TI_VIEWPOINT25_ID) {
            ErrorF("%s: Ti3025 Ramdac not found.\n", i128InfoRec.name);
            return(FALSE);
         }
         OFLG_SET(CLOCK_OPTION_TI3025, &i128InfoRec.clockOptions);
         i128InfoRec.ramdac = "ti3025";

         i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;			MB;
         i128mem.rbase_g[DATA_TI] = 0x00;				MB;
         i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;			MB;
         n = i128mem.rbase_g[DATA_TI]&0x7f;
         i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;			MB;
         i128mem.rbase_g[DATA_TI] = 0x01;				MB;
         i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;			MB;
         m = i128mem.rbase_g[DATA_TI]&0x7f;
         i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;			MB;
         i128mem.rbase_g[DATA_TI] = 0x02;				MB;
         i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;			MB;
         p = i128mem.rbase_g[DATA_TI]&0x03;
         i128mem.rbase_g[INDEX_TI] = TI_MCLK_DCLK_CONTROL;		MB;
         mdc = i128mem.rbase_g[DATA_TI]&0xFF;
         if (mdc&0x08)
	    mdc = (mdc&0x07)*2 + 2;
         else
	    mdc = 1;
         mclk = ((1431818 * ((m+2) * 8)) / (n+2) / (1 << p) / mdc + 50) / 100;

         if (xf86Verbose)
            ErrorF("%s %s: Using TI 3025 programmable clock (MCLK %1.3f MHz)\n",
	           XCONFIG_PROBED, i128InfoRec.name, mclk / 1000.0);
	 break;

      case IBM524_DAC:
         i128InfoRec.ramdac = "ibm524";
         OFLG_SET(CLOCK_OPTION_IBMRGB, &i128InfoRec.clockOptions);
         ErrorF("%s: Ramdac %s not supported.\n",
		i128InfoRec.name, i128InfoRec.ramdac);
         return(FALSE);

      case IBM526_DAC:
         /* verify that the ramdac is an IBM526 */

         i128InfoRec.ramdac = "ibm526";
	 tmph = i128mem.rbase_g[IDXH_I] & 0xFF;
	 tmpl = i128mem.rbase_g[IDXL_I] & 0xFF;
         i128mem.rbase_g[IDXH_I] = 0;					MB;
         i128mem.rbase_g[IDXL_I] = IBMRGB_id;				MB;
	 tmp = i128mem.rbase_g[DATA_I] & 0xFF;

         i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_ref_div;		MB;
	 n = i128mem.rbase_g[DATA_I] & 0x1f;
         i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_vco_div;		MB;
	 m = i128mem.rbase_g[DATA_I];
	 df = m>>6;
	 m &= 0x3f;
	 if (n == 0) { m=0; n=1; }
	 mclk = ((2517500 * (m+65)) / n / (8>>df) + 50) / 100;

	 i128mem.rbase_g[IDXL_I] = tmpl;				MB;
	 i128mem.rbase_g[IDXH_I] = tmph;				MB;
         if (tmp != 2) {
            ErrorF("%s: %s Ramdac not found.\n", i128InfoRec.name,
		i128InfoRec.ramdac);
            return(FALSE);
         }

         OFLG_SET(CLOCK_OPTION_IBMRGB, &i128InfoRec.clockOptions);

         if (xf86Verbose)
            ErrorF("%s %s: Using IBM 526 programmable clock (MCLK %1.3f MHz)\n",
	           XCONFIG_PROBED, i128InfoRec.name, mclk / 1000.0);
         break;

      case IBM528_DAC:
         /* verify that the ramdac is an IBM528 */

         i128InfoRec.ramdac = "ibm528";
	 tmph = i128mem.rbase_g[IDXH_I] & 0xFF;
	 tmpl = i128mem.rbase_g[IDXL_I] & 0xFF;
         i128mem.rbase_g[IDXH_I] = 0;					MB;
         i128mem.rbase_g[IDXL_I] = IBMRGB_id;				MB;
	 tmp = i128mem.rbase_g[DATA_I] & 0xFF;

         i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_ref_div;		MB;
	 n = i128mem.rbase_g[DATA_I] & 0x1f;
         i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_vco_div;		MB;
	 m = i128mem.rbase_g[DATA_I] & 0xFF;
	 df = m>>6;
	 m &= 0x3f;
	 if (n == 0) { m=0; n=1; }
	 mclk = ((2517500 * (m+65)) / n / (8>>df) + 50) / 100;

	 i128mem.rbase_g[IDXL_I] = tmpl;				MB;
	 i128mem.rbase_g[IDXH_I] = tmph;				MB;
         if (tmp != 2) {
            ErrorF("%s: %s Ramdac not found.\n", i128InfoRec.name,
		i128InfoRec.ramdac);
            return(FALSE);
         }

         OFLG_SET(CLOCK_OPTION_IBMRGB, &i128InfoRec.clockOptions);

         if (xf86Verbose)
            ErrorF("%s %s: Using IBM 528 programmable clock (MCLK %1.3f MHz)\n",
	           XCONFIG_PROBED, i128InfoRec.name, mclk / 1000.0);
         break;

      case SILVER_HAMMER_DAC:
         /* verify that the ramdac is a Silver Hammer */

         i128InfoRec.ramdac = "SilverHammer";
	 tmph = i128mem.rbase_g[IDXH_I] & 0xFF;
	 tmpl = i128mem.rbase_g[IDXL_I] & 0xFF;
	 tmp = i128mem.rbase_g[DATA_I] & 0xFF;

         i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_ref_div;		MB;
	 n = i128mem.rbase_g[DATA_I] & 0x1f;
         i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_vco_div;		MB;
	 m = i128mem.rbase_g[DATA_I];
	 df = m>>6;
	 m &= 0x3f;
	 if (n == 0) { m=0; n=1; }
	 mclk = ((3750000 * (m+65)) / n / (8>>df) + 50) / 100;

	 i128mem.rbase_g[IDXL_I] = tmpl;				MB;
	 i128mem.rbase_g[IDXH_I] = tmph;				MB;
         if (pcrp->_device_vendor != I128_DEVICE_ID4) {
            ErrorF("%s: %s Ramdac not found.\n", i128InfoRec.name,
		i128InfoRec.ramdac);
            return(FALSE);
         }

	 if (i128mem.rbase_g[CRT_1CON] & 0x00000100) {
            i128FlatPanel = TRUE;
            if (xf86Verbose)
               ErrorF("%s %s: Digital flat panel detected\n",
	              XCONFIG_PROBED, i128InfoRec.name);
         }

         OFLG_SET(CLOCK_OPTION_IBMRGB, &i128InfoRec.clockOptions);

         if (xf86Verbose)
            ErrorF("%s %s: Using IBM 526 programmable clock (MCLK %1.3f MHz)\n",
	           XCONFIG_PROBED, i128InfoRec.name, mclk / 1000.0);
         break;

      default:
         ErrorF("%s: Unknown Ramdac.\n", i128InfoRec.name);
         return(FALSE);
   }

   if (xf86Verbose)
      ErrorF("%s %s: Ramdac type: %s\n",
         XCONFIG_PROBED, i128InfoRec.name, i128InfoRec.ramdac);

/*
 * i128io.config2&0x80000000 is obsolete
 * rev 2 board or 8MB rev 1 board DAC is always 220 MHZ (for now)
 */
   if (i128InfoRec.dacSpeeds[0] <= 0) {
      if ((pcrp->_device_vendor == I128_DEVICE_ID2) ||
          (pcrp->_device_vendor == I128_DEVICE_ID3) ||
          (i128InfoRec.videoRam == 8192))
	 i128InfoRec.dacSpeeds[0] = 220000;
      else if (pcrp->_device_vendor == I128_DEVICE_ID4)
	 i128InfoRec.dacSpeeds[0] = 270000;
      else
	 i128InfoRec.dacSpeeds[0] = 175000;
   }
   i128InfoRec.maxClock = i128InfoRec.dacSpeeds[0];
   
   if (xf86Verbose)
      ErrorF("%s %s: Ramdac speed: %d\n",
	     OFLG_ISSET(XCONFIG_DACSPEED, &i128InfoRec.xconfigFlag) ?
	     XCONFIG_GIVEN : XCONFIG_PROBED, i128InfoRec.name,
	     i128InfoRec.dacSpeeds[0] / 1000);

   OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &i128InfoRec.clockOptions);

   tx = i128InfoRec.virtualX;
   ty = i128InfoRec.virtualY;
   pMode = i128InfoRec.modes;
   if (pMode == NULL) {
      ErrorF("No modes supplied in XF86Config\n");
      return (FALSE);
   }
   pEnd = NULL;
   do {
      DisplayModePtr pModeSv;

      pModeSv = pMode->next;
      /*
       * xf86LookupMode returns FALSE if it ran into an invalid
       * parameter 
       */
      if (!xf86LookupMode(pMode, &i128InfoRec, LOOKUP_DEFAULT)) {
	 xf86DeleteMode(&i128InfoRec, pMode);
      } else if (pMode->HDisplay > maxDisplayWidth) {
	 ErrorF("%s %s: Width of mode \"%s\" is too large (max is %d)\n",
		XCONFIG_PROBED, i128InfoRec.name, pMode->name, maxDisplayWidth);
	 xf86DeleteMode(&i128InfoRec, pMode);
      } else if (pMode->VDisplay > maxDisplayHeight) {
	 ErrorF("%s %s: Height of mode \"%s\" is too large (max is %d)\n",
		XCONFIG_PROBED, i128InfoRec.name, pMode->name, maxDisplayHeight);
	 xf86DeleteMode(&i128InfoRec, pMode);
      } else if ((pMode->HDisplay * (1 + pMode->VDisplay) *
                 (i128InfoRec.bitsPerPixel/8)) > i128InfoRec.videoRam * 1024) {
	 ErrorF("%s %s: Too little memory for mode \"%s\"\n", XCONFIG_PROBED,
		i128InfoRec.name, pMode->name);
	 xf86DeleteMode(&i128InfoRec, pMode);
      } else if (((tx > 0) && (pMode->HDisplay > tx)) ||
		 ((ty > 0) && (pMode->VDisplay > ty))) {
	 ErrorF("%s %s: Resolution %dx%d too large for virtual %dx%d\n",
		XCONFIG_PROBED, i128InfoRec.name,
		pMode->HDisplay, pMode->VDisplay, tx, ty);
	 xf86DeleteMode(&i128InfoRec, pMode);
      } else {
	 /*
	  * Successfully looked up this mode.  If pEnd isn't
	  * initialized, set it to this mode.
	  */
	 if (pEnd == (DisplayModePtr) NULL)
	    pEnd = pMode;

	 i128InfoRec.virtualX = max(i128InfoRec.virtualX, pMode->HDisplay);
	 i128InfoRec.virtualY = max(i128InfoRec.virtualY, pMode->VDisplay);
	 pMode->SynthClock = i128InfoRec.clock[pMode->Clock];
      }
      pMode = pModeSv;
   } while (pMode != pEnd);

   if ((tx != i128InfoRec.virtualX) || (ty != i128InfoRec.virtualY))
      OFLG_CLR(XCONFIG_VIRTUAL,&i128InfoRec.xconfigFlag);

#if 0
   /* The code below is whacked...
    *
    * The rules for virtualX are:
    *      On a WRAM board, the pitch must be a multiple of 128 bytes.
    *      Otherwise, the pitch must be a multiple of 256 bits.
    * There is no need to force the numbers to 800, 1024, etc.
    * The memory size check (above) should be done AFTER the pitch is changed!
    */
   if (pcrp->_device_vendor == I128_DEVICE_ID3) {
      i128DisplayWidth = i128InfoRec.virtualX;
      if ((i128InfoRec.virtualX % 128) != 0)
         i128DisplayWidth +=  128 - (i128InfoRec.virtualX % 128);
   } else if (i128InfoRec.virtualX <= 640)
      i128DisplayWidth = 640;
   else if (i128InfoRec.virtualX <= 800)
      i128DisplayWidth = 800;
   else if (i128InfoRec.virtualX <= 1024)
      i128DisplayWidth = 1024;
   else if (i128InfoRec.virtualX <= 1152)
      i128DisplayWidth = 1152;
   else if (i128InfoRec.virtualX <= 1280)
      i128DisplayWidth = 1280;
   else if (i128InfoRec.virtualX <= 1600)
      i128DisplayWidth = 1600;
   else if (i128InfoRec.virtualX <= 1920)
      i128DisplayWidth = 1920;
   else
      i128DisplayWidth = 2048;
#else
   i128DisplayWidth = i128InfoRec.virtualX;
   /* Normally, the pitch must be a multiple of 256 bits */
   PitchAlignment = 256;

   /* For WRAM, the pitch must be a multiple of 256 bytes */
   if (i128MemoryType == I128_MEMORY_WRAM)
      PitchAlignment = 256 * 8;

   PitchAlignment /= xf86bpp;
   if ((i128InfoRec.virtualX % PitchAlignment) != 0)
         i128DisplayWidth +=  PitchAlignment - (i128InfoRec.virtualX % PitchAlignment);
#endif

   if (i128InfoRec.videoRam > 4096 &&
       i128MemoryType != I128_MEMORY_DRAM &&
       i128MemoryType != I128_MEMORY_SGRAM)
      i128DisplayOffset = 0x400000L %
		          (i128DisplayWidth * (i128InfoRec.bitsPerPixel/8));

   i128VideoMem = (pointer)&((char *)i128VideoMem)[i128DisplayOffset];

   if (OFLG_ISSET(OPTION_DAC_8_BIT, &i128InfoRec.options) ||
       (i128InfoRec.bitsPerPixel > 8))
      i128DAC8Bit = TRUE;

   if (xf86bpp == 8)
	 xf86weight.green = (i128DAC8Bit ? 8 : 6);  /* for XAA */

   if (i128DAC8Bit && xf86Verbose && i128InfoRec.bitsPerPixel == 8)
      ErrorF("%s %s: Putting RAMDAC into 8-bit mode\n",
         XCONFIG_GIVEN, i128InfoRec.name);

   if (OFLG_ISSET(OPTION_SYNC_ON_GREEN, &i128InfoRec.options)) {
      i128DACSyncOnGreen = TRUE;
      if (xf86Verbose)
	 ErrorF("%s %s: Putting RAMDAC into sync-on-green mode\n",
		   XCONFIG_GIVEN, i128InfoRec.name);
   }

   if (xf86Verbose) {
      if (i128InfoRec.bitsPerPixel == 8)
	 ErrorF("%s %s: Using %d bits per RGB value\n",
		XCONFIG_PROBED, i128InfoRec.name,
		i128DAC8Bit ?  8 : 6);
      else if (i128InfoRec.bitsPerPixel == 16)
	 ErrorF("%s %s: Using 16 bpp.  Color weight: %1d%1d%1d\n",
		XCONFIG_GIVEN, i128InfoRec.name, xf86weight.red,
		xf86weight.green, xf86weight.blue);
      else if (i128InfoRec.bitsPerPixel == 32)
	 ErrorF("%s %s: Using sparse 32 bpp.  Color weight: %1d%1d%1d\n",
		XCONFIG_GIVEN, i128InfoRec.name, xf86weight.red,
		xf86weight.green, xf86weight.blue);
   }

   if (xf86Verbose) {
      ErrorF("%s %s: Virtual resolution set to %dx%d\n", 
             OFLG_ISSET(XCONFIG_VIRTUAL,&i128InfoRec.xconfigFlag) ?
                 XCONFIG_GIVEN : XCONFIG_PROBED,
             i128InfoRec.name,
	     i128InfoRec.virtualX, i128InfoRec.virtualY);
   }

#ifdef DPMSExtension
   if (DPMSEnabledSwitch ||
       (OFLG_ISSET(OPTION_POWER_SAVER, &i128InfoRec.options) &&
	!DPMSDisabledSwitch))
      defaultDPMSEnabled = DPMSEnabled = TRUE;
#endif

#ifdef XFreeXDGA
   i128InfoRec.displayWidth = i128DisplayWidth;
   i128InfoRec.directMode = XF86DGADirectPresent;
#endif

   /* Free PCI information */
   xf86cleanpci();

   return TRUE;  /* End of i128Probe() */
}


Bool
i128ProgramIBMRGB(freq, flags)
     int   freq;
     int   flags;

{
   unsigned char tmp, tmp2, m, n, df, best_m, best_n, best_df, max_n;
   CARD32 tmpl, tmph, tmpc;
   long f, vrf, outf, best_vrf, best_diff, best_outf, diff;
   long requested_freq;

#define REF_FREQ	 25175000
#define MAX_VREF	  3380000
/* Actually, MIN_VREF can be as low as 1000000;
 * this allows clock speeds down to 17 MHz      */
#define MIN_VREF	  1500000
#define MAX_VCO		220000000
#define MIN_VCO		 65000000

   if (freq < 25000) {
       ErrorF("%s %s: Specified dot clock (%.3f) too low for IBM RGB52x",
	      XCONFIG_PROBED, i128InfoRec.name, freq / 1000.0);
       return(FALSE);
   } else if (freq > MAX_VCO) {
       ErrorF("%s %s: Specified dot clock (%.3f) too high for IBM RGB52x",
	      XCONFIG_PROBED, i128InfoRec.name, freq / 1000.0);
       return(FALSE);
   }

   requested_freq = freq * 1000;

   best_m = best_n = best_df = 0;
   best_vrf = best_outf = 0;
   best_diff = requested_freq;  /* worst case */

   for (df=0; df<4; df++) {
   	max_n = REF_FREQ / MIN_VREF;
   	if (df < 3)
   		max_n >>= 1;
	for (n=2; n<max_n; n++)
		for (m=65; m<=128; m++) {
			vrf = REF_FREQ / n;
			if (df < 3)
				vrf >>= 1;
			if ((vrf > MAX_VREF) || (vrf < MIN_VREF))
				continue;

			f = vrf * m;
			outf = f;
			if (df < 2)
				outf >>= 2 - df;
			if ((f > MAX_VCO) || (f < MIN_VCO))
				continue;

			/* outf is a valid freq, pick the closest now */

			if ((diff = (requested_freq - outf)) < 0)
				diff = -diff;;
			if (diff < best_diff) {
				best_diff = diff;
				best_m = m;
				best_n = n;
				best_df = df;
				best_outf = outf;
			}
		}
   }

   /* do we have an acceptably close frequency? (less than 1% diff) */

   if (best_diff > (requested_freq/100)) {
       ErrorF("%s %s: Specified dot clock (%.3f) too far (best %.3f) IBM RGB52x",
	      XCONFIG_PROBED, i128InfoRec.name, requested_freq / 1000.0,
	      best_outf / 1000.0);
       return(FALSE);
   }

   i128mem.rbase_g[PEL_MASK] = 0xFF;					MB;

   tmpc = i128mem.rbase_g[IDXCTL_I] & 0xFF;
   tmph = i128mem.rbase_g[IDXH_I] & 0xFF;
   tmpl = i128mem.rbase_g[IDXL_I] & 0xFF;

   i128mem.rbase_g[IDXH_I] = 0;						MB;
   i128mem.rbase_g[IDXCTL_I] = 0;					MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_misc_clock;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xFF;
   i128mem.rbase_g[DATA_I] = tmp2 | 0x81;				MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_m0+4;				MB;
   i128mem.rbase_g[DATA_I] = (best_df<<6) | (best_m&0x3f);		MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_n0+4;				MB;
   i128mem.rbase_g[DATA_I] = best_n;					MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_pll_ctrl1;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xFF;
   i128mem.rbase_g[DATA_I] = (tmp2&0xf8) | 3;  /* 8 M/N pairs in PLL */	MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_pll_ctrl2;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xFF;
   i128mem.rbase_g[DATA_I] = (tmp2&0xf0) | 2;  /* clock number 2 */	MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_misc_clock;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xf0;
   i128mem.rbase_g[DATA_I] = tmp2 | ((flags & V_DBLCLK) ? 0x03 : 0x01);	MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_sync;				MB;
   i128mem.rbase_g[DATA_I] = ((flags & V_PHSYNC) ? 0x10 : 0x00)
                           | ((flags & V_PVSYNC) ? 0x20 : 0x00);	MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_hsync_pos;				MB;
   i128mem.rbase_g[DATA_I] = 0x01;  /* Delay syncs by 1 pclock */	MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_pwr_mgmt;				MB;
   i128mem.rbase_g[DATA_I] = 0x00;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_dac_op;				MB;
   tmp2 = (i128RamdacType == IBM528_DAC) ? 0x02 : 0x00;  /* fast slew */
   if (i128DACSyncOnGreen) tmp2 |= 0x08;
   i128mem.rbase_g[DATA_I] = tmp2;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_pal_ctrl;				MB;
   i128mem.rbase_g[DATA_I] = 0x00;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk;				MB;
   i128mem.rbase_g[DATA_I] = 0x01;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_misc1;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xbc;
   if ((i128MemoryType != I128_MEMORY_DRAM) &&
       (i128MemoryType != I128_MEMORY_SGRAM))
   	tmp2 |= (i128RamdacType == IBM528_DAC) ? 3 : 1;
   i128mem.rbase_g[DATA_I] = tmp2;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_misc2;				MB;
   tmp2 = 0x03;
   if (i128DAC8Bit)
	tmp2 |= 0x04;
   if (!((i128MemoryType == I128_MEMORY_DRAM) &&
	 (i128InfoRec.bitsPerPixel > 16)))
	tmp2 |= 0x40;
   if ((i128MemoryType == I128_MEMORY_SGRAM) &&
	 (i128InfoRec.bitsPerPixel > 16) &&
         (i128RamdacType != SILVER_HAMMER_DAC) )
	tmp2 &= 0x3F;
   i128mem.rbase_g[DATA_I] = tmp2;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_misc3;				MB;
   i128mem.rbase_g[DATA_I] = 0x00;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_misc4;				MB;
   i128mem.rbase_g[DATA_I] = 0x00;					MB;

   /* ?? There is no write to cursor control register */

   if (i128RamdacType == IBM526_DAC) {
	if (i128MemoryType == I128_MEMORY_SGRAM) {
	    i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_ref_div;		MB;
	    i128mem.rbase_g[DATA_I] = 0x09;				MB;
	    i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_vco_div;		MB;
	    i128mem.rbase_g[DATA_I] = 0x83;				MB;
	} else {
	/* program mclock to 52MHz */
	    i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_ref_div;		MB;
	    i128mem.rbase_g[DATA_I] = 0x08;				MB;
	    i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_vco_div;		MB;
	    i128mem.rbase_g[DATA_I] = 0x41;				MB;
	}
	/* should delay at least a millisec so we'll wait 50 */
   	usleep(50000);
   }

   switch (i128InfoRec.depth) {
   	case 24: /* 32 bit */
   		i128mem.rbase_g[IDXL_I] = IBMRGB_pix_fmt;		MB;
   		tmp2 = i128mem.rbase_g[DATA_I] & 0xf8;
   		i128mem.rbase_g[DATA_I] = tmp2 | 0x06;			MB;
   		i128mem.rbase_g[IDXL_I] = IBMRGB_32bpp;			MB;
   		i128mem.rbase_g[DATA_I] = 0x03;				MB;
   		break;
	case 16:
   		i128mem.rbase_g[IDXL_I] = IBMRGB_pix_fmt;		MB;
   		tmp2 = i128mem.rbase_g[DATA_I] & 0xf8;
   		i128mem.rbase_g[DATA_I] = tmp2 | 0x04;			MB;
   		i128mem.rbase_g[IDXL_I] = IBMRGB_16bpp;			MB;
   		i128mem.rbase_g[DATA_I] = 0xC7;				MB;
   		break;
	case 15:
   		i128mem.rbase_g[IDXL_I] = IBMRGB_pix_fmt;		MB;
   		tmp2 = i128mem.rbase_g[DATA_I] & 0xf8;
   		i128mem.rbase_g[DATA_I] = tmp2 | 0x04;			MB;
   		i128mem.rbase_g[IDXL_I] = IBMRGB_16bpp;			MB;
   		i128mem.rbase_g[DATA_I] = 0xC5;				MB;
   		break;
	default: /* 8 bit */
   		i128mem.rbase_g[IDXL_I] = IBMRGB_pix_fmt;		MB;
   		tmp2 = i128mem.rbase_g[DATA_I] & 0xf8;
   		i128mem.rbase_g[DATA_I] = tmp2 | 0x03;			MB;
   		i128mem.rbase_g[IDXL_I] = IBMRGB_8bpp;			MB;
   		i128mem.rbase_g[DATA_I] = 0x00;				MB;
   		break;
   }

   i128mem.rbase_g[IDXCTL_I] = tmpc;					MB;
   i128mem.rbase_g[IDXH_I] = tmph;					MB;
   i128mem.rbase_g[IDXL_I] = tmpl;					MB;

   return(TRUE);
}


Bool
i128ProgramSilverHammerDAC(freq, flags, skew)
     int   freq;
     int   flags;
     int   skew;
{
   /* The SilverHammer DAC is essentially the same as the IBMRGBxxx DACs,
    * but with fewer options and a different reference frequency.
    */

   unsigned char tmp, tmp2, m, n, df, best_m, best_n, best_df, max_n;
   CARD32 tmpl, tmph, tmpc;
   long f, vrf, outf, best_vrf, best_diff, best_outf, diff;
   long requested_freq;

#undef  REF_FREQ
#define REF_FREQ	 37500000
#undef  MAX_VREF
#define MAX_VREF	  9000000
#define MIN_VREF	  1500000
#undef  MAX_VCO
#define MAX_VCO		270000000
#define MIN_VCO		 65000000

   if (freq < 25000) {
       ErrorF("%s %s: Specified dot clock (%.3f) too low for SilverHammer",
	      XCONFIG_PROBED, i128InfoRec.name, freq / 1000.0);
       return(FALSE);
   } else if (freq > MAX_VCO) {
       ErrorF("%s %s: Specified dot clock (%.3f) too high for SilverHammer",
	      XCONFIG_PROBED, i128InfoRec.name, freq / 1000.0);
       return(FALSE);
   }

   requested_freq = freq * 1000;

   best_m = best_n = best_df = 0;
   best_vrf = best_outf = 0;
   best_diff = requested_freq;  /* worst case */

   for (df=0; df<4; df++) {
   	max_n = REF_FREQ / MIN_VREF;
   	if (df < 3)
   		max_n >>= 1;
	for (n=2; n<max_n; n++)
		for (m=65; m<=128; m++) {
			vrf = REF_FREQ / n;
			if (df < 3)
				vrf >>= 1;
			if ((vrf > MAX_VREF) || (vrf < MIN_VREF))
				continue;

			f = vrf * m;
			outf = f;
			if (df < 2)
				outf >>= 2 - df;
			if ((f > MAX_VCO) || (f < MIN_VCO))
				continue;

			/* outf is a valid freq, pick the closest now */

			if ((diff = (requested_freq - outf)) < 0)
				diff = -diff;;
			if (diff < best_diff) {
				best_diff = diff;
				best_m = m;
				best_n = n;
				best_df = df;
				best_outf = outf;
			}
		}
   }

   /* do we have an acceptably close frequency? (less than 1% diff) */

   if (best_diff > (requested_freq/100)) {
       ErrorF("%s %s: Specified dot clock (%.3f) too far (best %.3f) SilverHammer",
	      XCONFIG_PROBED, i128InfoRec.name, requested_freq / 1000.0,
	      best_outf / 1000.0);
       return(FALSE);
   }

   i128mem.rbase_g[PEL_MASK] = 0xFF;					MB;

   tmpc = i128mem.rbase_g[IDXCTL_I] & 0xFF;
   tmph = i128mem.rbase_g[IDXH_I] & 0xFF;
   tmpl = i128mem.rbase_g[IDXL_I] & 0xFF;

   i128mem.rbase_g[IDXH_I] = 0;						MB;
   i128mem.rbase_g[IDXCTL_I] = 0;					MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_misc_clock;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xFF;
   i128mem.rbase_g[DATA_I] = tmp2 | 0x81;				MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_m0+4;				MB;
   i128mem.rbase_g[DATA_I] = (best_df<<6) | (best_m&0x3f);		MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_n0+4;				MB;
   i128mem.rbase_g[DATA_I] = best_n;					MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_pll_ctrl1;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xFF;
   i128mem.rbase_g[DATA_I] = (tmp2&0xf8) | 3;  /* 8 M/N pairs in PLL */	MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_pll_ctrl2;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xFF;
   i128mem.rbase_g[DATA_I] = (tmp2&0xf0) | 2;  /* clock number 2 */	MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_misc_clock;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xf0;
   i128mem.rbase_g[DATA_I] = tmp2 | ((flags & V_DBLCLK) ? 0x03 : 0x01);	MB;

   i128mem.rbase_g[IDXL_I] = IBMRGB_sync;				MB;
   i128mem.rbase_g[DATA_I] = ((flags & V_PHSYNC) ? 0x10 : 0x00)
                           | ((flags & V_PVSYNC) ? 0x20 : 0x00);	MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_hsync_pos;				MB;
   i128mem.rbase_g[DATA_I] = ((flags & V_HSKEW)  ? skew : 0x01);	MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_pwr_mgmt;				MB;
/* Use 0x01 below with digital flat panel to conserve energy and reduce noise */
   i128mem.rbase_g[DATA_I] = (i128FlatPanel ? 0x01 : 0x00);		MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_dac_op;				MB;
   i128mem.rbase_g[DATA_I] = (i128DACSyncOnGreen ? 0x08 : 0x00);	MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_pal_ctrl;				MB;
   i128mem.rbase_g[DATA_I] = 0x00;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk;				MB;
   i128mem.rbase_g[DATA_I] = 0x01;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_misc1;				MB;
   tmp2 = i128mem.rbase_g[DATA_I] & 0xbc;
   if ((i128MemoryType != I128_MEMORY_DRAM) &&
       (i128MemoryType != I128_MEMORY_SGRAM))
   	tmp2 |= (i128RamdacType == IBM528_DAC) ? 3 : 1;
   i128mem.rbase_g[DATA_I] = tmp2;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_misc2;				MB;
   tmp2 = 0x03;
   if (i128DAC8Bit)
	tmp2 |= 0x04;
   if (!((i128MemoryType == I128_MEMORY_DRAM) &&
	 (i128InfoRec.bitsPerPixel > 16)))
	tmp2 |= 0x40;
   if ((i128MemoryType == I128_MEMORY_SGRAM) &&
	 (i128InfoRec.bitsPerPixel > 16) &&
         (i128RamdacType != SILVER_HAMMER_DAC) )
	tmp2 &= 0x3F;
   i128mem.rbase_g[DATA_I] = tmp2;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_misc3;				MB;
   i128mem.rbase_g[DATA_I] = 0x00;					MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_misc4;				MB;
   i128mem.rbase_g[DATA_I] = 0x00;					MB;

   /* ?? There is no write to cursor control register */

   /* Set the memory clock speed to 95 MHz */
   i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_ref_div;		MB;
   i128mem.rbase_g[DATA_I] = 0x08;				MB;
   i128mem.rbase_g[IDXL_I] = IBMRGB_sysclk_vco_div;		MB;
   i128mem.rbase_g[DATA_I] = 0x50;				MB;

   /* should delay at least a millisec so we'll wait 50 */
   usleep(50000);

   switch (i128InfoRec.depth) {
   	case 24: /* 32 bit */
   		i128mem.rbase_g[IDXL_I] = IBMRGB_pix_fmt;		MB;
   		tmp2 = i128mem.rbase_g[DATA_I] & 0xf8;
   		i128mem.rbase_g[DATA_I] = tmp2 | 0x06;			MB;
   		i128mem.rbase_g[IDXL_I] = IBMRGB_32bpp;			MB;
   		i128mem.rbase_g[DATA_I] = 0x03;				MB;
   		break;
	case 16:
   		i128mem.rbase_g[IDXL_I] = IBMRGB_pix_fmt;		MB;
   		tmp2 = i128mem.rbase_g[DATA_I] & 0xf8;
   		i128mem.rbase_g[DATA_I] = tmp2 | 0x04;			MB;
   		i128mem.rbase_g[IDXL_I] = IBMRGB_16bpp;			MB;
   		i128mem.rbase_g[DATA_I] = 0xC7;				MB;
   		break;
	case 15:
   		i128mem.rbase_g[IDXL_I] = IBMRGB_pix_fmt;		MB;
   		tmp2 = i128mem.rbase_g[DATA_I] & 0xf8;
   		i128mem.rbase_g[DATA_I] = tmp2 | 0x04;			MB;
   		i128mem.rbase_g[IDXL_I] = IBMRGB_16bpp;			MB;
   		i128mem.rbase_g[DATA_I] = 0xC5;				MB;
   		break;
	default: /* 8 bit */
   		i128mem.rbase_g[IDXL_I] = IBMRGB_pix_fmt;		MB;
   		tmp2 = i128mem.rbase_g[DATA_I] & 0xf8;
   		i128mem.rbase_g[DATA_I] = tmp2 | 0x03;			MB;
   		i128mem.rbase_g[IDXL_I] = IBMRGB_8bpp;			MB;
   		i128mem.rbase_g[DATA_I] = 0x00;				MB;
   		break;
   }

   i128mem.rbase_g[IDXCTL_I] = tmpc;					MB;
   i128mem.rbase_g[IDXH_I] = tmph;					MB;
   i128mem.rbase_g[IDXL_I] = tmpl;					MB;

   return(TRUE);
}


Bool
i128ProgramTi3025(freq)
int freq;
{
   unsigned char tmp, misc_ctrl, aux_ctrl, oclk, col_key, mux1_ctrl, mux2_ctrl;
   unsigned char n, m, p;
   double ffreq, diff, mindiff;
   int ni, mi, pi;
   int best_n=32, best_m=32;

   if (freq < 20000) {
      ErrorF("%s %s: Specified dot clock (%.3f) too low for TI 3025",
	     XCONFIG_PROBED, i128InfoRec.name, freq / 1000.0);
      return(FALSE);
   }

   
#define FREQ_MIN   12000
#define FREQ_MAX  220000

   if (freq < FREQ_MIN)
      ffreq = FREQ_MIN / 1000.0;
   else if (freq > FREQ_MAX)
      ffreq = FREQ_MAX / 1000.0;
   else
      ffreq = freq / 1000.0;
   
   for(pi=0; (pi<4) && (ffreq<110.0); pi++)
      ffreq *= 2;

   if (pi==4) {
      ffreq /= 2;
      pi--;
   }
   
   /* now 110.0 <= ffreq <= 220.0 */   
   
   ffreq /= TI_REF_FREQ;
   
   /* now 7.6825 <= ffreq <= 15.3650 */
   /* the remaining formula is  ffreq = (m+2)*8 / (n+2) */
   
   mindiff = ffreq;
   
   for (ni = 1; ni <= (int)(TI_REF_FREQ/0.5 - 2); ni++) {
      mi = (int)(ffreq * (ni+2) / 8.0 + 0.5) - 2;
      if (mi < 1)
	 mi = 1;
      else if (mi > 127) 
	 mi = 127;
      
      diff = ((mi+2) * 8) / (ni+2.0) - ffreq;
      if (diff<0)
	 diff = -diff;
      
      if (diff < mindiff) {
	 mindiff = diff;
	 best_n = ni;
	 best_m = mi;
      }
   }

   n = (unsigned char )best_n;
   m = (unsigned char )best_m;
   p = (unsigned char )pi;

   tmp = i128mem.rbase_g[INDEX_TI] & 0xFF;

   /*
    * Reset the clock data index
    */
   i128mem.rbase_g[INDEX_TI] = TI_PLL_CONTROL;				MB;
   i128mem.rbase_g[DATA_TI] = 0x00;					MB;

   /*
    * Now output the clock frequency
    */
   i128mem.rbase_g[INDEX_TI] = TI_PIXEL_CLOCK_PLL_DATA;			MB;
   i128mem.rbase_g[DATA_TI] = n;					MB;
   i128mem.rbase_g[DATA_TI] = m;					MB;
   i128mem.rbase_g[DATA_TI] = p | TI_PLL_ENABLE;			MB;

#ifdef NOTYET
   /*
    * Program the MCLK to 57MHz
    */
   i128mem.rbase_g[INDEX_TI] = TI_MCLK_PLL_DATA;
   i128mem.rbase_g[DATA_TI] = 0x05;
   i128mem.rbase_g[DATA_TI] = 0x05;
   i128mem.rbase_g[DATA_TI] = 0x05;
#endif

   switch (i128InfoRec.bitsPerPixel) {
	case 8:
		misc_ctrl = (i128DAC8Bit ? TI_MC_8_BPP : 0)
			    | TI_MC_INT_6_8_CONTROL;
		aux_ctrl  = TI_AUX_SELF_CLOCK | TI_AUX_W_CMPL;
   		oclk      = TI_OCLK_S | TI_OCLK_V4 | TI_OCLK_R8;
		col_key   = TI_COLOR_KEY_CMPL;
		break;
	case 16:
		misc_ctrl = 0x00;
		aux_ctrl  = 0x00;
   		oclk      = TI_OCLK_S | TI_OCLK_V4 | TI_OCLK_R4;
		col_key   = 0x00;
		break;
	case 32:
		misc_ctrl = 0x00;
		aux_ctrl  = 0x00;
   		oclk      = TI_OCLK_S | TI_OCLK_V4 | TI_OCLK_R2;
		col_key   = 0x00;
		break;
   }
   switch(i128InfoRec.depth) {
	case 8:
		mux1_ctrl = TI_MUX1_PSEUDO_COLOR;
		mux2_ctrl = TI_MUX2_BUS_PC_D8P64;
		break;
	case 15:
		mux1_ctrl = TI_MUX1_3025D_555;
		mux2_ctrl = TI_MUX2_BUS_DC_D15P64;
		break;
	case 16:
		mux1_ctrl = TI_MUX1_3025D_565;
		mux2_ctrl = TI_MUX2_BUS_DC_D16P64;
		break;
	case 24:
		mux1_ctrl = TI_MUX1_3025D_888;
		mux2_ctrl = TI_MUX2_BUS_DC_D24P64;
		break;
   }

   i128mem.rbase_g[INDEX_TI] = TI_CURS_CONTROL;				MB;
   i128mem.rbase_g[DATA_TI] = TI_CURS_SPRITE_ENABLE | TI_CURS_X_WINDOW_MODE;MB;

   i128mem.rbase_g[INDEX_TI] = TI_TRUE_COLOR_CONTROL;			MB;
   i128mem.rbase_g[DATA_TI] = 0x00;  /* 3025 mode, vga, 8/4bit */	MB;

   i128mem.rbase_g[INDEX_TI] = TI_VGA_SWITCH_CONTROL;			MB;
   i128mem.rbase_g[DATA_TI] = 0x00;					MB;

   i128mem.rbase_g[INDEX_TI] = TI_GENERAL_CONTROL;			MB;
   i128mem.rbase_g[DATA_TI] = 0x00;					MB;

   i128mem.rbase_g[INDEX_TI] = TI_MISC_CONTROL;				MB;
   i128mem.rbase_g[DATA_TI] = misc_ctrl;				MB;

   i128mem.rbase_g[INDEX_TI] = TI_AUXILIARY_CONTROL;			MB;
   i128mem.rbase_g[DATA_TI] = aux_ctrl;					MB;

   i128mem.rbase_g[INDEX_TI] = TI_COLOR_KEY_CONTROL;			MB;
   i128mem.rbase_g[DATA_TI] = col_key;					MB;

   i128mem.rbase_g[INDEX_TI] = TI_MUX_CONTROL_1;			MB;
   i128mem.rbase_g[DATA_TI] = mux1_ctrl;				MB;

   i128mem.rbase_g[INDEX_TI] = TI_MUX_CONTROL_2;			MB;
   i128mem.rbase_g[DATA_TI] = mux2_ctrl;				MB;

   i128mem.rbase_g[INDEX_TI] = TI_INPUT_CLOCK_SELECT;			MB;
   i128mem.rbase_g[DATA_TI] = TI_ICLK_PLL;				MB;

   i128mem.rbase_g[INDEX_TI] = TI_OUTPUT_CLOCK_SELECT;			MB;
   i128mem.rbase_g[DATA_TI] = oclk;					MB;

   usleep(150000);
   return(TRUE);
}


/*
 * i128ValidMode --
 *
 */
static int
i128ValidMode(mode, verbose,flag)
DisplayModePtr mode;
Bool verbose;
int flag;
{
return MODE_OK;
}
