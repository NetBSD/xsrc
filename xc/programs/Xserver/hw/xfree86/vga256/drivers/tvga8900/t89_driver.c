/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tvga8900/t89_driver.c,v 3.48 1996/10/24 14:25:20 dawes Exp $ */
/*
 * Copyright 1992 by Alan Hourihane, Wigan, England.
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
 * Author:  Alan Hourihane, alanh@logitek.co.uk, version 0.1beta
 * 	    David Wexelblat, added ClockSelect logic. version 0.2.
 *	    Alan Hourihane, tweaked Init code (5 reg hack). version 0.2.1.
 *	    Alan Hourihane, removed ugly tweak code. version 0.3
 *          		    changed vgaHW.c to accomodate changes.
 * 	    Alan Hourihane, fix Restore called incorrectly. version 0.4
 *
 *	    Alan Hourihane, sent to x386beta team, version 1.0
 *
 *	    David Wexelblat, edit for comments.  Support 8900C only, dual
 *			     bank mode.  version 2.0
 *
 *	    Alan Hourihane, move vgaHW.c changes here for now. version 2.1
 *	    David Wexelblat, fix bank restoration. version 2.2
 *	    David Wexelblat, back to single bank mode.  version 2.3
 *	    Alan Hourihane, fix monochrome text restoration. version 2.4
 *	    Gertjan Akkerman, add TVGA9000 hacks.  version 2.5
 *
 *	    David Wexelblat, massive rewrite to support 8800CS, 8900B,
 *			     8900C, 8900CL, 9000.  Support 512k and 1M.
 *			     Version 3.0.
 *
 *          Alan Hourihane, support for TGUI9440 and compatibles. Version 3.1
 *
 *	    Alan Hourihane, rewrote to support all cards with appropriate 
 * 			    speedups. Linear access, hw cursor etc... v4.0
 *			    Needs more work yet though !
 */
/* $XConsortium: t89_driver.c /main/19 1996/01/26 14:29:55 kaleb $ */

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
#include "t89_driver.h"
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
#include "tgui_drv.h"
#include "vga256.h"
extern vgaHWCursorRec vgaHWCursor;
#endif

typedef struct {
	vgaHWRec std;          		/* std IBM VGA register 	*/
	unsigned char ConfPort;		/* For memory selection 	*/
	unsigned char OldMode2;		/* To enable memory banks 	*/
	unsigned char OldMode1;		/* For clock select		*/
	unsigned char NewMode2;		/* For clock select 		*/
	unsigned char NewMode1;		/* For write bank select 	*/
	unsigned char CRTCModuleTest;	/* For interlace mode 		*/
	unsigned char LinearAddReg;	/* For Linear Register TGUI	*/
	unsigned char MCLK_A;		/* For MCLK (low byte)		*/
	unsigned char MCLK_B;		/* For MCLK (high byte)		*/
	unsigned char VCLK_O;		/* For MiscOutReg		*/
	unsigned char VCLK_A;		/* For Programmable Clock (low) */
	unsigned char VCLK_B;		/* For Programmable Clock (hi)  */
	unsigned char VLBusReg;		/* For VL Bus and 32bit mode 	*/
	unsigned char MiscExtFunc;	/* For Misc. Ext. Functions     */
	unsigned char GraphEngReg;	/* For Graphic Engine Control   */
	unsigned char PCIReg;		/* For PCI Bursts		*/
	unsigned char AddColReg;	/* For Address/Colour Register  */
	unsigned char CommandReg;	/* For DAC Command Register     */
	unsigned char TRDReg;		/* For DAC Setup 		*/
	unsigned char MiscIntContReg;	/* For Misc. Int. Cont. Reg.    */
	unsigned char PixelBusReg;	/* For Pixel Bus Register	*/
	unsigned char CRTHiOrd;		/* For CRTC bit 10, Start Add.  */
	unsigned char AltClock;		/* For Alternate Clock Selection*/
	unsigned char CurConReg;	/* For HW Cursor Control	*/
	unsigned char CursorRegs[16];	/* For Cursor Registers 	*/
#ifdef CYBER938X_SUPPORT
	unsigned char Cyber;		/* For Cyber 938x		*/
#endif
} vgaTVGA8900Rec, *vgaTVGA8900Ptr;

static Bool TVGA8900ClockSelect();
static char *TVGA8900Ident();
static Bool TVGA8900Probe();
static void TVGA8900EnterLeave();
static Bool TVGA8900Init();
static int  TVGA8900ValidMode();
static void *TVGA8900Save();
static void TVGA8900Restore();
static void TVGA8900FbInit();
static void TVGA8900Adjust();
extern void TVGA8900SetRead();
extern void TVGA8900SetWrite();
extern void TVGA8900SetReadWrite();
extern void TGUISetRead();
extern void TGUISetWrite();
extern void TGUISetReadWrite();

vgaVideoChipRec TVGA8900 = {
  TVGA8900Probe,
  TVGA8900Ident,
  TVGA8900EnterLeave,
  TVGA8900Init,
  TVGA8900ValidMode,
  TVGA8900Save,
  TVGA8900Restore,
  TVGA8900Adjust,
  vgaHWSaveScreen,
  (void (*)())NoopDDA,
  TVGA8900FbInit,
  TVGA8900SetRead,
  TVGA8900SetWrite,
  TVGA8900SetReadWrite,
  0x10000,
  0x10000,
  16,
  0xffff,
  0x00000, 0x10000,
  0x00000, 0x10000,
  FALSE,                               /* Uses a single bank */
  VGA_DIVIDE_VERT,
  {0,},
  8,				/* Set to 16 for 512k cards in Probe() */
  FALSE,			/* Linear Addressing */
  0,
  0,
  FALSE,
  FALSE,
  FALSE,
  NULL,
  1,
};

#define new ((vgaTVGA8900Ptr)vgaNewVideoState)

int TVGAchipset;
static int numClocks;
int tridentHWCursorType = 0;
static int tridentReprogrammedMCLK = 0;
int tridentDisplayWidth;
int tridentDACtype = -1;
Bool tridentUseLinear = FALSE;
Bool tridentTGUIProgrammableClocks = FALSE;
Bool tridentIsTGUI = FALSE;
Bool tridentLinearOK = FALSE;
#ifdef CYBER938X_SUPPORT
Bool IsCyber = FALSE;
#endif
static unsigned char DRAMspeed;
static int TridentDisplayableMemory;

static unsigned TGUI_ExtPorts[] = {0x43C8, 0x43C9,};
static int Num_TGUI_ExtPorts =
	(sizeof(TGUI_ExtPorts)/sizeof(TGUI_ExtPorts[0]));

static TGUI_Bpp_Clocks[] = {
	/* 8bpp, 16bpp, 24bpp, 32bpp */
	75000, 31500, 0, 25175, 	/* 80ns DRAM */
	80000, 40000, 0, 25175, 	/* 70ns DRAM */
	108000, 45000,0, 25175,		/* 45ns DRAM */
};

#ifdef PC98_TGUI
#include "pc98_tgui.h"
extern pointer mmioBase;
extern Bool BoardInit(void);
extern PC98TGUiTable *pc98TGUi;
#endif

/*
 * TGUISetClock -- Set programmable clock for TGUI cards !
 */
