/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cyrix/cyrix_driver.c,v 1.1.2.6 1998/11/06 09:47:08 hohndel Exp $ */
/*
 * Copyright 1998 by Annius V. Groenink (A.V.Groenink@zfc.nl, avg@cwi.nl),
 *                   Dirk H. Hohndel (hohndel@suse.de),
 *                   Portions: the GGI project & confidential CYRIX databooks.
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
/* $XConsortium: $ */

/*************************************************************************/

/*
   Log for the cyrix driver source as a whole

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

#ifdef XF86VGA16
#define MONOVGA
#endif

#if !defined(MONOVGA) && !defined(XF86VGA16)
#include "vga256.h"
#endif

#include "cyrix.h"

pciTagRec CyrixPciTag;
extern vgaHWCursorRec vgaHWCursor;

typedef struct {
	vgaHWRec std; /* IBM VGA */
	struct vgaCYRIXext
	{	/* extended SoftVGA registers */
		unsigned char VerticalTimingExtension;
		unsigned char ExtendedAddressControl;
		unsigned char ExtendedOffset;
		unsigned char ExtendedColorControl;
		unsigned char DisplayCompression;
		unsigned char DriverControl;
		unsigned char DACControl;
		unsigned char ClockControl;
		unsigned char CrtClockFrequency;
		unsigned char CrtClockFrequencyFraction;
		unsigned char RefreshRate;

		/* display controller hardware registers */
		CARD32 DcGeneralCfg;
		CARD32 DcCursStOffset;
		CARD32 DcCbStOffset;
		CARD32 DcLineDelta;
		CARD32 DcBufSize;
		CARD32 DcCursorX;
		CARD32 DcCursorY;
		CARD32 DcCursorColor;

		/* graphics pipeline registers */
		CARD32 GpBlitStatus;

		/* save area for cursor image */
		char cursorPattern[256];
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

#ifndef MONOVGA
static void	CYRIXFbInit();
static Bool	CYRIXScreenInit();
static Bool	CYRIXPitchAdjust();
#endif

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
#ifndef MONOVGA
	CYRIXFbInit,
#else
	(void (*)())NoopDDA,     /* CYRIXFbInit */
#endif
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
	FALSE,                   /* ChipUseLinearAddressing */
	0,                       /* ChipLinearBase */
	0,                       /* ChipLinearSize */
	TRUE,                    /* ChipHas16bpp */
	FALSE,                   /* ChipHas24bpp */
	FALSE,                   /* ChipHas32bpp */
	NULL,                    /* ChipBuiltinModes */
	1,                       /* ChipClockMulFactor */
	1                        /* ChipCLockDivFactor */
};


/* access to the MediaGX video hardware registers */

char* GXregisters;

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

#ifndef MONOVGA
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
#endif

	/* map the entire area from GX_BASE (scratchpad area)
	   up to the end of the control registers */
	GXregisters = (char*)xf86MapVidMem(vga256InfoRec.scrnIndex,
				    EXTENDED_REGION,
				    (void*)physbase, 0x9000);

	if (!GXregisters)
	{	ErrorF("%s %s: Cannot map hardware registers\n",
			XCONFIG_PROBED, vga256InfoRec.name);
		goto probeFailed;
	}

	return(TRUE);
}


