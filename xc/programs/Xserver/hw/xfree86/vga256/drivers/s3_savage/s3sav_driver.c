/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3sav_driver.c,v 1.1.2.4 1999/12/01 12:49:34 hohndel Exp $ */

/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */

/*
 * This version of the driver has been extended to support the Savage3D and
 * Savage4 drivers, and to add support for using the BIOS to switch modes.
 *
 * Tim Roberts, 21-June-1999.
 */

/* This is an intial version of the ViRGE driver for XAA 
 * Started 09/03/97 by S. Marineau
 *
 * What works: 
 * - Supports PCI hardware, ViRGE and ViRGE/VX, probably ViRGE/DXGX
 * - Supports 8bpp, 16bpp and 24bpp. There is some support for 32bpp.
 * - VT switching seems to work well, no corruption. 
 * - A whole slew of XConfig options for memory, PCI and acceleration
 * - Acceleration is quite complete
 * 
 * 
 * What does not work:
 * - None of this doublescan stuff
 *
 * 
 * What I attempt to do here:
 *
 *  - Rewrite the init/save functions from the accel server such that they
 *    work as XAA intends
 *  - Keep the driver variables inside a private data structure, to avoid having
 *    millions of global variables.
 *  - Keep the structure as simple as possible, try to move RAMDAC stuff to 
 *    separate source files.
 *
 *  So much for that.... We cannot use the s3 ramdac code, because we 
 *  want to wait before we write out any registers. Fortunately, the 
 *  built-in virge ramdac is straighforward to set-up. Also, I did not succeed
 *  in keeping the code as modular as I had wished... 
 *
 *
 * Notes:
 * - The driver only supports linear addressing and MMIO. 
 *
 */


/* General and xf86 includes */
#include "X.h"
#include "input.h"
#include "screenint.h"
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

/* Includes for Options flags */
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

/* DGA includes */
#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

/* S3V internal includes */
#include "s3sav_driver.h"
#include "regs3sav.h"

/*
 * If the symbol USEBIOS is defined, we try to use the onboard BIOS to do
 * mode switches.  We query the VESA BIOS list, and match the requested
 * timing mode to the VESA list as closely as possible.  This lets us
 * inherit the considerable effort placed into the S3 BIOS.
 *
 * The BIOS method will be attempted on all Linux x86 systems.
 */

static Bool    S3SAVProbe();
static char *  S3SAVIdent();
static Bool    S3SAVClockSelect();
static void    S3SAVEnterLeave();
static Bool    S3SAVInit();
static int     S3SAVValidMode();
static void *  S3SAVSave();
static void    S3SAVRestore();
static void    S3SAVAdjust();
static void    S3SAVFbInit();
void           S3SAVSetRead();
void           S3SAVAccelInit();
void           S3SAVInitialize2DEngine();
void           S3SAVInitSTREAMS();
void           S3SAVDisableSTREAMS();
void           S3SAVRestoreSTREAMS();
void           S3SAVSaveSTREAMS();
void           S3SAVSetGBD();

/* Temporary debug function to print virge regs */
void S3SAVPrintRegs();

/*
 * And the data structure which defines the driver itself 
 * This is mostly the struct from the s3 driver with adjusted names 
 * and adjusted default values.
 */

vgaVideoChipRec S3_SAVAGE = {
  S3SAVProbe,              /* Bool (* ChipProbe)() */
  S3SAVIdent,              /* char * (* ChipIdent)() */
  S3SAVEnterLeave,         /* void (* ChipEnterLeave)() */
  S3SAVInit,               /* Bool (* ChipInit)() */
  S3SAVValidMode,          /* int (* ChipValidMode)() */
  S3SAVSave,               /* void * (* ChipSave)() */
  S3SAVRestore,            /* void (* ChipRestore)() */
  S3SAVAdjust,             /* void (* ChipAdjust)() */
  vgaHWSaveScreen,       /* void (* ChipSaveScreen)() */
  (void(*)())NoopDDA,    /* void (* ChipGetMode)() */
  S3SAVFbInit,             /* void (* ChipFbInit)() */
  (void (*)())NoopDDA,   /* void (* ChipSetRead)() */
  (void (*)())NoopDDA,   /* void (* ChipSetWrite)() */
  (void (*)())NoopDDA,   /* void (* ChipSetReadWrite)() */
  0x10000,              /* int ChipMapSize */
  0x10000,              /* int ChipSegmentSize */
  16,                   /* int ChipSegmentShift */
  0xFFFF,               /* int ChipSegmentMask */
  0x00000, 0x10000,      /* int ChipReadBottom, int ChipReadTop */
  0x00000, 0x10000,     /* int ChipWriteBottom, int ChipWriteTop */
  FALSE,                /* Bool ChipUse2Banks */
  VGA_DIVIDE_VERT,      /* int ChipInterlaceType */
  {0,},                 /* OFlagSet ChipOptionFlags */
  8,                    /* int ChipRounding */
  TRUE,                 /* Bool ChipUseLinearAddressing */
  0,                    /* int ChipLinearBase */
  0,                    /* int ChipLinearSize */
  /*
   * This is TRUE if the driver has support for the given depth for
   * the detected configuration. It must be set in the Probe function.
   * It most cases it should be FALSE.
   */
  TRUE,        /* 16bpp */
  FALSE,       /* 24bpp */
  TRUE,        /* 32bpp */
  NULL,                 /* DisplayModePtr ChipBuiltinModes */
	/*
	 * This is a factor that can be used to scale the raw clocks
	 * to pixel clocks.	 This is rarely used, and in most cases, set
	 * it to 1.
	 */
	1
};

/* this code is largely based on the S3V code. In order to minimize the changes
   to the actual code we stick with most of the naming. That's what the following
   define does... 
 */
#define S3V S3_SAVAGE

/* entries must be in sequence with chipset numbers !! */
SymTabRec s3savChipTable[] = {
   { S3_UNKNOWN,      "unknown"},
   { S3_SAVAGE3D,     "Savage3D"}, 
   { S3_SAVAGE3D_MV,  "Savage3D/MV"},
   { S3_SAVAGE4,      "Savage4"},
   { S3_SAVAGE2000,   "Savage2000"},
   { S3_SAVAGE_MX_MV, "Savage/MX-MV"},
   { S3_SAVAGE_MX,    "Savage/MX"},
   { S3_SAVAGE_IX_MV, "Savage/IX-MV"},
   { S3_SAVAGE_IX,    "Savage/IX"},
   { -1,              ""},
   };

/* Declare the private structure which stores all internal info */

S3VPRIV s3vPriv;


/* And other global vars to hold vga base regs and MMIO base mem pointer */

int vgaCRIndex, vgaCRReg;
pointer s3savMmioMem = NULL;   /* MMIO base address */
extern vgaHWCursorRec vgaHWCursor;

#ifdef USEBIOS

/* Information about the current BIOS modes. */
#define iabs(a)	((int)(a)>0?(a):(-(a)))
S3VMODETABLE * s3vModeTable = NULL;
unsigned short s3vModeCount = 0;

#endif


/* This function returns the string name for a supported chipset */

static char *
S3SAVIdent(n)
int n;
{
   char *chipset = "s3_savage";

   if(n == 0) return(chipset);
   else return NULL;

}


/*
 * I'd rather have these wait macros be inline, but S3 has made it 
 * darned near impossible.  The bit fields are in a different place in
 * all three chips, the status register has a different address in the
 * three chips, and even the idle vs busy state flipped in the Sav2K
 */

/* Wait until "v" queue entries are free */

static int
WaitQueue3D( int v )
{
    int loop = 0;
    int slots = MAXFIFO - v;

    mem_barrier();
    while( ((STATUS_WORD0 & 0x0000ffff) > slots) && (loop++ < MAXLOOP))
	;
    return loop >= MAXLOOP;
}

static int
WaitQueue4( int v )
{
    int loop = 0;
    int slots = MAXFIFO - v;

    if( !s3vPriv.NoPCIRetry )
	return;
    mem_barrier();
    while( ((ALT_STATUS_WORD0 & 0x0001ffff) > slots) && (loop++ < MAXLOOP))
	;
    return loop >= MAXLOOP;
}

static int
WaitQueue2K( int v )
{
    int loop = 0;
    int slots = MAXFIFO - v;

    if( !s3vPriv.NoPCIRetry )
	return;
    mem_barrier();
    while( ((ALT_STATUS_WORD0 & 0x000fffff) > slots) && (loop++ < MAXLOOP))
	;
    return loop >= MAXLOOP;
}

/* Wait until GP is idle and queue is empty */

