/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/IBMRGB.c,v 1.9.2.5 1999/06/17 16:24:08 hohndel Exp $ */
/*
 * Copyright 1995 The XFree86 Project, Inc
 *
 * programming the on-chip clock on the IBM RGB52x
 * Harald Koenig <koenig@tat.physik.uni-tuebingen.de>
 */
/* $XConsortium: IBMRGB.c /main/5 1996/05/07 17:13:25 kaleb $ */

#include "Xfuncproto.h"
#include "compiler.h"
#define NO_OSLIB_PROTOTYPES
#include "xf86.h"
#include "xf86_Config.h"

#include "glint.h"
#include "glint_regs.h"
#define GLINT_SERVER
#include "IBMRGB.h"

extern Bool     glintDoubleBufferMode;
extern int      coprotype;
extern Bool	UsePCIRetry;
extern Bool	gamma;

int             ActualDacId;

typedef enum _GLINT_RAMDACS
  {
    RGB525_RAMDAC = 0,
    RGB526_RAMDAC,
    RGB526DB_RAMDAC,
    RGB528_RAMDAC,
    RGB528A_RAMDAC,
    RGB624_RAMDAC,
    RGB624DB_RAMDAC,
    RGB640_RAMDAC,
    RGB524_RAMDAC,
    RGB524A_RAMDAC,
    MAX_GLINT_RAMDAC
  }
GLINT_RAMDACS;


/*
 * glintOutIBMRGBIndReg() and glintInIBMRGBIndReg() are used to access 
 * the indirect RGB52x registers only.
 */

void
glintOutIBMRGBIndReg (CARD32 reg, unsigned char mask, unsigned char data)
{
  unsigned char tmp = 0x00;

  GLINT_SLOW_WRITE_REG (reg, IBMRGB_INDEX_LOW);
  GLINT_SLOW_WRITE_REG (reg>>8, IBMRGB_INDEX_HIGH);

  if (mask != 0x00)
    tmp = GLINT_READ_REG (IBMRGB_INDEX_DATA) & mask;
  GLINT_SLOW_WRITE_REG (tmp | data, IBMRGB_INDEX_DATA);
}

unsigned char
glintInIBMRGBIndReg (CARD32 reg)
{
  unsigned char ret;

  GLINT_SLOW_WRITE_REG (reg, IBMRGB_INDEX_LOW);
  GLINT_SLOW_WRITE_REG (reg>>8, IBMRGB_INDEX_HIGH);
  ret = GLINT_READ_REG (IBMRGB_INDEX_DATA);

  return (ret);
}

/* Standart Mode , video.c */

#define INITIALFREQERR 100000

unsigned long
                RGB640_CalculateMNPCForClock(
    unsigned long RefClock,	/* In 100Hz units */
    unsigned long ReqClock,	/* In 100Hz units */
    char IsPixClock,	/* boolean, is this the pixel or the sys clock */
    unsigned long MinClock,	/* Min VCO rating */
    unsigned long MaxClock,	/* Max VCO rating */
    unsigned long *rM,	/* M Out */
    unsigned long *rN,	/* N Out */
    unsigned long *rP,	/* Min P In, P Out */
    unsigned long *rC	/* C Out */
)
{
  unsigned long   M, N, P, iP = *rP;
  unsigned long   IntRef, VCO, Clock;
  long            freqErr, lowestFreqErr = INITIALFREQERR;
  unsigned long   ActualClock = 0;

  for (N = 0; N <= 63; N++)
    {
      IntRef = RefClock / (N + 1);
      if (IntRef < 10000)
	break;			/* IntRef needs to be >= 1MHz */
      for (M = 2; M <= 127; M++)
	{
	  VCO = IntRef * (M + 1);
	  if ((VCO < MinClock) || (VCO > MaxClock))
	    continue;
	  for (P = iP; P <= 4; P++)
	    {
	      if (P != 0)
		Clock = (RefClock * (M + 1)) / ((N + 1) * 2 * P);
	      else
		Clock = (RefClock * (M + 1)) / (N + 1);

	      freqErr = (Clock - ReqClock);

	      if (freqErr < 0)
		{
		  /* PixelClock gets rounded up always so monitor reports
		     correct frequency. */
		  if (IsPixClock)
		    continue;
		  freqErr = -freqErr;
		}

	      if (freqErr < lowestFreqErr)
		{
		  *rM = M;
		  *rN = N;
		  *rP = P;
		  *rC = (VCO <= 1280000 ? 1 : 2);
		  ActualClock = Clock;

		  lowestFreqErr = freqErr;
		  /* Return if we found an exact match */
		  if (freqErr == 0)
		    return (ActualClock);
		}
	    }
	}
    }

  return (ActualClock);
}

