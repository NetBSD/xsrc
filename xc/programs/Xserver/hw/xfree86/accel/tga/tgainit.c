/* $XConsortium: tgainit.c /main/9 1996/10/27 18:07:33 kaleb $ */
/*
 * Copyright 1995 by Alan Hourihane <alanh@fairlite.demon.co.uk>
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
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/tga/tgainit.c,v 3.8.2.1 1998/10/19 20:29:41 hohndel Exp $ */

#include "tga.h"
#include "tga_presets.h"

typedef struct {
	unsigned long Bt485[0x20];
	unsigned long tgaRegs[0x200];
        unsigned long Bt463[0x20];
        unsigned long Bt463win[48];
} tgaRegisters;
static tgaRegisters SR;

unsigned char *tgaVideoMemSave;

static int tgaInitialized = 0;
static Bool LUTInited = FALSE;
static LUTENTRY oldlut[256];
int tgaInitCursorFlag = TRUE;
int tgaHDisplay;
extern int tga_type;
extern struct tgamem tgamem;
extern int tgaWeight;
extern int tgaDisplayWidth;

void
tgaCalcCRTCRegs(crtcRegs, mode)
	tgaCRTCRegPtr	crtcRegs;
	DisplayModePtr	mode;
{
	crtcRegs->h_active = mode->CrtcHDisplay;
	crtcRegs->h_fporch = (mode->CrtcHSyncStart - mode->CrtcHDisplay) / 4;
	crtcRegs->h_sync   = (mode->CrtcHSyncEnd - mode->CrtcHSyncStart) / 4;
	crtcRegs->h_bporch = (mode->CrtcHTotal - mode->CrtcHSyncEnd) / 4;
	crtcRegs->v_active = mode->CrtcVDisplay;
	crtcRegs->v_fporch = mode->CrtcVSyncStart - mode->CrtcVDisplay;
	crtcRegs->v_sync   = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;
	crtcRegs->v_bporch = mode->CrtcVTotal - mode->CrtcVSyncEnd;

	/*
	 * We do polarity the Step B way of the 21030 
	 * Tell me how I can detect a Step A, and I'll support that too. 
	 * But I think that the Step B's are most common 
	 */
	if (mode->Flags & V_PHSYNC)
		crtcRegs->h_pol = 1;
	else
		crtcRegs->h_pol = 0;
	if (mode->Flags & V_PVSYNC)
		crtcRegs->v_pol = 1;
	else
		crtcRegs->v_pol = 0;

	crtcRegs->clock_sel = tgaInfoRec.clock[mode->Clock];
}

void
tgaSetCRTCRegs(crtcRegs)
	tgaCRTCRegPtr	crtcRegs;
{
	int virtX, virtY;

	virtX = ((crtcRegs->h_active / 4) & 0x1FF) |
		(((crtcRegs->h_active / 4) & 0x600) << 19) |
		(crtcRegs->h_fporch << 9) |
		(crtcRegs->h_sync << 14) |
		(crtcRegs->h_bporch << 21) |
		(crtcRegs->h_pol << 30);
	virtY = crtcRegs->v_active |
		(crtcRegs->v_fporch << 11) | 
		(crtcRegs->v_sync << 16) |
		(crtcRegs->v_bporch << 22) |
		(crtcRegs->v_pol << 30);

	TGA_WRITE_REG(0x00, TGA_VALID_REG); /* Disable Video */
	ICS1562ClockSelect(crtcRegs->clock_sel);
	TGA_WRITE_REG(virtX, TGA_HORIZ_REG);
	TGA_WRITE_REG(virtY, TGA_VERT_REG);
	TGA_WRITE_REG(0x01, TGA_VALID_REG); /* Enable Video */
}


unsigned
BT463_READ(unsigned m, unsigned a)
{
  unsigned val;

  BT463_LOAD_ADDR(a);
  TGA_WRITE_REG((m<<2)|0x2, TGA_RAMDAC_SETUP_REG);
  val = TGA_READ_REG(TGA_RAMDAC_REG);
  val = (val >> 16) & 0xff;
  return val;
}


unsigned
BT485_READ(unsigned reg)
{
  unsigned val;

  TGA_WRITE_REG(reg|0x1, TGA_RAMDAC_SETUP_REG);
  val = TGA_READ_REG(TGA_RAMDAC_REG);
  val = (val >> 16) & 0xff;
  return val;
}


