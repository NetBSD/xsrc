/*
 * MGA Millennium (MGA2064W) with Ti3026 RAMDAC driver v.1.1
 *
 * The driver is written without any chip documentation. All extended ports
 * and registers come from tracing the VESA-ROM functions.
 * The BitBlt Engine comes from tracing the windows BitBlt function.
 *
 * Author:	Radoslaw Kapitan, Tarnow, Poland
 *			kapitan@student.uci.agh.edu.pl
 *		original source
 *
 * Now that MATROX has released documentation to the public, enhancing
 * this driver has become much easier. Nevertheless, this work continues
 * to be based on Radoslaw's original source
 *
 * Contributors:
 *		Andrew Vanderstock, Melbourne, Australia
 *			vanderaj@mail2.svhm.org.au
 *		additions, corrections, cleanups
 *
 *		Dirk Hohndel
 *			hohndel@XFree86.Org
 *		integrated into XFree86-3.1.2Gg
 *		fixed some problems with PCI probing and mapping
 *
 *		David Dawes
 *			dawes@XFree86.Org
 *		some cleanups, and fixed some problems
 *
 *		Andrew E. Mileski
 *			aem@ott.hookup.net
 *		RAMDAC timing, and BIOS stuff
 */
 
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mgadriver.c,v 3.10 1996/10/23 13:10:33 dawes Exp $ */

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
#include "vgaPCI.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "vga256.h"
#include "mipointer.h"

#include "mgabios.h"
#include "mgareg.h"
#include "mga.h"

extern GCOps cfb16TEOps1Rect, cfb16TEOps, cfb16NonTEOps1Rect, cfb16NonTEOps;
extern GCOps cfb24TEOps1Rect, cfb24TEOps, cfb24NonTEOps1Rect, cfb24NonTEOps;
extern GCOps cfb32TEOps1Rect, cfb32TEOps, cfb32NonTEOps1Rect, cfb32NonTEOps;
extern vgaHWCursorRec vgaHWCursor;
extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern vgaPCIInformation *vgaPCIInfo;

/*
 * Driver data structures.
 */
unsigned long MGAMMIOAddr = 0;
unsigned char* MGAMMIOBase = NULL;
MGABiosInfo MGABios;
extern int MGAScrnWidth;
static pciTagRec MGAPciTag;
static int MGABppShft;
static int MGADAClong;
static unsigned char* MGAInitDAC;

static unsigned char MGADACregs[] = {
	0x0F, 0x18, 0x19, 0x1A, 0x1C, 0x1D, 0x1E, 0x2A, 0x2B, 0x30,
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A
};

static unsigned char MGADACbpp8[] = {
	0x06, 0x80,    0, 0x25, 0x00, 0x00, 0x00, 0x00, 0x1E, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,    0, 0x00
};

static unsigned char MGADACbpp16[] = {
	0x07, 0x05,    0, 0x15, 0x00, 0x00, 0x20, 0x00, 0x1E, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,	   0, 0x00
};

static unsigned char MGADACbpp24[] = {
	0x07, 0x16,    0, 0x25, 0x00, 0x00, 0x20, 0x00, 0x1E, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,	   0, 0x00
};

static unsigned char MGADACbpp32[] = {
	0x07, 0x06,    0, 0x05, 0x00, 0x00, 0x20, 0x00, 0x1E, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,	   0, 0x00
};

typedef struct {
	vgaHWRec std;				/* good old IBM VGA */
	unsigned long DAClong;
	unsigned char DACclk[6];
	unsigned char DACreg[sizeof(MGADACregs)];
	unsigned char ExtVga[6];
} vgaMGARec, *vgaMGAPtr;

/*
 * Forward definitions for the functions that make up the driver.
 */

static Bool		MGAProbe();
static char *		MGAIdent();
static void		MGAEnterLeave();
static Bool		MGAInit();
static Bool		MGAValidMode();
static void *		MGASave();
static void		MGARestore();
static void		MGAAdjust();
static void		MGAFbInit();
static void 		MGACursorInit();
static int		MGAPitchAdjust();
static int		MGAScrnInit();

extern void		MGASetRead();
extern void		MGASetWrite();
extern void		MGASetReadWrite();
extern void		MGABlitterInit();
extern void		MGADoBitbltCopy();
extern RegionPtr	MGA16CopyArea();
extern RegionPtr	MGA24CopyArea();
extern RegionPtr	MGA32CopyArea();

/*
 * This data structure defines the driver itself.
 */
vgaVideoChipRec MGA = {
	/* 
	 * Function pointers
	 */
	MGAProbe,
	MGAIdent,
	MGAEnterLeave,
	MGAInit,
	MGAValidMode,
	MGASave,
	MGARestore,
	MGAAdjust,
	vgaHWSaveScreen,
	(void (*)())NoopDDA,	/* MGAGetMode, */
	MGAFbInit,
	MGASetRead, 
	MGASetWrite,
	MGASetReadWrite,
	/*
	 * This is the size of the mapped memory window, usually 64k.
	 */
	0x10000,		
	/*
	 * This is the size of a video memory bank for this chipset.
	 */
	0x10000,
	/*
	 * This is the number of bits by which an address is shifted
	 * right to determine the bank number for that address.
	 */
	16,
	/*
	 * This is the bitmask used to determine the address within a
	 * specific bank.
	 */
	0xFFFF,
	/*
	 * These are the bottom and top addresses for reads inside a
	 * given bank.
	 */
	0x00000, 0x10000,
	/*
	 * And corresponding limits for writes.
	 */
	0x00000, 0x10000,
	/*
	 * Whether this chipset supports a single bank register or
	 * seperate read and write bank registers.	Almost all chipsets
	 * support two banks, and two banks are almost always faster
	 */
	FALSE,
	/*
	 * The chipset requires vertical timing numbers to be divided
	 * by two for interlaced modes
	 */
	VGA_DIVIDE_VERT,
	/*
	 * This is a dummy initialization for the set of option flags
	 * that this driver supports.	It gets filled in properly in the
	 * probe function, if the probe succeeds (assuming the driver
	 * supports any such flags).
	 */
	{0,},
	/*
	 * This determines the multiple to which the virtual width of
	 * the display must be rounded for the 256-color server. 
	 */
	0,
	/*
	 * If the driver includes support for a linear-mapped frame buffer
	 * for the detected configuratio this should be set to TRUE in the
	 * Probe or FbInit function. 
	 */
	FALSE,
	/*
	 * This is the physical base address of the linear-mapped frame
	 * buffer (when used).	Set it to 0 when not in use.
	 */
	0,
	/*
	 * This is the size	 of the linear-mapped frame buffer (when used).
	 * Set it to 0 when not in use.
	 */
	0x00800000,
	/*
	 * This is TRUE if the driver has support for 16bpp for the detected
	 * configuration. It must be set in the Probe function.
	 * It most cases it should be FALSE.
	 */
	TRUE,
	/*
	 * This is TRUE if the driver has support for 24bpp for the detected
	 * configuration.
	 */
	TRUE,
	/*
	 * This is TRUE if the driver has support for 32bpp for the detected
	 * configuration.
	 */
	TRUE,
	/*
	 * This is a pointer to a list of builtin driver modes.
	 * This is rarely used, and in must cases, set it to NULL
	 */
	NULL,
	/*
	 * This is a factor that can be used to scale the raw clocks
	 * to pixel clocks.	 This is rarely used, and in most cases, set
	 * it to 1.
	 */
	1,
};

