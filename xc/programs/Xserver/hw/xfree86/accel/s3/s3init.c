/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3init.c,v 3.103 1996/10/21 05:27:24 dawes Exp $ */
/*
 * Written by Jake Richter Copyright (c) 1989, 1990 Panacea Inc.,
 * Londonderry, NH - All Rights Reserved
 * 
 * This code may be freely incorporated in any program without royalty, as long
 * as the copyright notice stays intact.
 * 
 * Additions by Kevin E. Martin (martin@cs.unc.edu)
 * 
 * KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/*
 * Modified by Amancio Hasty and Jon Tombs
 * 
 */
/* $XConsortium: s3init.c /main/15 1996/01/31 10:04:46 kaleb $ */


#define USE_VGAHWINIT

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "site.h"

#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

#include "s3.h"
#include "regs3.h"
#include "s3Bt485.h"
#include "Ti302X.h"
#include "IBMRGB.h"
#define XCONFIG_FLAGS_ONLY 
#include "xf86_Config.h"

typedef struct {
   vgaHWRec std;                /* good old IBM VGA */
   unsigned char SC1148x;	/* Sierra SC 1148x command register */
   unsigned char SC15025[3];    /* Sierra SC 15025/6 command registers */
   unsigned char ATT490_1;	/* AT&T 20C490/1 command register */
   unsigned char SS2410;	/* Diamond SS2410 command register */
   unsigned char ATT498;	/* AT&T 20C498 command register */
   unsigned char Bt485[4];	/* Bt485 Command Registers 0-3 */
   unsigned char Ti3020[0x40];	/* Ti3020 Indirect Registers 0x0-0x3F */
   unsigned char Ti3025[9];	/* Ti3025 N,M,P for PCLK, MCLK, LOOP PLL */
   unsigned char IBMRGB[0x101];	/* IBM RGB52x registers */
   unsigned char STG1700[5];    /* STG1700 index and command registers */
   unsigned char SDAC[6];       /* S3 SDAC command and PLL registers */
   unsigned char Trio[14];      /* Trio32/64 ext. sequenzer (PLL) registers */
   unsigned char s3reg[10];     /* Video Atribute (CR30-34, CR38-3C) */
   unsigned char s3sysreg[46];  /* Video Atribute (CR40-6D)*/
   unsigned char ColorStack[8]; /* S3 hw cursor color stack CR4A/CR4B */
}
vgaS3Rec, *vgaS3Ptr;

int   vgaIOBase = 0x3d0; /* These defaults are overriden in s3Probe() */
int   vgaCRIndex = 0x3d4;
int   vgaCRReg = 0x3d5;
int   s3InitCursorFlag = TRUE;
int   s3HDisplay;
extern xf86InfoRec xf86Info;

static vgaS3Ptr oldS3 = NULL;

static short savedVgaIOBase;

pointer vgaNewVideoState = NULL;
static LUTENTRY oldlut[256];
static Bool LUTInited = FALSE;
static short s3Initialised = 0;
static int   old_clock;
extern Bool (*s3ClockSelectFunc) ();
extern int s3DisplayWidth;
extern Bool s3Localbus;
extern Bool s3Mmio928;
extern Bool s3NewMmio;
extern Bool s3PixelMultiplexing;
extern pointer vgaBase;
extern pointer vgaBaseLow;
extern pointer vgaBaseHigh;
int currents3dac_border = 0xff;

#ifdef PC98
extern void crtswitch(short);
#endif


#define new ((vgaHWPtr)vgaNewVideoState)

short s3ChipId = 0;
int  s3ChipRev = 0;

#define	cebank() do {							\
   	if (S3_801_928_SERIES(s3ChipId)) {				\
		unsigned char tmp;					\
		outb(vgaCRIndex, 0x51);					\
		tmp = inb(vgaCRReg);					\
		outb(vgaCRReg, (tmp & 0xf3));				\
	}								\
} while (1 == 0)

/*
 * Registers to save/restore in the 0x50 - 0x5f control range
 */

static short reg50_mask = 0x673b; /* was 0x6733; */ /* was 0x4023 */

extern unsigned char s3Port54;
extern unsigned char s3Port51;
extern unsigned char s3Port40;
extern unsigned char s3Port59;
extern unsigned char s3Port5A;
extern unsigned char s3Port31;

void
s3CleanUp(void)
{
   int   i;
   unsigned char c, tmp;

   UNLOCK_SYS_REGS;
   
   vgaProtect(TRUE);

   if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) &&
       !OFLG_ISSET(OPTION_NOLINEAR_MODE, &s3InfoRec.options) &&
       s3Mmio928) {
     /*
       Clear bit 7 of CRTC register 5C to map video memory normally.
     */
     int CR5C;
     outb(vgaCRIndex, 0x5C);
     CR5C = inb(vgaCRReg);
     outb(vgaCRIndex, 0x5C);
     outb(vgaCRReg, CR5C & ~0x80);
     vgaBase = vgaBaseLow;
   }
   /* BL */
   if (s3NewMmio)	{
      outb (vgaCRIndex, 0x58);
      outb (vgaCRReg, s3SAM256); /* disable linear mode */
   } /* end BL */


   WaitQueue(8);
   outb(vgaCRIndex, 0x35);
   tmp = inb(vgaCRReg);
   outb(vgaCRReg, (tmp & 0xf0));
   cebank();

   outw(ADVFUNC_CNTL, 0);
   if (s3Mmio928 || s3NewMmio) {
      outb(vgaCRIndex, 0x53);
      outb(vgaCRReg, 0x00);
   }

 /* (s3ClockSelectFunc)(restore->std.NoClock); */

   /*
    * Restore Sierra SC1148x command register.
    */
   if (DAC_IS_SC1148x_SERIES) {
       xf86setdaccomm(oldS3->SC1148x);
   }
   
   /*
    * Restore AT&T 20C490/1 command register.
    */
   if (DAC_IS_ATT490) {
       xf86setdaccomm(oldS3->ATT490_1);
   }

   /*
    * Restore Diamond SS2410 command register.
    */
   if ( DAC_IS_SS2410 ) {
 	outb( vgaCRIndex, 0x55 );
	tmp = inb( vgaCRReg );
	outb( vgaCRReg, tmp | 1 ); 
	xf86setdaccomm( oldS3->SS2410 );
	outb( vgaCRReg, tmp );
   }
   
   /*
    * Restore AT&T 20C498 command register.
    */
   if (DAC_IS_ATT498 || DAC_IS_ATT20C409) {
      xf86setdaccomm(oldS3->ATT498);
   }

   /*
    * Restore STG 1700 command and extended registers.
    */
   if (DAC_IS_STG1700)
   {
      xf86dactopel();
      xf86setdaccommbit(0x10);   /* enable extended registers */
      xf86dactocomm();
      inb(0x3c6);                /* command reg */

      outb(0x3c6, 0x03);         /* index low */
      outb(0x3c6, 0x00);         /* index high */
      
      outb(0x3c6, oldS3->STG1700[1]);  /* primary pixel mode */
      outb(0x3c6, oldS3->STG1700[2]);  /* secondary pixel mode */
      outb(0x3c6, oldS3->STG1700[3]);  /* PLL control */
      usleep(500);		       /* PLL settling time */

      xf86dactopel();
      xf86setdaccomm(oldS3->STG1700[0]);
   }

   /* Restore S3 SDAC Command and PLL registers */
#if !defined(PC98_PW) && !defined(PC98_PWLB)
   if (DAC_IS_SDAC || DAC_IS_GENDAC)
   {
      outb(vgaCRIndex, 0x55);
      tmp = inb(vgaCRReg);
      outb(vgaCRReg, tmp | 1);

      outb(0x3c6, oldS3->SDAC[0]);      /* Enhanced command register */
      outb(0x3c8, 2);                   /* index to f2 reg */
      outb(0x3c9, oldS3->SDAC[3]);      /* f2 PLL M divider */
      outb(0x3c9, oldS3->SDAC[4]);      /* f2 PLL N1/N2 divider */
      outb(0x3c8, 0x0e);                /* index to PLL control */
      outb(0x3c9, oldS3->SDAC[5]);      /* PLL control */
      outb(0x3c8, oldS3->SDAC[2]);      /* PLL write index */
      outb(0x3c7, oldS3->SDAC[1]);      /* PLL read index */

      outb(vgaCRReg, tmp & ~1);
   }
#endif
   
   /* Restore S3 Trio32/64 ext. sequenzer (PLL) registers */
   if (DAC_IS_TRIO)
   {
      outb(0x3c2, oldS3->Trio[0]);
      outb(0x3c4, 0x08); outb(0x3c5, 0x06);

      outb(0x3c4, 0x09); outb(0x3c5, oldS3->Trio[2]);
      outb(0x3c4, 0x0a); outb(0x3c5, oldS3->Trio[3]);
      outb(0x3c4, 0x0b); outb(0x3c5, oldS3->Trio[4]);
      outb(0x3c4, 0x0d); outb(0x3c5, oldS3->Trio[5]);

      outb(0x3c4, 0x10); outb(0x3c5, oldS3->Trio[8]); 
      outb(0x3c4, 0x11); outb(0x3c5, oldS3->Trio[9]); 
      outb(0x3c4, 0x12); outb(0x3c5, oldS3->Trio[10]); 
      outb(0x3c4, 0x13); outb(0x3c5, oldS3->Trio[11]); 
      outb(0x3c4, 0x1a); outb(0x3c5, oldS3->Trio[12]); 
      outb(0x3c4, 0x1b); outb(0x3c5, oldS3->Trio[13]); 
      outb(0x3c4, 0x15);
      tmp = inb(0x3c5);
      outb(0x3c4, tmp & ~0x20);
      outb(0x3c4, tmp |  0x20);
      outb(0x3c4, tmp & ~0x20);

      outb(0x3c4, 0x15); outb(0x3c5, oldS3->Trio[6]); 
      outb(0x3c4, 0x18); outb(0x3c5, oldS3->Trio[7]);

      outb(0x3c4, 0x08); outb(0x3c5, oldS3->Trio[1]);

   }
   
   /*
    * Restore Sierra SC 15025/6 registers.
    */
   if (DAC_IS_SC15025) {
      c=xf86getdaccomm();
      xf86setdaccomm( c | 0x10 );  /* set internal register access */
      (void)xf86dactocomm();
      outb(0x3c7, 0x8);   /* Auxiliary Control Register */
      outb(0x3c8, oldS3->SC15025[1]);
      outb(0x3c7, 0x10);  /* Pixel Repack Register */
      outb(0x3c8, oldS3->SC15025[2]);
      xf86setdaccomm( c );
      xf86setdaccomm(oldS3->SC15025[0]);
   }

   /*
    * Restore Bt485 registers
    */
   if (DAC_IS_BT485_SERIES) {

      /* Turn off parallel mode explicitly here */
      if (s3Bt485PixMux) {
         if (OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
             S3_928_ONLY(s3ChipId))
	 {
	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, 0x20);
	    outb(0x3C7, 0x00);
	    /* set s3 reg53 to non-parallel addressing by and'ing 0xDF     */
            outb(vgaCRIndex, 0x53);
            tmp = inb(vgaCRReg);
            outb(vgaCRReg, tmp & 0xDF);
	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, 0x00);
         }
         if (OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options) &&
             S3_928_ONLY(s3ChipId))
	 {
	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, 0x40); /* XXXXXXXXXXXXXXXXXXXXX */
	    outb(0x3C7, 0x00);
	    /* set s3 reg53 to non-parallel addressing by and'ing 0xDF     */
            outb(vgaCRIndex, 0x53);
            tmp = inb(vgaCRReg);
            outb(vgaCRReg, tmp & 0xDF);
         }
      }
	 
      s3OutBtReg(BT_COMMAND_REG_0, 0xFE, 0x01);
      s3OutBtRegCom3(0x00, oldS3->Bt485[3]);
      if (s3Bt485PixMux) {
	 s3OutBtReg(BT_COMMAND_REG_2, 0x00, oldS3->Bt485[2]);
	 s3OutBtReg(BT_COMMAND_REG_1, 0x00, oldS3->Bt485[1]);
      }
      s3OutBtReg(BT_COMMAND_REG_0, 0x00, oldS3->Bt485[0]);
   }

   /*
    * Restore Ti3020 registers
    */
   if (DAC_IS_TI3020_SERIES) {
      s3OutTiIndReg(TI_MUX_CONTROL_1, 0x00, oldS3->Ti3020[TI_MUX_CONTROL_1]);
      s3OutTiIndReg(TI_MUX_CONTROL_2, 0x00, oldS3->Ti3020[TI_MUX_CONTROL_2]);
      s3OutTiIndReg(TI_INPUT_CLOCK_SELECT, 0x00,
		    oldS3->Ti3020[TI_INPUT_CLOCK_SELECT]);
      s3OutTiIndReg(TI_OUTPUT_CLOCK_SELECT, 0x00,
		    oldS3->Ti3020[TI_OUTPUT_CLOCK_SELECT]);
      s3OutTiIndReg(TI_GENERAL_CONTROL, 0x00,
		    oldS3->Ti3020[TI_GENERAL_CONTROL]);
      s3OutTiIndReg(TI_AUXILIARY_CONTROL, 0x00,
		    oldS3->Ti3020[TI_AUXILIARY_CONTROL]);
      s3OutTiIndReg(TI_GENERAL_IO_CONTROL, 0x00, 0x1f);
      s3OutTiIndReg(TI_GENERAL_IO_DATA, 0x00,
		    oldS3->Ti3020[TI_GENERAL_IO_DATA]);
   }
   if (DAC_IS_TI3025) {
      s3OutTiIndReg(TI_PLL_CONTROL, 0x00, 0x00);
      s3OutTiIndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[0]);  /* N */
      s3OutTiIndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[1]);  /* M */
      s3OutTiIndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[2]);  /* P */

      s3OutTiIndReg(TI_MCLK_PLL_DATA, 0x00, oldS3->Ti3025[3]);         /* N */
      s3OutTiIndReg(TI_MCLK_PLL_DATA, 0x00, oldS3->Ti3025[4]);         /* M */
      s3OutTiIndReg(TI_MCLK_PLL_DATA, 0x00, oldS3->Ti3025[5] | 0x80 ); /* P */

      s3OutTiIndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[6]);  /* N */
      s3OutTiIndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[7]);  /* M */
      s3OutTiIndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[8]);  /* P */

      s3OutTiIndReg(TI_TRUE_COLOR_CONTROL, 0x00,
                    oldS3->Ti3020[TI_TRUE_COLOR_CONTROL]);
      s3OutTiIndReg(TI_MISC_CONTROL, 0x00, oldS3->Ti3020[TI_MISC_CONTROL]);
   }
   if (DAC_IS_TI3026 || DAC_IS_TI3030) {
      /* select pixel clock PLL as dot clock soure */
      s3OutTi3026IndReg(TI_INPUT_CLOCK_SELECT, 0x00, TI_ICLK_PLL);

      /* programm dot clock PLL to new MCLK frequency */
      s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x00);
      s3OutTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[3]);
      s3OutTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[4]);
      s3OutTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[5]);

      /* wait until PLL is locked */
      for (i=0; i<10000; i++)
	 if (s3InTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA) & 0x40) 
	    break;

      /* switch to output dot clock on the MCLK terminal */
      s3OutTi3026IndReg(0x39, 0xe7, 0x00);
      s3OutTi3026IndReg(0x39, 0xe7, 0x08);
      
      /* Set MCLK */
      s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x00);
      s3OutTi3026IndReg(TI_MCLK_PLL_DATA, 0x00, oldS3->Ti3025[3]);
      s3OutTi3026IndReg(TI_MCLK_PLL_DATA, 0x00, oldS3->Ti3025[4]);
      s3OutTi3026IndReg(TI_MCLK_PLL_DATA, 0x00, oldS3->Ti3025[5]);

      /* wait until PLL is locked */
      for (i=0; i<10000; i++) 
	 if (s3InTi3026IndReg(TI_MCLK_PLL_DATA) & 0x40) 
	    break;

      /* switch to output MCLK on the MCLK terminal */
      s3OutTi3026IndReg(0x39, 0xe7, 0x10);
      s3OutTi3026IndReg(0x39, 0xe7, 0x18);


      /* restore dot clock PLL */

      s3OutTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[0]);
      s3OutTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[1]);
      s3OutTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[2]);

      s3OutTi3026IndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[6]);
      s3OutTi3026IndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[7]);
      s3OutTi3026IndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[8]);

      
      s3OutTi3026IndReg(TI_CURS_CONTROL, 0x00, 
			oldS3->Ti3020[TI_CURS_CONTROL]);
      s3OutTi3026IndReg(TI_INPUT_CLOCK_SELECT, 0x00,
			oldS3->Ti3020[TI_INPUT_CLOCK_SELECT]);
      s3OutTi3026IndReg(TI_MUX_CONTROL_1, 0x00,
			oldS3->Ti3020[TI_MUX_CONTROL_1]);
      s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00,
			oldS3->Ti3020[TI_MUX_CONTROL_2]);
      s3OutTi3026IndReg(TI_COLOR_KEY_CONTROL, 0x00, 
			oldS3->Ti3020[TI_COLOR_KEY_CONTROL]);
      s3OutTi3026IndReg(TI_GENERAL_CONTROL, 0x00, 
			oldS3->Ti3020[TI_GENERAL_CONTROL]);
      s3OutTi3026IndReg(TI_MISC_CONTROL, 0x00, 
			oldS3->Ti3020[TI_MISC_CONTROL]);
      s3OutTi3026IndReg(TI_MCLK_LCLK_CONTROL, 0x00, 
			oldS3->Ti3020[TI_MCLK_LCLK_CONTROL]);
      s3OutTi3026IndReg(TI_GENERAL_IO_CONTROL, 0x00, 
			oldS3->Ti3020[TI_GENERAL_IO_CONTROL]);
      s3OutTi3026IndReg(TI_GENERAL_IO_DATA, 0x00, 
			oldS3->Ti3020[TI_GENERAL_IO_DATA]);
   }
   if (DAC_IS_TI3020_SERIES)
      s3OutTiIndReg(TI_CURS_CONTROL, 0x00, oldS3->Ti3020[TI_CURS_CONTROL]);
   if (DAC_IS_TI3025) {
      outb(vgaCRIndex, 0x5C);
      outb(vgaCRReg, oldS3->s3sysreg[0x0C + 16]);
   }
   if (DAC_IS_IBMRGB) {
      if (DAC_IS_IBMRGB525) {
	 s3OutIBMRGBIndReg(IBMRGB_vram_mask_0, 0, 3);
	 s3OutIBMRGBIndReg(IBMRGB_misc1, ~0x40, 0x40);
	 usleep (1000);
	 s3OutIBMRGBIndReg(IBMRGB_misc2, ~1, 0);
      }
      for (i=0; i<0x100; i++)
	 s3OutIBMRGBIndReg(i, 0, oldS3->IBMRGB[i]);
      outb(vgaCRIndex, 0x22);
      outb(vgaCRReg, oldS3->IBMRGB[0x100]);      
   }

 /* restore s3 special bits */
   if (S3_801_928_SERIES(s3ChipId)) {
    /* restore 801 specific registers */

      for (i = 32; i < (S3_x64_SERIES(s3ChipId) ? 46 : 
                      S3_805_I_SERIES(s3ChipId) ? 40 : 38) ; i++) {
	 outb(vgaCRIndex, 0x40 + i);
	 outb(vgaCRReg, oldS3->s3sysreg[i]);

      }
      for (i = 0; i < 16; i++) {
	 if (!((1 << i) & reg50_mask))
	   continue;
	 outb(vgaCRIndex, 0x50 + i);
	 outb(vgaCRReg, oldS3->s3sysreg[i + 16]);
      }
   }
   for (i = 0; i < 5; i++) {
      outb(vgaCRIndex, 0x30 + i);
      outb(vgaCRReg, oldS3->s3reg[i]);
      outb(vgaCRIndex, 0x38 + i);
      outb(vgaCRReg, oldS3->s3reg[5 + i]);
   }

   for (i = 0; i < 16; i++) {
      outb(vgaCRIndex, 0x40 + i);
      outb(vgaCRReg, oldS3->s3sysreg[i]);
   }

   outb(vgaCRIndex, 0x45);
   inb(vgaCRReg);         /* reset color stack pointer */
   outb(vgaCRIndex, 0x4A);
   for (i = 0; i < 4; i++)
      outb(vgaCRReg, oldS3->ColorStack[i]);

   outb(vgaCRIndex, 0x45);
   inb(vgaCRReg);         /* reset color stack pointer */
   outb(vgaCRIndex, 0x4B);
   for (i = 4; i < 8; i++)
      outb(vgaCRReg, oldS3->ColorStack[i]);


   if (OFLG_ISSET(CLOCK_OPTION_ICS2595, &s3InfoRec.clockOptions)){
      outb(vgaCRIndex, 0x42);
      outb(vgaCRReg, (oldS3->s3sysreg[2] & 0xf0) | 0x01);
      outb(vgaCRIndex, 0x5c);	/* switch back to 28MHz clock */
      outb(vgaCRReg,   0x20);
      outb(vgaCRReg,   0x00);
   }

   vgaHWRestore((vgaHWPtr)oldS3);

   outb(0x3c2, old_clock);

   i = inb(0x3CC);
   if (savedVgaIOBase == 0x3B0)
      i &= 0xFE;
   else
      i |= 0x01;
   outb(0x3C2, i);

   vgaIOBase = savedVgaIOBase;
   vgaCRIndex = vgaIOBase + 4;
   vgaCRReg = vgaIOBase + 5;
      
   vgaProtect(FALSE);

