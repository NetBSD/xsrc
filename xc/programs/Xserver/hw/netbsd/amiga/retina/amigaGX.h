/*
 * $XConsortium: amigaGX.h,v 1.4 94/04/17 20:29:39 rws Exp $
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

#define MEMSIZE 4        /* Set this to 1 or 4 (MB), according to the
                            RAM on your Retina BLT Z3 board */

/* have the pattern start right after the displayed framebuffer. That leaves
   us 4M-1280x3x1024 == 256k for pattern-space, which should do more than
   enough. A pattern can be at most 16x16 pixels in 8color, and 8x8 pixels
   in the other modes. (thus 8x3x8=192 bytes) */
#define PAT_MEM_OFF (1280*3*1024)

/* read VGA register */
#define vgar(ba, reg) (*(((volatile unsigned char *)ba)+reg))

/* write VGA register */
#define vgaw(ba, reg, val) \
	*(((volatile unsigned char *)ba)+reg) = val

/* defines for the used register addresses (mw)

   NOTE: there are some registers that have different addresses when
         in mono or color mode. We only support color mode, and thus
         some addresses won't work in mono-mode! */

/* General Registers: */
#define GREG_STATUS0_R		0x03C2
#define GREG_STATUS1_R		0x03DA
#define GREG_MISC_OUTPUT_R	0x03CC
#define GREG_MISC_OUTPUT_W	0x03C2	
#define GREG_FEATURE_CONTROL_R	0x03CA
#define GREG_FEATURE_CONTROL_W	0x03DA
#define GREG_POS		0x0102

/* Attribute Controller: */
#define ACT_ADDRESS		0x03C0
#define ACT_ADDRESS_R		0x03C0
#define ACT_ADDRESS_W		0x03C0
#define ACT_ADDRESS_RESET	0x03DA
#define ACT_ID_PALETTE0		0x00
#define ACT_ID_PALETTE1		0x01
#define ACT_ID_PALETTE2		0x02
#define ACT_ID_PALETTE3		0x03
#define ACT_ID_PALETTE4		0x04
#define ACT_ID_PALETTE5		0x05
#define ACT_ID_PALETTE6		0x06
#define ACT_ID_PALETTE7		0x07
#define ACT_ID_PALETTE8		0x08
#define ACT_ID_PALETTE9		0x09
#define ACT_ID_PALETTE10	0x0A
#define ACT_ID_PALETTE11	0x0B
#define ACT_ID_PALETTE12	0x0C
#define ACT_ID_PALETTE13	0x0D
#define ACT_ID_PALETTE14	0x0E
#define ACT_ID_PALETTE15	0x0F
#define ACT_ID_ATTR_MODE_CNTL	0x10
#define ACT_ID_OVERSCAN_COLOR	0x11
#define ACT_ID_COLOR_PLANE_ENA	0x12
#define ACT_ID_HOR_PEL_PANNING	0x13
#define ACT_ID_COLOR_SELECT	0x14

/* Graphics Controller: */
#define GCT_ADDRESS		0x03CE
#define GCT_ADDRESS_R		0x03CE
#define GCT_ADDRESS_W		0x03CF
#define GCT_ID_SET_RESET	0x00
#define GCT_ID_ENABLE_SET_RESET	0x01
#define GCT_ID_COLOR_COMPARE	0x02
#define GCT_ID_DATA_ROTATE	0x03
#define GCT_ID_READ_MAP_SELECT	0x04
#define GCT_ID_GRAPHICS_MODE	0x05
#define GCT_ID_MISC		0x06
#define GCT_ID_COLOR_XCARE	0x07
#define GCT_ID_BITMASK		0x08