/*
 * This is a convenience macro, so that entries in the driver structure
 * can simply be dereferenced with 'newVS->xxx'.
 * change ajv - new conflicts with the C++ reserved word. 
 */
#define newVS ((vgaMGAPtr)vgaNewVideoState)


/*
 * array of ports
 */ 
static unsigned mgaExtPorts[] =
{
	0x400			/* This is enough to enable all ports */
};

static int Num_mgaExtPorts =
	(sizeof(mgaExtPorts)/sizeof(mgaExtPorts[0]));

/*
 * MGAReadBios - Read the video BIOS info block.
 *
 * DESCRIPTION
 *   Warning! This code currently does not detect a video BIOS.
 *   In the future, support for motherboards with the mga2064w
 *   will be added (no video BIOS) - this is not a huge concern
 *   for me today though.
 *
 * EXTERNAL REFERENCES
 *   vga256InfoRec.BIOSbase	IN	Physical address of video BIOS.
 *   MGABios			OUT	The video BIOS info block.
 *
 * HISTORY
 *   October 7, 1996 - [aem] Andrew E. Mileski
 *   Written and tested.
 */ 
static void
MGAReadBios()
{
	CARD8 tmp[ 64 ];
	CARD16 offset;
	int i;

	/* Make sure the BIOS is present */
	xf86ReadBIOS( vga256InfoRec.BIOSbase, 0, tmp, sizeof( tmp ));
	if (
		tmp[ 0 ] != 0x55
		|| tmp[ 1 ] != 0xaa
		|| strncmp(( char * )( tmp + 45 ), "MATROX", 6 )
	) {
		ErrorF( "%s %s: Video BIOS info block not detected!" );
		return;
	}

	/* Get the info block offset */
	xf86ReadBIOS( vga256InfoRec.BIOSbase, 0x7ffc,
		( CARD8 * ) & offset, sizeof( offset ));

	/* Copy the info block */
	xf86ReadBIOS( vga256InfoRec.BIOSbase, offset,
		( CARD8 * ) & MGABios.StructLen, sizeof( MGABios ));

	/* Let the world know what we are up to */
	ErrorF( "%s %s: Video BIOS info block at 0x%08lx\n",
		XCONFIG_PROBED, vga256InfoRec.name,
		vga256InfoRec.BIOSbase + offset );	
}

/*
 * Read/write to the DAC.  This includes both MMIO and PCI config space
 * methods of accessing the DAC.
 */

#if 0      /* RK - It doesn't work with PCI config space */

void
MGADacWriteByte(reg, val)
	CARD8 reg;
	CARD8 val;
{
	if (MGAMMIOBase)
	{
		*(volatile CARD8 *) (MGAMMIOBase + RAMDAC_OFFSET + reg) = val;
	}
	else
	{
/* Which is correct ?? */
#if 0
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		pciWriteByte(MGAPciTag, PCI_MGA_DATA, val);
#else
		CARD8 offset = reg % 4;
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		pciWriteByte(MGAPciTag, PCI_MGA_DATA + offset, val);
#endif
	}
}

void
MGADacWriteWord(reg, val)
	CARD8 reg;
	CARD16 val;
{
	if (MGAMMIOBase)
	{
		*(volatile CARD16 *) (MGAMMIOBase + RAMDAC_OFFSET + reg) = val;
	}
	else
	{
/* Which is correct ?? */
#if 0
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		pciWriteWord(MGAPciTag, PCI_MGA_DATA, val);
#else
		CARD8 offset = reg % 4;
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		pciWriteWord(MGAPciTag, PCI_MGA_DATA + offset, val);
#endif
	}
}

void
MGADacWriteLong(reg, val)
	CARD8 reg;
	CARD32 val;
{
	if (MGAMMIOBase)
	{
		*(volatile CARD32 *) (MGAMMIOBase + RAMDAC_OFFSET + reg) = val;
	}
	else
	{
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		pciWriteLong(MGAPciTag, PCI_MGA_DATA, val);
	}
}

CARD8
MGADacReadByte(reg)
	CARD8 reg;
{
	if (MGAMMIOBase)
	{
		return *(volatile CARD8 *) (MGAMMIOBase + RAMDAC_OFFSET + reg);
	}
	else
	{
/* Which is correct ?? */
#if 0
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		return pciReadByte(MGAPciTag, PCI_MGA_DATA);
#else
		CARD8 offset = reg % 4;
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		return pciReadByte(MGAPciTag, PCI_MGA_DATA + offset);
#endif
	}
}