static int
WaitIdleEmpty3D()
{
    int loop = 0;
    mem_barrier();
    while( ((STATUS_WORD0 & 0x0008ffff) != 0x80000) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitIdleEmpty4()
{
    int loop = 0;
    mem_barrier();
    while( ((ALT_STATUS_WORD0 & 0x0081ffff) != 0x00800000) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitIdleEmpty2K()
{
    int loop = 0;
    mem_barrier();
    while( ((ALT_STATUS_WORD0 & 0x009fffff) != 0) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

/* Wait until GP is idle */

static int
WaitIdle3D()
{
    int loop = 0;
    mem_barrier();
    while( (!(STATUS_WORD0 & 0x00080000)) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitIdle4()
{
    int loop = 0;
    mem_barrier();
    while( (!(ALT_STATUS_WORD0 & 0x00800000)) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitIdle2K()
{
    int loop = 0;
    mem_barrier();
    while( (ALT_STATUS_WORD0 & 0x00900000) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

/* Wait until Command FIFO is empty */


static int
WaitCommandEmpty3D() {
    int loop = 0;
    mem_barrier();
    while( (STATUS_WORD0 & 0x0000ffff) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitCommandEmpty4() {
    int loop = 0;
    mem_barrier();
    while( (ALT_STATUS_WORD0 & 0x0001ffff) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}

static int
WaitCommandEmpty2K() {
    int loop = 0;
    mem_barrier();
    while( (ALT_STATUS_WORD0 & 0x001fffff) && (loop++ < MAXLOOP) )
	;
    return loop >= MAXLOOP;
}


/* The EnterLeave function which en/dis access to IO ports and ext. regs */

static void 
S3SAVEnterLeave(enter)
Bool enter;
{
   static int enterCalled = FALSE;
   unsigned char tmp;

#ifdef XFreeXDGA
	if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter)
		return;
#endif

   if (enter){
      xf86ClearIOPortList(vga256InfoRec.scrnIndex);
      xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
      xf86EnableIOPorts(vga256InfoRec.scrnIndex);

      /* Init the vgaIOBase reg index, depends on mono/color operation */
      vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
      vgaCRIndex = vgaIOBase + 4;
      vgaCRReg = vgaIOBase + 5;

      /* Unprotect CRTC[0-7]  */
      outb(vgaCRIndex, 0x11);      /* for register CR11 */
      tmp = inb(vgaCRReg);         /* enable CR0-7 and disable interrupts */
      outb(vgaCRReg, tmp & 0x7f);

      /* And unlock extended regs */
      outb(vgaCRIndex, 0x38);      /* for register CR38, (REG_LOCK1) */
      outb(vgaCRReg, 0x48);        /* unlock S3 register set for read/write */
      outb(vgaCRIndex, 0x39);    
      outb(vgaCRReg, 0xa5);
      outb(vgaCRIndex, 0x40);
      tmp = inb(vgaCRReg);
      outb(vgaCRReg, tmp & ~0x01);   /* avoid lockups when reading I/O port 0x92e8 */
      enterCalled = TRUE;
      }

   else {
      if (s3savMmioMem) {
	 unsigned char cr3a, cr53, cr66;
	 outb(vgaCRIndex, 0x53);
	 cr53 = inb(vgaCRReg);
	 outb(vgaCRReg, cr53 | 0x08);  /* Enable NEWMMIO temporarily */

	 outb(vgaCRIndex, 0x66);
	 cr66 = inb(vgaCRReg);
	 outb(vgaCRReg, cr66 | 0x80);
	 outb(vgaCRIndex, 0x3a);
	 cr3a = inb(vgaCRReg);
	 outb(vgaCRReg, cr3a | 0x80);

         WaitIdle();           /* DOn't know if these map properly ? */
         WaitCommandEmpty();   /* We should probably do a DMAEmpty() as well */

	 outb(vgaCRIndex, 0x53);
	 outb(vgaCRReg, cr53);   /* Restore CR53 to original for MMIO */

	 outb(vgaCRIndex, 0x66);
	 outb(vgaCRReg, cr66);
	 outb(vgaCRIndex, 0x3a);             
	 outb(vgaCRReg, cr3a);
         }
      if (enterCalled){

         /* Protect CR[0-7] */
         outb(vgaCRIndex, 0x11);      /* for register CR11 */
         tmp = inb(vgaCRReg);         /* disable CR0-7 */
         outb(vgaCRReg, (tmp & 0x7f) | 0x80);
      
         /* Relock extended regs-> To DO */
      
         xf86DisableIOPorts(vga256InfoRec.scrnIndex);
         enterCalled = FALSE;
         }
     }
}


/* 
 * This function is used to restore a video mode. It writes out all  
 * of the standard VGA and extended S3 registers needed to setup a 
 * video mode.
 *
 * Note that our life is made more difficult because of the STREAMS
 * processor which must be used for 24bpp. We need to disable STREAMS
 * before we switch video modes, or we risk locking up the machine. 
 * We also have to follow a certain order when reenabling it. 
 */

static void
S3SAVRestore (restore)
vgaS3VPtr restore;
{
unsigned char tmp, cr3a, cr53, cr66, cr67;
unsigned int width;

   vgaProtect(TRUE);

#ifdef USEBIOS

    /*
     * If S3VInit figured out a VESA mode number for this timing, the
     * just use the S3 BIOS to do the switching, with a few additional
     * tweaks.
     */

    if( restore->mode > 0x13 )
    {
	unsigned short cr6d;
	unsigned short cr79;

	/* Set up the mode.  Don't clear video RAM. */

	S3SAVSetVESAMode( restore->mode | 0x8000, restore->refresh );

	/* Restore the DAC.  Ordinarily, vgaHWRestore does this, */
	/* but since we use the BIOS, we don't call vgaHWRestore. */

	outb(0x03c6, 0xff);
	if( vgaBitsPerPixel == 8 )
	{
	    unsigned short i;
	    outb( 0x3c8, 0 );
	    for( i = 0; i < 3*256; i++ )
	    {
	        outb(0x03c9, restore->std.DAC[i]);
	    }
	}

	/* Enable linear addressing. */
	outw(vgaCRIndex, 0x1358);

	/* Disable old MMIO. */
	outb(vgaCRIndex, 0x53);
	outb(vgaCRReg, inb(vgaCRReg) & ~0x10);

	/* We may need TV/panel fixups here.  See s3bios.c line 2904. */

	/* Set FIFO fetch delay. */
	outb(vgaCRIndex, 0x85);
	outb(vgaCRReg, (inb(vgaCRReg) & 0xf8) | 0x03);

	/* Unlock the extended registers. */

	outw(vgaCRIndex, 0x4838);
	outw(vgaCRIndex, 0xA039);
	outw(0x3c4, 0x0608);

	/* Patch CR79.  These values are magical. */

	outb(vgaCRIndex, 0x6d);
	cr6d = inb(vgaCRReg);

	cr79 = 0x04;

	if( vga256InfoRec.displayWidth >= 1024 )
	{
	    if( vgaBitsPerPixel == 32 )
	    {
		if( restore->refresh >= 130 )
		    cr79 = 0x03;
		else if( vga256InfoRec.displayWidth >= 1280 )
		    cr79 = 0x02;
		else if(
		    (vga256InfoRec.displayWidth == 1024) &&
		    (restore->refresh >= 75)
		)
		{
		    if( cr6d && LCD_ACTIVE )
			cr79 = 0x05;
		    else
			cr79 = 0x08;
		}
	    }
	    else if( vgaBitsPerPixel == 16)
	    {
/* The windows driver uses 0x13 for 16-bit 130Hz, but I see terrible
 * screen artifacts with that value.  Let's keep it low for now.
 *		if( restore->refresh >= 130 )
 *		    cr79 = 0x13;
 *		else
 */
		if( vga256InfoRec.displayWidth == 1024 )
		{
		    if( cr6d && LCD_ACTIVE )
			cr79 = 0x08;
		    else
			cr79 = 0x0e;
		}
	    }
	}

        if( s3vPriv.chip != S3_SAVAGE2000)
	    outw(vgaCRIndex, (cr79 << 8) | 0x79);

	/* Make sure 16-bit memory access is enabled. */

	outb(vgaCRIndex, 0x31);
	outb(vgaCRReg, 0x0c);

	/* Enable the graphics engine. */

	outw(vgaCRIndex, 0x0140);
	if(!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
	    S3SAVInitialize2DEngine();

	/* Handle the pitch. */

        outb(vgaCRIndex, 0x50);
        outb(vgaCRReg, inb(vgaCRReg) | 0xC1);

	width = (vga256InfoRec.displayWidth * (vgaBitsPerPixel / 8)) >> 3;
	outw(vgaCRIndex, ((width & 0xff) << 8) | 0x13 );
	outw(vgaCRIndex, ((width & 0x300) << 4) | 0x51 );

	/* Some non-S3 BIOSes enable block write even on non-SGRAM devices. */

        if( s3vPriv.chip == S3_SAVAGE2000 )
	{
	    outb(vgaCRIndex, 0x73);
	    outb(vgaCRReg, inb(vgaCRReg) & 0xdf );
	}
	else
	{
	    outb(vgaCRIndex, 0x68);
	    if( !(inb(vgaCRReg) & 0x80) )
	    {
		/* Not SGRAM; disable block write. */
		outb(vgaCRIndex, 0x88);
		outb(vgaCRReg, inb(vgaCRReg) | 0x10);
	    }
	}

	outw(vgaCRIndex, 0x0140);
	S3SAVSetGBD();

    } else {

#endif

   /* Are we going to reenable STREAMS in this new mode? */
   s3vPriv.STREAMSRunning = restore->CR67 & 0x0c; 

   /* First reset GE to make sure nothing is going on */
   outb(vgaCRIndex, 0x66);
   if(inb(vgaCRReg) & 0x01) S3SAVGEReset(0,__LINE__,__FILE__);

   /* As per databook, always disable STREAMS before changing modes */
   outb(vgaCRIndex, 0x67);
   cr67 = inb(vgaCRReg);
   if ((cr67 & 0x04) == 0x04) {
      S3SAVDisableSTREAMS();     /* If STREAMS was running, disable it */
      }

   /* Restore S3 extended regs */
   outb(vgaCRIndex, 0x66);             
   outb(vgaCRReg, restore->CR66);
   outb(vgaCRIndex, 0x3a);             
   outb(vgaCRReg, restore->CR3A);
   outb(vgaCRIndex, 0x31);    
   outb(vgaCRReg, restore->CR31);
   outb(vgaCRIndex, 0x58);             
   outb(vgaCRReg, restore->CR58);

   outb(0x3c4, 0x08);
   outb(0x3c5, 0x06); 
   outb(0x3c4, 0x12);
   outb(0x3c5, restore->SR12);
   outb(0x3c4, 0x13);
   outb(0x3c5, restore->SR13);
   outb(0x3c4, 0x29);
   outb(0x3c5, restore->SR29);
   outb(0x3c4, 0x15);
   outb(0x3c5, restore->SR15); 

   /* Restore the standard VGA registers */
   vgaHWRestore((vgaHWPtr)restore);

   /* Extended mode timings registers */  
   outb(vgaCRIndex, 0x53);             
   outb(vgaCRReg, restore->CR53); 
   outb(vgaCRIndex, 0x5d);     
   outb(vgaCRReg, restore->CR5D);
   outb(vgaCRIndex, 0x5e);             
   outb(vgaCRReg, restore->CR5E);
   outb(vgaCRIndex, 0x3b);             
   outb(vgaCRReg, restore->CR3B);
   outb(vgaCRIndex, 0x3c);             
   outb(vgaCRReg, restore->CR3C);
   outb(vgaCRIndex, 0x43);             
   outb(vgaCRReg, restore->CR43);
   outb(vgaCRIndex, 0x65);             
   outb(vgaCRReg, restore->CR65);


   /* Restore the desired video mode with CR67 */
        
   outb(vgaCRIndex, 0x67);             
   cr67 = inb(vgaCRReg) & 0xf; /* Possible hardware bug on VX? */
   outb(vgaCRReg, 0x50 | cr67); 
   usleep(10000);
   outb(vgaCRIndex, 0x67);             
   outb(vgaCRReg, restore->CR67 & ~0x0c); /* Don't enable STREAMS yet */

   /* Other mode timing and extended regs */
   outb(vgaCRIndex, 0x34);             
   outb(vgaCRReg, restore->CR34);
   outb(vgaCRIndex, 0x40);             
   outb(vgaCRReg, restore->CR40);
   outb(vgaCRIndex, 0x42);             
   outb(vgaCRReg, restore->CR42);
   outb(vgaCRIndex, 0x45);
   outb(vgaCRReg, restore->CR45);
   outb(vgaCRIndex, 0x50);
   outb(vgaCRReg, restore->CR50);
   outb(vgaCRIndex, 0x51);
   outb(vgaCRReg, restore->CR51);
   
   /* Memory timings */
   outb(vgaCRIndex, 0x36);             
   outb(vgaCRReg, restore->CR36);
   outb(vgaCRIndex, 0x68);             
   outb(vgaCRReg, restore->CR68);
   outb(vgaCRIndex, 0x69);
   outb(vgaCRReg, restore->CR69);
   outb(vgaCRIndex, 0x6F);
   outb(vgaCRReg, restore->CR6F);

   outb(vgaCRIndex, 0x33);
   outb(vgaCRReg, restore->CR33);
   outb(vgaCRIndex, 0x86);
   outb(vgaCRReg, restore->CR86);
   outb(vgaCRIndex, 0x88);
   outb(vgaCRReg, restore->CR88);
   outb(vgaCRIndex, 0x90);
   outb(vgaCRReg, restore->CR90);
   outb(vgaCRIndex, 0x91);
   outb(vgaCRReg, restore->CR91);
   outb(vgaCRIndex, 0xB0); /* Savage4 config3 */
   outb(vgaCRReg, restore->CRB0);

   /* Unlock extended sequencer regs */
   outb(0x3c4, 0x08);
   outb(0x3c5, 0x06); 


   /* Restore extended sequencer regs for MCLK. SR10 == 255 indicates that 
    * we should leave the default SR10 and SR11 values there.
    */

   if (restore->SR10 != 255) {   
       outb(0x3c4, 0x10);
       outb(0x3c5, restore->SR10);
       outb(0x3c4, 0x11);
       outb(0x3c5, restore->SR11);
       }

   /* Restore extended sequencer regs for DCLK */

   outb(0x3c4, 0x12);
   outb(0x3c5, restore->SR12);
   outb(0x3c4, 0x13);
   outb(0x3c5, restore->SR13);
   outb(0x3c4, 0x29);
   outb(0x3c5, restore->SR29);

   outb(0x3c4, 0x18);
   outb(0x3c5, restore->SR18); 

   /* Load new m,n PLL values for DCLK & MCLK */
   outb(0x3c4, 0x15);
   tmp = inb(0x3c5) & ~0x21;

   outb(0x3c5, tmp | 0x03);
   outb(0x3c5, tmp | 0x23);
   outb(0x3c5, tmp | 0x03);
   outb(0x3c5, restore->SR15);

   outb(0x3c4, 0x08);
   outb(0x3c5, restore->SR8); 


   /* Now write out CR67 in full, possibly starting STREAMS */
   
   VerticalRetraceWait();
   outb(vgaCRIndex, 0x67);    
   outb(vgaCRReg, 0x50);   /* For possible bug on VX?! */          
   usleep(10000);
   outb(vgaCRIndex, 0x67);
   outb(vgaCRReg, restore->CR67); 


   /* And finally, we init the STREAMS processor if we have CR67 indicate 24bpp
    * We also restore FIFO and TIMEOUT memory controller registers.
    */

   outb(vgaCRIndex, 0x53);
   cr53 = inb(vgaCRReg);
   outb(vgaCRReg, cr53 | 0x08);  /* Enable NEWMMIO temporarily */

   outb(vgaCRIndex, 0x66);
   cr66 = inb(vgaCRReg);
   outb(vgaCRReg, cr66 | 0x80);
   outb(vgaCRIndex, 0x3a);
   cr3a = inb(vgaCRReg);
   outb(vgaCRReg, cr3a | 0x80);

   if (s3vPriv.NeedSTREAMS) {
      if(s3vPriv.STREAMSRunning) S3SAVRestoreSTREAMS(restore->STREAMS);
      }

   /* Now, before we continue, check if this mode has the graphic engine ON 
    * If yes, then we reset it. 
    * This fixes some problems with corruption at 24bpp with STREAMS
    * Also restore the MIU registers. 
    */


   if(restore->CR66 & 0x01) S3SAVGEReset(0,__LINE__,__FILE__);

   VerticalRetraceWait();
   ((mmtr)s3savMmioMem)->memport_regs.regs.fifo_control = restore->MMPR0;
   WaitIdle();                  /* Don't ask... */
   ((mmtr)s3savMmioMem)->memport_regs.regs.miu_control = restore->MMPR1;
   WaitIdle();                  
   ((mmtr)s3savMmioMem)->memport_regs.regs.streams_timeout = restore->MMPR2;
   WaitIdle();
   ((mmtr)s3savMmioMem)->memport_regs.regs.misc_timeout = restore->MMPR3;

   /* If we're going into graphics mode and acceleration was enabled, */
   /* go set up the BCI buffer and the global bitmap descriptor. */

   if( (restore->CR31 & 0x0a) &&
       (!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
   )
   {
      outb(vgaCRIndex, 0x50);
      outb(vgaCRReg, inb(vgaCRReg) | 0xC1);
      S3SAVInitialize2DEngine();
   }

   outb(vgaCRIndex, 0x53);
   outb(vgaCRReg, cr53);   /* Restore CR53 to original for MMIO */

   outb(vgaCRIndex, 0x66);             
   outb(vgaCRReg, cr66);
   outb(vgaCRIndex, 0x3a);             
   outb(vgaCRReg, cr3a);

   if( restore->CR31 & 0x0a ) 
     S3SAVSetGBD();

#ifdef USEBIOS
}
#endif

   if (xf86Verbose > 1) {
      ErrorF("\n\nViRGE driver: done restoring mode, dumping CR registers:\n\n");
      S3SAVPrintRegs();
   }

   vgaProtect(FALSE);

}

/* 
 * This function performs the inverse of the restore function: It saves all
 * the standard and extended registers that we are going to modify to set
 * up a video mode. Again, we also save the STREAMS context if it is needed.
 */

static void *
S3SAVSave (save)
vgaS3VPtr save;
{
int i;
unsigned char cr3a, cr53, cr66;

   /*
    * This function will handle creating the data structure and filling
    * in the generic VGA portion.
    */

   outb(vgaCRIndex, 0x66);
   cr66 = inb(vgaCRReg);
   outb(vgaCRReg, cr66 | 0x80);
   outb(vgaCRIndex, 0x3a);
   cr3a = inb(vgaCRReg);
   outb(vgaCRReg, cr3a | 0x80);

   outb(vgaCRIndex, 0x66);
   cr66 = inb(vgaCRReg);
   outb(vgaCRReg, cr66 | 0x80);
   outb(vgaCRIndex, 0x3a);
   cr3a = inb(vgaCRReg);
   outb(vgaCRReg, cr3a | 0x80);

   save = (vgaS3VPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaS3VRec));

#ifdef USEBIOS

   /*
    * If we are able to use the BIOS, and the mode number has already
    * been initialized to a VESA mode number, then we don't need to
    * save anything else.  We have what we need to know.
    */

   if( save->mode > 0x13 )
      return;

#endif

   outb(vgaCRIndex, 0x66);
   outb(vgaCRReg, cr66);
   outb(vgaCRIndex, 0x3a);             
   outb(vgaCRReg, cr3a);

   outb(vgaCRIndex, 0x66);
   outb(vgaCRReg, cr66);
   outb(vgaCRIndex, 0x3a);             
   outb(vgaCRReg, cr3a);

   /* First unlock extended sequencer regs */
   outb(0x3c4, 0x08);
   save->SR8 = inb(0x3c5);
   outb(0x3c5, 0x06); 

   /* Now we save all the s3 extended regs we need */
   outb(vgaCRIndex, 0x31);             
   save->CR31 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x34);             
   save->CR34 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x36);             
   save->CR36 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x3a);             
   save->CR3A = inb(vgaCRReg);
   outb(vgaCRIndex, 0x40);
   save->CR40 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x42);
   save->CR42 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x45);
   save->CR45 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x50);
   save->CR50 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x51);
   save->CR51 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x53);             
   save->CR53 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x58);             
   save->CR58 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x66);             
   save->CR66 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x67);             
   save->CR67 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x68);             
   save->CR68 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x69);
   save->CR69 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x6F);
   save->CR6F = inb(vgaCRReg);

   outb(vgaCRIndex, 0x33);             
   save->CR33 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x86);
   save->CR86 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x88);
   save->CR88 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x90);
   save->CR90 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x91);
   save->CR91 = inb(vgaCRReg);
   outb(vgaCRIndex, 0xB0); /* Savage4 config3 */
   save->CRB0 = inb(vgaCRReg) | 0x80; /* map all address together */

   /* Extended mode timings regs */

   outb(vgaCRIndex, 0x3b);             
   save->CR3B = inb(vgaCRReg);
   outb(vgaCRIndex, 0x3c);             
   save->CR3C = inb(vgaCRReg);
   outb(vgaCRIndex, 0x43);             
   save->CR43 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x5d);             
   save->CR5D = inb(vgaCRReg);
   outb(vgaCRIndex, 0x5e);
   save->CR5E = inb(vgaCRReg);  
   outb(vgaCRIndex, 0x65);             
   save->CR65 = inb(vgaCRReg);


   /* Save sequencer extended regs for DCLK PLL programming */

   outb(0x3c4, 0x10);
   save->SR10 = inb(0x3c5);
   outb(0x3c4, 0x11);
   save->SR11 = inb(0x3c5);

   outb(0x3c4, 0x12);
   save->SR12 = inb(0x3c5);
   outb(0x3c4, 0x13);
   save->SR13 = inb(0x3c5);
   outb(0x3c4, 0x29);
   save->SR29 = inb(0x3c5);

   outb(0x3c4, 0x15);
   save->SR15 = inb(0x3c5);
   outb(0x3c4, 0x18);
   save->SR18 = inb(0x3c5);


   /* And if streams is to be used, save that as well */

      outb(vgaCRIndex, 0x53);
   cr53 = inb(vgaCRReg);
   outb(vgaCRReg, cr53 | 0x08);  /* Enable NEWMMIO to save MIU context */

   outb(vgaCRIndex, 0x66);
   cr66 = inb(vgaCRReg);
   outb(vgaCRReg, cr66 | 0x80);
   outb(vgaCRIndex, 0x3a);
   cr3a = inb(vgaCRReg);
   outb(vgaCRReg, cr3a | 0x80);

   if(s3vPriv.NeedSTREAMS) {
      S3SAVSaveSTREAMS(save->STREAMS);
      }

   /* Now save Memory Interface Unit registers, enable MMIO for this */
   save->MMPR0 = ((mmtr)s3savMmioMem)->memport_regs.regs.fifo_control;
   save->MMPR1 = ((mmtr)s3savMmioMem)->memport_regs.regs.miu_control;
   save->MMPR2 = ((mmtr)s3savMmioMem)->memport_regs.regs.streams_timeout;
   save->MMPR3 = ((mmtr)s3savMmioMem)->memport_regs.regs.misc_timeout;

   if (xf86Verbose > 1) {
      /* Debug */
      ErrorF("MMPR regs: %08x %08x %08x %08x\n",
         ((mmtr)s3savMmioMem)->memport_regs.regs.fifo_control,
         ((mmtr)s3savMmioMem)->memport_regs.regs.miu_control,
         ((mmtr)s3savMmioMem)->memport_regs.regs.streams_timeout,
         ((mmtr)s3savMmioMem)->memport_regs.regs.misc_timeout );

      ErrorF("\n\nSavage driver: saved current video mode. Register dump:\n\n");
   }
   outb(vgaCRIndex, 0x53);
   outb(vgaCRReg, cr53);   /* Restore CR53 to original for MMIO */

   outb(vgaCRIndex, 0x3a);
   outb(vgaCRReg, cr3a);
   outb(vgaCRIndex, 0x66);
   outb(vgaCRReg, cr66);

   if (xf86Verbose > 1) S3SAVPrintRegs();

   return ((void *) save);
}



