/* $XConsortium: cpq_driver.c /main/6 1996/01/12 12:16:57 kaleb $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/compaq/cpq_driver.c,v 3.15 1996/10/16 14:42:43 dawes Exp $ */
/*
 * Copyright 1993 Hans Oey <hans@mo.hobby.nl>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Hans Oey not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Hans Oey makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * HANS OEY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL HANS OEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */


/*
  This XFree86 driver is intended to blow up your screen
  and crash your disks. Other damage to your system
  may occur. If the software fails this purpose it will
  try to make random changes to program and data files.
  It is provided "as is" without express or implied warranty
  by Hans Oey. On my Compaq LTE Lite/25c it failed completely
  and only gave me a boring X screen.

  The software was based on the other XFree86 drivers. I am
  very gratefull to Thomas Roell and the XFree86 team, but
  stacking up all copyright notices for the next twenty
  years seems a waste of disk space.
*/

#include "X.h"
#include "input.h"
#include "screenint.h"
#include "dix.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "extnsionst.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

typedef struct {
	vgaHWRec std;               /* good old IBM VGA */
	unsigned char PageRegister0;
	unsigned char PageRegister1;
	unsigned char ControlRegister0;
	unsigned char EnvironmentReg;
	unsigned char CRTCOverflow;
} vgaCOMPAQRec, *vgaCOMPAQPtr;


static Bool     COMPAQProbe();
static char *   COMPAQIdent();
static Bool     COMPAQClockSelect();
static void     COMPAQEnterLeave();
static Bool     COMPAQInit();
static int      COMPAQValidMode();
static void *   COMPAQSave();
static void     COMPAQRestore();
static void     COMPAQAdjust();
static void     COMPAQSaveScreen();
extern void     COMPAQSetRead();
extern void     COMPAQSetWrite();
extern void     COMPAQSetReadWrite();

vgaVideoChipRec COMPAQ = {
	COMPAQProbe,
	COMPAQIdent,
	COMPAQEnterLeave,
	COMPAQInit,
	COMPAQValidMode,
	COMPAQSave,
	COMPAQRestore,
	COMPAQAdjust,
	COMPAQSaveScreen,
	(void (*)())NoopDDA,
	(void (*)())NoopDDA,
	COMPAQSetRead,
	COMPAQSetWrite,
	COMPAQSetReadWrite,
	0x10000,
	0x08000,
	15,
	0x7FFF,
	0x00000, 0x08000,
	0x08000, 0x10000,
	TRUE,                                 /* Uses 2 banks */
	VGA_NO_DIVIDE_VERT,
	{0,},
	8,
	FALSE,
	0,
	0,
	FALSE,
	FALSE,
	FALSE,
	NULL,
	1,
};

#define new ((vgaCOMPAQPtr)vgaNewVideoState)

char *COMPAQIdent(n)
int n;
{
	static char *chipsets[] = {"cpq_avga"};

	if (n + 1 > sizeof(chipsets) / sizeof(char *))
		return NULL;
	else
		return chipsets[n];
}


/* 
   COMPAQClockSelect --  select one of the possible clocks ...
*/
static Bool
COMPAQClockSelect(no)
int no;
{
	static unsigned char save1;
	unsigned char temp;

	switch(no) {
	case CLK_REG_SAVE:
		save1 = inb(0x3CC);
		break;
	case CLK_REG_RESTORE:
		outb(0x3C2, save1);
		break;
	default:
		temp = inb(0x3CC);
		outb(0x3C2, (temp & 0xf3) | ((no << 2) & 0x0C));
		break;
	}
	return(TRUE);
}


/*
   COMPAQProbe() checks for a Compaq VGC
   Returns TRUE or FALSE on exit.

   Makes sure the following are set in vga256InfoRec:
     chipset
     videoRam
     clocks
*/
#define VGABIOS_START vga256InfoRec.BIOSbase
#define SIGNATURE_LENGTH 6
#define ATI_SIGNATURE_LENGTH 9
#define BUFSIZE ATI_SIGNATURE_LENGTH	/* but at least 3 bytes */

