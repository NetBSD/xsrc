/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/vga/vga.h,v 3.17 1996/09/29 13:41:33 dawes Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 *
 */
/* $XConsortium: vga.h /main/7 1995/12/02 09:07:16 kaleb $ */

#ifndef _XF86_VGA_H_
#define _XF86_VGA_H_

#define VGA2_PATCHLEVEL "0"
#define VGA16_PATCHLEVEL "0"
#define SVGA_PATCHLEVEL "0"

#include "X.h"
#include "misc.h"
#include "input.h"
#include "scrnintstr.h"
#include "colormapst.h"

#if !defined(MONOVGA) && !defined(XF86VGA16)
extern void    vgaBitBlt(
#if NeedFunctionPrototypes
	unsigned char *, unsigned char *,
	int, int,
	int, int, int, int, int, int,
	int, int,
	int,
	unsigned long
#endif
);
extern void    vgaImageRead(
#if NeedFunctionPrototypes
	unsigned char *, unsigned char *,
	int, int,
	int, int, int, int, int, int,
	int, int,
	int,
	unsigned long
#endif
);
extern void    vgaImageWrite(
#if NeedFunctionPrototypes
	unsigned char *, unsigned char *,
	int, int,
	int, int, int, int, int, int,
	int, int,
	int,
	unsigned long
#endif
);
extern void    vgaPixBitBlt(
#if NeedFunctionPrototypes
	unsigned char *, unsigned char *,
	int, int,
	int, int, int, int, int, int,
	int, int,
	int,
	unsigned long
#endif
);
#endif

#ifdef MONOVGA
extern int    vga2ValidTokens[];
#else
#ifdef XF86VGA16
extern int    vga16ValidTokens[];
#else
extern int    vga256ValidTokens[];
#endif
#endif


/*
 * structure for accessing the video chip`s functions
 */
typedef struct {
  Bool (* ChipProbe)();
  char * (* ChipIdent)();
  void (* ChipEnterLeave)();
  Bool (* ChipInit)();
  int (* ChipValidMode)();
  void * (* ChipSave)();
  void (* ChipRestore)();
  void (* ChipAdjust)();
  void (* ChipSaveScreen)();
  void (* ChipGetMode)();
  void (* ChipFbInit)();
  void (* ChipSetRead)();
  void (* ChipSetWrite)();
  void (* ChipSetReadWrite)();
  int ChipMapSize;
  int ChipSegmentSize;
  int ChipSegmentShift;
  int ChipSegmentMask;
  int ChipReadBottom;
  int ChipReadTop;
  int ChipWriteBottom;
  int ChipWriteTop;
  Bool ChipUse2Banks;               /* TRUE if the chip uses separate read
                                       and write banks */    
  int ChipInterlaceType;            /* divide vertical timing values for
                                       interlaced modes */
  OFlagSet ChipOptionFlags;         /* option flags support by this driver */
  int ChipRounding;                 /* the horizontal resolution rounding */
  Bool ChipUseLinearAddressing;	    /* TRUE if driver has requested linear
  				       addressing */
  int ChipLinearBase;		    /* Physical base address of the linear
  				       framebuffer */
  int ChipLinearSize;		    /* Size of the linear framebuffer */
  Bool ChipHas16bpp;		    /* Driver supports 16bpp */
  Bool ChipHas24bpp;		    /* Driver supports 24bpp */
  Bool ChipHas32bpp;		    /* Driver supports 32bpp */
  DisplayModePtr ChipBuiltinModes;  /* Pointer to builtin mode list */
  int ChipClockScaleFactor;	    /* Factor to divide raw clocks by */
} vgaVideoChipRec, *vgaVideoChipPtr;

/* Tables in vgatables.c */
extern unsigned char byte_reversed[256];

/* All each driver to set a display pitch other than virtualX */
void vgaSetPitchAdjustHook(int (* ChipPitchAdjust)());

/* Allow each driver to hook the ScreenInit function */
void vgaSetScreenInitHook(Bool (* ChipScrInit)());

/*
 * hooks for communicating with the VideoChip on the VGA
 */
extern Bool (* vgaInitFunc)(
#if NeedFunctionPrototypes
    DisplayModePtr
#endif
);
extern int (* vgaValidModeFunc)(
#if NeedFunctionPrototypes
    DisplayModePtr,
    Bool
#endif
);
extern void (* vgaEnterLeaveFunc)(
#if NeedFunctionPrototypes
    Bool
#endif
);
extern void * (* vgaSaveFunc)(
#if NeedFunctionPrototypes
    void *
#endif
);
extern void (* vgaRestoreFunc)(
#if NeedFunctionPrototypes
    void *
#endif
);
extern void (* vgaAdjustFunc)(
#if NeedFunctionPrototypes
    int,
    int
#endif
);
extern void (* vgaSaveScreenFunc)(
#if NeedFunctionPrototypes
	int
#endif
);
extern void (* vgaSetReadFunc)(
#if NeedFunctionPrototypes
	int
#endif
);
extern void (* vgaSetWriteFunc)(
#if NeedFunctionPrototypes
	int
#endif
);
extern void (* vgaSetReadWriteFunc)(
#if NeedFunctionPrototypes
	int
#endif
);
extern int vgaMapSize;
extern int vgaSegmentSize;
extern int vgaSegmentShift;
extern int vgaSegmentMask;
extern int vgaIOBase;
extern int vgaInterlaceType;
extern int vgaBitsPerPixel;
extern int vgaBytesPerPixel;
extern Bool vgaDAC8BitComponents;

