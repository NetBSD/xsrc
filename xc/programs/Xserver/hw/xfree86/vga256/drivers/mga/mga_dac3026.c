/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mga_dac3026.c,v 1.1.2.13 1999/02/26 02:04:36 dawes Exp $ */
/*
 * Copyright 1994 by Robin Cutshaw <robin@XFree86.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Robin Cutshaw not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Robin Cutshaw makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ROBIN CUTSHAW DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ROBIN CUTSHAW BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * Modified for TVP3026 by Harald Koenig <koenig@tat.physik.uni-tuebingen.de>
 * 
 * Modified for MGA Millennium by Xavier Ducoin <xavier@rd.lectra.fr>
 *
 * Doug Merritt <doug@netcom.com>
 * 24bpp: fixed high res stripe glitches, clock glitches on all res
 *
 */


#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86cursor.h"
#include "vga.h"
#include "vgaPCI.h"

#ifdef PC98_MGA
#ifdef XFreeXDGA
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif
#endif

#include "mga_bios.h"
#include "mga_reg.h"
#include "mga.h"

/* Set to 1 if you want to set MCLK from XF86Config - AT YOUR OWN RISK! */
#define MCLK_FROM_XCONFIG 1

/*
 * exported functions
 */
void	MGA3026RamdacInit();
Bool	MGA3026Init();
void	MGA3026Restore();
void*	MGA3026Save();
#ifdef PC98_MGA
void	MGA3026Reset();
#endif

/*
 * implementation
 */
 
/*
 * indexes to ti3026 registers (the order is important)
 */
static unsigned char MGADACregs[] = {
	0x0F, 0x18, 0x19, 0x1A, 0x1C,   0x1D, 0x1E, 0x2A, 0x2B, 0x30,
	0x31, 0x32, 0x33, 0x34, 0x35,   0x36, 0x37, 0x38, 0x39, 0x3A,
	0x06
};

/*
 * initial values of ti3026 registers
 */
static unsigned char MGADACbpp8[] = {
	0x06, 0x80,    0, 0x25, 0x00,   0x00, 0x0C, 0x00, 0x1E, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   0xFF, 0x00, 0x00,    0, 0x00,
	   0
};
static unsigned char MGADACbpp16[] = {
	0x07, 0x05,    0, 0x15, 0x00,   0x00, 0x2C, 0x00, 0x1E, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   0xFF, 0x00, 0x00,    0, 0x00,
	   0
};
/*
 * [0] value was 0x07, but changed to 0x06 by Doug Merrit to fix high res
 * stripe glitches and clock glitches at 24bpp.
 */
static unsigned char MGADACbpp24[] = {
	0x06, 0x16,    0, 0x25, 0x00,   0x00, 0x2C, 0x00, 0x1E, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   0xFF, 0x00, 0x00,    0, 0x00,
	   0
};
static unsigned char MGADACbpp32[] = {
	0x07, 0x06,    0, 0x05, 0x00,   0x00, 0x2C, 0x00, 0x1E, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   0xFF, 0x00, 0x00,    0, 0x00,
	   0
};

/*
 * This is a convenience macro, so that entries in the driver structure
 * can simply be dereferenced with 'newVS->xxx'.
 * change ajv - new conflicts with the C++ reserved word.
 */
#define newVS ((vgaMGAPtr)vgaNewVideoState)

typedef struct {
	vgaHWRec std;                           /* good old IBM VGA */
	unsigned long DAClong;
	unsigned char DACclk[6];
	unsigned char DACreg[sizeof(MGADACregs)];
	unsigned char ExtVga[6];
} vgaMGARec, *vgaMGAPtr;
    
/*
 * Read/write to the DAC via MMIO 
 */

/*
 * direct registers
 */
static unsigned char inTi3026dreg(reg)
unsigned char reg;
{
	if (!MGAMMIOBase)
		FatalError("MGA: IO registers not mapped\n");

	return INREG8(RAMDAC_OFFSET + reg);
}

static void outTi3026dreg(reg, val)
unsigned char reg, val;
{
	if (!MGAMMIOBase)
		FatalError("MGA: IO registers not mapped\n");

	OUTREG8(RAMDAC_OFFSET + reg, val);
}

/*
 * indirect registers
 */
static unsigned char inTi3026(reg)
unsigned char reg;
{
	outTi3026dreg(TVP3026_INDEX, reg);
	return inTi3026dreg(TVP3026_DATA);
}

static void outTi3026(reg, mask, val)
unsigned char reg, mask, val;
{
	unsigned char tmp = mask? (inTi3026(reg) & mask) : 0;

	outTi3026dreg(TVP3026_INDEX, reg);
	outTi3026dreg(TVP3026_DATA, tmp | val);
}