#ifdef PC98
	crtswitch(0);
#endif

   xf86DisableIOPorts(s3InfoRec.scrnIndex);
}

Bool
s3Init(mode)
     DisplayModePtr mode;
{
   short i, m, n;
   int   interlacedived = mode->Flags & V_INTERLACE ? 2 : 1;
   unsigned char tmp, tmp1, tmp2, CR5C;
   unsigned int itmp;
   extern Bool s3DAC8Bit, s3DACSyncOnGreen;
   int pixMuxShift = 0;

   s3HDisplay = mode->HDisplay;

   UNLOCK_SYS_REGS;

   /* Force use of colour I/O address */
   if (!s3Initialised) {
      savedVgaIOBase = vgaIOBase;
   }
   i = inb(0x3CC);
   outb(0x3C2, i | 0x01);
   vgaIOBase = 0x3D0;
   vgaCRIndex = 0x3D4;
   vgaCRReg = 0x3D5;
      
   if (!s3Initialised) {
    /* blanket save of state */
    /* unlock */
      outb(vgaCRIndex, 0x38);
      outb(vgaCRReg, 0x48);
      old_clock = inb(0x3CC);

      outb(vgaCRIndex, 0x35);	/* select segment 0 */
      i = inb(vgaCRReg);
      outb(vgaCRReg, i & 0xf0);
      cebank();

      oldS3 = vgaHWSave((vgaHWPtr)oldS3, sizeof(vgaS3Rec));

      /*
       * Set up the Serial Access Mode 256 Words Control
       *   (bit 6 in CR58)
       */
      if (S3_968_SERIES(s3ChipId) || (S3_964_SERIES(s3ChipId) &&
	  !OFLG_ISSET(OPTION_NUMBER_NINE, &s3InfoRec.options)))
         s3SAM256 = 0x40;
      else if ((OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
               S3_928_ONLY(s3ChipId)) ||
	       OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options))
         s3SAM256 = 0x80; /* set 6 MCLK cycles for R/W time on Mercury */
      else
         s3SAM256 = 0x00;

      /*
       * Save Sierra SC1148x command register.
       */
      if (DAC_IS_SC1148x_SERIES) {
         oldS3->SC1148x = xf86getdaccomm();
      }

      /*
       * Save AT&T 20C490/1 command register.
       */
      if (DAC_IS_ATT490) {
         oldS3->ATT490_1 = xf86getdaccomm();
      }

      /*
       * Save Diamond SS2410 command register.
       */
      if ( DAC_IS_SS2410 ) {
 	  outb( vgaCRIndex, 0x55 );
	  tmp = inb( vgaCRReg );
	  outb( vgaCRReg, tmp | 1 ); 
	  oldS3->SS2410 = xf86getdaccomm( );
	  outb( vgaCRReg, tmp );
       }
   


      /*
       * Save AT&T 20C498 command register.
       */
      if (DAC_IS_ATT498 || DAC_IS_ATT20C409) {
         oldS3->ATT498 = xf86getdaccomm();
      }

      /*
       * Save STG 1700 command and extended registers.
       */
      if (DAC_IS_STG1700) {
         xf86dactopel();
         oldS3->STG1700[0] = xf86getdaccomm();

         xf86setdaccommbit(0x10);   /* enable extended registers */
         xf86dactocomm();
         inb(0x3c6);                /* command reg */

         outb(0x3c6, 0x03);         /* index low */
         outb(0x3c6, 0x00);         /* index high */

         oldS3->STG1700[1] = inb(0x3c6);  /* primary pixel mode */
         oldS3->STG1700[2] = inb(0x3c6);  /* secondary pixel mode */
         oldS3->STG1700[3] = inb(0x3c6);  /* PLL control */

         xf86dactopel();
      }

      /* Save S3 SDAC Command and PLL registers */
#if !defined(PC98_PW) && !defined(PC98_PWLB)
      if (DAC_IS_SDAC || DAC_IS_GENDAC)
      {
         outb(vgaCRIndex, 0x55);
	 tmp = inb(vgaCRReg);
         outb(vgaCRReg, tmp | 1);

         oldS3->SDAC[0] = inb(0x3c6);      /* Enhanced command register */
         oldS3->SDAC[2] = inb(0x3c8);      /* PLL write index */
         oldS3->SDAC[1] = inb(0x3c7);      /* PLL read index */
         outb(0x3c7, 2);                   /* index to f2 reg */
         oldS3->SDAC[3] = inb(0x3c9);      /* f2 PLL M divider */
         oldS3->SDAC[4] = inb(0x3c9);      /* f2 PLL N1/N2 divider */
         outb(0x3c7, 0x0e);                /* index to PLL control */
         oldS3->SDAC[5] = inb(0x3c9);      /* PLL control */

         outb(vgaCRReg, tmp & ~1);
      }
#endif
   
      /* Save S3 Trio32/64 ext. sequenzer (PLL) registers */
      if (DAC_IS_TRIO)
      {
	 oldS3->Trio[0] = inb(0x3cc);

	 outb(0x3c4, 0x08); oldS3->Trio[1] = inb(0x3c5);
	 outb(0x3c5, 0x06);

	 outb(0x3c4, 0x09); oldS3->Trio[2]  = inb(0x3c5);
	 outb(0x3c4, 0x0a); oldS3->Trio[3]  = inb(0x3c5);
	 outb(0x3c4, 0x0b); oldS3->Trio[4]  = inb(0x3c5);
	 outb(0x3c4, 0x0d); oldS3->Trio[5]  = inb(0x3c5);
	 outb(0x3c4, 0x15); oldS3->Trio[6]  = inb(0x3c5) & 0xfe; 
	 outb(0x3c5, oldS3->Trio[6]);
	 outb(0x3c4, 0x18); oldS3->Trio[7]  = inb(0x3c5);

	 outb(0x3c4, 0x10); oldS3->Trio[8]  = inb(0x3c5);
	 outb(0x3c4, 0x11); oldS3->Trio[9]  = inb(0x3c5);
	 outb(0x3c4, 0x12); oldS3->Trio[10] = inb(0x3c5);
	 outb(0x3c4, 0x13); oldS3->Trio[11] = inb(0x3c5);
	 outb(0x3c4, 0x1a); oldS3->Trio[12] = inb(0x3c5);
	 outb(0x3c4, 0x1b); oldS3->Trio[13] = inb(0x3c5);

	 outb(0x3c4, 8);
	 outb(0x3c5, 0x00);
      }

      /*
       * Save Sierra SC15025/6 command registers.
       */
      LOCK_SYS_REGS;
      if (DAC_IS_SC15025) {
         oldS3->SC15025[0] = xf86getdaccomm();
	 xf86setdaccomm((oldS3->SC15025[0] | 0x10));
         (void)xf86dactocomm();
	 outb(0x3c7,0x8);   /* Auxiliary Control Register */
	 oldS3->SC15025[1] = inb(0x3c8);
	 outb(0x3c7,0x10);  /* Pixel Repack Register */
	 oldS3->SC15025[2] = inb(0x3c8);
	 xf86setdaccomm(oldS3->SC15025[0]);
      }
      UNLOCK_SYS_REGS;
      /*
       * Save Bt485 Registers
       */
      if (DAC_IS_BT485_SERIES) {
	 oldS3->Bt485[0] = s3InBtReg(BT_COMMAND_REG_0);
	 if (s3Bt485PixMux) {
	    oldS3->Bt485[1] = s3InBtReg(BT_COMMAND_REG_1);
	    oldS3->Bt485[2] = s3InBtReg(BT_COMMAND_REG_2);
	 }
	 oldS3->Bt485[3] = s3InBtRegCom3();
      }

      /*
       * Save Ti3020 registers
       */

      if (DAC_IS_TI3025) {
          /* switch the ramdac from bt485 to ti3020 mode clearing RS4 */
	  outb(vgaCRIndex, 0x5C);
	  CR5C = inb(vgaCRReg);
	  outb(vgaCRReg, CR5C & 0xDF);

          oldS3->Ti3020[TI_CURS_CONTROL] = s3InTiIndReg(TI_CURS_CONTROL);
          /* clear TI_PLANAR_ACCESS bit */
	  s3OutTiIndReg(TI_CURS_CONTROL, 0x7F, 0x00);
      }
      if (DAC_IS_TI3020_SERIES) {
          oldS3->Ti3020[TI_MUX_CONTROL_1] = s3InTiIndReg(TI_MUX_CONTROL_1);
          oldS3->Ti3020[TI_MUX_CONTROL_2] = s3InTiIndReg(TI_MUX_CONTROL_2);
          oldS3->Ti3020[TI_INPUT_CLOCK_SELECT] =
                 s3InTiIndReg(TI_INPUT_CLOCK_SELECT);
          oldS3->Ti3020[TI_OUTPUT_CLOCK_SELECT] =
                 s3InTiIndReg(TI_OUTPUT_CLOCK_SELECT);
          oldS3->Ti3020[TI_GENERAL_CONTROL] = s3InTiIndReg(TI_GENERAL_CONTROL);
          oldS3->Ti3020[TI_AUXILIARY_CONTROL] =
		 s3InTiIndReg(TI_AUXILIARY_CONTROL);
          s3OutTiIndReg(TI_GENERAL_IO_CONTROL, 0x00, 0x1f);
          oldS3->Ti3020[TI_GENERAL_IO_DATA] = s3InTiIndReg(TI_GENERAL_IO_DATA);
      }
      if (DAC_IS_TI3025) {
          oldS3->Ti3020[TI_TRUE_COLOR_CONTROL] =
                 s3InTiIndReg(TI_TRUE_COLOR_CONTROL);
          oldS3->Ti3020[TI_MISC_CONTROL] = s3InTiIndReg(TI_MISC_CONTROL);

          s3OutTiIndReg(TI_PLL_CONTROL, 0x00, 0x00);
          oldS3->Ti3025[0] = s3InTiIndReg(TI_PIXEL_CLOCK_PLL_DATA);  /* N */
          s3OutTiIndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[0]);
          oldS3->Ti3025[1] = s3InTiIndReg(TI_PIXEL_CLOCK_PLL_DATA);  /* M */
          s3OutTiIndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[1]);
          oldS3->Ti3025[2] = s3InTiIndReg(TI_PIXEL_CLOCK_PLL_DATA);  /* P */
          s3OutTiIndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[2]);

          oldS3->Ti3025[3] = s3InTiIndReg(TI_MCLK_PLL_DATA);  /* N */
          s3OutTiIndReg(TI_MCLK_PLL_DATA, 0x00, oldS3->Ti3025[3]);
          oldS3->Ti3025[4] = s3InTiIndReg(TI_MCLK_PLL_DATA);  /* M */
          s3OutTiIndReg(TI_MCLK_PLL_DATA, 0x00, oldS3->Ti3025[4]);
          oldS3->Ti3025[5] = s3InTiIndReg(TI_MCLK_PLL_DATA);  /* P */
          s3OutTiIndReg(TI_MCLK_PLL_DATA, 0x00, oldS3->Ti3025[5]);

          oldS3->Ti3025[6] = s3InTiIndReg(TI_LOOP_CLOCK_PLL_DATA);  /* N */
          s3OutTiIndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[6]);
          oldS3->Ti3025[7] = s3InTiIndReg(TI_LOOP_CLOCK_PLL_DATA);  /* M */
          s3OutTiIndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[7]);
          oldS3->Ti3025[8] = s3InTiIndReg(TI_LOOP_CLOCK_PLL_DATA);  /* P */
          s3OutTiIndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, oldS3->Ti3025[8]);
      }
      if (DAC_IS_TI3026 || DAC_IS_TI3030) {
	  for (i=0; i<0x40; i++) 
	     oldS3->Ti3020[i] = s3InTi3026IndReg(i);

          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x00);
          oldS3->Ti3025[0] = s3InTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA);
          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x01);
          oldS3->Ti3025[1] = s3InTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA);
          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x02);
          oldS3->Ti3025[2] = s3InTi3026IndReg(TI_PIXEL_CLOCK_PLL_DATA);

          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x00);
          oldS3->Ti3025[3] = s3InTi3026IndReg(TI_MCLK_PLL_DATA);
          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x04);
          oldS3->Ti3025[4] = s3InTi3026IndReg(TI_MCLK_PLL_DATA);
          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x08);
          oldS3->Ti3025[5] = s3InTi3026IndReg(TI_MCLK_PLL_DATA);

          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x00);
          oldS3->Ti3025[6] = s3InTi3026IndReg(TI_LOOP_CLOCK_PLL_DATA);
          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x10);
          oldS3->Ti3025[7] = s3InTi3026IndReg(TI_LOOP_CLOCK_PLL_DATA);
          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x20);
          oldS3->Ti3025[8] = s3InTi3026IndReg(TI_LOOP_CLOCK_PLL_DATA);
          s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x00);
      }
      if (DAC_IS_IBMRGB) {
	 for (i=0; i<0x100; i++)
	    oldS3->IBMRGB[i] = s3InIBMRGBIndReg(i);
	 outb(vgaCRIndex, 0x22);
	 oldS3->IBMRGB[0x100] = inb(vgaCRReg);
      }

      for (i = 0; i < 5; i++) {
	 outb(vgaCRIndex, 0x30 + i);
	 oldS3->s3reg[i] = inb(vgaCRReg);
#ifdef REG_DEBUG
	 ErrorF("CR%X = 0x%02x\n", 0x30 + i, oldS3->s3reg[i]);
#endif
	 outb(vgaCRIndex, 0x38 + i);
	 oldS3->s3reg[5 + i] = inb(vgaCRReg);
#ifdef REG_DEBUG
	 ErrorF("CR%X = 0x%02x\n", 0x38 + i, oldS3->s3reg[i + 5]);
#endif
      }

      outb(vgaCRIndex, 0x11);	/* allow writting? */
      outb(vgaCRReg, 0x00);
      for (i = 0; i < 16; i++) {
	 outb(vgaCRIndex, 0x40 + i);
	 oldS3->s3sysreg[i] = inb(vgaCRReg);
#ifdef REG_DEBUG
	 ErrorF("CR%X = 0x%02x\n", 0x40 + i, oldS3->s3sysreg[i]);
#endif
      }

      outb(vgaCRIndex, 0x45);
      inb(vgaCRReg);         /* reset color stack pointer */
      outb(vgaCRIndex, 0x4A);
      for (i = 0; i < 4; i++) {
	 oldS3->ColorStack[i] = inb(vgaCRReg);
	 outb(vgaCRReg,oldS3->ColorStack[i]);  /* advance stack pointer */
      }
      
      outb(vgaCRIndex, 0x45);
      inb(vgaCRReg);         /* reset color stack pointer */
      outb(vgaCRIndex, 0x4B);
      for (i = 4; i < 8; i++) {
	 oldS3->ColorStack[i] = inb(vgaCRReg);
	 outb(vgaCRReg,oldS3->ColorStack[i]);  /* advance stack pointer */
      }

      if (S3_801_928_SERIES(s3ChipId)) 
         for (i = 0; i < 16; i++) {
#ifdef REG_DEBUG
	     outb(vgaCRIndex, 0x50 + i);
	     ErrorF("CR%X = 0x%02x\n", 0x50 + i, inb(vgaCRReg));
#endif
	     if (!((1 << i) & reg50_mask))
	       continue;
	     outb(vgaCRIndex, 0x50 + i);
	     oldS3->s3sysreg[i + 16] = inb(vgaCRReg);
          }
      if (DAC_IS_TI3025)  /* restore 5C from above */
         oldS3->s3sysreg[0x0C + 16] = CR5C;

      for (i = 32; i < (S3_x64_SERIES(s3ChipId) ? 46 : 
                      S3_805_I_SERIES(s3ChipId) ? 40 : 38); i++) {
	 outb(vgaCRIndex, 0x40 + i);
	 oldS3->s3sysreg[i] = inb(vgaCRReg);
#ifdef REG_DEBUG
	 ErrorF("CR%X = 0x%02x\n", 0x40 + i, oldS3->s3sysreg[i]);
#endif
      }

      s3Initialised = 1;
      vgaNewVideoState = vgaHWSave(vgaNewVideoState, sizeof(vgaS3Rec));
      outb(DAC_MASK, 0);

   } else if (DAC_IS_TI3025) {
      /* switch the ramdac from bt485 to ti3020 mode clearing RS4 */
      outb(vgaCRIndex, 0x5C);
      CR5C = inb(vgaCRReg);
      outb(vgaCRReg, CR5C & 0xDF);

      /* clear TI_PLANAR_ACCESS bit */
      s3OutTiIndReg(TI_CURS_CONTROL, 0x7F, 0x00);
   }

   if (s3UsingPixMux && (mode->Flags & V_PIXMUX))
      s3PixelMultiplexing = TRUE;
   else
      s3PixelMultiplexing = FALSE;

   if (OFLG_ISSET(OPTION_ELSA_W2000PRO, &s3InfoRec.options))
      pixMuxShift = s3InfoRec.clock[mode->Clock] > 120000 ? 2 : 
		      s3InfoRec.clock[mode->Clock] > 60000 ? 1 : 0 ;
   else if ((mode->Flags & V_DBLCLK)
	    && (DAC_IS_TI3026) 
	    && (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions)))
      pixMuxShift =  (s3Bpp <= 2) ? 2 : 1;
   else if ((mode->Flags & V_DBLCLK) && DAC_IS_TI3030)
      pixMuxShift =  1;
   else if (S3_964_SERIES(s3ChipId) && DAC_IS_IBMRGB)
      pixMuxShift = mode->Flags & V_DBLCLK ? 1 : 0;
   else if (S3_964_SERIES(s3ChipId) && DAC_IS_TI3025)
      pixMuxShift =  mode->Flags & V_DBLCLK ? 1 : 0;
   else if (S3_964_SERIES(s3ChipId) && DAC_IS_BT485_SERIES)
      /* Stealth64 and Miro Crystal 20SV */
      pixMuxShift =  mode->Flags & V_DBLCLK ? 1 : 0;
   else if (S3_801_928_SERIES(s3ChipId) && DAC_IS_SC15025)
      pixMuxShift = -(s3Bpp>>1);  /* for 16/32 bpp */
   else if (S3_864_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId))
	    /* && (DAC_IS_ATT498 || DAC_IS_STG1700) */
      pixMuxShift = -(s3Bpp>>1);  /* for 16/32 bpp */
   else if (S3_TRIOxx_SERIES(s3ChipId))
      pixMuxShift = -(s3Bpp == 2);
   else if (S3_x64_SERIES(s3ChipId)) /* XXXX Better to test the DAC type? */
      pixMuxShift = 0;
   else if ((S3_928_SERIES(s3ChipId) && 
	     (DAC_IS_TI3020 || DAC_IS_BT485_SERIES)) &&
            s3PixelMultiplexing) {
      if (s3Bpp == 4)      pixMuxShift = 0;  /* 32 bit */
      else if (s3Bpp == 2) pixMuxShift = 1;  /* 16 bit */
      else                 pixMuxShift = 2;  /*  8 bit */
   } else if (s3PixelMultiplexing)  
      pixMuxShift = 2; /* old default for   if (s3PixelMultiplexing) shifting */
   else 
      pixMuxShift = 0;

   if (!mode->CrtcHAdjusted) {
      if (s3Bpp == 3 && 
	  (   (S3_968_SERIES(s3ChipId) && DAC_IS_TI3026)
	   || (S3_968_SERIES(s3ChipId) && DAC_IS_TI3030))) {	 
	 mode->CrtcHTotal     = (mode->CrtcHTotal     * 3) / 4;
	 mode->CrtcHDisplay   = (mode->CrtcHDisplay   * 3) / 4;
	 mode->CrtcHSyncStart = (mode->CrtcHSyncStart * 3) / 4;
	 mode->CrtcHSyncEnd   = (mode->CrtcHSyncEnd   * 3) / 4;
	 mode->CrtcHSkew      = (mode->CrtcHSkew      * 3) / 4;
      }
      if (pixMuxShift > 0) {
	 /* now divide the horizontal timing parameters as required */
	 mode->CrtcHTotal     >>= pixMuxShift;
	 mode->CrtcHDisplay   >>= pixMuxShift;
	 mode->CrtcHSyncStart >>= pixMuxShift;
	 mode->CrtcHSyncEnd   >>= pixMuxShift;
	 mode->CrtcHSkew      >>= pixMuxShift;
      }
      else if (pixMuxShift < 0) {
	 /* now multiply the horizontal timing parameters as required */
	 mode->CrtcHTotal     <<= -pixMuxShift;
	 mode->CrtcHDisplay   <<= -pixMuxShift;
	 mode->CrtcHSyncStart <<= -pixMuxShift;
	 mode->CrtcHSyncEnd   <<= -pixMuxShift;
	 mode->CrtcHSkew      <<= -pixMuxShift;
      }
      mode->CrtcHAdjusted = TRUE;
   }

   /* 
    * do some sanity checks on the horizontal timing parameters 
    */
   { 
      Bool changed=FALSE;
      int oldCrtcHSyncStart, oldCrtcHSyncEnd, oldCrtcHTotal;
      int p24_fact = 4;

      oldCrtcHSyncStart = mode->CrtcHSyncStart;
      oldCrtcHSyncEnd   = mode->CrtcHSyncEnd;
      oldCrtcHTotal     = mode->CrtcHTotal;
      if (mode->CrtcHTotal > 4096) {  /*  CrtcHTotal/8  is a 9 bit value */
	 mode->CrtcHTotal = 4096;
	 changed = TRUE;
      }
      if (mode->CrtcHSyncEnd >= mode->CrtcHTotal) {
	 mode->CrtcHSyncEnd = mode->CrtcHTotal - 1;
	          changed = TRUE;
      }
      if (mode->CrtcHSyncStart >= mode->CrtcHSyncEnd) {
	 mode->CrtcHSyncStart = mode->CrtcHSyncEnd - 1;
         changed = TRUE;
      }
      if (DAC_IS_TI3030 && s3Bpp==1) {
	 /* for 128bit bus we need multiple of 16 8bpp pixels... */
	 if (mode->CrtcHTotal & 0x0f) {
	    mode->CrtcHTotal = (mode->CrtcHTotal + 0x0f) & ~0x0f;
	    changed = TRUE;
	 }
      }
      if (s3Bpp == 3) {
	 /* for packed 24bpp CrtcHTotal must be multiple of 3*8... */
	 if ((mode->CrtcHTotal >> 3) % 3 != 0) {
	    mode->CrtcHTotal >>= 3;
	    mode->CrtcHTotal += 3 - mode->CrtcHTotal % 3;
	    mode->CrtcHTotal <<= 3;
	    changed = TRUE;
	    p24_fact = 3;
	 }
      }
      if (changed) {
	 ErrorF("%s %s: mode line has to be modified ...\n",
		XCONFIG_PROBED, s3InfoRec.name);
	 ErrorF("\t\tfrom   %4d %4d %4d %4d   %4d %4d %4d %4d\n"
		,mode->HDisplay, mode->HSyncStart, mode->HSyncEnd, mode->HTotal
		,mode->VDisplay, mode->VSyncStart, mode->VSyncEnd, mode->VTotal
		);
	 ErrorF("\t\tto     %4d %4d %4d %4d   %4d %4d %4d %4d\n"
		,((mode->CrtcHDisplay   << 8) >> (8-pixMuxShift)) * 4 / p24_fact
		,((mode->CrtcHSyncStart << 8) >> (8-pixMuxShift)) * 4 / p24_fact
		,((mode->CrtcHSyncEnd   << 8) >> (8-pixMuxShift)) * 4 / p24_fact
		,((mode->CrtcHTotal     << 8) >> (8-pixMuxShift)) * 4 / p24_fact
		,mode->VDisplay, mode->VSyncStart, mode->VSyncEnd, mode->VTotal
		);
      }
   }
   if (!vgaHWInit(mode, sizeof(vgaS3Rec)))
      return(FALSE);

   new->MiscOutReg |= 0x0C;		/* enable CR42 clock selection */
   new->Sequencer[0] = 0x03;		/* XXXX shouldn't need this */
   new->CRTC[19] = s3BppDisplayWidth >> 3;
   new->CRTC[23] = 0xE3;		/* XXXX shouldn't need this */
   new->Attribute[0x11] = currents3dac_border; /* The overscan colour AR11 gets */
					/* disabled anyway */

   if ((S3_TRIO64V_SERIES(s3ChipId) && (s3ChipRev <= 0x531) && (s3Bpp==1)) ^
       !!OFLG_ISSET(OPTION_TRIO64VP_BUG2, &s3InfoRec.options)) {
      /* set correct blanking for broken Trio64V+ to avoid bright left border:
	 blank signal needs to go off ~400 usec before video signal starts 
	 w/o border:  blank_shift = 0 */
      int blank_shift = 400 * s3InfoRec.clock[mode->Clock] / 1000000 / 8;
      new->CRTC[2]  = mode->CrtcHDisplay >> 3;
      new->CRTC[3] &= ~0x1f;
      new->CRTC[3] |=  ((mode->CrtcHTotal >> 3) - 2 - blank_shift) & 0x1f;
      new->CRTC[5] &= ~0x80;
      new->CRTC[5] |= (((mode->CrtcHTotal >> 3) - 2 - blank_shift) & 0x20) << 2;
      
      new->CRTC[21] =  (mode->CrtcVDisplay - 1) & 0xff; 
      new->CRTC[7] &= ~0x08;
      new->CRTC[7] |= ((mode->CrtcVDisplay - 1) & 0x100) >> 5;
      new->CRTC[9] &= ~0x20;
      new->CRTC[9] |= ((mode->CrtcVDisplay - 1) & 0x200) >> 4;

      new->CRTC[22] = (mode->CrtcVTotal - 2) & 0xFF;
   }

   vgaProtect(TRUE);

   if (vgaIOBase == 0x3B0)
      new->MiscOutReg &= 0xFE;
   else
      new->MiscOutReg |= 0x01;

   if (OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) && 
       S3_928_ONLY(s3ChipId)) {
      /*
       * Make sure that parallel option is already set correctly before
       * changing the clock doubler state.
       * XXXX maybe the !s3PixelMultiplexing bit is not required?
       */
      if (s3PixelMultiplexing) {
	 outb(vgaCRIndex, 0x5C);
	 outb(vgaCRReg, 0x20);
	 outb(0x3C7, 0x21);
	 /* set s3 reg53 to parallel addressing by or'ing 0x20		*/
	 outb(vgaCRIndex, 0x53);
	 tmp = inb(vgaCRReg);
	 outb(vgaCRReg, tmp | 0x20);  
	 outb(vgaCRIndex, 0x5C);
	 outb(vgaCRReg, 0x00);
      } else {
	 outb(vgaCRIndex, 0x5C);
	 outb(vgaCRReg, 0x20);
	 outb(0x3C7, 0x00); 
	 /* set s3 reg53 to non-parallel addressing by and'ing 0xDF	*/
	 outb(vgaCRIndex, 0x53);
	 tmp = inb(vgaCRReg);
	 outb(vgaCRReg, tmp & 0xDF);
	 outb(vgaCRIndex, 0x5C);
	 outb(vgaCRReg, 0x00);
      }
      if (s3Bpp == 4) 
         s3OutTi3026IndReg(TI_LATCH_CONTROL, ~1, 1);  /* 0x06 -> 0x07 */
      else
         s3OutTi3026IndReg(TI_LATCH_CONTROL, ~1, 0);  /* 0x06 */
   }

   if(    (DAC_IS_TI3026 || DAC_IS_TI3030) 
       && (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions))){
      /*
       * for the boards with Ti3026 and external ICD2061A clock chip we
       * need to enable clock doubling, if necessary
       */
      if( mode->Flags & V_DBLCLK ) {
#ifdef EXTENDED_DEBUG
	 ErrorF("Putting Ti3026 into external double clock mode\n");
#endif
         s3OutTi3026IndReg(TI_INPUT_CLOCK_SELECT,0x00,0x08);
      }
      else {
#ifdef EXTENDED_DEBUG
	 ErrorF("Putting Ti3026 into external clock mode\n");
#endif
         s3OutTi3026IndReg(TI_INPUT_CLOCK_SELECT,0x00,0x00);
      }
      if (s3Bpp == 4) 
         s3OutTi3026IndReg(TI_LATCH_CONTROL, ~1, 1);  /* 0x06 -> 0x07 */
      else
         s3OutTi3026IndReg(TI_LATCH_CONTROL, ~1, 0);  /* 0x06 */
   }

   if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) &&
       !OFLG_ISSET(OPTION_NOLINEAR_MODE, &s3InfoRec.options) &&
       s3Mmio928) {
     /*
       Set bit 7 of CRTC register 5C to map video memory with
       LAW31-26 = 011111 rather than 000000.  Note that this remaps
       all addresses seen by the 928, including the VGA Base Address.
     */
     int CR5C;
     outb(vgaCRIndex, 0x5C);
     CR5C = inb(vgaCRReg);
     outb(vgaCRIndex, 0x5C);
     outb(vgaCRReg, CR5C | 0x80);
     vgaBase = vgaBaseHigh;
   }

   if (OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options) &&
       !OFLG_ISSET(OPTION_NOLINEAR_MODE, &s3InfoRec.options) &&
       s3Mmio928) {
     int CR5C;
     outb(vgaCRIndex, 0x5C);
     CR5C = inb(vgaCRReg);
     outb(vgaCRIndex, 0x5C);
       switch(s3InfoRec.depth) {
       case 24:
       case 32:
	 outb(vgaCRReg, CR5C | 0xf0);
	 break;
       case 16:
       case 15:
	 outb(vgaCRReg, CR5C | 0x70);
	 break;
       default:
	 outb(vgaCRReg, CR5C | 0xa0);
       }
       vgaBase = vgaBaseHigh;
}

   /* Don't change the clock bits when using an external clock program */

   if (new->NoClock < 0) {
      tmp = inb(0x3CC);
      new->MiscOutReg = (new->MiscOutReg & 0xF3) | (tmp & 0x0C);
   } else {
      /* XXXX Should we really do something about the return value? */
      if (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions))
	 (void) (s3ClockSelectFunc)(mode->SynthClock);
      else
         (void) (s3ClockSelectFunc)(mode->Clock);
     
      if ((mode->Flags & V_DBLCLK)
	 && (DAC_IS_TI3026 || DAC_IS_TI3030) 
	 && (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions))){
	 if (s3Bpp <= 2)
	    Ti3026SetClock(mode->SynthClock / 2, 2, s3Bpp, TI_LOOP_CLOCK);
	 else 
	    Ti3026SetClock(mode->SynthClock, 2, s3Bpp, TI_LOOP_CLOCK);	    
	 s3OutTi3026IndReg(TI_MCLK_LCLK_CONTROL, ~0x20, 0x20);
      }
      if ((DAC_IS_TI3026 || DAC_IS_TI3030) && s3Bpp == 3) {
	 s3OutTi3026IndReg(TI_MCLK_LCLK_CONTROL, ~0x20, 0x20);
      }
   }

   /*
    * Set Sierra SC1148x command register for 8/15/16 bpp
    */

   if (DAC_IS_SC1148x_SERIES) {
      if (s3InfoRec.bitsPerPixel == 8) {
         xf86clrdaccommbit(0xe0);
      } else {
        if (s3InfoRec.depth == 16 && DAC_IS_SC1148x_M3) {
	   xf86setdaccommbit(0xe0);
        } else if (s3InfoRec.depth == 15) {
           xf86setdaccommbit(0xa0);
        } else {
	   ErrorF("unsupported mode!\n");
        }
      }
   }

   /*
    * Set AT&T 20C490/1 command register to 8-bit mode if desired.
    */

   if (DAC_IS_ATT490) {

      if (s3InfoRec.bitsPerPixel == 8) {
	if (s3DAC8Bit) {
         xf86setdaccomm(0x02);
	} else {
	 xf86setdaccomm(0x00);
	}
      } else {
	switch (s3InfoRec.depth) {
	  case 15:
		xf86setdaccomm(0xa0);
                break;
          case 16:
		xf86setdaccomm(0xc0);
                break;
          case 24:
          case 32:
		xf86setdaccomm(0xe0);  /* XXXX just a guess, check !!! */
                break;
          default:
		ErrorF("unsupported mode!\n");
	}
      }    
   }

   /*
    * Set the Diamond SS2410 command register to desired settings. 
    */
   if ( DAC_IS_SS2410 ) {
	if ( s3InfoRec.bitsPerPixel == 8 ) {
 		outb( vgaCRIndex, 0x55 );
		tmp = inb( vgaCRReg );
		outb( vgaCRReg, tmp | 1 ); 
		xf86setdaccomm( 0 );
		outb( vgaCRReg, tmp );
		}
	else {
		switch ( s3InfoRec.depth ) { 
			case 15:
 				outb( vgaCRIndex, 0x55 );
				tmp = inb( vgaCRReg );
				outb( vgaCRReg, tmp | 1 ); 
				xf86setdaccomm( 0xA0 );
				outb( vgaCRReg, tmp );
				break;
			case 16:
 				outb( vgaCRIndex, 0x55 );
				tmp = inb( vgaCRReg );
				outb( vgaCRReg, tmp | 1 ); 
				xf86setdaccomm( 0xA6 );
				outb( vgaCRReg, tmp );
				break;
			case 24:
 				outb( vgaCRIndex, 0x55 );
				tmp = inb( vgaCRReg );
				outb( vgaCRReg, tmp | 1 ); 
				xf86setdaccomm( 0x9E );
				outb( vgaCRReg, tmp );
				break;
			default:
				ErrorF("unsupported mode! \n");
			}
		}
   	}
   
   /*
    * Set AT&T 20C498/409/499 command register to 8-bit mode if desired.
    */
   if (DAC_IS_ATT498 || DAC_IS_ATT20C409) {
      if (s3DAC8Bit) {
         xf86setdaccommbit(0x02);
      } else {
	 xf86clrdaccommbit(0x02);
      }
   }

   /*
    * Set STG1700 command register to 8-bit mode if desired.
    */
   if (DAC_IS_STG1700) {
      if (s3DAC8Bit) {
         xf86setdaccommbit(0x02);
      } else {
	 xf86clrdaccommbit(0x02);
      }
   }

   /* NO 8bit mode for S3 SDAC or GENDAC or Trio32/64 */

   /*
    * Set Sierra SC 15025/6 command registers to 8-bit mode if desired.
    */
   if (DAC_IS_SC15025) {
      unsigned char aux=0, comm=0, prr=0;

      LOCK_SYS_REGS;
      if (s3DAC8Bit || s3InfoRec.bitsPerPixel > 8) aux=1;
      switch (s3InfoRec.bitsPerPixel) {
      case 8: 
	 comm = 0;  /* repack mode 0, color mode 0 */
	 break;
      case 16:
	 if (s3Weight == RGB16_555) {
	    comm = 0x80;  /* repack mode 1a using both clock edges */
	 }
	 else {  /* RGB16_565 */
	    comm = 0xc0;  /* repack mode 1a using both clock edges */
	 }
	 break;
      case 32:
	 comm = 0x40;  /* repack mode 3a using both clock edges */
	 prr = 1;
	 break;
      default:
	 ;
      }
      
      comm |= 0x08;  /* enable LUT for gamma correction */

      xf86setdaccomm(comm | 0x10);
      outb(0x3c7,0x8);
      outb(0x3c8,aux);
      outb(0x3c7,0x10);
      outb(0x3c8,prr);
      xf86setdaccomm(comm);
      UNLOCK_SYS_REGS;
   }

   if (DAC_IS_ATT498 || DAC_IS_ATT20C409) {
      outb(0x3C4, 1);
      tmp2 = inb(0x3C5);
      outb(0x3C5, tmp2 | 0x20); /* blank the screen */

      if (s3PixelMultiplexing) { /* x64:pixmux */
	 /* pixmux with 16/32 bpp not possible for 864 ==> only 8bit mode  */
	 int daccomm;
	 tmp = xf86getdaccomm();
	 
	 if (DAC_IS_ATT22C498) {
	    if (s3InfoRec.clock[mode->Clock]/2 < 22500) daccomm = 0x20;
	    else if (s3InfoRec.clock[mode->Clock]/2 < 45000) daccomm = 0x21;
	    else daccomm = 0x24;
#if 0
	    /* using digital clock doubler; 20C498 compatible */
	    daccomm = 0x25;
#endif
	 }
	 else daccomm = 0x20;

#ifdef EXTENDED_DEBUG
	 ErrorF("Putting AT&T 2xC4[09][89] RAMDAC into pixmux\n");
#endif
	 xf86setdaccomm( (tmp&0x02) | daccomm );  /* set mode 2,
						  pixel multiplexing on */

	 if ( ! DAC_IS_ATT20C409 ) {
	    outb(vgaCRIndex, 0x33);	/* set VCLK = -DCLK */
	    tmp = inb(vgaCRReg);
	    outb(vgaCRReg, tmp | 0x08 );
	 }
	 
	 if (S3_x64_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId)) {
	    outb(vgaCRIndex, 0x67);	/* set Mode 8: Two 8-bit color
	    				   1 VCLK / 2 pixels */
	    if (    OFLG_ISSET(OPTION_NUMBER_NINE, &s3InfoRec.options)
	         || DAC_IS_ATT20C409 )
	       outb(vgaCRReg, 0x10 );	/* VCLK is out of phase with DCLK */
	    else
	       outb(vgaCRReg, 0x11 );	/* VCLK is in phase with DCLK */

	    outb(vgaCRIndex, 0x6d);
	    outb(vgaCRReg, 2 );     /* delay -BLANK pulse by 2 DCLKs */
	 }
	 else {
	    /* don't know */
	 }
      }
      else { /* !s3PixelMultiplexing */
	 outb(vgaCRIndex, 0x33);
	 tmp = inb(vgaCRReg);
	 outb(vgaCRReg, tmp &  ~0x08 );

	 tmp = xf86getdaccomm() & 0x0f;

	 if (S3_x64_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId)) {
	    int invert_vclk = 0;
	    int delay_blank = 0;
	    outb(vgaCRIndex, 0x67);

	    switch (s3InfoRec.bitsPerPixel) {
	    case 8: /* set Mode  0:  8-bit color, 1 VCLK/pixel */
	       outb(vgaCRReg, 0x00 | invert_vclk); 
	       xf86setdaccomm(tmp | 0x00);  /* set mode 0 */
	       break;
	    case 16: 
	       if (s3Weight == RGB16_555) {
		  outb(vgaCRReg, 0x30 | invert_vclk); /* set Mode 9:
						15-bit color, 1 VCLK/pixel */
		  xf86setdaccomm(tmp | 0x10);  /* set mode 1 */
	       }
	       else {
		  outb(vgaCRReg, 0x50 | invert_vclk); /* set Mode 10:
						16-bit color, 1 VCLK/pixel */
		  xf86setdaccomm(tmp | 0x30);  /* set mode 3 */
	       }
	       delay_blank = 2;
	       break;
	    case 32: /* set Mode 11: 24/32-bit color, 2 VCLK/pixel */
	       outb(vgaCRReg, 0x70 | invert_vclk);  
	       xf86setdaccomm(tmp | 0x50);  /* set mode 5 */
	       if (mode->HDisplay > 640)   /* why not for 640 ?  tsk */
		  delay_blank = 2;
	       break;
	    default:
	       ;
	    }
	    outb(vgaCRIndex, 0x6d);
	    outb(vgaCRReg, delay_blank);
	 }
	 else {
	    /* don't know */
	 }
      }  /* end of s3PixelMultiplexing */

      outb(0x3C4, 1);
      outb(0x3C5, tmp2);        /* unblank the screen */
   }


   if (DAC_IS_STG1700) {
      int daccomm = (xf86getdaccomm() & 0x06) | 0x10;

      outb(0x3C4, 1);
      tmp2 = inb(0x3C5);
      outb(0x3C5, tmp2 | 0x20); /* blank the screen */

      if (s3PixelMultiplexing) {
	 /* x64:pixmux */
	 /* pixmux with 16/32 bpp not possible for 864 ==> only 8bit mode  */
         daccomm |= 0x08;           /* enable extended pixel modes */
	 xf86setdaccomm(daccomm);   /* pixel multiplexing on */

         xf86dactocomm();
         inb(0x3c6);                /* command reg */

         outb(0x3c6, 0x03);         /* index low */
         outb(0x3c6, 0x00);         /* index high */
      
         outb(0x3c6, 0x05);         /* primary pixel mode */
         outb(0x3c6, 0x05);         /* secondary pixel mode */
         outb(0x3c6, 0x02);         /* PLL control for 64-135 MHz pixclk */
         usleep(500);		    /* PLL settling time before LUT access */

         xf86dactopel();

	 outb(vgaCRIndex, 0x33);
	 tmp = inb(vgaCRReg);
	 outb(vgaCRReg, tmp | 0x08 );
	 
	 if (S3_x64_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId)) {
	    outb(vgaCRIndex, 0x67);
	    outb(vgaCRReg, 0x11 );   /* set Mode 5: double 8-bit indexed color,
					  1 VCLK/2 pixels */
	    outb(vgaCRIndex, 0x6d);
	    outb(vgaCRReg, 2 );     /* delay -BLANK pulse by 2 DCLKs */
	 }
	 else {
	    /* don't know */
	 }
      }
      else 
      { /* !s3PixelMultiplexing */
	 outb(vgaCRIndex, 0x33);
	 tmp = inb(vgaCRReg);
	 outb(vgaCRReg, tmp &  ~0x08 );

	 if (S3_x64_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId)) {
	    int invert_vclk = 0;
	    int delay_blank = 0;
            int pixmode     = 0;
            int s3mux       = 0;

	    outb(vgaCRIndex, 0x67);

	    switch (s3InfoRec.bitsPerPixel) 
            {
	       case 8: /* set Mode  0:  8-bit indexed color, 1 VCLK/pixel */
	          daccomm |= 0x00;
                  s3mux    = 0x00 | invert_vclk; 
	          break;

       	       case 16: 
	          if (s3Weight == RGB16_555) 
                  {  /* set Mode 2: 15-bit direct color */
	             daccomm |= 0xa8;
                     pixmode  = 0x02;
                     s3mux    = 0x30 | invert_vclk; 
	          }
	          else 
                  {  /* set Mode 3: 16-bit (565) direct color */
	             daccomm |= 0xc8;
                     pixmode  = 0x03;
                     s3mux    = 0x50 | invert_vclk; 
	          }
	          delay_blank = 2;
	          break;

	       case 32: /* set Mode 4: 24-bit direct color, 2 VCLK/pixel */
	          daccomm |= 0xe8;
                  pixmode  = 0x04;
                  s3mux    = 0x70 | invert_vclk; 
	          delay_blank = 2;
	          break;

	       default:
	          ErrorF("default switch 2\n");
	    }

	    outb(vgaCRReg, s3mux);  
	    xf86setdaccomm(daccomm);

            xf86dactocomm();
            inb(0x3c6);                /* command reg */

            outb(0x3c6, 0x03);         /* index low */
            outb(0x3c6, 0x00);         /* index high */
      
            outb(0x3c6, pixmode);      /* primary pixel mode */
            outb(0x3c6, pixmode);      /* secondary pixel mode */

            xf86dactopel();

	    outb(vgaCRIndex, 0x6d);
	    outb(vgaCRReg, delay_blank);
	 }
	 else {
	    /* don't know */
	 }
      }  /* end of s3PixelMultiplexing */

      outb(0x3C4, 1);
      outb(0x3C5, tmp2);        /* unblank the screen */
   }

   if (DAC_IS_SDAC)
   {
      int pixmux = 0;           /* SDAC command and CR67 */
      int blank_delay = 0;      /* CR6D */
      int invert_vclk = 0;      /* CR66 bit 0 */
      unsigned char tmp3;

      outb(0x3C4, 1);
      tmp2 = inb(0x3C5);
      outb(0x3C5, tmp2 | 0x20); /* blank the screen */

      if (s3PixelMultiplexing)
      {
	 /* x64:pixmux */
	 /* pixmux with 16/32 bpp not possible for 864 ==> only 8bit mode  */
         pixmux = 0x10;         /* two 8bit pixels per clock */
	 if (!S3_866_SERIES(s3ChipId) && !S3_868_SERIES(s3ChipId))
	    invert_vclk = 1;
         blank_delay = 2;
      }
      else
      {
         switch (s3InfoRec.bitsPerPixel) 
         {
            case 8:  /* 8-bit color, 1 VCLK/pixel */
               break;

            case 16: /* 15/16-bit color, 1VCLK/pixel */
               if (s3Weight == RGB16_555)
                  pixmux = 0x30;
               else
                  pixmux = 0x50;
               blank_delay = 2;
               break;

            case 32: /* 32-bit color, 2VCLK/pixel */
               pixmux = 0x70;
               blank_delay = 2;
         }
      }

      outb(vgaCRIndex, 0x55);
      tmp3 = inb(vgaCRReg);
      outb(vgaCRReg, tmp3 | 1);

      outb(vgaCRIndex, 0x67);
      outb(vgaCRReg, pixmux | invert_vclk);    /* set S3 mux mode */
      outb(0x3c6, pixmux);                     /* set SDAC mux mode */

      outb(vgaCRIndex, 0x6D);
      outb(vgaCRReg, blank_delay);             /* set blank delay */

      outb(vgaCRIndex, 0x55);
      tmp3 = inb(vgaCRReg);
      outb(vgaCRReg, tmp3 & ~1);

      outb(0x3C4, 1);
      outb(0x3C5, tmp2);        /* unblank the screen */
   }

   if (DAC_IS_GENDAC)
   {
      int daccomm = 0;           /* GENDAC command */
      unsigned char tmp3;

      outb(0x3C4, 1);
      tmp2 = inb(0x3C5);
      outb(0x3C5, tmp2 | 0x20); /* blank the screen */

      switch (s3InfoRec.bitsPerPixel) 
      {
         case 8:  /* 8-bit color, 1 VCLK/pixel */
            break;

         case 16: /* 15/16-bit color, 2VCLK/pixel */
            if (s3Weight == RGB16_555)
               daccomm = 0x20;
            else
               daccomm = 0x60;
            break;

         case 32: /* 32-bit color, 3VCLK/pixel */
            daccomm = 0x40;
      }

      outb(vgaCRIndex, 0x55);
      tmp3 = inb(vgaCRReg);
      outb(vgaCRReg, tmp3 | 1);

      outb(0x3c6, daccomm);                     /* set GENDAC mux mode */

      outb(vgaCRIndex, 0x55);
      tmp3 = inb(vgaCRReg);
      outb(vgaCRReg, tmp3 & ~1);

      outb(0x3C4, 1);
      outb(0x3C5, tmp2);        /* unblank the screen */
   }

   if (DAC_IS_TRIO)
   {
      int pixmux = 0;           /* SDAC command and CR67 */
      int invert_vclk = 0;      /* CR66 bit 0 */
      int sr8, sr15, sr18, cr33;
      
      outb(0x3C4, 1);
      tmp2 = inb(0x3C5);
      outb(0x3C5, tmp2 | 0x20); /* blank the screen */

      outb(0x3c4, 0x08);
      sr8 = inb(0x3c5);
      outb(0x3c5, 0x06);

      outb(0x3c4, 0x0d);        /* fix for VideoLogic GrafixStar cards: */
      tmp = inb(0x3c5);         /* disable feature connector to use 64bit DRAM bus */
      outb(0x3c5, tmp & ~1);

      outb(0x3c4, 0x15);
      sr15 = inb(0x3c5) & ~0x10;  /* XXXX ~0x40 and SynthClock /= 2 in s3.c might be better... */
      outb(0x3c4, 0x18);
      sr18 = inb(0x3c5) & ~0x80;
      outb(vgaCRIndex, 0x33);
      cr33 = inb(vgaCRReg) & ~0x28;

      /* for Trio64+ we need corrected blank signal timing */
      if (!(S3_TRIO64V_SERIES(s3ChipId) && (s3ChipRev <= 0x531)) ^ 
	  !!OFLG_ISSET(OPTION_TRIO64VP_BUG1, &s3InfoRec.options)) {
	 cr33 |= 0x20;
      }

      if (s3PixelMultiplexing)
      {
	 /* x64:pixmux */
	 /* pixmux with 16/32 bpp not possible for 864 ==> only 8bit mode  */
         pixmux = 0x10;         /* two 8bit pixels per clock */
         invert_vclk = 2;       /* XXXX strange: reserved bit which helps! */
	 sr15 |= 0x10;  /* XXXX 0x40? see above! */
	 sr18 |= 0x80;
      }
      else
      {
         switch (s3InfoRec.bitsPerPixel) 
         {
            case 8:  /* 8-bit color, 1 VCLK/pixel */
               break;

            case 16: /* 15/16-bit color, 1VCLK/pixel */
	       cr33 |= 0x08;
               if (s3Weight == RGB16_555)
                  pixmux = 0x30;
               else
                  pixmux = 0x50;
               break;

            case 32: /* 32-bit color, 2VCLK/pixel */
               pixmux = 0xd0;
         }
      }

      outb(vgaCRReg, cr33);

      outb(vgaCRIndex, 0x67);
      outb(vgaCRReg, pixmux | invert_vclk);    /* set S3 mux mode */

      outb(0x3c4, 0x15);
      outb(0x3c5, sr15);
      outb(0x3c4, 0x18);
      outb(0x3c5, sr18);
      outb(0x3c4, 0x08);
      outb(0x3c5, sr8);

      outb(0x3C4, 1);
      outb(0x3C5, tmp2);        /* unblank the screen */
   }

   if (DAC_IS_BT485_SERIES) {
      outb(0x3C4, 1);
      tmp2 = inb(0x3C5);
      outb(0x3C5, tmp2 | 0x20); /* blank the screen */
      s3OutBtReg(BT_COMMAND_REG_0, 0xFE, 0x01); /* sleep mode */
   }

   if (s3Bt485PixMux) {
      if (s3PixelMultiplexing) {
         /* fun timing mods for pixel-multiplexing!                     */

         /*
	   Pixel Multiplexing is selected for 16bpp, 32bpp, or 8bpp
	   with Width > 1024.  Pixel Multiplexing requires we also
	   Select Parallel VRAM Addressing (CR53.5), and Parallel
	   VRAM Addressing also requires a line width of 1024 or
	   2048, external SID enabled (CR55.3), and split transfers
	   disabled (CR51.6).
	 */
	 if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) ||
	     OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options)) {
	   outb(vgaCRIndex, 0x53);
	   tmp = inb(vgaCRReg);
	   outb(vgaCRReg, tmp | 0x20);
	 }

	 if (OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
              S3_928_ONLY(s3ChipId))	
	 {
	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, 0x20);
	    outb(0x3C7, 0x21);

            /* set s3 reg53 to parallel addressing by or'ing 0x20          */
            outb(vgaCRIndex, 0x53);
            tmp = inb(vgaCRReg);
            outb(vgaCRReg, tmp | 0x20);

	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, 0x00);
	 }

         /* set s3 reg55 to external serial by or'ing 0x08              */
         outb(vgaCRIndex, 0x55);
         tmp = inb(vgaCRReg); /* XXXX Something should be masked here */
	 if (s3InfoRec.bitsPerPixel == 32 &&
	     !OFLG_ISSET(OPTION_MIRO_MAGIC_S4,&s3InfoRec.options))  /* 24bpp truecolor */
 	   tmp |= 0x48;
	 else
	   tmp |= 0x08;
         outb(vgaCRReg, tmp);

	 if (S3_964_SERIES(s3ChipId) && DAC_IS_BT485_SERIES) {
	    /* Stealth 64 and Miro Crystal 20SV */
	    outb(vgaCRIndex, 0x66);
	    tmp = inb(vgaCRReg) & 0xc0;
	    if (mode->Flags & V_DBLCLK) {
	       /* Set VCLK = DCLCK/2 */
	       /* And set up a 32 bit interleaved bus */
	       if (s3Bpp == 1)
		  tmp |= 0x11;
	       else
		  tmp |= 0x10; /* 16bpp */
            } else {
	       if (s3Bpp == 1)
		  tmp |= 0x12;
	       else if (s3Bpp == 2)
		  tmp |= 0x11;
	       else
		  tmp |= 0x10;	/* for 20SV, Stealth needs 0x10 ? */
            }
	    outb(vgaCRReg, tmp);

	    /* blank_delay = 0 (at least for Miro Crystal 20SV) */
	    outb(vgaCRIndex, 0x6d);
	    if ((mode->Flags & V_DBLCLK) || s3Bpp > 1) 
	       outb(vgaCRReg, 0);
	    else
	       outb(vgaCRReg, 1);  /* or 2; needed for 20SV with ATT 20C505 */
         }
	 outb(vgaCRIndex, 0x65);
	 tmp = inb(vgaCRReg);

         if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options))
	   /*
	     Setting this register non-zero on the Pegasus causes a wrap of
	     the rightmost pixels back to the left of the display.
	   */
	   outb(vgaCRReg, 0x00);
         else if (!(OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
                    S3_928_ONLY(s3ChipId))) {
	    outb(vgaCRReg, tmp | 0x20);
 	    /* set s3 reg65 for some unknown reason                      */
	    /* Setting this for the SPEA Mercury affects clocks > 120MHz */
	  } else if (OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options)) {
	     /* do nothing */ ;
	  } else if ((s3DisplayWidth >= 1024) || (s3InfoRec.depth == 24)
		     || (s3InfoRec.depth == 32)) {
#ifndef PC98_PW
	    outb(vgaCRReg, tmp | 0x40);
#else
	    outb(vgaCRReg, tmp | 0x08);
#endif
	    /* remove horizontal stripes in 1600/8bpp and 1152/16bpp      */
	    /* 800/32bpp linewidth pixmux modes                           */
	    /* someone should check this for other 928 + Bt485 cards      */
	  } else outb(vgaCRReg, tmp & 0xBF);

         /*
          * set output clocking to 4:1 multiplexing
          */
	 if (s3InfoRec.depth == 24 || s3InfoRec.depth == 32)  /* 24bpp */
	    tmp = 0x10;
	 else if (s3InfoRec.depth == 16)               /* 5-6-5 */
	    tmp = 0x38;
	 else if (s3InfoRec.depth == 15)               /* 5-5-5 */
	    tmp = 0x30;
	 else
	    tmp = 0x40;                                /* 8bpp */
         s3OutBtReg(BT_COMMAND_REG_1, 0x00, tmp);

	 /* SCLK enable,pclk1,pixport	                           */
	 if (mode->Flags & V_INTERLACE)
	    s3OutBtReg(BT_COMMAND_REG_2, 0x00, 0x30 | 0x08);
	 else
	    s3OutBtReg(BT_COMMAND_REG_2, 0x00, 0x30);

      } else {

	 if (OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
             S3_928_ONLY(s3ChipId))
	 {
	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, 0x20);
	    outb(0x3C7, 0x00);
	 }
	 if (OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options) &&
             S3_928_ONLY(s3ChipId))
	 {
	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, 0x20);
	    outb(0x3C7, 0x00);
	 }

         /* set s3 reg53 to non-parallel addressing by and'ing 0xDF     */
         outb(vgaCRIndex, 0x53);
         tmp = inb(vgaCRReg);
	 if (OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) && 
	     S3_928_ONLY(s3ChipId) && (s3Bpp != 1)) {
            outb(vgaCRReg, tmp | 0x20);
	 } else {
            outb(vgaCRReg, tmp & 0xDF);
	 }

         /* set s3 reg65 for some unknown reason                        */
         outb(vgaCRIndex, 0x65);
         tmp = inb(vgaCRReg);
         outb(vgaCRReg, tmp & 0xDF);

	 if (OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
             S3_928_ONLY(s3ChipId))
	 {
	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, 0x00);
	 }

	 if (OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options))
	 {
 	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, 0x00);
	    outb(vgaCRIndex, 0x55);
	    outb(vgaCRReg, 0x20);
	 } else {
	   /* set s3 reg55 to non-external serial by and'ing 0xF7         */
	   outb(vgaCRIndex, 0x55);
	   tmp = inb(vgaCRReg);
	   outb(vgaCRReg, tmp & 0xF7);
	 }
	 
	 if (s3InfoRec.depth == 24 || s3InfoRec.depth == 32)	/* 24bpp */
	    tmp = 0x10;
	 else if (s3InfoRec.depth == 16)		/* 5-6-5 */
	    tmp = 0x3c;					/* 1:1 MUX */
	 else if (s3InfoRec.depth == 15)		/* 5-5-5 */
	    tmp = 0x34;					/* 1:1 MUX */
	 else
	    tmp = 0x00;
         s3OutBtReg(BT_COMMAND_REG_1, 0x00, tmp);

	 if (s3InfoRec.bitsPerPixel > 8)
	    tmp = 0x30;
	 else
	    tmp = 0x10;

	 /* pclk1,vgaport                                               */
	 if (mode->Flags & V_INTERLACE)
	    s3OutBtReg(BT_COMMAND_REG_2, 0x00, tmp | 0x08);
	 else
	    s3OutBtReg(BT_COMMAND_REG_2, 0x00, tmp);

      }  /* end of s3PixelMultiplexing */
   }

   /* Set 6/8 bit mode and sync-on-green if required */
   if (DAC_IS_BT485_SERIES) {
      s3OutBtReg(BT_COMMAND_REG_0, 0x00, 0x01 |
		 (s3DAC8Bit ? 0x02 : 0) | (s3DACSyncOnGreen ? 0x08 : 0x00));
#ifdef CLOCKDEBUG
      if (mode->Flags & V_DBLCLK) {
	 ErrorF("Setting clock doubler in s3Init(), freq = %.3f\n",
		s3InfoRec.clock[mode->Clock] / 1000.0);
      }
#endif
      /* Use Bt485 clock doubler - Bit 3 of Command Reg 3 */
      s3OutBtRegCom3(0xF7, (mode->Flags & V_DBLCLK ? 0x08 : 0x00));
      s3OutBtReg(BT_COMMAND_REG_0, 0xFE, 0x00); /* wake up    */
      outb(0x3C4, 1);
      outb(0x3C5, tmp2); /* unblank the screen */
   }


   if (DAC_IS_TI3020_SERIES) {
      outb(0x3C4, 1);
      tmp2 = inb(0x3C5);
      outb(0x3C5, tmp2 | 0x20); /* blank the screen */

      /* change polarity on S3 to pass through control to the 3020      */
      tmp = new->MiscOutReg; /* BUG tmp = inb(0x3CC); */
      new->MiscOutReg |= 0xC0;
      tmp1 = 0x00;
      if (!(tmp & 0x80)) tmp1 |= 0x02; /* invert bits for the 3020      */
      if (!(tmp & 0x40)) tmp1 |= 0x01;
      if (s3DACSyncOnGreen) tmp1 |= 0x20;  /* add IOG sync              */
      s3OutTiIndReg(TI_GENERAL_CONTROL, 0x00, tmp1);
      s3OutTiIndReg(TI_TRUE_COLOR_CONTROL, 0x00, 0x00);

      if (DAC_IS_TI3020) {
	 /* the 3025 clock programming code sets the input clock select */
         if (mode->Flags & V_DBLCLK)
	    s3OutTiIndReg(TI_INPUT_CLOCK_SELECT, 0x00, TI_ICLK_CLK1_DOUBLE);
         else
	    s3OutTiIndReg(TI_INPUT_CLOCK_SELECT, 0x00, TI_ICLK_CLK1);
      }

      outb(vgaCRIndex, 0x65);
      if (DAC_IS_TI3025) {
	 if (OFLG_ISSET(OPTION_NUMBER_NINE,&s3InfoRec.options)) {
	    outb(vgaCRReg, 0x82);
	 } else {
	    outb(vgaCRReg, 0);
	 }
      } else {
	 /* set s3 reg65 for some unknown reason			*/
	 if (s3InfoRec.bitsPerPixel == 32)
	    outb(vgaCRReg, 0x80);
	 else if (s3InfoRec.bitsPerPixel == 16)
	    outb(vgaCRReg, 0x40);
	 else
	    outb(vgaCRReg, 0x00);
      }

      if (s3PixelMultiplexing) {
	 /* fun timing mods for pixel-multiplexing!                     */

	 if (OFLG_ISSET(OPTION_ELSA_W2000PRO,&s3InfoRec.options)) {
	    /* set CR40 acording to Bernhard Bender */
	    outb(vgaCRIndex, 0x40);
	    outb(vgaCRReg, 0xd1);
	 } else if (DAC_IS_TI3025) {
	    outb(vgaCRIndex, 0x40);
	    outb(vgaCRReg, 0x11);
	    outb(vgaCRIndex, 0x55);
	    outb(vgaCRReg, 0x00);
	 } else {
            /* set s3 reg53 to parallel addressing by or'ing 0x20          */
            outb(vgaCRIndex, 0x53);
            tmp = inb(vgaCRReg);
            outb(vgaCRReg, tmp | 0x20);

            /* set s3 reg55 to external serial by or'ing 0x08              */
            outb(vgaCRIndex, 0x55);
            tmp = inb(vgaCRReg);
            outb(vgaCRReg, tmp | 0x08);
	 }
	 /* the input clock is already set to clk1 or clk1double (s3.c) */

	 if (DAC_IS_TI3025) {
	    if (s3InfoRec.bitsPerPixel > 8)
	       s3OutTiIndReg(TI_AUXILIARY_CONTROL, 0, 0x00);
            else
	       s3OutTiIndReg(TI_AUXILIARY_CONTROL, 0, TI_AUX_W_CMPL);
         } else {
	    /* set aux control to self clocked, window function complement */
	    if (s3InfoRec.bitsPerPixel > 8)
	       s3OutTiIndReg(TI_AUXILIARY_CONTROL, 0, TI_AUX_SELF_CLOCK);
            else
	       s3OutTiIndReg(TI_AUXILIARY_CONTROL, 0,
		          TI_AUX_SELF_CLOCK | TI_AUX_W_CMPL);
         }
	 if (OFLG_ISSET(OPTION_ELSA_W2000PRO,&s3InfoRec.options)) {
   	    int vclock,rclock;

	    /*
	     * The 964 needs different VCLK division depending on the 
	     * clock frequenzy used. VCLK/1 for 0-60MHz, VCLK/2 for
	     * 60-120MHz and VCLK/4 for 120-175MHz (or -200MHz, depending
	     * on the RAMDAC actually used)
	     * the RCLK output is tied to the LCLK input which is the same
	     * as SCLK but with no blanking.  SCLK is the actual pixel
	     * shift clock for the pixel bus.
	     * RCLK/8 is used because of the 8:1 pixel-multiplexing below.
	     * (964 uses always 8:1 in 256 color modes)
	     */
	    if (s3InfoRec.clock[mode->Clock] > 120000) {
	       vclock = TI_OCLK_V4;
	    } else if (s3InfoRec.clock[mode->Clock] > 60000){
	       vclock = TI_OCLK_V2;
            } else {
	       vclock = TI_OCLK_V1;
            }
	    if (s3InfoRec.bitsPerPixel == 32) {           /* 24bpp */
               rclock = TI_OCLK_R2;
            } else if ((s3InfoRec.bitsPerPixel == 16) ||
                       (s3InfoRec.bitsPerPixel == 15)) {  /* 15/16bpp */
               rclock = TI_OCLK_R4;
            } else {
               rclock = TI_OCLK_R8;
            }
            s3OutTiIndReg(TI_OUTPUT_CLOCK_SELECT, 0x00,
			  TI_OCLK_S | vclock | rclock);
            outb(vgaCRIndex, 0x66);
            tmp = inb(vgaCRReg);
            outb(vgaCRReg, (tmp & 0xf8) | ((rclock - (vclock >> 3)) & 7));
	 } else if (DAC_IS_TI3025) {
	    if (s3InfoRec.bitsPerPixel == 32) {           /* 24bpp */
	       s3OutTiIndReg(TI_OUTPUT_CLOCK_SELECT, 0x00, TI_OCLK_S_V2_R2);
	       outb(vgaCRIndex, 0x66);
	       tmp = inb(vgaCRReg);
               if (mode->Flags & V_DBLCLK)
	          outb(vgaCRReg, (tmp & 0xf8) | 0x00);
               else
	          outb(vgaCRReg, (tmp & 0xf8) | 0x01);
	    } else if (s3InfoRec.bitsPerPixel == 16) {      /* 5-6-5 */
	       s3OutTiIndReg(TI_OUTPUT_CLOCK_SELECT, 0x00, TI_OCLK_S_V2_R4);
	       outb(vgaCRIndex, 0x66);
	       tmp = inb(vgaCRReg);
               if (mode->Flags & V_DBLCLK)
	          outb(vgaCRReg, (tmp & 0xf8) | 0x01);
               else
	          outb(vgaCRReg, (tmp & 0xf8) | 0x02);
	    } else {
	       s3OutTiIndReg(TI_OUTPUT_CLOCK_SELECT, 0x00, TI_OCLK_S_V2_R8);
	       outb(vgaCRIndex, 0x66);
	       tmp = inb(vgaCRReg);
               if (mode->Flags & V_DBLCLK)
	          outb(vgaCRReg, (tmp & 0xf8) | 0x02);
               else
	          outb(vgaCRReg, (tmp & 0xf8) | 0x03);
	    }
	 } else {
	    /*
	     * for all other boards with Ti3020 (only #9 level 14/16 ?)
	     * set output clocking to VCLK/4, RCLK/8 like the fixed Bt485.
	     * RCLK/8 is used because of the 8:1 pixel-multiplexing below.
	     */
	    if (s3InfoRec.bitsPerPixel == 32) {           /* 24bpp */
	       s3OutTiIndReg(TI_OUTPUT_CLOCK_SELECT, 0x00, TI_OCLK_S_V1_R2);
	    } else if (s3InfoRec.bitsPerPixel == 16) {      /* 5-6-5 */
	       s3OutTiIndReg(TI_OUTPUT_CLOCK_SELECT, 0x00, TI_OCLK_S_V2_R4);
	    } else {
	       s3OutTiIndReg(TI_OUTPUT_CLOCK_SELECT, 0x00, TI_OCLK_S_V4_R8);
	    }
	 }

         /*
          * set the serial access mode 256 words control
          */
         outb(vgaCRIndex, 0x58);
         tmp = inb(vgaCRReg);
         outb(vgaCRReg, (tmp & 0xbf) | s3SAM256);

	 if (s3InfoRec.depth == 24 || s3InfoRec.depth == 32) {  /* 24bpp */
            if (DAC_IS_TI3025) {
               s3OutTiIndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_3025T_888);
               s3OutTiIndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_TC_D24P64);
               s3OutTiIndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x01);
            } else {
               s3OutTiIndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_DIRECT_888);
               s3OutTiIndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_DC_D24P64);
               s3OutTiIndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x00);
            }
	 } else if (s3InfoRec.depth == 16) {                    /* 5-6-5 */
            if (DAC_IS_TI3025) {
               s3OutTiIndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_3025T_565);
               s3OutTiIndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_TC_D16P64);
               s3OutTiIndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x01);
            } else {
               s3OutTiIndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_DIRECT_565);
               s3OutTiIndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_DC_D16P64);
               s3OutTiIndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x00);
            }
	 } else if (s3InfoRec.depth == 15) {                     /* 5-5-5 */
            if (DAC_IS_TI3025) {
               s3OutTiIndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_3025T_555);
               s3OutTiIndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_TC_D15P64);
               s3OutTiIndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x01);
            } else {
               s3OutTiIndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_DIRECT_555);
               s3OutTiIndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_DC_D15P64);
               s3OutTiIndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x00);
            }
	 } else {
            /* set mux control 1 and 2 to provide pseudocolor sub-mode 4   */
            /* this provides a 64-bit pixel bus with 8:1 multiplexing      */
            s3OutTiIndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_PSEUDO_COLOR);
            s3OutTiIndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_PC_D8P64);
	 }

         /* change to 8-bit DAC and re-route the data path and clocking */
         s3OutTiIndReg(TI_GENERAL_IO_CONTROL, 0x00, TI_GIC_ALL_BITS);
         if (s3DAC8Bit) {
            if (DAC_IS_TI3025) {
               s3OutTiIndReg(TI_GENERAL_IO_DATA , 0x00, TI_GID_N9_964);
               s3OutTiIndReg(TI_GENERAL_IO_CONTROL, 0x00, 0x00);
               s3OutTiIndReg(TI_MISC_CONTROL , 0xF0,
                             TI_MC_INT_6_8_CONTROL | TI_MC_8_BPP);
            } else if(OFLG_ISSET(OPTION_ELSA_W2000PRO,&s3InfoRec.options)) 
               s3OutTiIndReg(TI_GENERAL_IO_DATA , 0x00 , TI_GID_W2000_8BIT);
            else
               s3OutTiIndReg(TI_GENERAL_IO_DATA, 0x00, TI_GID_TI_DAC_8BIT);
         } else {
            if (DAC_IS_TI3025) {
               s3OutTiIndReg(TI_GENERAL_IO_DATA , 0x00, TI_GID_N9_964);
               s3OutTiIndReg(TI_GENERAL_IO_CONTROL, 0x00, 0x00);
               s3OutTiIndReg(TI_MISC_CONTROL , 0xF0, TI_MC_INT_6_8_CONTROL);
            } else if(OFLG_ISSET(OPTION_ELSA_W2000PRO,&s3InfoRec.options)) 
	       s3OutTiIndReg( TI_GENERAL_IO_DATA , 0x00 , TI_GID_W2000_6BIT );
            else
               s3OutTiIndReg(TI_GENERAL_IO_DATA, 0x00, TI_GID_TI_DAC_6BIT);
         }
         if (DAC_IS_TI3025) {
	    outb(vgaCRIndex, 0x6D);
	    if (s3Bpp == 1)
	       if (mode->Flags & V_DBLCLK) 
		  outb(vgaCRReg, 0x02);
	       else
		  outb(vgaCRReg, 0x03);
	    else if (s3Bpp == 2)
	       if (mode->Flags & V_DBLCLK) 
		  outb(vgaCRReg, 0x00);
	       else
		  outb(vgaCRReg, 0x01);
	    else /* (s3Bpp == 4) */
	       outb(vgaCRReg, 0x00);
	 }
      } else {
         /* set s3 reg53 to non-parallel addressing by and'ing 0xDF     */
         outb(vgaCRIndex, 0x53);
         tmp = inb(vgaCRReg);
         outb(vgaCRReg, tmp & 0xDF);

         /* set s3 reg55 to non-external serial by and'ing 0xF7         */
         outb(vgaCRIndex, 0x55);
         tmp = inb(vgaCRReg);
         outb(vgaCRReg, tmp & 0xF7);

         /* the input clock is already set to clk1 or clk1double (s3.c) */

	 if (DAC_IS_TI3025) {
	    if (s3InfoRec.bitsPerPixel > 8)
	       s3OutTiIndReg(TI_AUXILIARY_CONTROL, 0, 0);
	    else
	       s3OutTiIndReg(TI_AUXILIARY_CONTROL, 0, TI_AUX_W_CMPL);
	 }
	 else {
            /* set aux control to self clocked only                        */
            s3OutTiIndReg(TI_AUXILIARY_CONTROL, 0, TI_AUX_SELF_CLOCK);
	 }
         /*
          * set output clocking to default of VGA.
          */
         s3OutTiIndReg(TI_OUTPUT_CLOCK_SELECT, 0x00, TI_OCLK_VGA);

         /* set mux control 1 and 2 to provide pseudocolor VGA          */
         s3OutTiIndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_PSEUDO_COLOR);
         s3OutTiIndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_VGA);

         /* change to 8-bit DAC and re-route the data path and clocking */
         s3OutTiIndReg(TI_GENERAL_IO_CONTROL, 0x00, TI_GIC_ALL_BITS);
         if (s3DAC8Bit)
            s3OutTiIndReg(TI_GENERAL_IO_DATA, 0x00, TI_GID_S3_DAC_8BIT);
         else
            s3OutTiIndReg(TI_GENERAL_IO_DATA, 0x00, TI_GID_S3_DAC_6BIT);
      }  /* end of s3PixelMultiplexing */

      /* for some reason the bios doesn't set this properly          */
      s3OutTiIndReg(TI_SENSE_TEST, 0x00, 0x00);

      outb(0x3C4, 1);
      outb(0x3C5, tmp2);        /* unblank the screen */
   }  /* DAC_IS_TI3020_SERIES */

   if (DAC_IS_TI3026 || DAC_IS_TI3030) {
      outb(0x3C4, 1);
      tmp2 = inb(0x3C5);
      outb(0x3C5, tmp2 | 0x20); /* blank the screen */

      /* change polarity on S3 to pass through control to the 3020      */
      tmp = new->MiscOutReg;
      new->MiscOutReg |= 0xC0;
      tmp1 = 0x00;
      if (!(tmp & 0x80)) tmp1 |= 0x02; /* invert bits for the 3020      */
      if (!(tmp & 0x40)) tmp1 |= 0x01;
      if (s3DACSyncOnGreen) tmp1 |= 0x30;  /* add IOG sync  & 7.5 IRE   */
      s3OutTi3026IndReg(TI_GENERAL_CONTROL, 0x00, tmp1);

      if (s3DACSyncOnGreen) {  /* needed for ELSA Winner 2000PRO/X */
	 s3OutTi3026IndReg(TI_GENERAL_IO_CONTROL, 0x00, TI_GIC_ALL_BITS);
         s3OutTi3026IndReg(TI_GENERAL_IO_DATA, ~TI_GID_ELSA_SOG, 0);
      }

      outb(vgaCRIndex, 0x65);
      if (DAC_IS_TI3030)
	 outb(vgaCRReg, 0x80);
      else
	 outb(vgaCRReg, 0);

      if (s3PixelMultiplexing) {
	 int vclock,rclock;
	 /* fun timing mods for pixel-multiplexing!                     */

	 outb(vgaCRIndex, 0x40);
	 outb(vgaCRReg, 0x11);
	 outb(vgaCRIndex, 0x55);
	 outb(vgaCRReg, 0x00);

	 if (s3InfoRec.clock[mode->Clock] > 120000) {
	    vclock = TI_OCLK_V4;
	 } else if (s3InfoRec.clock[mode->Clock] > 60000){
	    vclock = TI_OCLK_V2;
	 } else {
	    vclock = TI_OCLK_V1;
	 }
	 if (s3InfoRec.bitsPerPixel >= 24) {   /* 32bpp or packed 24bpp */
	    rclock = TI_OCLK_R2;
	 } else if ((s3InfoRec.bitsPerPixel == 16) ||
		    (s3InfoRec.bitsPerPixel == 15)) {  /* 15/16bpp */
	    rclock = TI_OCLK_R4;
	 } else {
	    rclock = TI_OCLK_R8;
	 }

	 outb(vgaCRIndex, 0x66);
	 tmp = inb(vgaCRReg);
	 if (DAC_IS_TI3030)
	    tmp |= 0x60;
	 if ((mode->Flags & V_DBLCLK)
	     && OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions))
	    if (s3Bpp <= 2)
	       outb(vgaCRReg, (tmp & 0xf8) | (rclock-2));
	    else
	       outb(vgaCRReg, (tmp & 0xf8) | (rclock-1));
	 else if ((mode->Flags & V_DBLCLK) && DAC_IS_TI3030)
	    outb(vgaCRReg, (tmp & 0xf8) | (rclock-1));
	 else 
	    outb(vgaCRReg, (tmp & 0xf8) | (rclock-0));

         /*
          * set the serial access mode 256 words control
          */
         outb(vgaCRIndex, 0x58);
         tmp = inb(vgaCRReg);
         outb(vgaCRReg, (tmp & 0xbf) | s3SAM256);

	 if (xf86bpp == 24) {                        /* packed 24bpp */
	    s3OutTi3026IndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_3026T_888_P8);
	    if (DAC_IS_TI3030)
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3030TC_D24P128);
	    else
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3026TC_D24P64);
	    s3OutTi3026IndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x01);
	 } else if (s3InfoRec.bitsPerPixel == 32) {               /* 32bpp */
	    s3OutTi3026IndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_3026T_888);
	    if (DAC_IS_TI3030)
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3030TC_D24P128);
	    else
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3026TC_D24P64);
	    s3OutTi3026IndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x01);
	 } else if (s3InfoRec.depth == 16) {                    /* 5-6-5 */
	    s3OutTi3026IndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_3026T_565);
	    if (DAC_IS_TI3030)
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3030TC_D16P128);
	    else
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3026TC_D16P64);
	    s3OutTi3026IndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x01);
	 } else if (s3InfoRec.depth == 15) {                     /* 5-5-5 */
	    s3OutTi3026IndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_3026T_555);
	    if (DAC_IS_TI3030)
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3030TC_D15P128);
	    else
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3026TC_D15P64);
	    s3OutTi3026IndReg(TI_COLOR_KEY_CONTROL, 0x00, 0x01);
	 } else {
            /* set mux control 1 and 2 to provide pseudocolor sub-mode 4   */
            /* this provides a 64-bit pixel bus with 8:1 multiplexing      */
            s3OutTi3026IndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_PSEUDO_COLOR);
	    if (DAC_IS_TI3030)
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3030PC_D8P128);
	    else
	       s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_3026PC_D8P64);
	 }

         /* change to 8-bit DAC and re-route the data path and clocking */
         s3OutTi3026IndReg(TI_GENERAL_IO_CONTROL, 0x00, TI_GIC_ALL_BITS);
         if (s3DAC8Bit) {
	    s3OutTi3026IndReg(TI_MISC_CONTROL , 0xF0,
			      TI_MC_INT_6_8_CONTROL | TI_MC_8_BPP);
         } else {
	    s3OutTi3026IndReg(TI_MISC_CONTROL , 0xF0, TI_MC_INT_6_8_CONTROL);
         }
	 if (OFLG_ISSET(OPTION_DIAMOND, &s3InfoRec.options)) {
	    outb(vgaCRIndex, 0x67);
	    outb(vgaCRReg, 0x01);
	    outb(vgaCRIndex, 0x6D);
	    if (s3Bpp == 1)
	       outb(vgaCRReg, 0x72);
	    else if (s3Bpp == 2)
	       outb(vgaCRReg, 0x73);
	    else /* if (s3Bpp == 4) */
	       outb(vgaCRReg, 0x75);
	 }
	 else {
	    outb(vgaCRIndex, 0x67);
	    outb(vgaCRReg, 0x00);
	    outb(vgaCRIndex, 0x6d);
	    if (s3Bpp == 1)
	       outb(vgaCRReg, 0x00);
	    else if (s3Bpp == 2)
	       outb(vgaCRReg, 0x01);
	    else /* if (s3Bpp == 4) */
	       outb(vgaCRReg, 0x00);
	 }
	 if (DAC_IS_TI3030) {
	    /* set s3 reg53 to parallel addressing by or'ing 0x20     */
	    outb(vgaCRIndex, 0x53);
	    tmp = inb(vgaCRReg);
	    outb(vgaCRReg, tmp | 0x20);
	 }
      } else {
         outb(vgaCRIndex, 0x53);
         tmp = inb(vgaCRReg);
	 if (DAC_IS_TI3030)
	    /* set s3 reg53 to parallel addressing by or'ing 0x20     */
	    outb(vgaCRReg, tmp | 0x20);
	 else
	    /* set s3 reg53 to non-parallel addressing by and'ing 0xDF     */
	    outb(vgaCRReg, tmp & ~0x20);
         /* set s3 reg55 to non-external serial by and'ing 0xF7         */
         outb(vgaCRIndex, 0x55);
         tmp = inb(vgaCRReg);
         outb(vgaCRReg, tmp & 0xF7);

         /* set mux control 1 and 2 to provide pseudocolor VGA          */
         s3OutTi3026IndReg(TI_MUX_CONTROL_1, 0x00, TI_MUX1_PSEUDO_COLOR);
         s3OutTi3026IndReg(TI_MUX_CONTROL_2, 0x00, TI_MUX2_BUS_VGA);
      }  /* end of s3PixelMultiplexing */

      /* for some reason the bios doesn't set this properly          */
      s3OutTi3026IndReg(TI_SENSE_TEST, 0x00, 0x00);

      if (OFLG_ISSET(OPTION_TI3026_CURS, &s3InfoRec.options)) {
	 /* enable interlaced cursor;
	    not very useful without CR45 bit 5 set, but anyway */
	 if (mode->Flags & V_INTERLACE) {
	    static int already = 0;
	    if (!already) {
	       already++;
	       ErrorF("%s %s: Ti3026 hardware cursor in interlaced modes "
		      "doesn't work correctly,\n"
		      "\tplease use Option \"sw_cursor\" when using "
		      "interlaced modes!\n"
		      ,XCONFIG_PROBED, s3InfoRec.name);
	    }
	    s3OutTi3026IndReg(TI_CURS_CONTROL, ~0x60, 0x60);
	 }
	 else
	    s3OutTi3026IndReg(TI_CURS_CONTROL, ~0x60, 0x00);
      }

      outb(0x3C4, 1);
      outb(0x3C5, tmp2);        /* unblank the screen */
   }  /* DAC_IS_TI3026 || DAC_IS_TI3030 */

   if (DAC_IS_IBMRGB) {
      outb(0x3C4, 1);
      tmp2 = inb(0x3C5);
      outb(0x3C5, tmp2 | 0x20); /* blank the screen */

      if (mode->Flags & V_DBLCLK)
	 s3OutIBMRGBIndReg(IBMRGB_misc_clock, 0xf0, 0x03);
      else
	 s3OutIBMRGBIndReg(IBMRGB_misc_clock, 0xf0, 0x01);

      s3OutIBMRGBIndReg(IBMRGB_sync, 0, 0);
      if ((mode->Private[0] & (1 << S3_BLANK_DELAY))
	  && S3_968_SERIES(s3ChipId)) {
	 int pixels = (mode->Private[S3_BLANK_DELAY] & 0x07) * 8 / s3Bpp;
	 if (pixels > 15) pixels = 15;
	 s3OutIBMRGBIndReg(IBMRGB_hsync_pos, 0, pixels);
      }
      else
	 s3OutIBMRGBIndReg(IBMRGB_hsync_pos, 0, 0);
      s3OutIBMRGBIndReg(IBMRGB_pwr_mgmt, 0, 0);
      s3OutIBMRGBIndReg(IBMRGB_dac_op, ~8, s3DACSyncOnGreen ? 8 : 0);
      s3OutIBMRGBIndReg(IBMRGB_dac_op, ~2, 1 /* fast slew */ ? 2 : 0);
      s3OutIBMRGBIndReg(IBMRGB_pal_ctrl, 0, 0);
      /* set VRAM size to 64 bit and disable VRAM mask */
      s3OutIBMRGBIndReg(IBMRGB_misc1, ~0x43, 1);
      if (s3DAC8Bit)
	 s3OutIBMRGBIndReg(IBMRGB_misc2, 0, 0x47);
      else
	 s3OutIBMRGBIndReg(IBMRGB_misc2, 0, 0x43);

#if 0  /* this will lock up the S3 chip & PC sometimes */
      outb(vgaCRIndex, 0x22); /* don't know why it's important    */
      outb(vgaCRReg, 0xff);   /* to set a "read only" register ?? */
#else
      outb(vgaCRIndex, 0x22);
      tmp = inb(vgaCRReg);
      if (s3Bpp == 1 && S3_968_SERIES(s3ChipId))
	 outb(vgaCRReg, tmp | 8);
      else 
	 outb(vgaCRReg, tmp & ~8);
#endif

      outb(vgaCRIndex, 0x65);
      outb(vgaCRReg, 0);

      if (s3PixelMultiplexing) {
	 outb(vgaCRIndex, 0x40);
	 outb(vgaCRReg, 0x11);
	 outb(vgaCRIndex, 0x55);
	 outb(vgaCRReg, 0x00);

	 if (s3InfoRec.depth == 24 || s3InfoRec.depth == 32) { /* 24 bpp */
	    s3OutIBMRGBIndReg(IBMRGB_pix_fmt, 0xf8, 6);
	    s3OutIBMRGBIndReg(IBMRGB_32bpp, 0, 0);
	 } else if (s3InfoRec.depth == 16) {             /* 16 bpp */
	    s3OutIBMRGBIndReg(IBMRGB_pix_fmt, 0xf8, 4);
	    s3OutIBMRGBIndReg(IBMRGB_16bpp, 0, 0x02);
	 } else if (s3InfoRec.depth == 15) {             /* 15 bpp */
	    s3OutIBMRGBIndReg(IBMRGB_pix_fmt, 0xf8, 4);
	    s3OutIBMRGBIndReg(IBMRGB_16bpp, 0, 0x00);
	 } else {                                        /*  8 bpp */
	    s3OutIBMRGBIndReg(IBMRGB_pix_fmt, 0xf8, 3);
	    s3OutIBMRGBIndReg(IBMRGB_8bpp, 0, 0);
	 }
	 /* if (DAC_IS_RGB528) tmp++; */

	 outb(vgaCRIndex, 0x66);
	 tmp = inb(vgaCRReg) & 0xf8;
	 if (!S3_968_SERIES(s3ChipId)) {
	   if (s3Bpp == 1) tmp |= 3;
	   else if (s3Bpp == 2) tmp |= 2;
	   else /* if (s3Bpp == 4) */ tmp |= 1;
	   if (mode->Flags & V_DBLCLK) tmp--;
	 }

	 outb(vgaCRReg, tmp);

         /*
          * set the serial access mode 256 words control
          */
         outb(vgaCRIndex, 0x58);
         tmp = inb(vgaCRReg);
         outb(vgaCRReg, (tmp & 0xbf) | s3SAM256);

	 outb(vgaCRIndex, 0x67);
	 if (s3Bpp == 4)
	    outb(vgaCRReg, 0x00);
	 else
	    if (S3_968_SERIES(s3ChipId))
	       outb(vgaCRReg, 0x11);
	    else
	       outb(vgaCRReg, 0x01);

	 outb(vgaCRIndex, 0x6d);
	 if (s3Bpp == 1)
	    outb(vgaCRReg, 0x21);
	 else if (s3Bpp == 2)
	    outb(vgaCRReg, 0x10);
	 else /* if (s3Bpp == 4) */
	    outb(vgaCRReg, 0x00);

      } else {
         /* set s3 reg53 to non-parallel addressing by and'ing 0xDF     */
         outb(vgaCRIndex, 0x53);
         tmp = inb(vgaCRReg);
         outb(vgaCRReg, tmp & 0xDF);

         /* set s3 reg55 to non-external serial by and'ing 0xF7         */
         outb(vgaCRIndex, 0x55);
         tmp = inb(vgaCRReg);
         outb(vgaCRReg, tmp & 0xF7);

         /* provide pseudocolor VGA          */
         s3OutIBMRGBIndReg(IBMRGB_misc2, 0, 0);
      }  /* end of s3PixelMultiplexing */

#if 0
      if (OFLG_ISSET(OPTION_IBMRGB_CURS, &s3InfoRec.options)) {
	 /* enable interlaced cursor;
	    not very useful without CR45 bit 5 set, but anyway */
	 if (mode->Flags & V_INTERLACE) {
	    static int already = 0;
	    if (!already) {
	       already++;
	       ErrorF("%s %s: IBMRGB hardware cursor in interlaced modes "
		      "doesn't work correctly,\n"
		      "\tplease use Option \"sw_cursor\" when using "
		      "interlaced modes!\n"
		      ,XCONFIG_PROBED, s3InfoRec.name);
	    }
	    s3OutIBMRGBIndReg(TI_CURS_CONTROL, ~0x60, 0x60);
	 }
	 else
	    s3OutIBMRGBIndReg(TI_CURS_CONTROL, ~0x60, 0x00);
      }
#endif

      outb(0x3C4, 1);
      outb(0x3C5, tmp2);        /* unblank the screen */
   }  /* DAC_IS_IBMRGB */

   s3InitCursorFlag = TRUE;  /* turn on the cursor during the next load */

   outb(0x3C2, new->MiscOutReg);

   for (i = 1; i < 5; i++)
      outw(0x3C4, (new->Sequencer[i] << 8) | i);

   for (i = 0; i < 25; i++)
      outw(vgaCRIndex, (new->CRTC[i] << 8) | i);

   for (i = 0; i < 9; i++)
      outw(0x3CE, (new->Graphics[i] << 8) | i);

   i = inb(vgaIOBase + 0x0A);	/* reset flip-flop */
   for (i = 0; i < 16; i++) {
      outb(0x3C0, i);
      outb(0x3C0, new->Attribute[i]);
   }
   for (i = 16; i < 21; i++) {
      outb(0x3C0, i | 0x20);
      outb(0x3C0, new->Attribute[i]);
   }

   if (s3DisplayWidth == 2048)
      s3Port31 = 0x8f;
   else
      s3Port31 = 0x8d;

   outb(vgaCRIndex, 0x31);
   outb(vgaCRReg, s3Port31);
   outb(vgaCRIndex, 0x32);
   outb(vgaCRReg, 0x00);
   outb(vgaCRIndex, 0x33);
   if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options))
     /* Blank border comes earlier than display enable. */
     outb(vgaCRReg, 0x00);
   else if (!S3_TRIOxx_SERIES(s3ChipId))
      outb(vgaCRReg, 0x20);
   outb(vgaCRIndex, 0x34);
   outb(vgaCRReg, 0x10);		/* 1024 */
   outb(vgaCRIndex, 0x35);
   outb(vgaCRReg, 0x00);
   cebank();