CARD16
MGADacReadWord(reg)
	CARD8 reg;
{
	if (MGAMMIOBase)
	{
		return *(volatile CARD16 *) (MGAMMIOBase + RAMDAC_OFFSET + reg);
	}
	else
	{
/* Which is correct ?? */
#if 0
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		return pciReadWord(MGAPciTag, PCI_MGA_DATA);
#else
		CARD8 offset = reg % 4;
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		return pciReadWord(MGAPciTag, PCI_MGA_DATA + offset);
#endif
	}
}

CARD32
MGADacReadLong(reg)
	CARD8 reg;
{
	if (MGAMMIOBase)
	{
		return *(volatile CARD32 *) (MGAMMIOBase + RAMDAC_OFFSET + reg);
	}
	else
	{
		pciWriteWord(MGAPciTag, PCI_MGA_INDEX, RAMDAC_OFFSET + reg);
		return pciReadLong(MGAPciTag, PCI_MGA_DATA);
	}
}

static void
outTi3026(reg, val)
	CARD8 reg, val;
{
	MGADacWriteByte(TVP3026_INDEX, reg);
	MGADacWriteByte(TVP3026_DATA, val);
}

static CARD8
inTi3026(reg)
	CARD8 reg;
{
	MGADacWriteByte(TVP3026_INDEX, reg);
	return MGADacReadByte(TVP3026_DATA);
}

#endif 

static void outTi3026(reg, val)
unsigned char reg, val;
{
	if(MGAMMIOBase)
	{
		MGAREG8(RAMDAC_OFFSET + TVP3026_INDEX) = reg;
		MGAREG8(RAMDAC_OFFSET + TVP3026_DATA) = val;
	}
	else
	{
		outb(0x3C8, reg);    /* RK - PCI metod doesn't work - ??? */
		
                pciWriteWord(MGAPciTag, PCI_MGA_INDEX, 
	                		RAMDAC_OFFSET + TVP3026_DATA);
                pciWriteLong(MGAPciTag, PCI_MGA_DATA, val << 16);
	}
}

static unsigned char inTi3026(reg)
unsigned char reg;
{
	unsigned char val;
	
	if(MGAMMIOBase)
	{
		MGAREG8(RAMDAC_OFFSET + TVP3026_INDEX) = reg;
		val = MGAREG8(RAMDAC_OFFSET + TVP3026_DATA);
	}
	else
	{
		outb(0x3C8, reg);    /* RK - PCI metod doesn't work - ??? */
		
                pciWriteWord(MGAPciTag, PCI_MGA_INDEX, 
	                		RAMDAC_OFFSET + TVP3026_DATA);
                val = pciReadLong(MGAPciTag, PCI_MGA_DATA) >> 16;
	}
	return val;
}

/*
 * MGATi3026SetClock - Set the pixel and loop clock PLLs.
 *
 * DESCRIPTION
 *   For more information, refer to the Texas Instruments
 *   "TVP3026 Data Manual" (document SLAS098B).
 *     Section 2.4.1 "Pixel Clock PLL"
 *     Section 2.4.3 "Loop Clock PLL"
 *     Appendix A "Frequency Synthesis PLL Register Settings"
 *     Appendix B "PLL Programming Examples"
 *
 * PARAMETERS
 *   f_pll			IN	Pixel clock PLL frequencly in kHz.
 *   bpp			IN	Bytes per pixel.
 *
 * EXTERNAL REFERENCES
 *   vga256InfoRec.maxClock	IN	Max allowed pixel clock in kHz.
 *   vgaBitsPerPixel		IN	Bits per pixel.
 *
 * HISTORY
 *   October 19, 1996 - [aem] Andrew E. Mileski
 *   Commented the loop clock code (wow, I understand everything now),
 *   and simplified it a bit. This should really be two separate functions.
 *
 *   October 1, 1996 - [aem] Andrew E. Mileski
 *   Optimized the m & n picking algorithm. Added maxClock detection.
 *   Low speed pixel clock fix (per the docs). Documented what I understand.
 *
 *   ?????, ??, ???? - [???] ????????????
 *   Based on the TVP3026 code in the S3 driver.
 */

/*
 * It is _OKAY_ to have TI_MAX_VCO_FREQ > chip speed,
 * as long as the final output clock f_pll <= chip speed.
 * The VCO drives a divider (which can handle it), but
 * the output clock drives most of the chip.
 * Read the docs very carefully - and trust me :-) [aem]
 *
 * The following values are all in kHz
 */
#define TI_MAX_VCO_FREQ	250000
#define TI_MIN_VCO_FREQ	110000
#define TI_REF_FREQ	14318.18

