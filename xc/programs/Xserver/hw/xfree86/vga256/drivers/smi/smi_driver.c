/******************************************************************************\

				   Copyright (c) 1999 by Silicon Motion, Inc.
							   All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright notice
and this permission notice appear in supporting documentation, and that the name
of Silicon Motion, Inc. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Silicon Motion, Inc. and its suppliers make no representations about the
suitability of this software for any purpose.  It is provided "as is" without
express or implied warranty.

SILICON MOTION, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SILICON MOTION, INC. AND/OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
\******************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/smi/smi_driver.c,v 1.1.2.2 1999/12/11 17:43:21 hohndel Exp $ */

/* General and xf86 includes */
#include "X.h"
#include "input.h"
#include "screenint.h"
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

/* Includes for Options flags */
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

/* DGA includes */
#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

/* SMI internal includes */
#include "smi_driver.h"
#include "regsmi.h"

#define MESSAGE(x)	if (xf86Verbose) ErrorF x;
#define DSTN()		(outb(0x3C4, 0x30), (inb(0x3C5) & 0x01))

static Bool		SMIProbe(void);
static char *	SMIIdent(int);
static void		SMIEnterLeave(int);
static Bool		SMIInit(DisplayModePtr);
static int		SMIValidMode(DisplayModePtr, Bool, int);
static void *	SMISave(void *);
static void		SMIRestore(void *);
static void		SMIAdjust(int, int);
static void		SMIFbInit();
void			SMIAccelInit();
static void		SMIDisplayPowerManagementSet(int);
static void		SMISetBank(int);

/* Temporary debug function to print smi regs */
void SMIPrintRegs(vgaSMIRec *registers);

/*
 * And the data structure which defines the driver itself 
 */

vgaVideoChipRec SMI = {
  SMIProbe,             /* Bool (* ChipProbe)() */
  SMIIdent,             /* char * (* ChipIdent)() */
  SMIEnterLeave,        /* void (* ChipEnterLeave)() */
  SMIInit,              /* Bool (* ChipInit)() */
  SMIValidMode,         /* int (* ChipValidMode)() */
  SMISave,              /* void * (* ChipSave)() */
  SMIRestore,           /* void (* ChipRestore)() */
  SMIAdjust,            /* void (* ChipAdjust)() */
  vgaHWSaveScreen,		/* void (* ChipSaveScreen)() */
  (void(*)())NoopDDA,	/* void (* ChipGetMode)() */
  SMIFbInit,			/* void (* ChipFbInit)() */
  SMISetBank,			/* void (* ChipSetRead)() */
  SMISetBank,			/* void (* ChipSetWrite)() */
  SMISetBank,			/* void (* ChipSetReadWrite)() */
  0x10000,              /* int ChipMapSize */
  0x10000,              /* int ChipSegmentSize */
  16,                   /* int ChipSegmentShift */
  0xFFFF,               /* int ChipSegmentMask */
  0x00000, 0x10000,     /* int ChipReadBottom, int ChipReadTop */
  0x00000, 0x10000,     /* int ChipWriteBottom, int ChipWriteTop */
  FALSE,                /* Bool ChipUse2Banks */
  VGA_DIVIDE_VERT,      /* int ChipInterlaceType */
  {0,},                 /* OFlagSet ChipOptionFlags */
  8,                    /* int ChipRounding */
  TRUE,                 /* Bool ChipUseLinearAddressing */
  0,                    /* int ChipLinearBase */
  0,                    /* int ChipLinearSize */
  TRUE,        			/* Bool ChipHas16bpp */
  TRUE,        			/* Bool ChipHas24bpp */
  FALSE,       			/* Bool ChipHas32bpp */
  NULL,                 /* DisplayModePtr ChipBuiltinModes */
  1, 1					/* int ChipClockMulFactor, int ChipClockDivFactor */
};

/* entries must be in sequence with chipset numbers !! */
SymTabRec smiChipTable[] = {
   { SMI_UNKNOWN, "unknown"},
   { SMI_910,     "Lynx"},
   { SMI_810,	  "LynxE"},
   { SMI_820,	  "Lynx3D"},
   { SMI_710,	  "LynxEM"},
   { SMI_712,	  "LynxEM+"},
   { SMI_720,	  "Lynx3DM"},
   { -1,          ""},
   };

/* Declare the private structure which stores all internal info */

SMIPRIV smiPriv;

/* And other glabal vars to hold vga base regs and MMIO base mem pointer */

int vgaCRIndex, vgaCRReg;
pointer smiMmioMem = NULL;   /* MMIO base address */
extern vgaHWCursorRec vgaHWCursor;

/* This function returns the string name for a supported chipset */

static char *
SMIIdent(int n)
{
	static char *chipset = "smi";

	if (n > 0)
		return(NULL);
	else
		return(chipset);
}

/* The EnterLeave function which en/dis access to IO ports and ext. regs */

static void 
SMIEnterLeave(Bool enter)
{
	static int enterCalled = FALSE;
	unsigned char tmp;

#ifdef XFreeXDGA
	if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter)
		return;
#endif

	if (enter)
	{
		xf86ClearIOPortList(vga256InfoRec.scrnIndex);
		xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
		xf86EnableIOPorts(vga256InfoRec.scrnIndex);

		/* Init the vgaIOBase reg index, depends on mono/color operation */
		vgaIOBase  = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
		vgaCRIndex = vgaIOBase + 4;
		vgaCRReg   = vgaIOBase + 5;

		/* Unprotect CRTC[0-7]  */
		outb(vgaCRIndex, 0x11);		/* for register CR11 */
		tmp = inb(vgaCRReg);		/* enable CR0-7 and disable interrupts */
		outb(vgaCRReg, tmp & 0x7F);

		/* And unlock extended regs */
		outb(0x3C4, 0x33);
		tmp = inb(0x3C5);
		outb(0x3C5, tmp & ~0x20);

		enterCalled = TRUE;
	}

	else
	{
		WaitIdle();
		WaitCommandEmpty();

		if (enterCalled)
		{
			/* Protect CR[0-7] */
			outb(vgaCRIndex, 0x11);		/* for register CR11 */
			tmp = inb(vgaCRReg);		/* disable CR0-7 */
			outb(vgaCRReg, tmp | 0x80);

			/* Relock extended regs */
			outb(0x3C4, 0x33);
			tmp = inb(0x3C5);
			outb(0x3C5, tmp | 0x20);

			xf86DisableIOPorts(vga256InfoRec.scrnIndex);
			enterCalled = FALSE;
		}
	}
}