#if 0  /* x64: set to 1 if PCI read bursts should be enabled
        * NOTE: there are known problems with PCI burst mode in SATURN
        * chipset rev. 2 so this is commented out, maybe a new XF86Config
        * option should be used
        */
   if (S3_x64_SERIES(s3ChipId)) {
      outb(vgaCRIndex, 0x3a);
      outb(vgaCRReg, 0xb5 & 0x7f);
   } else
#endif
      {
	 outb(vgaCRIndex, 0x66);  /* set CR66_7 before CR3A_7 */
	 tmp = inb(vgaCRReg);
	 outb(vgaCRReg, tmp | 0x80);
	 
	 outb(vgaCRIndex, 0x3a);
	 if (OFLG_ISSET(OPTION_SLOW_DRAM_REFRESH, &s3InfoRec.options))
	    outb(vgaCRReg, 0xb7);
	 else
	    outb(vgaCRReg, 0xb5);
      }

   outb(vgaCRIndex, 0x3b);
   outb(vgaCRReg, (new->CRTC[0] + new->CRTC[4] + 1) / 2);
   outb(vgaCRIndex, 0x3c);
   outb(vgaCRReg, new->CRTC[0]/2);	/* Interlace mode frame offset */

   /* x64: CR40 changed a lot for 864/964; wait and see if this still works */
   outb(vgaCRIndex, 0x40);
   if (S3_911_SERIES (s3ChipId)) {
      i = (inb(vgaCRReg) & 0xf2);
      s3Port40 = (i | 0x09);
      outb(vgaCRReg, s3Port40);
   } else {
      if (s3Localbus) {
	 i = (inb(vgaCRReg) & 0xf2);
	 if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) ||
	     OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options))
	   /* Set no wait states on STB Pegasus. */
	   s3Port40 = (i | 0x01);
	 else s3Port40 = (i | 0x05);
	 outb(vgaCRReg, s3Port40);
      } else {
	 i = (inb(vgaCRReg) & 0xf6);
	 s3Port40 = (i | 0x01);
	 outb(vgaCRReg, s3Port40);
      }
   }

   outb(vgaCRIndex, 0x43);
   switch (s3InfoRec.depth) {
   case 24:
   case 32:
      if (S3_864_SERIES(s3ChipId))
	 outb(vgaCRReg, 0x08);  /* 0x88 can't be used for 864/964 */
      else if (S3_801_928_SERIES(s3ChipId) && DAC_IS_SC15025)
	 outb(vgaCRReg, 0x01);  /* ELSA Winner 1000 */
      else if (DAC_IS_BT485_SERIES && S3_928_SERIES(s3ChipId))
	 outb(vgaCRReg, 0x00);
      break;
   case 15:
   case 16:
      if (DAC_IS_ATT490 || DAC_IS_GENDAC || DAC_IS_SC1148x_SERIES /* JON */
		|| DAC_IS_SS2410 ) /*??? I'm not sure - but the 490 does it */  
	 outb(vgaCRReg, 0x80);
      else if (DAC_IS_TI3025)
	 outb(vgaCRReg, 0x10);
      else if (DAC_IS_TI3026 || DAC_IS_TI3030)
	 outb(vgaCRReg, 0x10);
      else if (DAC_IS_IBMRGB)
	 outb(vgaCRReg, 0x10);
      else if (S3_864_SERIES(s3ChipId))
	 outb(vgaCRReg, 0x08);  /* 0x88 can't be used for 864/964 */
      else if (S3_928_SERIES(s3ChipId)) {
	 if (DAC_IS_SC15025)
	    outb(vgaCRReg, 0x01);  /* ELSA Winner 1000 */
	 else if (DAC_IS_BT485_SERIES || DAC_IS_TI3020)
	    outb(vgaCRReg, 0x00);
	 else
	    outb(vgaCRReg, 0x09);  /* who uses this ? */
      }
      else if (DAC_IS_ATT498 && S3_805_I_SERIES(s3ChipId))
	 outb(vgaCRReg, 0x00);
      else
	 outb(vgaCRReg, 0x09);
      break;
   case 8:
   default:
      outb(vgaCRReg, 0x00); /* DON'T enable XOR addresses */
      break;
   }

   outb(vgaCRIndex, 0x44);
   outb(vgaCRReg, 0x00);

   outb(vgaCRIndex, 0x45);
   i = inb(vgaCRReg) & 0xf2;
   /* hi/true cursor color enable */
   switch (s3InfoRec.bitsPerPixel) {
   case 16:
      if (!S3_x64_SERIES(s3ChipId) && !S3_805_I_SERIES(s3ChipId) &&
          !DAC_IS_TI3020)
	 i = i | 0x04;
      break;
   case 32:
      if (S3_x64_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId) ||
	  DAC_IS_TI3020)
	 i = i | 0x04; /* for 16bit RAMDAC, 0x0c for 8bit RAMDAC */
      else
	 i = i | 0x08;
      break;
   }
   outb(vgaCRReg, i);

   if (S3_801_928_SERIES(s3ChipId) || S3_964_SERIES(s3ChipId)) {

      outb(vgaCRIndex, 0x50);
      i = inb(vgaCRReg);
      i &= ~0xf1;
      switch (s3InfoRec.bitsPerPixel) {
	case 8:
           break;
	case 16:
	   i |= 0x10;
	   break;
	case 24:
	   i |= 0x20;
	   break;
	case 32:
	   i |= 0x30;
	   break;
      }
      switch (s3DisplayWidth) { 
	case 640:
	   i |= 0x40;
	   break;
	case 800:
	   i |= 0x80;
	   break;
	case 1152:
	   i |= 0x01;
	   break;
	case 1280:
	   i |= 0xc0;
	   break;
	case 1600:
	   i |= 0x81;
	   break;
	default: /* 1024 and 2048 */
	   ;
      }
      outb(vgaCRReg, i);

      outb(vgaCRIndex, 0x51);
      s3Port51 = (inb(vgaCRReg) & 0xC0) | ((s3BppDisplayWidth >> 7) & 0x30);

      if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) ||
	  OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options)) {
	if (s3PixelMultiplexing)
	  /* In Pixel Multiplexing mode, disable split transfers. */
	  s3Port51 |= 0x40;
	else
	  /* In VGA mode, enable split transfers. */
	  s3Port51 &= ~0x40;
      }

      outb(vgaCRReg, s3Port51);

      outb(vgaCRIndex, 0x58);
      outb(vgaCRReg, s3SAM256);