static unsigned char *find_bios_string(int BIOSbase, char *match1, char *match2)
{
#define BIOS_BSIZE 1024
#define BIOS_BASE  0xc0000

   static unsigned char bios[BIOS_BSIZE];
   static int init=0;
   int i,j,l1,l2;

   if (!init) {
      init = 1;
      if (xf86ReadBIOS(BIOSbase, 0, bios, BIOS_BSIZE) != BIOS_BSIZE)
	 return NULL;
      if ((bios[0] != 0x55) || (bios[1] != 0xaa))
	 return NULL;
   }
   if (match1 == NULL)
      return NULL;

   l1 = strlen(match1);
   if (match2 != NULL) 
      l2 = strlen(match2);
   else	/* for compiler-warnings */
      l2 = 0;

   for (i=0; i<BIOS_BSIZE-l1; i++)
      if (bios[i] == match1[0] && !memcmp(&bios[i],match1,l1))
	 if (match2 == NULL) 
	    return &bios[i+l1];
	 else
	    for(j=i+l1; (j<BIOS_BSIZE-l2) && bios[j]; j++) 
	       if (bios[j] == match2[0] && !memcmp(&bios[j],match2,l2))
		  return &bios[j+l2];
   return NULL;
}


/* 
 * This is the main probe function for the virge chipsets.
 * Right now, I have taken a shortcut and get most of the info from
 * PCI probing.
 */

