/*
 * Stubs driver Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of David Wexelblat not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  David Wexelblat makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Neomagic VGA 800x600x256 driver developed from stub driver.
 *
 */

#include <stdio.h>
#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"
#include "vga.h"
#include "vgaPCI.h"

#ifdef XFreeXDGA 
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#define GRA_I	0x3CE
#define CRT_IC	0x3D4
#define SEQ_I	0x3C4

extern void NeomagicSetRead();
extern void NeomagicSetWrite();
extern void NeomagicSetReadWrite();

typedef struct {
	vgaHWRec std;               /* good old IBM VGA */
	unsigned char extraCRT[4];
	unsigned char extraGRA[23];
} vgaNeomagicRec, *vgaNeomagicPtr;

static Bool     NeomagicProbe();
static char *   NeomagicIdent();
static Bool     NeomagicClockSelect();
static void     NeomagicEnterLeave();
static Bool     NeomagicInit();
static int      NeomagicValidMode();
static void *   NeomagicSave();
static void     NeomagicRestore();
static void     NeomagicAdjust();
static void	NeomagicFbInit();

vgaVideoChipRec NEOMAGIC = {
	/* 
	 * Function pointers
	 */
	NeomagicProbe,
	NeomagicIdent,
	NeomagicEnterLeave,
	NeomagicInit,
	NeomagicValidMode,
	NeomagicSave,
	NeomagicRestore,
	NeomagicAdjust,
	vgaHWSaveScreen,
	(void (*)())NoopDDA,
	NeomagicFbInit,
	NeomagicSetRead,
	NeomagicSetWrite,
	NeomagicSetReadWrite,
	0x10000,	/* 64K VGA window. */
	0x10000,
	16,
	0xFFFF,
	0x00000, 0x10000,
	0x00000, 0x10000,
	FALSE,
	VGA_NO_DIVIDE_VERT,
	{0,},
	8,
	FALSE,
	0,
	0,
	TRUE,		/* Support 16bpp */
	FALSE,
	FALSE,
	NULL,
	1,
};

#define new ((vgaNeomagicPtr)vgaNewVideoState)

/*---------------------------------------------------------------------------*/

static char *
NeomagicIdent(n)
int n;
{
static char *chipsets[] = {"neomagic"};

if (n + 1 > sizeof(chipsets) / sizeof(char *))
  return(NULL);
else
  return(chipsets[n]);
}

/*---------------------------------------------------------------------------*/

static Bool NeomagicClockSelect(no)
int no;
{
static unsigned char save1, save2;
unsigned char temp;

return(TRUE);

switch(no)
  {
  case CLK_REG_SAVE:
    save1 = inb(0x3CC);
    break;
  case CLK_REG_RESTORE:
    outb(0x3C2, save1);
    break;
  default:
    /* Select the external clock, whatever it is */
    temp = inb(0x3CC);
    outb(0x3C2, (temp & 0xF3) | 0x08 );
  }
return(TRUE);
}

/*---------------------------------------------------------------------------*/

static Bool NeomagicProbe()
{
unsigned char temp1, temp2;
xf86ClearIOPortList(vga256InfoRec.scrnIndex);
xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);

/*
 * First we attempt to figure out if one of the supported chipsets
 * is present.
 */
if (vga256InfoRec.chipset)
  {
  if (StrCaseCmp(vga256InfoRec.chipset, NeomagicIdent(0)))
    return (FALSE);
  else
    NeomagicEnterLeave(ENTER);
  }
else
  {
  unsigned char temp, origVal, newVal;
  NeomagicEnterLeave(ENTER);

  if (vgaPCIInfo->Vendor != 0x10C8)	/* Look for the PCI id */
    {
    NeomagicEnterLeave(LEAVE);
    return(FALSE);
    }
  }

if (!vga256InfoRec.videoRam)
  {
  vga256InfoRec.videoRam = 1024;	/* Mine has 1MB - override for others */
  }