#ifdef DEBUG
      ErrorF("Writing CR59 0x%02x, CR5A 0x%02x\n", s3Port59, s3Port5A);
#endif

      outb(vgaCRIndex, 0x59);
      outb(vgaCRReg, s3Port59);
      outb(vgaCRIndex, 0x5A);
      outb(vgaCRReg, s3Port5A);
      
      outb(vgaCRIndex, 0x53);
      tmp = inb(vgaCRReg) & ~0x18;
      if (s3Mmio928)
	 tmp |= 0x10;
      if (s3NewMmio) {
	 if (s3InfoRec.MemBase != 0) {
	    s3Port59 = (s3InfoRec.MemBase >> 24) & 0xfc;
	    s3Port5A = 0;
	    outb(vgaCRIndex, 0x59);
	    outb(vgaCRReg, s3Port59);
	    outb(vgaCRIndex, 0x5a);
	    outb(vgaCRReg, s3Port5A);
	    outb(vgaCRIndex, 0x53);
	 }
	 tmp |= 0x18;
      }

      /*
       * Now the DRAM interleaving bit for the 801/805 chips
       * Note, we don't touch this bit for 928 chips because they use it
       * for pixel multiplexing control.
       */
      if (S3_801_SERIES(s3ChipId)) {
	 if (S3_805_I_SERIES(s3ChipId) && s3InfoRec.videoRam == 2048)
	    tmp |= 0x20;
	 else
	    tmp &= ~0x20;
      }
      outb(vgaCRReg, tmp);

      if (s3NewMmio) {      
	 outb (vgaCRIndex, 0x58);
	 outb (vgaCRReg, (s3LinApOpt & ~0x04) | s3SAM256);  /* window size for linear mode */
      }

      n = 255;
      outb(vgaCRIndex, 0x54);
      if (S3_x64_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId)) {
	 int clock,mclk;
	 clock = s3InfoRec.clock[mode->Clock] * s3Bpp;
	 if (s3InfoRec.s3MClk > 0) 
	    mclk = s3InfoRec.s3MClk;
	 else if (S3_805_I_SERIES(s3ChipId))
	    mclk = 50000;  /* 50 MHz, guess for 805i limit */
	 else
	    mclk = 60000;  /* 60 MHz, limit for 864 */
	 if (s3InfoRec.videoRam < 2048 || S3_TRIO32_SERIES(s3ChipId))
	    clock *= 2;
	 m = (int)((mclk/1000.0*.72+16.867)*89.736/(clock/1000.0+39)-21.1543);
	 if (s3InfoRec.videoRam < 2048 || S3_TRIO32_SERIES(s3ChipId))
	    m /= 2;
	 m -= s3InfoRec.s3Madjust;
	 if (m > 31) m = 31;
	 else if (m < 0) {
	    m = 0;
	    n = 16;
	 }
      }
      else if (s3InfoRec.videoRam == 512 || mode->HDisplay > 1200) /* XXXX */
	 m = 0;
      else if (s3InfoRec.videoRam == 1024)
         m = 2;
      else
	 m = 20;
      if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options))
 	s3Port54 = 0x7F;
      else if (OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options))
	s3Port54 = 0;
      else s3Port54 = m << 3;
      outb(vgaCRReg, s3Port54);
      
      n -= s3InfoRec.s3Nadjust;
      if (n < 0) n = 0;
      else if (n > 255) n = 255;
      outb(vgaCRIndex, 0x60);
      outb(vgaCRReg, n);

      if(!OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options)) {
	outb(vgaCRIndex, 0x55);
	tmp = inb(vgaCRReg) & 0x08;       /* save the external serial bit  */
	outb(vgaCRReg, tmp | 0x40);	/* remove mysterious dot at 60Hz */
      }

      /* This shouldn't be needed -- they should be set by vgaHWInit() */
      if (!mode->CrtcVAdjusted) {
	 mode->CrtcVTotal /= interlacedived;
	 mode->CrtcVDisplay /= interlacedived;
	 mode->CrtcVSyncStart /= interlacedived;
	 mode->CrtcVSyncEnd /= interlacedived;
	 mode->CrtcVAdjusted = TRUE;
      }

      if ((S3_TRIO64V_SERIES(s3ChipId) && (s3ChipRev <= 0x531) && (s3Bpp==1)) ^
	  !!OFLG_ISSET(OPTION_TRIO64VP_BUG2, &s3InfoRec.options))
	 i = (((mode->CrtcVTotal - 2) & 0x400) >> 10)  |
	     (((mode->CrtcVDisplay - 1) & 0x400) >> 9) |
	     (((mode->CrtcVDisplay - 1) & 0x400) >> 8) |
	     (((mode->CrtcVSyncStart) & 0x400) >> 6)   | 0x40;
      else
	 i = (((mode->CrtcVTotal - 2) & 0x400) >> 10)  |
	     (((mode->CrtcVDisplay - 1) & 0x400) >> 9) |
	     (((mode->CrtcVSyncStart) & 0x400) >> 8)   |
	     (((mode->CrtcVSyncStart) & 0x400) >> 6)   | 0x40;
	  
      outb(vgaCRIndex, 0x5e);
      outb(vgaCRReg, i);

      if ((S3_TRIO64V_SERIES(s3ChipId) && (s3ChipRev <= 0x531) && (s3Bpp==1)) ^
	  !!OFLG_ISSET(OPTION_TRIO64VP_BUG2, &s3InfoRec.options)) {
	 i = ((((mode->CrtcHTotal >> 3) - 5) & 0x100) >> 8) |
	     ((((mode->CrtcHDisplay >> 3) - 1) & 0x100) >> 7) |
	     (((mode->CrtcHDisplay >> 3) & 0x100) >> 6) |
	     ((mode->CrtcHSyncStart & 0x800) >> 7);
	 if ((mode->CrtcHTotal >> 3) - (mode->CrtcHDisplay >> 3) > 64)
	    i |= 0x08;   /* add another 64 DCLKs to blank pulse width */
      }
      else {
	 i = ((((mode->CrtcHTotal >> 3) - 5) & 0x100) >> 8) |
	     ((((mode->CrtcHDisplay >> 3) - 1) & 0x100) >> 7) |
	     ((((mode->CrtcHSyncStart >> 3) - 1) & 0x100) >> 6) |
	     ((mode->CrtcHSyncStart & 0x800) >> 7);
	 if ((mode->CrtcHSyncEnd >> 3) - (mode->CrtcHSyncStart >> 3) > 64)
	    i |= 0x08;   /* add another 64 DCLKs to blank pulse width */
      }

      if ((mode->CrtcHSyncEnd >> 3) - (mode->CrtcHSyncStart >> 3) > 32)
	 i |= 0x20;   /* add another 32 DCLKs to hsync pulse width */


      outb(vgaCRIndex, 0x3b);
      itmp = (  new->CRTC[0] + ((i&0x01)<<8)
	      + new->CRTC[4] + ((i&0x10)<<4) + 1) / 2;
      if (itmp-(new->CRTC[4] + ((i&0x10)<<4)) < 4)
	 if (new->CRTC[4] + ((i&0x10)<<4) + 4 <= new->CRTC[0]+ ((i&0x01)<<8))
	    itmp = new->CRTC[4] + ((i&0x10)<<4) + 4;
	 else
	    itmp = new->CRTC[0]+ ((i&0x01)<<8) + 1;
      outb(vgaCRReg, itmp & 0xff);
      i |= (itmp&0x100) >> 2;
      outb(vgaCRIndex, 0x3c);
      outb(vgaCRReg, (new->CRTC[0] + ((i&0x01)<<8)) /2);	/* Interlace mode frame offset */

      outb(vgaCRIndex, 0x5d);
      tmp = inb(vgaCRReg);
      outb(vgaCRReg, (tmp & 0x80) | i);

      if (s3InfoRec.videoRam > 1024 && S3_x64_SERIES(s3ChipId)) 
	 i = mode->HDisplay * s3Bpp / 8 + 1;
      else
	 i = mode->HDisplay * s3Bpp / 4 + 1; /* XXX should be checked for 801/805 */
      
      outb(vgaCRIndex, 0x61);
      tmp = inb(vgaCRReg);
      outb(vgaCRReg, 0x80 | (tmp & 0x60) | (i >> 8));
      outb(vgaCRIndex, 0x62);
      outb(vgaCRReg, i & 0xff);
   }

   if ((mode->Flags & V_INTERLACE) != 0) {
      outb(vgaCRIndex, 0x42);
      tmp = inb(vgaCRReg);
      outb(vgaCRReg, 0x20 | tmp);
   }
   else {
      outb(vgaCRIndex, 0x42);
      tmp = inb(vgaCRReg);
      outb(vgaCRReg, ~0x20 & tmp);
   }

   if (mode->Private) {
      if (mode->Private[0] & (1 << S3_INVERT_VCLK)) {
	outb(vgaCRIndex, 0x67);
	tmp = inb(vgaCRReg) & 0xfe;
	if (mode->Private[S3_INVERT_VCLK])
	  tmp |= 1;
	outb(vgaCRReg, tmp);
      }
      if (mode->Private[0] & (1 << S3_BLANK_DELAY)) {
	 outb(vgaCRIndex, 0x6d);
	 outb(vgaCRReg, mode->Private[S3_BLANK_DELAY]);
      }
      if (mode->Private[0] & (1 << S3_EARLY_SC)) {
	 outb(vgaCRIndex, 0x65);
	 tmp = inb(vgaCRReg) & ~2;
	 if (mode->Private[S3_EARLY_SC])
	    tmp |= 2;
	 outb(vgaCRReg, tmp);
      }
   }


   if (OFLG_ISSET(OPTION_FAST_VRAM, &s3InfoRec.options)) {
      outb(vgaCRIndex, 0x39);
      outb(vgaCRReg, 0xa5);
      outb(vgaCRIndex, 0x68);
      tmp = inb(vgaCRReg);
      /* -RAS low timing 3.5 MCLKs, -RAS precharge timing 2.5 MCLKs */
      outb(vgaCRReg, tmp | 0xf0);
   }

   if (OFLG_ISSET(OPTION_SLOW_VRAM, &s3InfoRec.options)) {
      /* 
       * some Diamond Stealth 64 VRAM cards have a problem with VRAM timing,
       * increase -RAS low timing from 3.5 MCLKs to 4.5 MCLKs 
       */ 
      outb(vgaCRIndex, 0x39);
      outb(vgaCRReg, 0xa5);
      outb(vgaCRIndex, 0x68);
      tmp = inb(vgaCRReg);
      if ((tmp & 0x30) == 0x30) 		/* 3.5 MCLKs */
	 outb(vgaCRReg, tmp & 0xef);		/* 4.5 MCLKs */
   }

   if (OFLG_ISSET(OPTION_SLOW_DRAM, &s3InfoRec.options)) {
      /* 
       * fixes some pixel errors for a SPEA Trio64V+ card
       * increase -RAS precharge timing from 2.5 MCLKs to 3.5 MCLKs 
       */ 
      outb(vgaCRIndex, 0x39);
      outb(vgaCRReg, 0xa5);
      outb(vgaCRIndex, 0x68);
      tmp = inb(vgaCRReg);
      outb(vgaCRReg, tmp & 0xf7);		/* 3.5 MCLKs */
   }

   if (OFLG_ISSET(OPTION_SLOW_EDODRAM, &s3InfoRec.options)) {
      /* 
       * fixes some pixel errors for a SPEA Trio64V+ card
       * increase from 1-cycle to 2-cycle EDO mode
       */ 
      outb(vgaCRIndex, 0x39);
      outb(vgaCRReg, 0xa5);
      outb(vgaCRIndex, 0x36);
      tmp = inb(vgaCRReg);
      if ((tmp & 0x0c) == 0x00) 		/* 1-cycle EDO */
	 outb(vgaCRReg, tmp | 0x08);		/* 2-cycle EDO */
   }


   if (S3_964_SERIES(s3ChipId) && DAC_IS_BT485_SERIES) {
      if (OFLG_ISSET(OPTION_S3_964_BT485_VCLK, &s3InfoRec.options)) {
	 /*
	  * This is the design alert from S3 with Bt485A and Vision 964. 
	  */
	 int i,last,tmp,cr55,cr67;
	 int port, bit;

#define VerticalRetraceWait() \
	 { \
	      while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x00) ; \
	      while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x08) ; \
	      while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x00) ; \
	}

	 if (OFLG_ISSET(OPTION_DIAMOND, &s3InfoRec.options)) {
	    port = 0x3c2;
	    bit  = 0x10;
	 }
	 else {  /* MIRO 20SV Rev.2 */
	    port = 0x3c8;
	    bit  = 0x04;
	 }

	 if (port == 0x3c8) {
	    outb(vgaCRIndex, 0x55);
	    cr55 = inb(vgaCRReg);
	    outb(vgaCRReg, cr55 | 0x04); /* enable rad of general input */
	 }
	 outb(vgaCRIndex, 0x67);
	 cr67 = inb(vgaCRReg);

	 for(last=i=30; --i;) {
	    VerticalRetraceWait();
	    VerticalRetraceWait();
	    if ((inb(port) & bit) == 0) { /* if GD2 is low then */
	       last = i;
	       outb(vgaCRIndex, 0x67);
	       tmp = inb(vgaCRReg);
	       outb(vgaCRReg, tmp ^ 0x01); /* clock should be inverted */
#if 0
	       ErrorF("inverted VCLK %d  to 0x%02x\n",i,tmp ^ 0x01);
#endif
	    }
	    if (last-i > 4) break;
	 }
	 if (!i) {  /* didn't get stable input, restore original CR67 value */
	    outb(vgaCRIndex, 0x67);
	    outb(vgaCRReg, cr67);
	 }
	 if (port == 0x3c8) {
	    outb(vgaCRIndex, 0x55);
	    outb(vgaCRReg, cr55); 
	 }