#ifdef PC98_MGA
/* taken from vgaCmap.c */
void
MGATi3026StoreColors(pmap, ndef, pdefs)
     ColormapPtr	pmap;
     int		ndef;
     xColorItem	        *pdefs;
{
    int		i;
    unsigned char *cmap, *tmp;
    xColorItem	directDefs[256];
    Bool          new_overscan = FALSE;
    unsigned char overscan = ((vgaHWPtr)vgaNewVideoState)->Attribute[OVERSCAN];
    unsigned char tmp_overscan;

    if (vgaCheckColorMap(pmap))
        return;

    if ((pmap->pVisual->class | DynamicClass) == DirectColor)
    {
        ndef = cfbExpandDirectColors (pmap, ndef, pdefs, directDefs);
        pdefs = directDefs;
    }

    for(i = 0; i < ndef; i++)
    {
        if (pdefs[i].pixel == overscan)
	{
	    new_overscan = TRUE;
	}
        cmap = &((vgaHWPtr)vgaNewVideoState)->DAC[pdefs[i].pixel*3];
	if (vgaDAC8BitComponents) {
            cmap[0] = pdefs[i].red   >> 8;
            cmap[1] = pdefs[i].green >> 8;
            cmap[2] = pdefs[i].blue  >> 8;
        }
        else {
            cmap[0] = pdefs[i].red   >> 10;
            cmap[1] = pdefs[i].green >> 10;
            cmap[2] = pdefs[i].blue  >> 10;
        }

        if (xf86VTSema
#ifdef XFreeXDGA
	    || ((vga256InfoRec.directMode & XF86DGADirectGraphics)
	        && !(vga256InfoRec.directMode & XF86DGADirectColormap))
	    || (vga256InfoRec.directMode & XF86DGAHasColormap)
#endif
	   )
	{
	    outTi3026dreg(TVP3026_WADR_PAL, pdefs[i].pixel);
	    outTi3026dreg(TVP3026_COL_PAL, cmap[0]);
	    outTi3026dreg(TVP3026_COL_PAL, cmap[1]);
	    outTi3026dreg(TVP3026_COL_PAL, cmap[2]);
	}
    }	
    if (new_overscan)
    {
	new_overscan = FALSE;
        for(i = 0; i < ndef; i++)
        {
            if (pdefs[i].pixel == overscan)
	    {
	        if ((pdefs[i].red != 0) || 
	            (pdefs[i].green != 0) || 
	            (pdefs[i].blue != 0))
	        {
	            new_overscan = TRUE;
		    tmp_overscan = overscan;
        	    tmp = &((vgaHWPtr)vgaNewVideoState)->DAC[pdefs[i].pixel*3];
	        }
	        break;
	    }
        }
        if (new_overscan)
        {
            /*
             * Find a black pixel, or the nearest match.
             */
            for (i=255; i >= 0; i--)
	    {
                cmap = &((vgaHWPtr)vgaNewVideoState)->DAC[i*3];
	        if ((cmap[0] == 0) && (cmap[1] == 0) && (cmap[2] == 0))
	        {
	            overscan = i;
	            break;
	        }
	        else
	        {
	            if ((cmap[0] < tmp[0]) && 
		        (cmap[1] < tmp[1]) && (cmap[2] < tmp[2]))
	            {
		        tmp = cmap;
		        tmp_overscan = i;
	            }
	        }
	    }
	    if (i < 0)
	    {
	        overscan = tmp_overscan;
	    }
	    ((vgaHWPtr)vgaNewVideoState)->Attribute[OVERSCAN] = overscan;
            if (xf86VTSema
#ifdef XFreeXDGA
	        || ((vga256InfoRec.directMode & XF86DGADirectGraphics)
	            && !(vga256InfoRec.directMode & XF86DGADirectColormap))
	        || (vga256InfoRec.directMode&XF86DGAHasColormap)
#endif
	       )
	    {
	      (void)inb(vgaIOBase + 0x0A);
	      outb(0x3C0, OVERSCAN);
	      outb(0x3C0, overscan);
	      (void)inb(vgaIOBase + 0x0A);
	      outb(0x3C0, 0x20);
	    }
        }
    }
}
#endif

/*
 * MGATi3026CalcClock - Calculate the PLL settings (m, n, p).
 *
 * DESCRIPTION
 *   For more information, refer to the Texas Instruments
 *   "TVP3026 Data Manual" (document SLAS098B).
 *     Section 2.4 "PLL Clock Generators"
 *     Appendix A "Frequency Synthesis PLL Register Settings"
 *     Appendix B "PLL Programming Examples"
 *
 * PARAMETERS
 *   f_out		IN	Desired clock frequency.
 *   f_max		IN	Maximum allowed clock frequency.
 *   m			OUT	Value of PLL 'm' register.
 *   n			OUT	Value of PLL 'n' register.
 *   p			OUT	Value of PLL 'p' register.
 *
 * HISTORY
 *   February 7, 1998 - Sebastien Marineau
 *   Minor improvement to the optimizer algorithm
 *
 *   January 11, 1997 - [aem] Andrew E. Mileski
 *   Split off from MGATi3026SetClock.
 */

/* The following values are in kHz */
#define TI_MIN_VCO_FREQ  110000
#define TI_MAX_VCO_FREQ  220000
#define TI_MAX_MCLK_FREQ 100000
#define TI_REF_FREQ      14318.18

static double
MGATi3026CalcClock ( f_out, f_max, m, n, p )
	long f_out;
	long f_max;
	int *m;
	int *n;
	int *p;
{
	int best_m, best_n;
	double f_pll, f_vco;
	double m_err, inc_m, calc_m;

	/* Make sure that f_min <= f_out <= f_max */
	if ( f_out < ( TI_MIN_VCO_FREQ / 8 ))
		f_out = TI_MIN_VCO_FREQ / 8;
	if ( f_out > f_max )
		f_out = f_max;

	/*
	 * f_pll = f_vco / 2 ^ p
	 * Choose p so that TI_MIN_VCO_FREQ <= f_vco <= TI_MAX_VCO_FREQ
	 * Note that since TI_MAX_VCO_FREQ = 2 * TI_MIN_VCO_FREQ
	 * we don't have to bother checking for this maximum limit.
	 */
	f_vco = ( double ) f_out;
	for ( *p = 0; *p < 3 && f_vco < TI_MIN_VCO_FREQ; ( *p )++ )
		f_vco *= 2.0;

	/*
	 * We avoid doing multiplications by ( 65 - n ),
	 * and add an increment instead - this keeps any error small.
	 */
	inc_m = f_vco / ( TI_REF_FREQ * 8.0 );

	/* Initial value of calc_m for the loop */
	calc_m = inc_m + inc_m + inc_m;

	/* Initial amount of error for an integer - impossibly large */
	m_err = 2.0;

	/* Search for the closest INTEGER value of ( 65 - m ) */
	for ( *n = 3; *n <= 25; ( *n )++, calc_m += inc_m ) {

		/* Ignore values of ( 65 - m ) which we can't use */
		if ( calc_m < 3.0 || calc_m > 64.0 )
			continue;

		/*
		 * Pick the closest INTEGER (has smallest fractional part).
		 * The optimizer should clean this up for us.
		 */
		if (( calc_m - ( int ) calc_m ) < m_err ) {
			m_err = calc_m - ( int ) calc_m;
			best_m = ( int ) calc_m;
			best_n = *n;
		}
		if ((( - calc_m + 1.0 + ( int ) calc_m) < m_err ) &&
			((int) calc_m < 64)) {
			m_err = -calc_m + ( int ) calc_m + 1;
			best_m = ( int ) calc_m + 1;
			best_n = *n;
 		}
	}
	
	/* 65 - ( 65 - x ) = x */
	*m = 65 - best_m;
	*n = 65 - best_n;

	/* Now all the calculations can be completed */
	f_vco = 8.0 * TI_REF_FREQ * best_m / best_n;
	f_pll = f_vco / ( 1 << *p );

#ifdef DEBUG
	ErrorF( "f_out=%ld f_pll=%.1f f_vco=%.1f n=%d m=%d p=%d\n",
		f_out, f_pll, f_vco, *n, *m, *p );
#endif

	return f_pll;
}