static void 
MGATi3026SetClock( f_pll, bpp, m24 )
	long	f_pll;
	int	bpp;
{
	/* Pixel clock: f_vco = 8 * TI_REF_FREQ * ( 65 - m ) / ( 65 - n ) */
	double f_vco;
	int n, p, m;

	/* Pixel clock: These are used to pick a value for m */
	double c, ic, m_err;
	int best_n, best_m;

	/* Loop clock values */
	int ln, lp, lm, lq;
	long z;

	/*
	 * First we deal with setting the pixel clock PLL.
	 * We will deal with the loop clock PLL later.
	 */

	/* Make sure that 13.75 MHz <= f_pll <= chip max */
	if ( f_pll < ( TI_MIN_VCO_FREQ / 8 ))
		f_pll = TI_MIN_VCO_FREQ / 8;
	if ( f_pll > vga256InfoRec.maxClock )
		f_pll = vga256InfoRec.maxClock;

	/* Assume a frequency multipler of 1.0 to start */
	f_vco = ( double ) f_pll;

	/*
	 * f_pll = f_vco / 2 ^ p
	 * Choose p so that f_vco >= TI_MIN_VCO_FREQ
	 */
	for ( p = 0; p < 3 && f_vco <= TI_MIN_VCO_FREQ; p++ )
		f_vco *= 2.0;

	/*
	 * We avoid doing multiplications by ( 65 - n ),
	 * and add an increment instead - this keeps any error small.
	 */
	ic = f_vco / ( TI_REF_FREQ * 8.0 );

	/* Initial value of c for the loop */
	c = ic + ic + ic;

	/* Initial amount of error for an integer - impossibly large */
	m_err = 2.0;

	/* Search for the closest INTEGER value of ( 65 - m ) */
	for ( n = 3; n <= 25; n++, c += ic ) {

		/* Ignore values of ( 65 - m ) which we can't use */
		if ( c < 3.0 || c > 64.0 )
			continue;

		/*
		 * Pick the closest INTEGER (has smallest fractional part).
		 * The optimizer should clean this up for us.
		 */
		if (( c - ( int ) c ) < m_err ) {
			m_err = c - ( int ) c;
			best_m = ( int ) c;
			best_n = n;
		}
	}
	
	/* 65 - ( 65 - x ) = x */
	m = 65 - best_m;
	n = 65 - best_n;

	/* Values for the pixel clock PLL registers */
	newVS->DACclk[ 0 ] = ( n & 0x3f ) | 0xC0;
	newVS->DACclk[ 1 ] = ( m & 0x3f );
	newVS->DACclk[ 2 ] = ( p & 0x03 ) | 0xB0;

	/*
	 * Now that the pixel clock PLL is setup,
	 * the loop clock PLL must be setup.
	 */

	/*
	 * First we figure out lm, ln, and z.
	 * Things are different in packed pixel mode (24bpp) though.
	 */
	 if ( vgaBitsPerPixel == 24 ) {

		/* ln:lm = ln:3 */
		lm = 65 - 3;

		/* Check for interleaved mode */
		if ( bpp == 2 )
			/* ln:lm = 4:3 */
			ln = 65 - 4;
		else
			/* ln:lm = 8:3 */
			ln = 65 - 8;

		/* Note: this is actually 100 * z for more precision */
		z = (11000 * (65 - ln)) / ((f_pll / 1000) * (65 - lm));
	}
	else {
		/* ln:lm = ln:4 */
		lm = 65 - 4;

		/* Note: bpp = bytes per pixel */
		ln = 65 - 4 * ( 64 / 8 ) / bpp;

		/* Note: this is actually 100 * z for more precision */
		z = ((11000 / 4) * (65 - ln)) / (f_pll / 1000) ;
	}

	/*
	 * Now we choose dividers lp and lq so that the VCO frequency
	 * is within the operating range of 110 MHz to 220 MHz.
	 */

	/* Assume no lq divider */
	lq = 0;

	/* Note: z is actually 100 * z for more precision */
	if ( z <= 200 )
		lp = 0;
	else if ( z <= 400 )
		lp = 1;
	else if ( z <= 800 )
		lp = 2;
	else if ( z <= 1600 )
		lp = 3;
	else {
		lp = 3;
		lq = z / 1600;
	}
 
	/* Values for the loop clock PLL registers */
	if ( vgaBitsPerPixel == 24 ) {

		/* Packed pixel mode values */
		newVS->DACclk[ 3 ] = ( ln & 0x3f ) | 0x80;
		newVS->DACclk[ 4 ] = ( lm & 0x3f ) | 0x80;
		newVS->DACclk[ 5 ] = ( lp & 0x03 ) | 0xf8;
 	} else {

		/* Non-packed pixel mode values */
		newVS->DACclk[ 3 ] = ( ln & 0x3f ) | 0xc0;
		newVS->DACclk[ 4 ] = ( lm & 0x3f );
		newVS->DACclk[ 5 ] = ( lp & 0x03 ) | 0xf0;
	}
	newVS->DACreg[ 18 ] = lq | 0x38;

#ifdef DEBUG
	ErrorF("bpp %d  ln %2d  lm %2d  lz %4d  lp %2d  lq %2d\n",
		bpp,ln,lm,z,lp,lq);
#endif
}

/*
 * MGACountRAM --
 *
 * Counts amount of installed RAM 
 */
static int
MGACountRam()
{
	if(MGA.ChipLinearBase)
	{
		volatile unsigned char* base;
		unsigned char tmp, tmp3, tmp5;
	
		base = xf86MapVidMem(vga256InfoRec.scrnIndex, LINEAR_REGION,
			(pointer)MGA.ChipLinearBase, MGA.ChipLinearSize);
	
		outb(0x3DE, 3);
		tmp = inb(0x3DF);
		outb(0x3DF, tmp | 0x80);
	
		base[0x500000] = 0x55;
		base[0x300000] = 0x33;
		tmp5 = base[0x500000];
		tmp3 = base[0x300000];

		outb(0x3DE, 3);
		outb(0x3DF, tmp);
	
		xf86UnMapVidMem(vga256InfoRec.scrnIndex, LINEAR_REGION, 
				(pointer)base, MGA.ChipLinearSize);
	
		if(tmp5 == 0x55)
			return 8192;
		if(tmp3 == 0x33)
			return 4096;
	}
	return 2048;
}

/*
 * MGAIdent --
 *
 * Returns the string name for supported chipset 'n'. 
 */
static char *
MGAIdent(n)
int n;
{
	static char *chipsets[] = {"mga2064w"};

	if (n + 1 > sizeof(chipsets) / sizeof(char *))
		return(NULL);
	else
		return(chipsets[n]);
}

/*
 * MGAProbe --
 *
 * This is the function that makes a yes/no decision about whether or not
 * a chipset supported by this driver is present or not. 
 */
