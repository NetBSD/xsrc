/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cyrix/cyrix_driver.c,v 1.1.2.7 1999/06/23 12:37:22 hohndel Exp $ */
/*
 * Copyright 1999 by Brian Falardeau 
 * Copyright 1998 by Annius V. Groenink (A.V.Groenink@zfc.nl, avg@cwi.nl),
 *                   Dirk H. Hohndel (hohndel@suse.de),
 *                   Portions: the GGI project & confidential CYRIX databooks.
 *
 * Substitute Brian Falardeau into a copy of the following legal jargon...
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Annius Groenink not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission. Annius Groenink makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ANNIUS GROENINK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*************************************************************************/

/*
   Log for the cyrix driver source as a whole

   May 1999, Brian Falardeau:
   - Fixed interaction with SoftVGA for setting 2K pitch at 1280x1024.
   - Added CRTC tables for 60 Hz and 75 Hz modes.
   - Removed enabling display compression directly for VSA1.

   5th Nov 1998 avg  - Fixed blit buffer organization using CPU_WRITE
                       instruction.  Support for older chipsets (color
                       treatment and different CPU_WRITE opcode).

   7th Sep 1998 avg  - Fine-tuned mode switching; line-dirty compression
                       in 16bpp now no longer gives problems on the 5520
                       based system.  8x8 pattern color expansion added.

   6th Sep 1998 avg  - The 16bpp server now takes frame-dirty compression;
                       the 8bpp server can live with line-dirty compression.
                       The latter required small modifications in the
                       mode switching code.  It seems that it is not
                       perfect yet (there is a little garbage on the
                       screen before setting a graphics mode).
                       Replaced numeric bit manipulations in SoftVGA
                       registers by identifiers defined in cyrix.h.
                       Acceleration modified for 16bpp.

   5th Sep 1998 avg  - Experiments with color expansion (no satisfying
                       results).  Mono and 16 color servers fixed,
                       16bpp working.  Virtual desktop scrolling fixed
                       (now using the DC framebuffer offset directly).

   4th Sep 1998 avg  - GX_BASE and scratchpad probing fixed (outb calls
                       were reversed), blit buffer size calculation,
                       transparent solid rectangles.

   3rd Sep 1998 avg  - Bresenham lines

   2nd Sep 1998 avg  - Solid rectangles and copy areas
   
   30th Aug 1998 avg - Palette code added, switched on DriverControl (vga256)
                       Hardware cursor

   29th Aug 1998 avg - Display compression through post-SoftVGA modification
                       of the display controller registers.

   27th Aug 1998 avg - Register dump
                     - Probed value 0xC0000000 for GX_BASE replaced
                       by hard-wired 0x40000000.

   4th May 1998 avg  - Mode switch (screen save) problem fixed (start
                       address reset)
                     - Support for VGA2 and VGA16 (the latter is very
                       slow due to the single bank option)
                     - Fixed mode h-sync check

   15th Apr 1998 avg - Uploaded first 256 colour only version (binary)
 */

/*************************************************************************/

#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

#include "xf86_PCI.h"
#include "vgaPCI.h"
extern vgaPCIInformation *vgaPCIInfo;

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif


#include "vga256.h"
#include "cyrix.h"

pciTagRec CyrixPciTag;
extern vgaHWCursorRec vgaHWCursor;

#define NUM_STD_CRTC_REGS 25
#define NUM_EXT_CRTC_REGS 16

typedef struct {
	vgaHWRec std; /* IBM VGA */
	struct vgaCYRIXext
	{	
		/* override of miscellaneous output register value */

		unsigned char miscOutput;

		/* override of standard CRTC register values */

		unsigned char stdCRTCregs[NUM_STD_CRTC_REGS];

		/* extended CRTC register values (specific to MediaGX) */

		unsigned char extCRTCregs[NUM_EXT_CRTC_REGS];

		/* graphics pipeline registers */

		CARD32 GpBlitStatus;
	}	ext;
} vgaCYRIXRec, *vgaCYRIXPtr;


static Bool	CYRIXProbe();
static char *	CYRIXIdent();
static Bool	CYRIXClockSelect();
static void	CYRIXEnterLeave();
static Bool	CYRIXInit();
static int	CYRIXValidMode();
static void *	CYRIXSave();
static void	CYRIXRestore();
static void	CYRIXAdjust();

static void	CYRIXFbInit();
static Bool	CYRIXScreenInit();
static Bool	CYRIXPitchAdjust();

void	 CYRIXSetRead();
void	 CYRIXSetWrite();
void	 CYRIXSetReadWrite();