static
TGUISetClock(no)
	int no;
{
	int clock_diff = 500;
	int freq, ffreq;
	int m, n, k;
	int p, q, r, s; 
	int startn, endn;
	int endm;
	unsigned char temp;

	p = q = r = s = 0;

#ifdef CYBER938X_SUPPORT
	if (IsCyber)
	{
		startn = 64;
		endn = 255;
		endm = 64;
	}
	else
#endif
	{
		startn = 1;
		endn = 122;
		endm = 32;
	}

	freq = vga256InfoRec.clock[no];

	if (vgaBitsPerPixel == 16)
		freq *= 2;
	if (vgaBitsPerPixel == 32)
		freq *= 3;

	for (k=1;k<3;k++)
	  for (n=startn;n<endn;n++)
	    for (m=1;m<endm;m++)
	    {
		ffreq = ((( (n + 8) * 14.31818) / ((m + 2) * k)) * 1000);
		if ((ffreq > freq - clock_diff) && (ffreq < freq + clock_diff)) 
		{
				p = n; q = m; r = k; s = ffreq;
		}
	    }

	if (s == 0)
	{
		TVGA8900EnterLeave(LEAVE);
		FatalError("Unable to set programmable clock.\n"
			   "Frequency %d is not a valid clock.\n"
			   "Please modify XF86Config for a new clock.\n",	
			   freq);
	}

	temp = inb(0x3CC);
	new->VCLK_O = (temp & 0xF3) | 0x08;
#ifdef CYBER938X_SUPPORT
	if (IsCyber)
	{
		new->VCLK_A = p;
		new->VCLK_B = (q & 0x2F) | ((r - 1) << 6);
	}
	else
#endif
	{
		new->VCLK_A = ((1 & q) << 7) | p;
		new->VCLK_B = (((q & 0xFE) >> 1) | ((r - 1) << 4));
	}
}

/*
 * TVGA8900Ident --
 */
static char *
TVGA8900Ident(n)
	int n;
{
	static char *chipsets[] = {"tvga8200lx", "tvga8800cs", "tvga8900b", 
				   "tvga8900c", 
			     	   "tvga8900cl", "tvga8900d", "tvga9000",  
				   "tvga9000i", "tvga9100b",
				   "tvga9200cxr", "tgui9320lcd",
				   "tgui9400cxi", "tgui9420",
				   "tgui9420dgi", "tgui9430dgi",
				   "tgui9440agi", "tgui9660xgi",
				   "tgui9680",
#ifdef CYBER938X_SUPPORT
				   "cyber938x",
#endif
				  };

	if (n + 1 > sizeof(chipsets) / sizeof(char *))
		return(NULL);
	else
		return(chipsets[n]);
}

/*
 * TVGA8900ClockSelect --
 * 	select one of the possible clocks ...
 */
static Bool
TVGA8900ClockSelect(no)
	int no;
{
	static unsigned char save1, save2, save3;
	unsigned char temp;

	/*
	 * CS0 and CS1 are in MiscOutReg
	 *
	 * For 8900B, 8900C, 8900CL and 9000, CS2 is bit 0 of
	 * New Mode Control Register 2.
	 *
	 * For 8900CL, CS3 is bit 4 of Old Mode Control Register 1.
	 *
	 * For 9000, CS3 is bit 6 of New Mode Control Register 2.
	 *
	 * For TGUI, we don't use the ClockSelect function at all.
	 */
	switch(no)
	{
	case CLK_REG_SAVE:
		save1 = inb(0x3CC);
		if (TVGAchipset != TVGA8800CS)
		{
			outw(0x3C4, 0x000B);	/* Switch to Old Mode */
			inb(0x3C5);		/* Now to New Mode */
			outb(0x3C4, 0x0D); save2 = inb(0x3C5);
			if ( (numClocks == 16) && 
			     (TVGAchipset != TVGA9000) &&
			     (TVGAchipset != TVGA9000i) )
			{
				outw(0x3C4, 0x000B);	/* Switch to Old Mode */
				outb(0x3C4, 0x0E); save3 = inb(0x3C5);
			}
		}
		break;
	case CLK_REG_RESTORE:
		outb(0x3C2, save1);
		if (TVGAchipset != TVGA8800CS)
		{
			outw(0x3C4, 0x000B);	/* Switch to Old Mode */
			inb(0x3C5);		/* Now to New Mode */
			outw(0x3C4, (save2 << 8) | 0x0D); 
			if ( (numClocks == 16) && 
			     (TVGAchipset != TVGA9000) &&
			     (TVGAchipset != TVGA9000i) )
			{
				outw(0x3C4, 0x000B);	/* Switch to Old Mode */
				outw(0x3C4, (save3 << 8) | 0x0E);
			}
		}
		break;
	default:
		/*
	 	 * Do CS0 and CS1
	 	 */
		temp = inb(0x3CC);
		outb(0x3C2, (temp & 0xF3) | ((no << 2) & 0x0C));
		if (TVGAchipset != TVGA8800CS)
		{
			/* 
	 	 	 * Go to New Mode for CS2 and TVGA9000 CS3.
	 	 	 */
			outw(0x3C4, 0x000B);	/* Switch to Old Mode */
			inb(0x3C5);		/* Now to New Mode */
			outb(0x3C4, 0x0D);
			/*
			 * Bits 1 & 2 are dividers - set to 0 to get no
			 * clock division.
			 */
			temp = inb(0x3C5) & 0xF8;
			temp |= (no & 0x04) >> 2;
			if ( (TVGAchipset == TVGA9000) ||
			     (TVGAchipset == TVGA9000i) )
			{
				temp &= ~0x40;
				temp |= (no & 0x08) << 3;
			}
			outb(0x3C5, temp);
			if ( (numClocks == 16) && 
			     (TVGAchipset != TVGA9000) &&
			     (TVGAchipset != TVGA9000i) )
			{
				/* 
				 * Go to Old Mode for CS3.
			 	 */
				outw(0x3C4, 0x000B);	/* Switch to Old Mode */
				outb(0x3C4, 0x0E);
				temp = inb(0x3C5) & 0xEF;
				temp |= (no & 0x08) << 1;
				outb(0x3C5, temp);
			}
		}
	}
	return(TRUE);
}

/* 
 * TVGA8900Probe --
 * 	check up whether a Trident based board is installed
 */
static Bool
TVGA8900Probe()
{
  	unsigned char temp;
	int i;

#ifdef PC98_TGUI
	OFLG_SET(OPTION_PC98TGUI, &TVGA8900.ChipOptionFlags);
	if( BoardInit() == FALSE )
		return(FALSE);
#endif
	/*
         * Set up I/O ports to be used by this card
	 */
	xf86ClearIOPortList(vga256InfoRec.scrnIndex);
	xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);

  	if (vga256InfoRec.chipset)
    	{
		/*
		 * If chipset from XF86Config matches...
		 */
		if (!StrCaseCmp(vga256InfoRec.chipset, "tvga8900"))
		{
			ErrorF("\ntvga8900 is no longer valid.  Use one of\n");
			ErrorF("the names listed by the -showconfig option\n");
			return(FALSE);
		}
		if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(0)))
			TVGAchipset = TVGA8200LX;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(1)))
			TVGAchipset = TVGA8800CS;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(2)))
			TVGAchipset = TVGA8900B;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(3)))
			TVGAchipset = TVGA8900C;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(4)))
			TVGAchipset = TVGA8900CL;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(5)))
			TVGAchipset = TVGA8900D;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(6)))
			TVGAchipset = TVGA9000;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(7)))
			TVGAchipset = TVGA9000i;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(8)))
			TVGAchipset = TVGA9100B;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(9)))
			TVGAchipset = TVGA9200CXr;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(10)))
			TVGAchipset = TGUI9320LCD;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(11)))
			TVGAchipset = TGUI9400CXi;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(12)))
			TVGAchipset = TGUI9420;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(13)))
			TVGAchipset = TGUI9420DGi;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(14)))
			TVGAchipset = TGUI9430DGi;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(15)))
			TVGAchipset = TGUI9440AGi;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(16)))
			TVGAchipset = TGUI9660XGi;
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(17)))
			TVGAchipset = TGUI9680;
#ifdef CYBER938X_SUPPORT
		else if (!StrCaseCmp(vga256InfoRec.chipset, TVGA8900Ident(18)))
		{
			TVGAchipset = TGUI9660XGi;
			IsCyber = TRUE;
		}