static Bool
MGAProbe()
{
	pciConfigPtr pcr = NULL;
	int i;

	/*
	 * First we attempt to figure out if one of the supported chipsets
	 * is present.
	 */
	if (vga256InfoRec.chipset)
		if (StrCaseCmp(vga256InfoRec.chipset, MGAIdent(0)))
			return(FALSE);

	i = 0;
	if (vgaPCIInfo && vgaPCIInfo->AllCards) {
	  while (pcr = vgaPCIInfo->AllCards[i++]) {
		if (pcr->_vendor == PCI_VENDOR_MATROX)
			if (pcr->_device == PCI_CHIP_MGA2064)
				break;
	  }
	} else return(FALSE);
	if (!pcr)
	{
		if (vga256InfoRec.chipset)
			ErrorF("%s %s: MGA: unknown PCI device vendor\n",
				XCONFIG_PROBED, vga256InfoRec.name);
		return(FALSE);
	}

	if ((pcr->_device != 0x0519) && !vga256InfoRec.chipset)
		return(FALSE);

	/*
	 *	OK. It's MGA Millennium (or something pretty close)
	 */
	 
	MGAPciTag = pcibusTag(pcr->_bus, pcr->_cardnum, pcr->_func);

	/* ajv changes to reflect actual values. see sdk pp 3-2. */
	/* these masks just get rid of the crap in the lower bits */
	/* XXX - ajv I'm assuming that pcr->_base0 is pci config space + 0x10 */
	/*				and _base1 is another four bytes on */
	/* XXX - these values are Intel byte order I believe. */
	
	if ( pcr->_base0 )	/* details: mgabase1 sdk pp 4-11 */
		MGAMMIOAddr = pcr->_base0 & 0xffffc000;
	else
		MGAMMIOAddr = 0;
	
	if ( pcr->_base1 )	/* details: mgabase2 sdk pp 4-12 */
		MGA.ChipLinearBase = pcr->_base1 & 0xff800000;
	else
		MGA.ChipLinearBase = 0;
	
	/* Allow this to be overriden in the XF86Config file */
	if (vga256InfoRec.BIOSbase == 0) {
		if ( pcr->_baserom )	/* details: rombase sdk pp 4-15 */
			vga256InfoRec.BIOSbase = pcr->_baserom & 0xffff0000;
		else
			vga256InfoRec.BIOSbase = 0xc0000;
	}

	/*
	 * Read the BIOS data struct
	 */
	MGAReadBios();
	
	/*
	 * Set up I/O ports to be used by this card.
	 */
	xf86ClearIOPortList(vga256InfoRec.scrnIndex);
	xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
	xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_mgaExtPorts,
				mgaExtPorts);

	MGAEnterLeave(ENTER);
	
#ifdef DEBUG
	ErrorF("Config Word %lx\n",pcibusRead(MGAPciTag, 0x40));
	ErrorF("RAMDACRev %x\n", inTi3026(0x01));
#endif

	/*
	 * If the user has specified the amount of memory in the XF86Config
	 * file, we respect that setting.
	 */
	if (!vga256InfoRec.videoRam)
		vga256InfoRec.videoRam = MGACountRam();
	
	/*
	 * If the user has specified ramdac speed in the XF86Config
	 * file, we respect that setting.
	 */
	vga256InfoRec.maxClock = ( vga256InfoRec.dacSpeed ) ?
		vga256InfoRec.dacSpeed :
		((( MGABios.RamdacType & 0xff ) == 1 ) ?  220000 : 175000 );
	
	/*
	 * Last we fill in the remaining data structures. 
	 */
	vga256InfoRec.chipset = MGAIdent(0);
	vga256InfoRec.bankedMono = FALSE;
	
	OFLG_SET(OPTION_NOLINEAR_MODE, &MGA.ChipOptionFlags);
	OFLG_SET(OPTION_NO_BITBLT, &MGA.ChipOptionFlags);
	OFLG_SET(OPTION_NOACCEL, &MGA.ChipOptionFlags);
	
	OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);

	/* Moved width checking because virtualX isn't set until after
	   the probing.  Instead, make use of the newly added
	   PitchAdjust hook. */

	vgaSetPitchAdjustHook(MGAPitchAdjust);

	return(TRUE);
}

/*
 * TestAndSetRounding
 *
 * used in MGAPitchAdjust (see there) - ansi
 */

static int
TestAndSetRounding(pitch)
	int pitch;
{
	int size;

	if (vga256InfoRec.videoRam <= 2048)
		size = 0;
	else
		size = pitch * vga256InfoRec.virtualY / 1024;
		
	if (vgaBitsPerPixel == 32)
	{
		MGAInitDAC = MGADACbpp32;

		if (((pitch % 32) && (size * 4 <= 2048)) || !size)
		{
			MGA.ChipRounding = 16;
			MGABppShft = 3;
			MGADAClong = 0x5F2C0100;    /* non-interleave */
			MGAInitDAC[2] = 0x5B;       /* 32 bits */
		}
		else
                {
			MGA.ChipRounding = 32;
			MGABppShft = 2;
			MGADAClong = 0x5F2C1100;    /* interleave */
			MGAInitDAC[2] = 0x5C;       /* 64 bits */
		}
	}
	if (vgaBitsPerPixel == 24)
	{
		MGAInitDAC = MGADACbpp24;

		if (((pitch % 128) && (size * 3 <= 2048)) || !size)
		{
			MGA.ChipRounding = 64;
			MGABppShft = 1;
			MGADAClong = 0x5F2C0100;    /* non-interleave */
			MGAInitDAC[2] = 0x5B;       /* 32 bits */
		}
		else
                {
			MGA.ChipRounding = 128;
			MGABppShft = 0;
			MGADAClong = 0x5F2C1100;    /* interleave */
			MGAInitDAC[2] = 0x5C;       /* 64 bits */
		}
	}
	if (vgaBitsPerPixel == 16)
	{
		MGAInitDAC = MGADACbpp16;
		
		if (((pitch % 64) && (size * 2 <= 2048)) || !size)
		{
			MGA.ChipRounding = 32;
			MGABppShft = 2;
			MGADAClong = 0x5F2C0100;    /* non-interleave */
			MGAInitDAC[2] = 0x53;       /* 32 bits */
                }
                else
                {
                	MGA.ChipRounding = 64;
                	MGABppShft = 1;
                	MGADAClong = 0x5F2C1100;    /* interleave */
			MGAInitDAC[2] = 0x54;       /* 64 bits */
		}
	}
	if (vgaBitsPerPixel == 8)
	{
		MGAInitDAC = MGADACbpp8;
		
		if (((pitch % 128) && (size <= 2048)) || !size)
		{
			MGA.ChipRounding = 64;
			MGABppShft = 1;
			MGADAClong = 0x5F2C0100;    /* non-interleave */
			MGAInitDAC[2] = 0x4B;       /* 32 bits */
		}
		else
		{
			MGA.ChipRounding = 128;
			MGABppShft = 0;
			MGADAClong = 0x5F2C1100;    /* interleave */
			MGAInitDAC[2] = 0x4C;       /* 64 bits */
		}
	}

	if (pitch % MGA.ChipRounding)
		pitch = pitch + MGA.ChipRounding - (pitch % MGA.ChipRounding);

	return pitch;
}