static Bool
COMPAQProbe()
{
        unsigned char temp, ver;
	Bool found = FALSE;

	/*
	 * Set up I/O ports to be used by this card
	 */
	xf86ClearIOPortList(vga256InfoRec.scrnIndex);
	xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);

	if (vga256InfoRec.chipset) {
		if (StrCaseCmp(vga256InfoRec.chipset, COMPAQIdent(0)))
			return(FALSE);
		COMPAQEnterLeave(ENTER);
	} 
	else {
		COMPAQEnterLeave(ENTER);
		temp = rdinx(0x3ce, 0x0f);
		wrinx(0x3ce, 0x0f, 0x00);
		if (!testinx(0x3ce, 0x45)) {
			wrinx(0x3ce, 0x0f, 0x05);
			if (testinx(0x3ce, 0x45)) {
				ver = rdinx(0x3ce, 0x0c) >> 3;
				switch (ver) {
				case 0x05:	/* AVGA */
				case 0x06:	/* QVision/1024 */
				case 0x0E:	/* QVision/1024 or /1280 */
				case 0x10:	/* AVGA Portable */
					found = TRUE;
					break;
				case 0x03:	/* IVGS */
				default:	/* Unknown */
					break;
				}
				
			}
		}
		wrinx(0x3ce, 0x0f, temp);
		if (!found) {
			COMPAQEnterLeave(LEAVE);
			return(FALSE);
		}
	}
    
	/* Detect how much memory is installed, that's easy :-) */
	if (!vga256InfoRec.videoRam) {
		if ((rdinx(0x3ce, 0x0c) & 0xb8) == 0x30) {
			/* QVision */
			temp = rdinx(0x3ce, 0x0f);
			wrinx(0x3ce, 0x0f, 0x05);
			switch(rdinx(0x3ce, 0x54)) {
			case 0x00:
				vga256InfoRec.videoRam = 1024;
				break;
			case 0x02:
				vga256InfoRec.videoRam = 512;
				break;
			case 0x04:
				vga256InfoRec.videoRam = 1024;
				break;
			case 0x08:
				vga256InfoRec.videoRam = 2048;
				break;
			default:	/* guess */
				vga256InfoRec.videoRam = 512;
				break;
			}
			wrinx(0x3ce, 0x0f, temp);
		}
		else {
			/* AVGA */
			vga256InfoRec.videoRam = 512;
		}
	}

	if (!vga256InfoRec.clocks)
		vgaGetClocks(4, COMPAQClockSelect);  /* 4? clocks available */

	vga256InfoRec.chipset = COMPAQIdent(0);
	vga256InfoRec.bankedMono = FALSE;     /* who cares ;-) */
#ifndef MONOVGA
#ifdef XFreeXDGA
	vga256InfoRec.directMode = XF86DGADirectPresent;
#endif
#endif

	return TRUE;
}

/*
   COMPAQEnterLeave() -- enable/disable io-mapping

   This routine is used when entering or leaving X (i.e., when starting or
   exiting an X session, or when switching to or from a vt which does not
   have an X session running.
*/
static void COMPAQEnterLeave(enter)
Bool enter;
{
#ifndef MONOVGA
#ifdef XFreeXDGA 
	if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter)
		return;
#endif
#endif

	if (enter) {
		xf86EnableIOPorts(vga256InfoRec.scrnIndex);
		vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0; 
	} 
	else {
		xf86DisableIOPorts(vga256InfoRec.scrnIndex);
	}
}

/*
   COMPAQRestore -- restore a video mode
*/

static void COMPAQRestore(restore)
vgaCOMPAQPtr restore;
{
	if (restore->EnvironmentReg == 0x0f)
		outw(0x3ce, 0 << 8 | 0x0f);		/* lock */
	else
		outw(0x3ce, 0x05 << 8 | 0x0f);		/* unlock */

	vgaHWRestore((vgaHWPtr)restore);

	/* Compaq doesn't like the sequencer reset in vgaHWRestore */
	outw(0x3ce, restore->ControlRegister0 << 8 | 0x40);
	outw(0x3ce, restore->CRTCOverflow << 8 | 0x42);
	outw(0x3ce, restore->PageRegister0 << 8 | 0x45);
	outw(0x3ce, restore->PageRegister1 << 8 | 0x46);

	/*
	 * Don't need to do any clock stuff - the bits are in MiscOutReg,
	 * which is handled by vgaHWRestore.
	 */
}

/*
  COMPAQSave -- save the current video mode
*/
static void *COMPAQSave(save)
vgaCOMPAQPtr save;
{
	unsigned char temp0, temp1, temp2, temp3, temp4;

	outb(0x3ce, 0x0f); temp0 = inb(0x3cf);       /* Environment Register */
	outb(0x3ce, 0x40); temp1 = inb(0x3cf);       /* Control Register 0 */
	outb(0x3ce, 0x42); temp2 = inb(0x3cf);       /* CRTC Overflow */
	outb(0x3ce, 0x45); temp3 = inb(0x3cf);       /* Page Register 0 */
	outb(0x3ce, 0x46); temp4 = inb(0x3cf);       /* Page Register 1 */

	save = (vgaCOMPAQPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaCOMPAQRec));

	save->EnvironmentReg = temp0;
	save->ControlRegister0 = temp1;
	save->CRTCOverflow = temp2;
	save->PageRegister0 = temp3;
	save->PageRegister1 = temp4;

	return ((void *) save);
}