#ifndef MONOVGA
static void
CYRIXFbInit()
{	int lineDelta    = vga256InfoRec.displayWidth * (vgaBitsPerPixel / 8);
	int virtualDelta = vga256InfoRec.virtualX * (vgaBitsPerPixel / 8);

	vgaSetScreenInitHook(CYRIXScreenInit);

	/* offscreen memory is, normally, right after the frame buffer;
	   always put the cursor at the end of video memory.
	   
	   (It would be nice to use the ignored 64KB block at the end of
	   the video memory (2112 - 2048) for the hardware cursor, but
	   it is not mapped.  This will not be a problem in Xfree 3.9 */
	CYRIXoffscreenAddress = (lineDelta * vga256InfoRec.virtualY);
	CYRIXcursorAddress    = CYRIX.ChipLinearSize - 256;
	CYRIXoffscreenSize    = CYRIXcursorAddress - CYRIXoffscreenAddress;

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

        /* call CYRIXAccelInit to setup the XAA accelerated functions */
	if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
		CYRIXAccelInit();

	/* install hardware cursor routines */
	if (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
	{	if (CYRIXoffscreenSize >= 0)
		{	vgaHWCursor.Initialized = TRUE;
			vgaHWCursor.Init = CYRIXCursorInit;
			vgaHWCursor.Restore = CYRIXRestoreCursor;
			vgaHWCursor.Warp = CYRIXWarpCursor;
			vgaHWCursor.QueryBestSize = CYRIXQueryBestSize;
			if (xf86Verbose)
		           ErrorF("%s %s: Using hardware cursor\n",
		                  XCONFIG_PROBED, vga256InfoRec.name);
		}
		else
			ErrorF("%s %s: No room for hardware cursor\n",
		               XCONFIG_PROBED, vga256InfoRec.name);
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
#endif /* not MONOVGA */


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
{	unsigned char temp;
	/* switch off compression and cursor the hard way */
	GX_REG(DC_UNLOCK)  = DC_UNLOCK_VALUE;
	GX_REG(DC_GENERAL_CFG) &= ~(DC_GCFG_CMPE | DC_GCFG_DECE | DC_GCFG_FDTY | DC_GCFG_CURE);
	GX_REG(DC_UNLOCK)  = 0;
	CYRIXmarkLinesDirty();

	/* reset SoftVGA extensions to standard VGA behaviour */
	outb(vgaIOBase + 4, CrtcExtendedAddressControl);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, temp & 0xf8);
	outb(vgaIOBase + 4, CrtcExtendedStartAddress);
	outb(vgaIOBase + 5, 0x00);
	outb(vgaIOBase + 4, CrtcWriteMemoryAperture);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, temp & 0xe0);
	outb(vgaIOBase + 4, CrtcReadMemoryAperture);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, temp & 0xe0);
	outb(vgaIOBase + 4, CrtcDriverControl);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, temp & 0xfe);
	outb(vgaIOBase + 4, CrtcDisplayCompression);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, temp & 0xf0);
}

static void
CYRIXRestore(restore)
vgaCYRIXPtr restore;
{	unsigned char temp;
	vgaProtect(TRUE);		/* Blank the screen */

	/* it would be ideal to be able to use the ModeSwitchControl
	   register to protect SoftVGA from reading the configuration
	   before all registers have been written.  But that bit must be
	   set somewhere in the middle of vgaHWRestore (after restoring
	   the font). Luckily things seem to work without it. */

	/* restore standard VGA portion */
	CYRIXresetVGA();
	vgaHWRestore((vgaHWPtr)restore);
	CYRIXmarkLinesDirty();

	/* restore SoftVGA extended registers */
	outb(vgaIOBase + 4, CrtcDriverControl);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, (restore->ext.DriverControl & 0x01)
	                  | (temp & 0xfe));

	outb(vgaIOBase + 4, CrtcVerticalTimingExtension);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, (restore->ext.VerticalTimingExtension & 0x55)
	                  | (temp & 0xaa));

	outb(vgaIOBase + 4, CrtcExtendedAddressControl);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, (restore->ext.ExtendedAddressControl & 0x07)
	                  | (temp & 0xf8));

	outb(vgaIOBase + 4, CrtcExtendedOffset);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, (restore->ext.ExtendedOffset & 0x03)
	                  | (temp & 0xfc));

	outb(vgaIOBase + 4, CrtcExtendedColorControl);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, (restore->ext.ExtendedColorControl & 0x07)
	                  | (temp & 0xf8));

	outb(vgaIOBase + 4, CrtcDisplayCompression);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, (restore->ext.DisplayCompression & 0x0f)
	                  | (temp & 0xf0));

	outb(vgaIOBase + 4, CrtcDACControl);
	temp = inb(vgaIOBase + 5);
	outb(vgaIOBase + 5, (restore->ext.DACControl & 0x0e)
	                  | (temp & 0xf1));

	if (restore->std.NoClock >= 0)
	{	outb(vgaIOBase + 4, CrtcClockControl);
		temp = inb(vgaIOBase + 5);
		outb(vgaIOBase + 5, (restore->ext.ClockControl & 0xb0)
		                  | (temp & 0x4f));

		outb(vgaIOBase + 4, CrtcClockFrequency);
		outb(vgaIOBase + 5, restore->ext.CrtClockFrequency);

		outb(vgaIOBase + 4, CrtcClockFrequencyFraction);
		outb(vgaIOBase + 5, restore->ext.CrtClockFrequencyFraction);

		outb(vgaIOBase + 4, CrtcRefreshRate);
		outb(vgaIOBase + 5, restore->ext.RefreshRate);
	}

	/* let SoftVGA programming settle before we access DC registers,
	   but don't wait too long */
	usleep(1000);
	CYRIXmarkLinesDirty();

	/* restore display controller hardware registers */