/* 
 * This function writes data to the DPR registers and validates the value
 * written.
 *
 */

static int32
SetMMIO(volatile int32 *pReg, int32 Value)
{
	int i;
	for (i = 0; i < 10; i++)
	{
		*pReg = Value;
		if (*pReg == Value) break;
	}
	return(*pReg);
}

/* 
 * This function is used to restore a video mode. It writes out all of the
 * standart VGA and extended SMI registers needed to setup a video mode.
 *
 */

static void
SMIRestore(void *restore)
{
	#define smiRestore ((vgaSMIRec*) restore)
	unsigned char tmp;
	int i;

	vgaProtect(TRUE);

	/* First reset GE to make sure nothing is going on */
	SMIGEReset(0, __LINE__,__FILE__);

	/* Restore SMI extended regs */
	outb(0x3C4, 0x17);
	outb(0x3C5, smiRestore->SR17);
	outb(0x3C4, 0x18);
	tmp = inb(0x3C5);
	outb(0x3C5, (tmp & ~0x1F) | (smiRestore->SR18 & 0x1F));
	outb(0x3C4, 0x21);
	tmp = inb(0x3C5);
	outb(0x3C5, (tmp & ~0x03) | (smiRestore->SR21 & 0x03));
	outb(0x3C4, 0x31);
	tmp = inb(0x3C5);
	outb(0x3C5, (tmp & ~0xC0) | (smiRestore->SR31 & 0xC0));
	outb(0x3C4, 0x32);
	tmp = inb(0x3C5);
	outb(0x3C5, (tmp & ~0x07) | (smiRestore->SR32 & 0x07));
	if (smiRestore->SR6B != 0xFF)
	{
		outb(0x3C4, 0x6A);
		outb(0x3C5, smiRestore->SR6A);
		outb(0x3C4, 0x6B);
		outb(0x3C5, smiRestore->SR6B);
	}
	outb(0x3C4, 0x81);
	outb(0x3C5, smiRestore->SR81);
	outb(0x3C4, 0xA0);
	outb(0x3C5, smiRestore->SRA0);

	/* Restore shadow registers */
	if (IS_AUTOCENTER(smiPriv.chip))
	{
		for (i = 0; i < 16; i++)
		{
			outb(vgaCRIndex, 0x90 + i);
			outb(vgaCRReg, smiRestore->CR90[i]);
		}
		for (i = 0; i < 14; i++)
		{
			outb(vgaCRIndex, 0xA0 + i);
			outb(vgaCRReg, smiRestore->CRA0[i]);
		}
		if (smiRestore->CR90[14] & 0x40 ||
			smiRestore->CR90[15] & 0x03 ||
			smiRestore->SR32 & 0x01)
		{
			outb(vgaCRIndex, 0x9E);
			outb(vgaCRReg, smiRestore->CR90[14] | 0x20);
		}
		else
		{
			outb(vgaCRIndex, 0x9E);
			outb(vgaCRReg, smiRestore->CR90[14] & ~0x20);
		}
		for (i = 0; i < 14; i++)
		{
			outb(vgaCRIndex, 0x40 + i);
			outb(vgaCRReg, smiRestore->CR40[i]);
		}
		outb(vgaCRIndex, 0x9E);
		outb(vgaCRReg, smiRestore->CR90[14]);
	}

	/* Restore the standard VGA registers */
	vgaHWRestore(restore);

	/* Restore shadow registers */
	if (! IS_AUTOCENTER(smiPriv.chip))
	{
		for (i = 0; i < 14; i++)
		{
			outb(vgaCRIndex, 0x40 + i);
			outb(vgaCRReg, smiRestore->CR40[i]);
		}
	}

	/* Extended mode timings registers */
	outb(vgaCRIndex, 0x33);
	outb(vgaCRReg, smiRestore->CR33);
	outb(vgaCRIndex, 0x3A);
	outb(vgaCRReg, smiRestore->CR3A);

	/* Restore the DPR registers */
	SetMMIO(&smiPriv.ptrDPR->dpr10, smiRestore->DPR10);
	SetMMIO(&smiPriv.ptrDPR->dpr1C, smiRestore->DPR1C);
	SetMMIO(&smiPriv.ptrDPR->dpr20, smiRestore->DPR20);
	SetMMIO(&smiPriv.ptrDPR->dpr24, smiRestore->DPR24);
	SetMMIO(&smiPriv.ptrDPR->dpr28, smiRestore->DPR28);
	SetMMIO(&smiPriv.ptrDPR->dpr3C, smiRestore->DPR3C);
	SetMMIO(&smiPriv.ptrDPR->dpr40, smiRestore->DPR40);
	SetMMIO(&smiPriv.ptrDPR->dpr44, smiRestore->DPR44);

	/* Restore the VPR registers */
	if (smiPriv.chip == SMI_720)
	{
		outb(vgaCRIndex, 0xC0);
		outb(vgaCRReg, smiRestore->VPR00 >> 0);
		outb(vgaCRIndex, 0xC1);
		outb(vgaCRReg, smiRestore->VPR00 >> 8);
		outb(vgaCRIndex, 0xC2);
		outb(vgaCRReg, smiRestore->VPR00 >> 16);
		outb(vgaCRIndex, 0xC3);
		outb(vgaCRReg, smiRestore->VPR00 >> 24);
		outb(vgaCRIndex, 0xD0);
		outb(vgaCRReg, smiRestore->VPR10 >> 0);
		outb(vgaCRIndex, 0xD1);
		outb(vgaCRReg, smiRestore->VPR10 >> 8);
		outb(vgaCRIndex, 0xD2);
		outb(vgaCRReg, smiRestore->VPR10 >> 16);
		outb(vgaCRIndex, 0xD3);
		outb(vgaCRReg, smiRestore->VPR10 >> 24);
	}
	else
	{
		SetMMIO(&smiPriv.ptrVPR->vpr00, smiRestore->VPR00);
		SetMMIO(&smiPriv.ptrVPR->vpr10, smiRestore->VPR10);
	}

	if (xf86Verbose > 1)
		SMIPrintRegs(smiRestore);

	vgaProtect(FALSE);
}

/* 
 * This function performs the inverse of the restore function: It saves all the
 * standard and extended registers that we are going to modify to set up a video
 * video mode.
 */