/* Not sure what to do about clocks yet...
if (!vga256InfoRec.clocks)
  {
  vga256InfoRec.clocks = 1;
  vga256InfoRec.clock[0] = 25180;
  }

vga256InfoRec.maxClock = 90000;
*/
vga256InfoRec.chipset = NeomagicIdent(0);
vga256InfoRec.bankedMono = FALSE;
#ifndef MONOVGA
#ifdef XFreeXDGA 
vga256InfoRec.directMode = XF86DGADirectPresent;
#endif
#endif

OFLG_SET(OPTION_LINEAR, &NEOMAGIC.ChipOptionFlags);
OFLG_SET(OPTION_NOLINEAR_MODE, &NEOMAGIC.ChipOptionFlags);

if ( (OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options)) &&
     (OFLG_ISSET(OPTION_LINEAR, &vga256InfoRec.options)) )
  {
  NeomagicEnterLeave(LEAVE);
  FatalError("%s %s: Can't have 'nolinear' and 'linear' "
             "defined. Remove one !\n", XCONFIG_GIVEN,
             vga256InfoRec.name);
  }

return (TRUE);
}

/*---------------------------------------------------------------------------*/

static void
NeomagicFbInit()
{
/* Use linear by default - must have it for 16 bpp */
if ( !OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options))
  {
  NEOMAGIC.ChipLinearBase = vgaPCIInfo->MemBase;
  NEOMAGIC.ChipUseLinearAddressing = TRUE;
  NEOMAGIC.ChipLinearSize = vga256InfoRec.videoRam*1024;
  }
}

/*---------------------------------------------------------------------------*/

static void 
NeomagicEnterLeave(enter)
Bool enter;
{
unsigned char temp;

#ifndef MONOVGA
#ifdef XFreeXDGA 
if (vga256InfoRec.directMode&XF86DGADirectGraphics && !enter)
  return;
#endif
#endif

if (enter)
  {
  xf86EnableIOPorts(vga256InfoRec.scrnIndex);

  vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;

  /* Unprotect CRTC[0-7] */
  outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
  outb(vgaIOBase + 5, temp & 0x7F);

  outw(GRA_I ,0x2609);	/* Unlock neo registers */
  }
else
  {
  /* Protect CRTC[0-7] */
  outb(vgaIOBase + 4, 0x11); temp = inb(vgaIOBase + 5);
  outb(vgaIOBase + 5, (temp & 0x7F) | 0x80);

  outw(GRA_I ,0x0009);	/* Lock neo registers */

  xf86DisableIOPorts(vga256InfoRec.scrnIndex);
  }
}

/*---------------------------------------------------------------------------*/