#if !defined(S3_SERVER) && !defined(MACH32_SERVER) && !defined(MACH64_SERVER)
#include "vgaBank.h"
#endif

extern pointer vgaOrigVideoState;    /* buffers for all video information */
extern pointer vgaNewVideoState;
extern pointer vgaBase;              /* the framebuffer himself */
extern pointer vgaLinearBase;


typedef struct {
  unsigned char MiscOutReg;     /* */
  unsigned char CRTC[25];       /* Crtc Controller */
  unsigned char Sequencer[5];   /* Video Sequencer */
  unsigned char Graphics[9];    /* Video Graphics */
  unsigned char Attribute[21];  /* Video Atribute */
  unsigned char DAC[768];       /* Internal Colorlookuptable */
  char NoClock;                 /* number of selected clock */
  pointer FontInfo1;            /* save area for fonts in plane 2 */ 
  pointer FontInfo2;            /* save area for fonts in plane 3 */ 
  pointer TextInfo;             /* save area for text */ 
} vgaHWRec, *vgaHWPtr;

typedef struct {
  Bool Initialized;
  void (*Init)();
  void (*Restore)();
  void (*Warp)();
  void (*QueryBestSize)();
} vgaHWCursorRec, *vgaHWCursorPtr;

#define OVERSCAN 0x11		/* Index of OverScan register */

#ifdef MONOVGA
#define BIT_PLANE 3		/* Which plane we write to in mono mode */
#else
#define BITS_PER_GUN 6
#define COLORMAP_SIZE 256

#endif

#define DACDelay \
	{ \
		unsigned char temp = inb(vgaIOBase + 0x0A); \
		temp = inb(vgaIOBase + 0x0A); \
	}

#ifdef MONOVGA
#define vga256InfoRec vga2InfoRec
#endif
#ifdef XF86VGA16
#define vga256InfoRec vga16InfoRec
#define vga2InfoRec vga16InfoRec
#endif
#ifdef S3_SERVER
#define vga256InfoRec s3InfoRec
#endif
#ifdef MACH32_SERVER
#define vga256InfoRec mach32InfoRec
#endif
#ifdef MACH64_SERVER
#define vga256InfoRec mach64InfoRec
#endif
#ifdef AGX_SERVER
#define vga256InfoRec agxInfoRec
#endif
extern ScrnInfoRec vga256InfoRec;

/* Values for vgaInterlaceType */
#define VGA_NO_DIVIDE_VERT     0
#define VGA_DIVIDE_VERT        1

/*
 * This are used to tell the ChipSaveScreen() functions to save/restore
 * anything that must be retained across a synchronous reset of the SVGA
 */
#define SS_START		0
#define SS_FINISH		1

/*
 * List of I/O ports for generic VGA
 */
extern unsigned VGA_IOPorts[];
extern int Num_VGA_IOPorts;

/* Function Prototypes */

/* vgaHW.c */
void vgaProtect(
#if NeedFunctionPrototypes
    Bool on
#endif
);

Bool vgaSaveScreen(
#if NeedFunctionPrototypes
    ScreenPtr pScreen,
    Bool on
#endif
);

void vgaHWSaveScreen(
#if NeedFunctionPrototypes
    Bool start
#endif
);

void vgaHWRestore(
#if NeedFunctionPrototypes
    vgaHWPtr restore
#endif
);

void *vgaHWSave(
#if NeedFunctionPrototypes
    vgaHWPtr save,
    int size
#endif
);

Bool vgaHWInit(
#if NeedFunctionPrototypes
    DisplayModePtr mode,
    int size
#endif
);

void vgaGetClocks(
#if NeedFunctionPrototypes
    int num,
    Bool (*ClockFunc)()
#endif
);

/* vga.c */
void NoopVGA(
#if NeedFunctionPrototypes
    void
#endif
);

void vgaRestore(
#if NeedFunctionPrototypes
    pointer mode
#endif
);

void vgaPrintIdent(
#if NeedFunctionPrototypes
    void
#endif
);

Bool vgaProbe(
#if NeedFunctionPrototypes
    void
#endif
);

Bool vgaScreenInit(
#if NeedFunctionPrototypes
    int scr_index,
    ScreenPtr pScreen,
    int argc,
    char **argv
#endif
);

int vgaValidMode(
#if NeedFunctionPrototypes
    DisplayModePtr,
    Bool
#endif
);

void vgaEnterLeaveVT(
#if NeedFunctionPrototypes
    Bool enter,
    int screen_idx
#endif
);

Bool vgaCloseScreen(
#if NeedFunctionPrototypes
    int screen_idx,
    ScreenPtr pScreen
#endif
);

void vgaAdjustFrame(
#if NeedFunctionPrototypes
    int x,
    int y
#endif
);

Bool vgaSwitchMode(
#if NeedFunctionPrototypes
    DisplayModePtr mode
#endif
);

/* vgaCmap.c */
int vgaListInstalledColormaps(
#if NeedFunctionPrototypes
    ScreenPtr pScreen,
    Colormap *pmaps
#endif
);

int vgaGetInstalledColormaps(
#if NeedFunctionPrototypes
    ScreenPtr pScreen,
    ColormapPtr *pmaps
#endif
);

void vgaStoreColors(
#if NeedFunctionPrototypes
    ColormapPtr pmap,
    int ndef,
    xColorItem *pdefs
#endif
);

void vgaInstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr pmap
#endif
);

void vgaUninstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr pmap
#endif
);

#endif /* _XF86_VGA_H_ */