/* Sequencer: */
#define SEQ_ADDRESS		0x03C4
#define SEQ_ADDRESS_R		0x03C4
#define SEQ_ADDRESS_W		0x03C5
#define SEQ_ID_RESET		0x00
#define SEQ_ID_CLOCKING_MODE	0x01
#define SEQ_ID_MAP_MASK		0x02
#define SEQ_ID_CHAR_MAP_SELECT	0x03
#define SEQ_ID_MEMORY_MODE	0x04
#define SEQ_ID_EXTENDED_ENABLE	0x05	/* down from here, all seq registers are NCR extensions */
#define SEQ_ID_UNKNOWN1         0x06
#define SEQ_ID_UNKNOWN2         0x07
#define SEQ_ID_CHIP_ID		0x08
#define SEQ_ID_UNKNOWN3         0x09
#define SEQ_ID_CURSOR_COLOR1	0x0A
#define SEQ_ID_CURSOR_COLOR0	0x0B
#define SEQ_ID_CURSOR_CONTROL	0x0C
#define SEQ_ID_CURSOR_X_LOC_HI	0x0D
#define SEQ_ID_CURSOR_X_LOC_LO	0x0E
#define SEQ_ID_CURSOR_Y_LOC_HI	0x0F
#define SEQ_ID_CURSOR_Y_LOC_LO	0x10
#define SEQ_ID_CURSOR_X_INDEX	0x11
#define SEQ_ID_CURSOR_Y_INDEX	0x12
#define SEQ_ID_CURSOR_STORE_HI	0x13	/* manual still wrong here.. argl! */
#define SEQ_ID_CURSOR_STORE_LO	0x14	/* downto 0x16 */
#define SEQ_ID_CURSOR_ST_OFF_HI	0x15
#define SEQ_ID_CURSOR_ST_OFF_LO	0x16
#define SEQ_ID_CURSOR_PIXELMASK	0x17
#define SEQ_ID_PRIM_HOST_OFF_HI	0x18
#define SEQ_ID_PRIM_HOST_OFF_LO	0x19
#define SEQ_ID_LINEAR_0		0x1A
#define SEQ_ID_LINEAR_1		0x1B
#define SEQ_ID_SEC_HOST_OFF_HI	0x1C
#define SEQ_ID_SEC_HOST_OFF_LO	0x1D
#define SEQ_ID_EXTENDED_MEM_ENA	0x1E
#define SEQ_ID_EXT_CLOCK_MODE	0x1F
#define SEQ_ID_EXT_VIDEO_ADDR	0x20
#define SEQ_ID_EXT_PIXEL_CNTL	0x21
#define SEQ_ID_BUS_WIDTH_FEEDB	0x22
#define SEQ_ID_PERF_SELECT	0x23
#define SEQ_ID_COLOR_EXP_WFG	0x24
#define SEQ_ID_COLOR_EXP_WBG	0x25
#define SEQ_ID_EXT_RW_CONTROL	0x26
#define SEQ_ID_MISC_FEATURE_SEL	0x27
#define SEQ_ID_COLOR_KEY_CNTL	0x28
#define SEQ_ID_COLOR_KEY_MATCH0	0x29
#define SEQ_ID_COLOR_KEY_MATCH1 0x2A
#define SEQ_ID_COLOR_KEY_MATCH2 0x2B
#define SEQ_ID_UNKNOWN6         0x2C
#define SEQ_ID_CRC_CONTROL	0x2D
#define SEQ_ID_CRC_DATA_LOW	0x2E
#define SEQ_ID_CRC_DATA_HIGH	0x2F
#define SEQ_ID_MEMORY_MAP_CNTL	0x30
#define SEQ_ID_ACM_APERTURE_1	0x31
#define SEQ_ID_ACM_APERTURE_2	0x32
#define SEQ_ID_ACM_APERTURE_3	0x33
#define SEQ_ID_BIOS_UTILITY_0	0x3e
#define SEQ_ID_BIOS_UTILITY_1	0x3f

