/* $XConsortium: ct_driver.c /main/6 1996/01/12 12:16:39 kaleb $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/chips/ct_driver.c,v 3.27 1996/10/24 12:31:22 dawes Exp $ */
/*
 * Copyright 1993 by Jon Block <block@frc.com>
 * Modified by Mike Hollick <hollick@graphics.cis.upenn.edu>
 * Modified 1994 by Régis Cridlig <cridlig@dmi.ens.fr>
 *
 * Major Contributors to XFree 3.2
 *   Modified 1995/6 by Nozomi Ytow
 *   Modified 1996 by Egbert Eich <Egbert.Eich@Physik.TH-Darmstadt.DE>
 *   Modified 1996 by David Bateman <dbateman@ee.uts.edu.au>
 *   Modified 1996 by Xavier Ducoin <xavier@rd.lectra.fr>
 *
 * Contributors to XFree 3.2
 *   Modified 1995/6 by Ken Raeburn <raeburn@raeburn.org>
 *   Modified 1996 by Shigehiro Nomura <nomura@sm.sony.co.jp>
 *   Modified 1996 by Marc de Courville <courvill@sig.enst.fr>
 *   Modified 1996 by Adam Sulmicki <adam@cfar.umd.edu>
 *   Modified 1996 by Jens Maurer <jmaurer@cck.uni-kl.de>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This driver is still worked on, however we believe it to be fairly
 * stable and complete.
 */

#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#include "vga.h"

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

#include "ct_driver.h"

/* Chips&Technologies internal specific variables */

/* Capabilities */
Bool ctLinearSupport = FALSE;	 /*linear addressing enable */
Bool ctAccelSupport = FALSE;	 /*acceleration enable */
Bool ctHDepth = FALSE;		 /*Chip has 16/24bpp */

/* Frame Buffer related */
unsigned long ctFrameBufferSize = 0;

/* MMIO related */
Bool ctSupportMMIO = FALSE;
Bool ctUseMMIO = FALSE;
unsigned char *ctMMIOBase = NULL;

/* Chip type */
int ct65545subtype = 0;
Bool ctisHiQV32 = FALSE;	  /*New architecture used in 65550 and 65554 */

/* Display related */
Bool ctLCD = TRUE;
Bool ctCRT = FALSE;
Bool ctPCI = FALSE;
/* Panel Types */
unsigned char ctPanelType = 0;
#define TFT 1
#define SS 2			       /* STN Types */
#define DS 4
#define DD 6
#define IS_STN(X) X&6

/* Bus related */
int ctBusType = 0;

/* current Mode */
Bool ctXMode = FALSE;		  /* we start with console mode */

/* IO Base */
unsigned char ctVgaIOBaseFlag = 0xFF;
unsigned int ctCRindex;
unsigned int ctCRvalue;
unsigned int ctST01reg;

/* Dummies used to temporarily store values */
unsigned char ctSWTmp;

/* HW cursor related */
Bool ctHWCursor = FALSE;
extern void CHIPSCursorInit();
extern void CHIPSRestoreCursor();
extern void CHIPSWarpCursor();
extern void CHIPSQueryBestSize();
extern vgaHWCursorRec vgaHWCursor;
unsigned int ctCursorAddress = 0;  /* The address in video ram of the cursor */

/* Clock related */

typedef struct {
    unsigned char msr;
    unsigned char xr54;
    unsigned char xr33;
    unsigned char fr03;
    int Clock;
} ctClockReg, *ctClockPtr;

int ctCurrentClock;
static unsigned char ctClockType;
static unsigned char ctConsole_clk[3];

#define TYPE_HW 0x01
#define TYPE_PROGRAMMABLE 0x02
#define OLD_STYLE 0x10
#define NEW_STYLE 0x20

#define LCD_TEXT_CLK_FREQ 25000	    /* lcd textclock if TYPE_PROGRAMMABLE */
#define CRT_TEXT_CLK_FREQ 28000     /* crt textclock if TYPE_PROGRAMMABLE */

static void ctClockSave();
static void ctClockLoad();
static Bool ctClockFind();
static void ctCalcClock();
static void ctScaleClock();


/* Blitter related */
unsigned int ctBLTPatternAddress = 0; /*address in video ram of tile pattern*/
unsigned char *ctBltDataWindow = NULL;
Bool ctAvoidImageBLT = FALSE;
int ctReg32MMIO[]={0x83D0,0x87D0,0x8BD0,0x8FD0,0x93D0,0x97D0,0x9BD0,0x9FD0,
		   0xA3D0,0xA7D0,0xABD0,0xAFD0,0xB3D0};
int ctReg32HiQV[]={0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20};
int * ctMMIO;
#ifndef MONOVGA
extern GCOps cfb16TEOps1Rect, cfb16TEOps, cfb16NonTEOps1Rect, cfb16NonTEOps;
extern GCOps cfb24TEOps1Rect, cfb24TEOps, cfb24NonTEOps1Rect, cfb24NonTEOps;
#endif

/* Driver data structures. */
struct {
    int HDisplay;
    int HRetraceStart;
    int HRetraceEnd;
    int HTotal;
    int VDisplay;
} ctSize;

typedef struct {
    vgaHWRec std;		    /* good old IBM VGA */
    unsigned char Port_3D6[0xFF];   /* Chips & Technologies Registers */
    unsigned char Port_3D0[0x80];
    unsigned char Port_3D4[0x80];   /* Storage for the CT specific CRT regs */
    unsigned long BltReg[0xD];	    /* Storage for the HiQV BitBLT registers */
    ctClockReg ctClock;
    Bool XMode;
    unsigned char Port_3DA;         /* Read at Port 3CA */
} vgaCHIPSRec, *vgaCHIPSPtr;


/* Forward definitions for the functions that make up the driver. */
static Bool CHIPSProbe();
static char *CHIPSIdent();
static Bool CHIPSClockSelect();
static void CHIPSEnterLeave();
static Bool CHIPSInit();
static Bool CHIPSInit655xx();
static Bool CHIPSInitHiQV32();
static int  CHIPSValidMode();
static void *CHIPSSave();
static void CHIPSSaveScreen();
static void CHIPSRestore();
static void CHIPSAdjust();
static void CHIPSFbInit();
#if 0			/*it is not used but left for the future */
static void CHIPSGetMode();
#endif


/* Bank select functions. */
extern void CHIPSSetRead();
extern void CHIPSSetWrite();
extern void CHIPSSetReadWrite();
extern void CHIPSHiQVSetRead();
extern void CHIPSHiQVSetWrite();
extern void CHIPSHiQVSetReadWrite();

/*internal functions */
static void ctRestore();
int ctVideoMode();
#if defined(DEBUG) && defined(CT_HW_DEBUG)
void ctHWDebug();
#endif

vgaVideoChipRec CHIPS =
{
    CHIPSProbe,
    CHIPSIdent,
    CHIPSEnterLeave,
    CHIPSInit,
    CHIPSValidMode,
    CHIPSSave,
    CHIPSRestore,
    CHIPSAdjust,
    CHIPSSaveScreen,
    (void (*)())NoopDDA,
    CHIPSFbInit,
    CHIPSSetRead,	
    CHIPSSetWrite,	
    CHIPSSetReadWrite,	

    0x10000,		
    0x08000,		
    15,
    0x7FFF,
    0x0000, 0x08000,
    0x08000, 0x10000,
    TRUE,
    VGA_DIVIDE_VERT,
    {0,},
    8,				       /* ChipRounding */
    FALSE,			       /* ChipUseLinearAddressing */
    0,				       /* ChipLinearBase */
    0,				       /* ChipLinearSize */
    FALSE,			       /* ChipHas16bpp */
    FALSE,			       /* ChipHas24bpp */
    FALSE,			       /* ChipHas32bpp */
    NULL,
    1,
};

#define new ((vgaCHIPSPtr)vgaNewVideoState)

static unsigned CHIPS_ExtPorts[] =
{
    0x46E8,
    0x4AE8,
    0x102,
    0x103,
    0x3C3,
    0x3C4,
    0x3C5,
    0x3C6,
    0x3C7,
    0x3D0,
    0x3D6,		           /*Chips & Technologies index, R/W by word */
    0x3D7,		           /*Chips & Technologies R/W by byte        */
    0x3D8,
  };

unsigned int CHIPS_ExtPorts32[] =
{
  /*BitBLT */
    0x83D0,			       /*DR0 src/dest offset                 */
    0x87D0,			       /*DR1 BitBlt. address of freeVram?    */
    0x8BD0,			       /*DR2 BitBlt. paintBrush, or tile pat.*/
    0x8FD0,                            /*DR3                                 */
    0x93D0,			       /*DR4 BitBlt.                         */
    0x97D0,			       /*DR5 BitBlt. srcAddr, or 0 in VRAM   */
    0x9BD0,			       /*DR6 BitBlt. dest?                   */
    0x9FD0,			       /*DR7 BitBlt. width << 16 | height    */
  /*H/W cursor */
    0xA3D0,			       /*DR8 write/erase cursor              */
		                       /*bit 0-1 if 0  cursor is not shown
		                        * if 1  32x32 cursor
					* if 2  64x64 cursor
					* if 3  128x128 cursor
					*/
                                        /* bit 7 if 1  cursor is not shown   */
		                        /* bit 9 cursor expansion in X       */
		                        /* bit 10 cursor expansion in Y      */
    0xA7D0,			        /* DR9 foreGroundCursorColor         */
    0xABD0,			        /* DR0xA backGroundCursorColor       */
    0xAFD0,			        /* DR0xB cursorPosition              */
		                        /* bit 0-7       x coordinate        */
		                        /* bit 8-14      0                   */
		                        /* bit 15        x signum            */
		                        /* bit 16-23     y coordinate        */
		                        /* bit 24-30     0                   */
		                        /* bit 31        y signum            */
    0xB3D0,			        /* DR0xC address of cursor pattern   */
};

static int Num_CHIPS_ExtPorts =
(sizeof(CHIPS_ExtPorts) / sizeof(CHIPS_ExtPorts[0]));

static int Num_CHIPS_ExtPorts32 =
(sizeof(CHIPS_ExtPorts32) / sizeof(CHIPS_ExtPorts32[0]));

#define CT_520   0
#define CT_530   1
#define CT_540   2
#define CT_545   3
#define CT_546   4
#define CT_548   5
#define CT_550   6
#define CT_554   7
#ifdef CT45X_SUPPORT
/* CT_451 - CT457 are not supported */
#define CT_451   8
#define CT_452   9
#define CT_453   10
#define CT_455   11
#define CT_456   12
#define CT_457   13
#endif

static unsigned char CHIPSchipset;

static char *
CHIPSIdent(n)
    int n;
{
    static char *chipsets[] =
    {
	"ct65520", "ct65530", "ct65540", "ct65545",
	"ct65546", "ct65548", "ct65550", "ct65554",
#ifdef CT45X_SUPPORT
	"ct451", "ct452", "ct453", "ct455",
	"ct456", "ct457",
#endif
    };

#ifdef DEBUG
    ErrorF("\nCHIPSIdent ");
#endif
    if (n + 1 > sizeof(chipsets) / sizeof(char *))
	    return (NULL);

    else
	return (chipsets[n]);
}

#define write_xr(num,val) {outb(0x3D6, num);outb(0x3D7, val);}
#define read_xr(num,var) {outb(0x3D6, num);var=inb(0x3D7);}
#define write_fr(num,val) {outb(0x3D0, num);outb(0x3D1, val);}
#define read_fr(num,var) {outb(0x3D0, num);var=inb(0x3D1);}

static Bool
CHIPSClockSelect(no)
    int no;
{
    static ctClockReg SaveClock;
    static ctClockPtr Clock = &SaveClock;
    ctClockReg TmpClock;

    switch (no) {
    case CLK_REG_SAVE:
	ctClockSave(&SaveClock);
	break;

    case CLK_REG_RESTORE:
	ctClockLoad(ctClockType, &SaveClock);
	break;

    default:
	if (!ctClockFind(ctClockType, no, &TmpClock))
	    return (FALSE);
	ctClockLoad(ctClockType, &TmpClock);
    }
    return (TRUE);
}

/*
 * 
 * Fout = (Fref * 4 * M) / (PSN * N * (1 << P) )
 * Fvco = (Fref * 4 * M) / (PSN * N)
 * where
 * M = XR31+2
 * N = XR32+2
 * P = XR30[3:1]
 * PSN = XR30[0]? 1:4
 * 
 * constraints:
 * 4 MHz <= Fref <= 20 MHz (typ. 14.31818 MHz)
 * 150 kHz <= Fref/(PSN * N) <= 2 MHz
 * 48 MHz <= Fvco <= 220 MHz
 * 2 < M < 128
 * 2 < N < 128
 */

static void
ctClockSave(Clock)
    ctClockPtr Clock;
{
    unsigned char temp;

    Clock->msr = (inb(0x3CC) & 0xFE);	/* save the standard VGA clock 
				      	 * registers */
    if (ctisHiQV32) {
	read_fr(0x03, Clock->fr03);  /* save alternate clock select reg.  */
	if (!ctCurrentClock) {	     /* save 65550+ console clock         */
	    temp = (Clock->fr03 & 0xC) >> 2;
	    if (temp == 3)
		temp = 2;
	    temp = temp << 2;
	    read_xr(0xC0 + temp, ctConsole_clk[0]);
	    read_xr(0xC1 + temp, ctConsole_clk[1]);
	    read_xr(0xC2 + temp, ctConsole_clk[2]);
	    read_xr(0xC3 + temp, ctConsole_clk[3]);
	}
    } else {
	read_xr(0x54, Clock->xr54);    /* save alternate clock select reg.   */
	read_xr(0x33, Clock->xr33);    /* get status of MCLK/VCLK select reg.*/
    }
    Clock->Clock = ctCurrentClock;     /* save current clock frequency       */
#ifdef DEBUG
    ErrorF("saved \n");
#endif
}

static void
ctClockLoad(Type, Clock)
    ctClockPtr Clock;
{
    volatile unsigned char temp, temp33, temp54, tempf03;

    if (ctisHiQV32) {
	read_fr(0x03, tempf03);	   /* save alternate clock select reg.  */
    } else {
	read_xr(0x33, temp33);	   /* get status of MCLK/VCLK select reg */
	read_xr(0x54, temp54);
    }

    /* Only write to soft clock registers if we really need to */
    if (Type == TYPE_PROGRAMMABLE) {
	unsigned char vclk[3];
       
	/* select fixed clock 0  before tampering with VCLK select */
	temp = inb(0x3CC);
	outb(0x3C2, (temp & ~0x0D) | ctVgaIOBaseFlag);
	if (ctisHiQV32) {
	    write_fr(0x03, (tempf03 & ~0x0C) | 0x04);
	    if (!Clock->Clock) {       /* Hack to load saved console clock */
		temp = (Clock->fr03 & 0xC) >> 2;
		if (temp == 3)
		    temp = 2;
		temp = temp << 2;
		write_xr(0xC0 + temp, (ctConsole_clk[0] & 0xFF));
		write_xr(0xC1 + temp, (ctConsole_clk[1] & 0xFF));
		write_xr(0xC2 + temp, (ctConsole_clk[2] & 0xFF));
		write_xr(0xC3 + temp, (ctConsole_clk[3] & 0xFF));
	    } else {
		/* 
		 * Don't use the extra 2 bits in the M, N registers available
		 *  on the 65550, so write zero to 0xCA 
		 */
		ctCalcClock(Clock->Clock, vclk);
		write_xr(0xC8, (vclk[1] & 0xFF));
		write_xr(0xC9, (vclk[2] & 0xFF));
		write_xr(0xCA, 0x0);
		write_xr(0xCB, (vclk[0] & 0xFF));
	    }
	} else {
	    ctCalcClock(Clock->Clock, vclk);
	    write_xr(0x54, (temp54 & 0xF3) | 0x04);
	    write_xr(0x33, temp33 & ~0x20);
	    write_xr(0x30, vclk[0]);
	    write_xr(0x31, vclk[1]);     /* restore VCLK regs.   */
	    write_xr(0x32, vclk[2]);
	}
	usleep(10000);		         /* Let VCO stabilise    */
    }

#ifdef IO_DEBUG
    ErrorF("ctClockLoad: 0x3C2: %X ->",inb(0x3CC));
#endif
    outb(0x3C2, (Clock->msr & 0xFE) | ctVgaIOBaseFlag);
#ifdef IO_DEBUG
    ErrorF(" %X\n",inb(0x3CC));
#endif
    if (ctisHiQV32) {
	write_fr(0x03, ((tempf03 & ~0x0C) | (Clock->fr03 & 0x0C)));
    } else {
	temp33 = (temp33 & ~0x80) | (Clock->xr33 & 0x80);
	write_xr(0x33, temp33);	         /* restore Select reg.           */
	/* 
	 * bit 2 and 3 correspond to bit 2 and 3 (clock select reg.) of 0x3C2
	 * bit 3 and 4 correspond to bit 0 and 1 (extended clock select reg.)
	 * in FCR which does not work reliably and should not be used !
	 * in 0x3DA; we make sure not to touch any other bits.                
	 */
	write_xr(0x54, ((temp54 & 0xF3) | (Clock->xr54 & ~0xF3)));
    }
    ctCurrentClock = Clock->Clock;
#ifdef DEBUG
    ErrorF("restored\n");
#endif
}