static Bool
S3SAVProbe()
{
S3PCIInformation *pciInfo = NULL;
unsigned char config1, m, n, n1, n2, cr66, sr8;
int mclk;
DisplayModePtr pMode, pEnd;

   if (vga256InfoRec.chipset) {
      if (StrCaseCmp(vga256InfoRec.chipset,S3SAVIdent(0)))
      return(FALSE);
   } 

   /* Start with PCI probing, this should get us quite far already */

   pciInfo = S3SAVGetPCIInfo();
   if (!pciInfo)
      return FALSE;

   if (pciInfo && pciInfo->MemBase && !vga256InfoRec.MemBase) {
      if (pciInfo->ChipType >= S3_SAVAGE4) {
	 s3vPriv.MmioBase = pciInfo->MemBase + S3_NEWMMIO_REGBASE_S4;
	 s3vPriv.FrameBufferBase = pciInfo->MemBase1;
      }
      else {
	 s3vPriv.MmioBase = pciInfo->MemBase + S3_NEWMMIO_REGBASE_S3;
	 s3vPriv.FrameBufferBase = pciInfo->MemBase;
      }
      vga256InfoRec.MemBase = s3vPriv.FrameBufferBase;
   }
   if (pciInfo)
      if(pciInfo->ChipType != S3_SAVAGE3D && 
	 pciInfo->ChipType != S3_SAVAGE3D_MV &&
	 pciInfo->ChipType != S3_SAVAGE4 &&
	 pciInfo->ChipType != S3_SAVAGE2000){
          if (xf86Verbose > 1)
             ErrorF("%s %s: Unsupported (non-Savage) S3 chipset detected!\n", 
                XCONFIG_PROBED, vga256InfoRec.name);
          return FALSE;
          }
      else {
         s3vPriv.chip = pciInfo->ChipType;
         ErrorF("%s %s: Detected S3 %s\n",XCONFIG_PROBED,
            vga256InfoRec.name, xf86TokenToString(s3savChipTable, s3vPriv.chip));
         ErrorF("%s %s: using driver for chipset \"%s\"\n",XCONFIG_PROBED, 
            vga256InfoRec.name, S3SAVIdent(0));
	 }

   vga256InfoRec.chipset = S3SAVIdent(0);

#ifdef __alpha__
   if (xf86bpp > 16)
     FatalError("%s %s: %d bpp not yet supported for Alpha/AXP\n",
		XCONFIG_GIVEN, vga256InfoRec.name, xf86bpp);
#endif

   /* Add/enable IO ports to list: call EnterLeave */
   S3SAVEnterLeave(ENTER);

   /* Unlock sys regs */
   outb(vgaCRIndex, 0x38);
   outb(vgaCRReg, 0x48);
 
   /* Next go on to detect amount of installed ram */

   outb(vgaCRIndex, 0x36);              /* for register CR36 (CONFG_REG1), */
   config1 = inb(vgaCRReg);              /* get amount of vram installed */

   outb(vgaCRIndex, 0x2e);
   s3vPriv.ChipId = inb(vgaCRReg);         /* get chip id */
   outb(vgaCRIndex, 0x2d);
   s3vPriv.ChipId |= (inb(vgaCRReg) << 8);

   /* And compute the amount of video memory and offscreen memory */
   s3vPriv.MemOffScreen = 0;
   if (!vga256InfoRec.videoRam) {
      if( (s3vPriv.ChipId == PCI_SAVAGE4) ||
      	  (s3vPriv.ChipId == PCI_SAVAGE2000)
      ) {
	 switch (config1 >> 5) {
	 case 0:
	    vga256InfoRec.videoRam = 2;
	    break;
	 case 1:
	    vga256InfoRec.videoRam = 4;
	    break;
	 case 2:
	    vga256InfoRec.videoRam = 8;
	    break;
	 case 3:
	    vga256InfoRec.videoRam = 12;
	    break;
	 case 4:
	    vga256InfoRec.videoRam = 16;
	    break;
	 case 5:
	    vga256InfoRec.videoRam = 32;
	    break;
	 case 6:
	    vga256InfoRec.videoRam = 64;
	    break;
	 default:
	    vga256InfoRec.videoRam = 2;
         }  
      }
      else {
	 switch((config1 & 0xC0) >> 6) {
	 case 0:
	    vga256InfoRec.videoRam = 8;
	    break;
	 case 1:
	 case 2:
	    vga256InfoRec.videoRam = 4;
	    break;
	 case 3:
	 default:
	    vga256InfoRec.videoRam = 2;
	 }
      }

      vga256InfoRec.videoRam *= 1024;

      if (xf86Verbose) {
         if (s3vPriv.MemOffScreen)
            ErrorF("%s %s: videoram:  %dk (plus %dk off-screen)\n",
                   XCONFIG_PROBED, vga256InfoRec.name, vga256InfoRec.videoRam,
                   s3vPriv.MemOffScreen);
         else
            ErrorF("%s %s: videoram:  %dk\n",
                   XCONFIG_PROBED, vga256InfoRec.name, vga256InfoRec.videoRam);
      }
   } else {
      if (xf86Verbose) {
         ErrorF("%s %s: videoram:  %dk\n",
              XCONFIG_GIVEN, vga256InfoRec.name, vga256InfoRec.videoRam);
      }
   }


   /* reset S3 graphics engine to avoid memory corruption */
   outb(vgaCRIndex, 0x66);
   cr66 = inb(vgaCRReg);
   outb(vgaCRReg, cr66 | 0x02);
   usleep(10000);  /* wait a little bit... */

   outb(vgaCRIndex, 0x66);
   outb(vgaCRReg, cr66 & ~0x02);  /* clear reset flag */
   usleep(10000);  /* wait a little bit... */

   /* ViRGE built-in ramdac speeds */

   if (vga256InfoRec.dacSpeeds[3] <= 0 && vga256InfoRec.dacSpeeds[2] > 0)
      vga256InfoRec.dacSpeeds[3] = vga256InfoRec.dacSpeeds[2];

   if (s3vPriv.chip == S3_SAVAGE3D ||
       s3vPriv.chip == S3_SAVAGE3D_MV ||
       s3vPriv.chip == S3_SAVAGE4 ||
       s3vPriv.chip == S3_SAVAGE2000) {
      if (vga256InfoRec.dacSpeeds[0] <= 0) vga256InfoRec.dacSpeeds[0] = 250000;
      if (vga256InfoRec.dacSpeeds[1] <= 0) vga256InfoRec.dacSpeeds[1] = 250000;
      if (vga256InfoRec.dacSpeeds[2] <= 0) vga256InfoRec.dacSpeeds[2] = 220000;
      if (vga256InfoRec.dacSpeeds[3] <= 0) vga256InfoRec.dacSpeeds[3] = 220000;
   }
   else {
      if (vga256InfoRec.dacSpeeds[0] <= 0) vga256InfoRec.dacSpeeds[0] = 250000;
      if (vga256InfoRec.dacSpeeds[1] <= 0) vga256InfoRec.dacSpeeds[1] = 250000;
      if (vga256InfoRec.dacSpeeds[2] <= 0) vga256InfoRec.dacSpeeds[2] = 220000;
      if (vga256InfoRec.dacSpeeds[3] <= 0) vga256InfoRec.dacSpeeds[3] = 220000;
   }

   /* Set status word positions based on chip type. */

   switch( s3vPriv.chip ) {
      case S3_SAVAGE3D:
      case S3_SAVAGE3D_MV:
         s3vPriv.WaitQueue = WaitQueue3D;
	 s3vPriv.WaitIdle = WaitIdle3D;
	 s3vPriv.WaitIdleEmpty = WaitIdleEmpty3D;
	 s3vPriv.WaitCommandEmpty = WaitCommandEmpty3D;
	 break;

      case S3_SAVAGE4:
         s3vPriv.WaitQueue = WaitQueue4;
	 s3vPriv.WaitIdle = WaitIdle4;
	 s3vPriv.WaitIdleEmpty = WaitIdleEmpty4;
	 s3vPriv.WaitCommandEmpty = WaitCommandEmpty4;
         break;

      case S3_SAVAGE2000:
         s3vPriv.WaitQueue = WaitQueue2K;
	 s3vPriv.WaitIdle = WaitIdle2K;
	 s3vPriv.WaitIdleEmpty = WaitIdleEmpty2K;
	 s3vPriv.WaitCommandEmpty = WaitCommandEmpty2K;
         break;
   }
   
   if (vga256InfoRec.dacSpeedBpp <= 0)
      if (xf86bpp > 24 && vga256InfoRec.dacSpeeds[3] > 0)
	 vga256InfoRec.dacSpeedBpp = vga256InfoRec.dacSpeeds[3];
      else if (xf86bpp >= 24 && vga256InfoRec.dacSpeeds[2] > 0)
	 vga256InfoRec.dacSpeedBpp = vga256InfoRec.dacSpeeds[2];
      else if (xf86bpp > 8 && xf86bpp < 24 && vga256InfoRec.dacSpeeds[1] > 0)
	 vga256InfoRec.dacSpeedBpp = vga256InfoRec.dacSpeeds[1];
      else if (xf86bpp <= 8 && vga256InfoRec.dacSpeeds[0] > 0)
	 vga256InfoRec.dacSpeedBpp = vga256InfoRec.dacSpeeds[0];

   if (xf86Verbose) {
      ErrorF("%s %s: Ramdac speed: %d MHz",
	     OFLG_ISSET(XCONFIG_DACSPEED, &vga256InfoRec.xconfigFlag) ?
	     XCONFIG_GIVEN : XCONFIG_PROBED, vga256InfoRec.name,
	     vga256InfoRec.dacSpeeds[0] / 1000);
      if (vga256InfoRec.dacSpeedBpp != vga256InfoRec.dacSpeeds[0])
	 ErrorF("  (%d MHz for %d bpp)",vga256InfoRec.dacSpeedBpp / 1000, xf86bpp);
      ErrorF("\n");
   }


   /* Now set RAMDAC limits */
   vga256InfoRec.maxClock = vga256InfoRec.dacSpeedBpp;

   /* Detect current MCLK and print it for user */
   outb(0x3c4, 0x08);
   sr8 = inb(0x3c5);
   outb(0x3c5, 0x06); 
   outb(0x3c4, 0x10);
   n = inb(0x3c5);
   outb(0x3c4, 0x11);
   m = inb(0x3c5);
   outb(0x3c4, 0x08);
   outb(0x3c5, sr8);
   m &= 0x7f;
   n1 = n & 0x1f;
   n2 = (n>>5) & 0x03;
   mclk = ((1431818 * (m+2)) / (n1+2) / (1 << n2) + 50) / 100;
   ErrorF("%s %s: Detected current MCLK value of %1.3f MHz\n",XCONFIG_PROBED, 
      vga256InfoRec.name, mclk / 1000.0);


   /* Now check if the user has specified "set_memclk" value in XConfig */
   if (vga256InfoRec.MemClk > 0) {
      if(vga256InfoRec.MemClk <= 100000) {
         ErrorF("%s %s: Using Memory Clock value of %1.3f MHz\n",
		OFLG_ISSET(XCONFIG_DACSPEED, &vga256InfoRec.xconfigFlag) ?
		XCONFIG_GIVEN : XCONFIG_PROBED, vga256InfoRec.name, 
		vga256InfoRec.MemClk/1000.0);
         s3vPriv.MCLK = vga256InfoRec.MemClk;
         }
      else {
         ErrorF("%s %s: Memory Clock value of %1.3f MHz is larger than limit of 100 MHz\n",
              XCONFIG_GIVEN, vga256InfoRec.name, vga256InfoRec.MemClk/1000.0);
         s3vPriv.MCLK = 0;
         }
      }
   else s3vPriv.MCLK = 0;


   /* Set scale factors for mode timings */

   if (vgaBitsPerPixel == 8){
      s3vPriv.HorizScaleFactor = 1;
      }
   else if (vgaBitsPerPixel == 16){
      s3vPriv.HorizScaleFactor = 2;
      }
   else {     
      s3vPriv.HorizScaleFactor = 1;
      }


   /* And map MMIO memory, abort if we cannot */
   s3savMmioMem = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION,
	     (pointer) s3vPriv.MmioBase, S3_NEWMMIO_REGSIZE);
   s3vPriv.MmioMem = s3savMmioMem;
   s3vPriv.BciMem = (char*)s3vPriv.MmioMem + 0x10000;

   if(s3savMmioMem == NULL) 
      FatalError("S3 Savage: Cannot map MMIO registers!\n");
   
   /* Determine if we need to use the STREAMS processor */