vgaVideoChipRec CYRIX = {
	CYRIXProbe,
	CYRIXIdent,
	CYRIXEnterLeave,
	CYRIXInit,
	CYRIXValidMode,
	CYRIXSave,
	CYRIXRestore,
	CYRIXAdjust,
	vgaHWSaveScreen,
	(void (*)())NoopDDA,     /* CYRIXGetMode */
	CYRIXFbInit,
	CYRIXSetRead,
	CYRIXSetWrite,
	CYRIXSetReadWrite,
	0x10000,                 /* ChipMapSize */
	0x10000,                 /* ChipSegmentSize */
	16,                      /* ChipSegmentShift */
	0xFFFF,                  /* ChipSegmentMask */
	0x00000, 0x10000,        /* ChipReadBottom/Top */
	0x00000, 0x10000,        /* ChipWriteBottom/Top */
	TRUE,                    /* ChipUse2Banks */
	VGA_NO_DIVIDE_VERT,      /* ChipInterlaceType */
	{0,},                    /* ChipOptionFlags */
	8,                       /* ChipRounding */
	TRUE,                    /* ChipUseLinearAddressing */
	0x40800000,              /* ChipLinearBase */
	0x001FFFFF,              /* ChipLinearSize */
	TRUE,                    /* ChipHas16bpp */
	FALSE,                   /* ChipHas24bpp */
	FALSE,                   /* ChipHas32bpp */
	NULL,                    /* ChipBuiltinModes */
	1,                       /* ChipClockMulFactor */
	1                        /* ChipCLockDivFactor */
};


/* access to the MediaGX video hardware registers */

char* GXregisters;

int CYRIXvsaversion;			/* VSA version */
#define CYRIX_VSA1	1
#define CYRIX_VSA2  2

int CYRIXcbufferAddress;      /* relative to video base */
int CYRIXoffscreenAddress;
int CYRIXcursorAddress;
int CYRIXcbLineDelta;         /* DWORDS */
int CYRIXoffscreenSize;       /* bytes */

int CYRIXbltBuf0Address;      /* relative to GXregisters */
int CYRIXbltBuf1Address;
int CYRIXbltBufSize;

int CYRIXisOldChipRevision;

#define newstate ((vgaCYRIXPtr)vgaNewVideoState)

typedef struct {
	int xsize;
	int ysize;
	int clock;
	unsigned char miscOutput;
	unsigned char stdCRTCregs[NUM_STD_CRTC_REGS];
	unsigned char extCRTCregs[NUM_EXT_CRTC_REGS];
} vgaCYRIXmode;

vgaCYRIXmode CYRIXmodes[] =
{
/*------------------------------------------------------------------------------*/
	{ 640, 480,         /* 640x480 */
	  25,               /* 25 MHz clock = 60 Hz refresh rate */
	  0xE3,             /* miscOutput register */
	{ 0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E, /* standard CRTC */ 
	  0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0xEA, 0x0C, 0xDF, 0x50, 0x00, 0xE7, 0x04, 0xE3, 0xFF },    
	{ 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, /* extended CRTC */
	  0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00 } },
/*------------------------------------------------------------------------------*/
	{ 800, 600,         /* 800x600 */
	  40,               /* 40 MHz clock = 60 Hz refresh rate */
	  0x23,             /* miscOutput register */
	{ 0x7F, 0x63, 0x64, 0x82, 0x6B, 0x1B, 0x72, 0xF0, /* standard CRTC */ 
	  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x59, 0x0D, 0x57, 0x64, 0x00, 0x57, 0x73, 0xE3, 0xFF },    
	{ 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, /* extended CRTC */
	  0x00, 0x00, 0x01, 0x03, 0xA0, 0x50, 0x00, 0x00 } },
/*------------------------------------------------------------------------------*/
	{ 1024, 768,        /* 1024x768 */
	  65,               /* 65 MHz clock = 60 Hz refresh rate */
	  0xE3,             /* miscOutput register */
	{ 0xA3, 0x7F, 0x80, 0x86, 0x85, 0x96, 0x24, 0xF5, /* standard CRTC */ 
	  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x04, 0x0A, 0xFF, 0x80, 0x00, 0xFF, 0x25, 0xE3, 0xFF },    
	{ 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, /* extended CRTC */
	  0x00, 0x00, 0x01, 0x03, 0x80, 0x41, 0x00, 0x00 } },
/*------------------------------------------------------------------------------*/
	{ 1280, 1024,       /* 1280x1024 */
	  108,              /* 108 MHz clock = 60 Hz refresh rate */
	  0x23,             /* miscOutput register */
	{ 0xCF, 0x9F, 0xA0, 0x92, 0xAA, 0x19, 0x28, 0x52, /* standard CRTC */ 
	  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x01, 0x04, 0xFF, 0xA0, 0x00, 0x00, 0x29, 0xE3, 0xFF },    
	{ 0x00, 0x51, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, /* extended CRTC */
	  0x00, 0x00, 0x01, 0x03, 0x80, 0x6C, 0x00, 0x00 } },
/*------------------------------------------------------------------------------*/
	{ 640, 480,         /* 640x480 */
	  31,               /* 31.5 MHz clock = 75 Hz refresh rate */
	  0xE3,             /* miscOutput register */
	{ 0x64, 0x4F, 0x4F, 0x88, 0x54, 0x9B, 0xF2, 0x1F, /* standard CRTC */ 
	  0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0xE1, 0x04, 0xDF, 0x50, 0x00, 0xDF, 0xF3, 0xE3, 0xFF },    
	{ 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, /* extended CRTC */
	  0x00, 0x00, 0x01, 0x03, 0xA0, 0x3F, 0x00, 0x00 } },
/*------------------------------------------------------------------------------*/
  	{ 800, 600,         /* 800x600 */
	  99,               /* 99 MHz clock = 75 Hz refresh rate */
	  0x23,             /* miscOutput register */
	{ 0x7F, 0x63, 0x63, 0x83, 0x68, 0x11, 0x6F, 0xF0, /* standard CRTC */ 
	  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x59, 0x1C, 0x57, 0x64, 0x00, 0x57, 0x70, 0xE3, 0xFF },    
	{ 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, /* extended CRTC */
	  0x00, 0x00, 0x01, 0x03, 0xA0, 0x63, 0x00, 0x00 } },
/*------------------------------------------------------------------------------*/
	{ 1024, 768,        /* 1024x768 */
	  79,               /* 79 MHz clock = 75 Hz refresh rate */
	  0xE3,             /* miscOutput register */
	{ 0x9F, 0x7F, 0x7F, 0x83, 0x84, 0x8F, 0x1E, 0xF5, /* standard CRTC */ 
	  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x01, 0x04, 0xFF, 0x80, 0x00, 0xFF, 0x1F, 0xE3, 0xFF },    
	{ 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, /* extended CRTC */
	  0x00, 0x00, 0x01, 0x03, 0x80, 0x4F, 0x00, 0x00 } },
/*------------------------------------------------------------------------------*/
	{ 1280, 1024,       /* 1280x1024 */
	  135,              /* 135 MHz clock = 75 Hz refresh rate */
	  0x23,             /* miscOutput register */
	{ 0xCE, 0x9F, 0x9F, 0x92, 0xA4, 0x15, 0x28, 0x52, /* standard CRTC */ 
	  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x01, 0x04, 0xFF, 0xA0, 0x00, 0x00, 0x29, 0xE3, 0xFF },    
	{ 0x00, 0x51, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, /* extended CRTC */
	  0x00, 0x00, 0x01, 0x03, 0x80, 0x87, 0x00, 0x00 } },
/*------------------------------------------------------------------------------*/
};