/*
 * MGATi3026SetMCLK - Set the memory clock (MCLK) PLL.
 *
 * HISTORY
 *   January 11, 1997 - [aem] Andrew E. Mileski
 *   Written and tested.
 */
static void
MGATi3026SetMCLK( f_out )
	long f_out;
{
	double f_pll;
	int mclk_m, mclk_n, mclk_p;
	int pclk_m, pclk_n, pclk_p;
	int mclk_ctl, rfhcnt;

	f_pll = MGATi3026CalcClock(
		f_out, TI_MAX_MCLK_FREQ,
		& mclk_m, & mclk_n, & mclk_p
	);

	/* Save PCLK settings */
	outTi3026( TVP3026_PLL_ADDR, 0, 0xfc );
	pclk_n = inTi3026( TVP3026_PIX_CLK_DATA );
	outTi3026( TVP3026_PLL_ADDR, 0, 0xfd );
	pclk_m = inTi3026( TVP3026_PIX_CLK_DATA );
	outTi3026( TVP3026_PLL_ADDR, 0, 0xfe );
	pclk_p = inTi3026( TVP3026_PIX_CLK_DATA );
	
	/* Stop PCLK (PLLEN = 0, PCLKEN = 0) */
	outTi3026( TVP3026_PLL_ADDR, 0, 0xfe );
	outTi3026( TVP3026_PIX_CLK_DATA, 0, 0x00 );
	
	/* Set PCLK to the new MCLK frequency (PLLEN = 1, PCLKEN = 0 ) */
	outTi3026( TVP3026_PLL_ADDR, 0, 0xfc );
	outTi3026( TVP3026_PIX_CLK_DATA, 0, ( mclk_n & 0x3f ) | 0xc0 );
	outTi3026( TVP3026_PIX_CLK_DATA, 0, mclk_m & 0x3f );
	outTi3026( TVP3026_PIX_CLK_DATA, 0, ( mclk_p & 0x03 ) | 0xb0 );
	
	/* Wait for PCLK PLL to lock on frequency */
	while (( inTi3026( TVP3026_PIX_CLK_DATA ) & 0x40 ) == 0 ) {
		;
	}
	
	/* Output PCLK on MCLK pin */
	mclk_ctl = inTi3026( TVP3026_MCLK_CTL );
	outTi3026( TVP3026_MCLK_CTL, 0, mclk_ctl & 0xe7 ); 
	outTi3026( TVP3026_MCLK_CTL, 0, ( mclk_ctl & 0xe7 ) | 0x08 );
	
	/* Stop MCLK (PLLEN = 0 ) */
	outTi3026( TVP3026_PLL_ADDR, 0, 0xfb );
	outTi3026( TVP3026_MEM_CLK_DATA, 0, 0x00 );
	
	/* Set MCLK to the new frequency (PLLEN = 1) */
	outTi3026( TVP3026_PLL_ADDR, 0, 0xf3 );
	outTi3026( TVP3026_MEM_CLK_DATA, 0, ( mclk_n & 0x3f ) | 0xc0 );
	outTi3026( TVP3026_MEM_CLK_DATA, 0, mclk_m & 0x3f );
	outTi3026( TVP3026_MEM_CLK_DATA, 0, ( mclk_p & 0x03 ) | 0xb0 );
	
	/* Wait for MCLK PLL to lock on frequency */
	while (( inTi3026( TVP3026_MEM_CLK_DATA ) & 0x40 ) == 0 ) {
		;
	}

	/* Set the WRAM refresh divider */
	/* these formulas assume nogscale=1; RTFM if gscale=0, esp. if
	 *   you are slowing the clocks for power-saving
	 * Also note that the 1064SG seems to treat rfhcnt differently,
	 *   but it doesn't use this dac.
	 */
	if (MGAchipset == PCI_CHIP_MGA2064 )
	  {
	    /* this one is from the 2064W manual. */
	    /* rfhcnt = (( (33.2 * f_pll) / 1000.0 ) / 128) - 1; */
	    /* changing to -.5 to get round-to-nearest approximation */
	    rfhcnt = (((33.2 * f_pll) / 1000.0 ) / 128.0) - .5;
	    if ( rfhcnt > 15 )
	      {
#ifdef DEBUG
		ErrorF( "error: rfhcnt=%d, setting to 0\n", rfhcnt );
#endif
		/* the 2064W manual implies that zero is OK (?)
		 * "minimum frequency" is supposedly set to 4MHz
		 * when rfhcnt=0 and nogscale=1
		 */
		
		rfhcnt = 0;
	      }
	  }
	else /* 2164W PCI or AGP and default */
	  {
	    /* this is calculated from the 2164W manual. It seems OK with
	     * both the 2164W-AGP and a PCI 2064W. Barring algebra errors,
	     * with expected rounding, this should be guaranteed to conform
	     * to the formula from p.3-18 of the 2164W manual.
	     */
	    rfhcnt = (( (33.2 * f_pll) / 1000.0 ) - 1) / 256;
	    if ( rfhcnt > 15 )
	      {
#ifdef DEBUG
		ErrorF( "error: rfhcnt=%d, setting to 15\n", rfhcnt );
#endif
		rfhcnt = 15;
	      }
#ifdef DEBUG  /* paranoia check, neither should ever happen */
	    /* check formula from p. 3-18 of MGA-2164W manual
	     * 33.2 >= (rfhcnt<3:1>*512 + rfhcnt<0>*64 + 1) *
	     *     gclk_period * gscaling_factor
	     */
	    {
	      double refresh_period = ((rfhcnt & 0xE)*256 +
			       (rfhcnt & 1)*64 + 1) * ( 1000.0 / f_pll);      
	      if (! ( 33.2 >= refresh_period ))
		{
		  ErrorF( "warning: rfhcnt=%d -> %lf usec > 33.2 usec\n",
			  rfhcnt, refresh_period );
		}
	    }
	    if ( rfhcnt == 0 )
	      ErrorF( "warning: 2164W memory refresh disabled!\n");
#endif
	  } /* MGAchipset choice */

#ifdef DEBUG
	ErrorF( "rfhcnt=%d\n", rfhcnt );
#endif	

	rfhcnt <<= 16;

    	if(!OFLG_ISSET(OPTION_PCI_RETRY, &vga256InfoRec.options))
	   rfhcnt |= (1 << 29);

 	pciWriteLong( MGAPciTag, PCI_OPTION_REG, rfhcnt |
		( pciReadLong( MGAPciTag, PCI_OPTION_REG ) & ~0x200f0000 ));

	/* Output MCLK PLL on MCLK pin */
	outTi3026( TVP3026_MCLK_CTL, 0, ( mclk_ctl & 0xe7 ) | 0x10 );
	outTi3026( TVP3026_MCLK_CTL, 0, ( mclk_ctl & 0xe7 ) | 0x18 );
	
	/* Stop PCLK (PLLEN = 0, PCLKEN = 0 ) */
	outTi3026( TVP3026_PLL_ADDR, 0, 0xfe );
	outTi3026( TVP3026_PIX_CLK_DATA, 0, 0x00 );
	
	/* Restore PCLK (PLLEN = ?, PCLKEN = ?) */
	outTi3026( TVP3026_PLL_ADDR, 0, 0xfc );
	outTi3026( TVP3026_PIX_CLK_DATA, 0, pclk_n );
	outTi3026( TVP3026_PIX_CLK_DATA, 0, pclk_m );
	outTi3026( TVP3026_PIX_CLK_DATA, 0, pclk_p );
	
	/* Wait for PCLK PLL to lock on frequency */
	while (( inTi3026( TVP3026_PIX_CLK_DATA ) & 0x40 ) == 0 ) {
		;
	}
}