static Bool
ctClockFind(Type, no, Clock)
    unsigned char Type;
    int no;
    ctClockPtr Clock;
{
    if (no > (vga256InfoRec.clocks - 1))
	return (FALSE);

    switch (Type & 0x0F) {
    case TYPE_PROGRAMMABLE:
	Clock->msr = 3 << 2;
	Clock->fr03 = Clock->msr;      /* 65550+           */
	Clock->xr33 = 0;	       /* 65520-548        */
	Clock->xr54 = Clock->msr;      /* 65520-548        */
	Clock->Clock = vga256InfoRec.clock[no];
	if (!ctisHiQV32)
		Clock->Clock *= vgaBytesPerPixel;
	break;

    case TYPE_HW:
	if ((Type & 0x0F) == OLD_STYLE) {	/* Old Chipsets */
	    Clock->msr = (no << 2) & 0xC;
	    Clock->xr54 = Clock->msr;
	    Clock->xr33 = 0;
	} else {
	    Clock->msr = (no == 4 ? 3 << 2: (no & 0x01) << 2);
	    Clock->fr03 = Clock->msr;  /* 65550+        */
	    Clock->xr54 = Clock->msr;  /* 65540-548     */
	    Clock->xr33 = no > 1 ? 0x80 : 0;	/* 65540-548     */
	}
	break;
    }
    Clock->msr |= (inb(0x3CC) & 0xF2);

#ifdef DEBUG
    ErrorF("found\n");
#endif
    return (TRUE);
}

int ctGetHWClock(Type)
unsigned char Type;
{
    unsigned char temp, temp1;

    if (!(Type & TYPE_HW))
        return 0;		/* shouldn't happen                   */
    switch (Type & 0xF0){
    case OLD_STYLE:
	return ((inb(0x3CC) & 0x0C) >> 2);
    case NEW_STYLE:
	if (ctLCD) {
	    read_xr(0x54, temp);
	} else
	temp = inb(0x3CC);
	temp = (temp & 0x0C) >> 2;
	if (temp & 0x02)	/* bios set SWClock doesn't get changed */
	    return (4);		/* it's No. 4                           */
	else {
	    read_xr(0x33, temp1); 
	    temp1 = temp1 >> 6; /* iso mode 25.175/28.322 or 32/36 MHz  */
	    return (temp + (temp1 & 0x02)); /* ^=0    ^=1     ^=2 ^=3   */  
	} 
    default:			/* we should never get here             */
        return (0);
    }
}
   
#define Fref 14318180

/* 
 * This is Ken Raeburn's <raeburn@raeburn.org> clock
 * calculation code just modified a little bit to fit in here.
 */

void
ctCalcClock(int Clock, unsigned char *vclk)
{
    int M, N, P, PSN, PSNx;

    int bestM, bestN, bestP, bestPSN;
    double bestError, abest = 42, bestFout;
    double target;

    double Fvco, Fout;
    double error, aerror;

    int M_min = 3;

    /* Hack to deal with problem of Toshiba 720CDT clock */
    int M_max = ctisHiQV32 ? 63 : 127;


    /* Other parameters available on the 65548 but not the 65545, and
     * not documented in the Clock Synthesizer doc in rev 1.0 of the
     * 65548 datasheet:
     * 
     * + XR30[4] = 0, VCO divider loop uses divide by 4 (same as 65545)
     * 1, VCO divider loop uses divide by 16
     * 
     * + XR30[5] = 1, reference clock is divided by 5
     * 
     * Other parameters available on the 65550 and not on the 65545
     * 
     * + XRCB[2] = 0, VCO divider loop uses divide by 4 (same as 65545)
     * 1, VCO divider loop uses divide by 16
     * 
     * + XRCB[1] = 1, reference clock is divided by 5
     * 
     * + XRCB[7] = Vclk = Mclk
     * 
     * + XRCA[0:1] = 2 MSB of a 10 bit M-Divisor
     * 
     * + XRCA[4:5] = 2 MSB of a 10 bit N-Divisor
     * 
     * I haven't put in any support for those here.  For simplicity,
     * they should be set to 0 on the 65548, and left untouched on
     * earlier chips.  */

    target = Clock * 1000;

    for (PSNx = 0; PSNx <= 1; PSNx++) {
	int low_N, high_N;
	double Fref4PSN;

	PSN = PSNx ? 1 : 4;

	low_N = 3;
	high_N = 127;

	while (Fref / (PSN * low_N) > 2.0e6)
	    low_N++;
	while (Fref / (PSN * high_N) < 150.0e3)
	    high_N--;

	Fref4PSN = Fref * 4 / PSN;
	for (N = low_N; N <= high_N; N++) {
	    double tmp = Fref4PSN / N;

	    for (P = ctisHiQV32 ? 1 : 0; P <= 5; P++) {	
	      /* to force post divisor on Toshiba 720CDT */
		double Fvco_desired = target * (1 << P);
		double M_desired = Fvco_desired / tmp;

		/* Which way will M_desired be rounded?  Do all three just to
		 * be safe.  */
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
		    if (Fvco <= 48.0e6)
			continue;
		    if (Fvco > 220.0e6)
			break;

		    Fout = Fvco / (1 << P);

		    error = (target - Fout) / target;

		    aerror = (error < 0) ? -error : error;
		    if (aerror < abest) {
			abest = aerror;
			bestError = error;
			bestM = M;
			bestN = N;
			bestP = P;
			bestPSN = PSN;
			bestFout = Fout;
		    }
		}
	    }
	}
    }
    vclk[0] = (bestP << (ctisHiQV32 ? 4 : 1)) + (bestPSN == 1);
    vclk[1] = bestM - 2;
    vclk[2] = bestN - 2;
#ifdef DEBUG
    ErrorF("Freq. selected: %.2f MHz, vclk[0]=%X, vclk[1]=%X, vclk[2]=%X\n",
	(float)(Clock / 1000.), vclk[0], vclk[1], vclk[2]);
    ErrorF("Freq. set: %.2f MHz\n", bestFout / 1.0e6);
#endif
}

static Bool
CHIPSProbe()
{
    unsigned char temp;
    int NoClocks, i;
    unsigned  PCIIOBase;

#ifdef DEBUG
    ErrorF("CHIPSProbe\n");
#endif

    xf86ClearIOPortList(vga256InfoRec.scrnIndex);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_CHIPS_ExtPorts, CHIPS_ExtPorts);
    xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_CHIPS_ExtPorts32, CHIPS_ExtPorts32);

    if (vga256InfoRec.chipset) {
	if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_520))) {
	    CHIPSchipset = CT_520;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_530))) {
	    CHIPSchipset = CT_530;
	    ctLinearSupport = TRUE;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_540))) {
	    CHIPSchipset = CT_540;
	    ctLinearSupport = TRUE;
	    ctHDepth = TRUE;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_545))) {
	    CHIPSchipset = CT_545;
	    ctLinearSupport = TRUE;
	    ctAccelSupport = TRUE;
	    ctSupportMMIO = TRUE;
	    ctHDepth = TRUE;
	    ct65545subtype = 2;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_546))) {
	    CHIPSchipset = CT_546;
	    ctLinearSupport = TRUE;
	    ctAccelSupport = TRUE;
	    ctSupportMMIO = TRUE;
	    ctHDepth = TRUE;
	    ct65545subtype = 3;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_548))) {
	    CHIPSchipset = CT_548;
	    ctLinearSupport = TRUE;
	    ctAccelSupport = TRUE;
	    ctSupportMMIO = TRUE;
	    ctHDepth = TRUE;
	    ct65545subtype = 4;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_550))) {
	    CHIPSchipset = CT_550;
	    ctLinearSupport = TRUE;
	    ctAccelSupport = TRUE;
	    ctSupportMMIO = TRUE;
	    ctUseMMIO = TRUE;	     /* MMIO seems to be usuable on all Buses
				      * In fact it seems that Blitting can only
				      * be done with MMIO */
	    ctHDepth = TRUE;
	    ctisHiQV32 = TRUE;	     /* Use the new HiQV32 architecture */
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_554))) {
	    CHIPSchipset = CT_554;
	    ctLinearSupport = TRUE;
	    ctAccelSupport = TRUE;
	    ctSupportMMIO = TRUE;
	    ctUseMMIO = TRUE;	     /* MMIO seems to be usuable on all Buses
				      * In fact it seems that Blitting can only
				      * be done with MMIO */
	    ctHDepth = TRUE;
	    ctisHiQV32 = TRUE;	       /* Use the new HiQV32 architecture */
	}
#ifdef CT45x_SUPPORT
	else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_451))) {
	    CHIPSchipset = CT_451;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_452))) {
	    CHIPSchipset = CT_452;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_453))) {
	    CHIPSchipset = CT_453;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_455))) {
	    CHIPSchipset = CT_455;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_456))) {
	    CHIPSchipset = CT_456;
	} else if (!StrCaseCmp(vga256InfoRec.chipset, CHIPSIdent(CT_457))) {
	    CHIPSchipset = CT_457;
	}
#endif
	else {
	    return (FALSE);
	}
	CHIPSEnterLeave(ENTER);
    } else {
	CHIPSEnterLeave(ENTER);
	temp = rdinx(0x3D6, 0x00);
	/*
	 *  Reading 0x103 causes segmentation violation, like 46E8 ???
	 *  So for now just force what I want!
	 *
	 *  Need to look at ioctl(console_fd, PCCONIOCMAPPORT, &ior)
	 *  for bsdi!
	 */
	CHIPSchipset = 99;
	if (temp != 0xA5) {
	    if ((temp & 0xF0) == 0x70) {
		CHIPSchipset = CT_520;
	    }
	    if ((temp & 0xF0) == 0x80) {
		ctLinearSupport = TRUE;
		CHIPSchipset = CT_530;
	    }
	    if ((temp & 0xF8) == 0xD0) {
		ctLinearSupport = TRUE;
		ctHDepth = TRUE;
		CHIPSchipset = CT_540;
	    }
	    if ((temp & 0xF8) == 0xD8) {	/*CT65545+ */
		ctLinearSupport = TRUE;
		ctAccelSupport = TRUE;
		ctSupportMMIO = TRUE;
		ctHDepth = TRUE;
		ct65545subtype = temp & 0x7;
		switch (ct65545subtype) {
		case 3:
		    CHIPSchipset = CT_546;
		    break;
		case 4:
		    CHIPSchipset = CT_548;
		    break;
		default:
		    CHIPSchipset = CT_545;
		}
	    }
	}
	if (CHIPSchipset != 99) {
	    ErrorF("%s %s: ct65545+: chip revision: %i\n",
		XCONFIG_PROBED, vga256InfoRec.name, temp & 07);
	}

	/* At this point the chip could still be a ct65550, so check for
	 * that. This test needs some looking at */
	if ((temp != 0) && (CHIPSchipset == 99)) {
	    outb(0x3D6, 0x02);
	    temp = inb(0x03D7);
	    if (temp == 0xE0) {
		CHIPSchipset = CT_550;
		ctLinearSupport = TRUE;
		ctAccelSupport = TRUE;
		ctSupportMMIO = TRUE;
		ctUseMMIO = TRUE;    /* MMIO seems to be usuable on all Buses
				      * In fact it seems that Blitting can only
				      * be done with MMIO */
		ctHDepth = TRUE;
		ctisHiQV32 = TRUE;
	    }
	    if (temp == 0xE4) {
		CHIPSchipset = CT_554;
		ctLinearSupport = TRUE;
		ctAccelSupport = TRUE;
		ctSupportMMIO = TRUE;
		ctUseMMIO = TRUE;    /* MMIO seems to be usuable on all Buses.
				      * In fact it seems that Blitting can only
				      * be done with MMIO */
		ctHDepth = TRUE;
		ctisHiQV32 = TRUE;
	    }
	    if (CHIPSchipset != 99) {
		outb(0x3D6, 0x04);
		temp = inb(0x03D7);
		ErrorF("%s %s: ct65550+: chip revision: %i\n",
		    XCONFIG_PROBED, vga256InfoRec.name, temp & 0xFF);
	    }
	}
	if (CHIPSchipset == 99) {      /* failure, if no good, then leave */
	    /*
	     * Turn things back off if the probe is going to fail.
	     * Returning FALSE implies failure, and the server
	     * will go on to the next driver.
	     */
	    CHIPSEnterLeave(LEAVE);
#ifdef DEBUG
	    ErrorF("Bombing out!\n");
#endif
	    return (FALSE);
	}
    }

#ifndef MONOVGA
#ifdef XFreeXDGA
     /* we support direct Video mode */
     vga256InfoRec.directMode = XF86DGADirectPresent;