/*
 * MGAPitchAdjust --
 *
 * This function adjusts the display width (pitch) once the virtual
 * width is known.  It returns the display width.
 */
static int
MGAPitchAdjust()
{
	int pitch = 0;
	int accel;
	
	/* XXX ajv - 512, 576, and 1536 may not be supported
	   virtual resolutions. see sdk pp 4-59 for more
	   details. Why anyone would want less than 640 is 
	   bizarre. (maybe lots of pixels tall?) */

#if 0		
	int width[] = { 512, 576, 640, 768, 800, 960, 
			1024, 1152, 1280, 1536, 1600, 1920, 2048, 0 };
#else
	int width[] = { 640, 768, 800, 960, 1024, 1152, 1280,
			1600, 1920, 2048, 0 };
#endif
	int i;

	if (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options) &&
	    !OFLG_ISSET(OPTION_NO_BITBLT, &vga256InfoRec.options))
	{
		accel = TRUE;
		
		for (i = 0; width[i]; i++)
		{
			if (width[i] >= vga256InfoRec.virtualX && 
			    TestAndSetRounding(width[i]) == width[i])
			{
				pitch = width[i];
				break;
			}
		}
	}
	else
	{
		accel = FALSE;
		pitch = TestAndSetRounding(vga256InfoRec.virtualX);
	}


	if (!pitch)
	{
		if(accel) 
		{
			FatalError("MGA: Can't find pitch, try using option"
				   "\"no_accel\"\n");
		}
		else
		{
			FatalError("MGA: Can't find pitch (Oups, should not"
				   "happen!)\n");
		}
	}

	if (pitch != vga256InfoRec.virtualX)
	{
		if (accel)
		{
			ErrorF("%s %s: Display pitch set to %d (a multiple "
			       "of %d & possible for acceleration)\n",
			       XCONFIG_PROBED, vga256InfoRec.name,
			       pitch, MGA.ChipRounding);
		}
		else
		{
			ErrorF("%s %s: Display pitch set to %d (a multiple "
			       "of %d)\n",
			       XCONFIG_PROBED, vga256InfoRec.name,
			       pitch, MGA.ChipRounding);
		}
	}
#ifdef DEBUG
	else
	{
		ErrorF("%s %s: pitch is %d, virtual x is %d, display width is %d\n", XCONFIG_PROBED, vga256InfoRec.name,
		       pitch, vga256InfoRec.virtualX, vga256InfoRec.displayWidth);
	}
#endif

	return pitch;
}

/*
 * MGAFbInit --
 *
 * This function is used to initialise chip-specific graphics functions.
 * It can be used to make use of the accelerated features of some chipsets.
 */