static void *
SMISave(void *save)
{
	#define smiSave ((vgaSMIRec*) save)
	unsigned char tmp;
	int i;

	save = vgaHWSave(save, sizeof(vgaSMIRec));

	/* Save SMI extended regs */
	outb(0x3C4, 0x17);
	smiSave->SR17 = inb(0x3C5);
	outb(0x3C4, 0x18);
	smiSave->SR18 = inb(0x3C5);
	outb(0x3C4, 0x21);
	smiSave->SR21 = inb(0x3C5);
	outb(0x3C4, 0x31);
	smiSave->SR31 = inb(0x3C5);
	outb(0x3C4, 0x32);
	smiSave->SR32 = inb(0x3C5);
	outb(0x3C4, 0x6A);
	smiSave->SR6A = inb(0x3C5);
	outb(0x3C4, 0x6B);
	smiSave->SR6B = inb(0x3C5);
	outb(0x3C4, 0x81);
	smiSave->SR81 = inb(0x3C5);
	outb(0x3C4, 0xA0);
	smiSave->SRA0 = inb(0x3C5);

	/* Extended mode timings registers */
	outb(vgaCRIndex, 0x33);
	smiSave->CR33 = inb(vgaCRReg);
	outb(vgaCRIndex, 0x3A);
	smiSave->CR3A = inb(vgaCRReg);

	if (IS_AUTOCENTER(smiPriv.chip))
	{
		for (i = 0; i < 16; i++)
		{
			outb(vgaCRIndex, 0x90 + i);
			smiSave->CR90[i] = inb(vgaCRReg);
		}
		for (i = 0; i < 14; i++)
		{
			outb(vgaCRIndex, 0xA0 + i);
			smiSave->CRA0[i] = inb(vgaCRReg);
		}
		if (smiSave->CR90[14] & 0x40 ||
			smiSave->CR90[15] & 0x03 ||
			smiSave->SR32 & 0x01)
		{
			outb(vgaCRIndex, 0x9E);
			outb(vgaCRReg, smiSave->CR90[14] | 0x20);
		}
		else
		{
			outb(vgaCRIndex, 0x9E);
			outb(vgaCRReg, smiSave->CR90[14] & ~0x20);
		}
	}
	for (i = 0; i < 14; i++)
	{
		outb(vgaCRIndex, 0x40 + i);
		smiSave->CR40[i] = inb(vgaCRReg);
	}

	/* Save the DPR registers */
	smiSave->DPR10 = smiPriv.ptrDPR->dpr10;
	smiSave->DPR1C = smiPriv.ptrDPR->dpr1C;
	smiSave->DPR20 = smiPriv.ptrDPR->dpr20;
	smiSave->DPR24 = smiPriv.ptrDPR->dpr24;
	smiSave->DPR28 = smiPriv.ptrDPR->dpr28;
	smiSave->DPR3C = smiPriv.ptrDPR->dpr3C;
	smiSave->DPR40 = smiPriv.ptrDPR->dpr40;
	smiSave->DPR44 = smiPriv.ptrDPR->dpr44;

	/* Restore the VPR registers */
	if (smiPriv.chip == SMI_720)
	{
		smiSave->VPR00 = 0;
		outb(vgaCRIndex, 0xC0);
		smiSave->VPR00 |= inb(vgaCRReg) << 0;
		outb(vgaCRIndex, 0xC1);
		smiSave->VPR00 |= inb(vgaCRReg) << 8;
		outb(vgaCRIndex, 0xC2);
		smiSave->VPR00 |= inb(vgaCRReg) << 16;
		outb(vgaCRIndex, 0xC3);
		smiSave->VPR00 |= inb(vgaCRReg) << 24;
		smiSave->VPR10 = 0;
		outb(vgaCRIndex, 0xD0);
		smiSave->VPR10 |= inb(vgaCRReg) << 0;
		outb(vgaCRIndex, 0xD1);
		smiSave->VPR10 |= inb(vgaCRReg) << 8;
		outb(vgaCRIndex, 0xD2);
		smiSave->VPR10 |= inb(vgaCRReg) << 16;
		outb(vgaCRIndex, 0xD3);
		smiSave->VPR10 |= inb(vgaCRReg) << 24;
	}
	else
	{
		smiSave->VPR00 = smiPriv.ptrVPR->vpr00;
		smiSave->VPR10 = smiPriv.ptrVPR->vpr10;
	}

	if (xf86Verbose > 1)
		SMIPrintRegs(smiSave);

	return(save);
}

/* 
 * This is the main probe function for the smi chipsets.
 *
 */

