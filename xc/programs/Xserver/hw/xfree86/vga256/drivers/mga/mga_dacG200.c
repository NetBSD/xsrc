/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mga_dacG200.c,v 1.1.2.17 1999/11/18 15:37:31 hohndel Exp $ */
/*
 * Millennium G200 RAMDAC driver
 */

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86cursor.h"
#include "vga.h"
#include "vgaPCI.h"

#include "mga_reg.h"
#include "mga_bios.h"
#include "mga.h"
#include "xf86_Config.h"

/*
 * exported functions
 */
void MGAG200RamdacInit();
Bool MGAG200Init();
void MGAG200Restore();
void *MGAG200Save();

/*
 * implementation
 */
 
/*
 * This is a convenience macro, so that entries in the driver structure
 * can simply be dereferenced with 'newVS->xxx'.
 * change ajv - new conflicts with the C++ reserved word.
 */
#define newVS ((vgaMGAPtr)vgaNewVideoState)

typedef struct {
	vgaHWRec std;                           /* good old IBM VGA */
	unsigned long DAClong;
	unsigned char DACreg[0x50];
	unsigned char ExtVga[6];
} vgaMGARec, *vgaMGAPtr;
    
/*
 * Read/write to the DAC via MMIO 
 */

/*
 * direct registers
 */
static unsigned char inMGAdreg(reg)
unsigned char reg;
{
	if (!MGAMMIOBase)
		FatalError("MGA: IO registers not mapped\n");

	return INREG8(RAMDAC_OFFSET + reg);
}

static void outMGAdreg(reg, val)
unsigned char reg, val;
{
	if (!MGAMMIOBase)
		FatalError("MGA: IO registers not mapped\n");

	OUTREG8(RAMDAC_OFFSET + reg, val);
}

/*
 * indirect registers
 */
static void outMGAdac(reg, val)
unsigned char reg, val;
{
	outMGAdreg(MGA1064_INDEX, reg);
	outMGAdreg(MGA1064_DATA, val);
}

static unsigned char inMGAdac(reg)
unsigned char reg;
{
	outMGAdreg(MGA1064_INDEX, reg);
	return inMGAdreg(MGA1064_DATA);
}

/*
 * MGACalcClock - Calculate the PLL settings (m, n, p, s).
 *
 * DESCRIPTION
 *   For more information, refer to the Matrox
 *   "MGA1064SG Developer Specification (document 10524-MS-0100).
 *     chapter 5.7.8. "PLLs Clocks Generators"
 *
 * PARAMETERS
 *   f_out		IN	Desired clock frequency.
 *   f_max		IN	Maximum allowed clock frequency.
 *   m			OUT	Value of PLL 'm' register.
 *   n			OUT	Value of PLL 'n' register.
 *   p			OUT	Value of PLL 'p' register.
 *   s			OUT	Value of PLL 's' filter register 
 *                              (pix pll clock only).
 *
 * HISTORY
 *   August 18, 1998 - Radoslaw Kapitan
 *   Adapted for G200 DAC
 *
 *   February 28, 1997 - Guy DESBIEF 
 *   Adapted for MGA1064SG DAC.
 *   based on MGACalcClock  written by [aem] Andrew E. Mileski
 */

/* The following values are in kHz */
/* they came from guess, need to be checked with doc !!!!!!!! */
#define MGA_MIN_VCO_FREQ    120000
#define MGA_MAX_VCO_FREQ    250000
#define MGA_MAX_PCLK_FREQ   250000
#define MGA_MAX_MCLK_FREQ   100000
#define MGA_REF_FREQ        27050.0
#define MGA_ALT_REF_FREQ    14318.0
#define MGA_FEED_DIV_MIN    8
#define MGA_FEED_DIV_MAX    127
#define MGA_IN_DIV_MIN      1
#define MGA_IN_DIV_MAX      6
#define MGA_POST_DIV_MIN    0
#define MGA_POST_DIV_MAX    3


