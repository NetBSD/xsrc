/* $XConsortium: regs3.h,v 1.3 94/12/27 11:29:42 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/regs3.h,v 3.14 1995/04/24 05:20:08 dawes Exp $ */
/*
 * regs3.h
 * 
 * Written by Jake Richter Copyright (c) 1989, 1990 Panacea Inc., Londonderry,
 * NH - All Rights Reserved
 * 
 * This code may be freely incorporated in any program without royalty, as long
 * as the copyright notice stays intact.
 * 
 * Additions by Kevin E. Martin (martin@cs.unc.edu)
 *
 * Changed for CV64 Amiga board by Michael Teske
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
 * Modified for the CyberVision 64 by Michael Teske
 */


#ifndef _CVREGS3_H
#define _CVREGS3_H
#include "dev/grf_cvreg.h"

/* VESA Approved Register Definitions */
#define	DAC_MASK	0x03c6
#define	DAC_R_INDEX	0x03c7
#define	DAC_W_INDEX	0x03c8
#define	DAC_DATA	0x03c9

#define	DISP_STAT	0x02e8
#define	H_TOTAL		0x02e8
#define	H_DISP		0x06e8
#define	H_SYNC_STRT	0x0ae8
#define	H_SYNC_WID	0x0ee8
#define	V_TOTAL		0x12e8
#define	V_DISP		0x16e8
#define	V_SYNC_STRT	0x1ae8
#define	V_SYNC_WID	0x1ee8
#define	DISP_CNTL	0x22e8
#define	ADVFUNC_CNTL	0x4ae8
#define	SUBSYS_STAT	0x42e8
#define	SUBSYS_CNTL	0x42e8
#define	ROM_PAGE_SEL	0x46e8
#define	CUR_Y		0x82e8
#define	CUR_X		0x86e8
#define	DESTY_AXSTP	0x8ae8
#define	DESTX_DIASTP	0x8ee8
#define	ERR_TERM	0x92e8
#define	MAJ_AXIS_PCNT	0x96e8
#define	GP_STAT		0x9ae8
#define	CMD		0x9ae8
#define	SHORT_STROKE	0x9ee8
#define	BKGD_COLOR	0xa2e8
#define	FRGD_COLOR	0xa6e8
#define	WRT_MASK	0xaae8
#define	RD_MASK		0xaee8
#define	COLOR_CMP	0xb2e8
#define	BKGD_MIX	0xb6e8
#define	FRGD_MIX	0xbae8
#define	MULTIFUNC_CNTL	0xbee8
#define	PIX_TRANS	0xe2e8
#define	PIX_TRANS_EXT	0xe2ea
#define	MIN_AXIS_PCNT	0x0000
#define	SCISSORS_T	0x1000
#define	SCISSORS_L	0x2000
#define	SCISSORS_B	0x3000
#define	SCISSORS_R	0x4000
#define	MEM_CNTL	0x5000
#define	PATTERN_L	0x8000
#define	PATTERN_H	0x9000
#define	PIX_CNTL	0xa000
#define	MULT_MISC2	0xd000
#define	MULT_MISC	0xe000
#define	READ_SEL	0xf000


/* Display Status Bit Fields */
#define	HORTOG		0x0004
#define	VBLANK		0x0002
#define	SENSE		0x0001

/* Horizontal Sync Width Bit Field */
#define HSYNCPOL_NEG	0x0020
#define	HSYNCPOL_POS	0x0000

/* Vertical Sync Width Bit Field */
#define	VSYNCPOL_NEG	0x0020
#define	VSYNCPOL_POS	0x0000

/* Display Control Bit Field */
#define	DISPEN_NC	0x0000
#define	DISPEN_DISAB	0x0040
#define	DISPEN_ENAB	0x0020
#define	INTERLACE	0x0010
#define	DBLSCAN		0x0008
#define	MEMCFG_2	0x0000
#define	MEMCFG_4	0x0002
#define	MEMCFG_6	0x0004
#define	MEMCFG_8	0x0006
#define	ODDBNKENAB	0x0001

/* Subsystem Status Register */
#define	_8PLANE		0x0080
#define	MONITORID_8503	0x0050
#define	MONITORID_8507	0x0010
#define	MONITORID_8512	0x0060
#define	MONITORID_8513	0x0060
#define	MONITORID_8514	0x0020
#define	MONITORID_NONE	0x0070
#define	MONITORID_MASK	0x0070
#define	GPIDLE		0x0008
#define	INVALIDIO	0x0004
#define	PICKFLAG	0x0002
#define	VBLNKFLG	0x0001

