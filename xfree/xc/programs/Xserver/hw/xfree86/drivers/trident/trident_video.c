/*
 * Copyright 1992-2000 by Alan Hourihane, Wigan, England.
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
 * Author:  Alan Hourihane, alanh@fairlite.demon.co.uk
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/trident/trident_video.c,v 1.4.2.2 2001/05/24 09:37:41 alanh Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86fbman.h"
#include "regionstr.h"

#include "trident.h"
#include "trident_regs.h"
#include "Xv.h"
#include "xaa.h"
#include "xaalocal.h"
#include "dixstruct.h"
#include "fourcc.h"

#define OFF_DELAY 	200  /* milliseconds */
#define FREE_DELAY 	60000

#define OFF_TIMER 	0x01
#define FREE_TIMER	0x02
#define CLIENT_VIDEO_ON	0x04

#define TIMER_MASK      (OFF_TIMER | FREE_TIMER)

#ifndef XvExtension
void TRIDENTInitVideo(ScreenPtr pScreen) {}
void TRIDENTResetVideo(ScrnInfoPtr pScrn) {}
#else

static XF86VideoAdaptorPtr TRIDENTSetupImageVideo(ScreenPtr);
static void TRIDENTInitOffscreenImages(ScreenPtr);
static void TRIDENTStopVideo(ScrnInfoPtr, pointer, Bool);
static int TRIDENTSetPortAttribute(ScrnInfoPtr, Atom, INT32, pointer);
static int TRIDENTGetPortAttribute(ScrnInfoPtr, Atom ,INT32 *, pointer);
static void TRIDENTQueryBestSize(ScrnInfoPtr, Bool,
	short, short, short, short, unsigned int *, unsigned int *, pointer);
static int TRIDENTPutImage( ScrnInfoPtr, 
	short, short, short, short, short, short, short, short,
	int, unsigned char*, short, short, Bool, RegionPtr, pointer);
static int TRIDENTQueryImageAttributes(ScrnInfoPtr, 
	int, unsigned short *, unsigned short *,  int *, int *);
static void TRIDENTVideoTimerCallback(ScrnInfoPtr pScrn, Time time);

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

static Atom xvColorKey;

void TRIDENTInitVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    XF86VideoAdaptorPtr *adaptors, *newAdaptors = NULL;
    XF86VideoAdaptorPtr newAdaptor = NULL;
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    int num_adaptors;
	
    if (pTrident->NoAccel || !pTrident->AccelInfoRec->SetupForSolidFill)
	return;

    newAdaptor = TRIDENTSetupImageVideo(pScreen);
    TRIDENTInitOffscreenImages(pScreen);

    num_adaptors = xf86XVListGenericAdaptors(pScrn, &adaptors);

    if(newAdaptor) {
	if(!num_adaptors) {
	    num_adaptors = 1;
	    adaptors = &newAdaptor;
	} else {
	    newAdaptors =  /* need to free this someplace */
		xalloc((num_adaptors + 1) * sizeof(XF86VideoAdaptorPtr*));
	    if(newAdaptors) {
		memcpy(newAdaptors, adaptors, num_adaptors * 
					sizeof(XF86VideoAdaptorPtr));
		newAdaptors[num_adaptors] = newAdaptor;
		adaptors = newAdaptors;
		num_adaptors++;
	    }
	}
    }

    if(num_adaptors)
        xf86XVScreenInit(pScreen, adaptors, num_adaptors);

    if(newAdaptors)
	xfree(newAdaptors);
}

/* client libraries expect an encoding */
static XF86VideoEncodingRec DummyEncoding[1] =
{
 {
   0,
   "XV_IMAGE",
   1024, 1024,
   {1, 1}
 }
};

#define NUM_FORMATS 4

static XF86VideoFormatRec Formats[NUM_FORMATS] = 
{
  {8, PseudoColor},  {15, TrueColor}, {16, TrueColor}, {24, TrueColor}
};

#define NUM_ATTRIBUTES 1

static XF86AttributeRec Attributes[NUM_ATTRIBUTES] =
{
   {XvSettable | XvGettable, 0, (1 << 24) - 1, "XV_COLORKEY"}
};

#define NUM_IMAGES 4