static void
MGAFbInit()
{
	if (xf86Verbose)
		ErrorF("%s %s: Using TI 3026 programmable clock\n",
			XCONFIG_PROBED, vga256InfoRec.name);
 
	if (OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options))
		OFLG_SET(OPTION_NOACCEL, &vga256InfoRec.options);
	else
	{
		if (vga256InfoRec.MemBase)
			MGA.ChipLinearBase = vga256InfoRec.MemBase;
		if (MGA.ChipLinearBase)
		{
			MGA.ChipUseLinearAddressing = TRUE;
			if (xf86Verbose)
				ErrorF("%s %s: Linear frame buffer at %lX\n", 
					vga256InfoRec.MemBase? XCONFIG_GIVEN : XCONFIG_PROBED,
					vga256InfoRec.name, MGA.ChipLinearBase);
			/* Probe found the MMIO base (or else!) */
#if 0
			MGAMMIOBase = xf86MapVidMem(vga256InfoRec.scrnIndex,
				EXTENDED_REGION,
				(pointer)(MGA.ChipLinearBase + 0x00800000), 0x4000);
			/* XXX ajv - do we still need to map the video memory ? */
#else
			/* I believe that this should map the registers!
			 * therefore the base0 value that is in MGAMMIOBase
			 * is needed...
			 */
			if (MGAMMIOAddr)
			{
				MGAMMIOBase =
				  xf86MapVidMem(vga256InfoRec.scrnIndex,
					MMIO_REGION,
					(pointer)(MGAMMIOAddr), 0x4000);
			}
			else
				MGAMMIOBase = NULL;

#endif
			if (!MGAMMIOBase)
			{
				ErrorF("%s %s: Can't map chip registers, "
					"acceleration disabled\n", XCONFIG_PROBED,
					vga256InfoRec.name);
				OFLG_SET(OPTION_NOACCEL, &vga256InfoRec.options);
			}
		}
		else
		{
			ErrorF("%s %s: Can't find PCI Base Address, "
				"acceleration disabled\n",
				XCONFIG_PROBED, vga256InfoRec.name,
				XCONFIG_PROBED, vga256InfoRec.name);
			OFLG_SET(OPTION_NOACCEL, &vga256InfoRec.options);
		}
	}
	
	if (OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
	{
		OFLG_SET(OPTION_NO_BITBLT, &vga256InfoRec.options);
	}
	
	if (!OFLG_ISSET(OPTION_NO_BITBLT, &vga256InfoRec.options))
	{

		/* width checking moved to MGAPitchAdjust */

		{
			if (xf86Verbose)
				ErrorF("%s %s: Using BitBlt Engine\n", XCONFIG_PROBED,
						vga256InfoRec.name);
		
			vga256LowlevFuncs.doBitbltCopy = MGADoBitbltCopy;
			vga256LowlevFuncs.fillRectSolidCopy = mgaFillRectSolidCopy;
		
			vga256TEOps1Rect.Polylines = mgaLine;
			vga256NonTEOps1Rect.Polylines = mgaLine;
			vga256TEOps.Polylines = mgaLine;
			vga256NonTEOps.Polylines = mgaLine;
		
			cfb16TEOps.CopyArea = MGA16CopyArea;
			cfb16NonTEOps.CopyArea = MGA16CopyArea;
			cfb16TEOps1Rect.CopyArea = MGA16CopyArea;
			cfb16NonTEOps1Rect.CopyArea = MGA16CopyArea;

			cfb24TEOps.CopyArea = MGA24CopyArea;
			cfb24NonTEOps.CopyArea = MGA24CopyArea;
			cfb24TEOps1Rect.CopyArea = MGA24CopyArea;
			cfb24NonTEOps1Rect.CopyArea = MGA24CopyArea;
	
			cfb32TEOps.CopyArea = MGA32CopyArea;
			cfb32NonTEOps.CopyArea = MGA32CopyArea;
			cfb32TEOps1Rect.CopyArea = MGA32CopyArea;
			cfb32NonTEOps1Rect.CopyArea = MGA32CopyArea;
			
			cfb16TEOps.PolyFillRect = mgaPolyFillRect;
			cfb16NonTEOps.PolyFillRect = mgaPolyFillRect;
			cfb16TEOps1Rect.PolyFillRect = mgaPolyFillRect;
			cfb16NonTEOps1Rect.PolyFillRect = mgaPolyFillRect;
			
			cfb24TEOps.PolyFillRect = mgaPolyFillRect;
			cfb24NonTEOps.PolyFillRect = mgaPolyFillRect;
			cfb24TEOps1Rect.PolyFillRect = mgaPolyFillRect;
			cfb24NonTEOps1Rect.PolyFillRect = mgaPolyFillRect;
			
			cfb32TEOps.PolyFillRect = mgaPolyFillRect;
			cfb32NonTEOps.PolyFillRect = mgaPolyFillRect;
			cfb32TEOps1Rect.PolyFillRect = mgaPolyFillRect;
			cfb32NonTEOps1Rect.PolyFillRect = mgaPolyFillRect;
			
	
			vgaSetScreenInitHook(MGAScrnInit);

#if 0
			/*
			 * Hardware cursor is not supported yet.
			 */
			vgaHWCursor.Initialized = TRUE;
			vgaHWCursor.Init = MGACursorInit;
			vgaHWCursor.Restore = (void (*)())NoopDDA;
			vgaHWCursor.Warp = xf86PointerScreenFuncs.WarpCursor;
			vgaHWCursor.QueryBestSize = mfbQueryBestSize;
#endif
		}
	}
	
	/*
	 * Some functions (eg, line drawing) are initialised via the
	 * cfbTEOps, cfbTEOps1Rect, cfbNonTEOps, cfbNonTEOps1Rect
	 * structs as well as in cfbLowlevFuncs.	These are of type
	 * 'struct GCFuncs' which is defined in mit/server/include/gcstruct.h.
	 
	cfbLowlevFuncs.lineSS = MGALineSS;
	cfbTEOps1Rect.Polylines = MGALineSS;
	cfbTEOps.Polylines = MGALineSS;
	cfbNonTEOps1Rect.Polylines = MGALineSS;
	cfbNonTEOps.Polylines = MGALineSS;
	 */
}

/*
 * MGAScrnInit --
 *
 * Sets some accelerated functions
 */		
static int
MGAScrnInit(pScreen, LinearBase, virtualX, virtualY, res1, res2, width)
ScreenPtr pScreen;
char *LinearBase;
int virtualX, virtualY, res1, res2, width;
{
	pScreen->CopyWindow = vga256CopyWindow;
	pScreen->PaintWindowBackground = mgaPaintWindow;
	pScreen->PaintWindowBorder = mgaPaintWindow;
	
	return(TRUE);
}

 /*
 * MGAInit --
 *
 * The 'mode' parameter describes the video mode.	The 'mode' structure 
 * as well as the 'vga256InfoRec' structure can be dereferenced for
 * information that is needed to initialize the mode.	The 'new' macro
 * (see definition above) is used to simply fill in the structure.
 */
static Bool
MGAInit(mode)
DisplayModePtr mode;
{
	int hd, hs, he, ht, vd, vs, ve, vt, wd;
	int i;

	/*
	 * This will allocate the datastructure and initialize all of the
	 * generic VGA registers.
	 */
	if (!vgaHWInit(mode,sizeof(vgaMGARec)))
		return(FALSE);

	/*
	 * Here all of the other fields of 'newVS' get filled in.
	 */
	hd = (mode->CrtcHDisplay >> 3)		- 1;
	hs = (mode->CrtcHSyncStart >> 3)	- 1;
	he = (mode->CrtcHSyncEnd	>> 3)	- 1;
	ht = (mode->CrtcHTotal		>> 3)	- 1;
	vd = mode->CrtcVDisplay				- 1;
	vs = mode->CrtcVSyncStart			- 1;
	ve = mode->CrtcVSyncEnd				- 1;
	vt = mode->CrtcVTotal				- 2;
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
		newVS->ExtVga[3]	= (((1 << MGABppShft) * 3) - 1) | 0x88;
	else
		newVS->ExtVga[3]	= ((1 << MGABppShft) - 1) | 0x88;

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

	for (i = 0; i < sizeof(MGADACregs); i++)
		newVS->DACreg[i] = MGAInitDAC[i]; 

	newVS->DAClong = MGADAClong;

	if (newVS->std.NoClock >= 2)
	{
		newVS->std.MiscOutReg |= 0x0C; 
		MGATi3026SetClock(vga256InfoRec.clock[newVS->std.NoClock],
				1 << MGABppShft);
	}

#ifdef DEBUG		
	ErrorF("%6ld: %02X %02X %02X	%02X %02X %02X	%08lX\n", vga256InfoRec.clock[newVS->std.NoClock],
		newVS->DACclk[0], newVS->DACclk[1], newVS->DACclk[2], newVS->DACclk[3], newVS->DACclk[4], newVS->DACclk[5], newVS->DAClong);
	for (i=0; i<sizeof(MGADACregs); i++) ErrorF("%02X ", newVS->DACreg[i]);
	for (i=0; i<6; i++) ErrorF(" %02X", newVS->ExtVga[i]);
	ErrorF("\n");
#endif
	return(TRUE);
}