/* CRT Controller: */
#define CRT_ADDRESS		0x03D4
#define CRT_ADDRESS_R		0x03D5
#define CRT_ADDRESS_W		0x03D5
#define CRT_ID_HOR_TOTAL	0x00
#define CRT_ID_HOR_DISP_ENA_END	0x01
#define CRT_ID_START_HOR_BLANK	0x02
#define CRT_ID_END_HOR_BLANK	0x03
#define CRT_ID_START_HOR_RETR	0x04
#define CRT_ID_END_HOR_RETR	0x05
#define CRT_ID_VER_TOTAL	0x06
#define CRT_ID_OVERFLOW		0x07
#define CRT_ID_PRESET_ROW_SCAN	0x08
#define CRT_ID_MAX_SCAN_LINE	0x09
#define CRT_ID_CURSOR_START	0x0A
#define CRT_ID_CURSOR_END	0x0B
#define CRT_ID_START_ADDR_HIGH	0x0C
#define CRT_ID_START_ADDR_LOW	0x0D
#define CRT_ID_CURSOR_LOC_HIGH	0x0E
#define CRT_ID_CURSOR_LOC_LOW	0x0F
#define CRT_ID_START_VER_RETR	0x10
#define CRT_ID_END_VER_RETR	0x11
#define CRT_ID_VER_DISP_ENA_END	0x12
#define CRT_ID_OFFSET		0x13
#define CRT_ID_UNDERLINE_LOC	0x14
#define CRT_ID_START_VER_BLANK	0x15
#define CRT_ID_END_VER_BLANK	0x16
#define CRT_ID_MODE_CONTROL	0x17
#define CRT_ID_LINE_COMPARE	0x18
#define CRT_ID_UNKNOWN1         0x19	/* are these register really void ? */
#define CRT_ID_UNKNOWN2         0x1A
#define CRT_ID_UNKNOWN3         0x1B
#define CRT_ID_UNKNOWN4         0x1C
#define CRT_ID_UNKNOWN5         0x1D
#define CRT_ID_UNKNOWN6         0x1E
#define CRT_ID_UNKNOWN7         0x1F
#define CRT_ID_UNKNOWN8         0x20
#define CRT_ID_UNKNOWN9         0x21
#define CRT_ID_UNKNOWN10      	0x22
#define CRT_ID_UNKNOWN11      	0x23
#define CRT_ID_UNKNOWN12      	0x24
#define CRT_ID_UNKNOWN13      	0x25
#define CRT_ID_UNKNOWN14      	0x26
#define CRT_ID_UNKNOWN15      	0x27
#define CRT_ID_UNKNOWN16      	0x28
#define CRT_ID_UNKNOWN17      	0x29
#define CRT_ID_UNKNOWN18      	0x2A
#define CRT_ID_UNKNOWN19      	0x2B
#define CRT_ID_UNKNOWN20      	0x2C
#define CRT_ID_UNKNOWN21      	0x2D
#define CRT_ID_UNKNOWN22      	0x2E
#define CRT_ID_UNKNOWN23      	0x2F
#define CRT_ID_EXT_HOR_TIMING1	0x30	/* down from here, all crt registers are NCR extensions */
#define CRT_ID_EXT_START_ADDR	0x31
#define CRT_ID_EXT_HOR_TIMING2	0x32
#define CRT_ID_EXT_VER_TIMING	0x33
#define CRT_ID_MONITOR_POWER	0x34

/* PLL chip  (clock frequency synthesizer) I'm guessing here... */
#define PLL_ADDRESS		0x83c8
#define PLL_ADDRESS_W		0x83c9


/* Video DAC */
#define VDAC_ADDRESS		0x03c8
#define VDAC_ADDRESS_W		0x03c8
#define VDAC_ADDRESS_R		0x03c7
#define VDAC_STATE		0x03c7
#define VDAC_DATA		0x03c9
#define VDAC_MASK		0x03c6


/* Accelerator Control Menu (memory mapped registers, includes blitter) */
#define ACM_PRIMARY_OFFSET	0x00
#define ACM_SECONDARY_OFFSET	0x04
#define ACM_MODE_CONTROL	0x08
#define ACM_CURSOR_POSITION	0x0c
#define ACM_START_STATUS	0x30
#define ACM_CONTROL		0x34
#define ACM_RASTEROP_ROTATION	0x38
#define ACM_BITMAP_DIMENSION	0x3c
#define ACM_DESTINATION		0x40
#define ACM_SOURCE		0x44
#define ACM_PATTERN		0x48
#define ACM_FOREGROUND		0x4c
#define ACM_BACKGROUND		0x50


#define WGfx(ba, idx, val) \
	do { vgaw(ba, GCT_ADDRESS, idx); vgaw(ba, GCT_ADDRESS_W , val); } while (0)

#define WSeq(ba, idx, val) \
	do { vgaw(ba, SEQ_ADDRESS, idx); vgaw(ba, SEQ_ADDRESS_W , val); } while (0)

#define WCrt(ba, idx, val) \
	do { vgaw(ba, CRT_ADDRESS, idx); vgaw(ba, CRT_ADDRESS_W , val); } while (0)