#endif
		else
			return(FALSE);
		TVGA8900EnterLeave(ENTER);
    	}
  	else
    	{
      		unsigned char origVal, newVal;
      		char *TVGAName;
		char *TreatAs = NULL;
	
      		TVGA8900EnterLeave(ENTER);

      		/* 
       		 * Check first that we have a Trident card.
       		 */
		outb(0x3C4, 0x0B);
		temp = inb(0x3C5);	/* Save old value */
		outb(0x3C4, 0x0B);	/* Switch to Old Mode */
		outb(0x3C5, 0x00);
		inb(0x3C5);		/* Now to New Mode */
      		outb(0x3C4, 0x0E);
      		origVal = inb(0x3C5);
      		outb(0x3C5, 0x00);
      		newVal = inb(0x3C5) & 0x0F;
      		outb(0x3C5, (origVal ^ 0x02));

		/* 
		 * Is it a Trident card ?? 
		 */
      		if (newVal != 2)
		{
			/*
			 * Nope, so quit
			 */
			outb(0x3C4, 0x0B);	/* Restore value of 0x0B */
			outb(0x3C5, temp);
			outb(0x3C4, 0x0E);
			outb(0x3C5, origVal);
	  		TVGA8900EnterLeave(LEAVE);
	  		return(FALSE);
		}

      		/* 
		 * We've found a Trident card, now check the version.
		 */
		TVGAchipset = -1;
      		outb(0x3C4, 0x0B);
		outb(0x3C5, 0x00);
      		temp = inb(0x3C5);
      		switch (temp)
      		{
		case 0x01:
			TVGAName = "TVGA8800BR";
			break;
      		case 0x02:
			TVGAchipset = TVGA8800CS;
      			TVGAName = "TVGA8800CS";
      			break;
      		case 0x03:
			TVGAchipset = TVGA8900B;
      			TVGAName = "TVGA8900B";
      			break;
      		case 0x04:
      		case 0x13:
			TVGAchipset = TVGA8900C;
      			TVGAName = "TVGA8900C";
      			break;
      		case 0x23:
			TVGAchipset = TVGA9000;
      			TVGAName = "TVGA9000";
      			break;
      		case 0x33:
			TVGAchipset = TVGA8900CL;
      			TVGAName = "TVGA8900CL/D";
      			break;
		case 0x43:
			TVGAchipset = TVGA9000i;
			TVGAName = "TVGA9000i";
			break;
		case 0x53:
			TVGAchipset = TVGA9200CXr;
			TVGAName = "TVGA9200CXr";
			break;
		case 0x63:
			TVGAchipset = TVGA9100B;
			TVGAName = "TVGA9100B";
			break;
		case 0x73:
			TVGAchipset = TGUI9420;
			TVGAName = "TGUI9420";
			break;
		case 0xC3:
			TVGAchipset = TGUI9420DGi;
			TVGAName = "TGUI9420DGi";		
			break;
		case 0x83:
			TVGAchipset = TVGA8200LX;
			TVGAName = "TVGA8200LX";
			break;
		case 0x93:
			TVGAchipset = TGUI9400CXi;
			TVGAName = "TGUI9400CXi";
			break;
		case 0xA3:
			TVGAchipset = TGUI9320LCD;
			TVGAName = "TGUI9320LCD";
			break;
		case 0xD3:
			TVGAchipset = TGUI9660XGi;		
			TVGAName = "TGUI96xx";
			break;
		case 0xE3:
			TVGAchipset = TGUI9440AGi;
			TVGAName = "TGUI9440AGi";
			break;
		case 0xF3:
			TVGAchipset = TGUI9430DGi;
			TVGAName = "TGUI9430DGi";
			break;
      		default:
      			TVGAName = "UNKNOWN";
      		}
      		ErrorF("%s Trident chipset version: 0x%02x (%s)\n", 
                       XCONFIG_PROBED, temp, TVGAName);
		if (TreatAs != (char *)NULL)
		{
			ErrorF("%s \tDriver will treat chipset as: %s\n",
			       XCONFIG_PROBED, TreatAs);
		}
		if (TVGAchipset == -1)
		{
			if (temp == 0x01)
			{
				ErrorF("Cannot support 8800BR, sorry\n");
			}
			else
			{
				ErrorF("Unknown Trident chipset.\n");
			}
	  		TVGA8900EnterLeave(LEAVE);
	  		return(FALSE);
		}
    	}

	/* Enable Trident enhancements according to chipset */

     	switch (TVGAchipset)
      	{
      	case TVGA8900D:
		tridentLinearOK = TRUE;
		tridentDACtype = TKD8001;
		TVGA8900.ChipHas16bpp = TRUE;	/* Has HiColor DAC */
      		break;
	case TVGA9200CXr:
		tridentIsTGUI = FALSE;			/* Not a TGUI */
		tridentTGUIProgrammableClocks = FALSE;
		tridentLinearOK = TRUE;
		tridentDACtype = TKD8001;
		TVGA8900.ChipHas16bpp = TRUE;
		break;
	case TGUI9320LCD:
		tridentIsTGUI = TRUE;		/* Reports of this works */
		tridentLinearOK = TRUE;
		tridentDACtype = TGUIDAC;
		TVGA8900.ChipUse2Banks = TRUE;
		tridentTGUIProgrammableClocks = TRUE;
		break;
	case TGUI9400CXi:
		tridentIsTGUI = TRUE;	
		tridentLinearOK = TRUE;
		tridentTGUIProgrammableClocks = FALSE;	/* Not programmable */
		tridentDACtype = TKD8001;
		TVGA8900.ChipHas16bpp = TRUE;
		TVGA8900.ChipUse2Banks = TRUE;
		break;
	case TGUI9430DGi:
		tridentHWCursorType = 2;		/* HW cursor */
		/* Fall through to 9420 - it is the same apart from cursor */
	case TGUI9420:					/* CHECK ME ! */
	case TGUI9420DGi:
		tridentIsTGUI = TRUE;			
		tridentLinearOK = TRUE;
		tridentTGUIProgrammableClocks = FALSE;	/* Not programmable */
		tridentDACtype = TKD8001;
		TVGA8900.ChipHas16bpp = TRUE;
		TVGA8900.ChipUse2Banks = TRUE;
		break;
	case TGUI9440AGi:
	case TGUI9660XGi:
	case TGUI9680:
		tridentIsTGUI = TRUE;
		tridentTGUIProgrammableClocks = TRUE;
		tridentLinearOK = TRUE;
		tridentHWCursorType = 1;
		tridentDACtype = TGUIDAC;
		TVGA8900.ChipHas16bpp = TRUE;
		TVGA8900.ChipUse2Banks = TRUE;
		break;
      	}

	/* 
	 * Set up 2 bank registers 
	 */

	if (TVGA8900.ChipUse2Banks == TRUE)
	{
		TVGA8900.ChipSetRead = TGUISetRead;
		TVGA8900.ChipSetWrite = TGUISetWrite;
		TVGA8900.ChipSetReadWrite = TGUISetReadWrite;
	}
	
 	/* 
	 * How much Video Ram have we got?
	 */
    	if (!vga256InfoRec.videoRam)
    	{
		unsigned char temp;

		outb(vgaIOBase + 4, 0x1F); 
		temp = inb(vgaIOBase + 5);

		switch (temp & 0x07) 
		{
		case 0: 
		case 4:
			vga256InfoRec.videoRam = 256; 
			break;
		case 1: 
		case 5:	/* New TGUI's don't support less than 1MB */
			if ( (TVGAchipset == TGUI9660XGi) || 
			     (TVGAchipset == TGUI9680) )
				vga256InfoRec.videoRam = 4096;
			else
				vga256InfoRec.videoRam = 512; 
			break;
		case 2: 
		case 6:
			vga256InfoRec.videoRam = 768; 
			break;
		case 3: 
			vga256InfoRec.videoRam = 1024; 
			break;
		case 7:
			vga256InfoRec.videoRam = 2048;
			break;
		}
     	}

	if ((vga256InfoRec.videoRam < 1024) && (!tridentTGUIProgrammableClocks))
		TVGA8900.ChipRounding = 16;

	if (!tridentTGUIProgrammableClocks)
	{
		if (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE,
			&vga256InfoRec.clockOptions))
		{
			TVGA8900EnterLeave(LEAVE);
			FatalError("%s %s: Please Remove ClockChip Entry "
				   "from XF86Config file.\n", XCONFIG_GIVEN,
				   vga256InfoRec.name);
		}
	}

	if (tridentTGUIProgrammableClocks) 
	{
		OFLG_SET(OPTION_NO_PROGRAM_CLOCKS, &TVGA8900.ChipOptionFlags);

		/* Do some sanity checking first ! */
		if (vga256InfoRec.clocks)
		{
			TVGA8900EnterLeave(LEAVE);
			FatalError("%s %s: Please Remove Clocks Line from "
				"your XF86Config file.\n",
				XCONFIG_GIVEN, vga256InfoRec.name);
		}
		if (!OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE,
			&vga256InfoRec.clockOptions))
		{
			ErrorF("%s %s: Using Trident programmable clocks\n",
				XCONFIG_PROBED, vga256InfoRec.name);
			OFLG_SET(CLOCK_OPTION_TRIDENT, 
					&vga256InfoRec.clockOptions);
			OFLG_SET(CLOCK_OPTION_PROGRAMABLE, 
					&vga256InfoRec.clockOptions);
		}
		else
		if (OFLG_ISSET(CLOCK_OPTION_TRIDENT,
			&vga256InfoRec.clockOptions))
		{
			ErrorF("%s %s: Using Trident programmable clocks\n",
				XCONFIG_GIVEN, vga256InfoRec.name);
		}
		else
		{
			ErrorF("%s %s: Ignoring unrecognised ClockChip\n",
		   		XCONFIG_GIVEN, vga256InfoRec.name);
			OFLG_SET(CLOCK_OPTION_TRIDENT,
				 &vga256InfoRec.clockOptions);
			ErrorF("%s %s: Using Trident programmable clocks\n",
		       		XCONFIG_PROBED, vga256InfoRec.name);
		}
		if (OFLG_ISSET(OPTION_NO_PROGRAM_CLOCKS,
			&vga256InfoRec.options))
		{
			ErrorF("%s %s: Turning off programmable clock.\n",
				XCONFIG_GIVEN, vga256InfoRec.name);
			tridentTGUIProgrammableClocks = FALSE;
		}
	}

	/*
	 * If clocks are not specified in XF86Config file, probe for them
	 */
    	if ((!vga256InfoRec.clocks) && (!tridentTGUIProgrammableClocks))
	{
		switch (TVGAchipset)
		{
		case TVGA8800CS:
			numClocks = 4;
			break;
		case TVGA8900B:
		case TVGA8900C:
			if (OFLG_ISSET(OPTION_16CLKS,&vga256InfoRec.options))
				numClocks = 16;
			else
				numClocks = 8;
			break;
		default:
			if (OFLG_ISSET(OPTION_NO_PROGRAM_CLOCKS,
				&vga256InfoRec.options))
			{
				numClocks = 4;
			} 
			else
			{
				numClocks = 16;
			}
			break;
		}
		vgaGetClocks(numClocks, TVGA8900ClockSelect);
	}

	/*
	 * Get to New Mode.
	 */
      	outb(0x3C4, 0x0B);
      	inb(0x3C5);	

	vga256InfoRec.chipset = TVGA8900Ident(TVGAchipset);
	vga256InfoRec.bankedMono = TRUE;