/* Subsystem Control Register */
#define	GPCTRL_NC	0x0000
#define	GPCTRL_ENAB	0x4000
#define	GPCTRL_RESET	0x8000
#define CHPTEST_NC	0x0000
#define CHPTEST_NORMAL	0x1000
#define CHPTEST_ENAB	0x2000
#define	IGPIDLE		0x0800
#define	IINVALIDIO	0x0400
#define	IPICKFLAG	0x0200
#define	IVBLNKFLG	0x0100
#define	RGPIDLE		0x0008
#define	RINVALIDIO	0x0004
#define	RPICKFLAG	0x0002
#define	RVBLNKFLG	0x0001

/* Current X, Y & Dest X, Y Mask */
#define	COORD_MASK	0x07ff

#ifdef CLKSEL
#undef CLKSEL
#endif

/* Advanced Function Control Regsiter */
#define	CLKSEL		0x0004
#define	DISABPASSTHRU	0x0001

/* Graphics Processor Status Register */
#define	GPBUSY		0x0200
#define	DATDRDY		0x0100

/* Command Register */
#define	CMD_NOP		0x0000
#define	CMD_LINE	0x2000
#define	CMD_RECT	0x4000
#define	CMD_RECTV1	0x6000
#define	CMD_RECTV2	0x8000
#define	CMD_LINEAF	0xa000
#define	CMD_BITBLT	0xc000
#define	CMD_OP_MSK	0xf000
#define	BYTSEQ		0x1000
#define	_16BIT		0x0200
#define	PCDATA		0x0100
#define	INC_Y		0x0080
#define	YMAJAXIS	0x0040
#define	INC_X		0x0020
#define	DRAW		0x0010
#define	LINETYPE	0x0008
#define	LASTPIX		0x0004
#define	PLANAR		0x0002
#define	WRTDATA		0x0001

/*
 * Short Stroke Vector Transfer Register (The angular Defs also apply to the
 * Command Register
 */
#define	VECDIR_000	0x0000
#define	VECDIR_045	0x0020
#define	VECDIR_090	0x0040
#define	VECDIR_135	0x0060
#define	VECDIR_180	0x0080
#define	VECDIR_225	0x00a0
#define	VECDIR_270	0x00c0
#define	VECDIR_315	0x00e0
#define	SSVDRAW		0x0010

/* Background Mix Register */
#define	BSS_BKGDCOL	0x0000
#define	BSS_FRGDCOL	0x0020
#define	BSS_PCDATA	0x0040
#define	BSS_BITBLT	0x0060

/* Foreground Mix Register */
#define	FSS_BKGDCOL	0x0000
#define	FSS_FRGDCOL	0x0020
#define	FSS_PCDATA	0x0040
#define	FSS_BITBLT	0x0060

/* The Mixes */
#define	MIX_MASK			0x001f

#define	MIX_NOT_DST			0x0000
#define	MIX_0				0x0001
#define	MIX_1				0x0002
#define	MIX_DST				0x0003
#define	MIX_NOT_SRC			0x0004
#define	MIX_XOR				0x0005
#define	MIX_XNOR			0x0006
#define	MIX_SRC				0x0007
#define	MIX_NAND			0x0008
#define	MIX_NOT_SRC_OR_DST		0x0009
#define	MIX_SRC_OR_NOT_DST		0x000a
#define	MIX_OR				0x000b
#define	MIX_AND				0x000c
#define	MIX_SRC_AND_NOT_DST		0x000d
#define	MIX_NOT_SRC_AND_DST		0x000e
#define	MIX_NOR				0x000f

#define	MIX_MIN				0x0010
#define	MIX_DST_MINUS_SRC		0x0011
#define	MIX_SRC_MINUS_DST		0x0012
#define	MIX_PLUS			0x0013
#define	MIX_MAX				0x0014
#define	MIX_HALF__DST_MINUS_SRC		0x0015
#define	MIX_HALF__SRC_MINUS_DST		0x0016
#define	MIX_AVERAGE			0x0017
#define	MIX_DST_MINUS_SRC_SAT		0x0018
#define	MIX_SRC_MINUS_DST_SAT		0x001a
#define	MIX_HALF__DST_MINUS_SRC_SAT	0x001c
#define	MIX_HALF__SRC_MINUS_DST_SAT	0x001e
#define	MIX_AVERAGE_SAT			0x001f