static XF86ImageRec Images[NUM_IMAGES] =
{
   {
	0x35315652,
        XvRGB,
	LSBFirst,
	{'R','V','1','5',
	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	16,
	XvPacked,
	1,
	15, 0x001F, 0x03E0, 0x7C00,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	{'R','V','B',0,
	  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	XvTopToBottom
   },
   {
	0x36315652,
        XvRGB,
	LSBFirst,
	{'R','V','1','6',
	  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	16,
	XvPacked,
	1,
	16, 0x001F, 0x07E0, 0xF800,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	{'R','V','B',0,
	  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	XvTopToBottom
   },
   XVIMAGE_YV12,
   XVIMAGE_YUY2
};

typedef struct {
   FBLinearPtr	linear;
   RegionRec	clip;
   CARD32	colorKey;
   CARD32	videoStatus;
   Time		offTime;
   Time		freeTime;
} TRIDENTPortPrivRec, *TRIDENTPortPrivPtr;


#define GET_PORT_PRIVATE(pScrn) \
   (TRIDENTPortPrivPtr)((TRIDENTPTR(pScrn))->adaptor->pPortPrivates[0].ptr)

void TRIDENTResetVideo(ScrnInfoPtr pScrn) 
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    TRIDENTPortPrivPtr pPriv = pTrident->adaptor->pPortPrivates[0].ptr;
    int vgaIOBase = VGAHWPTR(pScrn)->IOBase;
    int red, green, blue;
    int tmp;
    
    OUTW(vgaIOBase + 4, 0x008E);
    OUTW(vgaIOBase + 4, 0x008F);

    OUTW(vgaIOBase + 4, 0x80B9); 

    OUTW(0x3C4, 0x3037);
    OUTW(0x3C4, 0xC057);
    OUTW(0x3C4, 0x3420);

    switch (pScrn->depth) {
    case 8:
	OUTW(0x3C4, (pPriv->colorKey<<8) | 0x50);
	OUTW(0x3C4, 0x0051);
	OUTW(0x3C4, 0x0052);
	OUTW(0x3C4, 0xFF54);
	OUTW(0x3C4, 0x0055);
	OUTW(0x3C4, 0x0056);
	break;
    default:
	red = (pPriv->colorKey & pScrn->mask.red) >> pScrn->offset.red;
	green = (pPriv->colorKey & pScrn->mask.green) >> pScrn->offset.green;
	blue = (pPriv->colorKey & pScrn->mask.blue) >> pScrn->offset.blue;
	switch (pScrn->depth) {
	case 15:
	    tmp = (red << 10) | (green << 5) | (blue);
	    OUTW(0x3C4, (tmp & 0xff) << 8 | 0x50);
	    OUTW(0x3C4, (tmp & 0xff00)    | 0x51);
	    OUTW(0x3C4, 0x0052);
	    OUTW(0x3C4, 0xFF54);
	    OUTW(0x3C4, 0xFF55);
	    OUTW(0x3C4, 0x0056);
	    break;
	case 16:
	    tmp = (red << 11) | (green << 5) | (blue);
	    OUTW(0x3C4, (tmp & 0xff) << 8 | 0x50);
	    OUTW(0x3C4, (tmp & 0xff00)    | 0x51);
	    OUTW(0x3C4, 0x0052);
	    OUTW(0x3C4, 0xFF54);
	    OUTW(0x3C4, 0xFF55);
	    OUTW(0x3C4, 0x0056);
	    break;
	case 24:
	    OUTW(0x3C4, (blue<<8)   | 0x50);
	    OUTW(0x3C4, (green<<8) | 0x51);
	    OUTW(0x3C4, (red<<8)  | 0x52);
	    OUTW(0x3C4, 0xFF54);
	    OUTW(0x3C4, 0xFF55);
	    OUTW(0x3C4, 0xFF56);
	    break;
	}    
    }    
}


static XF86VideoAdaptorPtr 
TRIDENTSetupImageVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    XF86VideoAdaptorPtr adapt;
    TRIDENTPortPrivPtr pPriv;

    if(!(adapt = xcalloc(1, sizeof(XF86VideoAdaptorRec) +
			    sizeof(TRIDENTPortPrivRec) +
			    sizeof(DevUnion))))
	return NULL;

    adapt->type = XvWindowMask | XvInputMask | XvImageMask;
    adapt->flags = VIDEO_OVERLAID_IMAGES | VIDEO_CLIP_TO_VIEWPORT;
    adapt->name = "Trident Backend Scaler";
    adapt->nEncodings = 1;
    adapt->pEncodings = DummyEncoding;
    adapt->nFormats = NUM_FORMATS;
    adapt->pFormats = Formats;
    adapt->nPorts = 1;
    adapt->pPortPrivates = (DevUnion*)(&adapt[1]);
    pPriv = (TRIDENTPortPrivPtr)(&adapt->pPortPrivates[1]);
    adapt->pPortPrivates[0].ptr = (pointer)(pPriv);
    adapt->pAttributes = Attributes;
    adapt->nImages = NUM_IMAGES;
    adapt->nAttributes = NUM_ATTRIBUTES;
    adapt->pImages = Images;
    adapt->PutVideo = NULL;
    adapt->PutStill = NULL;
    adapt->GetVideo = NULL;
    adapt->GetStill = NULL;
    adapt->StopVideo = TRIDENTStopVideo;
    adapt->SetPortAttribute = TRIDENTSetPortAttribute;
    adapt->GetPortAttribute = TRIDENTGetPortAttribute;
    adapt->QueryBestSize = TRIDENTQueryBestSize;
    adapt->PutImage = TRIDENTPutImage;
    adapt->QueryImageAttributes = TRIDENTQueryImageAttributes;

    pPriv->colorKey = pTrident->videoKey & ((1 << pScrn->depth) - 1);
    pPriv->videoStatus = 0;
    
    /* gotta uninit this someplace */
    REGION_INIT(pScreen, &pPriv->clip, NullBox, 0); 

    pTrident->adaptor = adapt;

    xvColorKey   = MAKE_ATOM("XV_COLORKEY");

    TRIDENTResetVideo(pScrn);

    return adapt;
}


static Bool
RegionsEqual(RegionPtr A, RegionPtr B)
{
    int *dataA, *dataB;
    int num;

    num = REGION_NUM_RECTS(A);
    if(num != REGION_NUM_RECTS(B))
	return FALSE;

    if((A->extents.x1 != B->extents.x1) ||
       (A->extents.x2 != B->extents.x2) ||
       (A->extents.y1 != B->extents.y1) ||
       (A->extents.y2 != B->extents.y2))
	return FALSE;

    dataA = (int*)REGION_RECTS(A);
    dataB = (int*)REGION_RECTS(B);

    while(num--) {
	if((dataA[0] != dataB[0]) || (dataA[1] != dataB[1]))
	   return FALSE;
	dataA += 2; 
	dataB += 2;
    }

    return TRUE;
}

/* TRIDENTClipVideo -  

   Takes the dst box in standard X BoxRec form (top and left
   edges inclusive, bottom and right exclusive).  The new dst
   box is returned.  The source boundaries are given (x1, y1 
   inclusive, x2, y2 exclusive) and returned are the new source 
   boundaries in 16.16 fixed point. 
*/

#define DummyScreen screenInfo.screens[0]

static Bool
TRIDENTClipVideo(
  BoxPtr dst, 
  INT32 *x1, 
  INT32 *x2, 
  INT32 *y1, 
  INT32 *y2,
  RegionPtr reg,
  INT32 width, 
  INT32 height
){
    INT32 vscale, hscale, delta;
    BoxPtr extents = REGION_EXTENTS(DummyScreen, reg);
    int diff;

    hscale = ((*x2 - *x1) << 16) / (dst->x2 - dst->x1);
    vscale = ((*y2 - *y1) << 16) / (dst->y2 - dst->y1);

    *x1 <<= 16; *x2 <<= 16;
    *y1 <<= 16; *y2 <<= 16;

    diff = extents->x1 - dst->x1;
    if(diff > 0) {
	dst->x1 = extents->x1;
	*x1 += diff * hscale;     
    }
    diff = dst->x2 - extents->x2;
    if(diff > 0) {
	dst->x2 = extents->x2;
	*x2 -= diff * hscale;     
    }
    diff = extents->y1 - dst->y1;
    if(diff > 0) {
	dst->y1 = extents->y1;
	*y1 += diff * vscale;     
    }
    diff = dst->y2 - extents->y2;
    if(diff > 0) {
	dst->y2 = extents->y2;
	*y2 -= diff * vscale;     
    }

    if(*x1 < 0) {
	diff =  (- *x1 + hscale - 1)/ hscale;
	dst->x1 += diff;
	*x1 += diff * hscale;
    }
    delta = *x2 - (width << 16);
    if(delta > 0) {
	diff = (delta + hscale - 1)/ hscale;
	dst->x2 -= diff;
	*x2 -= diff * hscale;
    }
    if(*x1 >= *x2) return FALSE;

    if(*y1 < 0) {
	diff =  (- *y1 + vscale - 1)/ vscale;
	dst->y1 += diff;
	*y1 += diff * vscale;
    }
    delta = *y2 - (height << 16);
    if(delta > 0) {
	diff = (delta + vscale - 1)/ vscale;
	dst->y2 -= diff;
	*y2 -= diff * vscale;
    }
    if(*y1 >= *y2) return FALSE;

    if((dst->x1 != extents->x1) || (dst->x2 != extents->x2) ||
       (dst->y1 != extents->y1) || (dst->y2 != extents->y2))
    {
	RegionRec clipReg;
	REGION_INIT(DummyScreen, &clipReg, dst, 1);
	REGION_INTERSECT(DummyScreen, reg, reg, &clipReg);
	REGION_UNINIT(DummyScreen, &clipReg);
    }
    return TRUE;
} 


static void 
TRIDENTStopVideo(ScrnInfoPtr pScrn, pointer data, Bool exit)
{
  TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
  TRIDENTPortPrivPtr pPriv = (TRIDENTPortPrivPtr)data;
  int vgaIOBase = VGAHWPTR(pScrn)->IOBase;

  REGION_EMPTY(pScrn->pScreen, &pPriv->clip);   

  if(exit) {
     if(pPriv->videoStatus & CLIENT_VIDEO_ON) {
	OUTW(vgaIOBase + 4, 0x008E);
	OUTW(vgaIOBase + 4, 0x008F);
     }
     if(pPriv->linear) {
	xf86FreeOffscreenLinear(pPriv->linear);
	pPriv->linear = NULL;
     }
     pPriv->videoStatus = 0;
  } else {
     if(pPriv->videoStatus & CLIENT_VIDEO_ON) {
	pPriv->videoStatus |= OFF_TIMER;
	pPriv->offTime = currentTime.milliseconds + OFF_DELAY; 
	pTrident->VideoTimerCallback = TRIDENTVideoTimerCallback;
     }
  }
}

static int 
TRIDENTSetPortAttribute(
  ScrnInfoPtr pScrn, 
  Atom attribute,
  INT32 value, 
  pointer data
){
  TRIDENTPortPrivPtr pPriv = (TRIDENTPortPrivPtr)data;
  TRIDENTPtr pTrident = TRIDENTPTR(pScrn);

  if(attribute == xvColorKey) {
	int red, green, blue;
	int tmp;
	pPriv->colorKey = value;
	switch (pScrn->depth) {
	case 8:
	    OUTW(0x3C4, (pPriv->colorKey<<8) | 0x50);
	    OUTW(0x3C4, 0x0051);
	    OUTW(0x3C4, 0x0052);
	    break;
	default:
	    red = (pPriv->colorKey & pScrn->mask.red) >> pScrn->offset.red;
	    green = (pPriv->colorKey & pScrn->mask.green) >> pScrn->offset.green;
	    blue = (pPriv->colorKey & pScrn->mask.blue) >> pScrn->offset.blue;
	    switch (pScrn->depth) {
	    case 15:
	    	tmp = (red << 10) | (green << 5) | (blue);
	    	OUTW(0x3C4, (tmp&0xff)<<8 | 0x50);
	    	OUTW(0x3C4, (tmp&0xff00)  | 0x51);
	    	OUTW(0x3C4, 0x0052);
		break;
	    case 16:
	    	tmp = (red << 11) | (green << 5) | (blue);
	    	OUTW(0x3C4, (tmp&0xff)<<8 | 0x50);
	    	OUTW(0x3C4, (tmp&0xff00)  | 0x51);
	    	OUTW(0x3C4, 0x0052);
		break;
	    case 24:
	    	OUTW(0x3C4, (blue<<8)   | 0x50);
	    	OUTW(0x3C4, (green<<8) | 0x51);
	    	OUTW(0x3C4, (red<<8)  | 0x52);
		break;
	    }    
	}    
	REGION_EMPTY(pScrn->pScreen, &pPriv->clip);   
  } else return BadMatch;

  return Success;
}

static int 
TRIDENTGetPortAttribute(
  ScrnInfoPtr pScrn, 
  Atom attribute,
  INT32 *value, 
  pointer data
){
  TRIDENTPortPrivPtr pPriv = (TRIDENTPortPrivPtr)data;

  if(attribute == xvColorKey) {
	*value = pPriv->colorKey;
  } else return BadMatch;

  return Success;
}

static void 
TRIDENTQueryBestSize(
  ScrnInfoPtr pScrn, 
  Bool motion,
  short vid_w, short vid_h, 
  short drw_w, short drw_h, 
  unsigned int *p_w, unsigned int *p_h, 
  pointer data
){
  *p_w = drw_w;
  *p_h = drw_h; 

  if(*p_w > 16384) *p_w = 16384;
}


static void
TRIDENTCopyData(
  unsigned char *src,
  unsigned char *dst,
  int srcPitch,
  int dstPitch,
  int h,
  int w
){
    w <<= 1;
    while(h--) {
	memcpy(dst, src, w);
	src += srcPitch;
	dst += dstPitch;
    }
}

static void
TRIDENTCopyMungedData(
   unsigned char *src1,
   unsigned char *src2,
   unsigned char *src3,
   unsigned char *dst1,
   int srcPitch,
   int srcPitch2,
   int dstPitch,
   int h,
   int w
){
   CARD32 *dst = (CARD32*)dst1;
   int i, j;

   dstPitch >>= 2;
   w >>= 1;

   for(j = 0; j < h; j++) {
	for(i = 0; i < w; i++) {
	    dst[i] = src1[i << 1] | (src1[(i << 1) + 1] << 16) |
		     (src3[i] << 8) | (src2[i] << 24);
	}
	dst += dstPitch;
	src1 += srcPitch;
	if(j & 1) {
	    src2 += srcPitch2;
	    src3 += srcPitch2;
	}
   }
}

static FBLinearPtr
TRIDENTAllocateMemory(
   ScrnInfoPtr pScrn,
   FBLinearPtr linear,
   int size
){
   ScreenPtr pScreen;
   FBLinearPtr new_linear;

   if(linear) {
	if(linear->size >= size) 
	   return linear;
        
        if(xf86ResizeOffscreenLinear(linear, size))
	   return linear;

	xf86FreeOffscreenLinear(linear);
   }

   pScreen = screenInfo.screens[pScrn->scrnIndex];

   new_linear = xf86AllocateOffscreenLinear(pScreen, size, 16, 
   						NULL, NULL, NULL);

   if(!new_linear) {
	int max_size;

	xf86QueryLargestOffscreenLinear(pScreen, &max_size, 16, 
						PRIORITY_EXTREME);
	
	if(max_size < size)
	   return NULL;

	xf86PurgeUnlockedOffscreenAreas(pScreen);
	new_linear = xf86AllocateOffscreenLinear(pScreen, size, 16, 
						NULL, NULL, NULL);
   }

   return new_linear;
}

static void
TRIDENTDisplayVideo(
    ScrnInfoPtr pScrn,
    int id,
    int offset,
    short width, short height,
    int pitch, 
    int x1, int y1, int x2, int y2,
    BoxPtr dstBox,
    short src_w, short src_h,
    short drw_w, short drw_h
){
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    int vgaIOBase = VGAHWPTR(pScrn)->IOBase;
    int zoomx1, zoomx2, zoomy1, zoomy2;
    int tx1,tx2;
    int ty1,ty2;

    switch(id) {
    case 0x35315652:		/* RGB15 */
    case 0x36315652:		/* RGB16 */
    	OUTW(vgaIOBase + 4, 0x22BF);
	break;
    case FOURCC_YV12:		/* YV12 */
    case FOURCC_YUY2:		/* YUY2 */
    default:
    	OUTW(vgaIOBase + 4, 0x00BF);
	break;
    }  
    tx1 = dstBox->x1 + pTrident->hsync;
    tx2 = dstBox->x2 + pTrident->hsync; 
    ty1 = dstBox->y1 + pTrident->vsync - 2;
    ty2 = dstBox->y2 + pTrident->vsync + 2;

    OUTW(vgaIOBase + 4, (tx1 & 0xff) <<8 | 0x86);
    OUTW(vgaIOBase + 4, (tx1 & 0xff00)   | 0x87);
    OUTW(vgaIOBase + 4, (ty1 & 0xff) <<8 | 0x88);
    OUTW(vgaIOBase + 4, (ty1 & 0xff00)   | 0x89);
    OUTW(vgaIOBase + 4, (tx2 & 0xff) <<8 | 0x8A);
    OUTW(vgaIOBase + 4, (tx2 & 0xff00)   | 0x8B);
    OUTW(vgaIOBase + 4, (ty2 & 0xff) <<8 | 0x8C);
    OUTW(vgaIOBase + 4, (ty2 & 0xff00)   | 0x8D);

    offset += (x1 >> 15) & ~0x01;

    OUTW(vgaIOBase + 4, (((width<<1) & 0xff)<<8)      | 0x90);
    OUTW(vgaIOBase + 4, ((width<<1) & 0xff00)         | 0x91);
    OUTW(vgaIOBase + 4, ((offset>>3) & 0xff) << 8     | 0x92);
    OUTW(vgaIOBase + 4, ((offset>>3) & 0xff00)        | 0x93);
    OUTW(vgaIOBase + 4, ((offset>>3) & 0xff0000) >> 8 | 0x94);

    /* Horizontal Zoom */
    if (drw_w == src_w) {
	OUTW(vgaIOBase + 4, 0x0080);
	OUTW(vgaIOBase + 4, 0x0081);
    } else
    if (drw_w > src_w) {
	float z;
	if (pTrident->Chipset >= BLADE3D)
	    z = (float)src_w/(float)drw_w;
	else
            z = (float)drw_w/(float)src_w - 1;
	zoomx1 =  z;
	zoomx2 = (z - (int)zoomx1 ) * 1024;
	OUTW(vgaIOBase + 4, (zoomx2&0xff)<<8 | 0x80);
	OUTW(vgaIOBase + 4, (zoomx1&0x0f)<<10 | (zoomx2&0x0300) | 0x81);
    } else {
	zoomx1 =   ((float)drw_w/(float)src_w);
	zoomx2 = ( ((float)drw_w/(float)src_w) - (int)zoomx1 ) * 1024;
	OUTW(vgaIOBase + 4, (zoomx2&0xff)<<8 |   0x80);
	OUTW(vgaIOBase + 4, (zoomx2&0x0300)|
			(((int)((float)src_w/(float)drw_w)-1)&7)<<10 | 0x8081);
    }

    /* Vertical Zoom */
    if (drw_h == src_h) {
	OUTW(vgaIOBase + 4, 0x0082);
	OUTW(vgaIOBase + 4, 0x0083);
    } else
    if (drw_h > src_h) {
	float z;
	if (pTrident->Chipset >= BLADE3D)
	    z = (float)src_h/(float)drw_h;
	else
	    z = (float)drw_h/(float)src_h - 1;
	zoomy1 =  z;
	zoomy2 = (z - (int)zoomy1 ) * 1024;
	OUTW(vgaIOBase + 4, (zoomy2&0xff)<<8 | 0x82);
	OUTW(vgaIOBase + 4, (zoomy1&0x0f)<<10 | (zoomy2&0x0300) | 0x83);
    } else {
	zoomy1 =   ((float)drw_h/(float)src_h);
	zoomy2 = ( ((float)drw_h/(float)src_h) - (int)zoomy1 ) * 1024;
	OUTW(vgaIOBase + 4, (zoomy2&0xff)<<8 | 0x82);
	OUTW(vgaIOBase + 4, (zoomy2&0x0300)|
			(((int)((float)src_h/(float)drw_h)-1)&7)<<10 | 0x8083);
    }

    OUTW(vgaIOBase + 4, 0x0895);
    OUTW(vgaIOBase + 4, ((width+2)>>2)<<8 | 0x96);

    OUTW(vgaIOBase + 4, 0x0097); 
    OUTW(vgaIOBase + 4, 0x00BA);
    OUTW(vgaIOBase + 4, 0x00BB);
    OUTW(vgaIOBase + 4, 0xFFBC);
    OUTW(vgaIOBase + 4, 0xFFBD);
    OUTW(vgaIOBase + 4, 0x04BE); 

    OUTW(vgaIOBase + 4, 0xD48E);
    OUTW(vgaIOBase + 4, 0x208F);
}

static int 
TRIDENTPutImage( 
  ScrnInfoPtr pScrn, 
  short src_x, short src_y, 
  short drw_x, short drw_y,
  short src_w, short src_h, 
  short drw_w, short drw_h,
  int id, unsigned char* buf, 
  short width, short height, 
  Bool sync,
  RegionPtr clipBoxes, pointer data
){
   TRIDENTPortPrivPtr pPriv = (TRIDENTPortPrivPtr)data;
   TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
   INT32 x1, x2, y1, y2;
   unsigned char *dst_start;
   int pitch, new_size, offset, offset2 = 0, offset3 = 0;
   int srcPitch, srcPitch2 = 0, dstPitch;
   int top, left, npixels, nlines, bpp;
   BoxRec dstBox;
   CARD32 tmp;

   /* Clip */
   x1 = src_x;
   x2 = src_x + src_w;
   y1 = src_y;
   y2 = src_y + src_h;

   dstBox.x1 = drw_x;
   dstBox.x2 = drw_x + drw_w;
   dstBox.y1 = drw_y;
   dstBox.y2 = drw_y + drw_h;

   if(!TRIDENTClipVideo(&dstBox, &x1, &x2, &y1, &y2, clipBoxes, width, height))
	return Success;

   dstBox.x1 -= pScrn->frameX0;
   dstBox.x2 -= pScrn->frameX0;
   dstBox.y1 -= pScrn->frameY0;
   dstBox.y2 -= pScrn->frameY0;

   bpp = pScrn->bitsPerPixel >> 3;
   pitch = bpp * pScrn->displayWidth;

   dstPitch = ((width << 1) + 15) & ~15;
   new_size = ((dstPitch * height) + bpp - 1) / bpp;
   switch(id) {
   case FOURCC_YV12:
   case FOURCC_I420:
	srcPitch = (width + 3) & ~3;
	offset2 = srcPitch * height;
	srcPitch2 = ((width >> 1) + 3) & ~3;
	offset3 = (srcPitch2 * (height >> 1)) + offset2;
	break;
   case FOURCC_UYVY:
   case FOURCC_YUY2:
   default:
	srcPitch = (width << 1);
	break;
   }  

   if(!(pPriv->linear = TRIDENTAllocateMemory(pScrn, pPriv->linear, new_size)))
	return BadAlloc;

    /* copy data */
   top = y1 >> 16;
   left = (x1 >> 16) & ~1;
   npixels = ((((x2 + 0xffff) >> 16) + 1) & ~1) - left;
   left <<= 1;

   offset = pPriv->linear->offset * bpp;
   dst_start = pTrident->FbBase + offset + left + (top * dstPitch);

   switch(id) {
    case FOURCC_YV12:
    case FOURCC_I420:
	top &= ~1;
	tmp = ((top >> 1) * srcPitch2) + (left >> 2);
	offset2 += tmp;
	offset3 += tmp;
	if(id == FOURCC_I420) {
	   tmp = offset2;
	   offset2 = offset3;
	   offset3 = tmp;
	}
	nlines = ((((y2 + 0xffff) >> 16) + 1) & ~1) - top;
	TRIDENTCopyMungedData(buf + (top * srcPitch) + (left >> 1), 
			  buf + offset2, buf + offset3, dst_start,
			  srcPitch, srcPitch2, dstPitch, nlines, npixels);
	break;
    case FOURCC_UYVY:
    case FOURCC_YUY2:
    default:
	buf += (top * srcPitch) + left;
	nlines = ((y2 + 0xffff) >> 16) - top;
	TRIDENTCopyData(buf, dst_start, srcPitch, dstPitch, nlines, npixels);
        break;
    }

    /* update cliplist */
    if(!RegionsEqual(&pPriv->clip, clipBoxes)) {
        REGION_COPY(pScreen, &pPriv->clip, clipBoxes);
        /* draw these */
        XAAFillSolidRects(pScrn, pPriv->colorKey, GXcopy, ~0, 
					REGION_NUM_RECTS(clipBoxes),
					REGION_RECTS(clipBoxes));
    }

    offset += top * dstPitch;
    TRIDENTDisplayVideo(pScrn, id, offset, width, height, dstPitch,
	     x1, y1, x2, y2, &dstBox, src_w, src_h, drw_w, drw_h);

    pPriv->videoStatus = CLIENT_VIDEO_ON;

    return Success;
}

static int 
TRIDENTQueryImageAttributes(
  ScrnInfoPtr pScrn, 
  int id, 
  unsigned short *w, unsigned short *h, 
  int *pitches, int *offsets
){
    int size, tmp;

    if(*w > 1024) *w = 1024;
    if(*h > 1024) *h = 1024;

    *w = (*w + 1) & ~1;
    if(offsets) offsets[0] = 0;

    switch(id) {
    case FOURCC_YV12:		/* YV12 */
	*h = (*h + 1) & ~1;
	size = (*w + 3) & ~3;
	if(pitches) pitches[0] = size;
	size *= *h;
	if(offsets) offsets[1] = size;
	tmp = ((*w >> 1) + 3) & ~3;
	if(pitches) pitches[1] = pitches[2] = tmp;
	tmp *= (*h >> 1);
	size += tmp;
	if(offsets) offsets[2] = size;
	size += tmp;
	break;
    default:			/* RGB15, RGB16, YUY2 */
	size = *w << 1;
	if(pitches) pitches[0] = size;
	size *= *h;
	break;
    }

    return size;
}

/****************** Offscreen stuff ***************/

typedef struct {
  FBLinearPtr linear;
  Bool isOn;
} OffscreenPrivRec, * OffscreenPrivPtr;

static int 
TRIDENTAllocateSurface(
    ScrnInfoPtr pScrn,
    int id,
    unsigned short w, 	
    unsigned short h,
    XF86SurfacePtr surface
){
    FBLinearPtr linear;
    int pitch, fbpitch, size, bpp;
    OffscreenPrivPtr pPriv;

    if((w > 1024) || (h > 1024))
	return BadAlloc;

    w = (w + 1) & ~1;
    pitch = ((w << 1) + 15) & ~15;
    bpp = pScrn->bitsPerPixel >> 3;
    fbpitch = bpp * pScrn->displayWidth;
    size = ((pitch * h) + bpp - 1) / bpp;

    if(!(linear = TRIDENTAllocateMemory(pScrn, NULL, size)))
	return BadAlloc;

    surface->width = w;
    surface->height = h;

    if(!(surface->pitches = xalloc(sizeof(int)))) {
	xf86FreeOffscreenLinear(linear);
	return BadAlloc;
    }
    if(!(surface->offsets = xalloc(sizeof(int)))) {
	xfree(surface->pitches);
	xf86FreeOffscreenLinear(linear);
	return BadAlloc;
    }
    if(!(pPriv = xalloc(sizeof(OffscreenPrivRec)))) {
	xfree(surface->pitches);
	xfree(surface->offsets);
	xf86FreeOffscreenLinear(linear);
	return BadAlloc;
    }

    pPriv->linear = linear;
    pPriv->isOn = FALSE;

    surface->pScrn = pScrn;
    surface->id = id;   
    surface->pitches[0] = pitch;
    surface->offsets[0] = linear->offset * bpp;
    surface->devPrivate.ptr = (pointer)pPriv;

    return Success;
}

static int 
TRIDENTStopSurface(
    XF86SurfacePtr surface
){
    OffscreenPrivPtr pPriv = (OffscreenPrivPtr)surface->devPrivate.ptr;

    if(pPriv->isOn) {
	TRIDENTPtr pTrident = TRIDENTPTR(surface->pScrn);
    	int vgaIOBase = VGAHWPTR(surface->pScrn)->IOBase;
	OUTW(vgaIOBase + 4, 0x008E);
	OUTW(vgaIOBase + 4, 0x008F);
	pPriv->isOn = FALSE;
    }

    return Success;
}


static int 
TRIDENTFreeSurface(
    XF86SurfacePtr surface
){
    OffscreenPrivPtr pPriv = (OffscreenPrivPtr)surface->devPrivate.ptr;

    if(pPriv->isOn)
	TRIDENTStopSurface(surface);
    xf86FreeOffscreenLinear(pPriv->linear);
    xfree(surface->pitches);
    xfree(surface->offsets);
    xfree(surface->devPrivate.ptr);

    return Success;
}

static int
TRIDENTGetSurfaceAttribute(
    ScrnInfoPtr pScrn,
    Atom attribute,
    INT32 *value
){
    return TRIDENTGetPortAttribute(pScrn, attribute, value, 
			(pointer)(GET_PORT_PRIVATE(pScrn)));
}

static int
TRIDENTSetSurfaceAttribute(
    ScrnInfoPtr pScrn,
    Atom attribute,
    INT32 value
){
    return TRIDENTSetPortAttribute(pScrn, attribute, value, 
			(pointer)(GET_PORT_PRIVATE(pScrn)));
}

static int 
TRIDENTDisplaySurface(
    XF86SurfacePtr surface,
    short src_x, short src_y, 
    short drw_x, short drw_y,
    short src_w, short src_h, 
    short drw_w, short drw_h,
    RegionPtr clipBoxes
){
    OffscreenPrivPtr pPriv = (OffscreenPrivPtr)surface->devPrivate.ptr;
    ScrnInfoPtr pScrn = surface->pScrn;
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    TRIDENTPortPrivPtr portPriv = pTrident->adaptor->pPortPrivates[0].ptr;
    INT32 x1, y1, x2, y2;
    BoxRec dstBox;

    x1 = src_x;
    x2 = src_x + src_w;
    y1 = src_y;
    y2 = src_y + src_h;

    dstBox.x1 = drw_x;
    dstBox.x2 = drw_x + drw_w;
    dstBox.y1 = drw_y;
    dstBox.y2 = drw_y + drw_h;

    if(!TRIDENTClipVideo(&dstBox, &x1, &x2, &y1, &y2, clipBoxes, 
			surface->width, surface->height))
    {
	return Success;
    }

    dstBox.x1 -= pScrn->frameX0;
    dstBox.x2 -= pScrn->frameX0;
    dstBox.y1 -= pScrn->frameY0;
    dstBox.y2 -= pScrn->frameY0;

    TRIDENTResetVideo(pScrn);

    TRIDENTDisplayVideo(pScrn, surface->id, surface->offsets[0], 
	     surface->width, surface->height, surface->pitches[0],
	     x1, y1, x2, y2, &dstBox, src_w, src_h, drw_w, drw_h);

    XAAFillSolidRects(pScrn, portPriv->colorKey, GXcopy, ~0, 
                                        REGION_NUM_RECTS(clipBoxes),
                                        REGION_RECTS(clipBoxes));

    pPriv->isOn = TRUE;
    /* we've prempted the XvImage stream so set its free timer */
    if(portPriv->videoStatus & CLIENT_VIDEO_ON) {
	REGION_EMPTY(pScrn->pScreen, &portPriv->clip);   
	UpdateCurrentTime();
	portPriv->videoStatus = FREE_TIMER;
	portPriv->freeTime = currentTime.milliseconds + FREE_DELAY;
	pTrident->VideoTimerCallback = TRIDENTVideoTimerCallback;
    }

    return Success;
}

static void 
TRIDENTInitOffscreenImages(ScreenPtr pScreen)
{
    XF86OffscreenImagePtr offscreenImages;

    /* need to free this someplace */
    if(!(offscreenImages = xalloc(sizeof(XF86OffscreenImageRec))))
	return;

    offscreenImages[0].image = &Images[0];
    offscreenImages[0].flags = VIDEO_OVERLAID_IMAGES | 
			       VIDEO_CLIP_TO_VIEWPORT;
    offscreenImages[0].alloc_surface = TRIDENTAllocateSurface;
    offscreenImages[0].free_surface = TRIDENTFreeSurface;
    offscreenImages[0].display = TRIDENTDisplaySurface;
    offscreenImages[0].stop = TRIDENTStopSurface;
    offscreenImages[0].setAttribute = TRIDENTSetSurfaceAttribute;
    offscreenImages[0].getAttribute = TRIDENTGetSurfaceAttribute;
    offscreenImages[0].max_width = 1024;
    offscreenImages[0].max_height = 1024;
    offscreenImages[0].num_attributes = NUM_ATTRIBUTES;
    offscreenImages[0].attributes = Attributes;
    
    xf86XVRegisterOffscreenImages(pScreen, offscreenImages, 1);
}

static void
TRIDENTVideoTimerCallback(ScrnInfoPtr pScrn, Time time)
{
    TRIDENTPtr pTrident = TRIDENTPTR(pScrn);
    TRIDENTPortPrivPtr pPriv = pTrident->adaptor->pPortPrivates[0].ptr;
    int vgaIOBase = VGAHWPTR(pScrn)->IOBase;

    if(pPriv->videoStatus & TIMER_MASK) {
	if(pPriv->videoStatus & OFF_TIMER) {
	    if(pPriv->offTime < time) {
		OUTW(vgaIOBase + 4, 0x008E);
		OUTW(vgaIOBase + 4, 0x008F);
		pPriv->videoStatus = FREE_TIMER;
		pPriv->freeTime = time + FREE_DELAY;
	    }
	} else {  /* FREE_TIMER */
	    if(pPriv->freeTime < time) {
		if(pPriv->linear) {
		   xf86FreeOffscreenLinear(pPriv->linear);
		   pPriv->linear = NULL;
		}
		pPriv->videoStatus = 0;
	        pTrident->VideoTimerCallback = NULL;
	    }
        }
    } else  /* shouldn't get here */
	pTrident->VideoTimerCallback = NULL;
}

#endif  /* !XvExtension */
