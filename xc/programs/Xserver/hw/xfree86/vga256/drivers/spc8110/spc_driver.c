/*
 * Copyright 1997,1998 by Thomas Mueller
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Mueller not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.	Thomas Mueller makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS MUELLER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS MUELLER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/spc8110/spc_driver.c,v 1.1.2.3 1999/06/18 13:08:28 hohndel Exp $ */

/*
 * EPSON SPC8110 vga256 driver
 */

#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Procs.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"
#include "vga.h"

#include "xf86_PCI.h"
#include "vgaPCI.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#include "vga256.h"
#include "spc_driver.h"

#include <math.h>

/*
 * Driver data structures.
 */
typedef struct {
    vgaHWRec std;		/* good old IBM VGA	*/
    unsigned char AUX[0x40];
    unsigned char SegSel;
    unsigned char HWCursor[9];	/* CRTC 0x30..0x38 */
} vgaSPC8110Rec, *vgaSPC8110Ptr;

SPC8110cap	spc8110;

#define DEFAULT_LINEAR_BASE 0x03e00000

#define CLOCK_FACTOR 14318180

#define CLOCKVAL(n, d) \
     (((((n) & 0xF) * CLOCK_FACTOR) / ((d) & 0x0F)) / 1000)

#define MAXPCLK		65000

static Bool SPC8110Probe();
static char *SPC8110Ident();
static Bool SPC8110ClockSelect();
static void SPC8110EnterLeave();
static Bool SPC8110Init();
static int SPC8110ValidMode();
static void *SPC8110Save();
static void SPC8110Restore();
static void SPC8110Adjust();
static void SPC8110FbInit();
extern void SPC8110SetRead();
extern void SPC8110SetWrite();
extern void SPC8110SetReadWrite();

extern Bool SPC8110CursorInit();
extern void SPC8110RestoreCursor();
extern void SPC8110WarpCursor();
extern void SPC8110QueryBestSize();

extern void SPC8110BitBlt();
void (*SPC8110oldBitBlt)();
extern void SPC8110cfbFillBoxSolid();
void (*SPC8110oldcfbFillBoxSolid)();
extern void SPC8110cfbFillRectSolidCopy();
void (*SPC8110oldcfbFillRectSolidCopy)();

extern void SPC8110cfbFillSolidSpansGeneral();

extern vgaHWCursorRec vgaHWCursor;

vgaVideoChipRec SPC8110 =
{
    SPC8110Probe,
    SPC8110Ident,
    SPC8110EnterLeave,
    SPC8110Init,
    SPC8110ValidMode,
    SPC8110Save,
    SPC8110Restore,
    SPC8110Adjust,
    vgaHWSaveScreen,
    (void (*)()) NoopDDA,	/* GetMode	*/
    SPC8110FbInit,
    SPC8110SetRead,
    SPC8110SetWrite,
    SPC8110SetReadWrite,
    0x10000,			/* ChipMapSize	*/
    0x10000,			/* ChipSegmentSize */
    16,				/* ChipSegmentShift */
    0xFFFF,			/* ChipSegmentMask */
    0x00000, 0x10000,		/* ChipReadBottom, ChipReadTop	*/
    0x00000, 0x10000,		/* ChipWriteBottom,ChipWriteTop */
    TRUE,			/* ChipUse2Banks, Uses 2 bank */
    VGA_NO_DIVIDE_VERT,		/* ChipInterlaceType -- don't divide verts */
    {0,},			/* ChipOptionFlags */
    8,				/* ChipRounding */
    FALSE,			/* ChipUseLinearAddressing */
    0,				/* ChipLinearBase */
    0,				/* ChipLinearSize */
    FALSE,			/* ChipHas16bpp */
    FALSE,			/* ChipHas24bpp */
    FALSE,			/* ChipHas32bpp */
    NULL,			/* ChipBuiltinModes */
    1,				/* ChipClockScaleFactor */
    1				/* ChipClockDivFactor */
};

#define new ((vgaSPC8110Ptr)vgaNewVideoState)

#define TYPE_SPC8110		0

static SymTabRec chipsets[] =
{
    {TYPE_SPC8110, "spc8110"},
    {-1, ""},
};

