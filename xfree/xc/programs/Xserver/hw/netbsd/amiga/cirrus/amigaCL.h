/*
 * $XConsortium: amigaCL.h,v 1.4 94/04/17 20:29:39 rws Exp $
 *
Copyright (c) 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

typedef unsigned int	Uint;
typedef unsigned short	Ushort;
typedef unsigned char	Uchar;
#if __STDC__
typedef volatile Uint	VUint;
typedef volatile Ushort VUshort;
typedef volatile Uchar	VUchar;
#else
typedef Uint   VUint;
typedef Ushort VUshort;
typedef Uchar  VUchar;
#endif

/* taken from /usr/src/sys/arch/dev/grf_rt3regs.h. See Copyright there */

#define MEMSIZE 1        /* Set this to 1 or 2 (MB), according to the
                            RAM on your 5426 board */

/* have the pattern start right after the displayed framebuffer. That leaves
   us 2M-1280x1x1024 == 256k for pattern-space, which should do more than
   enough. A pattern can be at most 8x8 pixels in 8color, and 8x8 pixels
   in the other modes. (thus 8x1x8=64 bytes) */
#define PAT_MEM_OFF (1280*1*1024)

#define Map(m) \
	do { WGfx(ba, GCT_ID_READ_MAP_SELECT, m & 3 ); WSeq(ba, SEQ_ID_MAP_MASK, (1 << (m & 3))); } while (0)

#define WPLL(ba, idx, val) \
	do { 	vgaw(ba, PLL_ADDRESS, idx);\
	vgaw(ba, PLL_ADDRESS_W, (val & 0xff));\
	vgaw(ba, PLL_ADDRESS_W, (val >> 8)); } while (0)

#if 0
static inline unsigned char RAttr(volatile void * ba, short idx) {
	vgaw (ba, ACT_ADDRESS, idx);
	return vgar (ba, ACT_ADDRESS_R);
}

static inline unsigned char RSeq(volatile void * ba, short idx) {
	vgaw (ba, SEQ_ADDRESS, idx);
	return vgar (ba, SEQ_ADDRESS_R);
}

static inline unsigned char RCrt(volatile void * ba, short idx) {
	vgaw (ba, CRT_ADDRESS, idx);
	return vgar (ba, CRT_ADDRESS_R);
}

static inline unsigned char RGfx(volatile void * ba, short idx) {
	vgaw(ba, GCT_ADDRESS, idx);
	return vgar (ba, GCT_ADDRESS_R);
}
#endif

struct ACM {
    /* all shorts and longs are little-endian!! */
    VUshort	primary;
    VUshort	____________pad0;
    VUshort	secondary;
    VUshort	____________pad1;
    VUchar	____________pad2;
    VUchar	mode;
    VUshort	____________pad3;
    VUshort	cursor_x;
    VUshort	cursor_y;
    VUint	____________pad4;
    VUint	____________pad5[7];
    VUchar	start;
    VUchar	____________pad6;
    VUchar	status;
    VUchar	____________pad7;
    VUshort	control;
    VUshort	____________pad8;
    VUchar	rot_x;
    VUchar	rot_y;
    VUchar	rop;
    VUint	dimension;
    VUint	dst;
    VUint	src;
    VUint	pattern;
    VUint	fg;
    VUint	bg;
};


/* convert big-endian long into little-endian long */

#define M2I(val)\
	asm volatile (" rorw #8,%0   ; \
	                swap %0      ; \
	                rorw #8,%0   ; " : "=d" (val) : "0" (val) );

#define ACM_OFFSET	(0x00b00000)