static double
MGACalcClock ( f_out, f_max, m, n, p, s )
	long f_out;
	long f_max;
	int *m;
	int *n;
	int *p;
	int *s;
{
	int best_m, best_n;
	double f_pll, f_vco;
	double m_err, inc_m, calc_f, f_out_f,base_freq;
	static double ref = 0.0;

	if (ref < 1.0) 
	{
		if (MGABios2.PinID && (MGABios2.VidCtrl & 0x20))
			ref = MGA_ALT_REF_FREQ;
		else
			ref = MGA_REF_FREQ;
		ErrorF("%s %s: PLL reference freq: %.3f MHz\n",
		       XCONFIG_PROBED, vga256InfoRec.name, ref / 1000.0);
	}
	/* Make sure that f_min <= f_out <= f_max */

	if ( f_out < ( MGA_MIN_VCO_FREQ / 8))
		f_out = MGA_MIN_VCO_FREQ / 8;

	if ( f_out > f_max )
		f_out = f_max;

	/*
	 * f_pll = f_vco /  (2^p)
	 * Choose p so that MGA_MIN_VCO_FREQ   <= f_vco <= MGA_MAX_VCO_FREQ  
	 * we don't have to bother checking for this maximum limit.
	 */
	f_vco = ( double ) f_out;
	for ( *p = 0; *p < MGA_POST_DIV_MAX && f_vco < MGA_MIN_VCO_FREQ;
								( *p )++ )
		f_vco *= 2.0;

	/* Initial value of calc_f for the loop */
	calc_f = 0;

	base_freq = ref / ( 1 << *p );

	/* Initial amount of error for frequency maximum */
	m_err = f_out;

	/* Search for the different values of ( *m ) */
	for ( *m = MGA_IN_DIV_MIN ;
		*m < MGA_IN_DIV_MAX ; ( *m )++ )
	{
		/* see values of ( *n ) which we can't use */
		for ( *n = MGA_FEED_DIV_MIN;
			*n <= MGA_FEED_DIV_MAX; ( *n )++ )
		{ 
			calc_f = (base_freq * (*n)) / *m ;

		/*
		 * Pick the closest frequency.
		 */
			if (abs( calc_f - f_out ) < m_err ) {
				m_err = abs(calc_f - f_out);
				best_m = *m;
				best_n = *n;
			}
		}
	}
	
	/* Now all the calculations can be completed */
	f_vco = ref * best_n / best_m;

	/* Adjustments for filtering pll feed back */
	if ( (50000.0 <= f_vco)
	&& (f_vco < 100000.0) )
		*s = 0;	
	if ( (100000.0 <= f_vco)
	&& (f_vco < 140000.0) )
		*s = 1;	
	if ( (140000.0 <= f_vco)
	&& (f_vco < 180000.0) )
		*s = 2;	
	if ( (180000.0 <= f_vco)
	&& (f_vco < 220000.0) )
		*s = 3;	

	f_pll = f_vco / ( 1 << *p );

	*m = best_m - 1;
	*n = best_n - 1;
	*p = ( 1 << *p ) - 1 ; 

#ifdef DEBUG
	ErrorF( "f_out_requ =%ld f_pll_real=%.1f f_vco=%.1f n=0x%x m=0x%x p=0x%x s=0x%x\n",
		f_out, f_pll, f_vco, *n, *m, *p, *s );
#endif

	return f_pll;
}


/*
 * MGASetPCLK - Set the pixel (PCLK) and loop (LCLK) clocks.
 *
 * PARAMETERS
 *   f_pll			IN	Pixel clock PLL frequencly in kHz.
 */
static void 
MGASetPCLK( f_out )
	long	f_out;
{
	/* Pixel clock values */
	int m, n, p, s;

	/* The actual frequency output by the clock */
	double f_pll;
	long f_max;

	/* Get the maximum pixel clock frequency from the BIOS, 
         * or from a reasonable default
         */
	if ( MGABios2.PinID && MGABios2.PclkMax != 0xff )
		f_max = (MGABios2.PclkMax+100) * 1000; /* [ajv - scale it] */
	else
		f_max = MGA_MAX_PCLK_FREQ;

	if ( vga256InfoRec.maxClock < f_max )
		f_max = vga256InfoRec.maxClock;

	/* Do the calculations for m, n, and p */
	f_pll = MGACalcClock( f_out, f_max, &m, &n, &p , &s);

	/* Values for the pixel clock PLL registers */
	newVS->DACreg[ MGA1064_PIX_PLLC_M ] = ( m & 0x1f );
	newVS->DACreg[ MGA1064_PIX_PLLC_N ] = ( n & 0x7f );
	newVS->DACreg[ MGA1064_PIX_PLLC_P ] = ( p & 0x07 | ((s & 0x3) << 3) );
}

/*
 * MGAG200Init
 *
 * The 'mode' parameter describes the video mode.	The 'mode' structure 
 * as well as the 'vga256InfoRec' structure can be dereferenced for
 * information that is needed to initialize the mode.	The 'new' macro
 * (see definition above) is used to simply fill in the structure.
 */