#if 0
	 outb(vgaCRIndex, 0x67);
	 tmp = inb(vgaCRReg);
	 /* if ((cr67 ^ tmp) & 0x01)  */ 
	 ErrorF("VCLK has been inverted %d times from 0x%02x to 0x%02x now\n",i,cr67,tmp);
#endif
      }
   }

   if (OFLG_ISSET(CLOCK_OPTION_ICS2595, &s3InfoRec.clockOptions))
         (void) (s3ClockSelectFunc)(mode->SynthClock);
         /* fixes the ICS2595 initialisation problems */

   s3AdjustFrame(s3InfoRec.frameX0, s3InfoRec.frameY0);

#ifdef REG_DEBUG
   for (i=0; i<10; i++) {
	 outb(vgaCRIndex, i);
	 tmp = inb(vgaCRReg);
	 ErrorF("CR%X = 0x%02x\n", i, tmp);
   }
   for (i=0x10; i<0x19; i++) {
	 outb(vgaCRIndex, i);
	 tmp = inb(vgaCRReg);
	 ErrorF("CR%X = 0x%02x\n", i, tmp);
   }
   outb(vgaCRIndex, 0x3b);
   tmp = inb(vgaCRReg);
   ErrorF("CR%X = 0x%02x\n", 0x3b, tmp);
   outb(vgaCRIndex, 0x5d);
   tmp = inb(vgaCRReg);
   ErrorF("CR%X = 0x%02x\n", 0x5d, tmp);
   outb(vgaCRIndex, 0x5e);
   tmp = inb(vgaCRReg);
   ErrorF("CR%X = 0x%02x\n", 0x5e, tmp);
   for (i=0; i<0x40; i++) {
	 tmp = s3InTiIndReg(i);
	 ErrorF("TI%X = 0x%02x\n", i, tmp);
   }