#ifndef MONOVGA
	/* For 512k in 256 colour, the pixel clock is half the raw clock */
	if ((vga256InfoRec.videoRam < 1024) && (!tridentIsTGUI))
		TVGA8900.ChipClockScaleFactor = 2;
#endif
	/* Initialize option flags allowed for this driver */
	if ((TVGAchipset == TVGA8900B) || (TVGAchipset == TVGA8900C))
	{
		OFLG_SET(OPTION_16CLKS, &TVGA8900.ChipOptionFlags);
	}

#ifndef MONOVGA
	if (tridentLinearOK)
	{
		OFLG_SET(OPTION_LINEAR, &TVGA8900.ChipOptionFlags);
		OFLG_SET(OPTION_NOLINEAR_MODE, &TVGA8900.ChipOptionFlags);
	}

	if (tridentHWCursorType)
	{
		OFLG_SET(OPTION_HW_CURSOR, &TVGA8900.ChipOptionFlags);
	}

#ifdef XFreeXDGA
	/* We support Direct Video Access */
	vga256InfoRec.directMode = XF86DGADirectPresent;
#endif
#endif /* MONOVGA */

#ifdef MONOVGA
	if ( (TVGAchipset == TVGA8900C) ||
	     (TVGAchipset == TVGA8900B) ||
	     (TVGAchipset == TVGA8800CS) ||
	     (TVGAchipset == TVGA9000) ||
	     (TVGAchipset == TVGA9000i) )
	{
		if (vga256InfoRec.displayWidth > 1152)
		{
			TVGA8900EnterLeave(LEAVE);
			FatalError("%s %s: Chipset supports a max. width"
				   " of 1152, Adjust Virtual in XF86Config.\n",
				   XCONFIG_PROBED, vga256InfoRec.name);
		}
	}