#endif
#endif

    /* By default the page mapping variables are setup for
     * chips earlier than the 65550. Hence correct these
     */
    if (ctisHiQV32) {
	CHIPS.ChipSetRead = CHIPSHiQVSetRead;
	CHIPS.ChipSetWrite = CHIPSHiQVSetWrite;
	CHIPS.ChipSetReadWrite = CHIPSHiQVSetReadWrite;
	CHIPS.ChipMapSize = 0x10000;
	CHIPS.ChipSegmentSize = 0x10000;
	CHIPS.ChipSegmentShift = 16;
	CHIPS.ChipSegmentMask = 0xFFFF;
	CHIPS.ChipReadBottom = 0x00000;
	CHIPS.ChipReadTop = 0x10000;
	CHIPS.ChipWriteBottom = 0x00000;
	CHIPS.ChipWriteTop = 0x10000;
	CHIPS.ChipUse2Banks = FALSE;
    }

    /* memory size */
    if (ctisHiQV32) {
	if (!vga256InfoRec.videoRam) {
	    /* not given, probe it    */
	    /* XR43: DRAM interface   */
	    /* bit 2-1: memory size   */
	    /*          0: 1024 kB    */
	    /*          1: 2048 kB    */
	    /*          2: 4096 kB    */
	    /*          3: reserved   */
	    outb(0x3D6, 0x43);
	    switch ((inb(0x3D7) & 0x06) >> 1) {
	    case 0:
		vga256InfoRec.videoRam = 1024;
		break;
	    case 1:
		vga256InfoRec.videoRam = 2048;
		break;
	    case 2:
	    case 3:
		vga256InfoRec.videoRam = 4096;
		break;
	    }

	    ErrorF("%s %s: ct65550+: %d kB VRAM\n", XCONFIG_PROBED,
		vga256InfoRec.name, vga256InfoRec.videoRam);
	} else {
	    ErrorF("%s %s: ct65550+: %d kB VRAM\n", XCONFIG_GIVEN,
		vga256InfoRec.name, vga256InfoRec.videoRam);
	}
    } else {
	if (!vga256InfoRec.videoRam) {
	    /* not given, probe it    */
	    /* XR0F: Software flags 0 */
	    /* bit 1-0: memory size   */
	    /*          0: 256 kB     */
	    /*          1: 512 kB     */
	    /*          2: 1024 kB    */
	    /*          3: 1024 kB    */
	    outb(0x3D6, 0x0F);
	    switch (inb(0x3D7) & 3) {
	    case 0:
		vga256InfoRec.videoRam = 256;
		break;
	    case 1:
		vga256InfoRec.videoRam = 512;
		break;
	    case 2:
	    case 3:
		vga256InfoRec.videoRam = 1024;
		break;
	    }
	    ErrorF("%s %s: ct65545+: %d kB VRAM\n", XCONFIG_PROBED,
		vga256InfoRec.name, vga256InfoRec.videoRam);
	} else {
	    ErrorF("%s %s: ct65545+: %d kB VRAM\n", XCONFIG_GIVEN,
		vga256InfoRec.name, vga256InfoRec.videoRam);
	}
    }

    /*test STN / TFT */
    if (ctisHiQV32) {
	outb(0x3D0, 0x10);
	temp = inb(0x3D1);
    } else {
	outb(0x3D6, 0x51);
	temp = inb(0x3D7);
    }
    /* XR51 or FR10: DISPLAY TYPE REGISTER                      */
    /* XR51[1-0] or FR10[1:0] for ct65550 : PanelType,          */
    /* 0 = Single Panel Single Drive, 3 = Dual Panel Dual Drive */
    switch (temp & 0x3) {
    case 0:
	if (OFLG_ISSET(OPTION_STN, &vga256InfoRec.options)) {
	    ctPanelType = SS;
	    ErrorF("%s %s: ct65545+: SS-STN probed.\n",
		XCONFIG_GIVEN, vga256InfoRec.name);

	} else {
	    ctPanelType = TFT;
	    ErrorF("%s %s: ct65545+: TFT probed.\n",
		XCONFIG_PROBED, vga256InfoRec.name);
	}
	break;
    case 2:
	ctPanelType = DS;
	ErrorF("%s %s: ct65545+: DS-STN probed.\n",
	    XCONFIG_PROBED, vga256InfoRec.name);
    case 3:
	ctPanelType = DS;
	ErrorF("%s %s: ct65545+: DD-STN probed.\n",
	    XCONFIG_PROBED, vga256InfoRec.name);
	break;
    default:
	break;
    }

    /* test LCD */
    if (ctisHiQV32) {
	/* FR01: DISPLAY TYPE REGISTER                         */
	/* FR01[1:0]:   Display Type, 01 = CRT, 10 = FlatPanel */
	/* LCD                                                 */
	outb(0x3D0, 0x01);
	temp = inb(0x3D1);
	if ((temp & 0x03) == 0x02) {
	    ctLCD = TRUE;
	    ctCRT = FALSE;
	    ErrorF("%s %s: ct65545+: LCD\n",
		XCONFIG_PROBED, vga256InfoRec.name);
	} else {
	    ctLCD = FALSE;
	    ctCRT = TRUE;
	    ErrorF("%s %s: ct65545+: CRT\n",
		XCONFIG_PROBED, vga256InfoRec.name);
	}
    } else {
	if (temp & 0x4) {
	    /* XR51: DISPLAY TYPE REGISTER                     */
	    /* XR51[2]:   Display Type, 0 = CRT, 1 = FlatPanel */
	    /* LCD                                             */
	    ctLCD = TRUE;
	    ctCRT = FALSE;
	    ErrorF("%s %s: ct65545+: LCD\n",
		XCONFIG_PROBED, vga256InfoRec.name);
	} else {
	    ctLCD = FALSE;
	    ctCRT = TRUE;
	    ErrorF("%s %s: ct65545+: CRT\n",
		XCONFIG_PROBED, vga256InfoRec.name);
	}
    }

    /* screen size */
    /* 
     * In LCD mode / dual mode we want to derive the timing values from
     * the ones preset by bios
     */
    if (ctLCD) {
	if (ctisHiQV32) {
	    /* for 65550 we only need H/VDisplay values for screen size */
	    unsigned char fr25, tmp1;
#ifdef DEBUG
	    unsigned char fr26;
	    char tmp2;
#endif
	    read_fr(0x25, fr25);
	    read_fr(0x20, temp);
	    ctSize.HDisplay = ((temp + ((fr25 & 0x0F) << 8)) + 1) << 3;
	    read_fr(0x30, temp);
	    read_fr(0x35, tmp1);
	    ctSize.VDisplay = ((tmp1 & 0x0F) << 8) + temp + 1;
#ifdef DEBUG
	    read_fr(0x21, temp);
	    ctSize.HRetraceStart = ((temp + ((fr25 & 0xF0) << 4)) + 1) << 3;
	    read_fr(0x22, tmp1);
	    tmp2 = (tmp1 & 0x1F) - (temp & 0x3F);
	    ctSize.HRetraceEnd = ((((tmp2 < 0) ? (tmp2 + 0x40) : tmp2) << 3)
		+ ctSize.HRetraceStart);
	    read_fr(0x23, temp);
	    read_fr(0x26, fr26);
	    ctSize.HTotal = ((temp + ((fr26 & 0x0F) << 8)) + 5) << 3;
	    ErrorF("x=%i, y=%i; xSync=%i, xSyncEnd=%i, xTotal=%i\n",
		ctSize.HDisplay, ctSize.VDisplay, ctSize.HRetraceStart,
		ctSize.HRetraceEnd, ctSize.HTotal);
#endif
	    ErrorF("%s %s: ct65550+: Display Size: x=%i; y=%i\n",
		XCONFIG_PROBED, vga256InfoRec.name,
		ctSize.HDisplay, ctSize.VDisplay);
	} else {
	    unsigned char xr17, tmp1;
	    char tmp2;

	    read_xr(0x17, xr17);
	    read_xr(0x1B, temp);
	    ctSize.HTotal = ((temp + ((xr17 & 0x01) << 8)) + 5) << 3;
	    read_xr(0x1C, temp);
	    ctSize.HDisplay = ((temp + ((xr17 & 0x02) << 7)) + 1) << 3;
	    read_xr(0x19, temp);
	    ctSize.HRetraceStart = ((temp + ((xr17 & 0x04) << 9)) + 1) << 3;
	    read_xr(0x1A, tmp1);
	    tmp2 = (tmp1 & 0x1F) + ((xr17 & 0x08) << 2) - (temp & 0x3F);
	    ctSize.HRetraceEnd = ((((tmp2 < 0) ? (tmp2 + 0x40) : tmp2) << 3)
		+ ctSize.HRetraceStart);
	    read_xr(0x65, tmp1);
	    read_xr(0x68, temp);
	    ctSize.VDisplay = ((tmp1 & 0x02) << 7) 
	      + ((tmp1 & 0x40) << 3) + temp + 1;
#ifdef DEBUG
	    ErrorF("x=%i, y=%i; xSync=%i, xSyncEnd=%i, xTotal=%i\n",
		ctSize.HDisplay, ctSize.VDisplay, ctSize.HRetraceStart,
		ctSize.HRetraceEnd, ctSize.HTotal);
#endif
	    ErrorF("%s %s: ct65545+: Display Size: x=%i; y=%i\n",
		   XCONFIG_PROBED, vga256InfoRec.name,
		   ctSize.HDisplay, ctSize.VDisplay);
	}
	/* Warn the user if the panel size has been overridden by
	 * the modeline values
	 */
	if (OFLG_ISSET(OPTION_PANEL_SIZE, &vga256InfoRec.options)) {
	    ErrorF("%s %s: ct65545+: Display size overridden by modelines.\n",
		   XCONFIG_GIVEN, vga256InfoRec.name);
	}
    }

    /* Frame Buffer */
    if (IS_STN(ctPanelType)) {
	if (ctisHiQV32) {
	    outb(0x3D0, 0x1A);	       /*Frame Buffer Ctrl. */
	    temp = inb(0x3D1);
	} else {
	    outb(0x3D6, 0x6F);	       /*Frame Buffer Ctrl. */
	    temp = inb(0x3D7);
	}
	if (temp & 1) {
	    ErrorF("%s %s: ct65545+: Frame Buffer used.\n",
		XCONFIG_PROBED, vga256InfoRec.name);
	    if (!(OFLG_ISSET(OPTION_EXT_FRAM_BUF, &vga256InfoRec.options))) {
		/* Formula for calculating the size of the framebuffer. The
		 * extra 8k is added just because I seem to need it [DGB].
		 * I'm not sure if this formula holds for the 65550 
		 */
	        ctFrameBufferSize = ctSize.HDisplay * ctSize.VDisplay * 3 /
		    16 + 8192;
	    }
	}
	if (temp & 2)
	    ErrorF("%s %s: ct65545+: Frame Accelerator Enabled.\n",
		XCONFIG_PROBED, vga256InfoRec.name);
    }

    /* bus type */
    if (ctisHiQV32) {
      outb(0x3D6, 0x08);
      temp = inb(0x3D7) & 1;
      if (temp == 1) {
	temp = 6;
      } else {
	temp = 7;
      }
    } else {
      outb(0x3D6, 0x01);
      temp = inb(0x3D7) & 7;
    }
    ErrorF("%s %s: ct65545+: ",XCONFIG_PROBED, vga256InfoRec.name);
	if (temp == 6) {	       /*PCI */
	    ErrorF("PCI Bus\n");
	    ctPCI = TRUE;
	    if ((CHIPSchipset == CT_545) || (CHIPSchipset == CT_546)){
	      ErrorF("%s %s: ct65545+: 32Bit IO not supported on 65545 PCI.\n", 
		     XCONFIG_PROBED, vga256InfoRec.name);
	      ErrorF("%s %s: ct65545+: Enabling MMIO\n", 
		     XCONFIG_PROBED, vga256InfoRec.name);
	      ctSupportMMIO = TRUE;
	      ctUseMMIO = TRUE;
	    }
	    /* Turn on the MMIO addressing for 6554x chips with PCI */
	    if (OFLG_ISSET(OPTION_MMIO, &vga256InfoRec.options) 
		&& ctSupportMMIO)
		ctUseMMIO = TRUE;
	} else {   /* XR08: Linear addressing base, not for PCI */
	    switch (temp) {
	    case 3:
		ErrorF("CPU Direct\n");
		break;
	    case 5:
		ErrorF("ISA Bus?\n");
		break;
	    case 7:
		ErrorF("VL Bus\n");
		break;
	    default:
		ErrorF("Unknown Bus\n");
	    }
	  }

    /* Test whether linear addressing is turned off */
    if (OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options)) {
	ErrorF("%s %s: ct65545+: Disabling Linear Addressing\n",
	    XCONFIG_PROBED, vga256InfoRec.name);
	ctLinearSupport = FALSE;
	ctHDepth = FALSE;
	/* Much of the acceleration code wasn't written in a way that
	 * is usuable without linear addressing. This is a fault of the
	 * code, the chips actually do support acceleration without
	 * linear addressing */
	ctAccelSupport = FALSE;
    }

    /* linear base */
    if (ctLinearSupport) {
    if (vga256InfoRec.MemBase) {
	CHIPS.ChipLinearBase = vga256InfoRec.MemBase;
	ErrorF("%s %s: ct65545+: base address is set at 0x%X.\n",
	    XCONFIG_GIVEN, vga256InfoRec.name, CHIPS.ChipLinearBase);
    } else {
	if(ctPCI){
	    CHIPS.ChipLinearBase = ctPCIMemBase(ctisHiQV32);
	    /* If no valid PCI device was found then disable linear
	     * addressing. This would indicate a faulty PCI device */
	    if (CHIPS.ChipLinearBase == -1) {
		ErrorF("%s %s: ct65545+: Disabling Linear Addressing\n",
		    XCONFIG_PROBED, vga256InfoRec.name);
		ctLinearSupport = FALSE;
		ctHDepth = FALSE;
		ctAccelSupport = FALSE;
	    }
	  } else {
	    if (ctisHiQV32) {
		outb(0x3D6, 0x6);
		CHIPS.ChipLinearBase = ((0xFF00 & inl(0x3D6)) << 16);
		outb(0x3D6, 0x5);
		CHIPS.ChipLinearBase |= ((0x8000 & inl(0x3D6)) << 8);
	    } else {
		outb(0x3D6, 0x8);
		CHIPS.ChipLinearBase = ((0xFF00 & inl(0x3D6)) << 12);
	    }
	}
	ErrorF("%s %s: ct65545+: base address is set at 0x%X.\n",
	    XCONFIG_PROBED, vga256InfoRec.name, CHIPS.ChipLinearBase);
    }
  }

    /* Test whether 16/24 bpp is being used, and bomb out if not
     * using linear addressing */
    if (!ctLinearSupport && (vgaBitsPerPixel == 16 || vgaBitsPerPixel == 24)) {
	ErrorF("%s %s: ct65545+: Depth %d only supported with linear addressing\n",
	    XCONFIG_PROBED, vga256InfoRec.name, vgaBitsPerPixel);
	return (FALSE);
    }

    /*Linear addressing */
    if (xf86LinearVidMem() && ctLinearSupport) {
	CHIPS.ChipUseLinearAddressing = TRUE;
	ErrorF("%s %s: ct65545+: Linear addressing is enabled at 0x%X.\n",
	    XCONFIG_PROBED, vga256InfoRec.name, CHIPS.ChipLinearBase);
	if (ctHDepth) {
	    if (vgaBitsPerPixel == 16) {
		ErrorF("%s %s: ct65545+: 16 bpp.\n", XCONFIG_GIVEN,
		    vga256InfoRec.name);
		CHIPS.ChipHas16bpp = TRUE;
	    } else if (vgaBitsPerPixel > 16) {
		ErrorF("%s %s: ct65545+: 24 bpp.\n", XCONFIG_GIVEN,
		    vga256InfoRec.name);
		CHIPS.ChipHas24bpp = TRUE;
	    }
	}
	/* linear video Ram size */
	if (vga256InfoRec.videoRam) {
	  CHIPS.ChipLinearSize = vga256InfoRec.videoRam * 1024;
	}
    }

    /* DAC info */
    if (ctisHiQV32) {
	outb(0x3D6, 0xD0);
	if (!(inb(0x3D7) & 0x01))
	    ErrorF("%s %s: ct65550+: Internal DAC disabled.\n", XCONFIG_PROBED,
		vga256InfoRec.name);
    } else {
	outb(0x3D6, 0x06);
	if (inb(0x3D7) & 0x02)
	    ErrorF("%s %s: ct65545+: Internal DAC disabled.\n", XCONFIG_PROBED,
		vga256InfoRec.name);
    }

 /* MMIO address offset */
    if(ctUseMMIO){
      if(ctisHiQV32)
	ctMMIO=ctReg32HiQV;
      else
	ctMMIO=ctReg32MMIO;
    }
    /* Clock type */
    /*
     * Again, if the user has specified the clock values in the XF86Config
     * file, we respect those choices.
     */
    switch (CHIPSchipset) {
    case CT_520:
    case CT_530:
	NoClocks = 4;
	ctClockType = OLD_STYLE | TYPE_HW;
	break;
    default:
	if ((OFLG_ISSET(OPTION_HW_CLKS, &vga256InfoRec.options) 
	     && !ctisHiQV32)) {
	    NoClocks = 5;
	    ctClockType = NEW_STYLE | TYPE_HW;
	} else {
	    NoClocks = 26; /* some number */
	    ctClockType = TYPE_PROGRAMMABLE;
	    OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);
	}
    }

    if (ctClockType == TYPE_PROGRAMMABLE) {
      if(!vga256InfoRec.clockprog)
	vga256InfoRec.clocks = 0;
      if(vga256InfoRec.textClockFreq > 0) {
	ctCurrentClock = vga256InfoRec.textClockFreq;
	ErrorF("%s %s: ct65545+: using textclock freq: %7.3f.\n",
	       XCONFIG_GIVEN, vga256InfoRec.name,ctCurrentClock/1000.0);
      } else
	ctCurrentClock = ctisHiQV32 ? 0
	  : (ctLCD ? LCD_TEXT_CLK_FREQ : CRT_TEXT_CLK_FREQ);
      ErrorF("%s %s: ct65545+: using programmable clocks.\n",
	     XCONFIG_PROBED, vga256InfoRec.name);
    } else {  /* TYPE_PROGRAMMABLE */
      ctCurrentClock = ctGetHWClock(ctClockType);
      ErrorF("%s %s: ct65545+: Textmode Clock: %i.\n",
	       XCONFIG_PROBED, vga256InfoRec.name, ctCurrentClock);
      if (!vga256InfoRec.clocks) {
	vga256InfoRec.clocks = NoClocks;
	vgaGetClocks(NoClocks, CHIPSClockSelect);
      } else { 
	if (vga256InfoRec.clocks > NoClocks) {
	  ErrorF("%s %s: %s: Too many Clocks specified in configuration file.\n",
		 XCONFIG_PROBED, vga256InfoRec.name,
		 vga256InfoRec.chipset);
	  ErrorF("\t\tAt most %d clocks may be specified\n",
		 NoClocks);
	vga256InfoRec.clocks = NoClocks;
	}
      }
    }
      CHIPS.ChipClockScaleFactor = (ctisHiQV32 ? 1 : vgaBytesPerPixel);

    /* maximal clock */
    switch (CHIPSchipset) {
    case CT_550:
    case CT_554:
	outb(0x3D0, 0x0A);
	if (inb(0x3D1) & 2) {
	    /*5V Vcc */
	    vga256InfoRec.maxClock = 110000;
	} else {
	    /*3.3V Vcc */
	    vga256InfoRec.maxClock = 80000;
	}
	break;
    case CT_546:
    case CT_548:
	vga256InfoRec.maxClock = 80000;
	/* max VCLK is 80 MHz, max MCLK is 75 MHz for CT65548 */
	/* It is not sure for CT65546, but it works with 60 nsec EDODRAM */
	break;
    default:
	outb(0x3D6, 0x6C);
	if (inb(0x3D7) & 2) {
	    /*5V Vcc */
	    vga256InfoRec.maxClock = 68000;
	} else {
	    /*3.3V Vcc */
	    vga256InfoRec.maxClock = 56000;
	}
    }

    vga256InfoRec.chipset = CHIPSIdent(CHIPSchipset);
    vga256InfoRec.bankedMono = FALSE;
    /* allowed options */
    OFLG_SET(OPTION_LINEAR, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_NOACCEL, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_HW_CLKS, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_NOLINEAR_MODE, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_SW_CURSOR, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_STN, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_EXT_FRAM_BUF, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_USE_MODELINE, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_NO_BITBLT, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_LCD_STRETCH, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_LCD_CENTER, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_MMIO, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_SUSPEND_HACK, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_SYNC_ON_GREEN, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_PANEL_SIZE, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_18_BIT_BUS, &CHIPS.ChipOptionFlags);
    OFLG_SET(OPTION_NO_IMAGEBLT, &CHIPS.ChipOptionFlags);

    return (TRUE);
}

extern Bool ctHWcursorShown;
static unsigned int ctHWcursorContents;