/* Memory Control Register */
#define	BUFSWP		0x0010
#define	VRTCFG_2	0x0000
#define	VRTCFG_4	0x0004
#define	VRTCFG_6	0x0008
#define	VRTCFG_8	0x000C
#define	HORCFG_4	0x0000
#define	HORCFG_5	0x0001
#define	HORCFG_8	0x0002
#define	HORCFG_10	0x0003

/* Pixel Control Register */
#define	MIXSEL_FRGDMIX	0x0000
#define	MIXSEL_PATT	0x0040
#define	MIXSEL_EXPPC	0x0080
#define	MIXSEL_EXPBLT	0x00c0
#define COLCMPOP_F	0x0000
#define COLCMPOP_T	0x0008
#define COLCMPOP_GE	0x0010
#define COLCMPOP_LT	0x0018
#define COLCMPOP_NE	0x0020
#define COLCMPOP_EQ	0x0028
#define COLCMPOP_LE	0x0030
#define COLCMPOP_GT	0x0038
#define	PLANEMODE	0x0004

#define amigaInfo(s) (&amigaFbs[(s)->myNum]) 

static __inline unsigned char RAttr(volatile void * ba, short idx) {
	abort();
#if 0
	vgaw (ba, ACT_ADDRESS, idx);
	return vgar (ba, ACT_ADDRESS_R);
#endif
}

static __inline unsigned char RSeq(volatile void * ba, short idx) {
	abort();
#if 0
	vgaw (ba, SEQ_ADDRESS, idx);
	return vgar (ba, SEQ_ADDRESS_R);
#endif
}

static __inline unsigned char RCrt(volatile void * ba, short idx) {
	abort();
#if 0
	vgaw (ba, CRT_ADDRESS, idx);
	return vgar (ba, CRT_ADDRESS_R);
#endif
}

static __inline unsigned char RGfx(volatile void * ba, short idx) {
	abort();
#if 0
	vgaw(ba, GCT_ADDRESS, idx);
	return vgar (ba, GCT_ADDRESS_R);
#endif
}

#ifndef amiga_membarrier
static __inline void amiga_membarrier(void) {
	abort();
}
#endif

/* don't forget to init vgaBase and inf before using these macros! */

#if 0
/* Wait until "v" queue entries are free */
#define	WaitQueue(v) 	{int test; do {\
test = vgar(vgaBase, GP_STAT);\
asm("nop");\
} while (test & (0x0100 >> (v)));} 
#else

#define WaitQueue(v) \
{ \
unsigned short _test; \
volatile caddr_t extvga;\
\
extvga = vgaBase + 0x8000;\
do {\
asm volatile ("movew %1@(0x1ae8),%0" : "=r" (_test) : "a" (extvga));\
} while (_test & (0x0100 >> (v)));} 

#endif

#define WaitQueue16_32(n16,n32) \
         if (inf->info.gd_planes > 16) { \
            WaitQueue(n32); \
         } \
         else \
            WaitQueue(n16)   


# define S3_OUTW(p,n) do {*(volatile unsigned short *)((char *)vgaBase+(p)) = \
                        (unsigned short)(n); asm("nop");} while (0)
# define S3_OUTL(p,n) do {*(volatile unsigned long *)((char *)vgaBase+(p)) = \
                        (unsigned long)(n); asm("nop");}while (0)

#define S3_OUTW32(p,n) \
  if (inf->info.gd_planes > 16) { \
    S3_OUTW(MULTIFUNC_CNTL,MULT_MISC); \
    S3_OUTW(p,n); \
    S3_OUTW(p,(n)>>16); \
  } \
  else \
    S3_OUTW(p,n)        


#define WaitIdle() \
{ \
unsigned short _test; \
volatile caddr_t extvga;\
\
extvga = vgaBase + 0x8000;\
do {\
asm volatile ("movew %1@(0x1ae8),%0" : "=r" (_test) : "a" (extvga));\
} while (_test & (1 << 9) );}




/*GfxBusyWait ((caddr_t)vgaBase) */
#define WaitIdleEmpty()  WaitIdle()

#if 0
#define PCI_HACK()   if (1) WaitIdle() 
#else
#define PCI_HACK() 
#endif