/*
 * MGARestore --
 *
 * This function restores a video mode.	 It basically writes out all of
 * the registers that have previously been saved in the vgaMGARec data 
 * structure.
 */
static void 
MGARestore(restore)
vgaMGAPtr restore;
{
	int i;
	
	/*
	 * Code is needed to get things back to bank zero.
	 */
	for (i = 0; i < 6; i++)
		outw(0x3DE, (restore->ExtVga[i] << 8) | i);

	/*
	 * This function handles restoring the generic VGA registers.
	 */
	vgaHWRestore((vgaHWPtr)restore);

	/*
	 * Code to restore SVGA registers that have been saved/modified
	 * goes here. 
	 */
	outTi3026(0x2C, 0x00);
	for (i = 0; i < 3; i++)
		outTi3026(0x2D, restore->DACclk[i]);
	
	outTi3026(0x2C, 0x00);
	for (i = 3; i < 6; i++)
		outTi3026(0x2F, restore->DACclk[i]);
	
	for (i = 0; i < sizeof(MGADACregs); i++)
		outTi3026(MGADACregs[i], restore->DACreg[i]);

	pciWriteLong(MGAPciTag, PCI_OPTION_REG, restore->DAClong);

	MGABlitterInit(vgaBitsPerPixel,	vga256InfoRec.displayWidth);
	if (!MGAWaitForBlitter())
				FatalError("MGA: BitBlt Engine timeout\n");
}

/*
 * MGASave --
 *
 * This function saves the video state.	 It reads all of the SVGA registers
 * into the vgaMGARec data structure.
 */
static void *
MGASave(save)
vgaMGAPtr save;
{
	int i;
	
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
	for (i = 0; i < 6; i++)
	{
		outb(0x3DE, i);
		save->ExtVga[i] = inb(0x3DF);
	}
	
	outTi3026(0x2C, 0x00);
	for (i = 0; i < 3; i++)
		outTi3026(0x2D, save->DACclk[i] = inTi3026(0x2D));

	outTi3026(0x2C, 0x00);
	for (i = 3; i < 6; i++)
		outTi3026(0x2F, save->DACclk[i] = inTi3026(0x2F));
	
	for (i = 0; i < sizeof(MGADACregs); i++)
		save->DACreg[i]	 = inTi3026(MGADACregs[i]);
	
	save->DAClong = pciReadLong(MGAPciTag, PCI_OPTION_REG);
	
	return((void *) save);
}

/*
 * MGAEnterLeave --
 *
 * This function is called when the virtual terminal on which the server
 * is running is entered or left, as well as when the server starts up
 * and is shut down.	Its function is to obtain and relinquish I/O 
 * permissions for the SVGA device.	 This includes unlocking access to
 * any registers that may be protected on the chipset, and locking those
 * registers again on exit.
 */
static void 
MGAEnterLeave(enter)
Bool enter;
{
	unsigned char temp;

	if (enter)
	{
		xf86EnableIOPorts(vga256InfoRec.scrnIndex);
		if (MGAMMIOBase)
		{
			xf86MapDisplay(vga256InfoRec.scrnIndex,
					MMIO_REGION);
			if (!MGAWaitForBlitter())
				ErrorF("MGA: BitBlt Engine timeout\n");
		}
		
		vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;

		/* Unprotect CRTC[0-7] */
		outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
		outb(vgaIOBase + 5, temp & 0x7F);
	}
	else
	{
		/* Protect CRTC[0-7] */
		outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
		outb(vgaIOBase + 5, (temp & 0x7F) | 0x80);
		
		if (MGAMMIOBase)
		{
 			if (!MGAWaitForBlitter())
				ErrorF("MGA: BitBlt Engine timeout\n");
			xf86UnMapDisplay(vga256InfoRec.scrnIndex,
					MMIO_REGION);
		}
		
		xf86DisableIOPorts(vga256InfoRec.scrnIndex);
	}
}

/*
 * MGAAdjust --
 *
 * This function is used to initialize the SVGA Start Address - the first
 * displayed location in the video memory.
 */
static void 
MGAAdjust(x, y)
int x, y;
{
	int Base = (y * vga256InfoRec.displayWidth + x) >>
			(3 - MGABppShft);
	int tmp;

	if (vgaBitsPerPixel == 24)
		Base *= 3;

	/* Wait for vertical retrace */
	while (!(inb(0x3DA) & 0x08));
	
	outb(0x3DE, 0x00);
	tmp = inb(0x3DF);
	outb(0x3DF, (tmp & 0xF0) | ((Base & 0x0F0000) >> 16));
	outw(vgaIOBase + 4, (Base & 0x00FF00) | 0x0C);
	outw(vgaIOBase + 4, ((Base & 0x0000FF) << 8) | 0x0D);
}

/*
 * MGAValidMode -- 
 *
 * Checks if a mode is suitable for the selected chipset.
 */
static Bool
MGAValidMode(mode)
DisplayModePtr mode;
{
	int lace = 1 + ((mode->Flags & V_INTERLACE) != 0);
	
	if ((mode->CrtcHDisplay <= 4096) &&
	    (mode->CrtcHSyncStart <= 4096) && 
	    (mode->CrtcHSyncEnd <= 4096) && 
	    (mode->CrtcHTotal <= 4096) &&
	    (mode->CrtcVDisplay <= 2048 * lace) &&
	    (mode->CrtcVSyncStart <= 4096 * lace) &&
	    (mode->CrtcVSyncEnd <= 4096 * lace) &&
	    (mode->CrtcVTotal <= 4096 * lace))
	{
		return(MODE_OK);
	}
	else
	{
		return(MODE_BAD);
	}
}