static unsigned SPC8110_ExtPorts[] =
{
    0x3CD,			/* segment reg		 */
    0x3DE, 0x3DF,		/* AUXiliaries		 */
    0x3D0, 0x3D1, 0x3D2, 0x3D3	/* BitBlt engine	 */
};

static int Num_SPC8110_ExtPorts = 
	(sizeof(SPC8110_ExtPorts) / sizeof(SPC8110_ExtPorts[0]));

/*
 * SPC8110Ident
 */
static char *
SPC8110Ident(n)
int n;
{
    if (chipsets[n].token < 0)
	return (NULL);
    else
	return (chipsets[n].name);
}

/*
 * SPC8110ClockSelect --
 *	select one of the possible clocks ...
 */
static Bool
SPC8110ClockSelect(no)
int no;
{
    static unsigned char save1;
    unsigned char temp;

    switch (no) {
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
    return TRUE;
}

/*
 * SPC8110Probe --
 *	check whether a SPC8110 chip is installed
 */
static Bool
SPC8110Probe()
{
    unsigned char val;
    unsigned char val1;

    /*
     * Set up I/O ports to be used by this card
     */
    xf86ClearIOPortList(vga256InfoRec.scrnIndex);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_SPC8110_ExtPorts, 
		   SPC8110_ExtPorts);

    if (vga256InfoRec.chipset) {
	/* no auto-detect: chipset is given */
	if (xf86StringToToken(chipsets, vga256InfoRec.chipset) < 0)
	    return FALSE;
	SPC8110EnterLeave(ENTER);
    } else {
	SPC8110EnterLeave(ENTER);

	outb(0x3de, 0x08);	/* primary revision code */
	val = inb(0x3df);

	if ((val & 0xe0) != 0xe0) {
	    SPC8110EnterLeave(LEAVE);
	    return FALSE;
	}
	
	outb(0x3de, 0x0f);	/* secondary revision code */
	val = inb(0x3df);

	if ((val & 0xf8) != 0xa0) {
	    SPC8110EnterLeave(LEAVE);
	    return FALSE;
	}
	
	if (xf86Verbose) {
	    if (val != 0xa2) {
		spcQuirk |= TEST_SAMPLE;
		ErrorF("%s %s: SPC8110 Test Sample detected (0).\n",
			XCONFIG_PROBED, vga256InfoRec.name);
	    } else {
		outb(0x3de, 0x20);	/* PLL1 Set0 Numerator */
		val = inb(0x3df);

		if (val & 0x20)		/* Bit 3 of secondary revision code */
		    ErrorF("%s %s: SPC8110 Mass Production version detected.\n",
			XCONFIG_PROBED, vga256InfoRec.name);
		else {
		    spcQuirk |= TEST_SAMPLE;
		    ErrorF("%s %s: SPC8110 Test Sample detected (1).\n",
			XCONFIG_PROBED, vga256InfoRec.name);
		}
	    }
	}
	vga256InfoRec.chipset = xf86TokenToString(chipsets, TYPE_SPC8110);
    }

    /* video memory: */
    if (!vga256InfoRec.videoRam)
	vga256InfoRec.videoRam = (rdinx(0x3de, 3) & 0x40) ? 512 : 1024;

    /* display type: */
    val = rdinx(0x3de, 0x00);
    if (val & 0x20) {		/* LCD enable? */
	spcLCD = 1;
	if (val & 0x40)		/* CRT enable?	*/
	    ++spcLCD;		/* simultaneous mode */
    }
    else if (val & 0x40) {	/* CRT enable?	*/
	spcCRT = 1;
    } else {
	ErrorF("%s %s: All displays off!\n",
	       XCONFIG_PROBED, vga256InfoRec.name);
	return FALSE;
    }

    /* panel type: */
    if (spcLCD) {
	val = rdinx(0x3de, 0x02);

	if (val & 0x10) {		/* TFT select?	*/
	    ErrorF("%s %s: TFT display\n", XCONFIG_PROBED, vga256InfoRec.name);
	} else {
	    ErrorF("%s %s: LCD display\n", XCONFIG_PROBED, vga256InfoRec.name);
	}
	if (spcLCD > 1) {
	    ErrorF("%s %s: LCD+CRT simultaneous display mode\n",
		   XCONFIG_PROBED, vga256InfoRec.name);
	}
    } else
	ErrorF("%s %s: CRT mode\n", XCONFIG_PROBED, vga256InfoRec.name);

    /* bus type: */
    if (rdinx(0x3de, 0x0d) & 0x10)	/* Configuration feedback register */
	spcPCI = 1;

    /* linear base address: */
    if (spcPCI) {
	int i = 0;
	pciConfigPtr pcr = NULL;

	/* is this still the right way to do things ? */
	if (vgaPCIInfo && vgaPCIInfo->AllCards) {
	  while (pcr = vgaPCIInfo->AllCards[i++]) {
	      if (  (pcr->_vendor == PCI_VENDOR_EPSON) &&
		    (pcr->_command & PCI_CMD_IO_ENABLE) &&
		    (pcr->_command & PCI_CMD_MEM_ENABLE) &&
		    (pcr->_device == PCI_CHIP_SPC8110) )
		      break;
	  }
	  if (pcr) {
	      spcBASE = pcr->_base0;
	  } else
	      ErrorF("%s %s: chip says PCI, but device not in PCI config\n",
	       XCONFIG_PROBED, vga256InfoRec.name);
	}
    } else {
	/* video buffer location */
	val = rdinx(0x3de, 0x1a);
	val1 = rdinx(0x3de, 0x1b);

	if (val == 0 && val1 == 0)
	    spcBASE = vga256InfoRec.MemBase ?
		vga256InfoRec.MemBase : DEFAULT_LINEAR_BASE;
	else {
	    if (vga256InfoRec.MemBase) {
		spcBASE = vga256InfoRec.MemBase;
	    } else {
		/* let's hope this is of some use */
		spcBASE = ((val & 0x0f) << 28) + (val1 << 20);
	    }
	}
    }

    /* MClk: */
    val = spcPll2Num = rdinx(0x3de, 0x26);	/* PLL2 Numerator	*/
    val1 = spcPll2Den = rdinx(0x3de, 0x27);	/* PLL2 Denominator	*/
    spcMClk = CLOCKVAL(val, val1);
    ErrorF("%s %s: MClk %d.%3d MHz\n",
	   XCONFIG_PROBED, vga256InfoRec.name,
	   spcMClk / 1000, spcMClk % 1000);

    /* clocks: */
    vga256InfoRec.maxClock = MAXPCLK;
    if (spcCRT) {
	OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);
	/* Clobber XF86Config clock line */
	if (OFLG_ISSET(XCONFIG_CLOCKS, &vga256InfoRec.xconfigFlag))
	{
	    OFLG_CLR(XCONFIG_CLOCKS, &vga256InfoRec.xconfigFlag);
	    ErrorF("%s %s: using programmable clock, XF86Config clocks specification ignored.\n",
	       XCONFIG_PROBED, vga256InfoRec.name);
	}
    } else if (!vga256InfoRec.clocks) {
	/* in LCD/simultaneous mode we rely on the BIOS to initialize
	 * the whole stuff. Also we should accept only modes which match
	 * the actual panel size.
	 */
	vgaGetClocks(3, SPC8110ClockSelect);
    }

    /* Panel size: */
    if (spcLCD) {
	modinx(0x3de, 0x00, 0x01, 0x01);	/* enable CRTCB access */
	spcPWidth = rdinx(vgaIOBase + 4, 1) * 8;
	spcPHeight = rdinx(vgaIOBase + 4, 0x12) * 4;
	modinx(0x3de, 0x00, 0x01, 0x00);	/* disable CRTCB access */
	ErrorF("%s %s: Panel size %d x %d\n", 
	       XCONFIG_PROBED, vga256InfoRec.name,
	       spcPWidth, spcPHeight);
    }

    if (OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options)) {
	ErrorF("%s %s: Disabling Linear Addressing\n",
	    XCONFIG_PROBED, vga256InfoRec.name);
	spcBASE = 0;
    }
    if (spcBASE) {
	ErrorF("%s %s: linear base address is set to 0x%X.\n",
	    XCONFIG_GIVEN, vga256InfoRec.name, spcBASE);
	SPC8110.ChipLinearBase = spcBASE;
	if (xf86LinearVidMem()) {
	    SPC8110.ChipUseLinearAddressing = TRUE;
	    SPC8110.ChipLinearSize = vga256InfoRec.videoRam * 1024;
	}
    }

    OFLG_SET(OPTION_SW_CURSOR, &SPC8110.ChipOptionFlags);
    OFLG_SET(OPTION_NOLINEAR_MODE, &SPC8110.ChipOptionFlags);
    OFLG_SET(OPTION_LINEAR, &SPC8110.ChipOptionFlags);
    OFLG_SET(OPTION_NOACCEL, &SPC8110.ChipOptionFlags);
    OFLG_SET(OPTION_FIFO_MODERATE, &SPC8110.ChipOptionFlags);
    OFLG_SET(OPTION_FIFO_CONSERV, &SPC8110.ChipOptionFlags);
    OFLG_SET(OPTION_SHOWCACHE, &SPC8110.ChipOptionFlags);

    if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options))
	vga256InfoRec.videoRam -= 1;	/* reserve 1Kb for HW Cursor */

