/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3init.c,v 1.1.2.2 1999/12/11 15:31:01 hohndel Exp $ */
/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */
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
#include "s3reg.h"
#include "s3Bt485.h"
#define S3_SERVER
#include "Ti302X.h"
#include "IBMRGB.h"
#define XCONFIG_FLAGS_ONLY 
#include "xf86_Config.h"


#define new ((vgaHWPtr)vgaNewVideoState)
#define newVS ((vgaS3Ptr)vgaNewVideoState)
#define REG50_MASK	0x673b

static vgaS3Ptr OldS3 = NULL; 


extern Bool pixMuxPossible;
extern Bool allowPixMuxInterlace;
extern Bool allowPixMuxSwitching;
extern int nonMuxMaxClock;


/*
 *  S3Init --
 *  
 *	The SVGA server does Init then Save then Restore. The idea
 *	being that Init doesn't write any registers but merely fills
 * 	in a data structure. Save then saves the original state (you
 *	could do that if Init didn't write any registers). Then Restore
 *	restores the data structure that you completed in Init.  But
 *	since we couldn't refrain from writing registers in Init, our
 *	Save will be corrupted and the Restore is useless.  I have 
 *	essentially pounded the SVGA server into the form of the S3 
 *	server.  Now Init writes registers, Save saves a bogus state
 *	so our restore doesn't do anything (empty function).  Save
 *	is functional though.  I do use it to save the initial state.
 *	It's just not the same initial state that the SVGA server 
 *	saves.  I call it sooner on the first pass through Init (S3
 *	server style) and restore it explicitly in the EnterLeave
 *	function (also S3 server style).
 *
 *					MArk
 */


Bool S3Init(DisplayModePtr mode)
{
    static Bool FirstTime = TRUE;
    short InitLevel = 3;

#ifdef S3_DEBUG 
    ErrorF("In S3Init()\n");
#endif

    if(FirstTime) {
	/* save the initial state */	
        OldS3 = S3Save(OldS3); 
	FirstTime = FALSE;
    }
	
    /* There should be a flag or something that tells if we have already
	filled in mode info. Perhaps something in Private?  This only needs
	to be filled in when the mode is being run for the first time or
	when there is a change that forces us to go to init level 2 (MArk) */
    S3FillInModeInfo(mode);

     
    if(pixMuxPossible) {
        unsigned char verdict;
	Bool PriorPixMux = s3PixelMultiplexing;
	/* in case we are using a prior computed mode. not needed yet */
	Bool PriorModeMux = (mode->Flags & V_PIXMUX);

   /* If mux is changing, fill in mode info again and take to init level 2.
	Note that the S3MuxOrNot function merely states if we must or can't
	mux. If it's indifferent, we don't change the mux state. */ 

	if((verdict = S3MuxOrNot(mode)) == (MUSTMUX | CANTMUX)) 
	    return FALSE;
        else if(!verdict) { /* don't care */
            if(s3PixelMultiplexing) mode->Flags |= V_PIXMUX;
	    else mode->Flags &= ~V_PIXMUX;
  	} else {	/* must be one way or the other */
            if(verdict == MUSTMUX) mode->Flags |= V_PIXMUX;
	    else mode->Flags &= ~V_PIXMUX;
	}

     
        s3PixelMultiplexing = (mode->Flags & V_PIXMUX) ? TRUE : FALSE;

#ifdef S3_DEBUG 
    ErrorF("Mode \"%s\" %s pixmux\n",mode->name,s3PixelMultiplexing ?
				"Using" : "Not Using");
#endif
 	if((s3PixelMultiplexing != PriorPixMux) || 
		(s3PixelMultiplexing != PriorModeMux)) {
	    /* If multiplexing state has changed, recalculate Mux Shift
		and take initialization back a level */
	    S3FillInModeInfo(mode);
	    InitLevel = 2;
	}
     } 


     if(!s3Initialized) { 
	InitLevel = 1;
	s3Initialized = TRUE;
     }

     switch(InitLevel) {
	case 1:
	    if(!S3InitLevelOne(mode)) return FALSE;
	case 2:
	    if(!S3InitLevelTwo(mode)) return FALSE;
	default:
	    if(!S3InitLevelThree(mode)) return FALSE;
     }
	
     s3CurrentMode = mode;

     return TRUE;
}


/*  I'll work on Levels One and Two when I figure out where to make
	the cuts.  We essentially do a full init every time for the
	moment. That's almost what the S3 server was doing anyhow (MArk) */