static void
CHIPSEnterLeave(enter)
    Bool enter;
{
    static unsigned char xr15 = 0xFF;
    unsigned char temp;

#ifdef DEBUG
    ErrorF("CHIPSEnterLeave(");
    if (enter)
	ErrorF("Enter)\n");
    else
	ErrorF("Leave)\n");
#endif

#ifndef MONOVGA
#ifdef XFreeXDGA
    if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter) {
	/* 
	 * Disable HW cursor. I hope DGA can't call EnterLeave(leave) twice
	 * in a row, and that another EnterLeave(leave) can't be called
	 * before DGA finishes with an EnterLeave(enter). Otherwise the
	 * effect will be to hide the cursor, perhaps permanently!!
	 */
	if (ctHWcursorShown) {
	    if (ctisHiQV32) {
		outb(0x3D6, 0xA0);
		ctHWcursorContents = inb(0x3D7);
		outb(0x3D7, ctHWcursorContents & 0xF8);
	    } else {
		if(!ctUseMMIO){
		    HW_DEBUG(0x8);
		    ctHWcursorContents = inl(DR(0x8));
		    outw(DR(0x8), ctHWcursorContents & 0xFFFE);
		} else {
		    HW_DEBUG(ctMMIO[0x8]);
		    ctHWcursorContents = MMIOmeml(ctMMIO[0x8]);
		    MMIOmemw(ctMMIO[0x8]) = ctHWcursorContents & 0xFFFE;
		}
	    }
	}
	return;
    }
#endif
#endif
    if (enter) {
      /* enable IO ports */
	xf86EnableIOPorts(vga256InfoRec.scrnIndex);

      /* handle base for certain IO regs. */
	if (ctVgaIOBaseFlag == 0xFF){ /*save: suspend/resume might mess it up*/
	  ctVgaIOBaseFlag = (inb(0x3CC) & 0x01);
	  vgaIOBase = ctVgaIOBaseFlag ? 0x3D0 : 0x3B0;
	  ctCRindex = vgaIOBase + 4;
	  ctCRvalue = vgaIOBase + 5;
	  ctST01reg = vgaIOBase + 0x0A;
#ifdef DEBUG
	  if (ctVgaIOBaseFlag)
	    ErrorF("color\n");
	  else
	    ErrorF("monochrome\n");
#endif
	} else {
	  /* hack: if not 1st time fix it up as it might be messed up by s/r */
	  temp = inb(0x3CC);
	  outb(0x3C2, (temp & 0xFE) | ctVgaIOBaseFlag); 
	}

	/* Unprotect CRTC[0-7] */
	outb(ctCRindex, 0x11);
	temp = inb(ctCRvalue);
	outb(ctCRvalue, temp & 0x7F);

	/* group protection */
	if (!ctisHiQV32) {
	    outb(0x3D6, 0x15);
	    if (xr15 == 0xFF)
		xr15 = inb(0x3D7);
	    outb(0x3D7, 00);
	}

	/* enable HW cursor */
	if (ctHWcursorShown) {
	    if (ctisHiQV32) {
		outb(0x3D6, 0xA0);
		outb(0x3D7, ctHWcursorContents & 0xFF);
	    } else {
	      if(!ctUseMMIO){
		HW_DEBUG(0x8);
		outl(DR(0x8), ctHWcursorContents);
	      } else {
		HW_DEBUG(ctMMIO[0x8]);
		MMIOmeml(ctMMIO[0x8]) = ctHWcursorContents;
	      }
	    }
	}
    } else {
	/*
	 * Here undo what was done above.
	 */

      /* fix things that could be messed up by suspend/resume */
      temp = inb(0x3CC);
      outb(0x3C2, (temp & 0xFE) | ctVgaIOBaseFlag); 

      /* disable HW cursor */
	if (ctHWcursorShown) {
	    if (ctisHiQV32) {
		outb(0x3D6, 0xA0);
		ctHWcursorContents = inb(0x3D7);
		outb(0x3D7, ctHWcursorContents & 0xF8);
	    } else {
	      if(!ctUseMMIO){
		HW_DEBUG(0x8);
		ctHWcursorContents = inl(DR(0x8));
		outw(DR(0x8), ctHWcursorContents & 0xFFFE);
	      } else {
		HW_DEBUG(ctMMIO[0x8]);
		ctHWcursorContents = MMIOmeml(ctMMIO[0x8]);
		MMIOmemw(ctMMIO[0x8]) = ctHWcursorContents & 0xFFFE;
	      }
	    }
	}

	/* Protect CRTC[0-7] */
	outb(ctCRindex, 0x11);
	temp = inb(ctCRvalue);
	outb(ctCRvalue, (temp & 0x7F) | 0x80);

	/* group protection */
	if (!ctisHiQV32) {
	    outb(0x3D6, 0x15);
	    outb(0x3D7, xr15);
	}

      /* disable IO ports */
#ifdef IO_DEBUG
      ErrorF("E/L: 0x3CC: %X\n", (unsigned char)inb(0x3CC));
#endif
      xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}

static void
CHIPSRestore(restore)
    vgaCHIPSPtr restore;
{
    int i;
    unsigned char tmp;

#ifdef DEBUG
    ErrorF("CHIPSRestore\n");
#endif

    /* set registers so that we can program the controller */
    if (ctisHiQV32) {
	outw(0x3D6, 0x0E);
    } else {
	outw(0x3D6, 0x10);
	outw(0x3D6, 0x11);
	outw(0x3D6, 0x15);	       /* unprotect all registers */
    }
    /* fix things that could be messed up by suspend/resume */
    outb(0x3C2, (((((vgaHWPtr) restore)->MiscOutReg) & 0xFE) 
		 | ctVgaIOBaseFlag));
    outb(ctCRindex, 0x11);
    tmp = inb(ctCRvalue);
    outb(ctCRvalue, (tmp & 0x7F)); /*group 0 protection off */

    /* wait for vsync if sequencer is running - stop sequencer */
	while (((inb(ctST01reg)) & 0x08) == 0x08){};/* wait VSync off */
        while (((inb(ctST01reg)) & 0x08) == 0 ) {}; /* wait VSync on */
    outw(0x3C4,0x07);              /* reset hsync - just in case...*/

    /* set the clock */
#ifdef IO_DEBUG
    ErrorF("1: 0x3CC: %X ", (unsigned char)inb(0x3CC));
#endif
    if (restore->std.NoClock >= 0)
      ctClockLoad(ctClockType, &restore->ctClock);
#ifdef IO_DEBUG
    ErrorF("-> %X\n", (unsigned char)inb(0x3CC));
#endif

    /* set extended regs */
    ctRestore(restore);

    /* set CRTC registers - do it before sequencer restarts */
    for (i=0; i<25; i++) 
      outw(ctCRindex,(restore->std.CRTC[i] << 8) | i);

    /* set stretching registers */
    if (ctisHiQV32) {
      ctRestoreStretching(restore->Port_3D0[0x40],restore->Port_3D0[0x48]);
      /* why twice ? :
       * sometimes the console is not well restored even if these registers 
       * are good, re-write the registers works around it
       */
      ctRestoreStretching(restore->Port_3D0[0x40],restore->Port_3D0[0x48]);
    }
    else {
      ctRestoreStretching(restore->Port_3D6[0x55],restore->Port_3D6[0x57]);
    }

    /* set generic registers */
    if (!ctisHiQV32&&(vga256InfoRec.textclock>=0)&&(vga256InfoRec.textclock<=MAXCLOCKS)) 
      ctScaleClock(ENTER, &vga256InfoRec.textclock);
    vgaHWRestore((vgaHWPtr) restore);
    if (!ctisHiQV32&&(vga256InfoRec.textclock>=0)&&(vga256InfoRec.textclock<=MAXCLOCKS)) 
      ctScaleClock(LEAVE, &vga256InfoRec.textclock);

    /* Flag valid start address, if using CRT extensions */
    if (ctisHiQV32 && (restore->Port_3D6[0x09] & 0x1) == 0x1) {
      outb(ctCRindex, 0x40);
      tmp = inb(ctCRvalue);
      outb(ctCRvalue, tmp | 0x80);
    }

#ifdef IO_DEBUG
    ErrorF("3: 0x3CC: %X", (unsigned char)inb(0x3CC));
#endif
    /* fix things that could be messed up by suspend/resume. 
     * Do it again here, as Nozomi seems to need it          */
    outb(0x3C2, (((((vgaHWPtr) restore)->MiscOutReg) & 0xFE) 
		 | ctVgaIOBaseFlag));
    outb(ctCRindex, 0x11);
    tmp = inb(ctCRvalue);
    outb(ctCRvalue, (tmp & 0x7F));

#ifdef IO_DEBUG
    ErrorF("-> %X\n", (unsigned char)inb(0x3CC));
#endif

 /* set mode */
    ctXMode = restore->XMode;

#ifdef DEBUG
    /* debug - dump out all the extended registers... */
    if (ctisHiQV32) {
	for (i = 0; i < 0xFF; i++) {
	    outb(0x3D6, i);
	    ErrorF("XR%X - %X : %X\n", i, restore->Port_3D6[i], inb(0x3D7));
	}
	for (i = 0; i < 0x80; i++) {
	    outb(0x3D0, i);
	    ErrorF("FR%X - %X : %X\n", i, restore->Port_3D0[i], inb(0x3D1));
	}
    } else {
	for (i = 0; i < 0x80; i++) {
	    outb(0x3D6, i);
	    ErrorF("XR%X - %X : %X\n", i, restore->Port_3D6[i], inb(0x3D7));
	}
    }
#endif
}

/*
 * CHIPSSave --
 *
 * This function saves the video state.  It reads all of the SVGA registers
 * into the vgaCHIPSRec data structure.  There is in general no need to
 * mask out bits here - just read the registers.
 */

static void *
CHIPSSave(save)
    vgaCHIPSPtr save;
{
    int i;
    unsigned char tmp;

#ifdef DEBUG
    ErrorF("CHIPSSave\n");
#endif

    /* set registers that we can program the controller */
    /* bank 0 */
    if (ctisHiQV32) {
	outw(0x3D6, 0x0E);
    } else {
	outw(0x3D6, 0x10);
	outw(0x3D6, 0x11);
	outw(0x3D6, 0x15); /* could be messed up by suspend/resume */
    }
    /* fix things that could be messed up by suspend/resume */
    tmp = inb(0x3CC);
    outb(0x3C2, (tmp & 0xFE) | ctVgaIOBaseFlag); 
    outb(ctCRindex, 0x11);
    tmp = inb(ctCRvalue);
    outb(ctCRvalue, (tmp & 0x7F));

    /* get generic registers */
    save = (vgaCHIPSPtr) vgaHWSave((vgaHWPtr) save, sizeof(vgaCHIPSRec));

    /* save clock */
    ctClockSave(&save->ctClock);

    /* save extended registers */
    if (ctisHiQV32) {
	for (i = 0; i < 0xFF; i++) {
	    outb(0x3D6, i);
	    save->Port_3D6[i] = inb(0x3D7);
#ifdef DEBUG
	    ErrorF("XS%X - %X\n", i, save->Port_3D6[i]);
#endif
	}
	for (i = 0; i < 0x80; i++) {
	    outb(0x3D0, i);
	    save->Port_3D0[i] = inb(0x3D1);
#ifdef DEBUG
	    ErrorF("FS%X - %X\n", i, save->Port_3D0[i]);
#endif
	}
	/* Save CR0-CR40 even though we don't use them, so they can be 
	 *  printed */
	for (i = 0x0; i < 0x80; i++) {
	    outb(ctCRindex, i);
	    save->Port_3D4[i] = inb(ctCRvalue);
#ifdef DEBUG
	    ErrorF("CS%X - %X\n", i, save->Port_3D4[i]);
#endif
	}
/*
 * Save the contents of the BitBLT registers for the 65550, if needed
 */
	if (ctUseMMIO) {
	    for (i = 0x0; i < 0x9; i++) {
		save->BltReg[i] = *(volatile unsigned int *)
		    (ctMMIOBase + ctMMIO[i]);
	    }
	}
    } else {
	for (i = 0; i < 0x80; i++) {
	    outb(0x3D6, i);
	    save->Port_3D6[i] = inb(0x3D7);
#ifdef DEBUG
	    ErrorF("XS%X - %X\n", i, save->Port_3D6[i]);
#endif
	}
	if(!ctUseMMIO){ 
	  for (i=0;i<0xD;i++){
	    HW_DEBUG(i); save->BltReg[i] = inl(DR(i));
	  };
	} else {
	  for (i=0;i<0xD;i++){
	    HW_DEBUG(ctMMIO[i]); save->BltReg[i] = MMIOmeml(ctMMIO[i]);
	  };
	}
    }

    /* get mode */
    save->XMode = ctXMode;
    return ((void *)save);
}

/* 
 * Divide the CHIPSInit function into two, because the HiQV32 architecture
 * is so different from previous ct655xx chips. Note that much code is
 * repeated in these two version though
 */

static Bool
CHIPSInit(mode)
    DisplayModePtr mode;
{
    if (ctisHiQV32)
	return CHIPSInitHiQV32(mode);
    else
	return CHIPSInit655xx(mode);
}

