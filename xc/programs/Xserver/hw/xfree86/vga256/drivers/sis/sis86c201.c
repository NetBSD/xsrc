/* $XConsortium: sis86c201.c /main/3 1996/01/13 13:14:59 kaleb $ */
/*
 * Copyright 1995 by Alan Hourihane, Wigan, England.
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
 * Author:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *
 * ToDo: 	Set 64K/16M colours
 *		Linear is broke - FIXME ! - Can't find linear address space.
 *
 * Currently only works for VGA16 with Non-Interlaced modes.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/sis/sis86c201.c,v 3.13 1996/10/16 14:43:31 dawes Exp $ */

#include "X.h"
#include "input.h"
#include "screenint.h"
#include "dix.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"
#include "vga.h"
#include "vgaPCI.h"

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

#ifndef MONOVGA
extern vgaHWCursorRec vgaHWCursor;
#endif

typedef struct {
	vgaHWRec std;          		/* std IBM VGA register 	*/
	unsigned char ClockReg;
	unsigned char ClockReg2;
	unsigned char DualBanks;
	unsigned char BankReg;
	unsigned char CRTCOff;
	unsigned char DispCRT;
	unsigned char Unknown;
} vgaSISRec, *vgaSISPtr;

static Bool SISClockSelect();
static char *SISIdent();
static Bool SISProbe();
static void SISEnterLeave();
static Bool SISInit();
static int  SISValidMode();
static void *SISSave();
static void SISRestore();
static void SISFbInit();
static void SISAdjust();
extern void SISSetRead();
extern void SISSetWrite();
extern void SISSetReadWrite();

extern int SISCursorHotX;
extern int SISCursorHotY;
extern int SISCursorWidth;
extern int SISCursorHeight;

extern void SISCursorInit();
extern void SISRestoreCursor();
extern void SISWarpCursor();
extern void SISQueryBestSize();

vgaVideoChipRec SIS = {
  SISProbe,
  SISIdent,
  SISEnterLeave,
  SISInit,
  SISValidMode,
  SISSave,
  SISRestore,
  SISAdjust,
  vgaHWSaveScreen,
  (void (*)())NoopDDA,
  SISFbInit,
  SISSetRead,
  SISSetWrite,
  SISSetReadWrite,
  0x10000,
  0x10000,
  16,
  0xffff,
  0x00000, 0x10000,
  0x00000, 0x10000,
  TRUE,                           
  VGA_DIVIDE_VERT,
  {0,},
  16,				
  FALSE,
  0,
  0,
  TRUE,
  FALSE,
  TRUE,
  NULL,
  1,
};

#define new ((vgaSISPtr)vgaNewVideoState)

#define SIS86C201 0
#define SIS86C202 1
#define SIS86C205 2

int SISchipset;
Bool sisUseLinear = FALSE;
static int SISDisplayableMemory;

/*
 * SISIdent --
 */
static char *
SISIdent(n)
	int n;
{
	static char *chipsets[] = {"sis86c201", "sis86c202", "sis86c205", };

	if (n + 1 > sizeof(chipsets) / sizeof(char *))
		return(NULL);
	else
		return(chipsets[n]);
}

/*
 * SISClockSelect --
 * 	select one of the possible clocks ...
 */
static Bool
SISClockSelect(no)
	int no;
{
	static unsigned char save1, save2;
	unsigned char temp;

	/*
	 * CS0 and CS1 are in MiscOutReg
	 *
	 * CS2,CS3,CS4 are in 0x3C4 index 7
	 * But - only active when CS0/CS1 are set.
	 */
	switch(no)
	{
	case CLK_REG_SAVE:
		save1 = inb(0x3CC);
		outb(0x3C4, 0x07);
		save2 = inb(0x3C5);
		break;
	case CLK_REG_RESTORE:
		outb(0x3C2, save1);
		outw(0x3C4, (save2 << 8) | 0x07);
		break;
	default:
		/*
		 * Do CS0 and CS1 and set them - makes index 7 valid
		 */
		temp = inb(0x3CC);
		outb(0x3C2, temp | 0x0C);

		outw(0x3C4, (no << 8) | 0x07);
	}
	return(TRUE);
}