Bool
MGAG200Init(mode)
DisplayModePtr mode;
{
	/*
	 * initial values of the DAC registers
	 */
	static unsigned char initDAC[] = {
	/* 0x00: */	   0,    0,    0,    0,    0,    0, 0x00,    0,
	/* 0x08: */	   0,    0,    0,    0,    0,    0,    0,    0,
	/* 0x08: */	   0,    0,    0,    0,    0,    0,    0,    0,
	/* 0x18: */	0x03,    0, 0x09, 0xFF, 0xBF, 0x20, 0x1F, 0x20,
	/* 0x20: */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 0x28: */	0x00, 0x00, 0x00, 0x77, 0x04, 0x2D, 0x19, 0x40,
	/* 0x30: */	0x00, 0xB0, 0x00, 0xC2, 0x34, 0x14, 0x02, 0x83,
	/* 0x38: */	0x00, 0x93, 0x00, 0x77, 0x71, 0xDB, 0x02, 0x3A,
	/* 0x40: */	   0,    0,    0,    0,    0,    0,    0,    0,
	/* 0x48: */	   0,    0,    0,    0,    0,    0,    0,    0
	};

	int hd, hs, he, ht, vd, vs, ve, vt, wd;
	int i;
	int weight555 = FALSE;
	CARD32 orig;

	switch(vgaBitsPerPixel)
	{
	case 8:
		initDAC[ MGA1064_MUL_CTL ] = MGA1064_MUL_CTL_8bits;
		break;
	case 16:
		initDAC[ MGA1064_MUL_CTL ] = MGA1064_MUL_CTL_16bits;
		if ( (xf86weight.red == 5) && (xf86weight.green == 5)
					&& (xf86weight.blue == 5) ) {
			weight555 = TRUE;
			initDAC[ MGA1064_MUL_CTL ] = MGA1064_MUL_CTL_15bits;
		}
		break;
	case 24:
		initDAC[ MGA1064_MUL_CTL ] = MGA1064_MUL_CTL_24bits;
		break;
	case 32:
		initDAC[ MGA1064_MUL_CTL ] = MGA1064_MUL_CTL_32_24bits;
		break;
	default:
		FatalError("MGA: unsupported depth\n");
	}
		
	/*
	 * This will allocate the datastructure and initialize all of the
	 * generic VGA registers.
	 */
	if (!vgaHWInit(mode,sizeof(vgaMGARec)))
		return(FALSE);

	/*
	 * Here all of the other fields of 'newVS' get filled in.
	 */
	hd = (mode->CrtcHDisplay	>> 3)	- 1;
	hs = (mode->CrtcHSyncStart	>> 3)	- 1;
	he = (mode->CrtcHSyncEnd	>> 3)	- 1;
	ht = (mode->CrtcHTotal		>> 3)	- 1;
	vd = mode->CrtcVDisplay			- 1;
	vs = mode->CrtcVSyncStart		- 1;
	ve = mode->CrtcVSyncEnd			- 1;
	vt = mode->CrtcVTotal			- 2;
	
	/* HTOTAL & 0xF equal to 0xE in 8bpp or 0x4 in 24bpp causes strange
	 * vertical stripes
	 */  
	if((ht & 0x0F) == 0x0E || (ht & 0x0F) == 0x04)
		ht++;
		
	if (vgaBitsPerPixel == 24)
		wd = (vga256InfoRec.displayWidth * 3) >> (4 - MGABppShft);
	else
		wd = vga256InfoRec.displayWidth >> (4 - MGABppShft);

	newVS->ExtVga[0] = 0;
	newVS->ExtVga[5] = 0;
	
	if (mode->Flags & V_INTERLACE)
	{
		newVS->ExtVga[0] = 0x80;
		newVS->ExtVga[5] = (hs + he - ht) >> 1;
		wd <<= 1;
		vt &= 0xFFFE;
	}

	newVS->ExtVga[0]	|= (wd & 0x300) >> 4;
	newVS->ExtVga[1]	= (((ht - 4) & 0x100) >> 8) |
				((hd & 0x100) >> 7) |
				((hs & 0x100) >> 6) |
				(ht & 0x40);
	newVS->ExtVga[2]	= ((vt & 0x400) >> 10) |
				((vt & 0x800) >> 10) |
				((vd & 0x400) >> 8) |
				((vd & 0x400) >> 7) |
				((vd & 0x800) >> 7) |
				((vs & 0x400) >> 5) |
				((vs & 0x800) >> 5);
	if (vgaBitsPerPixel == 24)
		newVS->ExtVga[3]	= (((1 << MGABppShft) * 3) - 1) | 0x80;
	else
		newVS->ExtVga[3]	= ((1 << MGABppShft) - 1) | 0x80;

	newVS->ExtVga[3] &= 0xE7;	/* ajv - bits 4-5 MUST be 0 or bad karma happens */

	newVS->ExtVga[4]	= 0;
		
	newVS->std.CRTC[0]	= ht - 4;
	newVS->std.CRTC[1]	= hd;
	newVS->std.CRTC[2]	= hd;
	newVS->std.CRTC[3]	= (ht & 0x1F) | 0x80;
	newVS->std.CRTC[4]	= hs;
	newVS->std.CRTC[5]	= ((ht & 0x20) << 2) | (he & 0x1F);
	newVS->std.CRTC[6]	= vt & 0xFF;
	newVS->std.CRTC[7]	= ((vt & 0x100) >> 8 ) |
				((vd & 0x100) >> 7 ) |
				((vs & 0x100) >> 6 ) |
				((vd & 0x100) >> 5 ) |
				0x10 |
				((vt & 0x200) >> 4 ) |
				((vd & 0x200) >> 3 ) |
				((vs & 0x200) >> 2 );
	newVS->std.CRTC[9]	= ((vd & 0x200) >> 4) | 0x40; 
	newVS->std.CRTC[16] = vs & 0xFF;
	newVS->std.CRTC[17] = (ve & 0x0F) | 0x20;
	newVS->std.CRTC[18] = vd & 0xFF;
	newVS->std.CRTC[19] = wd & 0xFF;
	newVS->std.CRTC[21] = vd & 0xFF;
	newVS->std.CRTC[22] = (vt + 1) & 0xFF;

	if (mode->Flags & V_DBLSCAN)
		newVS->std.CRTC[9] |= 0x80;
    
	newVS->std.MiscOutReg |= 0x0C;

	orig = pciReadLong(MGAPciTag, PCI_OPTION_REG) & (0x17 << 10);
	if (MGA_IS_G200(MGAchipset) || MGA_IS_G400(MGAchipset)) {
	    /* we want to leave the hardpwmsk 
	       and memconfig bits alone, in case this is an SDRAM card */
	    newVS->DAClong = 0x40078121 | orig; 
	}
	else if (MGA_IS_G100(MGAchipset)) {
	    newVS->DAClong = 0x400781A9 | orig;
	}
	for (i = 0; i < sizeof(initDAC); i++)
	{
	    newVS->DACreg[i] = initDAC[i]; 
	}
	if (OFLG_ISSET(OPTION_SYNC_ON_GREEN, &vga256InfoRec.options)) {
		ErrorF("%s %s: sync on green\n",
		    XCONFIG_GIVEN, vga256InfoRec.name);
		newVS->DACreg[MGA1064_GEN_CTL] &= ~0x20;
		newVS->ExtVga[3] |= 0x40;
	}

        /* Have to leave retries on.  Artifacts without it. */
	newVS->DAClong &= ~(1 << 29);

	MGASetPCLK( vga256InfoRec.clock[newVS->std.NoClock] );

	/*
	 * init palette for palettized depths
	 */
	for(i = 0; i < 256; i++) {
		switch(vgaBitsPerPixel) 
		{
		case 16:
			newVS->std.DAC[i * 3 + 0] = i << 3;
			newVS->std.DAC[i * 3 + 1] = i << (weight555 ? 3 : 2);
			newVS->std.DAC[i * 3 + 2] = i << 3;
			break;
		case 24:
		case 32:
			newVS->std.DAC[i * 3 + 0] = i;
			newVS->std.DAC[i * 3 + 1] = i;
			newVS->std.DAC[i * 3 + 2] = i;
			break;
		}
	}
		
#ifdef DEBUG		
	ErrorF("MGAInit: Inforec pixclk=");
	ErrorF("%6ld pixclk: m=%02X n=%02X p=%02X\n",
		vga256InfoRec.clock[newVS->std.NoClock]);
	ErrorF("DAClong: %08lX\n",newVS->DAClong);

	ErrorF("NewVS ->DACregs:\n");

	for(i=0; i<sizeof(newVS->DACreg); i++) {
		ErrorF("%02X->%02X ",i,newVS->DACreg[i]);
		if ((i % 8) == 7 )
			ErrorF("\n");
	}
	ErrorF("\n");
	ErrorF("Physical DACregs\n");
	for(i=0; i<sizeof(newVS->DACreg); i++) {
		ErrorF("%02X->%02X ",i,inMGAdac(i));
		if ((i % 8) == 7 )
			ErrorF("\n");
	}
	ErrorF("\nExtVgaRegs:");
	for (i=0; i<6; i++) ErrorF(" %02X", newVS->ExtVga[i]);
	ErrorF("\n");
#endif

	/* Set adress of cursor image */
	newVS->DACreg[4] = (vga256InfoRec.videoRam-1) & 0xFF;
	newVS->DACreg[5] = (vga256InfoRec.videoRam-1) >> 8;

	return(TRUE);
}