static Bool
CHIPSInit655xx(mode)
    DisplayModePtr mode;
{
    unsigned char tmp;
    int i, k, temp;
    static int HSyncStart, HDisplay;
    int lcdHTotal, lcdHDisplay;
    int lcdVTotal, lcdVDisplay;
    int lcdHBlankStart, lcdHBlankEnd, lcdHRetraceStart, lcdHRetraceEnd;
    int lcdVRetraceStart, lcdVRetraceEnd;
    int CrtcHDisplay;

#ifdef DEBUG
    ErrorF("CHIPSInit655xx\n");
#endif

    /*
     * Possibly fix up the panel size, if the manufacture is stupid
     * enough to set it incorrectly in text modes
     */
    if (OFLG_ISSET(OPTION_PANEL_SIZE, &vga256InfoRec.options)) {
	ctSize.HDisplay = mode->CrtcHDisplay;
	ctSize.VDisplay = mode->CrtcVDisplay;
    }

    /* correct the timings for 16/24 bpp */
    if (vgaBitsPerPixel == 16) {
	if (!mode->CrtcHAdjusted) {
	    mode->CrtcHDisplay++;
	    mode->CrtcHDisplay <<= 1;
	    mode->CrtcHDisplay--;
	    mode->CrtcHSyncStart <<= 1;
	    mode->CrtcHSyncEnd <<= 1;
	    mode->CrtcHTotal <<= 1;
	    mode->CrtcHAdjusted = TRUE;
	}
    } else if (vgaBitsPerPixel == 24) {
	if (!mode->CrtcHAdjusted) {
	    mode->CrtcHDisplay++;
	    mode->CrtcHDisplay += ((mode->CrtcHDisplay) << 1);
	    mode->CrtcHDisplay--;
	    mode->CrtcHSyncStart += ((mode->CrtcHSyncStart) << 1);
	    mode->CrtcHSyncEnd += ((mode->CrtcHSyncEnd) << 1);
	    mode->CrtcHTotal += ((mode->CrtcHTotal) << 1);
	    mode->CrtcHAdjusted = TRUE;
	}
    }

    /* store orig. HSyncStart needed for flat panel mode */
    HSyncStart = mode->CrtcHSyncStart / vgaBytesPerPixel - 16;
    HDisplay = (mode->CrtcHDisplay + 1) / vgaBytesPerPixel;
    
    /* fix things that could be messed up by suspend/resume */
    outb(0x3D6,0x15);
    tmp = inb(0x3CC);
    outb(0x3C2, (tmp & 0xFE) | ctVgaIOBaseFlag); 
    outb(ctCRindex, 0x11);
    tmp = inb(ctCRvalue);
    outb(ctCRvalue, (tmp & 0x7F));

    /* generic init */
    ctScaleClock(ENTER, &mode->Clock);
    if (!vgaHWInit(mode, sizeof(vgaCHIPSRec))) {
    ctScaleClock(LEAVE, &mode->Clock);
      ErrorF("bomb 1\n");
      return (FALSE);
    }
    ctScaleClock(LEAVE, &mode->Clock);
    
    /* init clock */
    if (!ctClockFind(ctClockType, new->std.NoClock, &new->ctClock)) {
	ErrorF("bomb 1\n");
	return (FALSE);
    }

    /* some generic settings */
    new->std.Attribute[0x10] = 0x01;   /* mode */
    new->std.Attribute[0x11] = 0x00;   /* overscan (border) color */
    new->std.Attribute[0x12] = 0x0F;   /* enable all color planes */
    new->std.Attribute[0x13] = 0x00;   /* horiz pixel panning 0 */

    new->std.Graphics[0x05] = 0x00;    /* normal read/write mode */

    /* set virtual screen width */
    new->std.CRTC[0x13] = vga256InfoRec.displayWidth >> 3;
    if (vgaBitsPerPixel == 16) {
	new->std.CRTC[0x13] <<= 1;     /* double the width of the buffer */
    } else if (vgaBitsPerPixel == 24) {
	new->std.CRTC[0x13] += new->std.CRTC[0x13] << 1;
    }

    /* get  C&T Specific Registers */
    for (i = 0; i < 0x80; i++) {
	outb(0x3D6, i);
	new->Port_3D6[i] = inb(0x3D7);
    }

    /* set C&T Specific Registers */
    /* set virtual screen width */
    new->Port_3D6[0x1E] = new->std.CRTC[0x13];	/* alternate offset */
    /*databook is not clear about 0x1E might be needed for 65520/30 */

    new->Port_3D6[0x04] |= 4;	       /* enable addr counter bits 16-17 */
    /* XR04: Memory control 1 */
    /* bit 2: Memory Wraparound */
    /*        Enable CRTC addr counter bits 16-17 if set */

    new->Port_3D6[0x0B] = 0x07;	       /* extended mode, dual pages enabled */
    /* XR0B: CPU paging */
    /* bit 0: Memory mapping mode */
    /*        VGA compatible if 0 (default) */
    /*        Extended mode (mapping for > 256 kB mem) if 1 */
    /* bit 1: CPU single/dual mapping */
    /*        0, CPU uses only a single map to access (default) */
    /*        1, CPU uses two maps to access */
    /* bit 2: CPU address divide by 4 */

    new->Port_3D6[0x10] = 0;	       /* XR10: Single/low map */
    new->Port_3D6[0x11] = 0;	       /* XR11: High map      */
    new->Port_3D6[0x28] |= 0x10;       /* 256-color video     */

    /* set up extended display timings */
    if (ctCRT) {
      /* in CRTonly mode this is simple: only set overflow for CR00-CR06 */
	new->Port_3D6[0x17] = ((((mode->CrtcHTotal >> 3) - 5) & 0x100) >> 8)
	    | ((((mode->CrtcHDisplay >> 3) - 1) & 0x100) >> 7)
	    | ((((mode->CrtcHSyncStart >> 3) - 1) & 0x100) >> 6)
	    | ((((mode->CrtcHSyncEnd >> 3)) & 0x20) >> 2)
	    | ((((mode->CrtcHSyncStart >> 3) - 1) & 0x100) >> 4)
	    | (((mode->CrtcHSyncEnd >> 3) & 0x40) >> 1);
    } else {
      /* horizontal timing registers */
	/* in LCD/dual mode use saved bios values to derive timing values if
	 * not told otherwise */
	if (!OFLG_ISSET(OPTION_USE_MODELINE, &vga256InfoRec.options)) {
	    lcdHTotal = ctSize.HTotal;
	    lcdHRetraceStart = ctSize.HRetraceStart;
	    lcdHRetraceEnd = ctSize.HRetraceEnd;
	    if (vgaBitsPerPixel == 16) {
		lcdHRetraceStart <<= 1;
		lcdHRetraceEnd <<= 1;
		lcdHTotal <<= 1;
	    } else if (vgaBitsPerPixel == 24) {
		lcdHRetraceStart += (lcdHRetraceStart << 1);
		lcdHRetraceEnd += (lcdHRetraceEnd << 1);
		lcdHTotal += (lcdHTotal << 1);
	    }
 	    lcdHRetraceStart -=8;       /* HBlank =  HRetrace - 1: for */
 	    lcdHRetraceEnd   -=8;       /* compatibility with vgaHW.c  */
	} else {
	    /* use modeline values if bios values don't work */
	    lcdHTotal = mode->CrtcHTotal;
	    lcdHRetraceStart = mode->CrtcHSyncStart;
	    lcdHRetraceEnd = mode->CrtcHSyncEnd;
	}
	/* The chip takes the size of the visible display area from the
	 * CRTC values. We use bios screensize for LCD in LCD/dual mode
	 * wether or not we use modeline for LCD. This way we can specify
	 * always specify a smaller than default display size on LCD
	 * by writing it to the CRTC registers. */
	lcdHDisplay = ctSize.HDisplay;
	if (vgaBitsPerPixel == 16) {
	    lcdHDisplay++;
	    lcdHDisplay <<= 1;
	    lcdHDisplay--;
	} else if (vgaBitsPerPixel == 24) {
	    lcdHDisplay++;
	    lcdHDisplay += (lcdHDisplay << 1);
	    lcdHDisplay--;
	}
	lcdHTotal = (lcdHTotal >> 3) - 5;
	lcdHDisplay = (lcdHDisplay >> 3) - 1;
	lcdHRetraceStart = (lcdHRetraceStart >> 3);
	lcdHRetraceEnd = (lcdHRetraceEnd >> 3);
	/* This ugly hack is needed because CR01 and XR1C share the 8th bit!*/
	CrtcHDisplay = ((mode->CrtcHDisplay >> 3) - 1);
	if((lcdHDisplay & 0x100) != ( CrtcHDisplay & 0x100)){
	  ErrorF("This display configuration might cause problems !\n");
	  lcdHDisplay = 255;}

	/* now init register values */
	new->Port_3D6[0x17] = (((lcdHTotal) & 0x100) >> 8)
	    | ((lcdHDisplay & 0x100) >> 7)
	    | ((lcdHRetraceStart & 0x100) >> 6)
	    | (((lcdHRetraceEnd) & 0x20) >> 2);

	new->Port_3D6[0x19] = lcdHRetraceStart & 0xFF;
	new->Port_3D6[0x1A] = lcdHRetraceEnd & 0x1F;

	/* XR1B: Alternate horizontal total */
	/* used in all flat panel mode with horiz. compression disabled, */
	/* CRT CGA text and graphic modes and Hercules graphics mode */
	/* similar to CR00, actual value - 5 */
	new->Port_3D6[0x1B] = lcdHTotal & 0xFF;

	/*XR1C: Alternate horizontal blank start (CRT mode) */
	/*      /horizontal panel size (FP mode) */
	/* FP horizontal panel size (FP mode), */
	/* actual value - 1 (in characters unit) */
	/* CRT horizontal blank start (CRT mode) */
	/* similar to CR02, actual value - 1 */
	new->Port_3D6[0x1C] = lcdHDisplay & 0xFF;

	if (OFLG_ISSET(OPTION_USE_MODELINE, &vga256InfoRec.options)) {
	    /* for ext. packed pixel mode on 64520/64530 */
	    /* no need to rescale: used only in 65530    */
	    new->Port_3D6[0x21] = lcdHRetraceStart & 0xFF;
	    new->Port_3D6[0x22] = lcdHRetraceEnd & 0x1F;
	    new->Port_3D6[0x23] = lcdHTotal & 0xFF;

	    /* vertical timing registers */
	    lcdVTotal = mode->CrtcVTotal - 2;
	    lcdVDisplay = ctSize.VDisplay - 1;
	    lcdVRetraceStart = mode->CrtcVSyncStart;
	    lcdVRetraceEnd = mode->CrtcVSyncEnd;

	    new->Port_3D6[0x64] = lcdVTotal & 0xFF;
	    new->Port_3D6[0x66] = lcdVRetraceStart & 0xFF;
	    new->Port_3D6[0x67] = lcdVRetraceEnd & 0x0F;
	    new->Port_3D6[0x68] = lcdVDisplay & 0xFF;
	    new->Port_3D6[0x65] = ((lcdVTotal & 0x100) >> 8)
		| ((lcdVDisplay & 0x100) >> 7)
		| ((lcdVRetraceStart & 0x100) >> 6)
		| ((lcdVRetraceStart & 0x400) >> 7)
		| ((lcdVTotal & 0x400) >> 6)
		| ((lcdVTotal & 0x200) >> 4)
		| ((lcdVDisplay & 0x200) >> 3)
		| ((lcdVRetraceStart & 0x200) >> 2);

	    /* 
	     * These are important: 0x2C specifies the numbers of lines 
	     * (hsync pulses) between vertical blank start and vertical 
	     * line total, 0x2D specifies the number of clock ticks? to
	     * horiz. blank start ( caution ! 16bpp/24bpp modes: that's
	     * why we need HSyncStart - can't use mode->CrtcHSyncStart) 
	     */
	    new->Port_3D6[0x2C] = lcdVTotal - lcdVRetraceStart;
	    new->Port_3D6[0x2D] = (HSyncStart >> 3) & 0xFF;
	    new->Port_3D6[0x2F] = (new->Port_3D6[0x2F] & 0x1F)
		| (((HSyncStart >> 3) & 0x100) >> 3);
	}
	new->Port_3D6[0x55] &= 0xC0;   /* Mask off Polarity bits          */
	new->Port_3D6[0x55] |= 0x01;   /* enable horizontal-compensation  */

    /* set stretching/centering */	
    if(!OFLG_ISSET(OPTION_SUSPEND_HACK, &vga256InfoRec.options)) {
	new->Port_3D6[0x51] |= 0x40;   /* enable FP compensation          */
	new->Port_3D6[0x55] |= 0x01;   /* enable horiz. compensation      */
	new->Port_3D6[0x57] |= 0x01;   /* enable horiz. compensation      */
	if (OFLG_ISSET(OPTION_LCD_CENTER, &vga256InfoRec.options)){
	    if (mode->CrtcHDisplay < 1489)      /* HWBug                  */ 
		new->Port_3D6[0x55] |= 0x02;	/* enable h-centering     */
	    else if (vgaBitsPerPixel == 24)
		new->Port_3D6[0x56] = (lcdHDisplay - CrtcHDisplay) >> 1;
	}
	new->Port_3D6[0x57] = 0x03;    /* enable v-comp disable v-stretch */
	if (!OFLG_ISSET(OPTION_LCD_STRETCH, &vga256InfoRec.options)) {
	    new->Port_3D6[0x55] |= 0x20;   /* enable h-comp disable h-double */
	    new->Port_3D6[0x57] |= 0x60;   /* Enable vertical stretching     */
	    temp = (mode->CrtcVDisplay / (ctSize.VDisplay -
		    mode->CrtcVDisplay + 1));
	    if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) 
	        temp = (temp == 0 ? 1 : temp);  /* HWCursorBug when doubling */
	    new->Port_3D6[0x5A] = temp > 0x0F ? 0 : (unsigned char)temp;
	}
      }
    }

    /* set video mode */
    new->Port_3D6[0x2B] = ctVideoMode(vgaBitsPerPixel, xf86weight.green,
	ctLCD ? min(HDisplay, ctSize.HDisplay) : HDisplay);

#ifdef DEBUG
    ErrorF("VESA Mode: %Xh\n", new->Port_3D6[0x2B]);
#endif

    /* set some linear specific registers */
    if (ctLinearSupport) {
	/* enable linear addressing  */
	new->Port_3D6[0x0B] &= 0xFD;   /* dual page clear                */
	new->Port_3D6[0x0B] |= 0x15;   /* linear mode on                 */

	/* Initialise the HW cursor position */
	new->BltReg[0x0C] = vgaBytesPerPixel
	    * (unsigned long)vga256InfoRec.displayWidth;

	/* general setup */
	new->Port_3D6[0x03] |= 0x03;   /* 32 bit I/O enable etc.          */
	new->Port_3D6[0x07] = 0xF4;    /* 32 bit I/O port selection       */
	new->Port_3D6[0x03] |= 0x08;   /* High bandwidth                  */
	new->Port_3D6[0x40] = 0x01;    /*BitBLT Draw Mode for 8 and 24 bpp */

	/*
	 * Setup the address to write monchrome source data to, for
	 * system to the screen colour expansion.
	 */
	ctBltDataWindow = vgaLinearBase;
    }

    /* common general setup */
    new->Port_3D6[0x52] |= 0x01;       /* Refresh count                   */
    new->Port_3D6[0x0F] &= 0xEF;       /* not Hi-/True-Colour             */
    new->Port_3D6[0x02] |= 0x01;       /* 16bit CPU Memory Access         */
    new->Port_3D6[0x06] &= 0xF3;       /* bpp clear                       */

    /* PCI */
    if (ctPCI)
	new->Port_3D6[0x03] |= 0x40;   /*PCI burst */
    
    /* sync. polarities */
    if ((mode->Flags & (V_PHSYNC | V_NHSYNC))
	&& (mode->Flags & (V_PVSYNC | V_NVSYNC))) {
	if (mode->Flags & (V_PHSYNC | V_NHSYNC)) {
	    if (mode->Flags & V_PHSYNC) {
		new->Port_3D6[0x54] &= 0xBF;	/* FP Hsync positive  */
		new->Port_3D6[0x55] &= 0xBF;	/* CRT Hsync positive */
	    } else {
		new->Port_3D6[0x54] |= 0x40;	/* FP Hsync negative  */
		new->Port_3D6[0x55] |= 0x40;	/* CRT Hsync negative */
	    }
	}
	if (mode->Flags & (V_PVSYNC | V_NVSYNC)) {
	    if (mode->Flags & V_PVSYNC) {
		new->Port_3D6[0x54] &= 0x7F;	/* FP Vsync positive  */
		new->Port_3D6[0x55] &= 0x7F;	/* CRT Vsync positive */
	    } else {
		new->Port_3D6[0x54] |= 0x80;	/* FP Vsync negative  */
		new->Port_3D6[0x55] |= 0x80;	/* CRT Vsync negative */
	    }
	}
    }

    /* bpp depend */
	/*XR06: Palette control */
	/* bit 0: Pixel Data Pin Diag, 0 for flat panel pix. data (def)  */
	/* bit 1: Internal DAC disable                                   */
	/* bit 3-2: Colour depth, 0 for 4 or 8 bpp, 1 for 16(5-5-5) bpp, */
	/*          2 for 24 bpp, 3 for 16(5-6-5)bpp                     */
	/* bit 4:   Enable PC Video Overlay on colour key                */
	/* bit 5:   Bypass Internal VGA palette                          */
	/* bit 7-6: Colour reduction select, 0 for NTSC (default),       */
	/*          1 for Equivalent weighting, 2 for green only,        */
	/*          3 for Colour w/o reduction                           */
	/* XR50 Panel Format Register 1                                  */
	/* bit 1-0: Frame Rate Control; 00, No FRC;                      */
	/*          01, 16-frame FRC for colour STN and monochrome       */
	/*          10, 2-frame FRC for colour TFT or monochrome;        */
	/*          11, reserved                                         */
	/* bit 3-2: Dither Enable                                        */
	/*          00, disable dithering; 01, enable dithering          */
	/*          for 256 mode                                         */
	/*          10, enable dithering for all modes; 11, reserved     */
	/* bit6-4: Clock Divide (CD)                                     */
	/*          000, Shift Clock Freq = Dot Clock Freq;              */
	/*          001, SClk = DClk/2; 010 SClk = DClk/4;               */
	/*          011, SClk = DClk/8; 100 SClk = DClk/16;              */
	/* bit 7: TFT data width                                         */
	/*          0, 16 bit(565RGB); 1, 24bit (888RGB)                 */
    if (vgaBitsPerPixel == 16) {
	new->Port_3D6[0x06] |= 0xC4;   /*15 or 16 bpp colour         */
	new->Port_3D6[0x0F] |= 0x10;   /*Hi-/True-Colour             */
	new->Port_3D6[0x40] = 0x02;    /*BitBLT Draw Mode for 16 bpp */
	if (xf86weight.green != 5)
	    new->Port_3D6[0x06] |= 0x08;	/*16bpp              */
    } else if (vgaBitsPerPixel == 24) {
	new->Port_3D6[0x06] |= 0xC8;   /*24 bpp colour               */
	new->Port_3D6[0x0F] |= 0x10;   /*Hi-/True-Colour             */
	if (OFLG_ISSET(OPTION_18_BIT_BUS, &vga256InfoRec.options)) {
	    new->Port_3D6[0x50] &= 0x7F;   /*18 bit TFT data width   */
	} else {
	    new->Port_3D6[0x50] |= 0x80;   /*24 bit TFT data width   */
	}
    }

    /*CRT only: interlaced mode */
    if (!ctLCD) {
	if (mode->Flags & V_INTERLACE){
	    new->Port_3D6[0x28] |= 0x20;    /* set interlace         */
	    new->Port_3D6[0x29] = 
	      ((((mode->CrtcHDisplay >> 3) - 1) >> 1) 
		- 6 * vgaBytesPerPixel);    /* heuristical value     */
	} else
	    new->Port_3D6[0x28] &= ~0x20;   /* unset interlace       */
    }

    /* STN specific */
    if (IS_STN(ctPanelType)) {
	new->Port_3D6[0x50] &= ~0x03;  /* FRC clear                  */
	new->Port_3D6[0x50] |= 0x01;   /* 16 frame FRC               */
	new->Port_3D6[0x50] &= ~0x0C;  /* Dither clear               */
	new->Port_3D6[0x50] |= 0x08;   /* Dither                     */
	new->Port_3D6[0x03] |= 0x20;   /* CRT I/F priority           */
	new->Port_3D6[0x04] |= 0x10;   /* RAS precharge              */
    }

    /*chip specific trick */
    /*It's better to say LCD panel specific... */
    switch (ct65545subtype) {
    case 2:			  /*jet mini *//*DEC HighNote Ultra DSTN */
	new->Port_3D6[0x03] |= 0x10;   /* ??    */
	break;
    case 3:			       /*CT 65546, only for Toshiba */
	new->Port_3D6[0x05] |= 0x80;   /* EDO RAM enable */
	break;
    }

    /* set mode */
    new->XMode = TRUE;		       /*  we want to switch to X */

    /*
     * Ugly hack to get MMIO base address.
     * Registers are mapped at linear base address plus offset of
     * 2Mb for 65545/6/8 with PCI. There is a 2Mb range of addresses
     * reserved by the blitter here. But as we only use a smaller
     * portion of it, only map 64k. Makes X look smaller, even
     * though it's not really.
     */
    if (ctUseMMIO && ctMMIOBase == NULL) {
	ctMMIOBase = xf86MapVidMem(vga256InfoRec.scrnIndex, LINEAR_REGION,
	    (pointer) (CHIPS.ChipLinearBase + 0x200000L),
	    0x10000L);
    }

    return (TRUE);
}