#endif
   vgaProtect(FALSE);

   if (s3DisplayWidth == 1024) /* this is unclear Jon */
      outw(ADVFUNC_CNTL, 0x0007);
    else 
      outw(ADVFUNC_CNTL, 0x0003);

 /*
  * Blank the screen temporarily to display color 0 by turning the display of
  * all planes off
  */
   outb(DAC_MASK, 0x00);

 /* Reset the 8514/A, and disable all interrupts. */
   outw(SUBSYS_CNTL, GPCTRL_RESET | CHPTEST_NORMAL);
   outw(SUBSYS_CNTL, GPCTRL_ENAB | CHPTEST_NORMAL);

   i = inw(SUBSYS_STAT);

   outw(MULTIFUNC_CNTL, MEM_CNTL | VRTCFG_4 | HORCFG_8);

   outb(DAC_MASK, 0xff);

   LOCK_SYS_REGS;

#ifdef PC98
   crtswitch(1);
#endif
   return TRUE;
}

/* InitLUT() */

/*
 * Loads the Look-Up Table with all black. Assumes 8-bit board is in use.
 */
static void
InitLUT()
{
   short i, j;

   outb(DAC_R_INDEX, 0);
   for (i = 0; i < 256; i++) {
      oldlut[i].r = inb(DAC_DATA);
      oldlut[i].g = inb(DAC_DATA);
      oldlut[i].b = inb(DAC_DATA);
   }
      
   outb(DAC_W_INDEX, 0);

   /* Load all 256 LUT entries */
   for (i = 0; i < 256; i++) {
      outb(DAC_DATA, 0);
      outb(DAC_DATA, 0);
      outb(DAC_DATA, 0);
   }

   if (s3InfoRec.bitsPerPixel > 8 &&
       (DAC_IS_SC15025 || DAC_IS_TI3020_SERIES || DAC_IS_TI3026 
	|| DAC_IS_TI3030 || DAC_IS_IBMRGB)) {
      int r,g,b;
      int mr,mg,mb;
      int nr=5, ng=5, nb=5;
      extern unsigned char xf86rGammaMap[], xf86gGammaMap[], xf86bGammaMap[];
      extern LUTENTRY currents3dac[];

      if (!LUTInited) {
	 if (s3Weight == RGB32_888 || DAC_IS_TI3020_SERIES || DAC_IS_TI3026 
	     || DAC_IS_TI3030 || DAC_IS_IBMRGB) {
	    for(i=0; i<256; i++) {
	       currents3dac[i].r = xf86rGammaMap[i];
	       currents3dac[i].g = xf86gGammaMap[i];
	       currents3dac[i].b = xf86bGammaMap[i];
	    }
	 }
	 else {
	    if (s3Weight == RGB16_565) ng = 6;
	    mr = (1<<nr)-1;
	    mg = (1<<ng)-1;
	    mb = (1<<nb)-1;
	    
	    for(i=0; i<256; i++) {
	       r = (i >> (6-nr)) & mr;
	       g = (i >> (6-ng)) & mg;
	       b = (i >> (6-nb)) & mb;
	       currents3dac[i].r = xf86rGammaMap[(r*255+mr/2)/mr];
	       currents3dac[i].g = xf86gGammaMap[(g*255+mg/2)/mg];
	       currents3dac[i].b = xf86bGammaMap[(b*255+mb/2)/mb];
	    }
	 }
      }

      i = xf86getdaccomm();
      xf86setdaccomm(i | 0x08);
      
      outb(DAC_W_INDEX, 0);
      for(i=0; i<256; i++) {
	 outb(DAC_DATA, currents3dac[i].r);
	 outb(DAC_DATA, currents3dac[i].g);
	 outb(DAC_DATA, currents3dac[i].b);
      }
   }
   LUTInited = TRUE;
}