/*
 * MGATi3026SetPCLK - Set the pixel (PCLK) and loop (LCLK) clocks.
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
 *   February 7, 1998 - Sebastien Marineau
 *   Changes to 24 bpp PLL setup and changes for revision B Ramdacs
 *
 *   January 11, 1997 - [aem] Andrew E. Mileski
 *   Split to simplify code for MCLK (=GCLK) setting.
 *
 *   December 14, 1996 - [aem] Andrew E. Mileski
 *   Fixed loop clock to be based on the calculated, not requested,
 *   pixel clock. Added f_max = maximum f_vco frequency.
 *
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

static void 
MGATi3026SetPCLK( f_out, bpp )
	long	f_out;
	int	bpp;
{
	/* Pixel clock values */
	int m, n, p;

	/* Loop clock values */
	int lm, ln, lp, lq;
	double z;

	/* The actual frequency output by the clock */
	double f_pll;

	/* The silicon revision of the 3026 */
	int s_rev;

	/* Get the maximum pixel clock frequency */
	long f_max = TI_MAX_VCO_FREQ;
	if ( vga256InfoRec.maxClock > TI_MAX_VCO_FREQ )
		f_max = vga256InfoRec.maxClock;

	/* Get the silicon revision */
	s_rev = inTi3026 (TVP3026_SILICON_REV);
#ifdef DEBUG
	ErrorF("Found TVP3026 Rev. %c\n", ( s_rev > 0x20 ) ? 'B' : 'A' );