unsigned long
                RGB526_CalculateMNPCForClock
                (
		  unsigned long RefClock,	/* In 100Hz units */
		  unsigned long ReqClock,	/* In 100Hz units */
		  char IsPixClock,	/* boolean, is this the pixel or the sys clock */
		  unsigned long MinClock,	/* Min VCO rating */
		  unsigned long MaxClock,	/* Max VCO rating */
		  unsigned long *rM,	/* M Out */
		  unsigned long *rN,	/* N Out */
		  unsigned long *rP,	/* Min P In, P Out */
		  unsigned long *rC	/* C Out */
)
{
  unsigned long   M, N, P, iP = *rP;
  unsigned long   IntRef, VCO, Clock;
  long            freqErr, lowestFreqErr = INITIALFREQERR;
  unsigned long   ActualClock = 0;

  for (N = 0; N <= 63; N++)
    {
      IntRef = RefClock / (N + 1);
      if (IntRef < 10000)
	break;			/* IntRef needs to be >= 1MHz */
      for (M = 0; M <= 63; M++)
	{
	  VCO = IntRef * (M + 1);
	  if ((VCO < MinClock) || (VCO > MaxClock))
	    continue;
	  for (P = iP; P <= 4; P++)
	    {
	      if (P)
		Clock = (RefClock * (M + 1)) / ((N + 1) * 2 * P);
	      else
		Clock = VCO;

	      freqErr = (Clock - ReqClock);

	      if (freqErr < 0)
		{
		  /* PixelClock gets rounded up always so monitor reports
		     correct frequency. */
		  if (IsPixClock)
		    continue;
		  freqErr = -freqErr;
		}

	      if (freqErr < lowestFreqErr)
		{
		  *rM = M;
		  *rN = N;
		  *rP = P;
		  *rC = (VCO <= 1280000 ? 1 : 2);
		  ActualClock = Clock;

		  lowestFreqErr = freqErr;
		  /* Return if we found an exact match */
		  if (freqErr == 0)
		    return (ActualClock);
		}
	    }
	}
    }

  return (ActualClock);
}

void
CheckRGBClockInteraction (unsigned long *PixelClock,
			  unsigned long *SystemClock)
{
  unsigned long  *fLower;
  unsigned long  *fHigher;
  long            nfLower;

  if (*PixelClock < *SystemClock)
    {
      fLower = PixelClock;
      fHigher = SystemClock;
    }
  else
    {
      fLower = SystemClock;
      fHigher = PixelClock;
    }

  while (1)
    {
      /* ErrorF("Checking for interaction between %d and %d\n", *fLower, *fHigher); */
      nfLower = *fLower;
      while (nfLower - 20000 <= *fHigher)
	{
	  if (*fHigher <= (nfLower + 20000))
	    {
	      /*  100KHz adjustments */
	      if (*fHigher > nfLower)
		{
		  *fLower -= 1000;
		  *fHigher += 1000;
		}
	      else
		{
		  *fLower += 1000;
		  *fHigher -= 1000;
		}
#ifdef DEBUG
	      ErrorF ("Interaction problem: adjusted to %d and %d\n", *fLower, *fHigher);
#endif
	      break;
	    }
	  nfLower += *fLower;
	}
      if (nfLower - 20000 > *fHigher)
	break;
    }
}