#define WAttr(ba, idx, val) \
	do { vgaw(ba, ACT_ADDRESS, idx); vgaw(ba, ACT_ADDRESS_W, val); } while (0)

#define Map(m) \
	do { WGfx(ba, GCT_ID_READ_MAP_SELECT, m & 3 ); WSeq(ba, SEQ_ID_MAP_MASK, (1 << (m & 3))); } while (0)

#define WPLL(ba, idx, val) \
	do { 	vgaw(ba, PLL_ADDRESS, idx);\
	vgaw(ba, PLL_ADDRESS_W, (val & 0xff));\
	vgaw(ba, PLL_ADDRESS_W, (val >> 8)); } while (0)


static __inline unsigned char RAttr(volatile void * ba, short idx) {
	vgaw (ba, ACT_ADDRESS, idx);
	return vgar (ba, ACT_ADDRESS_R);
}

static __inline unsigned char RSeq(volatile void * ba, short idx) {
	vgaw (ba, SEQ_ADDRESS, idx);
	return vgar (ba, SEQ_ADDRESS_R);
}

static __inline unsigned char RCrt(volatile void * ba, short idx) {
	vgaw (ba, CRT_ADDRESS, idx);
	return vgar (ba, CRT_ADDRESS_R);
}

static __inline unsigned char RGfx(volatile void * ba, short idx) {
	vgaw(ba, GCT_ADDRESS, idx);
	return vgar (ba, GCT_ADDRESS_R);
}

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

#define RZ3BlitInit(acm,tab,op) \
{   acm->rop = tab[op];						\
  __dolog("BINIT: pattern = 0x%08lx\n",pmask);			\
}

#define RZ3MaskInit(acm,fb,pmask,bpp) \
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

#if 1
#define RZ3BlitFB2FB(acm,rop,sx,sy,dx,dy,w,h,fbw,bpp) \
{ Uint dst = 8 * bpp * (dx + dy * fbw);					\
  Ushort mod = 0xc0c2;/* RIGHT,DOWN,SRC=fb,DST=fb,static-pattern */	\
  /*Ushort mod = 0xc0c0;*//* RIGHT,DOWN,SRC=fb,DST=fb */		\
  Uint pat = 8 * PAT_MEM_OFF;						\
  __dolog("FB2FB: rop=%d,sx=%d,sy=%d,dx=%d,dy=%d,w=%d,h=%d,fbw=%d,bpp=%d\n", \
	  rop,sx,sy,dx,dy,w,h,fbw,bpp); \
  if (optabs[rop]) {							\
    Uint src = 8 * bpp * (sx + sy * fbw);				\
    if (dx > sx) {							\
      mod &= ~0x8000; /* LEFT */					\
      src += 8 * bpp * (w - 1);						\
      dst += 8 * bpp * (w - 1);						\
      pat += 8 * bpp * 2;						\
    }									\
    if (dy > sy) {							\
      mod &= ~0x4000; /* UP */						\
      src += 8 * bpp * (h - 1) * fbw;					\
      dst += 8 * bpp * (h - 1) * fbw;					\
      pat += 8 * bpp * 4;						\
    }									\
    M2I(src);								\
    acm->src = src;							\
  }									\
  M2I(pat);								\
  acm->pattern = pat;							\
  M2I(dst);								\
  acm->dst = dst;							\
  acm->control = mod;							\
  pat = w | (h << 16);							\
  M2I(pat);								\
  acm->dimension = pat;							\
  acm->start = 0;							\
  acm->start = 1;							\
}
#else
#define RZ3BlitFB2FB8(acm,rop,sx,sy,dx,dy,w,h,fbw) \
{ Ushort fw = fbw;							\
  Uint dst = (dx + dy * fw)<<3;					\
  Ushort mod = 0xc0c2;/* RIGHT,DOWN,SRC=fb,DST=fb,static-pattern */	\
  /*Ushort mod = 0xc0c0;*//* RIGHT,DOWN,SRC=fb,DST=fb */			\
  Uint pat = PAT_MEM_OFF << 3;						\
  __dolog("FB2FB: rop=%d,sx=%d,sy=%d,dx=%d,dy=%d,w=%d,h=%d,fw=%d,bpp=%d\n", \
	  rop,sx,sy,dx,dy,w,h,fw); \
  if (optabs[rop]) {							\
    Uint src = (sx + sy * fw) << 3;				\
    if (dx > sx) {							\
      mod &= ~0x8000; /* LEFT */					\
      src += (w - 1) << 3;						\
      dst += (w - 1) << 3;						\
      pat += 2 << 3;						\
    }									\
    if (dy > sy) {							\
      mod &= ~0x4000; /* UP */						\
      src += (h - 1) * fw << 3;					\
      dst += (h - 1) * fw << 3;					\
      pat += 4 << 3;						\
    }									\
    M2I(src);								\
    acm->src = src;							\
  }									\
  M2I(pat);								\
  acm->pattern = pat;							\
  M2I(dst);								\
  acm->dst = dst;							\
  acm->control = mod;							\
  pat = w | (h << 16);							\
  M2I(pat);								\
  acm->dimension = pat;							\
  acm->start = 0;							\
  acm->start = 1;							\
}