#define NUM_CYRIX_MODES sizeof(CYRIXmodes)/sizeof(vgaCYRIXmode)

static char *
CYRIXIdent(n)
int n;
{
	static char *chipsets[] = {"mediagx"};

	if (n + 1 > sizeof(chipsets) / sizeof(char *))
		return(NULL);
	else
		return(chipsets[n]);
}

static Bool
CYRIXProbe()
{
	pciConfigPtr pcr = NULL;
	pciConfigPtr cyrixPcr = NULL;
	pciConfigPtr companionPcr = NULL;
	unsigned char gcr, cfg;
	CARD32	id, i, physbase, padsize, videoram;
	int device_step, device_revision;

	xf86ClearIOPortList(vga256InfoRec.scrnIndex);
	xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);

	/* if the user specified a chipset it must be `mediagx' */
	if (vga256InfoRec.chipset &&
	    StrCaseCmp(vga256InfoRec.chipset, CYRIXIdent(0)))
		return (FALSE);

	/* use register probing to decide whether the chip is
	   `suitable' for us */
	CYRIXEnterLeave(ENTER);

	/* the lock register should read 0xFF after we have
	   written 0x00 to lock */
	outb(vgaIOBase + 4, CrtcExtendedRegisterLock);
	outb(vgaIOBase + 5, 0x00);
	
	if (inb(vgaIOBase + 5) != 0xFF) goto probeFailed;

	/* the lock register should read 0x00 after we have
	   writen the magic word 'WL' to unlock */
	outb(vgaIOBase + 5, 0x57);
	outb(vgaIOBase + 5, 0x4C);

	/* GGI's idea to do two comparisons */
	if (inb(vgaIOBase + 5) != 0x00) goto probeFailed;
	if (inb(vgaIOBase + 5) != 0x00)
	{	probeFailed:
		CYRIXEnterLeave(LEAVE);
		return (FALSE);
	}

	/* OK, it's most likely a MediaGX.  Now check the scratchpad
	   size.  If it is zero, we're not using the MediaGX graphics
	   facilities. */
	outb(GX_IOPORT_INDEX, GX_IOIDX_DIR0); /* doesn't work w/o that */
	outb(GX_IOPORT_INDEX, GX_IOIDX_GCR);
	gcr = inb(GX_IOPORT_DATA);

	physbase = (gcr & 3) << 30;
	padsize = (gcr & 12) ? (((gcr & 12) >> 2) + 1) : 0;

	/*	end GGI MediaGX driver based code */

	if (padsize == 0) return (FALSE);

	if (xf86Verbose > 1)
	{	ErrorF("%s %s: GX_BASE: 0x%x\n",XCONFIG_PROBED,
		       vga256InfoRec.name, physbase);
		ErrorF("%s %s: Scratchpad size: %d kbytes\n",XCONFIG_PROBED,
		       vga256InfoRec.name, padsize);
	}

	/* Probe for the MediaGX processor version details.  Older versions
	   use different op-codes for setting the organization of the
	   blit buffers within the scratch padarea.  We currently
	   also use this version ID to guess whether the chipset has
	   an external DAC (in which case we treat the colour maps
	   in a slightly different fashion) */
	outb(0x22, 0xFF);
	device_step = device_revision = inb(0x23);
	device_step >>= 8;
	device_revision &= 0xFF;
	if (xf86Verbose > 1)
		ErrorF("%s %s: MediaGX processor ID %d revision %d\n", XCONFIG_PROBED,
		       vga256InfoRec.name, device_step, device_revision);
	CYRIXisOldChipRevision = (device_step == 0 && device_revision < 40);

	/* Some  MediaGX systems have different blit buffer offsets than
	   is  indicated by the scratchpad size.  Make sure that we have
	   the  right offsets by writing them into the corresponding CPU
	   registers.  The op-codes to use depend on the processor
	   revision.  The value `40' is a guess (awaiting details from
	   Cyrix). */
	CYRIXbltBufSize = (padsize == 4) ? 1840 : (padsize == 3) ? 1328 : 816;
	CYRIXbltBuf1Address = 0x0E60 - CYRIXbltBufSize;
	CYRIXbltBuf0Address = CYRIXbltBuf1Address - CYRIXbltBufSize;
	if (CYRIXisOldChipRevision)
		CYRIXsetBlitBuffersOnOldChip();
	else
		CYRIXsetBlitBuffers();

	/* Now do PCI probing to get more detail (the correct way of
	   doing this most likely is via the pci routines that are
	   available...) [this code currently likely doesn't find
	   anything, at least not on tested systems] */
	  if (vgaPCIInfo && vgaPCIInfo->AllCards) {
	  int i=0;
	  while (pcr = vgaPCIInfo->AllCards[i++]) {
	    if (pcr->_vendor == PCI_VENDOR_CYRIX) {
	      id = pcr->_device;

	      if (vga256InfoRec.chipID) {
		ErrorF("%s %s: Cyrix chipset override, using ChipID "
		       "0x%04x instead of 0x%04x\n", XCONFIG_PROBED,
		       vga256InfoRec.name, vga256InfoRec.chipID,
		       pcr->_device);
		id = vga256InfoRec.chipID;
	      }

	      /* should add 5510 companion here, which may need a
	         different treatment due to its external RAMDAC */
	      switch(id) {
	      case PCI_CHIP_MEDIAGX:
		cyrixPcr = pcr;
		ErrorF("%s %s: Cyrix MediaGX CPU found\n",
		       XCONFIG_PROBED, vga256InfoRec.name);
		break;
	      case PCI_CHIP_CYRIX_5520:
		companionPcr = pcr;
		ErrorF("%s %s: Cyrix 5520 companion chip found\n",
		       XCONFIG_PROBED, vga256InfoRec.name);
		break;
	      case PCI_CHIP_CYRIX_5530:
		companionPcr = pcr;
		ErrorF("%s %s: Cyrix 5530 companion chip found\n",
		       XCONFIG_PROBED, vga256InfoRec.name);
		break;
	} } } }

        /* Round the amount of video RAM to megabytes.  This will
         * no longer be necessary for the 3.9 server and higher,
         * and has to do with the way the read/write masks are
         * calculated for linear frame buffer mode (for pre-3.9
         * servers, the mask is the number of video RAM bytes minus
         * one, yielding very strange results for the 2112KB the
         * MediaGX normally reports).
         */
	if (!vga256InfoRec.videoRam)
	{	int real_vram;

		outb(vgaIOBase + 4, CrtcGraphicsMemorySize);
		real_vram = (inb(vgaIOBase + 5) * 64);
		vga256InfoRec.videoRam = real_vram & 0xFFFFFC00;
	}

	vga256InfoRec.maxClock = 135000;

	/* set chipset and options */
	vga256InfoRec.chipset = CYRIXIdent(0);
	vga256InfoRec.bankedMono = TRUE;
	OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);

	/* define option set valid for the driver */
	OFLG_SET(OPTION_SW_CURSOR, &CYRIX.ChipOptionFlags);
	OFLG_SET(OPTION_HW_CURSOR, &CYRIX.ChipOptionFlags);
	OFLG_SET(OPTION_NOACCEL, &CYRIX.ChipOptionFlags);

	/* switch on hardware cursor option */
	if( !OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options) )
	    OFLG_SET(OPTION_HW_CURSOR, &vga256InfoRec.options);

	/* set pitch adjust hook (1024 or 2048 bytes only for accel) */
	vgaSetPitchAdjustHook(CYRIXPitchAdjust);

	/* Always use linear addressing for the 256 color server
	   (bank addressing fails for some reason) */
	CYRIX.ChipLinearBase  = (int)physbase + 0x800000;
	CYRIX.ChipLinearSize  = (1024 * vga256InfoRec.videoRam);

	CYRIX.ChipUseLinearAddressing = TRUE;

	/* map the entire area from GX_BASE (scratchpad area)
	   up to the end of the control registers */
	GXregisters = (char*)xf86MapVidMem(vga256InfoRec.scrnIndex,
				    EXTENDED_REGION,
				    (void*)physbase, 0x20000);

	if (!GXregisters)
	{	ErrorF("%s %s: Cannot map hardware registers\n",
			XCONFIG_PROBED, vga256InfoRec.name);
		goto probeFailed;
	}

	/* check VSA version */
	/* VSA2 contains a "CX" signature at registers 0x35 and 0x36. */
	/* The SoftVGA interface changed slightly for VSA2.  Originally, */
	/* VSA2 was intended for MXi only, but it may someday be */
	/* provided for MediaGX systems as well. */

	CYRIXvsaversion = CYRIX_VSA2;
	outb(vgaIOBase + 4, 0x35);
	if (inb(vgaIOBase + 5) != 'C') CYRIXvsaversion = CYRIX_VSA1;
	outb(vgaIOBase + 4, 0x36);
	if (inb(vgaIOBase + 5) != 'X') CYRIXvsaversion = CYRIX_VSA1;
	if (CYRIXvsaversion == CYRIX_VSA1)
	{
		ErrorF("%s %s: VSA1 detected\n", 
			XCONFIG_PROBED, vga256InfoRec.name);
	}
	else
	{
		ErrorF("%s %s: VSA2 detected\n", 
			XCONFIG_PROBED, vga256InfoRec.name);
	}
	return(TRUE);
}