/* 
 * SISProbe --
 * 	check up whether a SIS 86C201 based board is installed
 */
static Bool
SISProbe()
{
  	int numClocks;
  	unsigned char temp;

	/*
         * Set up I/O ports to be used by this card
	 */
	xf86ClearIOPortList(vga256InfoRec.scrnIndex);
	xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);

	SISchipset = -1;

  	if (vga256InfoRec.chipset)
    	{
		/*
		 * If chipset from XF86Config doesn't match...
		 */
		if (!StrCaseCmp(vga256InfoRec.chipset, SISIdent(0)))
			SISchipset = SIS86C201;
		else if (!StrCaseCmp(vga256InfoRec.chipset, SISIdent(1)))
			SISchipset = SIS86C202;
		else if (!StrCaseCmp(vga256InfoRec.chipset, SISIdent(2)))
			SISchipset = SIS86C205;
		else
			return(FALSE);
    	}
	else
	{
		/* Aparently there are only PCI based 86C201's */

		if (vgaPCIInfo && vgaPCIInfo->Vendor == PCI_VENDOR_SIS)
		{
			switch(vgaPCIInfo->ChipType)
			{
			case PCI_CHIP_SG86C201: 	/* 86C201 */
				SISchipset = SIS86C201;
				break;
			case PCI_CHIP_SG86C202:		/* 86C202 */
				SISchipset = SIS86C202;
				break;
			case PCI_CHIP_SG86C205:		/* 86C205 */
				SISchipset = SIS86C205;
				break;
			}
		}
		if (SISchipset == -1)
			return (FALSE);
		vga256InfoRec.chipset = SISIdent(SISchipset);
	}

	SISEnterLeave(ENTER);
	
 	/* 
	 * How much Video Ram have we got?
	 */
    	if (!vga256InfoRec.videoRam)
    	{
		unsigned char temp;

		outb(0x3C4, 0x0F); 
		temp = inb(0x3C5);

		switch (temp & 0x03) 
		{
		case 0: 
			vga256InfoRec.videoRam = 1024;
			break;
		case 1:
			vga256InfoRec.videoRam = 2048;
			break;
		case 2: 
			vga256InfoRec.videoRam = 4096;
			break;
		}
     	}

	/*
	 * If clocks are not specified in XF86Config file, probe for them
	 */
    	if (!vga256InfoRec.clocks)
	{
		numClocks = 32;
		vgaGetClocks(numClocks, SISClockSelect);
	}

	vga256InfoRec.bankedMono = TRUE;
#ifndef MONOVGA
	/* We support Direct Video Access */
	vga256InfoRec.directMode = XF86DGADirectPresent;

	/* MaxClock set at 90MHz for 256 - ??? */
	OFLG_SET(OPTION_LINEAR, &SIS.ChipOptionFlags);
#else
	/* Set to 130MHz at 16 colours */
	vga256InfoRec.maxClock = 130000;
#endif

    	return(TRUE);
}

/*
 * SISFbInit --
 *	enable speedups for the chips that support it
 */