/*
 *  S3InitLevelOne --
 *
 *	Full reinitialization
 */

Bool S3InitLevelOne(DisplayModePtr mode)
{
#ifdef S3_DEBUG 
    ErrorF("In S3InitLevelOne()\n");
#endif

  return TRUE;
}





/*
 *  S3InitLevelTwo --
 *
 *	Take it back one level in some cases.
 */

Bool S3InitLevelTwo(DisplayModePtr mode)
{
#ifdef S3_DEBUG 
    ErrorF("In S3InitLevelTwo()\n");
#endif


  return TRUE;

}



/*
 *  S3InitLevelThree --
 *
 *	Simple mode change.
 *
 * 	Everything is stuck in here for now (Init level 1 every time!)
 *	We migrate things back to 1 and 2 when we figure out where to
 *	make the cuts.  If you know what a particular piece of code does
 *	and it isn't labeled... label it! (MArk)
 *
 */

Bool S3InitLevelThree(DisplayModePtr mode)
{
    unsigned char CR5C, tmp;
    unsigned int itmp;
    short i,n,m;
    int   interlacedivisor = mode->Flags & V_INTERLACE ? 2 : 1;

#ifdef S3_DEBUG 
    ErrorF("In S3InitLevelThree()\n");
#endif
	 

  if (!vgaHWInit(mode, sizeof(vgaS3Rec))) 
	return FALSE;
   
 
   /* S3Adjust() needs this */
   s3HDisplay = mode->HDisplay;  

   new->MiscOutReg |= 0x0C;		/* enable CR42 clock selection */
   new->Sequencer[0] = 0x03;		/* XXXX shouldn't need this */
   new->CRTC[19] = s3BppDisplayWidth >> 3;
   new->CRTC[23] = 0xE3;		/* XXXX shouldn't need this */
   new->Attribute[0x11] = s3DACBoarder; /* The overscan colour AR11 gets */
					/* disabled anyway */

/* !! double negative? (MArk) */
   if ((S3_TRIO64V_SERIES(s3ChipId) && (s3ChipRev <= 0x53) && (s3Bpp==1)) ^
       !!OFLG_ISSET(OPTION_TRIO64VP_BUG2, &vga256InfoRec.options)) {
      /* set correct blanking for broken Trio64V+ to avoid bright left border:
	 blank signal needs to go off ~400 usec before video signal starts 
	 w/o border:  blank_shift = 0 */
      int blank_shift = 400 * vga256InfoRec.clock[mode->Clock] / 1000000 / 8;
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

   if (vgaIOBase == 0x3B0) 
      new->MiscOutReg &= 0xFE;
   else
      new->MiscOutReg |= 0x01;

	/***********************\
	|	BLAH!!!!!	|
	\***********************/

	/* here's where I blow it and give into the idea of actually
		writing registers in here (MArk). */

   vgaProtect(TRUE);

   if (DAC_IS_TI3025) {
      /* switch the ramdac from bt485 to ti3020 mode clearing RS4 */
      outb(vgaCRIndex, 0x5C);
      CR5C = inb(vgaCRReg);
      outb(vgaCRReg, CR5C & 0xDF);

      /* clear TI_PLANAR_ACCESS bit */
      s3OutTiIndReg(TI_CURS_CONTROL, 0x7F, 0x00);
   }



/* can this be moved to the DACInit functions ?  It gets it out
	of the common code and if we ever stick ramdac support
	in modules, the rest of the server doesn't have to 
	deal with it (MArk) */
   if (OFLG_ISSET(OPTION_SPEA_MERCURY, &vga256InfoRec.options) && 
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
	 tmp = inb(vgaCRReg) | 0x20;
	 outb(vgaCRReg, tmp);
	 outb(vgaCRIndex, 0x5C);
	 outb(vgaCRReg, 0x00);
      } else {
	 outb(vgaCRIndex, 0x5C);
	 outb(vgaCRReg, 0x20);
	 outb(0x3C7, 0x00); 
	 /* set s3 reg53 to non-parallel addressing by and'ing 0xDF	*/
	 outb(vgaCRIndex, 0x53);
	 tmp = inb(vgaCRReg) & 0xDF;
	 outb(vgaCRReg, tmp);
	 outb(vgaCRIndex, 0x5C);
	 outb(vgaCRReg, 0x00);
      }
      if (s3Bpp == 4) 
         s3OutTi3026IndReg(TI_LATCH_CONTROL, ~1, 1);  /* 0x06 -> 0x07 */
      else
         s3OutTi3026IndReg(TI_LATCH_CONTROL, ~1, 0);  /* 0x06 */
   }


/* should be able to move this to the DACInit functions (MArk) */
   if((DAC_IS_TI3026 || DAC_IS_TI3030) &&
       (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &vga256InfoRec.clockOptions))){
      /*
       * for the boards with Ti3026 and external ICD2061A clock chip we
       * need to enable clock doubling, if necessary
       */
      if( mode->Flags & V_DBLCLK ) {
         s3OutTi3026IndReg(TI_INPUT_CLOCK_SELECT,0x00,0x08);
      }
      else {
         s3OutTi3026IndReg(TI_INPUT_CLOCK_SELECT,0x00,0x00);
      }
      if (s3Bpp == 4) 
         s3OutTi3026IndReg(TI_LATCH_CONTROL, ~1, 1);  /* 0x06 -> 0x07 */
      else
         s3OutTi3026IndReg(TI_LATCH_CONTROL, ~1, 0);  /* 0x06 */
   }


   /* Don't change the clock bits when using an external clock program */

   if (new->NoClock < 0) {
      new->MiscOutReg = (new->MiscOutReg & 0xF3) | (inb(0x3CC) & 0x0C);
   } else {
      /* XXXX Should we really do something about the return value? */
      if (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions))
	 (void) (s3ClockSelectFunc)(mode->SynthClock);
      else
         (void) (s3ClockSelectFunc)(mode->Clock);