#endif
	/* Do the calculations for m, n, and p */
	f_pll = MGATi3026CalcClock( f_out, f_max, & m, & n, & p );

	/* Values for the pixel clock PLL registers */
	newVS->DACclk[ 0 ] = ( n & 0x3f ) | 0xc0;
	newVS->DACclk[ 1 ] = ( m & 0x3f );
	newVS->DACclk[ 2 ] = ( p & 0x03 ) | 0xb0;

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
		z = ( 11000 * ( 65 - ln )) / (( f_pll / 1000 ) * ( 65 - lm ));
	}
	else {
		/* ln:lm = ln:4 */
		lm = 65 - 4;

		/* Note: bpp = bytes per pixel */
		ln = 65 - 4 * ( 64 / 8 ) / bpp;

		/* Note: this is actually 100 * z for more precision */
		z = (( 11000 / 4 ) * ( 65 - ln )) / ( f_pll / 1000 );
	}

	/*
	 * Now we choose dividers lp and lq so that the VCO frequency
	 * is within the operating range of 110 MHz to 220 MHz.
	 */

	/* Assume no lq divider */
	lq = 0;

	/* Note: z is actually 100 * z for more precision */
	if ( z <= 200.0 )
		lp = 0;
	else if ( z <= 400.0 )
		lp = 1;
	else if ( z <= 800.0 )
		lp = 2;
	else if ( z <= 1600.0 )
		lp = 3;
	else {
		lp = 3;
		lq = ( int )( z / 1600.0 );
	}
 
	/* 
	 * Values for the loop clock PLL registers
	 * 
	 * [SM] We have changes for 24bpp modes
	 * for 3026B in 4:3 multiplex modes (see updated
	 * TI document). Also, the corruption problem seen
	 * for high pixel rates at 8:3 multiplex ratios is fixed
	 * by clearing bit 6 of the loop clock p register. 
	 */
	if ( vgaBitsPerPixel == 24 ) {
		int setbits;
		/* Packed pixel mode values */
		newVS->DACclk[ 3 ] = ( ln & 0x3f ) | 0x80;
		/* Set m-value depending on silicon rev. and multiplex */
		if (( bpp == 2 ) && ( s_rev > 0x20 ))
			newVS->DACclk[ 4 ] = ( lm & 0x3f ) | 0x00;
		else
			newVS->DACclk[ 4 ] = ( lm & 0x3f ) | 0x80;
		/* Clear bit 6 of p if high dot clock at 8:3 ratio
		 * this seems to be necessary for 8MB or below, but
		 * wrong for 16MB cards; to be on the save side we 
		 * have an option to flip the bit [SM,DHH]
		 */
		if ((MGAchipset == PCI_CHIP_MGA2164 ) &&
		    ( vga256InfoRec.videoRam <= 8192 ))
			setbits = 0xb8;
		else
			setbits = 0xf8;
		if (OFLG_ISSET(OPTION_MGA_24BPP_FIX, &vga256InfoRec.options))
			setbits ^= 0x40;
#ifdef DEBUG
	ErrorF("PLL bits set to 0x%2x\n",setbits);
#endif
		if (( bpp == 1) && ( f_pll > 100000 ))
			newVS->DACclk[ 5 ] = ( lp & 0x03 ) | setbits;
		else 
			newVS->DACclk[ 5 ] = ( lp & 0x03 ) | 0xf8;

 	} else {
		/* Non-packed pixel mode values */
		newVS->DACclk[ 3 ] = ( ln & 0x3f ) | 0xc0;
		newVS->DACclk[ 4 ] = ( lm & 0x3f );
		newVS->DACclk[ 5 ] = ( lp & 0x03 ) | 0xf0;
	}
	newVS->DACreg[ 18 ] = lq | 0x38;

	/* Finally, adjust latch-control register for 24bpp, 4:3, rev. B */
	if (( vgaBitsPerPixel == 24 ) && ( bpp == 2 ) && ( s_rev > 0x20 ))
		newVS->DACreg[ 0 ] = 0x08;

#ifdef DEBUG
	ErrorF( "bpp=%d z=%.1f ln=%d lm=%d lp=%d lq=%d\n",
		bpp, z, ln, lm, lp, lq );
#endif
}

/*
 * MGA3026Init -- for mga2064 with ti3026
 *
 * The 'mode' parameter describes the video mode.	The 'mode' structure 
 * as well as the 'vga256InfoRec' structure can be dereferenced for
 * information that is needed to initialize the mode.	The 'new' macro
 * (see definition above) is used to simply fill in the structure.
 */