#endif

	if (TVGAchipset >= TGUI9440AGi)
	{
#ifndef MONOVGA
		/* TGUI Accelerator stuff */

		if (vga256InfoRec.virtualX <= 8192)
			tridentDisplayWidth = 8192;
		if (vga256InfoRec.virtualX <= 4096)
			tridentDisplayWidth = 4096;
		if (vga256InfoRec.virtualX <= 2048)
			tridentDisplayWidth = 2048;
		if (vga256InfoRec.virtualX <= 1024)
			tridentDisplayWidth = 1024;
		if (vga256InfoRec.virtualX <= 512)
			tridentDisplayWidth = 512;

		OFLG_SET(OPTION_MMIO, &TVGA8900.ChipOptionFlags);
#endif
	}

	if (vgaPCIInfo && vgaPCIInfo->Vendor == PCI_VENDOR_TRIDENT)
	{
		OFLG_SET(OPTION_TGUI_PCI_READ_OFF,
			&TVGA8900.ChipOptionFlags);
		OFLG_SET(OPTION_TGUI_PCI_WRITE_OFF,
			&TVGA8900.ChipOptionFlags);
	}

	if ( (OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options)) &&
	     (OFLG_ISSET(OPTION_LINEAR, &vga256InfoRec.options)) )
	{
		TVGA8900EnterLeave(LEAVE);
		FatalError("%s %s: Can't have 'nolinear' and 'linear' "
			   "defined. Remove one !\n", XCONFIG_GIVEN,
			   vga256InfoRec.name);
	}
	
	if (tridentIsTGUI)
	{
		OFLG_SET(OPTION_SLOW_DRAM, &TVGA8900.ChipOptionFlags);
		OFLG_SET(OPTION_MED_DRAM, &TVGA8900.ChipOptionFlags);
		OFLG_SET(OPTION_FAST_DRAM, &TVGA8900.ChipOptionFlags);
		
		/*
		 * We set the Max Clock here, as 16 colour allows upto 108MHz
		 * Then, it depends on the DRAM speed for >4Bpp.
		 */
		vga256InfoRec.maxClock = 108000;
		DRAMspeed = 0;
		/*
		 * If DRAM speed isn't specified, assume lowest clock
		 * available for 80ns DRAM, so we don't damage the videocard.
		 */
		if (OFLG_ISSET(OPTION_SLOW_DRAM, &vga256InfoRec.options)) 
		{
			if (TVGAchipset < TGUI9440AGi)
				tridentReprogrammedMCLK = 0xFF50;
			else
				tridentReprogrammedMCLK = 0x02C6; /* 50MHz, 80ns */
			DRAMspeed = 80;
#ifndef MONOVGA
			vga256InfoRec.maxClock = 
				TGUI_Bpp_Clocks[(vgaBitsPerPixel/8)-1];
#endif
		}
		if (OFLG_ISSET(OPTION_MED_DRAM, &vga256InfoRec.options))
		{
			if (TVGAchipset < TGUI9440AGi)
				tridentReprogrammedMCLK = 0xFF40;
			else
				tridentReprogrammedMCLK = 0x0307; /* 58MHz, 70ns */
			DRAMspeed = 70;
#ifndef MONOVGA
			vga256InfoRec.maxClock = 
				TGUI_Bpp_Clocks[3+(vgaBitsPerPixel/8)];
#endif
		}
		if (OFLG_ISSET(OPTION_FAST_DRAM, &vga256InfoRec.options))
		{
			if (TVGAchipset < TGUI9440AGi)
				tridentReprogrammedMCLK = 0xFF00;
			else
				tridentReprogrammedMCLK = 0x00AF; /* 80MHz, 45ns */
			DRAMspeed = 45;
#ifndef MONOVGA
			vga256InfoRec.maxClock =
				TGUI_Bpp_Clocks[7+(vgaBitsPerPixel/8)];
#endif
		}
		/*
		 * If DRAM speed isn't specified, assume lowest clock
		 * available for 80ns DRAM, so we don't damage the videocard.
		 */
		if (DRAMspeed == 0) 
		{
#ifdef PC98_TGUI
			DRAMspeed = pc98TGUi->DRAMspeed;
			tridentReprogrammedMCLK = pc98TGUi->MCLK;
			vga256InfoRec.maxClock =
				pc98TGUi->Bpp_Clocks[(vgaBitsPerPixel/8)-1];
#else
			tridentReprogrammedMCLK = 0; /* Don't Reprogram */
#ifndef MONOVGA
			vga256InfoRec.maxClock =
				TGUI_Bpp_Clocks[(vgaBitsPerPixel/8)-1];
#endif
#endif/*XF98_TGUI*/
		}
		/* Enable extra IO ports for the TGUI */
		xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_TGUI_ExtPorts,
			       TGUI_ExtPorts);
		TVGA8900EnterLeave(LEAVE); /* force update of IO ports */
		TVGA8900EnterLeave(ENTER);
	}

	if (!tridentTGUIProgrammableClocks)
	{
		if (vgaBitsPerPixel == 16)
		{
			TVGA8900.ChipClockScaleFactor = 2;
			vga256InfoRec.maxClock *= 2;
		}
		if (vgaBitsPerPixel == 32)
		{
			TVGA8900.ChipClockScaleFactor = 3;
			vga256InfoRec.maxClock *= 3;
		}
	}

    	return(TRUE);
}

/*
 * TVGA8900FbInit --
 *	enable speedups for the chips that support it
 */
static void
TVGA8900FbInit()
{
	int offscreen_available;
	unsigned char temp;
	tridentUseLinear = FALSE;

#ifndef MONOVGA
	/* If not a TGUI then chipsets don't have any speedups - so exit */
	/* Exception is the 8900CL/D and the 9200CXr as they have linear */
	if (!tridentIsTGUI)
		if ( (TVGAchipset != TVGA8900CL) && 
		     (TVGAchipset != TVGA8900D)  &&
		     (TVGAchipset != TVGA9200CXr) )
			return;

	TVGA8900.ChipLinearSize = vga256InfoRec.videoRam * 1024;

	if (vgaPCIInfo && vgaPCIInfo->Vendor == PCI_VENDOR_TRIDENT)
	{
		if (vgaPCIInfo->MemBase != 0) {
		  TVGA8900.ChipLinearBase = vgaPCIInfo->MemBase;
		  tridentUseLinear = TRUE;
		} else {
		  ErrorF("%s %s: Unable to locate valid FrameBuffer,"
			" Linear Addressing Disabled\n",
		  XCONFIG_PROBED, vga256InfoRec.name);
		  tridentUseLinear = FALSE;
		}
	} 
	else /* VLBus, ISA, EISA */
	{
		/* We disable Linear for boards that aren't PCI */
		/* Unless specifically requested 		*/
		if ( (TVGAchipset == TVGA8900CL) ||
		     (TVGAchipset == TVGA8900D) )
 		{
		  /* This is for the 8900CL/D Linear Buffer */
		  /* 8900CL/D only has 1MB Ram, therefore.. */
		  TVGA8900.ChipLinearBase = (15 * 1024 * 1024);
		}
		else
		{
		  /* set a default of 60MB, must be on 2MB boundary */
		  TVGA8900.ChipLinearBase = (60 * 1024 * 1024);
		}
	}

	if (OFLG_ISSET(OPTION_LINEAR, &vga256InfoRec.options))
 	 	tridentUseLinear = TRUE;

	/* Use Membase when told to, then align on 2MB boundary */
	if (vga256InfoRec.MemBase != 0)
		TVGA8900.ChipLinearBase = vga256InfoRec.MemBase & 0xFFE00000;

	if (tridentIsTGUI)
	  if (tridentReprogrammedMCLK > 0) 
		ErrorF("%s %s: Using %dns DRAM\n", 
			XCONFIG_GIVEN, vga256InfoRec.name, DRAMspeed);
	  else
		ErrorF("%s %s: No DRAM speed specified. Low Clocks Available!\n",
			XCONFIG_PROBED, vga256InfoRec.name);

	if (OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options))
		tridentUseLinear = FALSE;

	if (!xf86LinearVidMem())
		tridentUseLinear = FALSE;

	if (tridentUseLinear) 
		ErrorF("%s %s: Using Linear Frame Buffer at 0x0%x, Size %dMB\n",
			XCONFIG_PROBED, vga256InfoRec.name,
			TVGA8900.ChipLinearBase, 
			TVGA8900.ChipLinearSize/1048576);

	if (tridentUseLinear)
		TVGA8900.ChipUseLinearAddressing = TRUE;

	TridentDisplayableMemory = vga256InfoRec.displayWidth 
					* vga256InfoRec.virtualY
					* (vgaBitsPerPixel / 8);

	offscreen_available = vga256InfoRec.videoRam * 1024 -
				TridentDisplayableMemory;

	if (tridentHWCursorType)
	{
	  if (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
	  {
		if (offscreen_available < 1024)
			ErrorF("%s %s: Not enough off-screen video"
				" memory for hardware cursor, using software cursor.\n",
				XCONFIG_PROBED, vga256InfoRec.name);
		else {
			TridentCursorWidth = 32;
			TridentCursorHeight = 32;
			vgaHWCursor.Initialized = TRUE;
			vgaHWCursor.Init = TridentCursorInit;
			vgaHWCursor.Restore = TridentRestoreCursor;
			vgaHWCursor.Warp = TridentWarpCursor;
			vgaHWCursor.QueryBestSize = TridentQueryBestSize;
			ErrorF("%s %s: Using hardware cursor\n",
				XCONFIG_GIVEN, vga256InfoRec.name);
		}
	  }
	  else
	  {
		ErrorF("%s %s: Using software cursor\n", XCONFIG_PROBED,
							vga256InfoRec.name);
	  }
	}

	/* Acceleration features for the TGUI's */
#if 0
	if (tridentIsTGUI)
	{
		vga256InfoRec.speedup = 0;	/* Turn off all speedups */
		if (vgaBitsPerPixel == 8)
		{
			vga256LowlevFuncs.doBitbltCopy = TridentDoBitbltCopy;
		}
	}
#endif
#endif /* MONOVGA */
}