static Bool
CHIPSInitHiQV32(mode)
    DisplayModePtr mode;
{
    unsigned char tmp;
    int i, k, temp;
    int lcdHTotal, lcdHDisplay;
    int lcdVTotal, lcdVDisplay;
    int lcdHRetraceStart, lcdHRetraceEnd;
    int lcdVRetraceStart, lcdVRetraceEnd;
    int lcdHSyncStart;

#ifdef DEBUG
    ErrorF("CHIPSInitHiQV32\n");
#endif

    /* fix things that could be messed up by suspend/resume */
    tmp = inb(0x3CC);
    outb(0x3C2, (tmp & 0xFE) | ctVgaIOBaseFlag); 
    outb(ctCRindex, 0x11);
    tmp = inb(ctCRvalue);
    outb(ctCRvalue, (tmp & 0x7F));

    /* generic init */
    if (!vgaHWInit(mode, sizeof(vgaCHIPSRec))) {
	ErrorF("bomb 1\n");
	return (FALSE);
    }

    /* init clock */
    if (!ctClockFind(ctClockType, new->std.NoClock, &new->ctClock)) {
	ErrorF("bomb 1\n");
	return (FALSE);
    }
   
    /* get C&T Specific Registers */
    for (i = 0; i < 0xFF; i++) {
	outb(0x3D6, i);
	new->Port_3D6[i] = inb(0x3D7);
    }
    for (i = 0; i < 0x80; i++) {
	outb(0x3D0, i);
	new->Port_3D0[i] = inb(0x3D1);
    }
    for (i = 0x30; i < 0x80; i++) {    /* These are the CT extended CRT regs */
	outb(ctCRindex, i);
	new->Port_3D4[i] = inb(ctCRvalue);
    }

    /*
     * Here all of the other fields of 'new' get filled in, to
     * handle the SVGA extended registers.  It is also allowable
     * to override generic registers whenever necessary.
     */

    /* some generic settings */
    new->std.Attribute[0x10] = 0x01;   /* mode */
    new->std.Attribute[0x11] = 0x00;   /* overscan (border) color */
    new->std.Attribute[0x12] = 0x0F;   /* enable all color planes */
    new->std.Attribute[0x13] = 0x00;   /* horiz pixel panning 0 */

    new->std.Graphics[0x05] = 0x00;    /* normal read/write mode */

    /* set virtual screen width */
    temp = vga256InfoRec.displayWidth >> 3;
    if (vgaBitsPerPixel == 16) {
	temp <<= 1;		       /* double the width of the buffer */
    } else if (vgaBitsPerPixel == 24) {
	temp += temp << 1;
    }
    new->std.CRTC[0x13] = temp;
    new->Port_3D4[0x41] = (temp >> 8) & 0x0F;

    /* Set paging mode on the HiQV32 architecture, if required */
    if (OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options))
	new->Port_3D6[0x0A] |= 0x1;

    new->Port_3D6[0x09] |= 0x1;	       /* Enable extended CRT registers */
    new->Port_3D6[0x0E] = 0;           /* Single map */
    new->Port_3D6[0x40] |= 0x3;	       /* High Resolution. XR40[1] reserved? */
    new->Port_3D6[0x81] &= 0xF8;       /* 256 Color Video */
    new->Port_3D6[0x81] |= 0x2;
    new->Port_3D6[0x80] |= 0x10;       /* Enable cursor output on P0 and P1 */

    /* set mem clk */
    /* Graphics Modes seem to need a Higher MClk, than at Console
     * Force a higher Mclk for now */
    new->Port_3D6[0xCC] = 0x43;
    new->Port_3D6[0xCD] = 0x18;
    new->Port_3D6[0xCE] = 0xA1;

    /* linear specific */
    if (ctLinearSupport) {	       /* Note much acceleration only 
					* supported with linear addressing */
	new->Port_3D6[0x0A] |= 0x02;   /* Linear Addressing Mode */
	new->Port_3D6[0x20] = 0x0;     /*BitBLT Draw Mode for 8 */

	/* Initialise the HW cursor position. What is this for ??? */
	new->Port_3D6[0xA2] = ((vgaBytesPerPixel
		* (unsigned long)vga256InfoRec.displayWidth) >> 8) & 0xFF;
	new->Port_3D6[0xA3] = ((vgaBytesPerPixel
		* (unsigned long)vga256InfoRec.displayWidth) >> 16) & 0x3F;
    }

    /* panel timing */
    /* By default don't set panel timings, but allow it as an option */
    if (OFLG_ISSET(OPTION_USE_MODELINE, &vga256InfoRec.options)) {
	lcdHTotal = (mode->CrtcHTotal >> 3) - 5;
	lcdHDisplay = (ctSize.HDisplay >> 3) - 1;
	lcdHRetraceStart = (mode->CrtcHSyncStart >> 3);
	lcdHRetraceEnd = (mode->CrtcHSyncEnd >> 3);
	lcdHSyncStart = lcdHRetraceStart - 2;

	lcdVTotal = mode->CrtcVTotal - 2;
	lcdVDisplay = ctSize.VDisplay - 1;
	lcdVRetraceStart = mode->CrtcVSyncStart;
	lcdVRetraceEnd = mode->CrtcVSyncEnd;

	new->Port_3D0[0x20] = lcdHDisplay & 0xFF;
	new->Port_3D0[0x21] = lcdHRetraceStart & 0xFF;
	new->Port_3D0[0x25] = ((lcdHRetraceStart & 0xF00) >> 4) |
	    ((lcdHDisplay & 0xF00) >> 8);
	new->Port_3D0[0x22] = lcdHRetraceEnd & 0x1F;
	new->Port_3D0[0x23] = lcdHTotal & 0xFF;
	new->Port_3D0[0x24] = (lcdHSyncStart >> 3) & 0xFF;
	new->Port_3D0[0x26] = (new->Port_3D0[0x26] & ~0x1F)
	    | ((lcdHTotal & 0xF00) >> 8)
	    | (((lcdHSyncStart >> 3) & 0x100) >> 4);
	new->Port_3D0[0x27] &= 0x7F;

	new->Port_3D0[0x30] = lcdVDisplay & 0xFF;
	new->Port_3D0[0x31] = lcdVRetraceStart & 0xFF;
	new->Port_3D0[0x35] = ((lcdVRetraceStart & 0xF00) >> 4)
	    | ((lcdVDisplay & 0xF00) >> 8);
	new->Port_3D0[0x32] = lcdVRetraceEnd & 0x0F;
	new->Port_3D0[0x33] = lcdVTotal & 0xFF;
	new->Port_3D0[0x34] = (lcdVTotal - lcdVRetraceStart) & 0xFF;
	new->Port_3D0[0x36] = ((lcdVTotal & 0xF00) >> 8) |
	    (((lcdVTotal - lcdVRetraceStart) & 0x700) >> 4);
	new->Port_3D0[0x37] |= 0x80;
    }

    /* Set up the extended CRT registers of the HiQV32 chips */
    new->Port_3D4[0x30] = ((mode->CrtcVTotal - 2) & 0xF00) >> 8;
    new->Port_3D4[0x31] = ((mode->CrtcVDisplay - 1) & 0xF00) >> 8;
    new->Port_3D4[0x32] = (mode->CrtcVSyncStart & 0xF00) >> 8;
    new->Port_3D4[0x33] = (mode->CrtcVSyncStart & 0xF00) >> 8;
    new->Port_3D4[0x40] |= 0x80;

    /* centering/stretching */
    if(!OFLG_ISSET(OPTION_SUSPEND_HACK, &vga256InfoRec.options)) {
    if (OFLG_ISSET(OPTION_LCD_STRETCH, &vga256InfoRec.options)) {
	new->Port_3D0[0x40] = 0x01;    /* Disable Horizontal stretching */
	new->Port_3D0[0x48] = 0x01;    /* Disable vertical stretching */
    } else {
	new->Port_3D0[0x40] = 0x21;    /* Enable Horizontal stretching */
	new->Port_3D0[0x48] = 0x05;    /* Enable vertical stretching */
    }
  }
    if (OFLG_ISSET(OPTION_LCD_CENTER, &vga256InfoRec.options)) {
	new->Port_3D0[0x40] |= 0x2;    /* Enable Horizontal centering */
	new->Port_3D0[0x48] |= 0x2;    /* Enable Vertical centering */
    }

    /* sync on green */
    if (OFLG_ISSET(OPTION_SYNC_ON_GREEN, &vga256InfoRec.options)) 
      new->Port_3D6[0x82] |=0x02;

    /* software flag */
    /* I've taken a real guess that indx 0xE2 corresponds to 0x2B
     * on older chips. The ct65550 documents specifies this as a
     * software flag, and it appears to change with the mode */
    new->Port_3D6[0xE2] = ctVideoMode(vgaBitsPerPixel, xf86weight.green,
	ctLCD ? min(mode->CrtcHDisplay, ctSize.HDisplay) :
	mode->CrtcHDisplay);
#ifdef DEBUG
    ErrorF("VESA Mode: %Xh\n", new->Port_3D6[0xE2]);
#endif

    /* sync. polarities */
    if ((mode->Flags & (V_PHSYNC | V_NHSYNC))
	&& (mode->Flags & (V_PVSYNC | V_NVSYNC))) {
	if (mode->Flags & (V_PHSYNC | V_NHSYNC)) {
	    if (mode->Flags & V_PHSYNC)
		new->Port_3D0[0x08] &= 0xFB;	/* FP Hsync positive */
	    else
		new->Port_3D0[0x08] |= 0x04;	/* FP Hsync negative */
	}
	if (mode->Flags & (V_PVSYNC | V_NVSYNC)) {
	    if (mode->Flags & V_PVSYNC)
		new->Port_3D0[0x08] &= 0xF7;	/* FP Vsync positive */
	    else
		new->Port_3D0[0x08] |= 0x08;	/* FP Vsync negative */
	}
    }

    /* bpp depend */
    if (vgaBitsPerPixel == 16) {
	new->Port_3D6[0x81] = (new->Port_3D6[0x81] & 0xF0) | 0x4;
	/* 16bpp = 5-5-5             */
	new->Port_3D0[0x10] |= 0x0C;   /*Colour Panel               */
	new->Port_3D6[0x20] = 0x10;    /*BitBLT Draw Mode for 16 bpp */
	if (xf86weight.green != 5)
	    new->Port_3D6[0x81] |= 0x01;	/*16bpp */
    } else if (vgaBitsPerPixel == 24) {
	new->Port_3D6[0x81] = new->Port_3D6[0x81] & 0xF0 | 0x6;
	/* 24bpp colour              */
	new->Port_3D6[0x20] = 0x20;    /*BitBLT Draw Mode for 24 bpp */
    }

    /*CRT only */
    if (!ctLCD) {
	if (mode->Flags & V_INTERLACE)
	    new->Port_3D4[0x70] = 0x80          /*   set interlace */
	      | (((((mode->CrtcHDisplay >> 3) - 1) >> 1) - 6) & 0x7F);
		else
	    new->Port_3D4[0x70] &= ~0x80;	/* unset interlace */
    }

    /* STN specific */
    if (IS_STN(ctPanelType)) {
	new->Port_3D0[0x11] &= ~0x03;	/* FRC clear                    */
	new->Port_3D0[0x11] |= 0x01;	/* 16 frame FRC                 */
	new->Port_3D0[0x11] &= ~0x8C;	/* Dither clear                 */
	new->Port_3D0[0x11] |= 0x84;	/* Dither                       */
	if (ctPanelType == DD)		/* Shift Clock Mask. Use to get */
	    new->Port_3D0[0x12] |= 0x4;	/* rid of line in DSTN screens  */
    }

    /* set mode */ 
    new->XMode = TRUE;			/*  we want to switch to X      */

    /*
     * Ugly hack to get MMIO base address.
     * Registers are mapped at linear base address plus offset of
     * 4 Mb for 65550/4. There is a 4Mb range of addresses
     * reserved by the blitter here. But as we only use a smaller
     * portion of it, only map 128k. Makes X look smaller, even
     * though it's not really.
     */
    if (ctUseMMIO && ctMMIOBase == NULL) {
	ctMMIOBase = xf86MapVidMem(vga256InfoRec.scrnIndex, LINEAR_REGION,
	    (pointer) (CHIPS.ChipLinearBase + 0x400000L),
	    0x20000L);
	/*
	 * Setup the address to write monchrome source data to, for 
	 * system to the screen colour expansion.
	 */
	ctBltDataWindow = ctMMIOBase + 0x10000;
    }

    return (TRUE);
}

static void
CHIPSAdjust(x, y)
    int x, y;
{
    /* MH - looks like we are in byte mode.... */
    int Base = (y * vga256InfoRec.displayWidth + x);
    unsigned char tmp;

    /* fix things that could be messed up by suspend/resume */
    if(!ctisHiQV32)
        outb(0x3D6,0x15);
    tmp = inb(0x3CC);
    outb(0x3C2, (tmp & 0xFE) | ctVgaIOBaseFlag); 
    outb(ctCRindex, 0x11);
    tmp = inb(ctCRvalue);
    outb(ctCRvalue, (tmp & 0x7F));

    /* calculate base bpp dep. */
    switch (vgaBitsPerPixel) {
    case 16:
	Base >>= 1;
	break;
    case 24:
	Base = (Base * 3) >> 2;
	break;
    default:			       /* 8bpp */
	Base >>= 2;
	break;
    }

    /* write base to chip */
    /*
     * These are the generic starting address registers.
     */
    outw(ctCRindex, (Base & 0x00FF00) | 0x0C);
    outw(ctCRindex, ((Base & 0x00FF) << 8) | 0x0D);

    /*
     * Here the high-order bits are masked and shifted, and put into
     * the appropriate extended registers.
     */

    /* MH - plug in the high order starting address bits */
    if (ctisHiQV32) {
	outb(0x3D6, 0x09);
	if ((inb(0x3D7) & 0x1) == 0x1)
	    outw(ctCRindex, ((Base & 0x0F0000) >> 8) | 0x8000 | 0x40);
    } else {
	outb(0x3D6, 0x0C);
	tmp = inb(0x3D7);
	outb(0x3D7, ((Base & 0x030000) >> 16) | (tmp & 0xF8));
    }
#ifdef XFreeXDGA
    if (vga256InfoRec.directMode & XF86DGADirectGraphics) {
	/* Wait until vertical retrace is in progress. */
	while (inb(vgaIOBase + 0xA) & 0x08);
	while (!(inb(vgaIOBase + 0xA) & 0x08));
    }
#endif
}

static int
CHIPSValidMode(mode, verbose)
    DisplayModePtr mode;
    Bool verbose;
{
    return MODE_OK;
}

/*adapted from pvg_driver.c and cir_driver.c */
static void
CHIPSFbInit()
{
    int useSpeedUp, size;

    if ((vga256InfoRec.displayWidth * vga256InfoRec.virtualY * vgaBytesPerPixel)
	> ((vga256InfoRec.videoRam << 10) - ctFrameBufferSize)) {
	ErrorF("%s %s: CHIPS: Virtual screen too large.\n",
	       XCONFIG_PROBED, vga256InfoRec.name);
	ErrorF("%s %s: CHIPS: Screen will be corrupted.\n",
	       XCONFIG_PROBED, vga256InfoRec.name);
    }

#ifdef DEBUG
    ErrorF("Frame Buffer Size: %i\n", ctFrameBufferSize);
#endif

    if (!ctAccelSupport || OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options)) {
	return;
    }
    size = ctInitializeAllocator();
    ErrorF("%s %s: CHIPS: %d bytes off-screen memory available\n",
	XCONFIG_PROBED, vga256InfoRec.name, size);