/*
 * s3InitEnvironment()
 * 
 * Initializes the 8514/A's drawing environment and clears the display.
 */
void
s3InitEnvironment()
{
 /* Current mixes, src, foreground active */

   WaitQueue(6);
   outw(FRGD_MIX, FSS_FRGDCOL | MIX_SRC);
   outw(BKGD_MIX, BSS_BKGDCOL | MIX_SRC);

 /* Clipping rectangle to full drawable space */
   outw(MULTIFUNC_CNTL, SCISSORS_T | 0x000);
   outw(MULTIFUNC_CNTL, SCISSORS_L | 0x000);
   outw(MULTIFUNC_CNTL, SCISSORS_R | (s3DisplayWidth-1));
   outw(MULTIFUNC_CNTL, SCISSORS_B | s3ScissB);

 /* Enable writes to all planes and reset color compare */
   WaitQueue16_32(3,5);

   outw32(WRT_MASK, ~0);
   outw32(RD_MASK, 0);
   outw(MULTIFUNC_CNTL, PIX_CNTL | 0x0000);

 /*
  * Clear the display.  Need to set the color, origin, and size. Then draw.
  */
   WaitQueue16_32(6,7);

   if (xf86FlipPixels && s3Bpp == 1)
      outw32(FRGD_COLOR, 1);
   else
      outw32(FRGD_COLOR, 0);

   outw(CUR_X, 0);
   outw(CUR_Y, 0);
   outw(MAJ_AXIS_PCNT, s3InfoRec.virtualX - 1);
   outw(MULTIFUNC_CNTL, MIN_AXIS_PCNT | s3ScissB);
   outw(CMD, CMD_RECT | INC_Y | INC_X | DRAW | PLANAR | WRTDATA);

   WaitQueue16_32(4,6);

 /* Reset current draw position */
   outw(CUR_X, 0);
   outw(CUR_Y, 0);

 /* Reset current colors, foreground is all on, background is 0. */
   outw32(FRGD_COLOR, ~0);
   outw32(BKGD_COLOR,  0);

 /* Load the LUT */
   InitLUT();

 /* fix a bug in early Trio64 chips (with Trio32 labels) */
   outb(DAC_MASK, 0xff); 
}

void
s3Unlock()
{
   unsigned char tmp;

   xf86EnableIOPorts(s3InfoRec.scrnIndex);

 /* unlock */
   outb(vgaCRIndex, 0x38);
   outb(vgaCRReg, 0x48);
   outb(vgaCRIndex, 0x39);
   outb(vgaCRReg, 0xa5);

   outb(vgaCRIndex, 0x35);		/* select segment 0 */
   tmp = inb(vgaCRReg);
   outb(vgaCRReg, tmp & 0xf0);
   cebank();

   outb(vgaCRIndex, 0x11);		/* allow writting? */
   outb(vgaCRReg, 0x00);

}