/* OK. how about
	if(!(s3ClockSelectFunc)(
	OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions) ?
	mode->SynthClock : mode->Clock)) return FALSE;

*/
     
      if ((mode->Flags & V_DBLCLK)
	 && (DAC_IS_TI3026 || DAC_IS_TI3030) 
	 && (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &vga256InfoRec.clockOptions))){
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


   /* Initialize Ramdac */
   if(!(s3Ramdacs[s3RamdacType].DacInit)(mode)) {
	ErrorF("%s RAMDAC initialization failed!\n",
		s3Ramdacs[s3RamdacType].DacName);
	return FALSE;
   }

   outb(0x3C2, new->MiscOutReg);

   for (i = 1; i < 5; i++)
      outw(0x3C4, (new->Sequencer[i] << 8) | i);

   for (i = 0; i < 25; i++)
      outw(vgaCRIndex, (new->CRTC[i] << 8) | i);

   for (i = 0; i < 9; i++)
      outw(0x3CE, (new->Graphics[i] << 8) | i);

   inb(vgaIOBase + 0x0A);	/* reset flip-flop */
   for (i = 0; i < 16; i++) {
      outb(0x3C0, i);
      outb(0x3C0, new->Attribute[i]);
   }
   for (i = 16; i < 21; i++) {
      outb(0x3C0, i | 0x20);
      outb(0x3C0, new->Attribute[i]);
   }

   /****** CR31 ******/
   /*  We need to save CR31 since it is also used for the display
	start address.  Here we initialize for two page setup when
	using a 2048 width */
   if (s3DisplayWidth == 2048)
      s3Port31 = 0x8f;
   else
      s3Port31 = 0x8d;
   outb(vgaCRIndex, 0x31); outb(vgaCRReg, s3Port31);

   /***** CR32, CR33 and CR34 *****/
   /* These are the backwards compatibility registers */
   outb(vgaCRIndex, 0x32); outb(vgaCRReg, 0x00); /* Back compat 1 */
   outb(vgaCRIndex, 0x33);			 /* Back compat 2 */
   if (OFLG_ISSET(OPTION_STB_PEGASUS, &vga256InfoRec.options))
     /* Blank border comes earlier than display enable. */
     outb(vgaCRReg, 0x00);
   else if (!S3_TRIOxx_SERIES(s3ChipId))
      outb(vgaCRReg, 0x20);
   outb(vgaCRIndex, 0x34); outb(vgaCRReg, 0x10); /* Back compat 3 */
   S3BankZero();


   /****** CR3A ******/
   outb(vgaCRIndex, 0x3a);
   if (OFLG_ISSET(OPTION_SLOW_DRAM_REFRESH, &vga256InfoRec.options))
	outb(vgaCRReg, 0xb7);
   else
	outb(vgaCRReg, 0xb5);

   /****** CR3B ******/
   /* CR3B and CR3C may undergo some additional modifications
	later on */
   outb(vgaCRIndex, 0x3b);
   if (!S3_AURORA64VP_SERIES(s3ChipId)) {
      outb(vgaCRIndex, 0x3b);
      outb(vgaCRReg, (new->CRTC[0] + new->CRTC[4] + 1) / 2);
   }

   /****** CR3C ******/
   outb(vgaCRIndex, 0x3c);
   outb(vgaCRReg, new->CRTC[0]/2);	/* Interlace mode frame offset */

   /****** CR40 ******/
   outb(vgaCRIndex, 0x40);
   tmp = inb(vgaCRReg);
   if (S3_911_SERIES (s3ChipId)) 
      tmp = (tmp & 0xf2) | 0x09;
   else {
      if (s3Localbus) {
	 tmp &= 0xf2;
	 if (OFLG_ISSET(OPTION_STB_PEGASUS, &vga256InfoRec.options) ||
	     OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &vga256InfoRec.options))
	   tmp |= 0x01; 	   /* no wait states */
	 else tmp |= 0x05;    /* one wait state */
      } else tmp = (tmp & 0xf6) | 0x01;
   }
   outb(vgaCRReg, tmp);

   /****** CR43 ******/
   outb(vgaCRIndex, 0x43);
   switch (vga256InfoRec.depth) {
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
      if (DAC_IS_ATT490 || DAC_IS_GENDAC || DAC_IS_SC1148x /* JON */
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

   /****** CR44 ******/
   /* What is this? */
   outb(vgaCRIndex, 0x44); outb(vgaCRReg, 0x00);

   /****** CR45 ******/
   outb(vgaCRIndex, 0x45);
   tmp = inb(vgaCRReg) & 0xf2;
   /* hi/true cursor color enable */
   switch (vga256InfoRec.bitsPerPixel) {
   case 16:
      if (!S3_x64_SERIES(s3ChipId) && !S3_805_I_SERIES(s3ChipId) &&
          !DAC_IS_TI3020)
	 tmp |= 0x04;
      break;
   case 32:
      if (S3_x64_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId) ||
	  DAC_IS_TI3020)
	 tmp |= 0x04; /* for 16bit RAMDAC, 0x0c for 8bit RAMDAC */
      else
	 tmp |= 0x08;
      break;
   }
   outb(vgaCRReg, tmp);


   /************  Big section for all but 911/914 **************/

   if (S3_801_928_SERIES(s3ChipId)) {
      /****** CR50 ******/
      outb(vgaCRIndex, 0x50);
      tmp = inb(vgaCRReg) & ~0xf1;
      switch (vga256InfoRec.bitsPerPixel) {
	case 8:
           break;
	case 16:
	   tmp |= 0x10;
	   break;
	case 24:
	   tmp |= 0x20;
	   break;
	case 32:
	   tmp |= 0x30;
	   break;
      }
      switch (s3DisplayWidth) { 
	case 640:
	   tmp |= 0x40;
	   break;
	case 800:
	   tmp |= 0x80;
	   break;
	case 1152:
	   tmp |= 0x01;
	   break;
	case 1280:
	   tmp |= 0xc0;
	   break;
	case 1600:
	   tmp |= 0x81;
	   break;
	default: /* 1024 and 2048 */
	   break;
      }
      outb(vgaCRReg, tmp);

      /****** CR51 ******/
      outb(vgaCRIndex, 0x51);
      s3Port51 = (inb(vgaCRReg) & 0xC0) | ((s3BppDisplayWidth >> 7) & 0x30);
      if (OFLG_ISSET(OPTION_STB_PEGASUS, &vga256InfoRec.options) ||
	  OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &vga256InfoRec.options)) {
	if (s3PixelMultiplexing)
	  /* In Pixel Multiplexing mode, disable split transfers. */
	  s3Port51 |= 0x40;
	else
	  /* In VGA mode, enable split transfers. */
	  s3Port51 &= ~0x40;
      }
      outb(vgaCRReg, s3Port51);

      
      /****** CR53 ******/
      outb(vgaCRIndex, 0x53);
      tmp = inb(vgaCRReg);

if(s3newmmio)
      tmp |= 0x18;
else
      tmp &= ~0x18;

      /*
       * Now the DRAM interleaving bit for the 801/805 chips
       * Note, we don't touch this bit for 928 chips because they use it
       * for pixel multiplexing control.
       */
      if (S3_801_SERIES(s3ChipId)) {
	 if (S3_805_I_SERIES(s3ChipId) && vga256InfoRec.videoRam == 2048)
	    tmp |= 0x20;
	 else
	    tmp &= ~0x20;
      }
      outb(vgaCRReg, tmp);


      /****** CR54 ******/
      n = 255;
      outb(vgaCRIndex, 0x54);
      if (S3_x64_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId)) {
	 int clock2,mclk;
	 clock2 = vga256InfoRec.clock[mode->Clock] * s3Bpp;
	 if (vga256InfoRec.s3MClk > 0) 
	    mclk = vga256InfoRec.s3MClk;
	 else if (S3_805_I_SERIES(s3ChipId))
	    mclk = 50000;  /* 50 MHz, guess for 805i limit */
	 else
	    mclk = 60000;  /* 60 MHz, limit for 864 */
	 if (vga256InfoRec.videoRam < 2048 || S3_TRIO32_SERIES(s3ChipId))
	    clock2 *= 2;
	 m = (int)((mclk/1000.0*.72+16.867)*89.736/(clock2/1000.0+39)-21.1543);
	 if (vga256InfoRec.videoRam < 2048 || S3_TRIO32_SERIES(s3ChipId))
	    m /= 2;
	 m -= vga256InfoRec.s3Madjust;
	 if (m > 31) m = 31;
	 else if (m < 0) {
	    m = 0;
	    n = 16;
	 }
      }
      else if (vga256InfoRec.videoRam == 512 || mode->HDisplay > 1200) 
	 m = 0;
      else if (vga256InfoRec.videoRam == 1024)
         m = 2;
      else
	 m = 20;

      if (OFLG_ISSET(OPTION_STB_PEGASUS, &vga256InfoRec.options))
 	tmp = 0x7F;
      else if (OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &vga256InfoRec.options))
	tmp = 0;
      else tmp = m << 3;
      outb(vgaCRReg, tmp);
      
      /****** CR60 ******/
      n -= vga256InfoRec.s3Nadjust;
      if (n < 0) n = 0;
      else if (n > 255) n = 255;
      outb(vgaCRIndex, 0x60);
      outb(vgaCRReg, n);

	
      /****** CR55 ******/
      if(!OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &vga256InfoRec.options)) {
	outb(vgaCRIndex, 0x55);
	/* remove mysterious dot at 60Hz */
	tmp = (inb(vgaCRReg) & 0x08) | 0x40;
	outb(vgaCRReg, tmp);
      }

      /* This shouldn't be needed -- they should be set by vgaHWInit() */
      if (!mode->CrtcVAdjusted) {
	 mode->CrtcVTotal /= interlacedivisor;
	 mode->CrtcVDisplay /= interlacedivisor;
	 mode->CrtcVSyncStart /= interlacedivisor;
	 mode->CrtcVSyncEnd /= interlacedivisor;
	 mode->CrtcVAdjusted = TRUE;
      }


      /****** CR5E ******/
      outb(vgaCRIndex, 0x5e);
      if ((S3_TRIO64V_SERIES(s3ChipId) && (s3ChipRev <= 0x53) && (s3Bpp==1)) ^
	  !!OFLG_ISSET(OPTION_TRIO64VP_BUG2, &vga256InfoRec.options))
	 i = (((mode->CrtcVTotal - 2) & 0x400) >> 10)  |
	     (((mode->CrtcVDisplay - 1) & 0x400) >> 9) |
	     (((mode->CrtcVDisplay - 1) & 0x400) >> 8) |
	     (((mode->CrtcVSyncStart) & 0x400) >> 6)   | 0x40;
      else
	 i = (((mode->CrtcVTotal - 2) & 0x400) >> 10)  |
	     (((mode->CrtcVDisplay - 1) & 0x400) >> 9) |
	     (((mode->CrtcVSyncStart) & 0x400) >> 8)   |
	     (((mode->CrtcVSyncStart) & 0x400) >> 6)   | 0x40;
      outb(vgaCRReg, i);

      if ((S3_TRIO64V_SERIES(s3ChipId) && (s3ChipRev <= 0x53) && (s3Bpp==1)) ^
	  !!OFLG_ISSET(OPTION_TRIO64VP_BUG2, &vga256InfoRec.options)) {
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


      /****** CR3B ******/
      outb(vgaCRIndex, 0x3b);
      if (DAC_IS_IBMRGB528) {
	 if (s3Bpp==1)
	    itmp = ((new->CRTC[4] + ((i&0x10)<<4) + 2) + 1) & ~1;
	 else
	    itmp = ((new->CRTC[4] + ((i&0x10)<<4) + 4) + 1) & ~1;
      }
      else {
	 itmp = (  new->CRTC[0] + ((i&0x01)<<8)
		 + new->CRTC[4] + ((i&0x10)<<4) + 1) / 2;
	 if (itmp-(new->CRTC[4] + ((i&0x10)<<4)) < 4)
	    if (new->CRTC[4] + ((i&0x10)<<4) + 4 <= new->CRTC[0]+ ((i&0x01)<<8))
	       itmp = new->CRTC[4] + ((i&0x10)<<4) + 4;
	    else
	       itmp = new->CRTC[0]+ ((i&0x01)<<8) + 1;
      }
      if (S3_AURORA64VP_SERIES(s3ChipId)) {
      	 outb(vgaCRReg, 0);
      	 i &= ~0x40;
      }
      else {
    	 outb(vgaCRReg, itmp & 0xff);
	 i |= (itmp&0x100) >> 2;
      }

      /****** CR3C ******/
      outb(vgaCRIndex, 0x3c);
     /* Interlace mode frame offset */
      outb(vgaCRReg, (new->CRTC[0] + ((i&0x01)<<8)) /2);	

      /****** CR5D ******/
      outb(vgaCRIndex, 0x5d);
      tmp = (inb(vgaCRReg) & 0x80) | i;
      outb(vgaCRReg, tmp);

      if (vga256InfoRec.videoRam > 1024 && S3_x64_SERIES(s3ChipId)) 
	 i = mode->HDisplay * s3Bpp / 8 + 1;
      else	/* XXX should be checked for 801/805 */
	 i = mode->HDisplay * s3Bpp / 4 + 1; 
      
      /****** CR61 ******/
      outb(vgaCRIndex, 0x61);
      tmp = 0x80 | (inb(vgaCRReg) & 0x60) | (i >> 8);
      outb(vgaCRReg, tmp);
      outb(vgaCRIndex, 0x62);
      outb(vgaCRReg, i & 0xff);
   } /*  (S3_801_928_SERIES(s3ChipId) */


   /****** CR42 ******/
   outb(vgaCRIndex, 0x42);
   tmp = inb(vgaCRReg);
   if ((mode->Flags & V_INTERLACE) != 0) 
      tmp |= 0x20;
   else 
      tmp &= ~0x20;
   outb(vgaCRReg, tmp);

   /****** CR67, CR6D and CR65 ******/
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


   /****** CR66 ******/
   /* PCI disconect enable -  perhaps this can be hidden from the 911
	and 914 in the 801_928 section above */
   outb(vgaCRIndex, 0x66);  
   tmp = inb(vgaCRReg);

   if(s3newmmio && s3PCIRetry)
   	outb(vgaCRReg, tmp | 0x88);
   else
   	outb(vgaCRReg, tmp | 0x80);


   if (OFLG_ISSET(OPTION_FAST_VRAM, &vga256InfoRec.options)) {
      outb(vgaCRIndex, 0x39);
      outb(vgaCRReg, 0xa5);
      outb(vgaCRIndex, 0x68);
      /* -RAS low timing 3.5 MCLKs, -RAS precharge timing 2.5 MCLKs */
      tmp = inb(vgaCRReg) | 0xf0;
      outb(vgaCRReg, tmp);
   }

   if (OFLG_ISSET(OPTION_SLOW_VRAM, &vga256InfoRec.options)) {
      /* 
       * some Diamond Stealth 64 VRAM cards have a problem with VRAM timing,
       * increase -RAS low timing from 3.5 MCLKs to 4.5 MCLKs 
       */ 
      outb(vgaCRIndex, 0x39);
      outb(vgaCRReg, 0xa5);
      outb(vgaCRIndex, 0x68);
      tmp = inb(vgaCRReg);
      if (tmp & 0x30) 				/* 3.5 MCLKs */
	 outb(vgaCRReg, tmp & 0xef);		/* 4.5 MCLKs */
   }

   if (OFLG_ISSET(OPTION_SLOW_DRAM, &vga256InfoRec.options)) {
      /* 
       * fixes some pixel errors for a SPEA Trio64V+ card
       * increase -RAS precharge timing from 2.5 MCLKs to 3.5 MCLKs 
       */ 
      outb(vgaCRIndex, 0x39);
      outb(vgaCRReg, 0xa5);
      outb(vgaCRIndex, 0x68);
      tmp = inb(vgaCRReg) & 0xf7;
      outb(vgaCRReg, tmp);	/* 3.5 MCLKs */
   }

   if (OFLG_ISSET(OPTION_SLOW_EDODRAM, &vga256InfoRec.options)) {
      /* 
       * fixes some pixel errors for a SPEA Trio64V+ card
       * increase from 1-cycle to 2-cycle EDO mode
       */ 
      outb(vgaCRIndex, 0x39);
      outb(vgaCRReg, 0xa5);
      outb(vgaCRIndex, 0x36);
      tmp = inb(vgaCRReg);
      if (!(tmp & 0x0c)) 		/* 1-cycle EDO */
	 outb(vgaCRReg, tmp | 0x08);		/* 2-cycle EDO */
   }

   if (S3_AURORA64VP_SERIES(s3ChipId)) {
      outb(0x3c4, 0x08);  /* unlock extended SEQ regs */
      outb(0x3c5, 0x06);
      if (OFLG_ISSET(OPTION_LCD_CENTER, &vga256InfoRec.options)) {
	 outb(0x3c4, 0x54);  outb(0x3c5, 0x10);
	 outb(0x3c4, 0x55);  outb(0x3c5, 0x00);
	 outb(0x3c4, 0x56);  outb(0x3c5, 0x1c);
	 outb(0x3c4, 0x57);  outb(0x3c5, 0x00);
      }
      else {
	 outb(0x3c4, 0x54);  outb(0x3c5, 0x1f);
	 outb(0x3c4, 0x55);  outb(0x3c5, 0x1f);
	 outb(0x3c4, 0x56);  outb(0x3c5, 0x1f);
	 outb(0x3c4, 0x57);  outb(0x3c5, 0xff);
      }
      outb(0x3c4, 0x08);  /* lock extended SEQ regs */
      outb(0x3c5, 0x00);
   }

   if (S3_964_SERIES(s3ChipId) && DAC_IS_BT485_SERIES) {
      if (OFLG_ISSET(OPTION_S3_964_BT485_VCLK, &vga256InfoRec.options)) {
	 /*
	  * This is the design alert from S3 with Bt485A and Vision 964. 
	  */
	 int i1, last, cr55, cr67;
	 int port, bit;


	 if (OFLG_ISSET(OPTION_DIAMOND, &vga256InfoRec.options)) {
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

	 for(last=i1=30; --i1;) {
	    S3RetraceWait();
	    S3RetraceWait();
	    if ((inb(port) & bit) == 0) { /* if GD2 is low then */
	       last = i1;
	       outb(vgaCRIndex, 0x67);
		/* clock should be inverted */
	       tmp = inb(vgaCRReg) ^ 0x01;
	       outb(vgaCRReg, tmp);
	    }
	    if (last-i1 > 4) break;
	 }
	 if (!i1) {  /* didn't get stable input, restore original CR67 value */
	    outb(vgaCRIndex, 0x67);
	    outb(vgaCRReg, cr67);
	 }
	 if (port == 0x3c8) {
	    outb(vgaCRIndex, 0x55);
	    outb(vgaCRReg, cr55); 
	 }
      }
   }

   if (OFLG_ISSET(OPTION_ELSA_W2000PRO_X8,  &vga256InfoRec.options)) {
      /* check LCLK/SCLK phase */
      unsigned char cr5c, cr42;

      outb(vgaCRIndex, 0x42);
      cr42 = inb(vgaCRReg);

      if (inb(0x3cc) & 0x40)   /* hsync polarity */
	 cr42 &= 0xfb;
      else
	 cr42 |= 0x04;
      outb(vgaCRReg, cr42);

      outb(vgaCRIndex, 0x5c);
      cr5c = inb(vgaCRReg);
      outb(vgaCRReg, cr5c | 0xa0);  /* set GD7 & GD5 */

      usleep(100000);   /* wait at least 2 VSYNCs to latch clock phase */

      if (inb(0x3c2) & 0x10)   /* query SENSE */
	 cr42 &= 0xf7;
      else
	 cr42 |= 0x08;

      outb(vgaCRIndex, 0x42);
      outb(vgaCRReg, cr42);

      outb(vgaCRIndex, 0x5c);
      outb(vgaCRReg, cr5c & 0x7f | 0x20);
   }

   if (OFLG_ISSET(CLOCK_OPTION_ICS2595, &vga256InfoRec.clockOptions))
         (void) (s3ClockSelectFunc)(mode->SynthClock);
         /* fixes the ICS2595 initialisation problems */

   S3Adjust(vga256InfoRec.frameX0, vga256InfoRec.frameY0);

   if ( S3_TRIO64V2_SERIES(s3ChipId) ) {
      /* disable DAC power saving to avoid bright left edge */
      outb(0x3d4,0x86);
      outb(0x3d5,0x80);
      /* disable the stream display fetch length control */
      outb(0x3d4,0x90);
      outb(0x3d5,0x00);
   }     

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

   inw(SUBSYS_STAT);

   outw(MULTIFUNC_CNTL, MEM_CNTL | VRTCFG_4 | HORCFG_8);

   outb(DAC_MASK, 0xff);

#ifdef PC98
   crtswitch(1);
#endif

	/***************************************\
	|	Set up Linear Addressing 	|
	\***************************************/
	

   if (s3InfoRec.ChipUseLinearAddressing) {
	outb(vgaCRIndex, 0x59); outb(vgaCRReg, s3Port59);
	outb(vgaCRIndex, 0x5a); outb(vgaCRReg, s3Port5A);
		
      	outb (vgaCRIndex, 0x58);
      	outb (vgaCRReg, s3LinApOpt | s3SAM256);        
   } else {
      	outb(vgaCRIndex, 0x58); 
	outb(vgaCRReg, s3SAM256);
   }

   WaitQueue(5);
   SET_SCISSORS(0,0,s3ScissR,s3ScissB);
   if(s3Bpp > 2) {
	if(s3newmmio)
      	  SET_MULT_MISC(0x200);
	else 
      	  SET_MULT_MISC(0);
    }

   return TRUE;
}