#define RZ3BlitFB2FB16(acm,rop,sx,sy,dx,dy,w,h,fbw) \
{ Ushort fw = fbw;							\
  Uint dst = (dx + dy * fw) << 4;					\
  Ushort mod = 0xc0c2;/* RIGHT,DOWN,SRC=fb,DST=fb,static-pattern */	\
  /*Ushort mod = 0xc0c0;*/ /* RIGHT,DOWN,SRC=fb,DST=fb */			\
  Uint pat = 8 * PAT_MEM_OFF;						\
  __dolog("FB2FB: rop=%d,sx=%d,sy=%d,dx=%d,dy=%d,w=%d,h=%d,fw=%d,bpp=%d\n", \
	  rop,sx,sy,dx,dy,w,h,fw); \
  if (optabs[rop]) {							\
    Uint src = (sx + sy * fw) << 4;				\
    if (dx > sx) {							\
      mod &= ~0x8000; /* LEFT */					\
      src += (w - 1) << 4;						\
      dst += (w - 1) << 4;						\
      pat += 2 << 4;						\
    }									\
    if (dy > sy) {							\
      mod &= ~0x4000; /* UP */						\
      src += (h - 1) * fw << 4;					\
      dst += (h - 1) * fw << 4;					\
      pat += 4 << 4;						\
    }									\
    M2I(src);								\
    acm->src = src;							\
  }									\
  M2I(pat);								\
  acm->pattern = pat;							\
  M2I(dst);								\
  acm->dst = dst;							\
  acm->control = mod;							\
  pat = w | (h << 16);							\
  M2I(pat);								\
  acm->dimension = pat;							\
  acm->start = 0;							\
  acm->start = 1;							\
}

#define RZ3BlitFB2FB24(acm,rop,sx,sy,dx,dy,w,h,fbw) \
{ Ushort fw = fbw;							\
  Uint dst = 24 * (dx + dy * fw);					\
  Ushort mod = 0xc0c2;/* RIGHT,DOWN,SRC=fb,DST=fb,static-pattern */	\
  /*Ushort mod = 0xc0c0;*/ /* RIGHT,DOWN,SRC=fb,DST=fb */			\
  Uint pat = 8 * PAT_MEM_OFF;						\
  __dolog("FB2FB: rop=%d,sx=%d,sy=%d,dx=%d,dy=%d,w=%d,h=%d,fw=%d,bpp=%d\n", \
	  rop,sx,sy,dx,dy,w,h,fw,bpp); \
  if (optabs[rop]) {							\
    Uint src = 24 * (sx + sy * fw);				\
    if (dx > sx) {							\
      mod &= ~0x8000; /* LEFT */					\
      src += 24 * (w - 1);						\
      dst += 24 * (w - 1);						\
      pat += 24 * 2;						\
    }									\
    if (dy > sy) {							\
      mod &= ~0x4000; /* UP */						\
      src += 24 * (h - 1) * fw;					\
      dst += 24 * (h - 1) * fw;					\
      pat += 24 * 4;						\
    }									\
    M2I(src);								\
    acm->src = src;							\
  }									\
  M2I(pat);								\
  acm->pattern = pat;							\
  M2I(dst);								\
  acm->dst = dst;							\
  acm->control = mod;							\
  pat = w | (h << 16);							\
  M2I(pat);								\
  acm->dimension = pat;							\
  acm->start = 0;							\
  acm->start = 1;							\
}

