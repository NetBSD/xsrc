 /***************************************************************************\
|*                                                                           *|
|*        Copyright (c) 1996-1998 NVIDIA, Corp.  All rights reserved.        *|
|*                                                                           *|
|*     NOTICE TO USER:   The source code  is copyrighted under  U.S. and     *|
|*     international laws.   NVIDIA, Corp. of Sunnyvale, California owns     *|
|*     the copyright  and as design patents  pending  on the design  and     *|
|*     interface  of the NV chips.   Users and possessors of this source     *|
|*     code are hereby granted  a nonexclusive,  royalty-free  copyright     *|
|*     and  design  patent license  to use this code  in individual  and     *|
|*     commercial software.                                                  *|
|*                                                                           *|
|*     Any use of this source code must include,  in the user documenta-     *|
|*     tion and  internal comments to the code,  notices to the end user     *|
|*     as follows:                                                           *|
|*                                                                           *|
|*     Copyright (c) 1996-1998  NVIDIA, Corp.    NVIDIA  design  patents     *|
|*     pending in the U.S. and foreign countries.                            *|
|*                                                                           *|
|*     NVIDIA, CORP.  MAKES  NO REPRESENTATION ABOUT  THE SUITABILITY OF     *|
|*     THIS SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT     *|
|*     EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA, CORP. DISCLAIMS     *|
|*     ALL WARRANTIES  WITH REGARD  TO THIS SOURCE CODE,  INCLUDING  ALL     *|
|*     IMPLIED   WARRANTIES  OF  MERCHANTABILITY  AND   FITNESS   FOR  A     *|
|*     PARTICULAR  PURPOSE.   IN NO EVENT SHALL NVIDIA, CORP.  BE LIABLE     *|
|*     FOR ANY SPECIAL, INDIRECT, INCIDENTAL,  OR CONSEQUENTIAL DAMAGES,     *|
|*     OR ANY DAMAGES  WHATSOEVER  RESULTING  FROM LOSS OF USE,  DATA OR     *|
|*     PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR OTHER     *|
|*     TORTIOUS ACTION, ARISING OUT  OF OR IN CONNECTION WITH THE USE OR     *|
|*     PERFORMANCE OF THIS SOURCE CODE.                                      *|
|*                                                                           *|
 \***************************************************************************/

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/nv/nvuser.h,v 1.1.2.5 1998/12/22 12:39:14 hohndel Exp $ */

#ifndef __NVUSER_H_
#define __NVUSER_H_

/* Basic types */
#ifndef XMD_H
typedef int INT32;
typedef short INT16;
#endif

typedef unsigned UINT32;
typedef unsigned short UINT16;



/*
 *  The first 256 bytes of each subchannel.
 */
typedef volatile struct {
 UINT32 object;              /* current object register         0000-0003*/
 UINT32 reserved01[0x003];
 UINT16 free;                /* free count, only readable reg.  0010-0011*/
 UINT16 reserved02[0x001];
 UINT32 reserved03[0x003];
 UINT32 unimp01[4]; /* Don't implement lunatic password stuff */
 UINT32 unimp02[2]; /* Don't implement push/pop channel state */
 UINT32 reserved04[0x032];
} NvControl;

typedef volatile struct {
 UINT32 unimp01;  /* Was SetNotifyCtxDma */
 UINT32 unimp02;  /* Was SetNotify */
 UINT32 reserved01[0x03e];
 UINT32 unimp03; /* Was SetRopOutput */
 UINT32 reserved02[0x03f];
 UINT32 setRop;            /* 8-bit index to std. MS Win ROPs 0300-0303*/
 UINT32 reserved03[0x73f];
} NvRopSolid;


typedef volatile struct {
 UINT32 unimp01; /* Was SetNotifyCtxDma */
 UINT32 unimp02; /* Was SetNotify */
 UINT32 reserved01[0x03e];
 UINT32 unimp03; /* SetImageOutput */
 UINT32 reserved02[0x03f];
 UINT32 unimp04; /* SetColorFormat */
 UINT32 unimp05; /* SetMonochromeFormat */
 UINT32 setPatternShape;         /* NV_PATTERN_SHAPE_{8X8,64X1,1X64}0308-030b*/
 UINT32 reserved03[0x001];
 UINT32 setColor0;               /* "background" color where pat=0  0310-0313*/
 UINT32 setColor1;               /* "foreground" color where pat=1  0314-0317*/
 struct {
   UINT32 monochrome[2];
 } setPattern;                 /* 64 bits of pattern data         0318-031f*/
 UINT32 reserved04[0x738];
} NvImagePattern;