#if 0 /* Don't need to do this for Savage3D */
   if (vgaBitsPerPixel >= 24) s3vPriv.NeedSTREAMS = TRUE;
      else s3vPriv.NeedSTREAMS = FALSE;
#endif
   s3vPriv.STREAMSRunning = FALSE;

#ifdef	USEBIOS
   /* Go probe the BIOS for all the modes and refreshes at this depth. */

   /*
    * What I want here is an array of all mode numbers valid for this
    * depth, and the list of valid refresh rates to go with them.  
    *
    * x, y, mode, array[8] of refresh
    */

   if( s3vModeTable )
   {
      xfree( s3vModeTable );
   }
   
   s3vModeCount = S3SAVGetBIOSModeCount( vgaBitsPerPixel );

   if( !s3vModeCount ) {
     FatalError("%s %s: Failed to fetch any BIOS modes.\n",
       XCONFIG_PROBED, vga256InfoRec.name
     );
   }

   s3vModeTable = (S3VMODETABLE*)xcalloc(sizeof(S3VMODETABLE), s3vModeCount);
   S3SAVGetBIOSModeTable( vgaBitsPerPixel, s3vModeTable );

   if( xf86Verbose )
   {
      int i;
      S3VMODETABLE* pmt;

      ErrorF("%s %s: Found %d modes at this depth:\n", 
         XCONFIG_PROBED, vga256InfoRec.name, s3vModeCount);
      
      for( i = 0, pmt = s3vModeTable; i < s3vModeCount; i++, pmt++ )
      {
         int j;
         ErrorF( "[%03x] %d x %d", pmt->VesaMode, pmt->Width, pmt->Height );
	 for( j = 0; j < pmt->RefreshCount; j++ )
	 {
	    ErrorF( ", %dHz", pmt->RefreshRate[j] );
	 }
	 ErrorF( "\n");
      }
   }
#endif

   pEnd = pMode = vga256InfoRec.modes;
   do {
      /* Setup the Mode.Private if required */
      if (!pMode->PrivSize || !pMode->Private) {
	 pMode->PrivSize = S3_MODEPRIV_SIZE;
	 pMode->Private = (INT32 *)xcalloc(sizeof(INT32), S3_MODEPRIV_SIZE);
	 pMode->Private[0] = 0;
      }
      
      /* Set default for invert_vclk */
      if (!(pMode->Private[0] & (1 << S3_INVERT_VCLK))) {
	 pMode->Private[S3_INVERT_VCLK] = 0;
	 pMode->Private[0] |= 1 << S3_INVERT_VCLK;
      }
      
      /* Set default for blank_delay */
      if (!(pMode->Private[0] & (1 << S3_BLANK_DELAY))) {
	 pMode->Private[0] |= (1 << S3_BLANK_DELAY);
	 if (vgaBitsPerPixel == 8)
	       pMode->Private[S3_BLANK_DELAY] = 0x00;
	    else if (vgaBitsPerPixel == 16)
	       pMode->Private[S3_BLANK_DELAY] = 0x02;
	    else
	       pMode->Private[S3_BLANK_DELAY] = 0x04;
      }
      
      /* Set default for early_sc */
      if (!(pMode->Private[0] & (1 << S3_EARLY_SC))) {
	 pMode->Private[0] |= 1 << S3_EARLY_SC;
	 pMode->Private[S3_EARLY_SC] = 0;
      }
      pMode = pMode->next;
   } while (pMode != pEnd);

   /* And finally set various possible option flags */

   vga256InfoRec.bankedMono = FALSE;

#ifdef XFreeXDGA
   vga256InfoRec.directMode = XF86DGADirectPresent;