static void
glintProgramIBMRGBClock (int clk, unsigned char m, unsigned char n,
			 unsigned char df)
{
#ifdef COMPATIBELMODE
  int             tmp;

  tmp = 1;
  glintOutIBMRGBIndReg (IBMRGB_misc_clock, ~1, 1);

  glintOutIBMRGBIndReg (IBMRGB_m0 + 2 * clk, 0, (df << 6) | (m & 0x3f));
  glintOutIBMRGBIndReg (IBMRGB_n0 + 2 * clk, 0, n);

  glintOutIBMRGBIndReg (IBMRGB_pll_ctrl2, 0xf0, clk);
  glintOutIBMRGBIndReg (IBMRGB_pll_ctrl1, 0xf8, 3);
#endif
}

void
IBMRGBGlintSetClock (long freq, int clk, long dacspeed, long fref)
{
  volatile double ffreq, fdacspeed, ffref;
  volatile int    df, n, m, max_n, min_df;
  volatile int    best_m = 69, best_n = 17, best_df = 0;
  volatile double diff, mindiff;

#define FREQ_MIN   16250	/* 1000 * (0+65) / 4 */
#define FREQ_MAX  dacspeed

  if (freq < FREQ_MIN)
    ffreq = FREQ_MIN / 1000.0;
  else if (freq > FREQ_MAX)
    ffreq = FREQ_MAX / 1000.0;
  else
    ffreq = freq / 1000.0;

  fdacspeed = dacspeed / 1e3;
  ffref = fref / 1e3;

  ffreq /= ffref;
  ffreq *= 16;
  mindiff = ffreq;

  if (freq <= dacspeed / 4)
    min_df = 0;
  else if (freq <= dacspeed / 2)
    min_df = 1;
  else
    min_df = 2;

  for (df = 0; df < 4; df++)
    {
      ffreq /= 2;
      mindiff /= 2;
      if (df < min_df)
	continue;

      /* the remaining formula is  ffreq = (m+65) / n */

      if (df < 3)
	max_n = fref / 1000 / 2;
      else
	max_n = fref / 1000;
      if (max_n > 31)
	max_n = 31;

      for (n = 2; n <= max_n; n++)
	{
	  m = (int) (ffreq * n + 0.5) - 65;
	  if (m < 0)
	    m = 0;
	  else if (m > 63)
	    m = 63;

	  diff = (m + 65.0) / n - ffreq;
	  if (diff < 0)
	    diff = -diff;

	  if (diff < mindiff)
	    {
	      mindiff = diff;
	      best_n = n;
	      best_m = m;
	      best_df = df;
	    }
	}
    }

#ifdef DEBUG
  ErrorF ("clk %d, setting to %f, m 0x%02x %d, n 0x%02x %d, df %d\n", clk,
	  ((best_m + 65.0) / best_n) / (8 >> best_df) * ffref,
	  best_m, best_m, best_n, best_n, best_df);
#endif

  glintProgramIBMRGBClock (clk, best_m, best_n, best_df);
}