/*
 * TVGA8900EnterLeave --
 * 	enable/disable io-mapping
 */
static void
TVGA8900EnterLeave(enter)
	Bool enter;
{
  	unsigned char temp;

#ifndef MONOVGA
#ifdef XFreeXDGA
	if (vga256InfoRec.directMode & XF86DGADirectGraphics && !enter)
	{
		if (tridentHWCursorType)
			TridentHideCursor();
		return;
	}
#endif
#endif
  	if (enter)
    	{
      		xf86EnableIOPorts(vga256InfoRec.scrnIndex);
		vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
      		outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
      		outb(vgaIOBase + 5, temp & 0x7F);
#ifdef PC98_TGUI
		pc98TGUi->crtsw(1);
#endif
    	}
  	else
    	{
#ifdef PC98_TGUI
		pc98TGUi->crtsw(0);
#endif
      		xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    	}
}

/*
 * TVGA8900Restore --
 *      restore a video mode
 */
static void
TVGA8900Restore(restore)
     	vgaTVGA8900Ptr restore;
{
	unsigned char temp, tmp;
	int i;

	/*
	 * Go to Old Mode.
	 */
	outw(0x3C4, 0x000B);
	
	/*
	 * Restore Old Mode Control Registers 1 & 2.
	 * Not needed for TGUI - we have a programmable clock.
	 */
	if (!tridentTGUIProgrammableClocks) 
	{
		if ( (numClocks == 16) &&
		     (TVGAchipset != TVGA9000) &&
		     (TVGAchipset != TVGA9000i) )
		{
			if (restore->std.NoClock >= 0)
			{
				outb(0x3C4, 0x0E);
				temp = inb(0x3C5) & 0xEF;
				outb(0x3C5, temp | (restore->OldMode1 & 0x10));
			}
		}
		outw(0x3C4, ((restore->OldMode2) << 8) | 0x0D);
	}

	/*
	 * Now go to New Mode
	 */
	outb(0x3C4, 0x0B);
	inb(0x3C5);

	/*
	 * Unlock Configuration Port Register, then restore:
	 *	Configuration Port Register 1
	 *	New Mode Control Register 2
	 *	New Mode Control Register 1
	 *	CRTC Module Testing Register
	 * 
	 * 	For the TGUI's, Set the Linear Address, 16/32Bpp etc.
	 */
	tmp = 0x82;
	if ((tridentIsTGUI) && (TVGAchipset >= TGUI9440AGi)) tmp |= 0x40;
	outw(0x3C4, (tmp << 8) | 0x0E);

	if (TVGAchipset < TGUI9440AGi) {
		outb(0x3C4, 0x0C);
		temp = inb(0x3C5) & 0xDF;
		outb(0x3C5, temp | (restore->ConfPort & 0x20));
        	if (restore->std.NoClock >= 0)
			outw(0x3C4, ((restore->NewMode2) << 8) | 0x0D);
	}

	outw(vgaIOBase + 4, ((restore->CRTCModuleTest) << 8) | 0x1E);

	if (tridentUseLinear)
		outw(vgaIOBase + 4, ((restore->LinearAddReg) << 8) | 0x21);

	if (tridentHWCursorType == 1)
	{
		outw(vgaIOBase + 4, ((restore->CurConReg) << 8) | 0x50);
		for (i=0;i<16;i++)
			outw(vgaIOBase + 4, 
				((restore->CursorRegs[i]) << 8) | 0x40 | i);
	}

#ifndef MONOVGA
	if ( (tridentDACtype == TKD8001) && (TVGAchipset != TVGA8900D) )
			outb(0x3C7, restore->TRDReg); 
#endif

	/*
	 * Now restore generic VGA Registers
	 */
	vgaHWRestore((vgaHWPtr)restore);

#ifndef MONOVGA
	/* Do the DAC */
	if (tridentDACtype != -1)
	{
		inb(0x3C8);
		inb(0x3C6); inb(0x3C6); inb(0x3C6); inb(0x3C6);
		outb(0x3C6, restore->CommandReg);
		inb(0x3C8);
	}
#endif
	if (tridentTGUIProgrammableClocks)
	{
		if (TVGAchipset != TGUI9320LCD)
			outb(0x3C2, restore->VCLK_O);
		outb(0x43C8, restore->VCLK_A);
		outb(0x43C9, restore->VCLK_B);
	}

	outw(vgaIOBase + 4, ((restore->CRTHiOrd) << 8) | 0x27);
	outw(vgaIOBase + 4, ((restore->AddColReg) << 8) | 0x29);

	if (tridentIsTGUI)
	{
#ifdef CYBER938X_SUPPORT
		if (IsCyber) outw(0x3CE, ((restore->Cyber) << 8) | 0x30);
#endif
#ifndef MONOVGA
		outw(0x3CE, ((restore->MiscExtFunc) << 8) | 0x0F);
#endif
		outw(vgaIOBase + 4, ((restore->VLBusReg) << 8) | 0x2A);
		/*
	 	* Set the MCLK values....
	 	*/
		if (tridentReprogrammedMCLK > 0) 
		{
			if (TVGAchipset < TGUI9440AGi)
			{
				outw(vgaIOBase + 4, ((restore->MCLK_A) << 8) | 0x2C);
				outw(vgaIOBase + 4, ((restore->MCLK_B) << 8) | 0x2B);
			}
			else
			{
				outb(0x43C6, restore->MCLK_A);
				outb(0x43C7, restore->MCLK_B);
			}
		}
		
#ifndef MONOVGA
		if (TVGAchipset >= TGUI9440AGi)
		{
			if (vgaBitsPerPixel > 8)
				outw(vgaIOBase + 4, 
					((restore->PixelBusReg) << 8) | 0x38);
			outw(vgaIOBase + 4, ((restore->GraphEngReg) << 8) | 0x36);
			outw(0x3CE, ((restore->MiscIntContReg) << 8) | 0x2F);
		}
#endif
	}

	if (vgaPCIInfo && vgaPCIInfo->Vendor == PCI_VENDOR_TRIDENT)
		outw(vgaIOBase + 4, ((restore->PCIReg) << 8) | 0x39);

	outw(0x3C4, ((restore->NewMode1 ^ 0x02) << 8) | 0x0E);
}

/*
 * TVGA8900Save --
 *      save the current video mode
 */