#endif

   OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions);
   OFLG_SET(OPTION_SLOW_EDODRAM, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_FAST_DRAM, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_FPM_VRAM, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_PCI_BURST_ON, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_FIFO_CONSERV, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_FIFO_MODERATE, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_FIFO_AGGRESSIVE, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_PCI_RETRY, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_NOACCEL, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_SW_CURSOR, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_HW_CURSOR, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_EARLY_RAS_PRECHARGE, &S3V.ChipOptionFlags);
   OFLG_SET(OPTION_LATE_RAS_PRECHARGE, &S3V.ChipOptionFlags);

   s3vPriv.NoPCIRetry = 1;
   S3V.ChipLinearBase = vga256InfoRec.MemBase;
   S3V.ChipLinearSize = vga256InfoRec.videoRam * 1024;

   return TRUE;   
}


/* This validates a given video mode. 
 * Right now, the checks are quite minimal.
 */

static int
S3SAVValidMode(mode, verbose, flag)
DisplayModePtr mode;
Bool verbose;
int flag;
{
   int mem;

/* Check horiz/vert total values */

   if(mode->HTotal*s3vPriv.HorizScaleFactor > 4088) {
      if (verbose)
         ErrorF("%s %s: %s: Horizontal mode timing overflow (%d)\n",
            XCONFIG_PROBED, vga256InfoRec.name,
            vga256InfoRec.chipset, mode->HTotal);
#ifndef	USEBIOS
         return MODE_BAD;
#endif
         }
   if (mode->VTotal > 2047) {
      if(verbose)
          ErrorF("%s %s: %s: Vertical mode timing overflow (%d)\n",
                  XCONFIG_PROBED, vga256InfoRec.name,
                  vga256InfoRec.chipset, mode->VTotal);
          return MODE_BAD;
        }
   if((mode->Flags & V_INTERLACE) && (vgaBitsPerPixel >= 24)){
      if(verbose)
          ErrorF("%s %s: Interlace modes are not supported at %d bpp\n",
                  XCONFIG_PROBED, vga256InfoRec.name,
                  vgaBitsPerPixel);
          return MODE_BAD;
        }
	 
   /* Now make sure we have enough vidram to support mode */
   mem = ((vga256InfoRec.displayWidth > mode->HDisplay) ? 
             vga256InfoRec.displayWidth : mode->HDisplay) 
             * (vga256InfoRec.bitsPerPixel / 8) * 
             vga256InfoRec.virtualY;
   if (mem > (1024 * vga256InfoRec.videoRam - 1024)) {
     ErrorF("%s %s: Mode \"%s\" requires %d of videoram, only %d is available\n",
         XCONFIG_PROBED, vga256InfoRec.name, mode->name, mem, 
         1024 * vga256InfoRec.videoRam - 1024);
     return MODE_BAD;
     }
 
/* Dont check anything else for now. Leave the warning, fix it later. */
   
   return MODE_OK;
}


/* Used to adjust start address in frame buffer. We use the new 
 * CR69 reg for this purpose instead of the older CR31/CR51 combo.
 * If STREAMS is running, we program the STREAMS start addr. registers. 
 */

static void
S3SAVAdjust(x, y)
int x, y;
{
int Base, hwidth;
unsigned char tmp;

   if(s3vPriv.STREAMSRunning == FALSE) {
      Base = ((y * vga256InfoRec.displayWidth + x)
		* (vgaBitsPerPixel / 8)) >> 2;
      if (vgaBitsPerPixel == 24) 
	Base = Base+2 - (Base+2) % 3;

      /* Now program the start address registers */
      outw(vgaCRIndex, (Base & 0x00FF00) | 0x0C);
      outw(vgaCRIndex, ((Base & 0x00FF) << 8) | 0x0D);
      outb(vgaCRIndex, 0x69);
      outb(vgaCRReg, (Base & 0xFF0000) >> 16);   
      }
   else {          /* Change start address for STREAMS case */
      VerticalRetraceWait();
      ((mmtr)s3savMmioMem)->streams_regs.regs.prim_fbaddr0 =
	 ((y * vga256InfoRec.displayWidth + (x & ~7)) * vgaBitsPerPixel / 8);
      }

   if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options))
     S3SAVRepositionCursor(NULL);

#ifdef XFreeXDGA
   if (vga256InfoRec.directMode & XF86DGADirectGraphics) {
      /* Wait until vertical retrace is in progress. */
      VerticalRetraceWait();
   }
#endif

   return;
}

#define BASE_FREQ 14.31818

static int
#if NeedFunctionPrototypes
savageCalcClock(long freq, int min_m, int min_n1, int max_n1, int min_n2, int max_n2, 
		long freq_min, long freq_max,
		unsigned int * mdiv, unsigned int * ndiv, unsigned int * r)
#else
savageCalcClock(freq, min_m, min_n1, max_n1, min_n2, max_n2, 
		freq_min, freq_max, mdiv, ndiv, r)
long freq;
int min_m, min_n1, max_n1, min_n2, max_n2;
long freq_min, freq_max;
unsigned int *mdiv, *ndiv, *r;
#endif
{
   double ffreq, ffreq_min, ffreq_max;
   double div, diff, best_diff;
   unsigned int m;
   unsigned char n1, n2;
   unsigned char best_n1=16+2, best_n2=2, best_m=125+2;

   ffreq     = freq     / 1000.0 / BASE_FREQ;
   ffreq_min = freq_min / 1000.0 / BASE_FREQ;
   ffreq_max = freq_max / 1000.0 / BASE_FREQ;

   if (ffreq < ffreq_min / (1<<max_n2)) {
      ErrorF("invalid frequency %1.3f MHz  [freq >= %1.3f MHz]\n", 
	     ffreq*BASE_FREQ, ffreq_min*BASE_FREQ / (1<<max_n2));
      ffreq = ffreq_min / (1<<max_n2);
   }
   if (ffreq > ffreq_max / (1<<min_n2)) {
      ErrorF("invalid frequency %1.3f MHz  [freq <= %1.3f MHz]\n", 
	     ffreq*BASE_FREQ, ffreq_max*BASE_FREQ / (1<<min_n2));
      ffreq = ffreq_max / (1<<min_n2);
   }

   /* work out suitable timings */

   best_diff = ffreq;
   
   for (n2=min_n2; n2<=max_n2; n2++) {
      for (n1 = min_n1+2; n1 <= max_n1+2; n1++) {
	 m = (int)(ffreq * n1 * (1<<n2) + 0.5) ;
	 if (m < min_m+2 || m > 127+2) 
	    continue;
	 div = (double)(m) / (double)(n1);	 
	 if ((div >= ffreq_min) &&
	     (div <= ffreq_max)) {
	    diff = ffreq - div / (1<<n2);
	    if (diff < 0.0) 
	       diff = -diff;
	    if (diff < best_diff) {
	       best_diff = diff;
	       best_m    = m;
	       best_n1   = n1;
	       best_n2   = n2;
	    }
	 }
      }
   }
   
#if EXTENDED_DEBUG
   ErrorF("Clock parameters for %1.6f MHz: m=%d, n1=%d, n2=%d\n",
	  ((double)(best_m) / (double)(best_n1) / (1 << best_n2)) * BASE_FREQ,
	  best_m-2, best_n1-2, best_n2);
#endif
  
   *ndiv = best_n1 - 2;
   *r = best_n2;
   *mdiv = best_m - 2;
}
  