/*
  COMPAQInit -- Handle the initialization of the VGAs registers
*/
static Bool COMPAQInit(mode)
DisplayModePtr mode;
{
#ifndef MONOVGA
	/* Double horizontal timings. */
	if (!mode->CrtcHAdjusted) {
		mode->CrtcHTotal <<= 1;
		mode->CrtcHDisplay <<= 1;
		mode->CrtcHSyncStart <<= 1;
		mode->CrtcHSyncEnd <<= 1;
		mode->CrtcHSkew <<= 1;
		mode->CrtcHAdjusted = TRUE;
	}
#endif

	if (!vgaHWInit(mode,sizeof(vgaCOMPAQRec)))
		return(FALSE);
#ifndef MONOVGA
	new->std.Sequencer[0x02] = 0xff; /* write plane mask for 256 colors */
	new->std.CRTC[0x13] = vga256InfoRec.displayWidth >> 3;
	new->std.CRTC[0x14] = 0x40;
#endif

#ifdef MONOVGA
	new->ControlRegister0 = 0x00;
#else
	new->ControlRegister0 = 0x01;
#endif
	new->CRTCOverflow = 0x0;

	new->EnvironmentReg = 0x05;
	new->PageRegister0 = 0x0;
	new->PageRegister1 = 0x0;

	return(TRUE);
}

/*
 * COMPAQAdjust --
 *      adjust the current video frame to display the mousecursor
 */

static void 
COMPAQAdjust(x, y)
     int x, y;
{
#ifdef MONOVGA
	int Base = (y * vga256InfoRec.displayWidth + x + 3) >> 3;
#else
	int Base = (y * vga256InfoRec.displayWidth + x + 1) >> 2;
#endif

	outw(vgaIOBase + 4, (Base & 0x00FF00) | 0x0C);
	outw(vgaIOBase + 4, ((Base & 0x00FF) << 8) | 0x0D);
	outw(0x3ce, ((Base & 0x030000) >> 6) | 0x42);

#ifdef XFreeXDGA
	if (vga256InfoRec.directMode & XF86DGADirectGraphics) {
		/* Wait until vertical retrace is in progress. */
		while (inb(vgaIOBase + 0xA) & 0x08);
		while (!(inb(vgaIOBase + 0xA) & 0x08));
	}
#endif
}

/*
 * COMPAQSaveScreen --
 *	Save registers that can be disrupted by a synchronous reset
 */
static void
COMPAQSaveScreen(mode)
int mode;
{

	/*
	 * Allow pairs of calls to COMPAWSaveScreen() to be nested.
	 * It may be sufficient to ignore all but the outer-most pair,
	 * but this method is probably safest for now.
	 */

#define MAX_NEST_DEPTH 2
#ifdef CR0
#undef CR0
#endif

	static struct save {
		unsigned char PR0, PR1, CR0;
	} regsave[MAX_NEST_DEPTH + 1];

	static int nest_depth = 0;

	if (mode == SS_START)
	{
		if (nest_depth > MAX_NEST_DEPTH)
		{
		    ErrorF("COMPAQSaveScreen: Warning: too much nesting\n");
		    nest_depth++;
		    return;
		}
		outb(0x3ce, 0x45);
		regsave[nest_depth].PR0 = inb(0x3cf);  /* Page Register 0 */
		outb(0x3ce, 0x46);
		regsave[nest_depth].PR1 = inb(0x3cf);  /* Page Register 1 */
		outb(0x3ce, 0x40);
		regsave[nest_depth].CR0 = inb(0x3cf);  /* Control Register 0 */
		nest_depth++;
		outb(0x3c4, 0x0100);		       /* Start reset */
	}
	else
	{
		outb(0x3c4, 0x0300);		       /* End reset */
		nest_depth--;
		if (nest_depth > MAX_NEST_DEPTH)
			return;
		outw(0x3ce, regsave[nest_depth].CR0 << 8 | 0x40);
		outw(0x3ce, regsave[nest_depth].PR1 << 8 | 0x46);
		outw(0x3ce, regsave[nest_depth].PR0 << 8 | 0x45);
	}
}

/*
 * COMPAQValidMode --
 *
 */
static int
COMPAQValidMode(mode, verbose)
DisplayModePtr mode;
Bool verbose;
{
return MODE_OK;
}