Bool
MGA3026Init(mode)
DisplayModePtr mode;
{
	int hd, hs, he, ht, vd, vs, ve, vt, wd;
	int i, index_1d;
	unsigned char* initDAC;

	switch(vgaBitsPerPixel)
	{
	case 8:
		initDAC = MGADACbpp8;
		initDAC[2] = MGAinterleave? 0x4C : 0x4B;
		break;
	case 16:
		initDAC = MGADACbpp16;
		initDAC[2] = MGAinterleave? 0x54 : 0x53;
		if ( (xf86weight.red == 5) && (xf86weight.green == 5)
					&& (xf86weight.blue == 5) )
			initDAC[1] = 0x04 ;
		break;
	case 24:
		initDAC = MGADACbpp24;
		initDAC[2] = MGAinterleave? 0x5C : 0x5B;
		break;
	case 32:
		initDAC = MGADACbpp32;
		initDAC[2] = MGAinterleave? 0x5C : 0x5B;
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
	initDAC[20] = 0;
	
	if (mode->Flags & V_INTERLACE)
	{
		newVS->ExtVga[0] = 0x80;
		newVS->ExtVga[5] = (hs + he - ht) >> 1;
		wd <<= 1;
		vt &= 0xFFFE;
		
		/* enable interlaced cursor */
		initDAC[20] |= 0x20;
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

	/* Set viddelay (CRTCEXT3 Bits 3-4). */
	newVS->ExtVga[3] |= (vga256InfoRec.videoRam >= 8192 ? 0x10
			     : vga256InfoRec.videoRam == 2048 ? 0x08 : 0x00);

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
    
	for (i = 0; i < sizeof(MGADACregs); i++)
	{
	    newVS->DACreg[i] = initDAC[i]; 
	    if (MGADACregs[i] == 0x1D)
		index_1d = i;
	}

	/* Per DDK vid.c line 75, sync polarity should be controlled
	 * via the TVP3026 RAMDAC register 1D and so MISC Output Register
	 * should always have bits 6 and 7 set. */

	newVS->std.MiscOutReg |= 0xC0;
	if ((mode->Flags & (V_PHSYNC | V_NHSYNC)) &&
	    (mode->Flags & (V_PVSYNC | V_NVSYNC)))
	{
	    if (mode->Flags & V_PHSYNC)
		newVS->DACreg[index_1d] |= 0x01;
	    if (mode->Flags & V_PVSYNC)
		newVS->DACreg[index_1d] |= 0x02;
	}
	else
	{
	  int VDisplay = mode->VDisplay;
	  if (mode->Flags & V_DBLSCAN)
	    VDisplay *= 2;
	  if      (VDisplay < 400)
		  newVS->DACreg[index_1d] |= 0x01; /* +hsync -vsync */
	  else if (VDisplay < 480)
		  newVS->DACreg[index_1d] |= 0x02; /* -hsync +vsync */
	  else if (VDisplay < 768)
		  newVS->DACreg[index_1d] |= 0x00; /* -hsync -vsync */
	  else
		  newVS->DACreg[index_1d] |= 0x03; /* +hsync +vsync */
	}
	
	if (OFLG_ISSET(OPTION_SYNC_ON_GREEN, &vga256InfoRec.options))
	    newVS->DACreg[index_1d] |= 0x20;

	newVS->DAClong = MGAinterleave << 12;

	newVS->std.MiscOutReg |= 0x0C; 
	MGATi3026SetPCLK(
		vga256InfoRec.clock[newVS->std.NoClock], 1 << MGABppShft
	);

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
 * MGA3026Restore -- for mga2064 with ti3026
 *
 * This function restores a video mode.	 It basically writes out all of
 * the registers that have previously been saved in the vgaMGARec data 
 * structure.
 */
void 
MGA3026Restore(restore)
vgaMGAPtr restore;
{
	int i;

	/*
	 * Code is needed to get things back to bank zero.
	 */
	for (i = 0; i < 6; i++)
		outw(0x3DE, (restore->ExtVga[i] << 8) | i);

	pciWriteLong(MGAPciTag, PCI_OPTION_REG, restore->DAClong |
		(pciReadLong(MGAPciTag, PCI_OPTION_REG) & ~0x1000));

	/* select pixel clock PLL as clock source */
	outTi3026(TVP3026_CLK_SEL, 0, restore->DACreg[3]);
	
	/* set loop and pixel clock PLL PLLEN bits to 0 */
	outTi3026(TVP3026_PLL_ADDR, 0, 0x2A);
	outTi3026(TVP3026_LOAD_CLK_DATA, 0, 0);
	outTi3026(TVP3026_PIX_CLK_DATA, 0, 0);
	 
	/*
	 * This function handles restoring the generic VGA registers.
	 */
	vgaHWRestore((vgaHWPtr)restore);

	/*
	 * Code to restore SVGA registers that have been saved/modified
	 * goes here. 
	 */

	/*
	 * this is needed to properly restore start address for 2164W-AGP
	 */
	outw(0x3DE, (restore->ExtVga[0] << 8) | 0);

	/* program pixel clock PLL */
	outTi3026(TVP3026_PLL_ADDR, 0, 0x00);
	for (i = 0; i < 3; i++)
		outTi3026(TVP3026_PIX_CLK_DATA, 0, restore->DACclk[i]);
	 
	if (restore->std.MiscOutReg & 0x08) {
		/* poll until pixel clock PLL LOCK bit is set */
		outTi3026(TVP3026_PLL_ADDR, 0, 0x3F);
		while ( ! (inTi3026(TVP3026_PIX_CLK_DATA) & 0x40) );
	}
	/* set Q divider for loop clock PLL */
	outTi3026(TVP3026_MCLK_CTL, 0, restore->DACreg[18]);
	
	/* program loop PLL */
	outTi3026(TVP3026_PLL_ADDR, 0, 0x00);
	for (i = 3; i < 6; i++)
		outTi3026(TVP3026_LOAD_CLK_DATA, 0, restore->DACclk[i]);
	
	if ((restore->std.MiscOutReg & 0x08) && ((restore->DACclk[3] & 0xC0) == 0xC0) ) {
		/* poll until loop PLL LOCK bit is set */
		outTi3026(TVP3026_PLL_ADDR, 0, 0x3F);
		while ( ! (inTi3026(TVP3026_LOAD_CLK_DATA) & 0x40) );
	}
	
	/*
	 * restore other DAC registers
	 */
	for (i = 0; i < sizeof(MGADACregs); i++)
		outTi3026(MGADACregs[i], 0, restore->DACreg[i]);

#ifdef DEBUG
	ErrorF("PCI retry (0-enabled / 1-disabled): %d\n",
		!!(restore->DAClong & 0x20000000));
#endif		 
}

/*
 * MGA3026Save -- for mga2064 with ti3026
 *
 * This function saves the video state.	 It reads all of the SVGA registers
 * into the vgaMGARec data structure.
 */
void *
MGA3026Save(save)
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
	
	outTi3026(TVP3026_PLL_ADDR, 0, 0x00);
	for (i = 0; i < 3; i++)
		outTi3026(TVP3026_PIX_CLK_DATA, 0, save->DACclk[i] =
					inTi3026(TVP3026_PIX_CLK_DATA));

	outTi3026(TVP3026_PLL_ADDR, 0, 0x00);
	for (i = 3; i < 6; i++)
		outTi3026(TVP3026_LOAD_CLK_DATA, 0, save->DACclk[i] =
					inTi3026(TVP3026_LOAD_CLK_DATA));
	
	for (i = 0; i < sizeof(MGADACregs); i++)
		save->DACreg[i]	 = inTi3026(MGADACregs[i]);
	
	save->DAClong = pciReadLong(MGAPciTag, PCI_OPTION_REG);
	
	return (void *)save;
}

/*
 * Convert the cursor from server-format to hardware-format.  The Ti3020
 * has two planes, plane 0 selects cursor color 0 or 1 and plane 1
 * selects transparent or display cursor.  The bits of these planes
 * loaded sequentially so that one byte has 8 pixels. The organization
 * looks like:
 *             Byte 0x000 - 0x007    top scan line, left to right plane 0
 *                  0x008 - 0x00F
 *                    .       .
 *                  0x1F8 - 0x1FF    bottom scan line plane 0
 *
 *                  0x200 - 0x207    top scan line, left to right plane 1
 *                  0x208 - 0x20F
 *                    .       .
 *                  0x3F8 - 0x3FF    bottom scan line plane 1
 *
 *             Byte/bit map - D7,D6,D5,D4,D3,D2,D1,D0  eight pixels each
 *             Pixel/bit map - P1P0  (plane 1) == 1 maps to cursor color
 *                                   (plane 1) == 0 maps to transparent
 *                                   (plane 0) maps to cursor colors 0 and 1
 */


static void
MGA3026LoadCursorImage(src, xorigin, yorigin)
    register unsigned char *src;
    int xorigin, yorigin;
{
    register int i;
    register unsigned char *mask = src + 1;
       
    outTi3026(TVP3026_CURSOR_CTL, 0xf3, 0x00); /* reset A9,A8 */
    /* reset cursor RAM load address A7..A0 */
    outTi3026dreg(TVP3026_WADR_PAL, 0x00); 

    for (i = 0; i < 512; i++, mask+=2) {
	if (MGACursorBug) {
	    while (INREG8(0x1FDA) & 0x01);
	    while (!(INREG8(0x1FDA) & 0x01));
	}
        outTi3026dreg(TVP3026_CUR_RAM, *mask);    
    }
    for (i = 0; i < 512; i++, src+=2) {
	if (MGACursorBug) {
	    while (INREG8(0x1FDA) & 0x01);
	    while (!(INREG8(0x1FDA) & 0x01));
	}
        outTi3026dreg(TVP3026_CUR_RAM, *src);   
    }
}

static void 
MGA3026ShowCursor()
{
    /* Enable cursor - X11 mode */
    outTi3026(TVP3026_CURSOR_CTL, 0x6c, 0x13);
}

static void
MGA3026HideCursor()
{
    /* Disable cursor */
    outTi3026(TVP3026_CURSOR_CTL, 0xfc, 0x00);
}

static void
MGA3026SetCursorPosition(x, y, xorigin, yorigin)
    int x, y;
{
    if(vga256InfoRec.modes->Flags & V_DBLSCAN)
	y *= 2;

    x += 64 - xorigin;
    y += 64 - yorigin;

    /* Output position - "only" 12 bits of location documented */
   
    outTi3026dreg(TVP3026_CUR_XLOW, x & 0xFF);
    outTi3026dreg(TVP3026_CUR_XHI, (x >> 8) & 0x0F);
    outTi3026dreg(TVP3026_CUR_YLOW, y & 0xFF);
    outTi3026dreg(TVP3026_CUR_YHI, (y >> 8) & 0x0F);
}

static void
MGA3026SetCursorColors(bg, fg)
   int bg, fg;
{
    /* The TI 3026 cursor is always 8 bits so shift 8, not 10 */

    /* Background color */
    outTi3026dreg(TVP3026_CUR_COL_ADDR, 1);
    outTi3026dreg(TVP3026_CUR_COL_DATA, (bg & 0x00FF0000) >> 16);
    outTi3026dreg(TVP3026_CUR_COL_DATA, (bg & 0x0000FF00) >> 8);
    outTi3026dreg(TVP3026_CUR_COL_DATA, (bg & 0x000000FF));

    /* Foreground color */
    outTi3026dreg(TVP3026_CUR_COL_ADDR, 2);
    outTi3026dreg(TVP3026_CUR_COL_DATA, (fg & 0x00FF0000) >> 16);
    outTi3026dreg(TVP3026_CUR_COL_DATA, (fg & 0x0000FF00) >> 8);
    outTi3026dreg(TVP3026_CUR_COL_DATA, (fg & 0x000000FF));
}


void
MGA3026RamdacInit()
{
    MGAdac.isHwCursor		= TRUE;
    MGAdac.CursorMaxWidth	= 64;
    MGAdac.CursorMaxHeight	= 64;
    MGAdac.SetCursorColors	= MGA3026SetCursorColors;
    MGAdac.SetCursorPosition	= MGA3026SetCursorPosition;
    MGAdac.LoadCursorImage	= MGA3026LoadCursorImage;
    MGAdac.HideCursor		= MGA3026HideCursor;
    MGAdac.ShowCursor		= MGA3026ShowCursor;
    MGAdac.CursorFlags		= USE_HARDWARE_CURSOR |
				HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
				HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
				HARDWARE_CURSOR_PROGRAMMED_ORIGIN |
				HARDWARE_CURSOR_AND_SOURCE_WITH_MASK | 	 
				HARDWARE_CURSOR_CHAR_BIT_FORMAT |
				HARDWARE_CURSOR_PROGRAMMED_BITS;


    if ( MGAchipset == PCI_CHIP_MGA2064 && MGABios2.PinID == 0 )
    {
	switch( MGABios.RamdacType & 0xff )
	{
	case 1: MGAdac.maxPixelClock = 220000;
	    break;
	case 2: MGAdac.maxPixelClock = 250000;
	    break;
	default:
	    MGAdac.maxPixelClock = 175000;
	    break;
	}
	/* Set MCLK based on amount of memory */
	if ( vga256InfoRec.videoRam < 4096 )
	    MGAdac.MemoryClock = MGABios.ClkBase * 10;
	else if ( vga256InfoRec.videoRam < 8192 )
	    MGAdac.MemoryClock = MGABios.Clk4MB * 10;
	else
	    MGAdac.MemoryClock = MGABios.Clk8MB * 10;
    }
    else
    {
	if ( MGABios2.PinID ) 	/* make sure BIOS is available */
	{
	    if ( MGABios2.PclkMax != 0xff )
	    {
		MGAdac.maxPixelClock = (MGABios2.PclkMax + 100) * 1000;
	    }
	    else
		MGAdac.maxPixelClock = 220000;

	    /* make sure we are not overdriving the GE for the amount of WRAM */
	    switch ( vga256InfoRec.videoRam )
	    {
		case 4096:
		    if (MGABios2.Clk4MB != 0xff)
			MGABios2.ClkGE = MGABios2.Clk4MB;
		    break;
		case 8192:
		    if (MGABios2.Clk8MB != 0xff)
			MGABios2.ClkGE = MGABios2.Clk8MB;
		    break;
		case 12288:
		    if (MGABios2.Clk12MB != 0xff)
			MGABios2.ClkGE = MGABios2.Clk12MB;
		    break;
		case 16384:
		    if (MGABios2.Clk16MB != 0xff)
			MGABios2.ClkGE = MGABios2.Clk16MB;
		    break;
		default:
		    break;
	    }

#if DEBUG
		ErrorF("ClkGE %d ClkMem %d\n",MGABios2.ClkGE,MGABios2.ClkMem);
#endif
		if ( MGABios2.ClkGE != 0xff && MGABios2.ClkMem == 0xff )
		    MGABios2.ClkMem = MGABios2.ClkGE;
		else if ( MGABios2.ClkGE == 0xff && MGABios2.ClkMem != 0xff )
		    ; /* don't need to do anything */
		else if ( MGABios2.ClkGE == MGABios2.ClkMem && MGABios2.ClkGE != 0xff )
		    MGABios2.ClkMem = MGABios2.ClkGE;
		else
		    MGABios2.ClkMem = 60;

		MGAdac.MemoryClock = MGABios2.ClkMem * 1000;

	    } /* BIOS enabled initialization */
	    else
	    {
		/* bios is not available, initialize to rational figures */
		MGAdac.MemoryClock = 60000;	/* 60 MHz WRAM */
		MGAdac.maxPixelClock = 220000;  /* 220 MHz */
            }
	} /* 2164 specific initialization */

#if MCLK_FROM_XCONFIG
    /* or get it from XF86Config */
    if (vga256InfoRec.MemClk) {
        MGAdac.MemoryClock = vga256InfoRec.MemClk;
#if DEBUG
	ErrorF("From XF86Config MemoryClock %d\n",MGAdac.MemoryClock);
#endif
    }
#endif

    /* safety check. Too slow = corruption, too fast = smoking chips */
    /* 40 MHz (=40000) is a little slower, 50 MHz is safe (and the default */
    /* 60 MHz is pushing it (YMMV), and > 65 is in Danger Will Robinson territory */

    if ( (MGAdac.MemoryClock < 40000) ||
         (MGAdac.MemoryClock > 70000) )
	MGAdac.MemoryClock = 50000; 

#if DEBUG
	ErrorF("Setting MemoryClock %d\n",MGAdac.MemoryClock);
#endif
    MGATi3026SetMCLK( MGAdac.MemoryClock );
}

#ifdef PC98_MGA
void
MGA3026Reset()
{
	static unsigned char initcrtc[] = {
			0x60, 0x4f, 0x50, 0x83, 0x55, 0x81, 0xbf, 0x1f,
			0x00, 0x4f, 0x0e, 0x2f, 0x00, 0x00, 0xff, 0xff,
			0x9c, 0x0e, 0x8f, 0x28, 0x1f, 0x96, 0xb9, 0xa3,
			0xff
	};
	static unsigned char initcrtcext[] = {
			0x00, 0x00, 0x00, 0x80, 0x00, 0x00
	};

	unsigned char tmp;
	int i;

	/* select CLK0 as clock source */
	outTi3026(TVP3026_CLK_SEL, 0, 0x77);

	/* select VGA mode */
	outTi3026(TVP3026_TRUE_COLOR_CTL, 0, 0x80);
	outTi3026(TVP3026_MUX_CTL, 0, 0x98);

	/* set loop and pixel clock PLL PLLEN bits to 0 */
	outTi3026(TVP3026_PLL_ADDR, 0, 0x2A);
	outTi3026(TVP3026_LOAD_CLK_DATA, 0, 0);
	outTi3026(TVP3026_PIX_CLK_DATA, 0, 0);

	/* select 28MHz fixed clock */
	outb(0x3C2, 0x6D);	/* once select PLL */
	outb(0x3C2, 0x65);	/* then select 28MHz */

	/* Pixel clock PLL routed to RCLK */
	outTi3026(TVP3026_MCLK_CTL, 0, 0x18);

	/* set MCLK */
	MGA3026RamdacInit();

	/* nogscale=1 */
	pciWriteLong(MGAPciTag, PCI_OPTION_REG, 0x002C0000);

	/* mgamode=1 */
	outb(0x3DE, 0x03);	/* Select CRTCEXT3 */
	tmp = inb(0x3DF);
	outb(0x3DF, tmp | 0x80);

	/* screen off */
	outb(0x3C4, 0x01);	/* Select SEQ1 */
	tmp = inb(0x3C5);
	outb(0x3C5, tmp | 0x20);

	/* unprotect CRTC registers 0-7 */
	outb(0x3D4, 0x11);
	outb(0x3D5, 0x2f);

	/* set initial CRTC regs value */
	for (i = 0; i <= 24; i++) {
		outb(0x3D4, i);
		outb(0x3D5, initcrtc[i]);
	}

	/* set initial CRTCEXT regs value */
	for (i=0; i<=5; i++) {
		outb(0x3DE, i);
		outb(0x3DF, initcrtcext[i]);
	}

	/* assert soft reset */
	OUTREG(MGAREG_Reset, 1);
	usleep(250);
	OUTREG(MGAREG_Reset, 0);
	usleep(250);

	/* wait vertical retrace */
	while ((inb(0x3DA) & 0x08) != 0x08);

	/* screen on */
	outb(0x3C4, 0x01);	/* Select SEQ1 */
	tmp = inb(0x3C5);
	outb(0x3C5, tmp & ~0x20);

	/* wait vertical retrace */
	while ((inb(0x3DA) & 0x08) != 0x08);
  
	/* set memreset */
	OUTREG(MGAREG_MACCESS, 0x00008000);
	usleep(100);

	/* screen off */
	outb(0x3C4, 0x01);	/* Select SEQ1 */
	tmp = inb(0x3C5);
	outb(0x3C5, tmp | 0x20);

	/*  disable HSYNC,VSYNC */
	outb(0x3DE, 0x01);	/* Select CRTCEXT1 */
	tmp = inb(0x3DF);
	outb(0x3DF, tmp | 0x30);
}
#endif