static Bool
S3SAVInit(mode)
DisplayModePtr mode;
{
unsigned char tmp;
int width,dclk;
int i, j;
unsigned int Refresh;
#ifdef USEBIOS
S3VMODETABLE * pmt;
#endif

   /* First we adjust the horizontal timings if needed */

   if(s3vPriv.HorizScaleFactor != 1)
      if (!mode->CrtcHAdjusted) {
             mode->CrtcHDisplay *= s3vPriv.HorizScaleFactor;
             mode->CrtcHSyncStart *= s3vPriv.HorizScaleFactor;
             mode->CrtcHSyncEnd *= s3vPriv.HorizScaleFactor;
             mode->CrtcHTotal *= s3vPriv.HorizScaleFactor;
             mode->CrtcHSkew *= s3vPriv.HorizScaleFactor;
             mode->CrtcHAdjusted = TRUE;
             }

   if(!vgaHWInit (mode, sizeof(vgaS3VRec)))
      return FALSE;

#ifdef USEBIOS
   /* Scan through our BIOS list to locate the closest valid mode. */

   /* If we ever break 4GHz clocks on video boards, we'll need to
    * change this.
    */

   Refresh = 
      (vga256InfoRec.clock[mode->Clock] * 1000) / 
      (mode->HTotal * mode->VTotal);

   new->mode = 0;

#if EXTENDED_DEBUG
   ErrorF( "Desired refresh rate = %dHz\n", Refresh );
#endif

   for( i = 0, pmt = s3vModeTable; i < s3vModeCount; i++, pmt++ )
   {
      if( (pmt->Width == mode->HDisplay) && (pmt->Height == mode->VDisplay) )
      {
         int j;
	 int jDelta = 99;
	 int jBest = 0;

	 /* We have an acceptable mode.  Find a refresh rate. */

	 new->mode = pmt->VesaMode;
	 for( j = 0; j < pmt->RefreshCount; j++ )
	 {
	    if( pmt->RefreshRate[j] == Refresh )
	    {
	       /* Exact match. */
	       jBest = j;
	       break;
	    }
	    else if( iabs(pmt->RefreshRate[j] - Refresh) < jDelta )
	    {
	       jDelta = iabs(pmt->RefreshRate[j] - Refresh);
	       jBest = j;
	    }
	 }

	 new->refresh = pmt->RefreshRate[jBest];
	 break;
      }
   }

   if( new->mode > 0 ) {
      /* Success: we found a match in the BIOS. */
      if( xf86Verbose ) {
	 ErrorF( "Chose mode %x at %dHz.\n", new->mode, new->refresh );
      }
      return TRUE;
   }

   /* We failed to find a match in the BIOS. */
   /* Fallthrough to the traditional register-crunching.  */

#endif /* USEBIOS */

   /* Now we fill in the rest of the stuff we need for the virge */
   /* Start with MMIO, linear addr. regs */

   outb(vgaCRIndex, 0x3a);
   tmp = inb(vgaCRReg);
   if(!OFLG_ISSET(OPTION_PCI_BURST_ON, &vga256InfoRec.options)) 
      new->CR3A = tmp | 0x95;      /* ENH 256, no PCI burst! */
   else 
      new->CR3A = (tmp & 0x7f) | 0x15; /* ENH 256, PCI burst */

   new->CR53 &= ~0x08;     /* Enables MMIO */
   new->CR31 = 0x8c;     /* Dis. 64k window, en. ENH maps */    

   /* Enables S3D graphic engine and PCI disconnects */
   new->CR66 = 0x89; 

/* Now set linear addr. registers */
/* LAW size: always 8 MB for Savage3D */

   outb(vgaCRIndex, 0x58);
   new->CR58 = inb(vgaCRReg) & 0x80;
   new->CR58 |= 0x13;

/* ** On PCI bus, no need to reprogram the linear window base address */

/* Now do clock PLL programming. Use the s3gendac function to get m,n */
/* Also determine if we need doubling etc. */

   dclk = vga256InfoRec.clock[mode->Clock];
   new->CR67 = 0x00;             /* Defaults */
   new->SR15 = 0x03 | 0x80; 
   new->SR18 = 0x00;
   new->CR43 = 0x00;
   new->CR45 = 0x00;
   new->CR65 = 0x00;

   outb(vgaCRIndex, 0x40);
   new->CR40 = inb(vgaCRReg) & ~0x01;
   
   /* Memory controller registers. Optimize for better graphics engine 
    * performance. These settings are adjusted/overridden below for other bpp/
    * XConfig options.The idea here is to give a longer number of contiguous
    * MCLK's to both refresh and the graphics engine, to diminish the 
    * relative penalty of 3 or 4 mclk's needed to setup memory transfers. 
    */
   new->MMPR0 = 0x010400; /* defaults */
   new->MMPR1 = 0x00;   
   new->MMPR2 = 0x0808;  
   new->MMPR3 = 0x08080810; 


   if (OFLG_ISSET(OPTION_FIFO_AGGRESSIVE, &vga256InfoRec.options) || 
      OFLG_ISSET(OPTION_FIFO_MODERATE, &vga256InfoRec.options) ||
      OFLG_ISSET(OPTION_FIFO_CONSERV, &vga256InfoRec.options)) {
         new->MMPR1 = 0x0200;   /* Low P. stream waits before filling */
         new->MMPR2 = 0x1808;   /* Let the FIFO refill itself */
         new->MMPR3 = 0x08081810; /* And let the GE hold the bus for a while */
      }

   /* And setup here the new value for MCLK. We use the XConfig 
    * option "set_mclk", whose value gets stored in vga256InfoRec.s3MClk.
    * I'm not sure what the maximum "permitted" value should be, probably
    * 100 MHz is more than enough for now.  
    */

   if(s3vPriv.MCLK> 0) {
       commonCalcClock(s3vPriv.MCLK, 1, 1, 31, 0, 3,
	   135000, 270000, &new->SR11, &new->SR10);
       }
   else {
       new->SR10 = 255; /* This is a reserved value, so we use as flag */
       new->SR11 = 255; 
       }

   {
       if (vgaBitsPerPixel == 8) {
          if (dclk <= 110000) new->CR67 = 0x00; /* 8bpp, 135MHz */
          else new->CR67 = 0x10;                /* 8bpp, 220MHz */
          }
       else if ((vgaBitsPerPixel == 16) && (vga256InfoRec.weight.green == 5)) {
          if (dclk <= 110000) new->CR67 = 0x20; /* 15bpp, 135MHz */
          else new->CR67 = 0x30;                /* 15bpp, 220MHz */
          }
       else if (vgaBitsPerPixel == 16) {
          if (dclk <= 110000) new->CR67 = 0x40; /* 16bpp, 135MHz */
          else new->CR67 = 0x50;                /* 16bpp, 220MHz */
          }
       else if ((vgaBitsPerPixel == 24) || (vgaBitsPerPixel == 32)) {
          new->CR67 = 0xd0;                     /* 24bpp, 135MHz */
          }
       {
          unsigned int m, n, r;

	  savageCalcClock(dclk, 1, 1, 127, 0, 4, 180000, 360000, &m, &n, &r);
          new->SR12 = (r << 6) | (n & 0x3F);
          new->SR13 = m & 0xFF;
          new->SR29 = (r & 4) | (m & 0x100) >> 5 | (n & 0x40) >> 2;
       }
   }

   /* Now adjust the value of the FIFO based upon options specified */
   if(OFLG_ISSET(OPTION_FIFO_MODERATE, &vga256InfoRec.options)) {
      if(vgaBitsPerPixel < 24)
         new->MMPR0 -= 0x8000;
      else 
         new->MMPR0 -= 0x4000;
      }
   else if(OFLG_ISSET(OPTION_FIFO_AGGRESSIVE, &vga256InfoRec.options)) {
      if(vgaBitsPerPixel < 24)
         new->MMPR0 -= 0xc000;
      else 
         new->MMPR0 -= 0x6000;
      }

   /* If we have an interlace mode, set the interlace bit. Note that mode
    * vertical timings are already adjusted by the standard VGA code 
    */
   if(mode->Flags & V_INTERLACE) {
        new->CR42 = 0x20; /* Set interlace mode */
        }
   else {
        new->CR42 = 0x00;
        }

   /* Set display fifo */
   new->CR34 = 0x10;  

   /* Now we adjust registers for extended mode timings */
   /* This is taken without change from the accel/s3_virge code */

   i = ((((mode->CrtcHTotal >> 3) - 5) & 0x100) >> 8) |
       ((((mode->CrtcHDisplay >> 3) - 1) & 0x100) >> 7) |
       ((((mode->CrtcHSyncStart >> 3) - 1) & 0x100) >> 6) |
       ((mode->CrtcHSyncStart & 0x800) >> 7);

   if ((mode->CrtcHSyncEnd >> 3) - (mode->CrtcHSyncStart >> 3) > 64)
      i |= 0x08;   /* add another 64 DCLKs to blank pulse width */

   if ((mode->CrtcHSyncEnd >> 3) - (mode->CrtcHSyncStart >> 3) > 32)
      i |= 0x20;   /* add another 32 DCLKs to hsync pulse width */

   j = (  new->std.CRTC[0] + ((i&0x01)<<8)
        + new->std.CRTC[4] + ((i&0x10)<<4) + 1) / 2;

   if (j-(new->std.CRTC[4] + ((i&0x10)<<4)) < 4)
      if (new->std.CRTC[4] + ((i&0x10)<<4) + 4 <= new->std.CRTC[0]+ ((i&0x01)<<8))
         j = new->std.CRTC[4] + ((i&0x10)<<4) + 4;
      else
         j = new->std.CRTC[0]+ ((i&0x01)<<8) + 1;

   new->CR3B = j & 0xFF;
   i |= (j & 0x100) >> 2;
   new->CR3C = (new->std.CRTC[0] + ((i&0x01)<<8))/2;
   new->CR5D = i;

   new->CR5E = (((mode->CrtcVTotal - 2) & 0x400) >> 10)  |
               (((mode->CrtcVDisplay - 1) & 0x400) >> 9) |
               (((mode->CrtcVSyncStart) & 0x400) >> 8)   |
               (((mode->CrtcVSyncStart) & 0x400) >> 6)   | 0x40;

   
   width = (vga256InfoRec.displayWidth * (vgaBitsPerPixel / 8))>> 3;
   new->std.CRTC[19] = 0xFF & width;
   new->CR51 = (0x300 & width) >> 4; /* Extension bits */
   
   /* And finally, select clock source 2 for programmable PLL */
   new->std.MiscOutReg |= 0x0c;      

   /* Set frame buffer description */
   if (vgaBitsPerPixel <= 8)
      new->CR50 = 0;
   else if (vgaBitsPerPixel <= 16)
      new->CR50 = 0x10;
   else
      new->CR50 = 0x30;

   if (vga256InfoRec.displayWidth == 640)
      new->CR50 |= 0x40;
   else if (vga256InfoRec.displayWidth == 800)
      new->CR50 |= 0x80;
   else if (vga256InfoRec.displayWidth == 1024);
   else if (vga256InfoRec.displayWidth == 1152)
      new->CR50 |= 0x01;
   else if (vga256InfoRec.displayWidth == 1280)
      new->CR50 |= 0x41;
   else if (vga256InfoRec.displayWidth == 2048 && new->CR31 & 2);
   else if (vga256InfoRec.displayWidth == 1600)
      new->CR50 |= 0x81; /* TODO: need to consider bpp=4 */
   else
      new->CR50 |= 0xC1; /* default to use GlobalBD */

   new->CR33 = 0x20;
	 

   /* Now we handle various XConfig memory options and others */

   outb(vgaCRIndex, 0x36);
   new->CR36 = inb(vgaCRReg);
   
   if (mode->Private) {
      new->CR67 &= ~1;
      if( 
        (s3vPriv.chip != S3_SAVAGE2000) &&
        (mode->Private[0] & (1 << S3_INVERT_VCLK)) &&
	(mode->Private[S3_INVERT_VCLK])
      )
	 new->CR67 |= 1;

      if (mode->Private[0] & (1 << S3_BLANK_DELAY)) {
	 new->CR65 = (new->CR65 & ~0x38) 
	    | (mode->Private[S3_BLANK_DELAY] & 0x07) << 3;
      }
   }

   outb(vgaCRIndex, 0x68);
   new->CR68 = inb(vgaCRReg);
   new->CR69 = 0;
   outb(vgaCRIndex, 0x6F);
   new->CR6F = inb(vgaCRReg);
   outb(vgaCRIndex, 0x86);
   new->CR86 = inb(vgaCRReg);
   outb(vgaCRIndex, 0x88);
   new->CR88 = inb(vgaCRReg) | 0x08;
   outb(vgaCRIndex, 0xB0);
   new->CRB0 = inb(vgaCRReg) | 0x80;

   return TRUE;
}

/* This function inits the frame buffer. Right now, it is is rather limited 
 * but the hardware cursor hooks should probably end up here 
 */