#endif

#define RZ3Blit1toFB(acm, rop, srcaddr, dx, dy, w, h, fbw, bpp, trans) \
{ Uint dst = 8 * bpp * (dx + dy * fbw);					\
  Ushort mod = 0xc06a; /* RIGHT,DOWN,SRC=M,DST=fb,ColExp,static-pattern */ \
  Uint pat = 8 * PAT_MEM_OFF;						\
  __dolog("1toFB: rop=%d,src=%d,dx=%d,dy=%d,w=%d,h=%d,fbw=%d,bpp=%d\n",	\
	  rop,srcaddr,dx,dy,w,h,fbw,bpp); 				\
  if (trans) mod |= 0x2000;						\
  if (optabs[rop]) {							\
    /* only alignment info is needed */					\
    Uint src = ((Uint)srcaddr) & 0x07;					\
    M2I(src);								\
    acm->src = src;							\
  }									\
  M2I(pat);								\
  acm->pattern = pat;							\
  M2I(dst);								\
  acm->dst = dst;							\
  acm->control = mod;							\
  pat = w | (h << 16);							\
  M2I(pat);								\
  acm->dimension = pat;							\
  acm->start = 0;							\
  acm->start = 1;							\
}

#define RZ3WaitDone(acm) \
{ while ((acm->status & 1) == 0) ; }

#if 0

#define GXWait(gx,r)\
    do\
	(r) = (int) (gx)->s; \
    while ((r) & GX_INPROGRESS)

#define GXDrawDone(gx,r) \
    do \
	(r) = (int) (gx)->draw; \
    while ((r) < 0 && ((r) & GX_FULL))

#define GXBlitDone(gx,r)\
    do\
	(r)= (int) (gx)->blit; \
    while ((r) < 0 && ((r) & GX_BLT_INPROGRESS))

#define GXBlitInit(gx,rop,pmsk) {\
    gx->fg = 0xff;\
    gx->bg = 0x00;\
    gx->pixelm = ~0;\
    gx->s = 0;\
    gx->alu = rop;\
    gx->pm = pmsk;\
    gx->clip = 0;\
}

#define GXDrawInit(gx,fore,rop,pmsk) {\
    gx->fg = fore;\
    gx->bg = 0x00; \
    gx->pixelm = ~0; \
    gx->s = 0; \
    gx->alu = rop; \
    gx->pm = pmsk; \
    gx->clip = 0;\
}

#define GXStippleInit(gx,stipple) {\
    int		_i; \
    Uint	*sp; \
    VUint	*dp; \
    _i = 8;  \
    sp = stipple->bits; \
    dp = gx->pattern; \
    while (_i--) {  \
	dp[_i] =  sp[_i]; \
    } \
    gx->fg = stipple->fore; \
    gx->bg = stipple->back; \
    gx->patalign = stipple->patalign; \
    gx->alu = stipple->alu; \
}

#endif

extern int  amigaGXScreenPrivateIndex;
extern int  amigaGXGCPrivateIndex;
extern int  amigaGXWindowPrivateIndex;

#define amigaGXGetScreenPrivate(s)    ((amigaGXPtr) \
			    (s)->devPrivates[amigaGXScreenPrivateIndex].ptr)

typedef struct _amigaGXStipple {
    Uint	fore, back;
    Uint	patalign;
    Uint	alu;
    Uint	bits[8];	/* actually 16 shorts */
} amigaGXStippleRec, *amigaGXStipplePtr;

typedef struct _amigaGXPrivGC {
    int		    type;
    amigaGXStipplePtr stipple;
} amigaGXPrivGCRec, *amigaGXPrivGCPtr;

#define amigaGXGetGCPrivate(g)	    ((amigaGXPrivGCPtr) \
			    (g)->devPrivates[amigaGXGCPrivateIndex].ptr)

#define amigaGXGetWindowPrivate(w)    ((amigaGXStipplePtr) \
			    (w)->devPrivates[amigaGXWindowPrivateIndex].ptr)

#define amigaGXSetWindowPrivate(w,p) (\
	    (w)->devPrivates[amigaGXWindowPrivateIndex].ptr = (pointer) p)


#define amigaInfo(s) (&amigaFbs[(s)->myNum])