static void 
NeomagicRestore(restore)
vgaNeomagicPtr restore;
{
  outw(GRA_I ,0x0015);	/* Reset bank */

  vgaHWRestore((vgaHWPtr)restore);

  outb(CRT_IC,0x1D);outb(CRT_IC+1,restore->extraCRT[ 0]);
  outb(CRT_IC,0x1F);outb(CRT_IC+1,restore->extraCRT[ 1]);
  outb(CRT_IC,0x21);outb(CRT_IC+1,restore->extraCRT[ 2]);
  outb(CRT_IC,0x23);outb(CRT_IC+1,restore->extraCRT[ 3]);

  outb(GRA_I ,0x0E);outb(GRA_I +1,restore->extraGRA[ 0]);
  outb(GRA_I ,0x0F);outb(GRA_I +1,restore->extraGRA[ 1]);
  outb(GRA_I ,0x10);outb(GRA_I +1,restore->extraGRA[ 2]);
  outb(GRA_I ,0x11);outb(GRA_I +1,restore->extraGRA[ 3]);
  outb(GRA_I ,0x15);outb(GRA_I +1,restore->extraGRA[ 4]);
  outb(GRA_I ,0x16);outb(GRA_I +1,restore->extraGRA[ 5]);
  outb(GRA_I ,0x25);outb(GRA_I +1,restore->extraGRA[ 6]);
  outb(GRA_I ,0x2F);outb(GRA_I +1,restore->extraGRA[ 7]);
  outb(GRA_I ,0x30);outb(GRA_I +1,restore->extraGRA[ 8]);
  outb(GRA_I ,0x82);outb(GRA_I +1,restore->extraGRA[ 9]);
  outb(GRA_I ,0x90);outb(GRA_I +1,restore->extraGRA[10]);
  outb(GRA_I ,0x96);outb(GRA_I +1,restore->extraGRA[11]);
  outb(GRA_I ,0x97);outb(GRA_I +1,restore->extraGRA[12]);
  outb(GRA_I ,0x98);outb(GRA_I +1,restore->extraGRA[13]);
  outb(GRA_I ,0x99);outb(GRA_I +1,restore->extraGRA[14]);
  outb(GRA_I ,0x9A);outb(GRA_I +1,restore->extraGRA[15]);
  outb(GRA_I ,0x9B);outb(GRA_I +1,restore->extraGRA[16]);
  outb(GRA_I ,0x9C);outb(GRA_I +1,restore->extraGRA[17]);
  outb(GRA_I ,0x9D);outb(GRA_I +1,restore->extraGRA[18]);
  outb(GRA_I ,0x9E);outb(GRA_I +1,restore->extraGRA[19]);
  outb(GRA_I ,0x9F);outb(GRA_I +1,restore->extraGRA[20]);
  outb(GRA_I ,0xAE);outb(GRA_I +1,restore->extraGRA[21]);
  outb(GRA_I ,0xB0);outb(GRA_I +1,restore->extraGRA[22]);
}

/*---------------------------------------------------------------------------*/

static void *
NeomagicSave(save)
vgaNeomagicPtr save;
{
  outw(GRA_I ,0x0015);	/* Reset bank */

  save = (vgaNeomagicPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaNeomagicRec));

  outb(CRT_IC,0x1D);save->extraCRT[0] = inb(CRT_IC+1);
  outb(CRT_IC,0x1F);save->extraCRT[1] = inb(CRT_IC+1);
  outb(CRT_IC,0x21);save->extraCRT[2] = inb(CRT_IC+1);
  outb(CRT_IC,0x23);save->extraCRT[3] = inb(CRT_IC+1);

  outb(GRA_I ,0x0E);save->extraGRA[0] = inb(GRA_I +1);
  outb(GRA_I ,0x0F);save->extraGRA[1] = inb(GRA_I +1);
  outb(GRA_I ,0x10);save->extraGRA[2] = inb(GRA_I +1);
  outb(GRA_I ,0x11);save->extraGRA[3] = inb(GRA_I +1);
  outb(GRA_I ,0x15);save->extraGRA[4] = inb(GRA_I +1);
  outb(GRA_I ,0x16);save->extraGRA[5] = inb(GRA_I +1);
  outb(GRA_I ,0x25);save->extraGRA[6] = inb(GRA_I +1);
  outb(GRA_I ,0x2F);save->extraGRA[7] = inb(GRA_I +1);
  outb(GRA_I ,0x30);save->extraGRA[8] = inb(GRA_I +1);
  outb(GRA_I ,0x82);save->extraGRA[9] = inb(GRA_I +1);
  outb(GRA_I ,0x90);save->extraGRA[10] = inb(GRA_I +1);
  outb(GRA_I ,0x96);save->extraGRA[11] = inb(GRA_I +1);
  outb(GRA_I ,0x97);save->extraGRA[12] = inb(GRA_I +1);
  outb(GRA_I ,0x98);save->extraGRA[13] = inb(GRA_I +1);
  outb(GRA_I ,0x99);save->extraGRA[14] = inb(GRA_I +1);
  outb(GRA_I ,0x9A);save->extraGRA[15] = inb(GRA_I +1);
  outb(GRA_I ,0x9B);save->extraGRA[16] = inb(GRA_I +1);
  outb(GRA_I ,0x9C);save->extraGRA[17] = inb(GRA_I +1);
  outb(GRA_I ,0x9D);save->extraGRA[18] = inb(GRA_I +1);
  outb(GRA_I ,0x9E);save->extraGRA[19] = inb(GRA_I +1);
  outb(GRA_I ,0x9F);save->extraGRA[20] = inb(GRA_I +1);
  outb(GRA_I ,0xAE);save->extraGRA[21] = inb(GRA_I +1);
  outb(GRA_I ,0xB0);save->extraGRA[22] = inb(GRA_I +1);

  return ((void *) save);
}