static Bool
SMIProbe()
{
	SMIPCIInformation *pciInfo = NULL;
	DisplayModePtr pMode, pEnd;
	unsigned char tmp, mnr, mdr;
	int mclk;
	long mmioRegBase;

	if (vga256InfoRec.chipset)
	{
		if (StrCaseCmp(vga256InfoRec.chipset, SMIIdent(0)))
			return(FALSE);
	}

	/* Start with PCI probing */

	pciInfo = smiGetPCIInfo();
	if (!pciInfo)
		return(FALSE);

	if (pciInfo->MemBase && !vga256InfoRec.MemBase)
	{
		if (pciInfo->ChipType == SMI_720)
		{
			vga256InfoRec.MemBase = pciInfo->MemBase + 0x200000;
		}
		else
		{
			vga256InfoRec.MemBase = pciInfo->MemBase;
		}
	}

	if (pciInfo->ChipType == SMI_UNKNOWN)
	{
		MESSAGE(("%s %s: Unsupported SMI chipset detected!\n", XCONFIG_PROBED,
				vga256InfoRec.name));
		return(FALSE);
	}
	else
	{
		smiPriv.chip = pciInfo->ChipType;
		MESSAGE(("%s %s: Detected SMI %s\n", XCONFIG_PROBED, vga256InfoRec.name,
				xf86TokenToString(smiChipTable, smiPriv.chip)));
		MESSAGE(("%s %s: using driver for chipset \"%s\"\n", XCONFIG_PROBED,
				vga256InfoRec.name, SMIIdent(0)));
	}

	vga256InfoRec.chipset = SMIIdent(0);

	/* Add/enable IO ports to list: call EnterLeave */
	SMIEnterLeave(ENTER);

	/* Next go on to detect amount of installed ram */
	outb(0x3C4, 0x71);
	tmp = inb(0x3C5);

	/* And compute the amount of video memory */
	switch (tmp & 0xC0)
	{
	 	case 0x00:
	 		smiPriv.MemOffScreen = 1 * 1024;
	 		break;

		case 0x40:
			smiPriv.MemOffScreen = 2 * 1024;
			break;

		case 0x80:
			smiPriv.MemOffScreen = 4 * 1024;
			break;

		case 0xC0:
			smiPriv.MemOffScreen = 6 * 1024;
			break;
	}
	if (smiPriv.chip == SMI_820)
	{
		smiPriv.MemOffScreen += 512;
	}
	else if (smiPriv.chip == SMI_720)
	{
		smiPriv.MemOffScreen = (2 * 1024) << (tmp >> 6);
	}

	if (vga256InfoRec.videoRam == 0)
	{
		/*
			WHY? It seems there is a bug in XFree86 SVGA driver. We have to
			limit memory to 4MB.
		*/
		vga256InfoRec.videoRam = min(smiPriv.MemOffScreen, 4096);
		MESSAGE(("%s %s: videoram: %dkB (present %dkB)\n", XCONFIG_PROBED,
				vga256InfoRec.name, vga256InfoRec.videoRam,
				smiPriv.MemOffScreen));
	}
	else
	{
		MESSAGE(("%s %s: videoram: %dkB (present %dkB)\n", XCONFIG_GIVEN,
				vga256InfoRec.name, vga256InfoRec.videoRam,
				smiPriv.MemOffScreen));
	}

	/* SMI built-in ramdac speeds */

	if (vga256InfoRec.dacSpeeds[3] <= 0 && vga256InfoRec.dacSpeeds[2] > 0)
		vga256InfoRec.dacSpeeds[3] = vga256InfoRec.dacSpeeds[2];

	if (vga256InfoRec.dacSpeeds[0] <= 0) vga256InfoRec.dacSpeeds[0] = 135000;
	if (vga256InfoRec.dacSpeeds[1] <= 0) vga256InfoRec.dacSpeeds[1] = 135000;
	if (vga256InfoRec.dacSpeeds[2] <= 0) vga256InfoRec.dacSpeeds[2] = 135000;
	if (vga256InfoRec.dacSpeeds[3] <= 0) vga256InfoRec.dacSpeeds[3] = 135000;

	if (vga256InfoRec.dacSpeedBpp <= 0)
		if (xf86bpp > 24 && vga256InfoRec.dacSpeeds[3] > 0)
			vga256InfoRec.dacSpeedBpp = vga256InfoRec.dacSpeeds[3];
		else if (xf86bpp >= 24 && vga256InfoRec.dacSpeeds[2] > 0)
			vga256InfoRec.dacSpeedBpp = vga256InfoRec.dacSpeeds[2];
		else if (xf86bpp > 8 && xf86bpp < 24 && vga256InfoRec.dacSpeeds[1] > 0)
			vga256InfoRec.dacSpeedBpp = vga256InfoRec.dacSpeeds[1];
		else if (xf86bpp <= 8 && vga256InfoRec.dacSpeeds[0] > 0)
			vga256InfoRec.dacSpeedBpp = vga256InfoRec.dacSpeeds[0];

	MESSAGE(("%s %s: Ramdac speed: %d MHz", OFLG_ISSET(XCONFIG_DACSPEED,
			&vga256InfoRec.xconfigFlag) ? XCONFIG_GIVEN : XCONFIG_PROBED,
			vga256InfoRec.name, vga256InfoRec.dacSpeeds[0] / 1000));
	if (vga256InfoRec.dacSpeedBpp != vga256InfoRec.dacSpeeds[0])
		MESSAGE(("  (%d MHz for %d bpp)", vga256InfoRec.dacSpeedBpp / 1000,
				xf86bpp));
	MESSAGE(("\n"));

	/* Now set RAMDAC limits */
	vga256InfoRec.maxClock = vga256InfoRec.dacSpeedBpp;

	/* Detect current MCLK and print it for user */
	outb(0x3C4, 0x6A);
	mnr = inb(0x3C5);
	outb(0x3C4, 0x6B);
	mdr = inb(0x3C5) & 0x3F;
	mclk = (1431818 * (mnr / mdr) + 50) / 100;
	MESSAGE(("%s %s: Detected current MCLK value of %1.3f MHz\n",
			XCONFIG_PROBED, vga256InfoRec.name, mclk / 1000.0));

	/* Now check if the user has specified "set_memclk" value in XConfig */
	if (vga256InfoRec.MemClk > 0)
	{
		if(vga256InfoRec.MemClk <= 100000)
		{
			MESSAGE(("%s %s: Using Memory Clock value of %1.3f MHz\n",
					OFLG_ISSET(XCONFIG_DACSPEED, &vga256InfoRec.xconfigFlag) ?
					XCONFIG_GIVEN : XCONFIG_PROBED, vga256InfoRec.name,
					vga256InfoRec.MemClk / 1000.0));
			smiPriv.MCLK = vga256InfoRec.MemClk;
		}
		else
		{
			MESSAGE(("%s %s: Memory Clock value of %1.3f MHz is larger than "
					"limit of 100 MHz\n", XCONFIG_GIVEN, vga256InfoRec.name,
					vga256InfoRec.MemClk / 1000.0));
			smiPriv.MCLK = 0;
		}
	}
	else
		smiPriv.MCLK = 0;

#ifdef DPMSExtension
	vga256InfoRec.DPMSSet = SMIDisplayPowerManagementSet;
#endif

	/* Get the panel size */
	outb(0x3C4, 0x30);
	tmp = inb(0x3C5);
	switch (tmp & 0x0C)
	{
		case 0x00:
			smiPriv.panelWidth  = 640;
			smiPriv.panelHeight = 480;
			break;

		case 0x04:
			smiPriv.panelWidth  = 800;
			smiPriv.panelHeight = 600;
			break;

		case 0x08:
			smiPriv.panelWidth  = 1024;
			smiPriv.panelHeight = 768;
			break;

		case 0x0C:
			smiPriv.panelWidth  = 1280;
			smiPriv.panelHeight = 1024;
			break;
	}

	if (xf86Verbose)
	{
		int lcdclk, h_lcd, v_lcd;
		if (OFLG_ISSET(XCONFIG_LCDCLOCK, &vga256InfoRec.xconfigFlag))
		{
			lcdclk = vga256InfoRec.LCDClk;
		}
		else
		{
			unsigned char vnr, vdr, ps;
			outb(0x3C4, 0x6C);
			vnr = inb(0x3C5);
			outb(0x3C4, 0x6D);
			vdr = inb(0x3C5) & 0x3F;
			ps  = inb(0x3C5) >> 7;
			lcdclk = (1431818 * (vnr / vdr) / (1 + ps) + 50) / 100;
		}
		outb(0x3C4, 0x30);
		tmp = inb(0x3C5);
		switch (tmp & 0x0C)
		{
			case 0x00:
				h_lcd = 640;
				v_lcd = 480;
				break;

			case 0x04:
				h_lcd = 800;
				v_lcd = 600;
				break;

			case 0x08:
				h_lcd = 1024;
				v_lcd = 768;
				break;

			case 0x0C:
				h_lcd = 1280;
				v_lcd = 1024;
				break;
		}
		MESSAGE(("%s %s: LCD size %dx%d, type = %s, clock %1.3f MHz\n",
				OFLG_ISSET(XCONFIG_LCDCLOCK, &vga256InfoRec.xconfigFlag) ?
				XCONFIG_GIVEN : XCONFIG_PROBED, vga256InfoRec.name, h_lcd,
				v_lcd, (tmp & 0x01) ? "DSTN" : "TFT", lcdclk / 1000.0));
	}

	/* And map MMIO memory, abort if we cannot */
	if (smiPriv.chip == SMI_720)
		mmioRegBase = 0x000000;
	else if (smiPriv.chip == SMI_820)
		mmioRegBase = 0x680000;
	else
		mmioRegBase = 0x400000;
	smiMmioMem = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
			(pointer)(pciInfo->MemBase + mmioRegBase), 0x10000);

	if (smiMmioMem == NULL)
		FatalError("SMI: Cannot map MMIO registers!\n");

	if (IS_NEWMMIO(smiPriv.chip))
	{
		smiPriv.ptrDPR = (DPR *) ((char *) smiMmioMem + 0x0000);
		smiPriv.ptrVPR = (VPR *) ((char *) smiMmioMem + 0x0800);
	}
	else
	{
		smiPriv.ptrDPR = (DPR *) ((char *) smiMmioMem + 0x8000);
		smiPriv.ptrVPR = (VPR *) ((char *) smiMmioMem + 0xC000);
	}
	MESSAGE(("%s %s: MMIO=%08X DPR=%08X VPR=%08X\n", XCONFIG_PROBED,
			vga256InfoRec.name, smiMmioMem, smiPriv.ptrDPR, smiPriv.ptrVPR));

	/* And finally set various possible option flags */
	vga256InfoRec.bankedMono = FALSE;