void 
S3SAVFbInit()
{

   /* Call S3VAccelInit to setup the XAA accelerated functions */

   if(!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options))
      S3SAVAccelInit();

   if(OFLG_ISSET(OPTION_PCI_RETRY, &vga256InfoRec.options))
      if(OFLG_ISSET(OPTION_PCI_BURST_ON, &vga256InfoRec.options)) {
         s3vPriv.NoPCIRetry = 0;
         }
      else {
         s3vPriv.NoPCIRetry = 1;   
         ErrorF("%s %s: \"pci_retry\" option requires \"pci_burst\".\n",
              XCONFIG_GIVEN, vga256InfoRec.name);
         }

   /* If an LCD panel is attached, any modes less than the LCD size */
   /* might use stretching.  In that case, the hardware cursor does */
   /* not track correctly.  Thus, we have to abandon the hardware  */
   /* cursor. */

   outb(vgaCRIndex, 0x6d);
   if( inb(vgaCRReg) & LCD_ATTACHED ) {
       ErrorF("%s %s: %s: LCD forces software cursor\n",
	       XCONFIG_PROBED, vga256InfoRec.name,
	       vga256InfoRec.chipset);
   }
   else {
       if (!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) {
	  vgaHWCursor.Initialized = TRUE;
	  vgaHWCursor.Init = S3SAVCursorInit;
	  vgaHWCursor.Restore = S3SAVRestoreCursor;
	  vgaHWCursor.Warp = S3SAVWarpCursor;
	  vgaHWCursor.QueryBestSize = S3SAVQueryBestSize;
	  if (xf86Verbose)
		    ErrorF("%s %s: %s: Using hardware cursor\n",
			    XCONFIG_PROBED, vga256InfoRec.name,
			    vga256InfoRec.chipset);
	  }
   }

}


/* This function inits the STREAMS processor variables. 
 * This has essentially been taken from the accel/s3_virge code and the databook.
 */
void
S3SAVInitSTREAMS(streams, mode)
int * streams;
DisplayModePtr mode;
{
  
   if ( vga256InfoRec.bitsPerPixel == 24 ) {
                         /* data format 8.8.8 (24 bpp) */
      streams[0] = 0x06000000;
      } 
   else if (vga256InfoRec.bitsPerPixel == 32) {
                         /* one more bit for X.8.8.8, 32 bpp */
      streams[0] = 0x07000000;
   }
                         /* NO chroma keying... */
   streams[1] = 0x0;
                         /* Secondary stream format KRGB-16 */
                         /* data book suggestion... */
   streams[2] = 0x03000000;

   streams[3] = 0x0;

   streams[4] = 0x0;
                         /* use 0x01000000 for primary over second. */
                         /* use 0x0 for second over prim. */
   streams[5] = 0x01000000;

   streams[6] = 0x0;

   streams[7] = 0x0;
                                /* Stride is 3 bytes for 24 bpp mode and */
                                /* 4 bytes for 32 bpp. */
   if ( vga256InfoRec.bitsPerPixel == 24 ) {
      streams[8] = 
             vga256InfoRec.displayWidth * 3;
      } 
   else {
      streams[8] = 
             vga256InfoRec.displayWidth * 4;
      }
                                /* Choose fbaddr0 as stream source. */
   streams[9] = 0x0;
   streams[10] = 0x0;
   streams[11] = 0x0;
   streams[12] = 0x1;

                                /* Set primary stream on top of secondary */
                                /* stream. */
   streams[13] = 0xc0000000;
                               /* Vertical scale factor. */
   streams[14] = 0x0;

   streams[15] = 0x0;
                                /* Vertical accum. initial value. */
   streams[16] = 0x0;
                                /* X and Y start coords + 1. */
   streams[18] =  0x00010001;

         /* Specify window Width -1 and Height of */
         /* stream. */
   streams[19] =
         (mode->HDisplay - 1) << 16 |
         (mode->VDisplay);
   
                                /* Book says 0x07ff07ff. */
   streams[20] = 0x07ff07ff;

   streams[21] = 0x00010001;
                            
}

/* This function saves the STREAMS registers to our private structure */

void
S3SAVSaveSTREAMS(streams)
int * streams;
{

   streams[0] = ((mmtr)s3savMmioMem)->streams_regs.regs.prim_stream_cntl;
   streams[1] = ((mmtr)s3savMmioMem)->streams_regs.regs.col_chroma_key_cntl;
   streams[2] = ((mmtr)s3savMmioMem)->streams_regs.regs.second_stream_cntl;
   streams[3] = ((mmtr)s3savMmioMem)->streams_regs.regs.chroma_key_upper_bound;
   streams[4] = ((mmtr)s3savMmioMem)->streams_regs.regs.second_stream_stretch;
   streams[5] = ((mmtr)s3savMmioMem)->streams_regs.regs.blend_cntl;
   streams[6] = ((mmtr)s3savMmioMem)->streams_regs.regs.prim_fbaddr0;
   streams[7] = ((mmtr)s3savMmioMem)->streams_regs.regs.prim_fbaddr1;
   streams[8] = ((mmtr)s3savMmioMem)->streams_regs.regs.prim_stream_stride;
   streams[9] = ((mmtr)s3savMmioMem)->streams_regs.regs.double_buffer;
   streams[10] = ((mmtr)s3savMmioMem)->streams_regs.regs.second_fbaddr0;
   streams[11] = ((mmtr)s3savMmioMem)->streams_regs.regs.second_fbaddr1;
   streams[12] = ((mmtr)s3savMmioMem)->streams_regs.regs.second_stream_stride;
   streams[13] = ((mmtr)s3savMmioMem)->streams_regs.regs.opaq_overlay_cntl;
   streams[14] = ((mmtr)s3savMmioMem)->streams_regs.regs.k1;
   streams[15] = ((mmtr)s3savMmioMem)->streams_regs.regs.k2;
   streams[16] = ((mmtr)s3savMmioMem)->streams_regs.regs.dda_vert;
   streams[17] = ((mmtr)s3savMmioMem)->streams_regs.regs.streams_fifo;
   streams[18] = ((mmtr)s3savMmioMem)->streams_regs.regs.prim_start_coord;
   streams[19] = ((mmtr)s3savMmioMem)->streams_regs.regs.prim_window_size;
   streams[20] = ((mmtr)s3savMmioMem)->streams_regs.regs.second_start_coord;
   streams[21] = ((mmtr)s3savMmioMem)->streams_regs.regs.second_window_size;

}

/* This function restores the saved STREAMS registers */

void
S3SAVRestoreSTREAMS(streams)
int * streams;
{

/* For now, set most regs to their default values for 24bpp 
 * Restore only those that are needed for width/height/stride
 * Otherwise, we seem to get lockups because some registers 
 * when saved have some reserved bits set.
 */

   ((mmtr)s3savMmioMem)->streams_regs.regs.prim_stream_cntl = 
         streams[0] & 0x77000000;
   ((mmtr)s3savMmioMem)->streams_regs.regs.col_chroma_key_cntl = streams[1];
   ((mmtr)s3savMmioMem)->streams_regs.regs.second_stream_cntl = streams[2];
   ((mmtr)s3savMmioMem)->streams_regs.regs.chroma_key_upper_bound = streams[3];
   ((mmtr)s3savMmioMem)->streams_regs.regs.second_stream_stretch = streams[4];
   ((mmtr)s3savMmioMem)->streams_regs.regs.blend_cntl = streams[5];
   ((mmtr)s3savMmioMem)->streams_regs.regs.prim_fbaddr0 = streams[6];
   ((mmtr)s3savMmioMem)->streams_regs.regs.prim_fbaddr1 = streams[7];
   ((mmtr)s3savMmioMem)->streams_regs.regs.prim_stream_stride = 
         streams[8] & 0x0fff;
   ((mmtr)s3savMmioMem)->streams_regs.regs.double_buffer = streams[9];
   ((mmtr)s3savMmioMem)->streams_regs.regs.second_fbaddr0 = streams[10];
   ((mmtr)s3savMmioMem)->streams_regs.regs.second_fbaddr1 = streams[11];
   ((mmtr)s3savMmioMem)->streams_regs.regs.second_stream_stride = streams[12];
   ((mmtr)s3savMmioMem)->streams_regs.regs.opaq_overlay_cntl = streams[13];
   ((mmtr)s3savMmioMem)->streams_regs.regs.k1 = streams[14];
   ((mmtr)s3savMmioMem)->streams_regs.regs.k2 = streams[15];
   ((mmtr)s3savMmioMem)->streams_regs.regs.dda_vert = streams[16];
   ((mmtr)s3savMmioMem)->streams_regs.regs.prim_start_coord = streams[18];
   ((mmtr)s3savMmioMem)->streams_regs.regs.prim_window_size = 
         streams[19] & 0x07ff07ff;
   ((mmtr)s3savMmioMem)->streams_regs.regs.second_start_coord = streams[20];
   ((mmtr)s3savMmioMem)->streams_regs.regs.second_window_size = streams[21];


}


/* And this function disables the STREAMS processor as per databook.
 * This is usefull before we do a mode change 
 */

void
S3SAVDisableSTREAMS()
{
unsigned char tmp;

   VerticalRetraceWait();
   ((mmtr)s3savMmioMem)->memport_regs.regs.fifo_control = 0xC000;
   outb(vgaCRIndex, 0x67);
   tmp = inb(vgaCRReg);
                         /* Disable STREAMS processor */
   outb( vgaCRReg, tmp & ~0x0C );


}

/* This function is used to debug, it prints out the contents of s3 regs */

void
S3SAVPrintRegs(void)
{
unsigned char i;

   ErrorF( "SR    x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF" );

   for( i = 0; i < 0x70; i++ ) {
      if( !(i % 16) )
         ErrorF( "\nSR%xx ", i >> 4 );
      outb( 0x3c4, i );
      ErrorF( " %02x", inb(0x3c5) );
   }

   ErrorF( "\n\nCR    x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF" );

   for( i = 0; i < 0xB7; i++ ) {
      if( !(i % 16) )
         ErrorF( "\nCR%xx ", i >> 4 );
      outb( vgaCRIndex, i );
      ErrorF( " %02x", inb(vgaCRReg) );
   }

   ErrorF("\n\n");
}