void
saveTGAstate()
{
	int i, j;

	/* Do the RAMDAC */
	if (tga_type == 0) { /* 8-plane */
	  SR.Bt485[0] = BT485_READ(BT485_CMD_0);
	  SR.Bt485[1] = BT485_READ(BT485_CMD_1);
	  SR.Bt485[2] = BT485_READ(BT485_CMD_2);
	  SR.Bt485[3] = BT485_READ(BT485_CMD_3);
	  SR.Bt485[4] = BT485_READ(BT485_ADDR_PAL_WRITE);
	  SR.Bt485[5] = BT485_READ(BT485_PIXEL_MASK);
	} else {
	  SR.Bt463[0] = BT463_READ(BT463_REG_ACC, BT463_CMD_REG_0);
	  SR.Bt463[1] = BT463_READ(BT463_REG_ACC, BT463_CMD_REG_1);
	  SR.Bt463[2] = BT463_READ(BT463_REG_ACC, BT463_CMD_REG_2);

	  SR.Bt463[3] = BT463_READ(BT463_REG_ACC, BT463_READ_MASK_0);
	  SR.Bt463[4] = BT463_READ(BT463_REG_ACC, BT463_READ_MASK_1);
	  SR.Bt463[5] = BT463_READ(BT463_REG_ACC, BT463_READ_MASK_2);
	  SR.Bt463[6] = BT463_READ(BT463_REG_ACC, BT463_READ_MASK_3);

	  SR.Bt463[7] = BT463_READ(BT463_REG_ACC, BT463_BLINK_MASK_0);
	  SR.Bt463[8] = BT463_READ(BT463_REG_ACC, BT463_BLINK_MASK_1);
	  SR.Bt463[9] = BT463_READ(BT463_REG_ACC, BT463_BLINK_MASK_2);
	  SR.Bt463[10] = BT463_READ(BT463_REG_ACC, BT463_BLINK_MASK_3);

	  BT463_LOAD_ADDR(BT463_WINDOW_TYPE_BASE);
	  TGA_WRITE_REG((BT463_REG_ACC<<2)|0x2, TGA_RAMDAC_SETUP_REG);
  
	  for (i = 0, j = 0; i < 16; i++) {
	    SR.Bt463win[j++] = (TGA_READ_REG(TGA_RAMDAC_REG)>>16)&0xff;
	    SR.Bt463win[j++] = (TGA_READ_REG(TGA_RAMDAC_REG)>>16)&0xff;
	    SR.Bt463win[j++] = (TGA_READ_REG(TGA_RAMDAC_REG)>>16)&0xff;
	  }
	}

	SR.tgaRegs[0] = TGA_READ_REG(TGA_HORIZ_REG);
	SR.tgaRegs[1] = TGA_READ_REG(TGA_VERT_REG);
	SR.tgaRegs[2] = TGA_READ_REG(TGA_VALID_REG);
}