#ifdef XFreeXDGA
	vga256InfoRec.directMode = XF86DGADirectPresent;
#endif

	OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);
	OFLG_SET(OPTION_PCI_BURST_ON, &SMI.ChipOptionFlags);
	OFLG_SET(OPTION_PCI_RETRY, &SMI.ChipOptionFlags);
	OFLG_SET(OPTION_NOACCEL, &SMI.ChipOptionFlags);
	OFLG_SET(OPTION_SW_CURSOR, &SMI.ChipOptionFlags);
	OFLG_SET(OPTION_HW_CURSOR, &SMI.ChipOptionFlags);

	vga256InfoRec.speedup = 0;

	if (IS_128BIT(smiPriv.chip))
	{
		SMI.ChipRounding = 128 / 8;
	}
	else
	{
		SMI.ChipRounding = 64 / 8;
	}

	smiPriv.NoPCIRetry = 1;
	SMI.ChipLinearBase = vga256InfoRec.MemBase;
	SMI.ChipLinearSize = vga256InfoRec.videoRam * 1024;

	MESSAGE(("%s %s: frame @ 0x%08X, mmio @ 0x%08X\n", XCONFIG_PROBED,
			vga256InfoRec.name, vga256InfoRec.MemBase,
			pciInfo->MemBase + mmioRegBase));

	/* Check for DSTN panel and calculate FIFO size */
	if (DSTN())
	{
		long FIFOOffset = 0;
		outb(0x3C4, 0x46); FIFOOffset |= inb(0x3C5) << 3;
		outb(0x3C4, 0x47); FIFOOffset |= inb(0x3C5) << 11;
		outb(0x3C4, 0x49); FIFOOffset |= (inb(0x3C5) & 0x1C) << 17;
		smiPriv.MemReserved = smiPriv.MemOffScreen * 1024 - FIFOOffset;
	}
	else
	{
		/* Reserve room for hardware cursor */
		smiPriv.MemReserved = 2048;
	}

	return(TRUE);
}

/* This validates a given video mode. 
 * Right now, the checks are quite minimal.
 */

static int
SMIValidMode(DisplayModePtr mode, Bool verbose, int flag)
{
	int mem;

	/* Check horiz/vert total values */
	if (mode->Flags & V_INTERLACE)
	{
		if (verbose)
			ErrorF("%s %s: Interlace modes are not supported\n", XCONFIG_PROBED,
					vga256InfoRec.name);
		return(MODE_BAD);
	}

	/* Does the mode size match the panel size? */
	outb(0x3C4, 0x31);
	if (inb(0x3C5) & 0x01)
	{
		if (mode->HDisplay != smiPriv.panelWidth ||
			mode->VDisplay != smiPriv.panelHeight)
		{
			if (verbose)
				ErrorF("%s %s: Mode \"%s\" has invalid size %dx%d for panel "
						"%dx%d\n", XCONFIG_PROBED, vga256InfoRec.name,
						mode->name, mode->HDisplay, mode->VDisplay,
						smiPriv.panelWidth, smiPriv.panelHeight);
			return(MODE_BAD);
		}
	}

	/* We only support 640x480, 800x600 and 1024x768 modes */
	if ( !(  ((mode->HDisplay == 640)  && (mode->VDisplay == 480))
		  || ((mode->HDisplay == 800)  && (mode->VDisplay == 600))
		  || ((mode->HDisplay == 1024) && (mode->VDisplay == 768))
		  )
	   )
	{
		if (verbose)
			ErrorF("%s %s: Mode \"%s\" has invalid size %dx%d\n",
				XCONFIG_PROBED, vga256InfoRec.name, mode->name, mode->HDisplay,
				mode->VDisplay);
		return(MODE_BAD);
	}

	/* Now make sure we have enough vidram to support mode */
	mem = mode->HDisplay * (vga256InfoRec.bitsPerPixel / 8) * mode->VDisplay;
	if (mem > (1024 * smiPriv.MemOffScreen - smiPriv.MemReserved))
	{
		if (verbose)
			ErrorF("%s %s: Mode \"%s\" requires %d of videoram, only %d is "
					"available\n", XCONFIG_PROBED, vga256InfoRec.name,
					mode->name, mem, 1024 * smiPriv.MemReserved);
		return(MODE_BAD);
	}
 
	return(MODE_OK);
}

