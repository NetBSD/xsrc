/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tdfx/vb.h,v 1.1.2.6 1999/10/21 12:08:07 hohndel Exp $ */
/*
   Voodoo Banshee driver version 1.0.2

   Author: Daryll Strauss

   Copyright: 1998,1999
*/

#ifndef _VB_H_
#define _VB_H_

#define VB_PCI_IO 1
#if 0
/* These are not normally turned on. They are only included for debugging. */
#define TRACEACCEL 1
#define TRACE 1
#define TRACECURS 1
#define REGDEBUG 1
#endif

#ifdef TRACE
#define VBTRACE ErrorF
#else
#define VBTRACE 0 && (unsigned long)
#endif

#ifdef TRACEACCEL
#define VBTRACEACCEL ErrorF
#else
#define VBTRACEACCEL 0 && (unsigned long)
#endif

#ifdef TRACECURS
#define VBTRACECURS ErrorF
#else
#define VBTRACECURS 0 && (unsigned long)
#endif

#ifdef TRACEREG
#define VBTRACEREG ErrorF
#else
#define VBTRACEREG 0 && (unsigned long)
#endif

#include "vga.h"
#include "vgaPCI.h"

typedef struct {
  vgaHWRec std;
  unsigned char ExtVga[2];
  unsigned int vidcfg;
  unsigned int vidpll;
  unsigned int dacmode;
  unsigned int vgainit0;
  unsigned int vgainit1;
  unsigned int screensize;
  unsigned int stride;
  unsigned int cursloc;
  unsigned int startaddr;
  unsigned int clip0min;
  unsigned int clip0max;
  unsigned int clip1min;
  unsigned int clip1max;
} vgaVBRec, *vgaVBPtr;

typedef struct {
  pciConfigPtr		PciConfig;
  CARD32		IOAddress;
  CARD32		FbAddress;
  long			FbMapSize;
  CARD32		IOBase;
  volatile unsigned char *IOMap;
#ifdef __alpha__
  volatile unsigned char *IOMapDense;
#endif
  volatile unsigned char *FbMap;
  int			MinClock;
  int			MaxClock;
  int			cpp; /* char per pixel */
  int			stride; /* bytes per line */
  int			CursorData;
  pciTagRec		pciTag;
  int			maxClip;
  int			BlitDir;
  int			CurrentROP;
  int			CurrentCmd;
  int			ClipSelect;
  int			DashedLineSize;
  Bool			Transparent;
  int			LinePatternBuffer;
  int			PciCnt;
  int			BltPrevY;
  Bool			ErrorSet;
  Bool			vgaInitDone;
} VBRec, *VBPtr;

extern VBRec VBinfo;

#define VBPTR() ((VBPtr)(&VBinfo))

extern Bool VBHWCursorInit();
extern void VBHideCursor();
extern Bool VBAccelInit();
extern Bool VB8AccelInit();
extern Bool VB16AccelInit();
extern Bool VB24AccelInit();
extern Bool VB32AccelInit();

#define REF32(addr) *((volatile int*)&pVB->IOMap[addr])

#ifdef VB_PCI_IO
#define VGA_REG(reg) (pVB->IOBase+(reg)-0x300)
#else
#define VGA_REG(reg) (reg)
#endif

#include "vbdefs.h"

#endif