#ifndef MONOVGA

    useSpeedUp = vga256InfoRec.speedup & SPEEDUP_ANYWIDTH;
    if (useSpeedUp && !OFLG_ISSET(OPTION_NO_BITBLT, &vga256InfoRec.options)) {

	ErrorF("%s %s: CHIPS: SpeedUps selected (Flags=0x%X)\n",
	    OFLG_ISSET(XCONFIG_SPEEDUP, &vga256InfoRec.xconfigFlag) ?
	    XCONFIG_GIVEN : XCONFIG_PROBED,
	    vga256InfoRec.name, useSpeedUp);

	if (OFLG_ISSET(OPTION_NO_IMAGEBLT, &vga256InfoRec.options)) {
	    ErrorF("%s %s: CHIPS: Not using system-to-video BitBLT.\n",
		   XCONFIG_GIVEN, vga256InfoRec.name);
	    ctAvoidImageBLT = TRUE;
	}
	
	if (!ctisHiQV32) {
	    switch (vgaBitsPerPixel) {
	    case 8:
		vga256LowlevFuncs.doBitbltCopy = ctcfbDoBitbltCopy;
		vga256LowlevFuncs.vgaBitblt = ctBitBlt;
		vga256LowlevFuncs.fillRectSolidCopy = ctcfbFillRectSolid;

		/* Hook special op. fills (and tiles): */
		vga256TEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		vga256NonTEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		vga256TEOps.PolyFillRect = ctcfbPolyFillRect;
		vga256NonTEOps.PolyFillRect = ctcfbPolyFillRect;

		vga256LowlevFuncs.fillBoxSolid = ctcfbFillBoxSolid;
		vga256TEOps1Rect.FillSpans = ctcfbFillSolidSpansGeneral;
		vga256TEOps.FillSpans = ctcfbFillSolidSpansGeneral;
		vga256LowlevFuncs.fillSolidSpans = ctcfbFillSolidSpansGeneral;
	    
#ifdef CT_POST_312F_ACCL
		if (!OFLG_ISSET(OPTION_NO_IMAGEBLT, &vga256InfoRec.options)) {
		    vga256LowlevFuncs.copyPlane1to8 = ctcfbCopyPlane1to8;
		     vga256TEOps1Rect.PolyGlyphBlt = ctcfbPolyGlyphBlt;
		     vga256TEOps.PolyGlyphBlt = ctcfbPolyGlyphBlt;
		     vga256LowlevFuncs.teGlyphBlt8 = ctcfbImageGlyphBlt;
		     vga256TEOps1Rect.ImageGlyphBlt = ctcfbImageGlyphBlt;
		     vga256TEOps.ImageGlyphBlt = ctcfbImageGlyphBlt;
		}
#endif

		/* Setup the address of the tile in vram. Tile must
		 * be aligned on a 64 bytes value. Size of the space
		 * is 32x8xBytesPerPixel */
		ctBLTPatternAddress = ctAllocate(256, 0x3F);
		if (ctBLTPatternAddress == -1)
		ErrorF("%s %s: CHIPS: Too little space for accelerated tile.\n",
		       XCONFIG_PROBED, vga256InfoRec.name);

		break;
		
	    case 16:
		/* There are no corresponding structures to vga256LowlevFuncs
		 * for 16/24bpp. Hence we have to hook to the cfb functions in
		 * a similar way to the cirrus driver. For now I've just
		 * implemented the most basic of blits */
		cfb16TEOps1Rect.CopyArea = ctcfb16CopyArea;
		cfb16TEOps.CopyArea = ctcfb16CopyArea;
		cfb16NonTEOps1Rect.CopyArea = ctcfb16CopyArea;
		cfb16NonTEOps.CopyArea = ctcfb16CopyArea;

		cfb16TEOps1Rect.FillSpans = ctcfbFillSolidSpansGeneral;
		cfb16TEOps.FillSpans = ctcfbFillSolidSpansGeneral;
		cfb16NonTEOps1Rect.FillSpans = ctcfbFillSolidSpansGeneral;
		cfb16NonTEOps.FillSpans = ctcfbFillSolidSpansGeneral;
		cfb16TEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		cfb16TEOps.PolyFillRect = ctcfbPolyFillRect;
		cfb16NonTEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		cfb16NonTEOps.PolyFillRect = ctcfbPolyFillRect;

#ifdef CT_POST_312F_ACCL
		if (!OFLG_ISSET(OPTION_NO_IMAGEBLT, &vga256InfoRec.options)) {
		    cfb16TEOps1Rect.PolyGlyphBlt = ctcfbPolyGlyphBlt;
		    cfb16TEOps.PolyGlyphBlt = ctcfbPolyGlyphBlt;
		    cfb16TEOps1Rect.ImageGlyphBlt = ctcfbImageGlyphBlt;
		    cfb16TEOps.ImageGlyphBlt = ctcfbImageGlyphBlt;
		}
#endif

		/* Setup the address of the tile in vram. Tile must
		 * be aligned on a 128 bytes value. Size of the space
		 * is 32x8xBytesPerPixel. However tiling code doesn't
		 * seem to be called by the server. This should be
		 * investigated */
		ctBLTPatternAddress = ctAllocate(512, 0x7F);
		if (ctBLTPatternAddress == -1)
		    ErrorF("%s %s: CHIPS: Too little space for accelerated tile.\n",
			   XCONFIG_PROBED, vga256InfoRec.name);
		break;

	    case 24:
		/* There are no corresponding structures to vga256LowlevFuncs
		 * for 16/24bpp. Hence we have to hook to the cfb functions in
		 * a similar way to the cirrus driver. For now I've just 
		 * implemented the most basic of blits */

		cfb24TEOps1Rect.CopyArea = ctcfb24CopyArea;
		cfb24TEOps.CopyArea = ctcfb24CopyArea;
		cfb24NonTEOps1Rect.CopyArea = ctcfb24CopyArea;
		cfb24NonTEOps.CopyArea = ctcfb24CopyArea;

		cfb24TEOps1Rect.FillSpans = ctcfbFillSolidSpansGeneral;
		cfb24TEOps.FillSpans = ctcfbFillSolidSpansGeneral;
		cfb24NonTEOps1Rect.FillSpans = ctcfbFillSolidSpansGeneral;
		cfb24NonTEOps.FillSpans = ctcfbFillSolidSpansGeneral;
		cfb24TEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		cfb24TEOps.PolyFillRect = ctcfbPolyFillRect;
		cfb24NonTEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		cfb24NonTEOps.PolyFillRect = ctcfbPolyFillRect;

#ifdef CT_POST_312F_ACCL
#ifdef CT_24BPP_TEXT
		if (!OFLG_ISSET(OPTION_NO_IMAGEBLT, &vga256InfoRec.options)) {
		    cfb24TEOps1Rect.PolyGlyphBlt = ctcfbPolyGlyphBlt;
		    cfb24TEOps.PolyGlyphBlt = ctcfbPolyGlyphBlt;
		    cfb24TEOps1Rect.ImageGlyphBlt = ctcfbImageGlyphBlt;
		    cfb24TEOps.ImageGlyphBlt = ctcfbImageGlyphBlt;
		}
#endif
#endif
		/* Setup the address of the tile in vram. Tile must
		 * be aligned on a 256 bytes value. Size of the space
		 * is 32x8xBytesPerPixel. 24bpp tile file only 
		 * available on 65550, this is here for use later */
		ctBLTPatternAddress = ctAllocate(768, 0xFF);
		if (ctBLTPatternAddress == -1)
		    ErrorF("%s %s: CHIPS: Too little space for accelerated tile.\n",
			   XCONFIG_PROBED, vga256InfoRec.name);
		break;

	    }

	    /* If direct memory access to the blitter registers is available,
	     * then use it to do the blitting as it is faster. Many blit
	     * operation are too slow to utilise without MMIO (eg Line
	     * acceleration) and so are only included here. Note that vgaBase
	     *  is not initialised at this point. So we can't set ctMMIOBase
	     * here. It is done in CHIPSInit. */

	    if (ctUseMMIO) {
		ErrorF("%s %s: CHIPS: Memory mapped I/O selected\n",
		    OFLG_ISSET(OPTION_MMIO, &vga256InfoRec.options) ?
		    XCONFIG_GIVEN : XCONFIG_PROBED, vga256InfoRec.name);
		switch (vgaBitsPerPixel) {
		case 8:
		    vga256LowlevFuncs.vgaBitblt = ctMMIOBitBlt;
		    vga256LowlevFuncs.fillRectSolidCopy = ctMMIOFillRectSolid;
		    vga256TEOps1Rect.FillSpans = ctMMIOFillSolidSpansGeneral;
		    vga256TEOps.FillSpans = ctMMIOFillSolidSpansGeneral;
		    vga256LowlevFuncs.fillSolidSpans = ctMMIOFillSolidSpansGeneral;

#ifdef CT_POST_312F_ACCL
		    if (!OFLG_ISSET(OPTION_NO_IMAGEBLT,
				   &vga256InfoRec.options)) {
			vga256TEOps1Rect.PolyGlyphBlt = ctMMIOPolyGlyphBlt;
			vga256TEOps.PolyGlyphBlt = ctMMIOPolyGlyphBlt;
			vga256LowlevFuncs.teGlyphBlt8 = ctMMIOImageGlyphBlt;
			vga256TEOps1Rect.ImageGlyphBlt = ctMMIOImageGlyphBlt;
			vga256TEOps.ImageGlyphBlt = ctMMIOImageGlyphBlt;
		    }
#endif

#ifdef CT_LINE_ACCL
		    vga256TEOps1Rect.Polylines = ctMMIOLineSS;
		    vga256NonTEOps1Rect.Polylines = ctMMIOLineSS;
		    vga256TEOps.Polylines = ctMMIOLineSS;
		    vga256NonTEOps.Polylines = ctMMIOLineSS;
		    vga256TEOps1Rect.PolySegment = ctMMIOSegmentSS;
		    vga256NonTEOps1Rect.PolySegment = ctMMIOSegmentSS;
		    vga256TEOps.PolySegment = ctMMIOSegmentSS;
		    vga256TEOps.PolySegment = ctMMIOSegmentSS;
#endif
		    break;
		case 16:
		    cfb16TEOps1Rect.FillSpans = ctMMIOFillSolidSpansGeneral;
		    cfb16TEOps.FillSpans = ctMMIOFillSolidSpansGeneral;
		    cfb16NonTEOps1Rect.FillSpans = ctMMIOFillSolidSpansGeneral;
		    cfb16NonTEOps.FillSpans = ctMMIOFillSolidSpansGeneral;

#ifdef CT_POST_312F_ACCL
		    if (!OFLG_ISSET(OPTION_NO_IMAGEBLT,
				   &vga256InfoRec.options)) {
			cfb16TEOps1Rect.PolyGlyphBlt = ctMMIOPolyGlyphBlt;
			cfb16TEOps.PolyGlyphBlt = ctMMIOPolyGlyphBlt;
			cfb16TEOps1Rect.ImageGlyphBlt = ctMMIOImageGlyphBlt;
			cfb16TEOps.ImageGlyphBlt = ctMMIOImageGlyphBlt;
		    }
#endif

#ifdef CT_LINE_ACCL
		    /* The way of hooking to these should be looked in to.
		     * see the files ../../../../../cfb24/cfbgc.c and
		     * ../../../../../cfb24/cfbmskbits.h. It is possible that 
		     * this isn't always the right way to do this */

		    cfb16TEOps1Rect.Polylines = ctMMIOLineSS;
		    cfb16NonTEOps1Rect.Polylines = ctMMIOLineSS;
		    cfb16TEOps1Rect.PolySegment = ctMMIOSegmentSS;
		    cfb16NonTEOps1Rect.PolySegment = ctMMIOSegmentSS;

		    cfb16TEOps.Polylines = ctMMIOLineSS;
		    cfb16NonTEOps.Polylines = ctMMIOLineSS;
		    cfb16TEOps.PolySegment = ctMMIOSegmentSS;
		    cfb16TEOps.PolySegment = ctMMIOSegmentSS;
#endif
		    break;
		case 24:
		    cfb24TEOps1Rect.FillSpans = ctMMIOFillSolidSpansGeneral;
		    cfb24TEOps.FillSpans = ctMMIOFillSolidSpansGeneral;
		    cfb24NonTEOps1Rect.FillSpans = ctMMIOFillSolidSpansGeneral;
		    cfb24NonTEOps.FillSpans = ctMMIOFillSolidSpansGeneral;

#ifdef CT_POST_312F_ACCL
#ifdef CT_24BPP_TEXT
		    if (!OFLG_ISSET(OPTION_NO_IMAGEBLT,
				   &vga256InfoRec.options)) {
			cfb24TEOps1Rect.PolyGlyphBlt = ctMMIOPolyGlyphBlt;
			cfb24TEOps.PolyGlyphBlt = ctMMIOPolyGlyphBlt;
			cfb24TEOps1Rect.ImageGlyphBlt = ctMMIOImageGlyphBlt;
			cfb24TEOps.ImageGlyphBlt = ctMMIOImageGlyphBlt;
		    }
#endif
#endif

#ifdef CT_LINE_ACCL
		    /* The way of hooking to these should be looked in to.
		     * see the files ../../../../../cfb24/cfbgc.c and
		     * ../../../../../cfb24/cfbmskbits.h. It is possible that 
		     * this isn't always the right way to do this */

		    cfb24TEOps1Rect.Polylines = ctMMIOLineSS;
		    cfb24NonTEOps1Rect.Polylines = ctMMIOLineSS;
		    cfb24TEOps1Rect.PolySegment = ctMMIOSegmentSS;
		    cfb24NonTEOps1Rect.PolySegment = ctMMIOSegmentSS;

		    cfb24TEOps.Polylines = ctMMIOLineSS;
		    cfb24NonTEOps.Polylines = ctMMIOLineSS;
		    cfb24TEOps.PolySegment = ctMMIOSegmentSS;
		    cfb24TEOps.PolySegment = ctMMIOSegmentSS;
#endif
		    break;
		}
	    }
	} else {	/* HiQV acceleration support */ 
#ifdef CT_POST_312F_ACCL
	    if (ctUseMMIO) {
		ErrorF("%s %s: CHIPS: Memory mapped I/O selected\n",
		    OFLG_ISSET(OPTION_MMIO, &vga256InfoRec.options) ?
		    XCONFIG_GIVEN : XCONFIG_PROBED, vga256InfoRec.name);
		switch (vgaBitsPerPixel) {
		case 8:
		    vga256LowlevFuncs.doBitbltCopy = ctcfbDoBitbltCopy;
		    vga256LowlevFuncs.vgaBitblt = ctHiQVBitBlt;

		    vga256LowlevFuncs.fillRectSolidCopy = ctHiQVFillRectSolid;

		    vga256LowlevFuncs.fillBoxSolid = ctcfbFillBoxSolid;
		    vga256TEOps1Rect.FillSpans = ctHiQVFillSolidSpansGeneral;
		    vga256TEOps.FillSpans = ctHiQVFillSolidSpansGeneral;
		    vga256LowlevFuncs.fillSolidSpans = ctHiQVFillSolidSpansGeneral;
		    /* Setup the address of the tile in vram. Tile must
		     * be aligned on a 64 bytes value. Size of the space
		     * is 32x8xBytesPerPixel */
		    ctBLTPatternAddress = ctAllocate(256, 0x3F);
		    if (ctBLTPatternAddress == -1)
		        ErrorF("%s %s: CHIPS: Too little space for accelerated tile.\n",
			       XCONFIG_PROBED, vga256InfoRec.name);

#ifdef CT_LINE_ACCL
		    vga256TEOps1Rect.Polylines = ctHiQVLineSS;
		    vga256NonTEOps1Rect.Polylines = ctHiQVLineSS;
		    vga256TEOps.Polylines = ctHiQVLineSS;
		    vga256NonTEOps.Polylines = ctHiQVLineSS;
		    vga256TEOps1Rect.PolySegment = ctHiQVSegmentSS;
		    vga256NonTEOps1Rect.PolySegment = ctHiQVSegmentSS;
		    vga256TEOps.PolySegment = ctHiQVSegmentSS;
		    vga256TEOps.PolySegment = ctHiQVSegmentSS;
#endif

#if 0	/* Untested with the HiQV. Need adaptation */

		    /* Hook special op. fills (and tiles): */
		    vga256TEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		    vga256NonTEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		    vga256TEOps.PolyFillRect = ctcfbPolyFillRect;
		    vga256NonTEOps.PolyFillRect = ctcfbPolyFillRect;

		    if (!OFLG_ISSET(OPTION_NO_IMAGEBLT,
				   &vga256InfoRec.options)) {	    
			vga256LowlevFuncs.copyPlane1to8 = ctcfbCopyPlane1to8;
			vga256TEOps1Rect.PolyGlyphBlt = ctHiQVPolyGlyphBlt;
			vga256TEOps.PolyGlyphBlt = ctHiQVPolyGlyphBlt;
			vga256LowlevFuncs.teGlyphBlt8 = ctHiQVImageGlyphBlt;
			vga256TEOps1Rect.ImageGlyphBlt = ctHiQVImageGlyphBlt;
			vga256TEOps.ImageGlyphBlt = ctHiQVImageGlyphBlt;
		    }
		    
#endif	/* Untested with the HiQV. Need adaptation */
		    break;
		
		case 16:
		    /* There are no corresponding structures to vga256LowlevFuncs
		     * for 16/24bpp. Hence we have to hook to the cfb functions
		     * in a similar way to the cirrus driver. For now I've just
		     * implemented the most basic of blits */
		    cfb16TEOps1Rect.CopyArea = ctcfb16CopyArea;
		    cfb16TEOps.CopyArea = ctcfb16CopyArea;
		    cfb16NonTEOps1Rect.CopyArea = ctcfb16CopyArea;
		    cfb16NonTEOps.CopyArea = ctcfb16CopyArea;

		    cfb16TEOps1Rect.FillSpans = ctHiQVFillSolidSpansGeneral;
		    cfb16TEOps.FillSpans = ctHiQVFillSolidSpansGeneral;
		    cfb16NonTEOps1Rect.FillSpans = ctHiQVFillSolidSpansGeneral;
		    cfb16NonTEOps.FillSpans = ctHiQVFillSolidSpansGeneral;

		    /* Setup the address of the tile in vram. Tile must
		     * be aligned on a 128 bytes value. Size of the space
		     * is 32x8xBytesPerPixel. However tiling code doesn't
		     * seem to be called by the server. This should be
		     * investigated */
		    ctBLTPatternAddress = ctAllocate(512, 0x7F);
		    if (ctBLTPatternAddress == -1)
		        ErrorF("%s %s: CHIPS: Too little space for accelerated tile.\n",
			       XCONFIG_PROBED, vga256InfoRec.name);
#ifdef CT_LINE_ACCL
		    /* The way of hooking to these should be looked in to.
		     * see the files ../../../../../cfb24/cfbgc.c and
		     * ../../../../../cfb24/cfbmskbits.h. It is possible that 
		     * this isn't always the right way to do this */

		    cfb16TEOps1Rect.Polylines = ctHiQVLineSS;
		    cfb16NonTEOps1Rect.Polylines = ctHiQVLineSS;
		    cfb16TEOps1Rect.PolySegment = ctHiQVSegmentSS;
		    cfb16NonTEOps1Rect.PolySegment = ctHiQVSegmentSS;

		    cfb16TEOps.Polylines = ctHiQVLineSS;
		    cfb16NonTEOps.Polylines = ctHiQVLineSS;
		    cfb16TEOps.PolySegment = ctHiQVSegmentSS;
		    cfb16TEOps.PolySegment = ctHiQVSegmentSS;
#endif

#if 0	/* Untested with the HiQV. Need adaptation */
		    cfb16TEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		    cfb16TEOps.PolyFillRect = ctcfbPolyFillRect;
		    cfb16NonTEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		    cfb16NonTEOps.PolyFillRect = ctcfbPolyFillRect;

		    if (!OFLG_ISSET(OPTION_NO_IMAGEBLT, 
				   &vga256InfoRec.options)) {
			cfb16TEOps1Rect.PolyGlyphBlt = ctHiQVPolyGlyphBlt;
			cfb16TEOps.PolyGlyphBlt = ctHiQVPolyGlyphBlt;
			cfb16TEOps1Rect.ImageGlyphBlt = ctHiQVImageGlyphBlt;
			cfb16TEOps.ImageGlyphBlt = ctHiQVImageGlyphBlt;
		    }
		    
#endif	/* Untested with the HiQV. Need adaptation */

		    break;

		case 24:
		    /* There are no corresponding structures to vga256LowlevFuncs
		     * for 16/24bpp. Hence we have to hook to the cfb functions
		     * in a similar way to the cirrus driver. For now I've
		     * just implemented the most basic of blits */

		    cfb24TEOps1Rect.CopyArea = ctcfb24CopyArea;
		    cfb24TEOps.CopyArea = ctcfb24CopyArea;
		    cfb24NonTEOps1Rect.CopyArea = ctcfb24CopyArea;
		    cfb24NonTEOps.CopyArea = ctcfb24CopyArea;

		    cfb24TEOps1Rect.FillSpans = ctHiQVFillSolidSpansGeneral;
		    cfb24TEOps.FillSpans = ctHiQVFillSolidSpansGeneral;
		    cfb24NonTEOps1Rect.FillSpans = ctHiQVFillSolidSpansGeneral;
		    cfb24NonTEOps.FillSpans = ctHiQVFillSolidSpansGeneral;

		    /* Setup the address of the tile in vram. Tile must
		     * be aligned on a 256 bytes value. Size of the space
		     * is 32x8xBytesPerPixel. 24bpp tile file only 
		     * available on 65550, this is here for use later */
		    ctBLTPatternAddress = ctAllocate(768, 0xFF);
		    if (ctBLTPatternAddress == -1)
		        ErrorF("%s %s: CHIPS: Too little space for accelerated tile.\n",
			       XCONFIG_PROBED, vga256InfoRec.name);
		    break;

#ifdef CT_LINE_ACCL
		    /* The way of hooking to these should be looked in to.
		     * see the files ../../../../../cfb24/cfbgc.c and
		     * ../../../../../cfb24/cfbmskbits.h. It is possible that 
		     * this isn't always the right way to do this */

		    cfb24TEOps1Rect.Polylines = ctHiQVLineSS;
		    cfb24NonTEOps1Rect.Polylines = ctHiQVLineSS;
		    cfb24TEOps1Rect.PolySegment = ctHiQVSegmentSS;
		    cfb24NonTEOps1Rect.PolySegment = ctHiQVSegmentSS;

		    cfb24TEOps.Polylines = ctHiQVLineSS;
		    cfb24NonTEOps.Polylines = ctHiQVLineSS;
		    cfb24TEOps.PolySegment = ctHiQVSegmentSS;
		    cfb24TEOps.PolySegment = ctHiQVSegmentSS;
#endif

#if 0	/* Untested with the HiQV. Need adaptation */
		    cfb24TEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		    cfb24TEOps.PolyFillRect = ctcfbPolyFillRect;
		    cfb24NonTEOps1Rect.PolyFillRect = ctcfbPolyFillRect;
		    cfb24NonTEOps.PolyFillRect = ctcfbPolyFillRect;

		    if (!OFLG_ISSET(OPTION_NO_IMAGEBLT, 
				   &vga256InfoRec.options)) {
			cfb24TEOps1Rect.PolyGlyphBlt = ctHiQVPolyGlyphBlt;
			cfb24TEOps.PolyGlyphBlt = ctHiQVPolyGlyphBlt;
			cfb24TEOps1Rect.ImageGlyphBlt = ctHiQVImageGlyphBlt;
			cfb24TEOps.ImageGlyphBlt = ctHiQVImageGlyphBlt;
		    }
		    
#endif	/* Untested with the HiQV. Need adaptation */

		}
	    } else {
		ErrorF("%s %s: CHIPS: Memory mapped I/O not available\n",
		       XCONFIG_PROBED, vga256InfoRec.name);
		ErrorF("%s %s: CHIPS: HiQV acceleration support disabled\n",
			XCONFIG_PROBED, vga256InfoRec.name); 
	    }
#else	/* CT_POST_312F_ACCL */
	    ErrorF("%s %s: CHIPS: HiQV acceleration support disabled\n",
		   XCONFIG_PROBED, vga256InfoRec.name); 
#endif
	}
    }