extern Bool amigaCVBlockCursor;
extern Bool amigaCVReloadCursor;  
extern ScreenPtr amigaCVsavepScreen;

#define BLOCK_CURSOR amigaCVBlockCursor = TRUE;
#define UNBLOCK_CURSOR  { \
                           if (amigaCVReloadCursor) \
                              amigaCVRestoreCursor(amigaCVsavepScreen); \
                           amigaCVBlockCursor = FALSE; \
                        }    

#ifndef modulus
#define modulus(a,b,d)  if (((d) = (a) % (b)) < 0) (d) += (b)
#endif 


#if 0
/* x64: Wait until "v" queue entries are free, v>8 for 864/964 */
#define	WaitQueue16(v)	do { while (inw(GP_STAT) & (0x8000 >> (v-9))); } while(0)

/* Wait until GP is idle and queue is empty */
/* x64: bits 15-11 are reserved in 928 and should be zero,
        for 864/964 these are FIFO-STATUS bits 9-13 */
#define	WaitIdleEmpty() \
   do { int fx86=(S3_x64_SERIES(s3ChipId)?0xF800:0); while (inw(GP_STAT) & (GPBUSY | 1 | fx86)); } while (0)

/* Wait until GP is idle */
#define WaitIdle() do { while (inw(GP_STAT) & GPBUSY) ; } while (0)

#define	MODE_800	1
#define	MODE_1024	2
#define	MODE_1280	3
#define MODE_1152	4	/* 801/805 C-step, 928 E-step */
#define MODE_1600	5	/* 928 E-step */

#ifndef NULL
#define NULL	0
#endif

#define RGB8_PSEUDO      (-1)
#define RGB16_565         0
#define RGB16_555         1
#define RGB32_888         2

#endif


#if 0
#include <stdio.h>

static FILE *__log;
#define __dolog(fmt,args...) \
{ __log=fopen("/tmp/xlog","a"); \
  if (__log){\
     fprintf(__log, fmt, ##args); fflush(__log);\
     fclose(__log);\
  }\
}
#else
#define __dolog(fmt,args...)
#endif

typedef unsigned int  Uint; 
extern int  amigaCVScreenPrivateIndex;
extern int  amigaCVGCPrivateIndex;
extern int  amigaCVWindowPrivateIndex;


#if 0
typedef struct _CVInfo {
  volatile char *fb; /* Framebuffer address */
  int depth;         /* depth */
  int s3Bpp;         /* Byte per pixel */
  int s3BppDisplayWidth;   /* Display Width in Bytes */
  unsigned long s3BppPMask; /* PlaneMask for all Planes */ 
  int s3Shift;             /* x * s3Bpp == x << s3Shift ! Faster ! */
} CVInfo;
#endif






#if 0
#define amigaCVGetScreenPrivate(s)    ((amigaCVPtr) \
			    (s)->devPrivates[amigaCVScreenPrivateIndex].ptr)
#endif

typedef struct _amigaCVStipple {
    Uint	fore, back;
    Uint	patalign;
    Uint	alu;
    Uint	bits[8];	/* actually 16 shorts */
} amigaCVStippleRec, *amigaCVStipplePtr;

typedef struct _amigaCVPrivGC {
    int		    type;
    amigaCVStipplePtr stipple;
} amigaCVPrivGCRec, *amigaCVPrivGCPtr;

#define amigaCVGetGCPrivate(g)	    ((amigaCVPrivGCPtr) \
			    (g)->devPrivates[amigaCVGCPrivateIndex].ptr)

#define amigaCVGetWindowPrivate(w)    ((amigaCVStipplePtr) \
			    (w)->devPrivates[amigaCVWindowPrivateIndex].ptr)

#define amigaCVSetWindowPrivate(w,p) (\
	    (w)->devPrivates[amigaCVWindowPrivateIndex].ptr = (pointer) p)

/* amiga16CVgc.c */
Bool s3CreateGC16(GCPtr);
void amiga16CVValidateGC(GCPtr, Mask, DrawablePtr);

/* amiga32CVgc.c */
Bool s3CreateGC32(GCPtr);
void amiga32CVValidateGC(GCPtr, Mask, DrawablePtr);

/* amiga8CVgc.c */
Bool s3CreateGC(GCPtr);
void amiga8CVValidateGC(GCPtr, Mask, DrawablePtr);