#ifndef MONOVGA
#define DCFG_MASK       (DC_GCFG_FDTY | DC_GCFG_DECE | DC_GCFG_CMPE)
#define GPBS_MASK       (BC_16BPP | BC_FB_WIDTH_2048)

	GX_REG(DC_UNLOCK) = DC_UNLOCK_VALUE;

	GX_REG(DC_CURS_ST_OFFSET) = restore->ext.DcCursStOffset;
	GX_REG(DC_CB_ST_OFFSET)  = restore->ext.DcCbStOffset;
	GX_REG(DC_LINE_DELTA)    = (GX_REG(DC_LINE_DELTA) & 0xFFC00FFF)
	                         | (restore->ext.DcLineDelta & 0x003FF000);
	GX_REG(DC_BUF_SIZE)      = (GX_REG(DC_BUF_SIZE) & 0xFFFF01FF)
                                 | (restore->ext.DcBufSize & 0x0000FE00);
	GX_REG(DC_CURSOR_X)      = restore->ext.DcCursorX;
	GX_REG(DC_CURSOR_Y)      = restore->ext.DcCursorY;
	GX_REG(DC_CURSOR_COLOR)  = restore->ext.DcCursorColor;

	GX_REG(DC_GENERAL_CFG)   = (GX_REG(DC_GENERAL_CFG) & (~DCFG_MASK))
	                         | (restore->ext.DcGeneralCfg & DCFG_MASK);

	GX_REG(DC_UNLOCK) = 0;

	GX_REG(GP_BLIT_STATUS)   = (GX_REG(GP_BLIT_STATUS) & (~GPBS_MASK))
	                         | (restore->ext.GpBlitStatus & GPBS_MASK);

	/* restore cursor pattern */
	if (restore->ext.DcCursStOffset < 1024 * vga256InfoRec.videoRam)
		memcpy((char*)vgaLinearBase + restore->ext.DcCursStOffset,
		       restore->ext.cursorPattern, 256);
#endif

	vgaProtect(FALSE);		/* Turn on screen */
}