static void *
TVGA8900Save(save)
     	vgaTVGA8900Ptr save;
{
	unsigned char temp, tmp;
	int i;

	/*
	 * Get current bank
	 */
	outb(0x3C4, 0x0e); temp = inb(0x3C5);

	/*
	 * Save generic VGA registers
	 */

  	save = (vgaTVGA8900Ptr)vgaHWSave((vgaHWPtr)save, sizeof(vgaTVGA8900Rec));

	/*
	 * Go to Old Mode.
	 */
	outw(0x3C4, 0x000B);

	/*
	 * Save Old Mode Control Registers 1 & 2.
	 * Not needed for TGUI - we have a programmable clock.
	 */
	if (!tridentTGUIProgrammableClocks)
	{
		if ( (numClocks == 16) &&
		     (TVGAchipset != TVGA9000) &&
		     (TVGAchipset != TVGA9000i) )
		{
			outb(0x3C4, 0x0E); save->OldMode1 = inb(0x3C5); 
		}
		outb(0x3C4, 0x0D); save->OldMode2 = inb(0x3C5); 
	}
	
	/*
	 * Now go to New Mode
	 */
	outb(0x3C4, 0x0B);
	inb(0x3C5);

	/*
	 * Unlock Configuration Port Register, then save:
	 *	Configuration Port Register 1
	 *	New Mode Control Register 2
	 *	New Mode Control Register 1
	 *	CRTC Module Testing Register
	 */
	tmp = 0x82;
	if ((tridentIsTGUI) && (TVGAchipset >= TGUI9440AGi)) tmp |= 0x40;
	outw(0x3C4, (tmp << 8) | 0x0E);

	if (tridentUseLinear)
	{
		outb(vgaIOBase + 4, 0x21); 
		save->LinearAddReg = inb(vgaIOBase + 5);
	}

	if (TVGAchipset < TGUI9440AGi)
	{
		outb(0x3C4, 0x0C); save->ConfPort = inb(0x3C5);
		outb(0x3C4, 0x0D); save->NewMode2 = inb(0x3C5);
	}

	if (tridentHWCursorType == 1)
	{
		outb(vgaIOBase + 4, 0x50); save->CurConReg = inb(vgaIOBase + 5);
		for (i=0;i<16;i++)
		{
			outb(vgaIOBase + 4, 0x40 | i);
			save->CursorRegs[i] = inb(vgaIOBase + 5);
		}
	}

	outb(vgaIOBase + 4, 0x1E); save->CRTCModuleTest = inb(vgaIOBase + 5);
	save->NewMode1 = temp;

	/* Do the DAC */
	if (tridentDACtype != -1)
	{
		inb(0x3C8);
		inb(0x3C6); inb(0x3C6); inb(0x3C6); inb(0x3C6);
		save->CommandReg = inb(0x3C6);
		inb(0x3C8);
	}

	outb(vgaIOBase + 4, 0x27); save->CRTHiOrd = inb(vgaIOBase + 5);
	outb(vgaIOBase + 4, 0x29); save->AddColReg = inb(vgaIOBase + 5);

	if (tridentIsTGUI)
	{
#ifdef CYBER938X_SUPPORT
		if (IsCyber) 
		{
			outb(0x3CE, 0x30);
			save->Cyber = inb(0x3CF);
		}
#endif
#ifndef MONOVGA
		outb(0x3CE, 0x0F); save->MiscExtFunc = inb(0x3CF);
#endif
		outb(vgaIOBase + 4, 0x2A); save->VLBusReg = inb(vgaIOBase + 5);
		/*
		 * Save the MCLK values....
	 	 */
		if (tridentReprogrammedMCLK > 0) 
		{
			if (TVGAchipset < TGUI9440AGi)
			{
				outb(vgaIOBase + 4, 0x2C);
				save->MCLK_A = inb(vgaIOBase + 5);
				outb(vgaIOBase + 4, 0x2B);
				save->MCLK_B = inb(vgaIOBase + 5);
			}	
			else
			{
				save->MCLK_A = inb(0x43C6);
				save->MCLK_B = inb(0x43C7);
			}
		}

		if (tridentTGUIProgrammableClocks)
		{
			save->VCLK_O = inb(0x3CC);
			save->VCLK_A = inb(0x43C8);
			save->VCLK_B = inb(0x43C9);
		}

#ifndef MONOVGA
		if (TVGAchipset >= TGUI9440AGi)
		{
			if (vgaBitsPerPixel > 8)
			{
				outb(vgaIOBase + 4, 0x38); 
				save->PixelBusReg = inb(vgaIOBase + 5);
			}
			outb(vgaIOBase + 4, 0x36); 
			save->GraphEngReg = inb(vgaIOBase + 5);
			outb(0x3CE, 0x2F);
			save->MiscIntContReg = inb(0x3CF);
		}
#endif
	}

	if (vgaPCIInfo && vgaPCIInfo->Vendor == PCI_VENDOR_TRIDENT)
	{
		outb(vgaIOBase + 4, 0x39);
		save->PCIReg = inb(vgaIOBase + 5);
	}

#ifndef MONOVGA
	if ( (tridentDACtype == TKD8001) && (TVGAchipset != TVGA8900D) )
			save->TRDReg = inb(0x3C7); 
#endif

  	return ((void *) save);
}

/*
 * TVGA8900Init --
 *      Handle the initialization, etc. of a screen.
 */
static Bool
TVGA8900Init(mode)
    	DisplayModePtr mode;
{
	unsigned char temp;
	int i;
	int offset;

#ifndef MONOVGA
	/*
	 * In 256-color mode, with less than 1M memory, the horizontal
	 * timings and the dot-clock must be doubled.  We can (and
	 * should) do the former here.  The latter must, unfortunately,
	 * be handled by the user in the XF86Config file.
	 */
	if (vga256InfoRec.videoRam < 1024)
	{
		/*
		 * Double horizontal timings.
		 */
		if (!mode->CrtcHAdjusted)
		{
			mode->CrtcHTotal <<= 1;
			mode->CrtcHDisplay <<= 1;
			mode->CrtcHSyncStart <<= 1;
			mode->CrtcHSyncEnd <<= 1;
			mode->CrtcHSkew <<= 1;
			mode->CrtcHAdjusted = TRUE;
		}
		/*
		 * Initialize generic VGA registers.
		 */
		if (!vgaHWInit(mode, sizeof(vgaTVGA8900Rec)))
			return(FALSE);
  
		/*
		 * Now do Trident-specific stuff.  This one is also
		 * affected by the x2 requirement.
		 */
		offset = vga256InfoRec.displayWidth >> 
			(mode->Flags & V_INTERLACE ? 2 : 3);
	} else {
#endif
		/*
		 * Initialize generic VGA Registers
		 */
		if (!vgaHWInit(mode, sizeof(vgaTVGA8900Rec)))
			return(FALSE);

		/*
		 * Now do Trident-specific stuff.
		 */
		offset = vga256InfoRec.displayWidth >>
				(mode->Flags & V_INTERLACE ? 3 : 4);

#ifndef MONOVGA
		switch (tridentDACtype)
		{
		case TGUIDAC:
			if (vgaBitsPerPixel == 8)
			offset = vga256InfoRec.displayWidth >> 3;
			if (vgaBitsPerPixel == 16)
			offset = vga256InfoRec.displayWidth >> 2;
			if (vgaBitsPerPixel == 32)
			offset = vga256InfoRec.displayWidth >> 1;
			break;
		case TKD8001:
			if (vgaBitsPerPixel == 16)
			offset = vga256InfoRec.displayWidth >> 3;
			if (vgaBitsPerPixel == 32)
			offset = vga256InfoRec.displayWidth >> 2;
			break;
		}
	}

	new->std.CRTC[20] = 0x40;
	if (TVGAchipset < TGUI9440AGi)
	{
		new->std.CRTC[23] = 0xA3;
		if (vga256InfoRec.videoRam > 512)
			new->OldMode2 = 0x10;
		else
			new->OldMode2 = 0x00;
	}
	new->NewMode1 = 0x02;
#endif
	if (TVGAchipset < TGUI9440AGi)
	{
		if (vga256InfoRec.videoRam > 512)
			new->ConfPort = 0x20;
		else
			new->ConfPort = 0x00;
	}

	new->CRTCModuleTest = (mode->Flags & V_INTERLACE ? 0x84 : 0x80); 

	if (tridentUseLinear) 
	{
		new->LinearAddReg = 
			  ((TVGA8900.ChipLinearBase >> 24) << 6) |
			  ((TVGA8900.ChipLinearBase >> 20) & 0x0F);
		new->LinearAddReg |= 0x20;	/* Enable Linear */
		if (TVGAchipset < TGUI9440AGi)
			if (TVGA8900.ChipLinearSize == (2048*1024))
				new->LinearAddReg |= 0x10;
	}

	if (tridentReprogrammedMCLK > 0) 
	{
		new->MCLK_A = tridentReprogrammedMCLK & 0x00FF;
		if (TVGAchipset < TGUI9440AGi)
			new->MCLK_B = 0x03;
		else
			new->MCLK_B = 
				(tridentReprogrammedMCLK & 0xFF00) >> 8;
	}

	new->std.CRTC[19] = offset & 0xFF;
	outb(vgaIOBase + 4, 0x29);
	new->AddColReg = inb(vgaIOBase + 5) | ((offset & 0x100) >> 4);
 	new->CRTHiOrd = ((mode->CrtcVSyncStart & 0x400) >> 4) |
 			(((mode->CrtcVTotal - 2) & 0x400) >> 3) |
 			((mode->CrtcVSyncStart & 0x400) >> 5) |
 			(((mode->CrtcVDisplay - 1) & 0x400) >> 6) |
 			0x08;
#ifndef MONOVGA
	if (tridentIsTGUI)
	{
		/* Turn on 32 bit mode - applies to VLBus and PCI */
		outb(vgaIOBase + 4, 0x2A);
		new->VLBusReg = inb(vgaIOBase + 5) | 0x40; /* 32bit mode */
		outb(0x3CE, 0x0F);
		new->MiscExtFunc = inb(0x3CF) | 0x07; /* Set Dual Banks */
	}
	new->CommandReg = 0x00;		/* DAC Standard colourmap */

	if (tridentHWCursorType)
	  if (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options))
		new->std.Attribute[17] = 0x00; /* Black overscan */

	if ( (TVGAchipset == TVGA8900D) ||
	     (TVGAchipset == TVGA9200CXr) ||
	     (TVGAchipset == TGUI9400CXi) ||
	     (TVGAchipset == TGUI9420DGi) ||
	     (TVGAchipset == TGUI9430DGi) )
	{
		if (TVGAchipset != TVGA8900D)
		{
			temp = inb(0x3C7);
			new->TRDReg = temp & 0xBF; 	/* Sierra DAC */
		}
		if (vgaBitsPerPixel == 16)
		{
			new->std.Attribute[17] = 0x00;
			new->CommandReg = 0xE0;
			new->MiscExtFunc |= 0x08;
		}
		if (vgaBitsPerPixel == 32)
		{
			new->std.Attribute[17] = 0x00;
			if (TVGAchipset == TVGA8900D)
				new->CommandReg = 0x42;
			else
				new->CommandReg = 0xC0;
			new->MiscExtFunc |= 0x08;
		}
	}