/*------------------------------------------------------------------------*\
** FbInit()
** 
** From README file: "The FbInit() function is required for drivers with
** accelerated graphics support.  It is used to replace default cfb.banked
** functions with accelerated chip-specific versions.
**
** For the Cyrix driver, this routine is also used to allocate video 
** memory.  This is more complicated than it needs to be...
**
** For VSA1, SoftVGA manages all of graphics memory, including the 
** compression buffer, cursor buffer, and offscreen memory.  The driver 
** should not allocate memory itself.  For offscreen memory it reads 
** registers 0x3C and 0x3D.  For the cursor buffer it reads the hardware 
** register after validating a mode.  For compression, it just sets bit 0
** of register 0x49 if it wants to use compression, and SoftVGA will 
** enable it if memory has been allocated.
**
** This model, however, breaks down for this driver.  There is a bug in 
** SoftVGA that keeps the 0x3C register from working properly.  This bug
** also prevents compression from being enabled when using a virtual 
** desktop.  This driver also cannot use the memory past 2 Meg, which 
** effects the memory calculation. 
**
** Therefore, this driver does what it is not supposed to and allocates
** video memory itself.  But, this is required due to bugs in SoftVGA and,
** as it turns out, works out fine (with limiting compression use).
**
** For VSA2, the driver is supposed to do this allocation itself.
\*------------------------------------------------------------------------*/