#endif /* MONOVGA */

    /*
     * If hardware cursor is supported, the vgaHWCursor struct should
     * be filled in here.
     */
    if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) {

	/* Allocate 1kB of vram to the cursor, with 1kB alignment for
	 * 6554x's and 4kb alignment for 65550's */
	if (ctisHiQV32)
	    ctCursorAddress = ctAllocate(1024, 0xFFF);
	else
	    ctCursorAddress = ctAllocate(1024, 0x3FF);
	if (ctCursorAddress == -1) {
	    ErrorF("%s %s: CHIPS: Too little space for H/W cursor.\n",
		XCONFIG_PROBED, vga256InfoRec.name);
	} else {
	    ErrorF("%s %s: CHIPS: H/W cursor selected\n",
		OFLG_ISSET(XCONFIG_SPEEDUP, &vga256InfoRec.xconfigFlag) ?
		XCONFIG_GIVEN : XCONFIG_PROBED,
		vga256InfoRec.name);
	    vgaHWCursor.Initialized = TRUE;
	    vgaHWCursor.Init = CHIPSCursorInit;
	    vgaHWCursor.Restore = CHIPSRestoreCursor;
	    vgaHWCursor.Warp = CHIPSWarpCursor;
	    vgaHWCursor.QueryBestSize = CHIPSQueryBestSize;
	    ctHWCursor = TRUE;
	}
    }
#ifdef DEBUG
    ErrorF("CHIPSFbInit: exit\n");
#endif
}

ctRestoreStretching(ctHorizontalStretch, ctVerticalStretch)
    unsigned char ctHorizontalStretch, ctVerticalStretch;
{
  unsigned char tmp;
    /* write to regs. */
    if (ctisHiQV32) {
        read_fr(0x40, tmp);
        write_fr(0x40, (tmp & 0xFE) | (ctHorizontalStretch & 0x01));
        read_fr(0x48, tmp);
        write_fr(0x48, (tmp & 0xFE) | (ctVerticalStretch & 0x01));
    }
    else {
        read_xr(0x55, tmp);
        write_xr(0x55, (tmp & 0xFE) | (ctHorizontalStretch & 0x01));
        read_xr(0x57, tmp);
        write_xr(0x57, (tmp & 0xFE) | (ctVerticalStretch & 0x01));
    }

    usleep(20000);			/* to be active */
}


void
ctRestore(restore)
    vgaCHIPSPtr restore;
{
    int i, j;
    unsigned char tmp;

    if (ctisHiQV32) {
      
      /* save BltRegs */
	if (ctUseMMIO) {
	    for (i = 0x0; i < 0x9; i++) {
		*(unsigned int *)(ctMMIOBase + ctMMIO[i]) = restore->BltReg[i];
	    }
	}

	/* set extended regs */
	for (i = 0; i < 0x43; i++) {
	    outb(0x3D6, i);
	    if (inb(0x3D7) != restore->Port_3D6[i])
		outb(0x3D7, restore->Port_3D6[i]);
	}
	/* Don't touch reserved memory control registers */
	for (i = 0x50; i < 0xBF; i++) {
	    outb(0x3D6, i);
	    if (inb(0x3D7) != restore->Port_3D6[i])
		outb(0x3D7, restore->Port_3D6[i]);
	}
	/* Don't touch VCLK regs, but fix up MClk */
	
	/* set mem clock */
	outb(0x3D6, 0xCE);	       /* Select Fixed MClk before */
	tmp = inb(0x3D7);
	outb(0x3D7, tmp & 0x7F);
	outb(0x3D6, 0xCC);
	if (inb(0x3D7) != restore->Port_3D6[0xCC])
	    outb(0x3D7, restore->Port_3D6[0xCC]);
	outb(0x3D6, 0xCD);
	if (inb(0x3D7) != restore->Port_3D6[0xCD])
	    outb(0x3D7, restore->Port_3D6[0xCD]);
	outb(0x3D6, 0xCE);
	if (inb(0x3D7) != restore->Port_3D6[0xCE])
	    outb(0x3D7, restore->Port_3D6[0xCE]);

	/* set flat panel regs. */
	for (i = 0xD0; i < 0xFF; i++) {
	    outb(0x3D6, i);
	    if (inb(0x3D7) != restore->Port_3D6[i])
		outb(0x3D7, restore->Port_3D6[i]);
	}
	for (i = 0; i < 0x2; i++) {
	    outb(0x3D0, i);
	    if (inb(0x3D1) != restore->Port_3D0[i])
		outb(0x3D1, restore->Port_3D0[i]);
	}
	outb(0x3D0, 0x03);
	tmp = inb(0x3D1);	       /* restore the non clock bits */
	outb(0x3D0, 0x03);
	outb(0x3D1, ((restore->Port_3D0[0x03] & 0xC3) | (tmp & ~0xC3)));
	i++;
	/* Don't touch alternate clock select reg. */
	for (; i < 0x80; i++) {
	    if ( (i == 0x40) || (i==0x48)) 
	        continue ;  /* some registers must be set before FR40/FR48 */
	    outb(0x3D0, i);
	    if (inb(0x3D1) != restore->Port_3D0[i])/* only modify if changed */
		outb(0x3D1, restore->Port_3D0[i]);
	}

	/* set extended crtc regs. */
	for (i = 0x30; i < 0x80; i++) {
	  outb(ctCRindex, i);
	  if (inb(ctCRvalue) != restore->Port_3D4[i]) /* only modify if changed */
		outb(ctCRvalue, restore->Port_3D4[i]);
	}
    } else {
      /* save BitBlt regs. */
	if(!ctUseMMIO){
	  for(i=0;i<0xD;i++){
	     HW_DEBUG(i); outl(DR(i), restore->BltReg[i]);
	   };
	} else {
	  for (i=0;i<0xD;i++){
	    HW_DEBUG(ctMMIO[i]); MMIOmeml(ctMMIO[i]) = restore->BltReg[i];
	  }	
	}
	/* set extended regs. */
	for (i = 0; i < 0x30; i++) {
	    outb(0x3D6, i);
	    if (inb(0x3D7) != restore->Port_3D6[i])
		outb(0x3D7, restore->Port_3D6[i]);
	}
	outw(0x3D6,0x15); /* unprotect just in case ... */
	/* Don't touch MCLK/VCLK regs. */
	for (i = 0x34; i < 0x54; i++) {
	    outb(0x3D6, i);
	    if (inb(0x3D7) != restore->Port_3D6[i])/* only modify if changed */
	      outb(0x3D7, restore->Port_3D6[i]);
	}
	outb(0x3D6, 0x54);
	tmp = inb(0x3D7);	/*  restore the non clock bits             */
	outb(0x3D6, 0x54); 	/* Don't touch alternate clock select reg. */
	outb(0x3D7, ((restore->Port_3D6[0x54] & 0xF3) | (tmp & ~0xF3)));
	outb(0x3D6, 0x55);      /* output with h-comp off                  */
	outb(0x3D7, (restore->Port_3D6[0x55] & 0xFE));
	outb(0x3D6, 0x56);
	outb(0x3D7, restore->Port_3D6[0x56]);
	outb(0x3D6, 0x57);      /* output with v-comp off                  */
	outb(0x3D7, (restore->Port_3D6[0x57] & 0xFE));
	for (i=0x58; i < 0x80; i++) {
	  outb(0x3D6, i);
	    if (inb(0x3D7) != restore->Port_3D6[i])/* only modify if changed */
	        outb(0x3D7, restore->Port_3D6[i]);
	}
    }
}

int
ctVideoMode(vgaBitsPerPixel, weightGreen, displaySize)
    int vgaBitsPerPixel, weightGreen, displaySize;
{
    /*     4 bpp  8 bpp  16 bpp  18 bpp  24 bpp  32 bpp */
    /* 640  0x20   0x30    0x40    -      0x50     -    */
    /* 800  0x22   0x32    0x42    -      0x52     -    */
    /*1024  0x24   0x34    0x44    -      0x54     -    */
    /*1152  0x27   0x37    0x47    -      0x57     -    */
    /*1280  0x28   0x38    0x49    -        -      -    */
    /*1600  0x2C   0x3C    0x4C   0x5D      -      -    */
    /*This value is only for BIOS.... */
    int videoMode = 0;

    switch (vgaBitsPerPixel) {
    case 4:
	videoMode = 0x20;
	break;
    case 8:
	videoMode = 0x30;
	break;
    case 16:
	videoMode = 0x40;
	if (weightGreen != 5)
	    videoMode |= 0x01;
	break;
    default:
	videoMode = 0x50;
	break;
    }

    switch (displaySize) {
    case 800:
	videoMode |= 0x02;
	break;
    case 1024:
	videoMode |= 0x04;
	break;
    case 1152:
	videoMode |= 0x07;
	break;
    case 1280:
	videoMode |= 0x08;
	if (vgaBitsPerPixel == 16)
	    videoMode |= 0x01;
	break;
    case 1600:
	videoMode |= 0x0C;
	if (vgaBitsPerPixel == 16)
	    videoMode |= 0x01;
	break;
    }

    return videoMode;
}

#if defined(DEBUG) && defined(CT_HW_DEBUG)
void ctHWDebug(addr)
     int addr;
{
#if CT_DEBUG_WAIT
    usleep(CT_DEBUG_WAIT);
#endif
    ErrorF("Register/Address: %X\n",addr);
}
#endif

static void ctScaleClock(enter,clock)
Bool enter;
int *clock;
{
    /* 
     * This is needed to scale programmable and fixed clocks to their
     * real value. Clocks from a clock line are handled differently
     * from the programmable clocks where text clock don't need rescaling.
     * Using a clock line for programmable clocks does not really make 
     * sense but as long as the higher level drivers need this we have to 
     * do a pretty bad hack to rescale the textclock too, as a clock setting
     * program needs the frequency - not the number.
     */
    static int saveClock;
    static int saveTextClock;

    if(enter) {
	if (*clock < MAXCLOCKS){
	    /* save clock */
	    saveClock = vga256InfoRec.clock[*clock];
	    /* scale clock */
	    vga256InfoRec.clock[*clock] *= CHIPS.ChipClockScaleFactor;
	} else {
	    /* save clock */
	    saveClock = *clock;
	    /* scale clock */
	    *clock *= CHIPS.ChipClockScaleFactor;
	}
    } else {
	if (*clock < MAXCLOCKS)
	    /* restore clock */
	    vga256InfoRec.clock[*clock] = saveClock;
	else 
	    /* restore clock */
	    *clock = saveClock; 
    }
}

void
CHIPSSaveScreen(start)
     Bool start;
{ 
  /* Differences to original: reset h-counter of sequencer */

  static char counter = 0;
  unsigned char tmp;
  
#ifdef DEBUG
  ErrorF("CHIPSSaveScreen\n");
#endif
  
  /* fix things that could be messed up by suspend/resume */
  if(!ctisHiQV32)
    outb(0x3D6,0x15);

  if (start == SS_START) {
    
    /* reset horizontal counter */
    outw(0x3C4,0x07);
    
    /* synchronous reset - stop counters */
    outw(0x3C4, 0x0100);        
#ifdef DEBUG
    ErrorF("Seq. reset on\n");
#endif
  } else {
    /* reset hcounter again !don't fiddle with this! */
    outw(0x3C4,0x07);
    
    /* end reset - start counters */
    outw(0x3C4, 0x0300); 
#ifdef DEBUG
    ErrorF("Seq. reset off\n");
#endif
  }
}