#ifdef XFreeXDGA
    vga256InfoRec.directMode = XF86DGADirectPresent;
#endif

    return TRUE;
}

/*
 * SPC8110FbInit --
 *	initialise the cfb SpeedUp functions
 */

static void
SPC8110FbInit()
{
    /*
     * If hardware cursor is supported, the vgaHWCursor struct should
     * be filled in here.
     */
    if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) {
	ErrorF("%s %s: SPC8110: H/W cursor selected\n",
	    OFLG_ISSET(XCONFIG_SPEEDUP, &vga256InfoRec.xconfigFlag) ?
		XCONFIG_GIVEN : XCONFIG_PROBED,
		vga256InfoRec.name);
	vgaHWCursor.Initialized = TRUE;
	vgaHWCursor.Init = SPC8110CursorInit;
	vgaHWCursor.Restore = SPC8110RestoreCursor;
	vgaHWCursor.Warp = SPC8110WarpCursor;
	vgaHWCursor.QueryBestSize = SPC8110QueryBestSize;
	spcHWCursor = TRUE;
    }

    if (OFLG_ISSET(OPTION_FIFO_MODERATE, &vga256InfoRec.options))
	spcFIFO = 0x10;
    if (OFLG_ISSET(OPTION_FIFO_CONSERV, &vga256InfoRec.options))
	spcFIFO = 0x20;

    if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options)) {
	SPCAccelInit();
    }
}