/*
 * MGAG200Restore
 *
 * This function restores a video mode.	 It basically writes out all of
 * the registers that have previously been saved in the vgaMGARec data 
 * structure.
 */
void 
MGAG200Restore(restore)
vgaMGAPtr restore;
{
	int i;

	/*
	 * Code is needed to get things back to bank zero.
	 */

	/* restore DAC regs */
	for (i = 0; i < 0x50; i++)
	    if (i < MGA1064_SYS_PLL_M || i > MGA1064_SYS_PLL_STAT)
		outMGAdac(i, restore->DACreg[i]);

	pciWriteLong(MGAPciTag, PCI_OPTION_REG, restore->DAClong );

	/* restore CRTCEXT regs */
	for (i = 0; i < 6; i++)
		outw(0x3DE, (restore->ExtVga[i] << 8) | i);

	/*
	 * This function handles restoring the generic VGA registers.
	 */
	vgaHWRestore((vgaHWPtr)restore);

	/*
	 * this is needed to properly restore start address (???)
	 */
	outw(0x3DE, (restore->ExtVga[0] << 8) | 0);
}


/*
 * MGAG200Save --
 *
 * This function saves the video state.	 It reads all of the SVGA registers
 * into the vgaMGARec data structure.
 */
void *
MGAG200Save(save)
vgaMGAPtr save;
{
	int i;
	static Bool firstTime = TRUE;
	
	/*
	 * Code is needed to get back to bank zero.
	 */
	outw(0x3DE, 0x0004);
	
	/*
	 * This function will handle creating the data structure and filling
	 * in the generic VGA portion.
	 */
	save = (vgaMGAPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaMGARec));

	/*
	 * The port I/O code necessary to read in the extended registers 
	 * into the fields of the vgaMGARec structure.
	 */
	for (i = 0; i < 0x50; i++)
		save->DACreg[i]	 = inMGAdac(i);

	save->DAClong = pciReadLong(MGAPciTag, PCI_OPTION_REG);
	if (firstTime && xf86Verbose)
	    ErrorF("%s %s: Read OPTION 0x%08x\n",
		XCONFIG_PROBED, vga256InfoRec.name,save->DAClong);

	firstTime = FALSE;

	for (i = 0; i < 6; i++) {
		outb(0x3DE, i);
		save->ExtVga[i] = inb(0x3DF);
	}
	
	return((void *) save);
}

void
MGAG200RamdacInit()
{
	MGAdac.isHwCursor        = TRUE;
	MGAdac.CursorMaxWidth    = 64;
	MGAdac.CursorMaxHeight   = 64;
	MGAdac.SetCursorColors   = MGA1064SetCursorColors;
	MGAdac.SetCursorPosition = MGA1064SetCursorPosition;
	MGAdac.LoadCursorImage   = MGA1064LoadCursorImage;
	MGAdac.HideCursor        = MGA1064HideCursor;
	MGAdac.ShowCursor        = MGA1064ShowCursor;
	MGAdac.CursorFlags       = USE_HARDWARE_CURSOR |
	                           HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
	                           HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
	                           HARDWARE_CURSOR_PROGRAMMED_ORIGIN |
	                           HARDWARE_CURSOR_CHAR_BIT_FORMAT |
				   HARDWARE_CURSOR_SWAP_SOURCE_AND_MASK |
	                           HARDWARE_CURSOR_PROGRAMMED_BITS;

        if(MGA_IS_G400(MGAchipset))
	    MGAdac.maxPixelClock = 300000;
	else
	    MGAdac.maxPixelClock = 250000;
}