/*---------------------------------------------------------------------------*/

static Bool
NeomagicInit(mode)
DisplayModePtr mode;
{
int i;

if (!vgaHWInit(mode,sizeof(vgaNeomagicRec)))
  return(FALSE);

/* Need to make these modifications to the standard register settings */
/* CRTC[19] is a better guess than what is in vgaHW.c */
if (vgaBitsPerPixel==8)
  new->std.CRTC[19] = vga256InfoRec.displayWidth >> 3;
else	/* 16 bpp */
  new->std.CRTC[19] = vga256InfoRec.displayWidth >> 2;
/* Must have this */
new->std.Attribute[16] = 0x01;

if (vgaBitsPerPixel==16)
  for (i=0;i < 0x40; i++)
    {
    new->std.DAC[i*3+0] = i<<1;
    new->std.DAC[i*3+1] = i   ;
    new->std.DAC[i*3+2] = i<<1;
    }

new->extraCRT[0] = 0x00;
new->extraCRT[1] = 0x00;
new->extraCRT[2] = 0x00;

if (vgaBitsPerPixel==8)
  new->extraCRT[3] = 0x23;
else	/* 16 bpp */
  new->extraCRT[3] = 0x34;

/* GRA registers 0e,0f,10,11,15,16,25,2f,30,82,90,96-9f,ae,b0 */
new->extraGRA[0]  = 0x10;
new->extraGRA[1]  = 0x00;
new->extraGRA[2]  = 0x30;
new->extraGRA[3]  = 0x80;
new->extraGRA[4]  = 0x00;
new->extraGRA[5]  = 0x00;
new->extraGRA[6]  = 0x00;
new->extraGRA[7]  = 0x00;
new->extraGRA[8]  = 0x00;
new->extraGRA[9]  = 0x00;
if (vgaBitsPerPixel==8)
  new->extraGRA[10] = 0x11;
else
  new->extraGRA[10] = 0x13;

new->extraGRA[11] = 0x06;
new->extraGRA[12] = 0x1A;
new->extraGRA[13] = 0xB7;
new->extraGRA[14] = 0xB7;
new->extraGRA[15] = 0xB7;
new->extraGRA[16] = 0xB7;
new->extraGRA[17] = 0x09;
new->extraGRA[18] = 0x09;
new->extraGRA[19] = 0x09;
new->extraGRA[20] = 0x09;
new->extraGRA[21] = 0x00;
new->extraGRA[22] = 0x00;

return(TRUE);
}

/*---------------------------------------------------------------------------*/

static void 
NeomagicAdjust(x, y)
int x, y;
{
/* This works for vertical panning on the 8 bpp mode */
int Base = (y * vga256InfoRec.displayWidth + x + 1)>>2;

outw(vgaIOBase + 4, (Base & 0x00FF00) | 0x0C);
outw(vgaIOBase + 4, ((Base & 0x00FF) << 8) | 0x0D);

#ifdef XFreeXDGA
if (vga256InfoRec.directMode & XF86DGADirectGraphics)
  {
  /* Wait until vertical retrace is in progress. */
  while (inb(vgaIOBase + 0xA) & 0x08);
  while (!(inb(vgaIOBase + 0xA) & 0x08));
  }
#endif
}

/*---------------------------------------------------------------------------*/

static int
NeomagicValidMode(mode, verbose)
DisplayModePtr mode;
Bool verbose;
{
return MODE_OK;
}

/*---------------------------------------------------------------------------*/