#include <stdio.h>
#if 0
static FILE *__log;
#define __dolog(fmt,args...) \
{ if (!__log) __log=fopen("/tmp/xlog","w"); \
  fprintf(__log, fmt, ##args); fflush(__log);}
#else
#define __dolog(fmt,args...)
#endif

#define CLBlitInit(regs,tab,op) \
{  *(VUshort *)(regs+0x3ce) = 0x3200|tab[op];		        \
  __dolog("BINIT: pattern = 0x%08lx\n",pmask);			\
}

#define CLMaskInit(regs,fb,pmask,bpp) \
{ unsigned long *_pt = (unsigned long*)((char*)fb+PAT_MEM_OFF);	\
  short _i;							\
  /* pattern size is 8x8xbpp */					\
  if (bpp <= 1) {						\
    Uint _pat;							\
    _pat = (Uchar) pmask;					\
    _pat = (_pat<<24)|(_pat<<16)|(_pat<<8)|_pat;		\
    for (_i = 0; _i < 8*8/4; ++_i) *_pt++ = _pat;		\
  } else if (bpp <= 2) {					\
    Uint _pat;							\
    _pat = (Ushort) pmask;					\
    _pat = (_pat<<16)|_pat;					\
    for (_i = 0; _i < 8*8*2/4; ++_i) *_pt++ = _pat;		\
  } else if (bpp <= 3) {					\
    Uint _pat1, _pat2, _pat3;					\
    _pat3 = pmask & 0x00ffffff;					\
    _pat1 = _pat3 << 8;						\
    _pat2 = (_pat3 << 16)|(_pat3>>8);				\
    _pat1 |= (_pat3 >> 16);					\
    _pat3 |= (_pat3 & 0xff) << 24;				\
    for (_i = 0; _i < 8*8*3/12; ++_i) {				\
      *_pt++ = _pat1; *_pt++ = _pat2; *_pt++ = _pat3;		\
    }								\
  } else { 							\
    /* not available... */					\
  }								\
}

#define CLSetBG(regs,color)\
{ *(VUshort *)(regs+0x3ce) = 0x0000 | (VUchar)(color&0xff); \
  *(VUshort *)(regs+0x3ce) = 0x1000 | (VUchar)(color&0xff); \
}

#define CLSetFG(regs,color)\
{ *(VUshort *)(regs+0x3ce) = 0x0100 | (VUchar)(color&0xff); \
  *(VUshort *)(regs+0x3ce) = 0x1100 | (VUchar)(color&0xff); \
}


/* no pattern implemented !! */
#define CLBlitFB2FB(regs,rop,sx,sy,dx,dy,w,h,fbw,bpp) \
{ Uint dst = (dx + dy * fbw);					\
  Uchar mod = 0x00;/* INCREMENT,SRC=fb,DST=fb,(not static-pattern) */	\
  Uint src=0; \
  /*Ushort mod = 0xc0c0;*//* RIGHT,DOWN,SRC=fb,DST=fb */		\
  __dolog("FB2FB: rop=%d,sx=%d,sy=%d,dx=%d,dy=%d,w=%d,h=%d,fbw=%d,bpp=%d\n", \
	  rop,sx,sy,dx,dy,w,h,fbw,bpp); \
  if (optabs[rop]) {							\
    src = (sx + sy * fbw);				\
    if ((dx+(dy*fbw)) > (sx+(sy*fbw))) {							\
      mod |= 0x01; /* DECREMENT */					\
      src += (h-1)*fbw+w;						\
      dst += (h-1)*fbw+w;						\
    }\
  }else{								\
    src=dst;\
  }\
  *(VUshort *)(regs+0x3ce) = 0x2800 | (VUchar)(dst&0xff);                         \
  *(VUshort *)(regs+0x3ce) = 0x2900 | (VUchar)((dst>>8)&0xff);                                \
  *(VUshort *)(regs+0x3ce) = 0x2a00 | (VUchar)((dst>>16)&0x1f);                                \
  *(VUshort *)(regs+0x3ce) = 0x2c00 | (VUchar)(src&0xff);                  \
  *(VUshort *)(regs+0x3ce) = 0x2d00 | (VUchar)((src>>8)&0xff);              \
  *(VUshort *)(regs+0x3ce) = 0x2e00 | (VUchar)((src>>16)&0x1f);              \
\
  *(VUshort *)(regs+0x3ce) = 0x2400 | (VUchar)(fbw&0xff);                  \
  *(VUshort *)(regs+0x3ce) = 0x2500 | (VUchar)((fbw>>8)&0x0f);              \
\
  *(VUshort *)(regs+0x3ce) = 0x2600 | (VUchar)(fbw&0xff);              \
  *(VUshort *)(regs+0x3ce) = 0x2700 | (VUchar)((fbw>>8)&0x0f);                  \
\
  *(VUshort *)(regs+0x3ce) = 0x2000 | (VUchar)(w&0xff);                  \
  *(VUshort *)(regs+0x3ce) = 0x2100 | (VUchar)((w>>8)&0x07);              \
\
  *(VUshort *)(regs+0x3ce) = 0x2200 | (VUchar)(h&0xff);              \
  *(VUshort *)(regs+0x3ce) = 0x2300 | (VUchar)((h>>8)&0x03);                  \
\
  *(VUshort *)(regs+0x3ce) = 0x3000 | (VUchar)mod;							\
  *(VUshort *)(regs+0x3ce) = 0x3100;\
  *(VUshort *)(regs+0x3ce) = 0x3102;\
}

#define RZ3WaitDone(acm) \
{ while ((acm->status & 1) == 0) ; }

extern int  amigaCLScreenPrivateIndex;
extern int  amigaCLGCPrivateIndex;
extern int  amigaCLWindowPrivateIndex;

#define amigaCLGetScreenPrivate(s)    ((amigaCLPtr) \
			    (s)->devPrivates[amigaCLScreenPrivateIndex].ptr)

typedef struct _amigaCLStipple {
    Uint	fore, back;
    Uint	patalign;
    Uint	alu;
    Uint	bits[8];	/* actually 16 shorts */
} amigaCLStippleRec, *amigaCLStipplePtr;

typedef struct _amigaCLPrivGC {
    int		    type;
    amigaCLStipplePtr stipple;
} amigaCLPrivGCRec, *amigaCLPrivGCPtr;

#define amigaCLGetGCPrivate(g)	    ((amigaCLPrivGCPtr) \
			    (g)->devPrivates[amigaCLGCPrivateIndex].ptr)

#define amigaCLGetWindowPrivate(w)    ((amigaCLStipplePtr) \
			    (w)->devPrivates[amigaCLWindowPrivateIndex].ptr)

#define amigaCLSetWindowPrivate(w,p) (\
	    (w)->devPrivates[amigaCLWindowPrivateIndex].ptr = (pointer) p)


#define amigaInfo(s) (&amigaFbs[(s)->myNum])

/* amigaCL.c */
Bool amigaCLGXInit(ScreenPtr, fbFd *);
/* clgc.c */
void clValidateGC(GCPtr, unsigned long, DrawablePtr);
Bool clCreateGC(GCPtr);
GCOps *clMatchCommon(GCPtr, cfbPrivGCPtr);
/* clbitblt.c */
void clDoBitblt(DrawablePtr, DrawablePtr, int, RegionPtr, DDXPointPtr,
    unsigned long);
RegionPtr clCopyArea(DrawablePtr, DrawablePtr, GC *, int, int, int, int, int,
    int);
/* clfillrct.c */
void clPolyFillRect(DrawablePtr, GCPtr, int, xRectangle	*);
/* clwindow.c */
void clCopyWindow(WindowPtr, DDXPointRec, RegionPtr);

void clDoBitbltCopy(DrawablePtr, DrawablePtr, int, RegionPtr,
    DDXPointPtr, unsigned long);