#if NeedFunctionPrototypes
void
restoreTGAstate(void)
#else
void
restoreTGAstate()
#endif
{
	int i;

	if (tga_type == 0) { /* 8-plane */ 
	  BT485_WRITE(SR.Bt485[0], BT485_CMD_0);
	  BT485_WRITE(SR.Bt485[1], BT485_CMD_1);
	  BT485_WRITE(SR.Bt485[2], BT485_CMD_2);
	  BT485_WRITE(SR.Bt485[3], BT485_CMD_3);
	  BT485_WRITE(SR.Bt485[4], BT485_ADDR_PAL_WRITE);
	  BT485_WRITE(SR.Bt485[5], BT485_PIXEL_MASK);
	} else {
	  int i, j;

	  BT463_WRITE(BT463_REG_ACC, BT463_CMD_REG_0, SR.Bt463[0]);
	  BT463_WRITE(BT463_REG_ACC, BT463_CMD_REG_1, SR.Bt463[1]);
	  BT463_WRITE(BT463_REG_ACC, BT463_CMD_REG_2, SR.Bt463[2]);

	  BT463_WRITE(BT463_REG_ACC, BT463_READ_MASK_0, SR.Bt463[3]);
	  BT463_WRITE(BT463_REG_ACC, BT463_READ_MASK_1, SR.Bt463[4]);
	  BT463_WRITE(BT463_REG_ACC, BT463_READ_MASK_2, SR.Bt463[5]);
	  BT463_WRITE(BT463_REG_ACC, BT463_READ_MASK_3, SR.Bt463[6]);

	  BT463_WRITE(BT463_REG_ACC, BT463_BLINK_MASK_0, SR.Bt463[7]);
	  BT463_WRITE(BT463_REG_ACC, BT463_BLINK_MASK_1, SR.Bt463[8]);
	  BT463_WRITE(BT463_REG_ACC, BT463_BLINK_MASK_2, SR.Bt463[9]);
	  BT463_WRITE(BT463_REG_ACC, BT463_BLINK_MASK_3, SR.Bt463[10]);

	  BT463_LOAD_ADDR(BT463_WINDOW_TYPE_BASE);
	  TGA_WRITE_REG((BT463_REG_ACC<<2), TGA_RAMDAC_SETUP_REG);
	  
	  for (i = 0, j = 0; i < 16; i++) {
	    TGA_WRITE_REG(SR.Bt463win[j++]|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
	    TGA_WRITE_REG(SR.Bt463win[j++]|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
	    TGA_WRITE_REG(SR.Bt463win[j++]|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
	  }
	}
	  
	TGA_WRITE_REG(0x00, TGA_VALID_REG); /* Disable Video */

	TGA_WRITE_REG(SR.tgaRegs[0], TGA_HORIZ_REG);
	TGA_WRITE_REG(SR.tgaRegs[1], TGA_VERT_REG);

	/*
	 * We MUST! do this to restore to a 25.175MHz clock as
	 * chosen by Jay Estabrook for Linux....
	 * Until I can find out how to read the clock register
	 */
	ICS1562ClockSelect(25175);

	TGA_WRITE_REG(SR.tgaRegs[2], TGA_VALID_REG); /* Re-enable Video */
}


#if NeedFunctionPrototypes
void
tgaCleanUp(void)
#else
void
tgaCleanUp()
#endif
{
	if (!tgaInitialized)
		return;

	restoreTGAstate();
	memcpy(tgaVideoMemSave, (unsigned char *)tgaVideoMem, 0x200000L);
	memset(tgaVideoMem, 0, 0x200000L);
}


#if NeedFunctionPrototypes
Bool
tgaInit(DisplayModePtr mode)
#else
Bool
tgaInit(mode)
	DisplayModePtr mode;
#endif
{
	int pitch_multiplier;

	if (!tgaInitialized)
		saveTGAstate();

	memcpy((unsigned char *)tgaVideoMem, tgaVideoMemSave, 0x200000L);

	tgaInitialized = 1;
	tgaInitCursorFlag = TRUE;

	return(TRUE);
}

void
#if NeedFunctionPrototypes
BT485Enable(void)
#else
BT485Enable()
#endif
{
   /* Specific BT485 setup, for UDB(Multia) 8plane TGA */
   BT485_WRITE(0x80 | (tgaDAC8Bit ? 2 : 0) | (tgaDACSyncOnGreen ? 8 : 0),
		 BT485_CMD_0);
   BT485_WRITE(0x01, BT485_ADDR_PAL_WRITE);
   BT485_WRITE(0x14, BT485_CMD_3); /* 64x64 cursor */
   switch (tgaInfoRec.depth) {
   case 8:
     BT485_WRITE(0x40, BT485_CMD_1);
     break;
   case 15:
     BT485_WRITE(0x30, BT485_CMD_1);
     break;
   case 16:
     BT485_WRITE(0x38, BT485_CMD_1);
     break;
   case 24:
     BT485_WRITE(0x10, BT485_CMD_1);
     break;
   }
   BT485_WRITE(0x20, BT485_CMD_2);
   BT485_WRITE(0xFF, BT485_PIXEL_MASK);
}

void
#if NeedFunctionPrototypes
BT463Enable(void)
#else
BT463Enable()
#endif
{
  int i;

  BT463_WRITE(BT463_REG_ACC, BT463_CMD_REG_0, 0x40);
/*  BT463_WRITE(BT463_REG_ACC, BT463_CMD_REG_1, 0x00); */
  BT463_WRITE(BT463_REG_ACC, BT463_CMD_REG_1, 0x08);
  BT463_WRITE(BT463_REG_ACC, BT463_CMD_REG_2, (tgaDACSyncOnGreen?0x80:0x00));
  
  BT463_WRITE(BT463_REG_ACC, BT463_READ_MASK_0, 0xff);
  BT463_WRITE(BT463_REG_ACC, BT463_READ_MASK_1, 0xff);
  BT463_WRITE(BT463_REG_ACC, BT463_READ_MASK_2, 0xff);
  BT463_WRITE(BT463_REG_ACC, BT463_READ_MASK_3, 0x0f);
  
  BT463_WRITE(BT463_REG_ACC, BT463_BLINK_MASK_0, 0x00);
  BT463_WRITE(BT463_REG_ACC, BT463_BLINK_MASK_1, 0x00);
  BT463_WRITE(BT463_REG_ACC, BT463_BLINK_MASK_2, 0x00);
  BT463_WRITE(BT463_REG_ACC, BT463_BLINK_MASK_3, 0x00);

  BT463_LOAD_ADDR(BT463_WINDOW_TYPE_BASE);
  TGA_WRITE_REG((BT463_REG_ACC<<2), TGA_RAMDAC_SETUP_REG);
  
  for (i = 0; i < 16; i++) {
    TGA_WRITE_REG(0x00|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
    TGA_WRITE_REG(0x01|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
    TGA_WRITE_REG(0x80|(BT463_REG_ACC<<10), TGA_RAMDAC_REG);
  }
}

#if NeedFunctionPrototypes
static void
InitLUT(void)
#else
static void
InitLUT()
#endif
{
   int i;

   if (tga_type == 0) { /* 8-plane  */

     /* Get BT485's pallette */
     BT485_WRITE(0x00, BT485_ADDR_PAL_WRITE);
     TGA_WRITE_REG(BT485_DATA_PAL, TGA_RAMDAC_SETUP_REG);
     
     for (i=0; i<256; i++) {
       oldlut[i].r = TGA_READ_REG(TGA_RAMDAC_REG);
       oldlut[i].g = TGA_READ_REG(TGA_RAMDAC_REG);
       oldlut[i].b = TGA_READ_REG(TGA_RAMDAC_REG);
     }

     for (i=0; i<16; i++) {
       TGA_WRITE_REG(0x00|(BT485_DATA_PAL<<8),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT485_DATA_PAL<<8),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT485_DATA_PAL<<8),TGA_RAMDAC_REG);
     }

     for (i=0; i<720; i+=4) {
       TGA_WRITE_REG(0x55|(BT485_DATA_PAL<<8),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT485_DATA_PAL<<8),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT485_DATA_PAL<<8),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT485_DATA_PAL<<8),TGA_RAMDAC_REG);
     } 

   } else {
     /* Running in TrueColor... we don't need this, do we? -tor */
#if 0
     /* Get BT463's pallette */
     BT463_LOAD_ADDR(0x0000);
     TGA_WRITE_REG((BT463_PALETTE<<2), TGA_RAMDAC_REG);
     
     for (i=0; i<256; i++) {
       oldlut[i].r = TGA_READ_REG(TGA_RAMDAC_REG);
       oldlut[i].g = TGA_READ_REG(TGA_RAMDAC_REG);
       oldlut[i].b = TGA_READ_REG(TGA_RAMDAC_REG);
     }

     for (i=0; i<16; i++) {
       TGA_WRITE_REG(0x00|(BT463_PALETTE<<10),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT463_PALETTE<<10),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT463_PALETTE<<10),TGA_RAMDAC_REG);
     }
     
     for (i=0; i<720; i+=4) {
       TGA_WRITE_REG(0x55|(BT463_PALETTE<<10),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT463_PALETTE<<10),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT463_PALETTE<<10),TGA_RAMDAC_REG);
       TGA_WRITE_REG(0x00|(BT463_PALETTE<<10),TGA_RAMDAC_REG);
     }