static void *
CYRIXSave(save)
vgaCYRIXPtr save;
{	struct vgaCYRIXext ext;

#ifndef MONOVGA
	/* save graphics pipeline registers */
	ext.GpBlitStatus   = GX_REG(GP_BLIT_STATUS);

	/* save display controller hardware registers */
	GX_REG(DC_UNLOCK)  = DC_UNLOCK_VALUE;
	ext.DcGeneralCfg   = GX_REG(DC_GENERAL_CFG);
	ext.DcCursStOffset = GX_REG(DC_CURS_ST_OFFSET);
	ext.DcCbStOffset   = GX_REG(DC_CB_ST_OFFSET);
	ext.DcLineDelta    = GX_REG(DC_LINE_DELTA);
	ext.DcBufSize      = GX_REG(DC_BUF_SIZE);
	ext.DcCursorX      = GX_REG(DC_CURSOR_X);
	ext.DcCursorY      = GX_REG(DC_CURSOR_Y);
	ext.DcCursorColor  = GX_REG(DC_CURSOR_COLOR);
	GX_REG(DC_UNLOCK)  = 0;

	/* save cursor pattern.
	   In the 3.3.1 solution, we don't need to do this
	   if it is in the extra 64KB block of frame buffer memory
	   that we ignore (and is not mapped anyway) */
	if (ext.DcCursStOffset < 1024 * vga256InfoRec.videoRam)
		memcpy(ext.cursorPattern,
		       (char*)vgaLinearBase + ext.DcCursStOffset, 256);
#endif

	/* save SoftVGA extended registers */
	outb(vgaIOBase + 4, CrtcVerticalTimingExtension);
	ext.VerticalTimingExtension = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcExtendedAddressControl);
	ext.ExtendedAddressControl = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcExtendedOffset);
	ext.ExtendedOffset = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcExtendedColorControl);
	ext.ExtendedColorControl = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcDisplayCompression);
	ext.DisplayCompression = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcDriverControl);
	ext.DriverControl = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcDACControl);
	ext.DACControl = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcClockControl);
	ext.ClockControl = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcClockFrequency);
	ext.CrtClockFrequency = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcClockFrequencyFraction);
	ext.CrtClockFrequencyFraction = inb(vgaIOBase + 5);

	outb(vgaIOBase + 4, CrtcRefreshRate);
	ext.RefreshRate = inb(vgaIOBase + 5);

	/* save standard VGA portion */
	CYRIXresetVGA();
	save = (vgaCYRIXPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaCYRIXRec));
	save->ext = ext;

	return ((void *) save);
}