int
glintIBMRGB_Probe ()
{
  unsigned char   ilow, ihigh, id, rev, id2, rev2;
  int             ret = 0;

  /* read ID and revision */
  ilow = GLINT_READ_REG (IBMRGB_INDEX_LOW);
  ihigh = GLINT_READ_REG (IBMRGB_INDEX_HIGH);
  GLINT_SLOW_WRITE_REG (0, IBMRGB_INDEX_HIGH);	/* index high */
  GLINT_SLOW_WRITE_REG (IBMRGB_rev, IBMRGB_INDEX_LOW);
  rev = GLINT_READ_REG (IBMRGB_INDEX_DATA);
  GLINT_SLOW_WRITE_REG (IBMRGB_id, IBMRGB_INDEX_LOW);
  id = GLINT_READ_REG (IBMRGB_INDEX_DATA);


  if (id == 0x30)
    {
      if (rev == 0xc0)
	ActualDacId = RGB624_RAMDAC;
      if (rev == 0x80)
	ActualDacId = RGB624DB_RAMDAC;
    }

  if (id == 0x12)
    ActualDacId = RGB640_RAMDAC;

  /* known IDs:  
     1 = RGB525
     2 = RGB524, RGB528 
    12 = RGB640
   */

  if (id == 1 || id == 2 || id == 0x12)
    {

      if (id == 1)
	ActualDacId = RGB525_RAMDAC;

      if (id == 2)
	{
	  switch (rev)
	    {
	    case 0xf0:
	      ActualDacId = RGB524_RAMDAC;
	      break;
	    case 0xe0:
	      ActualDacId = RGB524A_RAMDAC;
	      break;
	    case 0xc0:
	      ActualDacId = RGB526_RAMDAC;
	      break;
	    case 0x80:
	      ActualDacId = RGB526DB_RAMDAC;
	      break;
	    }
	}

      /* check if ID and revision are read only */
      GLINT_SLOW_WRITE_REG (IBMRGB_rev, IBMRGB_INDEX_LOW);
      GLINT_SLOW_WRITE_REG (~rev, IBMRGB_INDEX_DATA);
      GLINT_SLOW_WRITE_REG (IBMRGB_id, IBMRGB_INDEX_LOW);
      GLINT_SLOW_WRITE_REG (~id, IBMRGB_INDEX_DATA);
      GLINT_SLOW_WRITE_REG (IBMRGB_rev, IBMRGB_INDEX_LOW);
      rev2 = GLINT_READ_REG (IBMRGB_INDEX_DATA);
      GLINT_SLOW_WRITE_REG (IBMRGB_id, IBMRGB_INDEX_LOW);
      id2 = GLINT_READ_REG (IBMRGB_INDEX_DATA);

      if (id == id2 && rev == rev2)
	{			/* IBM RGB52x found */
	  ret = (id << 8) | rev;

	  /* check for 128bit VRAM -> RGB528 */
	  GLINT_SLOW_WRITE_REG (IBMRGB_misc1, IBMRGB_INDEX_LOW);

	  if ((GLINT_READ_REG (IBMRGB_INDEX_DATA) & 0x03) == 0x03)
	    {
	      /* 128bit DAC found */
	      ret |= 1 << 16;
	      ActualDacId = RGB528_RAMDAC;

	      if (rev == 0xe0)
		ActualDacId = RGB528A_RAMDAC;
	    }
	}
      else
	{
	  GLINT_SLOW_WRITE_REG (IBMRGB_rev, IBMRGB_INDEX_LOW);
	  GLINT_SLOW_WRITE_REG (rev, IBMRGB_INDEX_DATA);
	  GLINT_SLOW_WRITE_REG (IBMRGB_id, IBMRGB_INDEX_LOW);
	  GLINT_SLOW_WRITE_REG (id, IBMRGB_INDEX_DATA);
	}
    }
  GLINT_SLOW_WRITE_REG (ilow, IBMRGB_INDEX_LOW);
  GLINT_SLOW_WRITE_REG (ihigh, IBMRGB_INDEX_HIGH);

  return ret;
}

void
glintIBMRGB_Init ()
{
  unsigned char tmp;

  tmp = GLINT_READ_REG (IBMRGB_INDEX_CONTROL);
  /* turn off auto-increment */
  GLINT_SLOW_WRITE_REG (tmp & ~1, IBMRGB_INDEX_CONTROL);
  GLINT_SLOW_WRITE_REG (0, IBMRGB_INDEX_HIGH);	/* index high byte */
}