static void
CYRIXFbInit()
{	int lineDelta    = vga256InfoRec.displayWidth * (vgaBitsPerPixel / 8);
	int virtualDelta = vga256InfoRec.virtualX * (vgaBitsPerPixel / 8);
	int base;

	vgaSetScreenInitHook(CYRIXScreenInit);

	/* always put the cursor at the end of video memory. */

	CYRIXcursorAddress = CYRIX.ChipLinearSize - 256;

	/* offscreen memory is, normally, right after the frame buffer;
	   (It would be nice to use the ignored 64KB block at the end of
	   the video memory (2112 - 2048) for the hardware cursor, but
	   it is not mapped.  This will not be a problem in Xfree 3.9 */

	CYRIXoffscreenAddress = (lineDelta * vga256InfoRec.virtualY);
	CYRIXoffscreenSize = CYRIXcursorAddress - CYRIXoffscreenAddress;

	/* if there is enough room between lines, put the compression
	   buffer there */
	if (lineDelta - virtualDelta >= 256)
	{	CYRIXcbufferAddress = virtualDelta;
		CYRIXcbLineDelta    = (lineDelta >> 2);
		if (xf86Verbose > 1)
			ErrorF("%s %s: Interleaving frame buffer and compression buffer\n",
				   XCONFIG_PROBED, vga256InfoRec.name);
	}
	/* otherwise, put it directly after the virtual frame */
	else
	{	int cbuffer_size = 256 * vga256InfoRec.virtualY;

		if (cbuffer_size > CYRIXoffscreenSize)
		{	CYRIXcbLineDelta  =  0;
			ErrorF("%s %s: No room for the compression buffer\n",
				   XCONFIG_PROBED, vga256InfoRec.name);
		}
		else
		{	CYRIXcbufferAddress    = CYRIXoffscreenAddress;
			CYRIXcbLineDelta       = 0x40;
			CYRIXoffscreenAddress += cbuffer_size;
			CYRIXoffscreenSize    -= cbuffer_size;
	}	}

	/* print results of offscreen memory configuration */

	if (CYRIXoffscreenSize <= 0)
	{
		ErrorF("%s %s: No offscreen memory available.\n", 
			XCONFIG_PROBED, vga256InfoRec.name);
	}
	else
	{
		ErrorF("%s %s: Offscreen memory from 0x%8.8X-0x%8.8X\n",
			XCONFIG_PROBED, vga256InfoRec.name, 
			CYRIXoffscreenAddress, 
			CYRIXoffscreenAddress+CYRIXoffscreenSize-1);
	}

    /* call CYRIXAccelInit to setup the XAA accelerated functions */

	if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
		CYRIXAccelInit();

	/* install hardware cursor routines */

	if (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
	{	if (CYRIXoffscreenSize > 0)
		{	vgaHWCursor.Initialized = TRUE;
			vgaHWCursor.Init = CYRIXCursorInit;
			vgaHWCursor.Restore = CYRIXRestoreCursor;
			vgaHWCursor.Warp = CYRIXWarpCursor;
			vgaHWCursor.QueryBestSize = CYRIXQueryBestSize;
			ErrorF("%s %s: Using hardware cursor at %8.8X\n",
				XCONFIG_PROBED, vga256InfoRec.name, CYRIXcursorAddress);
		}
		else
		{
			ErrorF("%s %s: No room for hardware cursor\n",
				XCONFIG_PROBED, vga256InfoRec.name);
		}
	}
}


static Bool CYRIXScreenInit(pScreen, base, x, y, resx, resy, width)
ScreenPtr pScreen;
pointer base;
int x;
int y;
int resx;
int resy;
int width;
{	/* use dedicated color map routines on new chipsets in 8bpp */
	if (vgaBitsPerPixel == 8 && !(CYRIXisOldChipRevision))
	{	pScreen->ListInstalledColormaps = CYRIXListInstalledColormaps;
		pScreen->StoreColors = CYRIXStoreColors;
		pScreen->InstallColormap = CYRIXInstallColormap;
		pScreen->UninstallColormap = CYRIXUninstallColormap;
	}

	return TRUE;
}


static int
CYRIXPitchAdjust()
{	int bytesPerPixel = (vgaBitsPerPixel / 8);
	int lineDelta     = vga256InfoRec.virtualX * bytesPerPixel;
	int pitch;

	if (lineDelta > 2048)
		FatalError("virtual display width requires a line size of more than 2048 bytes\n");

	if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
	{	if (lineDelta > 1024)
			lineDelta = 2048;
		else
			lineDelta = 1024;
	}
	else
		lineDelta = (lineDelta + 15) & 0xFF80;  /* ChipRounding = 8 implies this */

	pitch = lineDelta / bytesPerPixel;

	if (pitch != vga256InfoRec.virtualX)
		ErrorF("%s %s: Display pitch set to %d\n",
		       XCONFIG_PROBED, vga256InfoRec.name, pitch);

	return pitch;
}


static void
CYRIXEnterLeave(enter)
Bool enter;
{
	unsigned char temp;

	if (enter)
	{
		xf86EnableIOPorts(vga256InfoRec.scrnIndex);

		vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;

		/* Unprotect CRTC[0-7] */
		outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
		outb(vgaIOBase + 5, temp & 0x7F);

		/* Unprotect MediaGX extended registers */
		outb(vgaIOBase + 4, CrtcExtendedRegisterLock);
		outb(vgaIOBase + 5, 0x57);
		outb(vgaIOBase + 5, 0x4C);

		/* unprotecting the hardware registers can't be done
		   here, because EnterLeave is called before those
		   registers are mapped */
	}
	else
	{
		/* Protect MediaGX extended registers */
		outb(vgaIOBase + 4, CrtcExtendedRegisterLock);
		outb(vgaIOBase + 5, 0x00);

		/* Protect CRTC[0-7] */
		outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
		outb(vgaIOBase + 5, (temp & 0x7F) | 0x80);

		xf86DisableIOPorts(vga256InfoRec.scrnIndex);
	}
}

static void
CYRIXmarkLinesDirty()
{	int k = 0;
	while (k < 1024)
	{	GX_REG(MC_DR_ADD) = k++;
		GX_REG(MC_DR_ACC) = 0;
}	}

static void
CYRIXresetVGA()
{
	int i;

	/* reset SoftVGA extensions to standard VGA behaviour */

	for (i = 0; i < NUM_EXT_CRTC_REGS; i++)
	{
		outb(vgaIOBase + 4, 0x40 + i);
		outb(vgaIOBase + 5, 0x00);
	}
}

static void
CYRIXRestore(restore)
vgaCYRIXPtr restore;
{	unsigned char i, temp, temp2;
	unsigned long value;

	/* unlock extended CRTC registers */

	outb(vgaIOBase + 4, 0x30);
	outb(vgaIOBase + 5, 0x57);
	outb(vgaIOBase + 5, 0x4C);

	/* SIGNAL THE BEGINNING OF THE MODE SWITCH 
       SoftVGA will hold off validating the back end hardware. */

	outb(vgaIOBase + 4, CrtcModeSwitchControl);
	outb(vgaIOBase + 5, 0x01);

	/* restore standard VGA portion */

	CYRIXresetVGA();
	vgaHWRestore((vgaHWPtr)restore);

	/* override restored miscellaneous output regiter value */
	
	outb(0x3C2, restore->ext.miscOutput);

	/* override restored standard CRTC register values */

	outb(vgaIOBase + 4, 0x11); 
	outb(vgaIOBase + 5, 0x00);
	for (i = 0; i < NUM_STD_CRTC_REGS; i++)
	{
		outb(vgaIOBase + 4, i);
		outb(vgaIOBase + 5, restore->ext.stdCRTCregs[i]);
	}
	
	/* restore SoftVGA extended registers */

	for (i = 0; i < NUM_EXT_CRTC_REGS; i++)
	{
		outb(vgaIOBase + 4, 0x40+i);
		outb(vgaIOBase + 5, restore->ext.extCRTCregs[i]);
	}

	/* signal the end of the mode switch */

	outb(vgaIOBase + 4, CrtcModeSwitchControl);
	outb(vgaIOBase + 5, 0x00);

	/* wait until SoftVGA has validated the mode.
	   This is for VSA1 only, where SoftVGA waits until the next
	   vertical blank to recalculate the hardware state.  For VSA2
	   the hardware us updated immediately, so this is not needed. 
	   THIS MUST BE DONE FOR VSA1 before loading the GP_BLT_STATUS
	   register, otherwise SoftVGA will override the value. */

	if (CYRIXvsaversion == CYRIX_VSA1)
	{
		outb(vgaIOBase + 4, 0x33); 
		while(inb(vgaIOBase + 5) & 0x80); 
	}

	/* overrite what SoftVGA may have stored into GP_BLIT_STATUS */

	GX_REG(GP_BLIT_STATUS) = restore->ext.GpBlitStatus;
}

static void *
CYRIXSave(save)
vgaCYRIXPtr save;
{	unsigned char i;
	struct vgaCYRIXext ext;

	/* save miscellaneous output register */

	ext.miscOutput = inb(0x3CC);

	/* save standard CRTC registers */

	for (i = 0; i < NUM_STD_CRTC_REGS; i++)
	{
		outb(vgaIOBase + 4, i);
		ext.stdCRTCregs[i] = inb(vgaIOBase + 5);
	}

	/* save extended CRTC registers */

	for (i = 0; i < NUM_EXT_CRTC_REGS; i++)
	{
		outb(vgaIOBase + 4, 0x40+i);
		ext.extCRTCregs[i] = inb(vgaIOBase + 5);
	}

	/* save graphics pipeline registers */

	ext.GpBlitStatus   = GX_REG(GP_BLIT_STATUS);

	/* save standard VGA portion */

	CYRIXresetVGA();
	save = (vgaCYRIXPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaCYRIXRec));
	save->ext = ext;
	return ((void *) save);
}