#endif
   }

   LUTInited = TRUE;
}


#if NeedFunctionPrototypes
void
tgaInitEnvironment(void)
#else
void
tgaInitEnvironment()
#endif
{
   if (tga_type == TYPE_TGA_8PLANE)
	BT485Enable();
   else
	BT463Enable();
   InitLUT();
}

#if NeedFunctionPrototypes
void
tgaInitAperture(screen_idx)
	int screen_idx;
#else
void
tgaInitAperture()
#endif
{
	tgaVideoMem = xf86MapVidMem(screen_idx, LINEAR_REGION,
					(pointer)(tgaInfoRec.MemBase |
					fb_offset_presets[tga_type]),
					tgaInfoRec.videoRam * 1024);

	if (tga_type != 0)
		tgaCursorMem = xf86MapVidMem(screen_idx, LINEAR_REGION,
					(pointer)(tgaInfoRec.MemBase |
					fb_offset_presets[tga_type] & 0xfff0000),
					0x4000);

	tgaVideoMemSave = (unsigned char *)xalloc(tgaInfoRec.videoRam * 1024);
	if (tgaVideoMemSave == NULL)
		FatalError("Unable to allocate save/restore buffer, "
			   "aborting.....\n");

#ifdef XFreeXDGA
	tgaInfoRec.physBase = (tgaInfoRec.MemBase + 
					fb_offset_presets[tga_type]);
	tgaInfoRec.physSize = tgaInfoRec.videoRam * 1024;
#endif
}	