static Bool
CYRIXInit(mode)
DisplayModePtr mode;
{	int offset_shift = (vgaBitsPerPixel == 16) ? 2 :
                           (vgaBitsPerPixel == 8) ? 3 : 4;
	int line_offset = vga256InfoRec.displayWidth >> offset_shift;

	/* initialize standard VGA portion */
	if (!vgaHWInit(mode,sizeof(vgaCYRIXRec)))
		return(FALSE);

	newstate->std.CRTC[19] = line_offset;

	/* initialize SoftVGA extended registers */
	newstate->ext.VerticalTimingExtension =
		((mode->CrtcVSyncStart & 0x400) >> 4) |
		(((mode->CrtcVDisplay - 1) & 0x400) >> 8) |
		(((mode->CrtcVTotal - 2) & 0x400) >> 10) |
		((mode->CrtcVSyncStart & 0x400) >> 6);

	if (vgaBitsPerPixel < 8)
		newstate->ext.ExtendedAddressControl = EAC_DIRECT_FRAME_BUFFER;
	else
		newstate->ext.ExtendedAddressControl = EAC_DIRECT_FRAME_BUFFER |
		                                       EAC_PACKED_CHAIN4;

	newstate->ext.ExtendedOffset = ((line_offset >> 8) & 0x03);

	newstate->ext.ExtendedColorControl = (vgaBitsPerPixel == 16)
                                           ? ECC_16BPP | ECC_565_FORMAT
                                           : ECC_8BPP;

	/* display compression is set using the DC registers */
	newstate->ext.DisplayCompression = 0x00;

	/* we drive the palette through the display controller (in new
	   chipsets only) in 8bpp and 16bpp (that is, whenever the
	   hardware cursor is used). */
	if (vgaBitsPerPixel < 8)
		newstate->ext.DriverControl = 0x00;
	else
		newstate->ext.DriverControl = DRVCT_DISPLAY_DRIVER_ACTIVE;

	/* set `16 bit bus' or else compression will hang the
	   system in 16bpp mode */
	if (vgaBitsPerPixel == 16)
		newstate->ext.DACControl = DACCT_ENABLE_16BIT_BUS;
	else
		newstate->ext.DACControl = 0;


	if (newstate->std.NoClock >= 0)
	{	int entier_clock   = (vga256InfoRec.clock[mode->Clock] / 1000);
		int clock_fraction = (vga256InfoRec.clock[mode->Clock] / 100)
				   - (entier_clock * 10);

		newstate->ext.ClockControl = CLKCT_EXT_CLOCK_MODE;
		newstate->ext.CrtClockFrequency = entier_clock;
		newstate->ext.CrtClockFrequencyFraction = clock_fraction;
		newstate->ext.RefreshRate = 0 /* relevant to VGA BIOS only */;
	}

#ifndef MONOVGA
	/* initialize masked contents of display controller
	   hardware registers. */
	newstate->ext.DcCursStOffset =  CYRIXcursorAddress;
	newstate->ext.DcCbStOffset  =  CYRIXcbufferAddress;
	newstate->ext.DcLineDelta   =  CYRIXcbLineDelta << 12;
	newstate->ext.DcBufSize     =  0x41 << 9;
	newstate->ext.DcCursorX     =  0;
	newstate->ext.DcCursorY     =  0;
	newstate->ext.DcCursorColor =  0;

	/* Compression  is enabled only  when a buffer  was allocated by
	   FbInit  and provided that the displayed screen is the virtual
	   screen.  If the line delta is not 1024 or 2048, entire frames
	   will be flagged dirty as opposed to lines.  Problems with 16bpp
	   and line-dirty flagging seem to have been solved now.  */
	if (CYRIXcbLineDelta != 0 &&
	    mode->CrtcVDisplay == vga256InfoRec.virtualY &&
	    mode->CrtcHDisplay == vga256InfoRec.virtualX)
	{	newstate->ext.DcGeneralCfg = DC_GCFG_DECE
		                           | DC_GCFG_CMPE;
		if (/* vgaBitsPerPixel != 8 ||   -- this is OK now */
		   (vga256InfoRec.displayWidth * (vgaBitsPerPixel / 8)) & 0x03FF)
			newstate->ext.DcGeneralCfg |= DC_GCFG_FDTY;
	}
	else
		newstate->ext.DcGeneralCfg = 0;


	/* initialize the graphics pipeline registers */
	newstate->ext.GpBlitStatus  =  ((vga256InfoRec.displayWidth == 2048) ?
	                                BC_FB_WIDTH_2048 : BC_FB_WIDTH_1024) |
	                               ((vgaBitsPerPixel == 16) ?
	                                BC_16BPP : BC_8BPP);
#endif

	return(TRUE);
}

static void
CYRIXAdjust(x, y)
int x, y;
{	int Base = (y * vga256InfoRec.displayWidth + x);

	if (vgaBitsPerPixel > 8) Base *= (vgaBitsPerPixel / 8);
	if (vgaBitsPerPixel < 8) Base /= 2;

	/* doing this using the SoftVGA registers does not work reliably */
	GX_REG(DC_UNLOCK) = DC_UNLOCK_VALUE;
	GX_REG(DC_FB_ST_OFFSET) = Base;
	GX_REG(DC_UNLOCK) = 0;
}

static int
CYRIXValidMode(mode, verbose, flag)
DisplayModePtr mode;
Bool verbose;
int flag;
{	/* note (avg): there seems to be a lot more to this if you look
	   at the GGI code (adjustment). */
	if (mode->CrtcHSyncStart - mode->CrtcHDisplay >= 24 ||
            mode->CrtcHSyncStart - mode->CrtcHDisplay <= 8)
	{	if (verbose)
			ErrorF("%s %s: mode %s: horizontal sync out of range (sync - display should be between 8 and 24)\n",
				XCONFIG_PROBED, vga256InfoRec.name, mode->name
				);
		return MODE_HSYNC;
	}
	return(MODE_OK);
}