static Bool
CYRIXInit(mode)
DisplayModePtr mode;
{	int i, mode_index;
	int clock = vga256InfoRec.clock[mode->Clock] / 1000;
	int min, diff;
	int offset_shift = (vgaBitsPerPixel == 16) ? 2 :
                           (vgaBitsPerPixel == 8) ? 3 : 4;
	int line_offset = vga256InfoRec.displayWidth >> offset_shift;

	/* initialize standard VGA portion */

	if (!vgaHWInit(mode,sizeof(vgaCYRIXRec)))
		return(FALSE);

	/* search for specified mode in the table */
    /* Need to find the entry with the closest dot clock value */ 
	/* Assume within at least 200 MHz and then maintain closest natch. */

	mode_index = 0;
	min = 200; 
	for (i = 0; i < NUM_CYRIX_MODES; i++)
	{
		diff = clock - CYRIXmodes[i].clock;
		if (diff < 0) diff = -diff;
		if ((mode->CrtcHDisplay == CYRIXmodes[i].xsize) &&
			(mode->CrtcVDisplay == CYRIXmodes[i].ysize) &&
			(diff < min))
		{
			mode_index = i;
			min = diff;
		}
	}

	/* override standard miscOutput register value */

	newstate->ext.miscOutput = CYRIXmodes[mode_index].miscOutput;

	/* override standard CRTC register values */

	for (i = 0; i < NUM_STD_CRTC_REGS; i++)
	{
		newstate->ext.stdCRTCregs[i] = 
			CYRIXmodes[mode_index].stdCRTCregs[i];
	}

	/* set extended CRTC registers */

	for (i = 0; i < NUM_EXT_CRTC_REGS; i++)
	{
		newstate->ext.extCRTCregs[i] = 
			CYRIXmodes[mode_index].extCRTCregs[i];
	}

	/* override pitch from the mode tables */
	/* (same tables are used for 8BPP and 16BPP) */

	newstate->ext.stdCRTCregs[19] = line_offset;
	newstate->ext.extCRTCregs[5] = ((line_offset >> 8) & 0x03);

	/* override color control from the mode tables */
	/* (same tables are used for 8BPP and 16BPP) */

	newstate->ext.extCRTCregs[6] = (vgaBitsPerPixel == 16)
		? ECC_16BPP | ECC_565_FORMAT : ECC_8BPP;

	/* enable display compression when appropriate */

	if (CYRIXvsaversion == CYRIX_VSA1)
	{
		/* For VSA1, SoftVGA manages the compression buffer. */
		/* Enabling compression directly causes unpredictable results. */
		/* Only enable if not panning (there is a bug in SoftVGA that */
		/* will put the compression buffer in the wrong place when */
		/* using a virtual desktop. */
		/* By setting bit 0 of register 0x49, SoftVGA will enable */
		/* compression whenever possible, based on memory available */
		/* and starting address of memory. */

	    if ((mode->CrtcVDisplay == vga256InfoRec.virtualY) &&
			(mode->CrtcHDisplay == vga256InfoRec.virtualX))
		{
			newstate->ext.extCRTCregs[9] = 0x01;
			ErrorF("%s %s: Display compression enabled.\n", 
				XCONFIG_PROBED, vga256InfoRec.name);
		}
		else
		{
			ErrorF("%s %s: Display compression disabled.\n", 
				XCONFIG_PROBED, vga256InfoRec.name);
		}
	}
	else
	{
		/* ### TO DO ### */
		/* Enable display compression directly for VSA2. */
		/* For VSA2, the display driver manages all graphics memory. */
	}

	/* initialize the graphics pipeline registers */

	newstate->ext.GpBlitStatus  =  ((vga256InfoRec.displayWidth == 2048) ?
	                                BC_FB_WIDTH_2048 : BC_FB_WIDTH_1024) |
	                               ((vgaBitsPerPixel == 16) ?
	                                BC_16BPP : BC_8BPP);
	return(TRUE);
}