/*
 * SPC8110EnterLeave --
 *	enable/disable io-mapping
 */

static void
SPC8110EnterLeave(enter)
Bool enter;
{
#ifdef XFreeXDGA
    if (vga256InfoRec.directMode & XF86DGADirectGraphics && !enter)
	return;
#endif

    if (enter) {
	xf86EnableIOPorts(vga256InfoRec.scrnIndex);

	vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
    } else {
	xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}

/*
 * SPC8110Restore --
 *	set (restore) a video mode
 */

static void
SPC8110Restore(restore)
vgaSPC8110Ptr restore;
{
    int i;

    vgaProtect(TRUE);

    outb(0x3CD, 0);

    if (SPC8110.ChipUseLinearAddressing) {
	wrinx(0x3de, 0x1a, restore->AUX[0x1a]);
	wrinx(0x3de, 0x1b, restore->AUX[0x1b]);
    }
    if (spcCRT) {
	/* PLL1 Set #2 */
	wrinx(0x3de, 0x25, restore->AUX[0x25]);
	wrinx(0x3de, 0x24, restore->AUX[0x24]);
	/* Enhanced FIFO register */
	wrinx(0x3de, 0x3c, restore->AUX[0x3c]);
    }
    wrinx(0x3de, 0, restore->AUX[0]);
    wrinx(0x3de, 5, restore->AUX[5]);

    wrinx(0x3de, 0x15, restore->AUX[0x15]);	/* PowerSaveEnable */

    /* gets modified by code in spc_cursor.c */
    if (spcHWCursor) {
	for (i = 0x30; i < 0x39; i++)
	    wrinx(vgaIOBase + 4, i, restore->HWCursor[i-0x30]);
    }

    vgaHWRestore((vgaHWPtr) restore);

    /* looks like we have to do this again.... */
    if (spcCRT) {
	/* PLL1 Set #2 */
	wrinx(0x3de, 0x25, restore->AUX[0x25]);
	wrinx(0x3de, 0x24, restore->AUX[0x24]);
	/* Enhanced FIFO register */
	wrinx(0x3de, 0x3c, restore->AUX[0x3c]);
    }

    outb(0x3CD, restore->SegSel);

    vgaProtect(FALSE);
}

/*
 * SPC8110Save --
 *	save the current video mode
 */

static void *
SPC8110Save(save)
vgaSPC8110Ptr save;
{
    int i;
    unsigned char temp1;

    temp1 = inb(0x3CD);
    outb(0x3CD, 0x00);		/* segment select */
    save = (vgaSPC8110Ptr) vgaHWSave((vgaHWPtr) save, sizeof(vgaSPC8110Rec));
    save->SegSel = temp1;

    for (i = 0; i < sizeof(save->AUX); i++)
	save->AUX[i] = rdinx(0x3de, i);
    save->AUX[0x20] &= ~0x20;	/* RO bit, must be 0 */

    for (i = 0x30; i < 0x39; i++)
	save->HWCursor[i-0x30] = rdinx(vgaIOBase + 4, i);

    return ((void *) save);
}

static void
spcCalcClock(Clock, vclk)
int Clock;
unsigned int *vclk;
{
    /* adapted from sis driver for SPC8110, could need some 
     * cleaning up because of the simpler scheme on the SPC
     */
    int M, N, P, PSN, VLD, PSNx;
    
    int bestM, bestN, bestP, bestPSN, bestVLD;
    double bestError, abest = 42, bestFout;
    double target;
    
    double Fvco, Fout;
    double error, aerror;
    
    
    /*
     *	fd = fref*(Numerator/Denumerator)*(Divider/PostScaler)
     *
     *	M	= Numerator [1:15] 
     *	N	= DeNumerator [1:15]
     *	VLD	= Divider (Vco Loop Divider) : divide by 1, 2
     *	P	= Post Scaler : divide by 1, 2, 3, 4
     *	PSN	= Pre Scaler (Reference Divisor Select) 
     * 
     * result in vclk[]
     */
#define Midx	0
#define Nidx	1
#define VLDidx	2
#define Pidx	3
#define PSNidx	4
#define Fref 14318180
    /* stability constraints for internal VCO -- MAX_VCO also determines 
     * the maximum Video pixel clock */
#define MIN_VCO Fref
#define MAX_VCO 65000000
#define MAX_VLD 1
#define MAX_PSN 0 /* no pre scaler for this chip */
#define MAX_P	1 /* no post scaler for this chip */
    
    int M_min = 1;
    int M_max = 16;
    
    target = Clock * 1000;
    
    for (PSNx = 0; PSNx <= MAX_PSN ; PSNx++) {
	int low_N, high_N;
	double FrefVLDPSN;
	
	PSN = !PSNx ? 1 : 4;
	
	low_N = 1;
	high_N = 16;
	
	for ( VLD = 1 ; VLD <= MAX_VLD ; VLD++ ) {
	    
	    FrefVLDPSN = (double)Fref * VLD / PSN;
	    for (N = low_N; N <= high_N; N++) {
		double tmp = FrefVLDPSN / N;
		
		for (P = 1; P <= MAX_P; P++) {	
		    double Fvco_desired = target * ( P );
		    double M_desired = Fvco_desired / tmp;
		    
		    /* Which way will M_desired be rounded?  
		     *	Do all three just to be safe.  
		     */
		    int M_low = M_desired - 1;
		    int M_hi = M_desired + 1;
		    
		    if (M_hi < M_min || M_low > M_max)
			continue;
		    
		    if (M_low < M_min)
			M_low = M_min;
		    if (M_hi > M_max)
			M_hi = M_max;
		    
		    for (M = M_low; M <= M_hi; M++) {
			Fvco = tmp * M;
			if (Fvco <= MIN_VCO)
			    continue;
			if (Fvco > MAX_VCO)
			    break;
			
			Fout = Fvco / ( P );
			
			error = (target - Fout) / target;
			aerror = (error < 0) ? -error : error;
			if (aerror < abest) {
			    abest = aerror;
			    bestError = error;
			    bestM = M;
			    bestN = N;
			    bestP = P;
			    bestPSN = PSN;
			    bestVLD = VLD;
			    bestFout = Fout;
			}
#ifdef DEBUG1
			ErrorF("Freq. selected: %.2f MHz, M=%d, N=%d, VLD=%d,"
			       " P=%d, PSN=%d\n",
			       (float)(Clock / 1000.), M, N, P, VLD, PSN);
			ErrorF("Freq. set: %.2f MHz\n", Fout / 1.0e6);
#endif
		    }
		}
	    }
	}
    }
    vclk[Midx]	  = bestM;
    vclk[Nidx]	  = bestN;
    vclk[VLDidx]  = bestVLD;
    vclk[Pidx]	  = bestP;
    vclk[PSNidx]  = bestPSN;
#ifdef DEBUG
    ErrorF("Freq. selected: %.2f MHz, M=%d, N=%d, VLD=%d, P=%d, PSN=%d\n",
	   (float)(Clock / 1000.), vclk[Midx], vclk[Nidx], vclk[VLDidx], 
	   vclk[Pidx], vclk[PSNidx]);
    ErrorF("Freq. set: %.2f MHz\n", bestFout / 1.0e6);
#endif
}

static Bool
spcCalcPCLK(clock, den, num)
int clock;
int *den;
int *num;
{
    unsigned vclk[5];

    spcCalcClock(clock, vclk);
    /* %% check where the error is: num/den Midx/Nidx */
    *den = vclk[Nidx];
    *num = vclk[Midx];
    return TRUE;
}

static void
spcCalcFIFO(pclock, p1n, p1d, p2n, p2d, flow, fhigh)
int pclock;
int *flow;
int *fhigh;
{
    float C, K;
    int mclock = 40;

    C = (vga256InfoRec.videoRam <= 512) ? 15.45 : 7.45;
    K = (vga256InfoRec.videoRam <= 512) ? 19.50 : 15.50;

    *flow = (int) floor((C * p1n * p2d) / (4 * p1d * p2n) + 4.0);
    *fhigh = (int) ceil((K * p1n * p2d + 43.3e-9 * 14.31818e6 * p1n * p2d) / 
		  (4 * p1d * p2n) - 1.0);
}

/*
 * SPC8110Init --
 *	Handle the initialization of the VGAs registers
 */

static Bool
SPC8110Init(mode)
DisplayModePtr mode;
{
    int i;

    if (!vgaHWInit(mode, sizeof(vgaSPC8110Rec)))
	return FALSE;

    for (i = 0; i < sizeof(new->AUX); i++)
	new->AUX[i] = rdinx(0x3de, i);
    if (spcHWCursor)
	for (i = 0x30; i < 0x39; i++)
	    new->HWCursor[i-0x30] = rdinx(vgaIOBase + 4, i);

    if (new->std.NoClock >= 0) {
	if (spcLCD) {
	    /* in LCD/simultaneous display mode we rely on
	     * the BIOS to do everything right (no idea wether
	     * this still works on 800x600 panels though)
	     * we select clock 11b in MiscOutReg since this
	     * usually has the FIFO thresholds initialized properly.
	     */
	    new->std.MiscOutReg |= 0x0C;
	} else {
	    int fifoLow;
	    int fifoHigh;
	    int pllDen;
	    int pllNum;
	    int gain;

	    /* program PLL1 set 2 to an adequate frequency, and
	     * calculate FIFO thresholds accordingly.
	     * BTW, we use clock 11b in MiscOutReg here as well.
	     */
	    new->std.MiscOutReg |= 0x0C;
	    if (!spcCalcPCLK(vga256InfoRec.clock[new->std.NoClock],
			     &pllDen, &pllNum))
		return FALSE;

	    new->AUX[0x24] = pllNum & 0x0F;

	    gain = 0;
	    if (vga256InfoRec.clock[new->std.NoClock] <= 51000)
		gain |= 0x40;		/* gain 0 */
	    if (vga256InfoRec.clock[new->std.NoClock] <= 31000)
		gain |= 0x80;		/* gain 1 */

	    if ((spcQuirk & TEST_SAMPLE) == 0)
		new->AUX[0x24] |= gain;

	    new->AUX[0x25] = pllDen & 0x0F;

	    /* now for the FIFO values */
	    spcCalcFIFO(vga256InfoRec.clock[new->std.NoClock], 
			pllNum, pllDen, spcPll2Num, spcPll2Den,
			&fifoLow, &fifoHigh);

	    new->AUX[0x3c] = ((fifoHigh & 0x07) << 4) | (fifoLow & 0x07);
	    if (spcFIFO) {
		if ((new->AUX[0x3c] & 0x0f) + (spcFIFO & 0x0f) < 0x08)
			new->AUX[0x3c] += spcFIFO & 0x0f;
		if ((new->AUX[0x3c] & 0xf0) + (spcFIFO & 0xf0) < 0x80)
			new->AUX[0x3c] += spcFIFO & 0xf0;
	    }
#ifdef DEBUG
	    ErrorF("Pclk %d.%d (%d.%d) num %d den %d  fhigh %d flow %d\n",
		   vga256InfoRec.clock[new->std.NoClock] / 1000,
		   vga256InfoRec.clock[new->std.NoClock] % 1000,
		   CLOCKVAL(pllNum, pllDen) / 1000,
		   CLOCKVAL(pllNum, pllDen) % 1000,
		   pllNum, pllDen, fifoHigh, fifoLow);
#endif
	}
    }

    new->std.CRTC[0x14] |= 0x40;	/* double word select */
    new->std.CRTC[0x13] *= 2;
    new->std.CRTC[0x17] &= ~0x40;
    new->std.CRTC[0x17] |= 0x20;

    new->AUX[0] |= 0x10;	/* Extended address (> 256KB) enable	*/

    if (SPC8110.ChipUseLinearAddressing) {
	new->AUX[5] = 0x92;		/* HR256, GDC Bypass, Linear */
	new->AUX[0x1a] = (SPC8110.ChipLinearBase >> 28) & 0x0F;
	new->AUX[0x1b] = (SPC8110.ChipLinearBase >> 20) & 0xFF;
    } else {
	new->AUX[5] = 0x91;		/* HR256, GDC Bypass, Paging */
    }

    if (spcHWCursor) {
	new->HWCursor[8] = 2;		/* CRTC 0x38 */
#if 0
	new->HWCursor[8] |= 0x80;	/* HW Cursor vertical dbl */
	new->HWCursor[8] |= 0x40;	/* HW Cursor horizontal dbl */
#endif
    }

    new->AUX[0x15] &= 0x0F;		/* disable PowerSaving */

    return TRUE;
}

/*
 * SPC8110Adjust --
 *	adjust the current video frame to display the mousecursor.
 */

static void
SPC8110Adjust(x, y)
int x;
int y;
{
    int BytesPerPix;
    int Base;

    if (OFLG_ISSET(OPTION_SHOWCACHE, &vga256InfoRec.options)) {
	if(y) y += 128;
    }

    BytesPerPix = vga256InfoRec.bitsPerPixel >> 3;
    Base = ((y * vga256InfoRec.displayWidth + x + 1) * BytesPerPix) >> 2;

    /* adjust Base address so it is a non-fractional multiple of BytesPerPix */
    Base -= (Base % BytesPerPix);

    wrinx(vgaIOBase + 4, 0x0C, (Base >> 8) & 0xff);
    wrinx(vgaIOBase + 4, 0x0D, Base & 0xff);

    modinx(0x3de, 5, 0x0C, (Base & 0x30000) >> 14);

#ifdef XFreeXDGA
    if (vga256InfoRec.directMode & XF86DGADirectGraphics) {
	/* Wait until vertical retrace is in progress. */
	while (inb(vgaIOBase + 0xA) & 0x08);
	while (!(inb(vgaIOBase + 0xA) & 0x08));
    }
#endif
}

/*
 * SPC8110ValidMode --
 */
static int
SPC8110ValidMode(mode, verbose)
DisplayModePtr mode;
Bool verbose;
{
    if (mode->Flags & (V_INTERLACE | V_DBLSCAN))
        return MODE_BAD;
    return MODE_OK;
}