int
IBMRGB52x_Init_Stdmode (int clock)
{
  CARD32 temp;
  unsigned long P, M, N, C, RefClkSpeed;
  unsigned long PixelClock, SystemClock, MaxVco = 2200000;

  /*
   * for the RGB640 we need to initialize a few things to save values
   */
  if (ActualDacId == RGB640_RAMDAC) {
    /*
     * turn off pixel interleave, set to DAC8Bit Mode and VRAM access
     */
    glintOutIBMRGBIndReg (RGB640_PIXEL_INTERLEAVE, 0, 0);
    glintOutIBMRGBIndReg (RGB640_VGA_CONTROL, 0, 
			  IBM640_RDBK | IBM640_PSIZE8 | IBM640_VRAM);
    /* enable all three DACs and groud the complementary output */
    glintOutIBMRGBIndReg (RGB640_DAC_CONTROL, 0, 
			  IBM640_DACENBL | IBM640_SHUNT);
    /* don't autoincrement for read, do autoincrement for write, update window
       attribute table during retrace only */
    glintOutIBMRGBIndReg (RGB640_OUTPUT_CONTROL, 0, 
			  IBM640_RDAI | IBM640_WATCTL);
    glintOutIBMRGBIndReg (RGB640_SYNC_CONTROL, 0, 0);
    /* disable VRAM masking */
    glintOutIBMRGBIndReg (RGB640_VRAM_MASK0, 0, 0xff);
    glintOutIBMRGBIndReg (RGB640_VRAM_MASK1, 0, 0xff);
    glintOutIBMRGBIndReg (RGB640_VRAM_MASK2, 0, 0x0f);
    /* next we set up the serializer; this depends on the color depth */
    switch (glintInfoRec.bitsPerPixel) {
    case 8:  
      glintOutIBMRGBIndReg (RGB640_SER_07_00, 0, 0x00);
      glintOutIBMRGBIndReg (RGB640_SER_15_08, 0, 0x00);
      glintOutIBMRGBIndReg (RGB640_SER_23_16, 0, 0x00);
      glintOutIBMRGBIndReg (RGB640_SER_31_24, 0, 0x00);
      glintOutIBMRGBIndReg (RGB640_SER_MODE,  0, IBM640_SER_16_1);
      temp = 0x03;
      break;
    case 15: 
    case 16:
      glintOutIBMRGBIndReg (RGB640_SER_07_00, 0, 0x10);
      glintOutIBMRGBIndReg (RGB640_SER_15_08, 0, 0x11);
      glintOutIBMRGBIndReg (RGB640_SER_23_16, 0, 0x00);
      glintOutIBMRGBIndReg (RGB640_SER_31_24, 0, 0x00);
      glintOutIBMRGBIndReg (RGB640_SER_MODE,  0, IBM640_SER_8_1);
      temp = 0x05;
      break;
    case 24:
    case 32:
      glintOutIBMRGBIndReg (RGB640_SER_07_00, 0, 0x30);
      glintOutIBMRGBIndReg (RGB640_SER_15_08, 0, 0x31);
      glintOutIBMRGBIndReg (RGB640_SER_23_16, 0, 0x32);
      glintOutIBMRGBIndReg (RGB640_SER_31_24, 0, 0x33);
      glintOutIBMRGBIndReg (RGB640_SER_MODE,  0, IBM640_SER_4_1);
      temp = 0x09;
      break;
    }
    { 
	int i;
    	for (i=0x100;i<0x140;i+=4) {
	    /* Initialize FrameBuffer Window Attribute Table */
	    glintOutIBMRGBIndReg(i, 0, temp);
	    glintOutIBMRGBIndReg(i+1, 0, 0x00);
	    glintOutIBMRGBIndReg(i+2, 0, 0x00);
	    glintOutIBMRGBIndReg(i+3, 0, 0x00);
	    /* Initialize Overlay Window Attribute Table */
	    glintOutIBMRGBIndReg(i+0x100, 0, 0x00);
	    glintOutIBMRGBIndReg(i+0x100, 0, 0x00);
	    glintOutIBMRGBIndReg(i+0x100, 0, 0x00);
	    glintOutIBMRGBIndReg(i+0x100, 0, 0x44);
        }
    }
    M = N = P = C = 0;
    if (gamma) {
      PixelClock =
          RGB640_CalculateMNPCForClock (28322, clock, 1, 65000, 220000,
				      &M, &N, &P, &C);
    } else {
      PixelClock =
          RGB640_CalculateMNPCForClock (40000, clock, 1, 65000, 220000,
				      &M, &N, &P, &C);
    }
    glintOutIBMRGBIndReg (RGB640_AUX_PLL_CTL, 0, 0x00);
    glintOutIBMRGBIndReg (RGB640_MISC_CONF, 0, IBM640_PCLK_8);
    glintOutIBMRGBIndReg (RGB640_PLL_M,   0, M);
    glintOutIBMRGBIndReg (RGB640_PLL_N,   0, N);
    glintOutIBMRGBIndReg (RGB640_PLL_P,   0, P<<1);
    glintOutIBMRGBIndReg (RGB640_PLL_CTL, 0, C | IBM640_PLL_EN);
	return 1;
  } 
  else {
    /*
     * the GLINT uses SCLK so there's no need to set the
     * DDOTCLK to a special value
     * we disable the DDOTCLK
     */
    /* hefa, System Clock Control, 0x05 0=1 -> SYSCLK PLL enabled,
       2 = 1 -> Standard Mode - program with N, M, P, and C registers */
    /* glintOutIBMRGBIndReg(IBMRGB_sysclk, 0, 0x05); */

    if (IS_3DLABS_TX_MX_CLASS (coprotype))
    {
      glintOutIBMRGBIndReg (IBMRGB_misc1, 0, SENS_DSAB_DISABLE | VRAM_SIZE_64);
      glintOutIBMRGBIndReg (IBMRGB_misc2, 0, COL_RES_8BIT | PORT_SEL_VRAM | PCLK_SEL_PLL);

    }
    else if (IS_3DLABS_PERMEDIA_CLASS (coprotype))
    {
      glintOutIBMRGBIndReg (IBMRGB_misc1, 0, SENS_DSAB_DISABLE | VRAM_SIZE_32);
      if (glintInfoRec.bitsPerPixel == 32)
	glintOutIBMRGBIndReg (IBMRGB_misc2, 0, COL_RES_8BIT | PORT_SEL_VRAM | PCLK_SEL_LCLK);
      else
	glintOutIBMRGBIndReg (IBMRGB_misc2, 0, COL_RES_8BIT | PORT_SEL_VRAM | PCLK_SEL_PLL);
    }

    /* this is a hack for accelerated 16 bpp mode for 3.9o, which swaps
       red and blue pixel components */

    /* Swap RB for no_accel */
    if (glintDoubleBufferMode)
      glintOutIBMRGBIndReg (IBMRGB_misc3, 0, 0x01);
    else
      glintOutIBMRGBIndReg (IBMRGB_misc3, 0, 0x00);

    if (IS_3DLABS_TX_MX_CLASS (coprotype))
    {
      glintOutIBMRGBIndReg (IBMRGB_misc_clock, 0xf0, 0x87);
    }
    else if (IS_3DLABS_PERMEDIA_CLASS (coprotype))
    {
      glintOutIBMRGBIndReg (IBMRGB_misc_clock, 0x0, 0x1);
    }

    glintOutIBMRGBIndReg (IBMRGB_sync, 0, 0);
    glintOutIBMRGBIndReg (IBMRGB_hsync_pos, 0, 0);

    /* glintOutIBMRGBIndReg(IBMRGB_pwr_mgmt, 0, 0x08); *//* disable DDOTCLK */
    glintOutIBMRGBIndReg (IBMRGB_pwr_mgmt, 0, 0x0);
    /* glintOutIBMRGBIndReg(IBMRGB_dac_op, 0, DPE_ENABLE | DSR_DAC_SLOW); *//* Fast DAC mode */
    glintOutIBMRGBIndReg (IBMRGB_dac_op, 0, 0);

    glintOutIBMRGBIndReg (IBMRGB_pal_ctrl, 0, 0);
    glintOutIBMRGBIndReg (IBMRGB_curs, 0, 0x20);

    switch (glintInfoRec.bitsPerPixel)
    {
    case 32:
      glintOutIBMRGBIndReg (IBMRGB_pix_fmt, 0xf8, 6);
      glintOutIBMRGBIndReg (IBMRGB_32bpp, 0, 0x03);
      break;
      /* not supported by glint 
	 case 24:
	 glintOutIBMRGBIndReg(IBMRGB_pix_fmt, 0xf8, 5);
	 glintOutIBMRGBIndReg(IBMRGB_24bpp, 0, 0x01);
	 break; */
    case 16:
      glintOutIBMRGBIndReg (IBMRGB_pix_fmt, 0xf8, 4);
      glintOutIBMRGBIndReg (IBMRGB_16bpp, 0, 0xC7);
      break;
    case 15:
      glintOutIBMRGBIndReg (IBMRGB_pix_fmt, 0xf8, 4);
      glintOutIBMRGBIndReg (IBMRGB_16bpp, 0, 0xC5);
      break;
    case 8:
      glintOutIBMRGBIndReg (IBMRGB_pix_fmt, 0xf8, 3);
      glintOutIBMRGBIndReg (IBMRGB_8bpp, 0, 0x00);
      break;
    }
    /* if 526 oder 624 ramdac */
    if ((ActualDacId == RGB526DB_RAMDAC) ||
	(ActualDacId == RGB526_RAMDAC) ||
	(ActualDacId == RGB640_RAMDAC) ||
	(ActualDacId == RGB624DB_RAMDAC) ||
	(ActualDacId == RGB624_RAMDAC))
    {

      RefClkSpeed = IS_3DLABS_TX_MX_CLASS (coprotype) ?
	GLINT_DEFAULT_CLOCK_SPEED_DELTA : PERMEDIA_REF_CLOCK_SPEED;

      PixelClock = clock * 1000;

      SystemClock = IS_3DLABS_TX_MX_CLASS (coprotype) ?
	GLINT_DEFAULT_CLOCK_SPEED :
	(1000000000L) / ((GLINT_READ_REG (ChipConfig) >> 28) + 10);

      SystemClock /= 100;
      RefClkSpeed /= 100;
      PixelClock /= 100;
      CheckRGBClockInteraction (&PixelClock, &SystemClock);

      P = M = N = C = 0;
      PixelClock =
	RGB526_CalculateMNPCForClock (RefClkSpeed, PixelClock, 1, 650000, MaxVco,
				      &M, &N, &P, &C);
#ifdef DEBUG
      if (!PixelClock)
	ErrorF ("Glit - RGB526_CalculateMNPCForClock == 0!\n");
      else
	ErrorF ("Systemclock: %d\nRefClkSpeed: %d\nPixelclock: %d\nPixelclockregister: M:0x%x N:0x%x P:0x%x C:0x%x.\n", SystemClock * 100, RefClkSpeed * 100, PixelClock * 100, M, N, P, C);
#endif
      glintOutIBMRGBIndReg (IBMRGB_pll_ctrl1, 0x00, 0x5);
      glintOutIBMRGBIndReg (IBMRGB_pll_ctrl2, 0x00, 0x0);

      glintOutIBMRGBIndReg (IBMRGB_m0, 0x00, M);
      glintOutIBMRGBIndReg (IBMRGB_n0, 0x00, N);
      glintOutIBMRGBIndReg (IBMRGB_p0, 0x00, P);
      glintOutIBMRGBIndReg (IBMRGB_c0, 0x00, C);


      glintOutIBMRGBIndReg (IBMRGB_sysclk, 0x00, 0x05);

      P = 1;
      SystemClock =
	RGB526_CalculateMNPCForClock (RefClkSpeed, PixelClock, 0, 650000, MaxVco,
				      &M, &N, &P, &C);
#ifdef DEBUG
      if (!SystemClock)
	ErrorF ("Glit - RGB526_CalculateMNPCForClock == 0!\n");
      else
	ErrorF ("Systemclockregister: M:0x%x N:0x%x P:0x%x C:0x%x.\n", M, N, P, C);
#endif

      /* set the registers */
      glintOutIBMRGBIndReg (IBMRGB_sysclk_m, 0x00, M);
      glintOutIBMRGBIndReg (IBMRGB_sysclk_n, 0x00, N);
      glintOutIBMRGBIndReg (IBMRGB_sysclk_p, 0x00, P);
      glintOutIBMRGBIndReg (IBMRGB_sysclk_c, 0x00, C);
    }
  }
  return 1;
}