static void
CYRIXAdjust(x, y)
int x, y;
{	int Base = (y * vga256InfoRec.displayWidth + x);
	unsigned long active, sync, count1, count2;

	if (vgaBitsPerPixel > 8) Base *= (vgaBitsPerPixel / 8);
	if (vgaBitsPerPixel < 8) Base /= 2;

	/* wait until out of active display area */

	active = GX_REG(DC_V_TIMING_1) & 0x07FF;	
	sync = GX_REG(DC_V_TIMING_3) & 0x07FF;

	do
	{
		/* read twice to avoid transition values */

		count1 = GX_REG(DC_V_LINE_CNT) & 0x07FF;
		count2 = GX_REG(DC_V_LINE_CNT) & 0x07FF;
	} while ((count1 != count2) || (count1 < active) || (count1 >= sync));

	/* load the start address directly */

	GX_REG(DC_UNLOCK) = DC_UNLOCK_VALUE;
	GX_REG(DC_FB_ST_OFFSET) = Base;
	GX_REG(DC_UNLOCK) = 0;
}

/*------------------------------------------------------------------------*\
** ValidMode()
** 
** From README file: "The ValidMode() function is required.  It is used to 
** check for any chipset dependent reasons why a graphics mode might not be
** valid.  It gets called by higher levels of the code after the Probe()
** stage.  In many cases no special checking will be required and this 
** function will simply return TRUE always."
**
** For the Cyrix driver, this routine loops through modes provided in a 
** table at the beginning of this file and returns OK if it finds a match.
** These tables were required to make the standard VESA 60 Hz and 75 Hz 
** modes work correctly.  Doing this, however, takes away the flexibility 
** of adding different resolutions or different refresh rates to the 
** XF86Config file.
\*------------------------------------------------------------------------*/

static int
CYRIXValidMode(mode, verbose, flag)
DisplayModePtr mode;
Bool verbose;
int flag;
{
	int i;

	/* loop through table of modes */

	for (i = 0; i < NUM_CYRIX_MODES; i++)
	{
		if ((mode->CrtcHDisplay == CYRIXmodes[i].xsize) &&
			(mode->CrtcVDisplay == CYRIXmodes[i].ysize))
		{
			return MODE_OK;
		}
	}
	return MODE_BAD;
}

/* END OF FILE */