/* values for NV_IMAGE_PATTERN SetPatternShape() */
#define NV_PATTERN_SHAPE_8X8   0
#define NV_PATTERN_SHAPE_64X1  1
#define NV_PATTERN_SHAPE_1X64  2


typedef volatile struct {
 UINT32 unimp01; /* Was SetNotifyCtxDma */
 UINT32 unimp02; /* SetNotify */
 UINT32 reserved01[0x03e];
 UINT32 unimp03; /* SetImageOutput */
 UINT32 reserved02[0x03f];
 struct {
  UINT32 yx;                      /* S16_S16 in pixels, 0 at top left 00-04*/
  UINT32 heightWidth;             /* U16_U16 in pixels                05-07*/
 } setRectangle;               /* region in image where alpha=1   0300-0307*/
 UINT32 reserved03[0x73e];
} NvClip,NvImageBlackRectangle;


typedef volatile struct {
 UINT32 unimp01; /* SetNotifyCtxDma */
 UINT32 unimp02; /* SetNotify */ 
 UINT32 reserved01[0x03e];
 UINT32 unimp03; /* SetImageOutput */ 
 UINT32 reserved02[0x03f];
 UINT32 unimp04; /* SetColorFormat */
 UINT32 color;                   /*                                 0304-0307*/
 UINT32 reserved03[0x03e];
 struct {
  UINT32 yx;                      /* S16_S16 in pixels, 0 at top left 00-03*/
  UINT32 heightWidth;             /* U16_U16 in pixels                04-07*/
 } rectangle[16];              /*                                 0400-047f*/
 UINT32 reserved04[0x6e0];
} NvRenderSolidRectangle;

/***** Image Rendering Classes *****/

/* class NV_IMAGE_BLIT */
#define NV_IMAGE_BLIT  31
typedef volatile struct
 tagNvImageBlit {
 UINT32 unimp01; /* SetNotifyCtxDma */
 UINT32 unimp02; /* SetNotify */
 UINT32 reserved01[0x03e];
 UINT32 unimp03; /* SetImageOutput */
 UINT32 unimp04; /* SetImageInput */
 UINT32 reserved02[0x03e];
 UINT32 yxIn;          /* S16_S16 in pixels, u.r. of src  300-0303*/
 UINT32 yxOut;         /* S16_16 in pixels, u.r. of dest  0304-0307*/
 UINT32 heightWidth;         /* U16_U16 in pixels               0308-030b*/
 UINT32 reserved03[0x73d];
} NvImageBlit;

#define NV_IMAGE_MONOCHROME_FROM_CPU  34
typedef volatile struct
 tagNvImageMonochromeFromCpu {
 UINT32 unimp01; /* SetNotifyCtxDma */
 UINT32 unimp02; /* SetNotify */
 UINT32 reserved01[0x03e];
 UINT32 unimp03; /* SetImage Output */
 UINT32 reserved02[0x03f];
 UINT32 unimp04; /* SetColorFormat */
 UINT32 unimp05; /* SetMonochromeFormat */
 UINT32 color0;
 UINT32 color1;
 UINT32 point;
 UINT32 size;
 UINT32 sizeIn;
 UINT32 reserved03[0x039];
 UINT32 monochrome[32];
 UINT32 reserved04[0x6e0];
} NvImageMonochromeFromCpu;


#define NV_RENDER_SOLID_LINE  27
typedef volatile struct {
 UINT32 unimp01;
 UINT32 unimp02;      
 UINT32 reserved01[0x03e];
 UINT32 unimp03;
 UINT32 reserved02[0x03f];
 UINT32 unimp04;
 UINT32 color;
 UINT32 reserved03[0x03e];
 struct {
  UINT32 y0_x0;
  UINT32 y1_x1;
 } line[16];
 struct {
  INT32 x0;
  INT32 y0;
  INT32 x1;
  INT32 y1;
 } line32[8];
 UINT32 polyLine[32];
 struct {
  INT32 x;
  INT32 y;
 } polyLine32[16];
 struct {
  UINT32 color;
  UINT32 y_x; 
 } colorPolyLine[16];
 UINT32 reserved04[0x660];
} NvRenderSolidLine;