/* Used to adjust start address in frame buffer. We use the new 
 * CR69 reg for this purpose instead of the older CR31/CR51 combo.
 * If STREAMS is running, we program the STREAMS start addr. registers. 
 */

static void
SMIAdjust(int x, int y)
{
	unsigned int Base;

	Base = (y * vga256InfoRec.displayWidth + x) * (vgaBitsPerPixel / 8);

	if (smiPriv.chip == SMI_720)
	{
		while (smiPriv.ptrVPR->vpr0C & (1 << 31)) ;
		smiPriv.ptrVPR->vpr0C = (Base >> 3) | (1 << 31);
	}
	else if (smiPriv.chip == SMI_820)
	{
		while (smiPriv.ptrVPR->vpr0C & (1 << 20)) ;
		smiPriv.ptrVPR->vpr0C = (Base >> 3) | (1 << 20);
	}
	else
	{
		while (smiPriv.ptrVPR->vpr0C & (1 << 19)) ;
		smiPriv.ptrVPR->vpr0C = (Base >> 3) | (1 << 19);
	}

	if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options))
		SMIRepositionCursor(NULL);

#ifdef XFreeXDGA
	if (vga256InfoRec.directMode & XF86DGADirectGraphics)
	{
		/* Wait until vertical retrace is in progress. */
		VerticalRetraceWait();
	}
#endif
}

static void
smiCalcClock(long freq, unsigned char *num, unsigned char *den)
{
	float ffreq, ffreqDelta, ffreqBest, range;
	int clkN, clkD;
	Bool best;

	ffreq = freq / 1000.0;
	best = FALSE;

	range = 0.0;
	while (!best)
	{
		/* Next 0.5% range */
		range += 0.5 / 100;

		/* Loop through all possible numerator values */
		for (clkN = 1; clkN < 256; clkN++)
		{
			/* Loop through all possible denominator values */
			for (clkD = 1; clkD < 16; clkD++)
			{
				/* Calculate the frequency delta */
				ffreqDelta = 14.31818 * clkN / clkD - ffreq;
				if (ffreqDelta < 0)
					ffreqDelta = -ffreqDelta;

				/* Is this frequency within the range? */
				if (ffreqDelta <= ffreq * range)
				{
					if (best)
					{
						/* Is this a better one (smaller delta and smaller
						   denominator)? */
						if (ffreqDelta <= ffreqBest)
						{
							/* Store the better value */
							ffreqBest = ffreqDelta;
							*num = clkN;
							*den = clkD;
						}
					}
					else
					{
						/* Store the first value */
						best = TRUE;
						ffreqBest = ffreqDelta;
						*num = clkN;
						*den = clkD;
					}
				}
			}
		}
	}
}

