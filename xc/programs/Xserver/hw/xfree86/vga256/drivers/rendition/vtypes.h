/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/rendition/vtypes.h,v 1.1.2.4 1999/08/17 07:39:37 hohndel Exp $ */
#ifndef _VTYPES_H_
#define _VTYPES_H_



/*
 * includes
 */

#include <stdio.h>
#include <sys/types.h>
#include "Xmd.h"


/*
 * defines
 */

/* chip types */
#define V1000_DEVICE            0x0001
#define V2000_DEVICE            0x2000



/*
 * typedefs
 */

/* generic type definitions for central services */
typedef CARD32 vu32;
typedef CARD16 vu16;
typedef CARD8  vu8;
typedef INT32 vs32;
typedef INT16 vs16;
typedef INT8  vs8;
#ifdef __alpha__
typedef unsigned long vu64;
typedef          long vs64;
#endif

typedef enum {
  V_PIXFMT_DSTFMT=0,
  V_PIXFMT_332=1,       /**/
#define V_PIXFMT_233 V_PIXFMT_332
  V_PIXFMT_8I=2,        /**/
  V_PIXFMT_8A=3,
  V_PIXFMT_565=4,       /**/
  V_PIXFMT_4444=5,      /**/
  V_PIXFMT_1555=6,      /**/
  /* 7 reserved */
  V_PIXFMT_4I_565=8,
  V_PIXFMT_4I_4444=9,
  V_PIXFMT_4I_1555=10,
  /* 11 reserved */
  V_PIXFMT_8888=12,     /**/
  V_PIXFMT_Y0CRY1CB=13
#define V_PIXFMT_Y0CBY1CR V_PIXFMT_Y0CRY1CB
  /* 14 reserved */
  /* 15 reserved */
} vpixfmt;



/*
 * structs
 */

struct v_modeinfo_t {
  int clock;              /* pixel clock */
  int hdisplay;           /* horizontal timing */
  int hsyncstart;
  int hsyncend;
  int htotal;
  int hskew;
  int vdisplay;           /* vertical timing */
  int vsyncstart;
  int vsyncend;
  int vtotal;
  int screenwidth;        /* further mode information */
  int virtualwidth;
  int bitsperpixel;
  int hsynchi;
  int vsynchi;
  int pixelformat;        /* set by the mode init routines */
  int fifosize;
  vu8 pll_n;
  vu8 pll_m;
  vu8 pll_p;
  vu8 refresh;
  vu8 doubleclock;
};



/* structure describing the Verite board and its functionality */
struct v_board_t {
  /* type of chip */
  vu16  chip;

  /* */
  vu16 io_base;
  vu32 mmio_base;
  vu32 vmmio_base;
  vu32 mem_size;
  vu8 *mem_base;
  vu8 *vmem_base;
  vu8 init;

  /* */
  vu32 csucode_base;
  vu32 ucode_base;
  vu32 ucode_entry;
  vu32 cursor_base;

  /* mode information */
  struct v_modeinfo_t mode;

  /* saved text mode settings */
  vu8 cursor_hi;
  vu8 cursor_low;
  vu8 offset_hi;
  vu8 offset_low;
  vu8 *scr_contents;

  /* is the board initialized */
  int initialized;
};
  


#endif /* #ifndef _VTYPES_H_ */

/*
 * end of file vtypes.h
 */