#define NV_RENDER_GDI0_RECTANGLE_AND_TEXT  75
typedef volatile struct {
 UINT32 unimp01;
 UINT32 unimp02;
 UINT32 unimp03;
 UINT32 reserved01[0x03d];
 UINT32 unimp04;
 UINT32 reserved02[0x03f];
 UINT32 unimp05;
 UINT32 unimp06;
 UINT32 reserved03[(0x03e)-1];
 UINT32 Color1A;
 struct {
  UINT32 x_y;
  UINT32 width_height;
 } UnclippedRectangle[64];
 UINT32 reserved04[(0x080)-3];
 struct {
  UINT32 top_left;
  UINT32 bottom_right;
 } ClipB;
 UINT32 Color1B;
 struct {
  UINT32 top_left;
  UINT32 bottom_right;
 } ClippedRectangle[64];
 UINT32 reserved05[(0x080)-5];
 struct {
  UINT32 top_left;
  UINT32 bottom_right;
 } ClipC;
 UINT32 Color1C;
 UINT32 SizeC;
 UINT32 PointC;
 UINT32 MonochromeColor1C[128];
 UINT32 reserved06[(0x080)-6];
 struct {
  UINT32 top_left;
  UINT32 bottom_right;
 } ClipD;
 UINT32 Color1D;
 UINT32 SizeInD;
 UINT32 SizeOutD;
 UINT32 PointD;
 UINT32 MonochromeColor1D[128];
 UINT32 reserved07[(0x080)-7];
 struct {
  UINT32 top_left;
  UINT32 bottom_right;
 } ClipE;
 UINT32 Color0E;
 UINT32 Color1E;
 UINT32 SizeInE;
 UINT32 SizeOutE;
 UINT32 PointE;
 UINT32 MonochromeColor01E[128];
 UINT32 reserved08[0x280];
} NvRenderGdi0RectangleAndText;


typedef struct {
  NvControl control;
  union {
    NvRopSolid                   ropSolid;
    NvImagePattern               imagePattern;
    NvClip                       clip;
    NvRenderSolidRectangle       renderSolidRectangle;
    NvImageBlit                  blit;
    NvImageMonochromeFromCpu     imageMonochromeFromCpu;
    NvRenderSolidLine            line;
    NvRenderGdi0RectangleAndText glyphRectText;
  }method;
}NvSubChannel;

/* A channel consists of 8 subchannels */
typedef struct {
 NvSubChannel subChannel[8];
} NvChannel;



/* Used to load 16 bit values into a packed 32 value */
/* We must do the AND */
#define PACK_INT16(y,x) (((((UINT32)(y))<<16))| ((x)&0xffff))
/* For unsigned 16 bits we don't need to AND. Make sure the X coord
 * can never be negative or strange things will happen
 */
#define PACK_UINT16(y,x) ((((y)<<16))|(x))

#define MAX_UINT16 0xffff
#define MAX_INT16 0x7fff


/* These are the objects that are hard coded into the driver.
 * DON'T USE ANYTHING OTHER THAN THESE NUMBERS
 * YOU WILL LOCK THE GRAPHICS ENGINE UP 
 */

/* "Control" objects */
#define ROP_OBJECT_ID            0x99000000
#define CLIP_OBJECT_ID           0x99000001
#define PATTERN_OBJECT_ID        0x99000002

/* Rendering objects */
#define RECT_OBJECT_ID           0x88000000
#define BLIT_OBJECT_ID           0x88000001
#define COLOUR_EXPAND_OBJECT_ID  0x88000002
#define LINE_OBJECT_ID           0x88000003
#define LIN_OBJECT_ID            0x88000004
#define GLYPH_OBJECT_ID          0x88000005

/* Maps the user channel into the address space of the client */
/* NULL on failure */
NvChannel * NvOpenChannel(void);

/* Shuts down the channel */
void NvCloseChannel(void);

/* Checks to see if an interrupt has been raised */
void NvCheckForErrors(void);

/* How much ram is reserved by the graphics hardware. 
 * Returns amount used in KiloBytes
 */
int NvKbRamUsedByHW(void);

#endif