static Bool
SMIInit(DisplayModePtr mode)
{
	unsigned char tmp;
	int panelIdx, modeIdx, i, width;

	MESSAGE(("SMIInit mode \"%s\" display=%dx%d virtual=%dx%d\n", mode->name,
			mode->HDisplay, mode->VDisplay, vga256InfoRec.virtualX,
			vga256InfoRec.virtualY));

	/* First we adjust the horizontal timings if needed */

	if (!vgaHWInit(mode, sizeof(vgaSMIRec)))
		return(FALSE);

	/* Now we fill in the rest of the stuff we need for the lynx */

	outb(0x3C4, 0x17);
	tmp = inb(0x3C5);
	if (!OFLG_ISSET(OPTION_PCI_BURST_ON, &vga256InfoRec.options))
		new->SR17 = tmp & ~0x20;	/* No PCI burst */
	else
		new->SR17 = tmp | 0x20;		/* PCI burst */

	outb(0x3C4, 0x18);
	tmp = inb(0x3C5);
	new->SR18 = tmp | 0x11;			/* Enable linear mode */

	outb(0x3C4, 0x21);
	tmp = inb(0x3C5);
	new->SR21 = tmp & ~0x03;		/* Enable video & drawing engines */

	outb(0x3C4, 0x31);
	tmp = inb(0x3C5);
	new->SR31 = tmp & ~0xC0;		/* Disable virtual refresh */
	if (vgaBitsPerPixel > 8)
		new->SR31 |= 0x40;

	outb(0x3C4, 0x32);
	tmp = inb(0x3C5);
	new->SR32 = tmp & ~0x07;		/* Disable centering and expansion */
	if (! IS_AUTOCENTER(smiPriv.chip))
	{
		new->SR32 |= 0x04;			/* Enable centering */
	}

	new->SRA0 = 0x00;				/* Turn off panel video */

	/* Set mode timing parameters */
	new->CR33 = 0x00;
	new->CR3A = 0x00;

	if (smiPriv.panelWidth == 640)
		panelIdx = 0;
	else if (smiPriv.panelWidth == 800)
		panelIdx = 1;
	else
		panelIdx = 2;

	if (mode->HDisplay == 640)
		modeIdx = 0;
	else if (mode->HDisplay == 800)
		modeIdx = 1;
	else
		modeIdx = 2;

	if (IS_AUTOCENTER(smiPriv.chip))
	{
		static unsigned char PanelTable[3][14] =
		{
			{ 0x5F, 0x4F, 0x00, 0x52, 0x1E, 0x0B, 0xDF, 0x00, 0xE9, 0x0B, 0x2E,
			  0x00, 0x4F, 0xDF },
			{ 0x7F, 0x63, 0x00, 0x69, 0x19, 0x72, 0x57, 0x00, 0x58, 0x0C, 0xA2,
			  0x20, 0x4F, 0xDF },
			{ 0xA3, 0x7F, 0x00, 0x83, 0x14, 0x24, 0xFF, 0x00, 0x02, 0x08, 0xA7,
			  0xE0, 0x4F, 0xDF },
		};

		for (i = 0; i < 14; i++)
		{
			new->CR40[i] = PanelTable[panelIdx][i];
		}
		new->CR90[14] = 0x63;
		new->CR90[15] = 0x00;
		if (mode->VDisplay < smiPriv.panelHeight)
			new->CRA0[6] = (smiPriv.panelHeight - mode->VDisplay) / 8;
		else
			new->CRA0[6] = 0;
		if (mode->HDisplay < smiPriv.panelWidth)
			new->CRA0[7] = (smiPriv.panelWidth - mode->HDisplay) / 16;
		else
			new->CRA0[7] = 0;
	}
	else
	{
		static unsigned char PanelTable[3][3][14] =
		{
			{ /* 640x480 panel */
				{ 0x5F, 0x4F, 0x00, 0x53, 0x00, 0x0B, 0xDF, 0x00, 0xEA, 0x0C,
				  0x2E, 0x00, 0x4F, 0xDF },
				{ 0x5F, 0x4F, 0x00, 0x53, 0x00, 0x0B, 0xDF, 0x00, 0xEA, 0x0C,
				  0x2E, 0x00, 0x4F, 0xDF },
				{ 0x5F, 0x4F, 0x00, 0x53, 0x00, 0x0B, 0xDF, 0x00, 0xEA, 0x0C,
				  0x2E, 0x00, 0x4F, 0xDF },
			},
			{ /* 800x600 panel */
				{ 0x7F, 0x59, 0x19, 0x5E, 0x8E, 0x72, 0x1C, 0x37, 0x1D, 0x00,
				  0xA2, 0x20, 0x4F, 0xDF },
				{ 0x7F, 0x63, 0x00, 0x68, 0x18, 0x72, 0x58, 0x00, 0x59, 0x0C,
				  0xE0, 0x20, 0x63, 0x57 },
				{ 0x7F, 0x63, 0x00, 0x68, 0x18, 0x72, 0x58, 0x00, 0x59, 0x0C,
				  0xE0, 0x20, 0x63, 0x57 },
			},
			{ /* 1024x768 panel */
				{ 0xA3, 0x67, 0x0F, 0x6D, 0x1D, 0x24, 0x70, 0x95, 0x72, 0x07,
				  0xA3, 0x20, 0x4F, 0xDF },
				{ 0xA3, 0x71, 0x19, 0x77, 0x07, 0x24, 0xAC, 0xD1, 0xAE, 0x03,
				  0xE1, 0x20, 0x63, 0x57 },
				{ 0xA3, 0x7F, 0x00, 0x85, 0x15, 0x24, 0xFF, 0x00, 0x01, 0x07,
				  0xE5, 0x20, 0x7F, 0xFF },
			},
		};

		for (i = 0; i < 14; i++)
		{
			new->CR40[i] = PanelTable[panelIdx][modeIdx][i];
		}
	}

	/*
		DSTN support.
	*/
	if (DSTN())
	{
		new->SR21 = 0x00;
	}

	/*
		And setup here the new value for MCLK. We use the XConfig option
		"set_mclk".
    */
	if (smiPriv.MCLK > 0)
	{
		smiCalcClock(smiPriv.MCLK, &new->SR6A, &new->SR6B);
	}
	else
	{
		new->SR6B = 0xFF;	/* This is a reserved value, so we use as flag */
	}

	/* Override the Misccellaneous Output register */
	if ( (vga256InfoRec.displayWidth == 640) && IS_AUTOCENTER(smiPriv.chip) )
		new->std.MiscOutReg &= ~0x0C;
	else
		new->std.MiscOutReg |= 0x0C;
	new->std.MiscOutReg |= 0xE0;
	if (vga256InfoRec.displayWidth == 800)
		new->std.MiscOutReg &= ~0xC0;
	if ( (vga256InfoRec.displayWidth == 1024) && IS_AUTOCENTER(smiPriv.chip) )
		new->std.MiscOutReg &= ~0xC0;

	/* Now, setup the MMIO registers */
	new->VPR00 = ((vgaBitsPerPixel - 8) / 4) << 16;
	width = (vga256InfoRec.displayWidth * (vgaBitsPerPixel / 8)) >> 3;
	new->VPR10 = width + ((width + 2) << 16);

	if (vgaBitsPerPixel == 24)
		width = vga256InfoRec.displayWidth * 3;
	else
		width = vga256InfoRec.displayWidth;
	new->DPR10 = width | (width << 16);
	if (vga256InfoRec.displayWidth == 640)
		new->DPR1C = 3 << 16;
	else if (vga256InfoRec.displayWidth == 800)
		new->DPR1C = 4 << 16;
	else if (vga256InfoRec.displayWidth == 1024)
		new->DPR1C = 5 << 16;
	if (vgaBitsPerPixel == 16)
		new->DPR1C |= (1 << 4) << 16;
	else if (vgaBitsPerPixel == 24)
		new->DPR1C |= (3 << 4) << 16;
	new->DPR20 = 0;
	new->DPR24 = 0xFFFFFFFF;
	new->DPR28 = 0xFFFFFFFF;
	new->DPR3C = width + (width << 16);
	new->DPR40 = 0;
	new->DPR44 = 0;

	return(TRUE);
}

/* This function inits the frame buffer.
 */

void 
SMIFbInit()
{
	/* Call SMIAccelInit to setup the XAA accelerated functions */
	if(!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
		SMIAccelInit();

	if(OFLG_ISSET(OPTION_PCI_RETRY, &vga256InfoRec.options))
		if(OFLG_ISSET(OPTION_PCI_BURST_ON, &vga256InfoRec.options))
		{
			smiPriv.NoPCIRetry = 0;
		}
		else
		{
			smiPriv.NoPCIRetry = 1;   
			MESSAGE(("%s %s: \"pci_retry\" option requires \"pci_burst\".\n",
					XCONFIG_GIVEN, vga256InfoRec.name));
		}

	if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options))
	{
		vgaHWCursor.Initialized   = TRUE;
		vgaHWCursor.Init          = SMICursorInit;
		vgaHWCursor.Restore       = SMIRestoreCursor;
		vgaHWCursor.Warp          = SMIWarpCursor;
		vgaHWCursor.QueryBestSize = SMIQueryBestSize;
		MESSAGE(("%s %s: %s: Using hardware cursor\n", XCONFIG_PROBED,
				vga256InfoRec.name, vga256InfoRec.chipset));
	}
}

/* This function is used to debug, it prints out the contents of lynx regs */