/* amigaCV.c */
GCOps *amigaCVMatchCommon(GCPtr, cfbPrivGCPtr, int);
void amigaCVDestroyGC(GCPtr);
Bool amigaCVCreateGC(GCPtr);
void amigaCVCopyWindow(WindowPtr, DDXPointRec, RegionPtr);
void amigaCVadjustVirtual(volatile caddr_t);
Bool amigaCVGXInit(ScreenPtr, fbFd *);

/* amigaCVCursor.c */
short swap16(unsigned short);
Bool amigaCVCursorInit(ScreenPtr);
void amigaCVRestoreCursor(ScreenPtr);
void amigaCVRepositionCursor(ScreenPtr);
void amigaCVSetPanning2(fbFd *, unsigned short, unsigned short);
void amigaCVRenewCursorColor(ScreenPtr);
void s3WarpCursor(ScreenPtr, int, int );

/* amigaCVblt.c */
void amigaCVFindOrdering(DrawablePtr, DrawablePtr, GC *, int, BoxPtr, int, int, int, int, unsigned int *);

/* amigaCVbstor.c */
void amigaCVSaveAreas(PixmapPtr, RegionPtr, int, int, WindowPtr);
void amigaCVRestoreAreas(PixmapPtr, RegionPtr, int, int, WindowPtr);

/* amigaCVfrect.c */
void amigaCVInitFrect(int, int, int);
void amigaCVPolyFillRect(DrawablePtr, GCPtr, int, xRectangle *);
void amigaCVFillSpans(DrawablePtr, GCPtr, int, DDXPointPtr, int *, int);

/* amigaCVim.c */
void amigaCVImageInit(fbFd *);
void amigaCVImageWrite(int, int, int, int, char *, int, int, int, short,
    unsigned long, fbFd *);
void amigaCVImageRead(int, int, int, int, char *, int, int, int, unsigned long, fbFd *);
void amigaCVImageFill(int, int, int, int, char *, int, int, int, int, int,
    short, unsigned long, fbFd *);
void amigaCVImageWriteNoMem(int, int, int, int, char *, int, int, int, short, unsigned long, fbFd *);
void amigaCVImageReadNoMem(int, int, int, int, char *, int, int, int, unsigned long, fbFd *);
void amigaCVImageFillNoMem(int, int, int, int, char *, int, int, int, int, int, short, unsigned long, fbFd *);
void amigaCVImageStipple(int, int, int, int, char *, int, int, int, int, int, Pixel, short, unsigned long, fbFd *);
void amigaCVImageOpStipple(int, int, int, int, char *, int, int, int, int, int, Pixel, Pixel, short, unsigned long, fbFd *);

/* amigaCVline.c */
void amigaCVLine(DrawablePtr, GCPtr, int, int, DDXPointPtr);

/* amigaCVplypt.c */
void amigaCVPolyPoint(DrawablePtr, GCPtr, int, int, xPoint *);

/* amigaCVscrin.c */
Bool amigaCVInit(int, ScreenPtr, int, char **);

/* amigaCVseg.c */
void amigaCVSegment(DrawablePtr, GCPtr, int, xSegment *);

/* s3bcach.c */
void s3CacheMoveBlock(int, int, int, int, int, int, unsigned int);

/* s3font.c */
Bool s3RealizeFont(ScreenPtr, FontPtr);
Bool s3UnrealizeFont(ScreenPtr, FontPtr);

/* s3ss.c */
void s3SetSpans(DrawablePtr, GCPtr, char *, DDXPointPtr, int *, int, int);

/* s3text.c */
void s3SimpleStipple(int, int, int, int, unsigned char *, int, fbFd *);
void s3FontStipple(int, int, int, int, unsigned char *, int, Pixel);
int s3NoCPolyText(DrawablePtr, GCPtr, int, int, int, char *, Bool);
int s3NoCImageText(DrawablePtr, GCPtr, int, int, int, char *, Bool);

/* xf86text.c */
void xf86InitText(void (*GlyphWriteFunc )(), int (*NoCPolyTextFunc )(), int (*NoCImageTextFunc )());
int xf86PolyText8(DrawablePtr, GCPtr, int, int, int, char *);
int xf86PolyText16(DrawablePtr, GCPtr, int, int, int, unsigned short *);
void xf86ImageText8(DrawablePtr, GCPtr, int, int, int, char *);
void xf86ImageText16(DrawablePtr, GCPtr, int, int, int, unsigned short *);

#endif /* _REGS3_H */