static void
SISFbInit()
{
#ifndef MONOVGA
	unsigned long j;
	unsigned long i;
	pointer sisVideoMem;
	long *poker;
	int offscreen_available;
	/*
	 * The PCI Configuration Space doesn't seem to set a base
	 * address for the linear aperture. We can do this with the
	 * linear registers. But - We must use MemBase to do this.
	 */
#if 0	/* BROKE - FIXME ! Can't find linear address space */	
	/* Must be missing some bit flip to turn it on.    */
	outb(0x3C4, 0x20);
	SIS.ChipLinearBase = (inb(0x3C5) << 8) * 1024; 	/* Get Linear Status */

	if (SIS.ChipLinearBase == 0)
		SIS.ChipLinearBase = (62 * 1024 * 1024);

	if (vga256InfoRec.MemBase != 0)
		SIS.ChipLinearBase = vga256InfoRec.MemBase;

	if (OFLG_ISSET(OPTION_LINEAR, &vga256InfoRec.options))
	{
		sisUseLinear = TRUE;
		outw(0x3C4, 0xF820); /* Set at 62MB */
		outw(0x3C4, 0x6021); /* Enable Linear */
	}

	for (i=0x2000000;i<0xff000000;i+=0x10000){
		sisVideoMem = xf86MapVidMem(0, LINEAR_REGION, (pointer)i,
						1024);
		poker = (long *) sisVideoMem;
		if (*poker != 0xffffffff)
		ErrorF("Memory at 0x%x is 0x%x\n",i,*poker);
		xf86UnMapVidMem(0, LINEAR_REGION, sisVideoMem, 1024);
	}
#endif

	if (xf86LinearVidMem() && sisUseLinear)
	{
		SIS.ChipLinearSize = vga256InfoRec.videoRam * 1024;
		ErrorF("%s %s: Using Linear Frame Buffer at 0x0%x, Size %dMB\n",
			XCONFIG_PROBED, vga256InfoRec.name,
			SIS.ChipLinearBase, SIS.ChipLinearSize/1048576);
	}

	SIS.ChipLinearSize = 2048 * 1024;

	if (sisUseLinear)
		SIS.ChipUseLinearAddressing = TRUE;
	else
		SIS.ChipUseLinearAddressing = FALSE;

	SISDisplayableMemory = vga256InfoRec.displayWidth
				* vga256InfoRec.virtualY
				* (vgaBitsPerPixel / 8);

	offscreen_available = vga256InfoRec.videoRam * 1024 - 
					SISDisplayableMemory;

	if (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
	{
		/* SiS needs upper 16K for hardware cursor */
		if (offscreen_available < 16384)
			ErrorF("%s %s: Not enough off-screen video"
				" memory for hardware cursor, using software cursor.\n",
				XCONFIG_PROBED, vga256InfoRec.name);
		else {
			SISCursorWidth = 64;
			SISCursorHeight = 64;
			vgaHWCursor.Initialized = TRUE;
			vgaHWCursor.Init = SISCursorInit;
			vgaHWCursor.Restore = SISRestoreCursor;
			vgaHWCursor.Warp = SISWarpCursor;
			vgaHWCursor.QueryBestSize = SISQueryBestSize;
			ErrorF("%s %s: Using hardware cursor\n",
				XCONFIG_GIVEN, vga256InfoRec.name);
		}
	}
#endif /* MONOVGA */
}

/*
 * SISEnterLeave --
 * 	enable/disable io-mapping
 */
static void
SISEnterLeave(enter)
	Bool enter;
{
  	unsigned char temp;

#ifndef MONOVGA
#ifdef XFreeXDGA
	if (vga256InfoRec.directMode & XF86DGADirectGraphics && !enter)
	if (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
		SISHideCursor();
#endif
#endif

  	if (enter)
    	{
      		xf86EnableIOPorts(vga256InfoRec.scrnIndex);
		vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
      		outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
      		outb(vgaIOBase + 5, temp & 0x7F);

		outw(0x3C4, 0x8605); 	/* Unlock Specials */
    	}
  	else
    	{
		outw(0x3C4, 0x0005);	/* Lock Specials */

      		xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    	}
}

/*
 * SISRestore --
 *      restore a video mode
 */
static void
SISRestore(restore)
     	vgaSISPtr restore;
{
	outw(0x3C4, ((restore->BankReg) << 8) | 0x06);
	outw(0x3C4, ((restore->Unknown) << 8) | 0x08);
	outw(0x3C4, ((restore->CRTCOff) << 8) | 0x0A);
	outw(0x3C4, ((restore->ClockReg) << 8) | 0x07);
	outw(0x3C4, ((restore->DualBanks) << 8) | 0x0B);
	outw(0x3C4, ((restore->DispCRT) << 8) | 0x27);

	/*
	 * Now restore generic VGA Registers
	 */
	vgaHWRestore((vgaHWPtr)restore);

	outb(0x3C2, restore->ClockReg2);
}

/*
 * SISSave --
 *      save the current video mode
 */
static void *
SISSave(save)
     	vgaSISPtr save;
{
  	save = (vgaSISPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaSISRec));

	outb(0x3C4, 0x06); save->BankReg = inb(0x3C5);
	outb(0x3C4, 0x08); save->Unknown = inb(0x3C5);
	outb(0x3C4, 0x0A); save->CRTCOff = inb(0x3C5);
	outb(0x3C4, 0x07); save->ClockReg = inb(0x3C5);
	outb(0x3C4, 0x0B); save->DualBanks = inb(0x3C5);
	outb(0x3C4, 0x27); save->DispCRT = inb(0x3C5);
	save->ClockReg2 = inb(0x3CC);

  	return ((void *) save);
}

/*
 * SISInit --
 *      Handle the initialization, etc. of a screen.
 */
static Bool
SISInit(mode)
    	DisplayModePtr mode;
{
	unsigned char temp;
	int offset;

	/*
	 * Initialize generic VGA registers.
	 */
	vgaHWInit(mode, sizeof(vgaSISRec));
  
	offset = vga256InfoRec.displayWidth >>
#ifdef MONOVGA
		(mode->Flags & V_INTERLACE ? 3 : 4);
#else
		(mode->Flags & V_INTERLACE ? 2 : 3);

	new->std.Attribute[16] = 0x01;
	new->std.CRTC[20] = 0x40;
	new->std.CRTC[23] = 0xA3;

#if 0
	if (sisUseLinear)
	{
		temp = ((SIS.ChipLinearBase/1024) & 0xFF00) >> 8;
		temp = 0x7C;
		outw(0x3C4, (temp << 8) | 0x20);
	}
#endif
#endif
	new->Unknown = 0x3f;
	new->BankReg = 0x02;
	if (vgaBitsPerPixel == 16) new->BankReg |= 0x08;
	if (vgaBitsPerPixel == 32) new->BankReg |= 0x10;

	new->DualBanks = 0x08;

	new->std.CRTC[0x13] = offset & 0xFF;
	new->CRTCOff = (offset & 0xF00) >> 4;

	if (mode->Flags & V_INTERLACE)
		new->BankReg |= 0x20;

	if (new->std.NoClock >= 0)
	{
		new->ClockReg = new->std.NoClock;
		new->ClockReg2 = inb(0x3CC) | 0x0C;
	}

        return(TRUE);
}

/*
 * SISAdjust --
 *      adjust the current video frame to display the mousecursor
 */

static void 
SISAdjust(x, y)
	int x, y;
{
	unsigned char temp;
	int base;

#ifdef MONOVGA
	base = (y * vga256InfoRec.displayWidth + x + 3) >> 3;
#else
	base = (y * vga256InfoRec.displayWidth + x + 1) >> 2;
#endif

  	outw(vgaIOBase + 4, (base & 0x00FF00) | 0x0C);
	outw(vgaIOBase + 4, ((base & 0x00FF) << 8) | 0x0D);

	outb(0x3C4, 0x27); temp = inb(0x3C5) & 0xF0;
	temp |= (base & 0x0F0000) >> 16;
	outb(0x3C5, temp);

#ifdef XFreeXDGA
	if (vga256InfoRec.directMode & XF86DGADirectGraphics) {
		/* Wait until vertical retrace is in progress. */
		while (inb(vgaIOBase + 0xA) & 0x08);
		while (!(inb(vgaIOBase + 0xA) & 0x08));
	}
#endif
}

/*
 * SISValidMode --
 *
 */
static int
SISValidMode(mode, verbose)
DisplayModePtr mode;
Bool verbose;
{
	return MODE_OK;
}