void
SMIPrintRegs(vgaSMIRec *registers)
{
	unsigned char tmp;
	unsigned long vpr00, vpr10;

	ErrorF("MISC=%02X\n", inb(0x3CC));

	for (tmp = 0x00; tmp <= 0x04; tmp++)
	{
		outb(0x3C4, tmp);
		ErrorF("SR%02X=%02X ", tmp, inb(0x3C5));
	}
	ErrorF("\n");
	for (tmp = 0x15; tmp <= 0x1A; tmp++)
	{
		outb(0x3C4, tmp);
		ErrorF("SR%02X=%02X ", tmp, inb(0x3C5));
	}
	ErrorF("\n");
	for (tmp = 0x20; tmp <= 0x24; tmp++)
	{
		outb(0x3C4, tmp);
		ErrorF("SR%02X=%02X ", tmp, inb(0x3C5));
	}
	ErrorF("\n");
	for (tmp = 0x30; tmp <= 0xAF; tmp++)
	{
		if (tmp == 0x35) tmp = 0x3E;
		if (tmp == 0x4D) tmp = 0x50;
		if (tmp == 0x5B) tmp = 0xA0;
		outb(0x3C4, tmp);
		ErrorF("SR%02X=%02X ", tmp, inb(0x3C5));
	}
	ErrorF("\n");
	for (tmp = 0x60; tmp <= 0x62; tmp++)
	{
		outb(0x3C4, tmp);
		ErrorF("SR%02X=%02X ", tmp, inb(0x3C5));
	}
	ErrorF("\n");
	for (tmp = 0x63; tmp <= 0x6F; tmp++)
	{
		if (tmp == 0x67) tmp = 0x68;
		outb(0x3C4, tmp);
		ErrorF("SR%02X=%02X ", tmp, inb(0x3C5));
	}
	ErrorF("\n");
	for (tmp = 0x70; tmp <= 0x75; tmp++)
	{
		outb(0x3C4, tmp);
		ErrorF("SR%02X=%02X ", tmp, inb(0x3C5));
	}
	ErrorF("\n");
	for (tmp = 0x80; tmp <= 0x93; tmp++)
	{
		if (tmp == 0x87) tmp = 0x88;
		if (tmp == 0x8E) tmp = 0x90;
		outb(0x3C4, tmp);
		ErrorF("SR%02X=%02X ", tmp, inb(0x3C5));
	}
	ErrorF("\n");

	for (tmp = 0x00; tmp <= 0x18; tmp++)
	{
		outb(vgaCRIndex, tmp);
		ErrorF("CR%02X=%02X ", tmp, inb(vgaCRReg));
	}
	ErrorF("\n");
	for (tmp = 0x30; tmp <= 0x3A; tmp++)
	{
		outb(vgaCRIndex, tmp);
		ErrorF("CR%02X=%02X ", tmp, inb(vgaCRReg));
	}
	ErrorF("\n");
	for (tmp = 0x40; tmp <= 0x4D; tmp++)
	{
		outb(vgaCRIndex, tmp);
		ErrorF("CR%02X=%02X ", tmp, inb(vgaCRReg));
	}
	ErrorF("\n");
	if (IS_AUTOCENTER(smiPriv.chip))
	{
		for (tmp = 0x90; tmp <= 0xAD; tmp++)
		{
			if (tmp == 0x9C) tmp = 0x9E;
			outb(vgaCRIndex, tmp);
			ErrorF("CR%02X=%02X ", tmp, inb(vgaCRReg));
		}
		ErrorF("\n");
	}

	if (smiPriv.chip == SMI_720)
	{
		outb(vgaCRIndex, 0xC0);
		vpr00 = inb(vgaCRReg);
		outb(vgaCRIndex, 0xC1);
		vpr00 |= inb(vgaCRReg) << 8;
		outb(vgaCRIndex, 0xC2);
		vpr00 |= inb(vgaCRReg) << 16;
		outb(vgaCRIndex, 0xC3);
		vpr00 |= inb(vgaCRReg) << 24;
		outb(vgaCRIndex, 0xD0);
		vpr10 = inb(vgaCRReg);
		outb(vgaCRIndex, 0xD1);
		vpr10 |= inb(vgaCRReg) << 8;
		outb(vgaCRIndex, 0xD2);
		vpr10 |= inb(vgaCRReg) << 16;
		outb(vgaCRIndex, 0xD3);
		vpr10 |= inb(vgaCRReg) << 24;
	}
	else
	{
		vpr00 = smiPriv.ptrVPR->vpr00;
		vpr10 = smiPriv.ptrVPR->vpr10;
	}
	ErrorF("VPR00=%08X(%08X) VPR10=%08X(%08X)\n", registers->VPR00, vpr00,
			registers->VPR10, vpr10);
	ErrorF("DPR10=%08X DPR3C=%08X DPR40=%08X DPR44=%08X\n", registers->DPR10,
			registers->DPR3C, registers->DPR40, registers->DPR44);
	ErrorF("\n");
}

#ifdef DPMSExtension
static void
SMIDisplayPowerManagementSet(int PowerManagementMode)
{
	unsigned char SR01, SR22, SR31;
	static unsigned char savedSR31, savedSR34;

	if (!xf86VTSema)
		return;

	switch (PowerManagementMode)
	{
		case DPMSModeOn:
			/* Screen: On; HSync: On, VSync: On */
			SR01 = 0x00;
			SR22 = 0x00;
			SR31 = savedSR31;
			savedSR31 = 0;
			break;

		case DPMSModeStandby:
			/* Screen: Off; HSync: Off, VSync: On */
			SR01 = 0x20;
			SR22 = 0x10;
			SR31 = 0x00;
			break;

		case DPMSModeSuspend:
			/* Screen: Off; HSync: On, VSync: Off */
			SR01 = 0x20;
			SR22 = 0x20;
			SR31 = 0x00;
			break;

		case DPMSModeOff:
			/* Screen: Off; HSync: Off, VSync: Off */
			SR01 = 0x20;
			SR22 = 0x30;
			SR31 = 0x00;
			break;
	}

	/* Save the current SR31 status */
	if (SR31 == 0 && savedSR31 == 0)
	{
		outb(0x3C4, 0x31);
		savedSR31 = inb(0x3C5);
	}

	/* Turn LCD to hardware sequencing */
	outb(0x3C4, 0x34);
	savedSR34 = inb(0x3C5);
	outb(0x3C5, savedSR34 | 0x80);

    /* Turn the screen on/off */
	outb(0x3C4, 0x01);
	SR01 |= inb(0x3C5) & ~0x20;
	outb(0x3C5, SR01);

	/* Set the DPMS mode */
	outb(0x3C4, 0x22);
	SR22 |= inb(0x3C5) & ~0x30;
	outb(0x3C5, SR22);

	/* Turn the LCD on/off */
	outb(0x3C4, 0x31);
	SR31 |= inb(0x3C5) & ~0x07;
	outb(0x3C5, SR31);

	/* Restore FPR34 */
	VerticalRetraceWait();
	outb(0x3C4, 0x34);
	outb(0x3C5, savedSR34);

}
#endif

void SMISetBank(int bank)
{
	MESSAGE(("SMISetBank: bank=%d\n", bank));
	if (smiPriv.chip == SMI_820)
	{
		bank |= 0x80;
	}
	outb(0x3C4, 0x61);
	outb(0x3C5, bank);
}