/*
 *  S3CleanUp --
 *
 */


void S3CleanUp()
{
   vgaS3Ptr restore = OldS3;
   short i;

#ifdef S3_DEBUG
  ErrorF("In S3CleanUp()\n");
#endif

   vgaProtect(TRUE);


   WaitQueue(8);
   S3BankZero();

   outw(ADVFUNC_CNTL, 0);

if(s3newmmio)
   outb(vgaCRIndex, 0x53); outb(vgaCRReg, 0x00);

  (s3Ramdacs[s3RamdacType].DacRestore)(restore);

   if (DAC_IS_TI3025) {
      outb(vgaCRIndex, 0x5C);
      outb(vgaCRReg, restore->s3sysreg[0x0C + 16]);
   }

 /* restore s3 special bits */
   if (S3_801_928_SERIES(s3ChipId)) {
    /* restore 801 specific registers */

      for (i = 32; i < (S3_x64_SERIES(s3ChipId) ? 46 : 
                      S3_805_I_SERIES(s3ChipId) ? 40 : 38) ; i++) {
	 outb(vgaCRIndex, 0x40 + i);
	 outb(vgaCRReg, restore->s3sysreg[i]);

      }
      for (i = 0; i < 16; i++) {
	 if (!((1 << i) & REG50_MASK))
	   continue;
	 outb(vgaCRIndex, 0x50 + i);
	 outb(vgaCRReg, restore->s3sysreg[i + 16]);
      }
   }
   for (i = 0; i < 5; i++) {
      outb(vgaCRIndex, 0x30 + i);
      outb(vgaCRReg, restore->s3reg[i]);
      outb(vgaCRIndex, 0x38 + i);
      outb(vgaCRReg, restore->s3reg[5 + i]);
   }

   for (i = 0; i < 16; i++) {
      outb(vgaCRIndex, 0x40 + i);
      outb(vgaCRReg, restore->s3sysreg[i]);
   }

   outb(vgaCRIndex, 0x45);
   inb(vgaCRReg);         /* reset color stack pointer */
   outb(vgaCRIndex, 0x4A);
   for (i = 0; i < 4; i++)
      outb(vgaCRReg, restore->ColorStack[i]);

   outb(vgaCRIndex, 0x45);
   inb(vgaCRReg);         /* reset color stack pointer */
   outb(vgaCRIndex, 0x4B);
   for (i = 4; i < 8; i++)
      outb(vgaCRReg, restore->ColorStack[i]);


   if (OFLG_ISSET(CLOCK_OPTION_ICS2595, &vga256InfoRec.clockOptions)){
      outb(vgaCRIndex, 0x42);
      outb(vgaCRReg, (restore->s3sysreg[2] & 0xf0) | 0x01);
      outb(vgaCRIndex, 0x5c);	/* switch back to 28MHz clock */
      outb(vgaCRReg,   0x20);
      outb(vgaCRReg,   0x00);
   }

   vgaHWRestore((vgaHWPtr)restore);

   outb(0x3c2, restore->Clock);

      
   vgaProtect(FALSE);
}