#endif

	if (vgaPCIInfo && vgaPCIInfo->Vendor == PCI_VENDOR_TRIDENT)
	{
		outb(vgaIOBase + 4, 0x39);
		new->PCIReg = inb(vgaIOBase + 5);
		/* Turn PCI Burst Read and Write ON - By Default ! */
		new->PCIReg |= 0x06;
		if (OFLG_ISSET(OPTION_TGUI_PCI_READ_OFF,
					&vga256InfoRec.options))
			new->PCIReg &= 0xFD;
		if (OFLG_ISSET(OPTION_TGUI_PCI_WRITE_OFF,
					&vga256InfoRec.options))
			new->PCIReg &= 0xFB;
	}

	if (TVGAchipset >= TGUI9440AGi)
	{
#ifdef CYBER938X_SUPPORT
		if (IsCyber) new->Cyber = 0x00;
#endif

		if (TVGAchipset >= TGUI9660XGi)
			new->AddColReg |= (offset & 0x200) >> 4;
#ifndef MONOVGA
		if (OFLG_ISSET(OPTION_MMIO, &vga256InfoRec.options))
		{
			new->GraphEngReg = 0x82; /* Enable MMIO, GER */
			/* mmap MMIO address here ! */
		}
		else
			new->GraphEngReg = 0x80; /* Enable 0x21XX, GER */
		outb(0x3CE, 0x2F);
		new->MiscIntContReg = inb(0x3CF) | 0x04; /* double line width */
		if (vgaBitsPerPixel == 16)
		{
			new->std.Attribute[17] = 0x00;
			new->MiscExtFunc |= 0x08; /* Clock Division by 2 */
			new->CommandReg = 0x30;	 /* 16bpp */
			new->PixelBusReg = 0x04;
		}
		if (vgaBitsPerPixel == 32)
		{
			new->std.Attribute[17] = 0x00;
			new->CommandReg = 0xD0; /* 32bpp */
			new->MiscExtFunc |= 0x40; /* Clock Division by 3 */
			new->PixelBusReg = 0x08;
		}
#endif
	}

	if (new->std.NoClock >= 0)
	{
		if (tridentTGUIProgrammableClocks)
			TGUISetClock(new->std.NoClock);
		else
		{
  			new->NewMode2 = (new->std.NoClock & 0x04) >> 2;
			if ( (TVGAchipset == TVGA9000) ||
			     (TVGAchipset == TVGA9000i) )
				new->NewMode2 |= (new->std.NoClock & 0x08) << 3;
			else
			if (numClocks == 16)
				new->OldMode1 = (new->std.NoClock & 0x08) << 1;
		}
	}
        return(TRUE);
}

/*
 * TVGA8900Adjust --
 *      adjust the current video frame to display the mousecursor
 */
static void 
TVGA8900Adjust(x, y)
	int x, y;
{
	unsigned char temp;
	int base;
	int shift = 0;

#ifndef MONOVGA
	if (vgaBitsPerPixel == 16)
		shift = 1;
	if (vgaBitsPerPixel == 32)
		shift = 2;
	/* 
	 * Go see the comments in the Init function.
	 */
	if (tridentIsTGUI)
		base = (y * vga256InfoRec.displayWidth + x) >> (2 - shift);
	else
	{
		if (vga256InfoRec.videoRam < 1024) 
			base = (y * vga256InfoRec.displayWidth + x + 1) 
								>> (2 - shift);
		else
			base = (y * vga256InfoRec.displayWidth + x + 3) 
								>> (3 - shift);
	}
#else
	base = (y * vga256InfoRec.displayWidth + x + 3) >> 3;
#endif

  	outw(vgaIOBase + 4, (base & 0x00FF00) | 0x0C);
	outw(vgaIOBase + 4, ((base & 0x00FF) << 8) | 0x0D);

	/* CRT bit 16 */
	outb(vgaIOBase + 4, 0x1E); 
	temp = inb(vgaIOBase + 5) & 0xDF;
	temp |= (base & 0x10000) >> 11;
	outb(vgaIOBase + 5, temp);

	/* CRT bits 17-19 */
	outb(vgaIOBase + 4, 0x27); 
	temp = inb(vgaIOBase + 5) & 0xF8;
	temp |= (base & 0xE0000) >> 17;
	outb(vgaIOBase + 5, temp);

#ifdef XFreeXDGA
	if (vga256InfoRec.directMode & XF86DGADirectGraphics) {
		/* Wait until vertical retrace is in progress. */
		while (inb(vgaIOBase + 0xA) & 0x08);
		while (!(inb(vgaIOBase + 0xA) & 0x08));
	}
#endif
}

/*
 * TVGA8900ValidMode --
 *
 */
static int
TVGA8900ValidMode(mode, verbose)
DisplayModePtr mode;
Bool verbose;
{
#ifdef MONOVGA
	if ( (TVGAchipset == TVGA8900C) ||
	     (TVGAchipset == TVGA8900B) ||
	     (TVGAchipset == TVGA8800CS) ||
	     (TVGAchipset == TVGA9000) ||
	     (TVGAchipset == TVGA9000i) )
	{
		if (mode->HDisplay > 1152)
		{
		    if (verbose)
			ErrorF("%s %s: Chipset supports a max. width"
			       " of 1152, Adjust Modes in XF86Config.\n",
			       XCONFIG_PROBED, vga256InfoRec.name);
		    return(MODE_BAD);
		}
	}
#endif

	if ( (tridentDACtype == TKD8001) && (vgaBitsPerPixel > 8) )
	{
		if (mode->Flags & V_INTERLACE)
		{
		    if (verbose)
			ErrorF("%s %s: Chipset does not support Interlaced "
			       "modes at >8bpp\n", XCONFIG_PROBED, 
			       vga256InfoRec.name);
		    return MODE_BAD;
		}
	}
	return MODE_OK;
}
