/***************************************************************************
 
Copyright 2000 Silicon Integrated Systems Corp, Inc., HsinChu, Taiwan.  
All Rights Reserved. 

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the 
"Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, 
distribute, sub license, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so, subject to 
the following conditions: 

The above copyright notice and this permission notice (including the 
next paragraph) shall be included in all copies or substantial portions 
of the Software. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. 
IN NO EVENT SHALL INTEL, AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_video.c,v 1.1 2000/12/02 01:16:19 dawes Exp $ */

/*
 * sis_video.c: SIS Xv driver. Based on the mga Xv driver by Mark Vojkovich
 *              and i810 Xv driver by Jonathan Bian <jonathan.bian@intel.com>.
 *
 * Authors: 
 *      Sung-Ching Lin <sclin@sis.com.tw>
 *
 * Notes:
 *
 */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "xf86PciInfo.h"
#include "xf86Pci.h"
#include "xf86fbman.h"
#include "regionstr.h"

#include "sis.h"
#include "xf86xv.h"
#include "Xv.h"
#include "xaa.h"
#include "xaalocal.h"
#include "dixstruct.h"

/* TODO: move to sis_regs.h */
#include "sis_vidregs.h"

#define OFF_DELAY 	200  /* milliseconds */
#define FREE_DELAY 	60000

#define OFF_TIMER 	0x01
#define FREE_TIMER	0x02
#define CLIENT_VIDEO_ON	0x04

#define TIMER_MASK      (OFF_TIMER | FREE_TIMER)

static XF86VideoAdaptorPtr SISSetupImageVideo(ScreenPtr);
static void SISStopVideo(ScrnInfoPtr, pointer, Bool);
static int SISSetPortAttribute(ScrnInfoPtr, Atom, INT32, pointer);
static int SISGetPortAttribute(ScrnInfoPtr, Atom ,INT32 *, pointer);
static void SISQueryBestSize(ScrnInfoPtr, Bool,
	short, short, short, short, unsigned int *, unsigned int *, pointer);
static int SISPutImage( ScrnInfoPtr, 
	short, short, short, short, short, short, short, short,
	int, unsigned char*, short, short, Bool, RegionPtr, pointer);
static int SISQueryImageAttributes(ScrnInfoPtr, 
	int, unsigned short *, unsigned short *,  int *, int *);

static void SISBlockHandler(int, pointer, pointer, pointer);

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

static Atom xvBrightness, xvContrast, xvColorKey;

#define IMAGE_MIN_WIDTH		32
#define IMAGE_MIN_HEIGHT	24
#define IMAGE_MAX_WIDTH		720
#define IMAGE_MAX_HEIGHT	576

#define DISPMODE_SINGLE1 0x1
#define DISPMODE_SINGLE2 0x2
#define DISPMODE_MIRROR	 0x4

/****************************************************************************
* raw register access : these routines directly interact with the sis's
*                       control aperature.  must not be called until after
*                       the board's pci memory has been mapped.
****************************************************************************/

static CARD32 _sisread(SISPtr pSIS, CARD32 reg)
{
    return *(pSIS->IOBase + reg);
}

static void _siswrite(SISPtr pSIS, CARD32 reg, CARD32 data)
{
    *(pSIS->IOBase + reg) = data;
}

static CARD8 getvideoreg(SISPtr pSIS, CARD8 reg)
{
	outb (pSIS->RelIO + vi_index_offset, reg);
	return inb(pSIS->RelIO + vi_data_offset);
}

static void setvideoreg(SISPtr pSIS, CARD8 reg, CARD8 data)
{
	outb (pSIS->RelIO + vi_index_offset, reg);
	outb (pSIS->RelIO + vi_data_offset, data);
}

static void setvideoregmask(SISPtr pSIS, CARD8 reg, CARD8 data, CARD8 mask)
{
	CARD8	old;

	outb (pSIS->RelIO + vi_index_offset, reg);
	old = inb(pSIS->RelIO + vi_data_offset);
	data = (data & mask) | (old & (~mask));
	outb (pSIS->RelIO + vi_data_offset, data);
}

static CARD8 getsrreg(SISPtr pSIS, CARD8 reg)
{
	outb (pSIS->RelIO + sr_index_offset, 0x05);
	if (inb (pSIS->RelIO + sr_data_offset) != 0xa1)
		outb (pSIS->RelIO + sr_data_offset, 0x86);
	outb (pSIS->RelIO + sr_index_offset, reg);
	return inb(pSIS->RelIO + sr_data_offset);
}

static void setsrreg(SISPtr pSIS, CARD8 reg, CARD8 data)
{
	outb (pSIS->RelIO + sr_index_offset, 0x05);
	if (inb (pSIS->RelIO + sr_data_offset) != 0xa1)
		outb (pSIS->RelIO + sr_data_offset, 0x86);
	outb (pSIS->RelIO + sr_index_offset, reg);
	outb (pSIS->RelIO + sr_data_offset, data);
}

static void setsrregmask(SISPtr pSIS, CARD8 reg, CARD8 data, CARD8 mask)
{
	CARD8	old;

	outb (pSIS->RelIO + sr_index_offset, 0x05);
	if (inb (pSIS->RelIO + sr_data_offset) != 0xa1)
		outb (pSIS->RelIO + sr_data_offset, 0x86);
	outb (pSIS->RelIO + sr_index_offset, reg);
	old = inb(pSIS->RelIO + sr_data_offset);
	data = (data & mask) | (old & (~mask));
	outb (pSIS->RelIO + sr_data_offset, data);
}

static CARD8 getsisreg(SISPtr pSIS, CARD8 index_offset, CARD8 reg)
{
	outb (pSIS->RelIO + index_offset, reg);
	return inb(pSIS->RelIO + index_offset+1);
}

static void setsisreg(SISPtr pSIS, CARD8 index_offset, CARD8 reg, CARD8 data)
{
	outb (pSIS->RelIO + index_offset, reg);
	outb (pSIS->RelIO + index_offset+1, data);
}

/* VBlank */
static CARD8 vblank_active_CRT1(SISPtr pSIS)
{
	return (inb(pSIS->RelIO + input_stat) & 0x08);
}

static CARD8 vblank_active_CRT2(SISPtr pSIS)
{
	return (getsisreg(pSIS, crt2_index_offset, Index_CRT2_FC_VR) & 0x02);
}

/* Scanline */
static CARD32 get_scanline_CRT1(SISPtr pSIS)
{
	CARD32 line;

	_siswrite (pSIS, REG_PRIM_CRT_COUNTER, 0x00000001);
	line = _sisread (pSIS, REG_PRIM_CRT_COUNTER);

	return ((line >> 16) & 0x07FF);
}

static CARD32 get_scanline_CRT2(SISPtr pSIS)
{
	CARD32 line;

	line = (CARD32)(getsisreg(pSIS, crt2_index_offset, Index_CRT2_FC_VCount1) & 0x70) * 16
				+ getsisreg(pSIS, crt2_index_offset, Index_CRT2_FC_VCount);

	return line;
}

void SISInitVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    XF86VideoAdaptorPtr *adaptors, *newAdaptors = NULL;
    XF86VideoAdaptorPtr newAdaptor = NULL;
    int num_adaptors;
	
    if (pScrn->bitsPerPixel != 8) 
    {
	newAdaptor = SISSetupImageVideo(pScreen);
    }

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
   IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT,
   {1, 1}
 }
};

#define NUM_FORMATS 2

static XF86VideoFormatRec Formats[NUM_FORMATS] = 
{
   {16, TrueColor}, {24, TrueColor}
};

#define NUM_ATTRIBUTES 3

static XF86AttributeRec Attributes[NUM_ATTRIBUTES] =
{
   {XvSettable | XvGettable, 0, (1 << 24) - 1, "XV_COLORKEY"},
   {XvSettable | XvGettable, -128, 127, "XV_BRIGHTNESS"},
   {XvSettable | XvGettable, 0, 255, "XV_CONTRAST"}
};

#define NUM_IMAGES 2
#define PIXEL_FMT_YV12 0x32315659
#define PIXEL_FMT_YUY2 0x32595559

static XF86ImageRec Images[NUM_IMAGES] =
{
   {
	PIXEL_FMT_YUY2,
        XvYUV,
	LSBFirst,
	{'Y','U','Y','2',
	  0x00,0x00,0x00,0x10,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71},
	16,
	XvPacked,
	1,
	0, 0, 0, 0 ,
	8, 8, 8, 
	1, 2, 2,
	1, 1, 1,
	{'Y','U','Y','V',
	  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	XvTopToBottom
   },
   {
	PIXEL_FMT_YV12,
        XvYUV,
	LSBFirst,
	{'Y','V','1','2',
	  0x00,0x00,0x00,0x10,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71},
	12,
	XvPlanar,
	3,
	0, 0, 0, 0 ,
	8, 8, 8, 
	1, 2, 2,
	1, 2, 2,
	{'Y','V','U',
	  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	XvTopToBottom
   }
};

typedef struct {
	int pixelFormat;

	CARD16	pitch;

	CARD8	keyOP;
	CARD16	HUSF;
	CARD16	VUSF;
	CARD8	IntBit;
	CARD8	wHPre;

	CARD16	srcW;
	CARD16	srcH;

	BoxRec	dstBox;

	CARD32	PSY;
	CARD32	PSV;
	CARD32	PSU;
	CARD8	bobEnable;

	CARD8	contrastCtrl;
	CARD8	contrastFactor;

        CARD8	lineBufSize;

	CARD8 (*VBlankActiveFunc)(SISPtr);
	CARD32 (*GetScanLineFunc)(SISPtr pSIS);
} SISOverlayRec, *SISOverlayPtr;

typedef struct {
	FBAreaPtr    fbAreaPtr;
	int          fbSize;
	CARD32       bufAddr[2];

	unsigned char currentBuf;

	short drw_x, drw_y, drw_w, drw_h;
	short src_x, src_y, src_w, src_h;	
	int id;
	short srcPitch, height;
	
	unsigned char brightness;
	unsigned char contrast;

	RegionRec    clip;
	CARD32       colorKey;

	CARD32       videoStatus;
	Time         offTime;
	Time         freeTime;

	CARD32 displayMode;
} SISPortPrivRec, *SISPortPrivPtr;        

#define GET_PORT_PRIVATE(pScrn) \
   (SISPortPrivPtr)((SISPTR(pScrn))->adaptor->pPortPrivates[0].ptr)


static void 
SISResetVideo(ScrnInfoPtr pScrn) 
{
    SISPtr pSIS = SISPTR(pScrn);

	if (getsrreg (pSIS, 0x05) != 0xa1)
	{
		setsrreg (pSIS, 0x05, 0x86);
		if (getsrreg (pSIS, 0x05) != 0xa1)
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
			           "Standard password not initialize\n");
	}
	if (getvideoreg (pSIS, Index_VI_Passwd) != 0xa1)
	{
		setvideoreg (pSIS, Index_VI_Passwd, 0x86);
		if (getvideoreg (pSIS, Index_VI_Passwd) != 0xa1)
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
			           "Video password not initialize\n");
	}

	/* Initial first set */
	setvideoregmask (pSIS, Index_VI_Control_Misc2, 0x80, 0x81);
	setvideoregmask(pSIS, Index_VI_Control_Misc0, 0x00, 0x02);
	setvideoregmask(pSIS, Index_VI_Control_Misc1, 0x02, 0x02);
	setvideoregmask(pSIS, Index_VI_Scale_Control, 0x60, 0x60);
	setvideoregmask(pSIS, Index_VI_Contrast_Enh_Ctrl, 0x04, 0x1F);
  
	setvideoreg(pSIS, Index_VI_Disp_Y_Buf_Preset_Low,     0x00);
	setvideoreg(pSIS, Index_VI_Disp_Y_Buf_Preset_Middle,  0x00);
	setvideoreg(pSIS, Index_VI_UV_Buf_Preset_Low,         0x00);
	setvideoreg(pSIS, Index_VI_UV_Buf_Preset_Middle,      0x00);
	setvideoreg(pSIS, Index_VI_Disp_Y_UV_Buf_Preset_High, 0x00);
	setvideoreg(pSIS, Index_VI_Play_Threshold_Low,        0x00);
	setvideoreg(pSIS, Index_VI_Play_Threshold_High,       0x00);

	/* Initial second set */
	setvideoregmask(pSIS, Index_VI_Control_Misc2, 0x81, 0x81);
	setvideoregmask(pSIS, Index_VI_Control_Misc0, 0x00, 0x02);
	setvideoregmask(pSIS, Index_VI_Control_Misc1, 0x02, 0x02);
	setvideoregmask(pSIS, Index_VI_Scale_Control, 0x60, 0x60);
	setvideoregmask(pSIS, Index_VI_Contrast_Enh_Ctrl, 0x04, 0x1F);

	setvideoreg(pSIS, Index_VI_Disp_Y_Buf_Preset_Low,     0x00);
	setvideoreg(pSIS, Index_VI_Disp_Y_Buf_Preset_Middle,  0x00);
	setvideoreg(pSIS, Index_VI_UV_Buf_Preset_Low,         0x00);
	setvideoreg(pSIS, Index_VI_UV_Buf_Preset_Middle,      0x00);
	setvideoreg(pSIS, Index_VI_Disp_Y_UV_Buf_Preset_High, 0x00);
	setvideoreg(pSIS, Index_VI_Play_Threshold_Low,        0x00);
	setvideoreg(pSIS, Index_VI_Play_Threshold_High,       0x00);
	
	/* set default contrast */
	setvideoregmask (pSIS, Index_VI_Control_Misc2, 0x00, 0x01);
	setvideoregmask (pSIS, Index_VI_Contrast_Enh_Ctrl, 0x04, 0x07);
	setvideoreg (pSIS, Index_VI_Brightness, 0x20);

	setvideoregmask (pSIS, Index_VI_Control_Misc2, 0x01, 0x01);
	setvideoregmask (pSIS, Index_VI_Contrast_Enh_Ctrl, 0x04, 0x07);
	setvideoreg (pSIS, Index_VI_Brightness, 0x20);

}


static XF86VideoAdaptorPtr 
SISSetupImageVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SISPtr pSIS = SISPTR(pScrn);
    XF86VideoAdaptorPtr adapt;
    SISPortPrivPtr pPriv;

    if(!(adapt = xcalloc(1, sizeof(XF86VideoAdaptorRec) +
			    sizeof(SISPortPrivRec) +
			    sizeof(DevUnion))))
	return NULL;

    adapt->type = XvWindowMask | XvInputMask | XvImageMask;
    adapt->flags = VIDEO_OVERLAID_IMAGES | VIDEO_CLIP_TO_VIEWPORT;
    adapt->name = "SIS Video Overlay";
    adapt->nEncodings = 1;
    adapt->pEncodings = DummyEncoding;
    adapt->nFormats = NUM_FORMATS;
    adapt->pFormats = Formats;
    adapt->nPorts = 1;
    adapt->pPortPrivates = (DevUnion*)(&adapt[1]);

    pPriv = (SISPortPrivPtr)(&adapt->pPortPrivates[1]);

    adapt->pPortPrivates[0].ptr = (pointer)(pPriv);
    adapt->pAttributes = Attributes;
    adapt->nImages = 2;
    adapt->nAttributes = 3;
    adapt->pImages = Images;
    adapt->PutVideo = NULL;
    adapt->PutStill = NULL;
    adapt->GetVideo = NULL;
    adapt->GetStill = NULL;
    adapt->StopVideo = SISStopVideo;
    adapt->SetPortAttribute = SISSetPortAttribute;
    adapt->GetPortAttribute = SISGetPortAttribute;
    adapt->QueryBestSize = SISQueryBestSize;
    adapt->PutImage = SISPutImage;
    adapt->QueryImageAttributes = SISQueryImageAttributes;

    pPriv->colorKey = 0x000101fe;
    pPriv->videoStatus = 0;
    pPriv->brightness = 0;
    pPriv->contrast = 128;

    pPriv->currentBuf = 0;
    
    pPriv->fbAreaPtr = NULL;
    pPriv->fbSize = 0;

    /* gotta uninit this someplace */
    REGION_INIT(pScreen, &pPriv->clip, NullBox, 0); 

    pSIS->adaptor = adapt;

    pSIS->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = SISBlockHandler;

    xvBrightness = MAKE_ATOM("XV_BRIGHTNESS");
    xvContrast   = MAKE_ATOM("XV_CONTRAST");
    xvColorKey   = MAKE_ATOM("XV_COLORKEY");

    /* set display mode */
    /* TODO: support CRT2-only mode */
    if(pSIS->VBFlags) {
      pPriv->displayMode = DISPMODE_MIRROR;
      setsrregmask (pSIS, 0x06, 0x80, 0xc0);
      setsrregmask (pSIS, 0x32, 0x80, 0xc0);
    }
    else {
      pPriv->displayMode = DISPMODE_SINGLE1;
      setsrregmask (pSIS, 0x06, 0x00, 0xc0);
      setsrregmask (pSIS, 0x32, 0x00, 0xc0);
    }

    SISResetVideo(pScrn);

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



static int 
SISSetPortAttribute(
  ScrnInfoPtr pScrn, 
  Atom attribute,
  INT32 value, 
  pointer data
){
  SISPortPrivPtr pPriv = (SISPortPrivPtr)data;

  if(attribute == xvBrightness) {
	if((value < -128) || (value > 127))
	   return BadValue;
	pPriv->brightness = value;
  } else
  if(attribute == xvContrast) {
	if((value < 0) || (value > 255))
	   return BadValue;
	pPriv->contrast = value;
  } else
  if(attribute == xvColorKey) {
	pPriv->colorKey = value;
	REGION_EMPTY(pScrn->pScreen, &pPriv->clip);   
  } else return BadMatch;

  return Success;
}

static int 
SISGetPortAttribute(
  ScrnInfoPtr pScrn, 
  Atom attribute,
  INT32 *value, 
  pointer data
){
  SISPortPrivPtr pPriv = (SISPortPrivPtr)data;

  if(attribute == xvBrightness) {
	*value = pPriv->brightness;
  } else
  if(attribute == xvContrast) {
	*value = pPriv->contrast;
  } else
  if(attribute == xvColorKey) {
	*value = pPriv->colorKey;
  } else return BadMatch;

  return Success;
}

static void 
SISQueryBestSize(
  ScrnInfoPtr pScrn, 
  Bool motion,
  short vid_w, short vid_h, 
  short drw_w, short drw_h, 
  unsigned int *p_w, unsigned int *p_h, 
  pointer data
){
  *p_w = drw_w;
  *p_h = drw_h; 

  /* TODO: report the HW limitation */
}


static void
set_scale_factor(SISOverlayPtr pOverlay)
{
  CARD32 I=0;
  
  int dstW = pOverlay->dstBox.x2 - pOverlay->dstBox.x1;
  int dstH = pOverlay->dstBox.y2 - pOverlay->dstBox.y1;  
  int srcW = pOverlay->srcW;
  int srcH = pOverlay->srcH;
  
  int srcPitch = pOverlay->pitch;

	if (dstW == srcW) {
		pOverlay->HUSF   = 0x00;
		pOverlay->IntBit = 0x05;
	}
	else if (dstW > srcW) {
		dstW   += 2;
		pOverlay->HUSF   = (srcW << 16) / dstW;
		pOverlay->IntBit = 0x04;
	} 
	else {
		int tmpW = dstW;

		I = 0x00;
		pOverlay->IntBit = 0x01;
		while (srcW >= tmpW)
		{
			tmpW <<= 1;
			I++;
		}
		pOverlay->wHPre = (CARD8)(I - 1);
		dstW <<= (I - 1);
		if ((srcW % dstW))
			pOverlay->HUSF = ((srcW - dstW) << 16) / dstW;
		else
			pOverlay->HUSF = 0x00;
	}

	if (dstH == srcH) {
		pOverlay->VUSF   = 0x00;
		pOverlay->IntBit |= 0x0A;
	}
	else if (dstH > srcH) {
		dstH += 0x02;
		pOverlay->VUSF = (srcH << 16) / dstH;
		pOverlay->IntBit |= 0x08;
	}
	else {
		CARD32 realI;

		I = realI = srcH / dstH;
		pOverlay->IntBit |= 0x02;

		if (I < 2)
		{
			pOverlay->VUSF = ((srcH - dstH)<<16)/dstH;
		}
		else
		{
#if 0
			if (((pOverlay->bobEnable & 0x08) == 0x00) &&
		    	(((srcPitch * I)>>2) > 0xFFF))
			{
				pOverlay->bobEnable |= 0x08;
				srcPitch >>= 1;
			}
#endif
			if (((srcPitch * I)>>2) > 0xFFF)
			{
				I = (0xFFF*2/srcPitch);
				pOverlay->VUSF = 0xFFFF;
			}
			else
			{
				dstH = I * dstH;
				if (srcH % dstH)
					pOverlay->VUSF = ((srcH - dstH) << 16) / dstH;
				else
					pOverlay->VUSF = 0x00;
			}
			/* set video frame buffer offset */
			pOverlay->pitch = (CARD16)(srcPitch*I);
		}
	}    
}


static void
set_line_buf_size(SISOverlayPtr pOverlay)
{
	CARD8	preHIDF;
	CARD32 I;
	CARD32 line = pOverlay->srcW;

	if (pOverlay->pixelFormat == PIXEL_FMT_YV12)
	{
		preHIDF = pOverlay->wHPre & 0x07;
		switch (preHIDF)
		{
			case 3 :
			    if ((line & 0xffffff00) == line)
				I = (line >> 8);
			    else
				I = (line >> 8) + 1;
			    pOverlay->lineBufSize = (CARD8)(I * 32 - 1);
			    break;
			case 4 :
			    if ((line & 0xfffffe00) == line)
				I = (line >> 9);
			    else
				I = (line >> 9) + 1;
			    pOverlay->lineBufSize = (CARD8)(I * 64 - 1);
			    break;
			case 5 :
			    if ((line & 0xfffffc00) == line)
				I = (line >> 10);
			    else
				I = (line >> 10) + 1;
			    pOverlay->lineBufSize = (CARD8)(I * 128 - 1);
			    break;
			case 6 :
			    if ((line & 0xfffff800) == line)
				I = (line >> 11);
			    else
				I = (line >> 11) + 1;
			    pOverlay->lineBufSize = (CARD8)(I * 256 - 1);
			    break;
			default :
			    if ((line & 0xffffff80) == line)
				I = (line >> 7);
			    else
				I = (line >> 7) + 1;
			    pOverlay->lineBufSize = (CARD8)(I * 16 - 1);
			    break;
		}
	}
	else
	{
		if ((line & 0xffffff8) == line)
			I = (line >> 3);
		else
			I = (line >> 3) + 1;
		pOverlay->lineBufSize = (CARD8)(I - 1);
	}
}

static void
merge_line_buf(SISPtr pSIS, SISPortPrivPtr pPriv, Bool enable)
{
  if(enable) {
    if(pPriv->displayMode == DISPMODE_MIRROR) { 
      setvideoregmask(pSIS, Index_VI_Control_Misc2, 0x00, 0x11);
      setvideoregmask(pSIS, Index_VI_Control_Misc1, 0x04, 0x04);
      setvideoregmask(pSIS, Index_VI_Control_Misc2, 0x01, 0x11);
      setvideoregmask(pSIS, Index_VI_Control_Misc1, 0x04, 0x04);
    }
    else {
      setvideoregmask(pSIS, Index_VI_Control_Misc2, 0x10, 0x11);
      setvideoregmask(pSIS, Index_VI_Control_Misc1, 0x00, 0x04);
      setvideoregmask(pSIS, Index_VI_Control_Misc2, 0x11, 0x11);
      setvideoregmask(pSIS, Index_VI_Control_Misc1, 0x00, 0x04);
    }
  }
  else {
    setvideoregmask(pSIS, Index_VI_Control_Misc2, 0x00, 0x11);    
    setvideoregmask(pSIS, Index_VI_Control_Misc1, 0x00, 0x04);
    setvideoregmask(pSIS, Index_VI_Control_Misc2, 0x01, 0x11);    
    setvideoregmask(pSIS, Index_VI_Control_Misc1, 0x00, 0x04);
  }
}


static void
set_format(SISPtr pSIS, SISOverlayPtr pOverlay)
{
	CARD8 fmt;

	switch (pOverlay->pixelFormat)
	{
	    case PIXEL_FMT_YV12:
		fmt = 0x0c;
		break;
	    case PIXEL_FMT_YUY2:
		fmt = 0x28;
		break;
	    default:
		fmt = 0x00;
		break;
	}
	setvideoregmask(pSIS, Index_VI_Control_Misc0, fmt, 0x7c);
}

static void
set_colorkey(SISPtr pSIS, CARD32 colorkey)
{
	CARD8 r, g, b;

	b = (CARD8)(colorkey & 0xFF);
	g = (CARD8)((colorkey>>8) & 0xFF);
	r = (CARD8)((colorkey>>16) & 0xFF);

        /* Activate the colorkey mode */
	setvideoreg(pSIS, Index_VI_Overlay_ColorKey_Blue_Min  ,(CARD8)b);
	setvideoreg(pSIS, Index_VI_Overlay_ColorKey_Green_Min ,(CARD8)g);
	setvideoreg(pSIS, Index_VI_Overlay_ColorKey_Red_Min   ,(CARD8)r);

	setvideoreg(pSIS, Index_VI_Overlay_ColorKey_Blue_Max  ,(CARD8)b);
	setvideoreg(pSIS, Index_VI_Overlay_ColorKey_Green_Max ,(CARD8)g);
	setvideoreg(pSIS, Index_VI_Overlay_ColorKey_Red_Max   ,(CARD8)r);
}


static void
set_brightness(SISPtr pSIS, CARD8 brightness)
{
	setvideoreg(pSIS, Index_VI_Brightness  ,brightness);
}


static void
set_overlay(SISPtr pSIS, SISOverlayPtr pOverlay)
{
	ScrnInfoPtr pScrn = pSIS->pScrn;

	CARD16 pitch=0;
	CARD8  h_over=0, v_over=0;
	CARD16 bottom, right;
	CARD16 screenX = pScrn->currentMode->HDisplay;
	CARD16 screenY = pScrn->currentMode->VDisplay;

	bottom = pOverlay->dstBox.y2;
	if (bottom > screenY)
		bottom = screenY;

	right = pOverlay->dstBox.x2;
	if (right > screenX)
		right = screenX;

	h_over = (((pOverlay->dstBox.x1>>8) & 0x0f) | ((right>>4) & 0xf0));
	v_over = (((pOverlay->dstBox.y1>>8) & 0x0f) | ((bottom>>4) & 0xf0));

	pitch = pOverlay->pitch;

	/* set line buffer size */
	setvideoreg(pSIS, Index_VI_Line_Buffer_Size, pOverlay->lineBufSize);
	
	setvideoregmask (pSIS, Index_VI_Key_Overlay_OP, pOverlay->keyOP, 0x0f);

	while (pOverlay->VBlankActiveFunc(pSIS));
	while (!pOverlay->VBlankActiveFunc(pSIS));
	
	setvideoreg (pSIS, Index_VI_Disp_Y_Buf_Pitch_Low, (CARD8)(pitch>>2));
	setvideoregmask (pSIS, Index_VI_Disp_Y_UV_Buf_Pitch_High, (CARD8)(pitch >> 10), 0x0f);

	setvideoregmask (pSIS, Index_VI_Control_Misc1, 0x20, 0x20);
	if (pOverlay->pixelFormat == PIXEL_FMT_YV12)
	{
		CARD32	PSU=0, PSV=0;

		PSU = pOverlay->PSU;
		PSV = pOverlay->PSV;

		setvideoreg (pSIS, Index_VI_Disp_UV_Buf_Pitch_Low, (CARD8)(pitch >> 3));
		setvideoregmask (pSIS, Index_VI_Disp_Y_UV_Buf_Pitch_High, (CARD8)(pitch >> 7), 0xf0);
		/* set U/V start address */
		setvideoreg (pSIS, Index_VI_U_Buf_Start_Low,   (CARD8)PSU);
		setvideoreg (pSIS, Index_VI_U_Buf_Start_Middle,(CARD8)(PSU>>8));
		setvideoreg (pSIS, Index_VI_U_Buf_Start_High,  (CARD8)(PSU>>16));

		setvideoreg (pSIS, Index_VI_V_Buf_Start_Low,   (CARD8)PSV);
		setvideoreg (pSIS, Index_VI_V_Buf_Start_Middle,(CARD8)(PSV>>8));
		setvideoreg (pSIS, Index_VI_V_Buf_Start_High,  (CARD8)(PSV>>16));
	}
	/* set scale factor */
	setvideoreg (pSIS, Index_VI_Hor_Post_Up_Scale_Low, (CARD8)(pOverlay->HUSF));
	setvideoreg (pSIS, Index_VI_Hor_Post_Up_Scale_High,(CARD8)((pOverlay->HUSF)>>8));
	setvideoreg (pSIS, Index_VI_Ver_Up_Scale_Low,      (CARD8)(pOverlay->VUSF));
	setvideoreg (pSIS, Index_VI_Ver_Up_Scale_High,     (CARD8)((pOverlay->VUSF)>>8));

	setvideoregmask (pSIS, Index_VI_Scale_Control, (pOverlay->IntBit << 3)|(pOverlay->wHPre), 0x7f);

	/* set destination position */
	setvideoreg(pSIS, Index_VI_Win_Hor_Disp_Start_Low, (CARD8)pOverlay->dstBox.x1);
	setvideoreg(pSIS, Index_VI_Win_Hor_Disp_End_Low, (CARD8)right);
	setvideoreg(pSIS, Index_VI_Win_Hor_Over, (CARD8)h_over);

	setvideoreg(pSIS, Index_VI_Win_Ver_Disp_Start_Low, (CARD8)pOverlay->dstBox.y1);
	setvideoreg(pSIS, Index_VI_Win_Ver_Disp_End_Low, (CARD8)bottom);
	setvideoreg(pSIS, Index_VI_Win_Ver_Over, (CARD8)v_over);

	/* set display start address */
	setvideoreg (pSIS, Index_VI_Disp_Y_Buf_Start_Low, (CARD8)(pOverlay->PSY));
	setvideoreg (pSIS, Index_VI_Disp_Y_Buf_Start_Middle, (CARD8)((pOverlay->PSY)>>8));
	setvideoreg (pSIS, Index_VI_Disp_Y_Buf_Start_High, (CARD8)((pOverlay->PSY)>>16));
	setvideoregmask(pSIS, Index_VI_Control_Misc1, pOverlay->bobEnable, 0x1a);
	setvideoregmask (pSIS, Index_VI_Control_Misc1, 0x00, 0x20);
	
	/* set contrast factor */
/*
	setvideoregmask(pSIS, Index_VI_Contrast_Enh_Ctrl, pOverlay->contrastCtrl<<6, 0xc0);
	setvideoreg (pSIS, Index_VI_Contrast_Factor, pOverlay->contrastFactor);
*/
}


static void
close_overlay(SISPtr pSIS, SISPortPrivPtr pPriv)
{
  setvideoregmask (pSIS, Index_VI_Control_Misc2, 0, 0x01);
  setvideoregmask(pSIS, Index_VI_Control_Misc0, 0x00, 0x02);
  setvideoregmask (pSIS, Index_VI_Control_Misc2, 1, 0x01);
  setvideoregmask(pSIS, Index_VI_Control_Misc0, 0x00, 0x02);
}


static void
SISDisplayVideo(ScrnInfoPtr pScrn, SISPortPrivPtr pPriv)
{
   SISPtr pSIS = SISPTR(pScrn);
   
   short srcPitch = pPriv->srcPitch;
   short height = pPriv->height;
   SISOverlayRec overlay; 
   int srcOffsetX=0, srcOffsetY=0;
   int sx, sy;
   int index = 0;

   memset(&overlay, 0, sizeof(overlay));
   overlay.pixelFormat = pPriv->id;
   overlay.pitch = srcPitch;
   overlay.keyOP = 0x03;
   /* overlay.bobEnable = 0x02; */
   overlay.bobEnable = 0x00;
   
   overlay.dstBox.x1 = pPriv->drw_x - pScrn->frameX0;
   overlay.dstBox.x2 = pPriv->drw_x + pPriv->drw_w - pScrn->frameX0;
   overlay.dstBox.y1 = pPriv->drw_y - pScrn->frameY0;
   overlay.dstBox.y2 = pPriv->drw_y + pPriv->drw_h - pScrn->frameY0;

   /* FIXME: assume (x2 > x1), (y2 > y1) */
   if((overlay.dstBox.x2 < 0) || (overlay.dstBox.y2 < 0))
     return;

   if(overlay.dstBox.x1 < 0) {
     srcOffsetX = pPriv->src_w * (-overlay.dstBox.x1) / pPriv->drw_w;
     overlay.dstBox.x1 = 0;
   }
   if(overlay.dstBox.y1 < 0) {
     srcOffsetY = pPriv->src_h * (-overlay.dstBox.y1) / pPriv->drw_h;
     overlay.dstBox.y1 = 0;   
   }

   switch(pPriv->id){
     case PIXEL_FMT_YV12:
       sx = (pPriv->src_x + srcOffsetX) & ~7;
       sy = (pPriv->src_y + srcOffsetY) & ~1;
       overlay.PSY = pPriv->bufAddr[pPriv->currentBuf] + sx + sy*srcPitch;
       overlay.PSV = pPriv->bufAddr[pPriv->currentBuf] + height*srcPitch + ((sx + sy*srcPitch/2) >> 1);
       overlay.PSU = pPriv->bufAddr[pPriv->currentBuf] + height*srcPitch*5/4 + ((sx + sy*srcPitch/2) >> 1);
       overlay.PSY >>= 2;
       overlay.PSV >>= 2;
       overlay.PSU >>= 2;
       break;
     case PIXEL_FMT_YUY2:
     default:
       sx = (pPriv->src_x + srcOffsetX) & ~1;
       sy = (pPriv->src_y + srcOffsetY);
       overlay.PSY = (pPriv->bufAddr[pPriv->currentBuf] + sx*2 + sy*srcPitch) >> 2;
       break;      
   }

   /* FIXME: is it possible that srcW < 0 */
   overlay.srcW = pPriv->src_w - (sx - pPriv->src_x);
   overlay.srcH = pPriv->src_h - (sy - pPriv->src_y);

   /* merge line buffer */
   /* TODO: unnecessay to do it several times */ 
   merge_line_buf (pSIS, pPriv, (overlay.srcW > 384));

   /* set line buffer length */
   set_line_buf_size (&overlay);
  
   /* set scale factor */
   set_scale_factor (&overlay);

   if(pPriv->displayMode == DISPMODE_SINGLE2) {
     index = 1;
     overlay.VBlankActiveFunc = vblank_active_CRT2;
     overlay.GetScanLineFunc = get_scanline_CRT2;
   }
   else {
     index = 0;
     overlay.VBlankActiveFunc = vblank_active_CRT1;
     overlay.GetScanLineFunc = get_scanline_CRT1;   
   }

MIRROR:

   setvideoregmask (pSIS, Index_VI_Control_Misc2, index, 0x01);
   
   /* set scale temporarily */
   {
     int dstW = overlay.dstBox.x2 - overlay.dstBox.x1;
     int srcW = overlay.srcW;
     unsigned char i = 0;
     
     dstW <<= 1;
     while(srcW > dstW) {
       dstW <<= 1;
       i++;
     }   
     setvideoregmask (pSIS, Index_VI_Scale_Control, i, 0x07);       
   }
         
   /* set format */
   set_format(pSIS, &overlay);
      
   /* set color key */
   /* TODO: update only when colorkey changed */
   /* FIXME, is the RGB order correct? */
   set_colorkey(pSIS, pPriv->colorKey);

   /* set brightness */
   set_brightness(pSIS, pPriv->brightness);   
   
   /* set overlay */
   set_overlay(pSIS, &overlay);
   
   /* enable overlay */    
   setvideoregmask (pSIS, Index_VI_Control_Misc0, 0x02, 0x02);

   if((pPriv->displayMode == DISPMODE_MIRROR) && (index == 0)) {
     index = 1;
     overlay.VBlankActiveFunc = vblank_active_CRT2;
     overlay.GetScanLineFunc = get_scanline_CRT2;
     goto MIRROR;
   }
}


static void 
SISStopVideo(ScrnInfoPtr pScrn, pointer data, Bool exit)
{
  SISPortPrivPtr pPriv = (SISPortPrivPtr)data;
  SISPtr pSIS = SISPTR(pScrn);

  REGION_EMPTY(pScrn->pScreen, &pPriv->clip);   

  if(exit) {
     if(pPriv->videoStatus & CLIENT_VIDEO_ON) {
       close_overlay(pSIS, pPriv);
     }
     if(pPriv->fbAreaPtr) {
       xf86FreeOffscreenArea(pPriv->fbAreaPtr);
       pPriv->fbAreaPtr = NULL;
       pPriv->fbSize = 0;
     }
     pPriv->videoStatus = 0;
  } else {
     if(pPriv->videoStatus & CLIENT_VIDEO_ON) {
	pPriv->videoStatus |= OFF_TIMER;
	pPriv->offTime = currentTime.milliseconds + OFF_DELAY; 
	/* FIXME */
/*	SISDisplayVideo(pScrn, pPriv); */
     }
  }
}


static int 
SISPutImage( 
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
   SISPtr pSIS = SISPTR(pScrn);
   SISPortPrivPtr pPriv = (SISPortPrivPtr)data;

   int totalSize=0;
      
   pPriv->drw_x = drw_x;
   pPriv->drw_y = drw_y;
   pPriv->drw_w = drw_w;
   pPriv->drw_h = drw_h;
   pPriv->src_x = src_x;
   pPriv->src_y = src_y;
   pPriv->src_w = src_w;
   pPriv->src_h = src_h;
   pPriv->id = id;
   pPriv->height = height;

   switch(id){
     case PIXEL_FMT_YV12:
       pPriv->srcPitch = (width + 7) & ~7;
       totalSize = (pPriv->srcPitch * height * 3) >> 1;
       break;
     case PIXEL_FMT_YUY2:
     default:
       pPriv->srcPitch = (width*2 + 3) & ~3;
       totalSize = pPriv->srcPitch * height;
   }
   
    /* allocate memory */
    do {
      int lines, pitch, depth;
      BoxPtr pBox;
      
      if(totalSize == pPriv->fbSize)
        break;
      
      pPriv->fbSize = totalSize;

      /* TODO: use xf86AllocateOffscreenLinear is better */
      if(pPriv->fbAreaPtr) {
        /* TODO: resize */     
	xf86FreeOffscreenArea(pPriv->fbAreaPtr);
      }
      depth = (pScrn->bitsPerPixel + 7 ) / 8;
      pitch = pScrn->displayWidth * depth;
      lines = ((totalSize * 2) / pitch) + 1;
      pPriv->fbAreaPtr = xf86AllocateOffscreenArea(pScrn->pScreen, 
                         pScrn->displayWidth,
  			 lines, 0, NULL, NULL, NULL);

      if(!pPriv->fbAreaPtr) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
                   "Allocate video memory fails\n");
        return BadAlloc;
      }
      pBox = &(pPriv->fbAreaPtr->box);     
      pPriv->bufAddr[0] = (pBox->x1 * depth) + (pBox->y1 * pitch); 
      pPriv->bufAddr[1] = pPriv->bufAddr[0] + totalSize;
    } while(0);

   /* copy data */
   /* TODO: subimage */
   memcpy(pSIS->FbBase + pPriv->bufAddr[pPriv->currentBuf], buf, totalSize);
         
   SISDisplayVideo(pScrn, pPriv);

    /* update cliplist */
    if(!RegionsEqual(&pPriv->clip, clipBoxes)) {
	REGION_COPY(pScreen, &pPriv->clip, clipBoxes);
	/* draw these */
	XAAFillSolidRects(pScrn, pPriv->colorKey, GXcopy, ~0, 
					REGION_NUM_RECTS(clipBoxes),
					REGION_RECTS(clipBoxes));
    }

    if (pPriv->currentBuf == 0)
	pPriv->currentBuf = 1;
    else
	pPriv->currentBuf = 0;
    
    pPriv->videoStatus = CLIENT_VIDEO_ON;

    return Success;
}


static int 
SISQueryImageAttributes(
  ScrnInfoPtr pScrn, 
  int id, 
  unsigned short *w, unsigned short *h, 
  int *pitches, int *offsets
){
    int pitchY, pitchUV;
    int size, sizeY, sizeUV;

    if(*w < IMAGE_MIN_WIDTH) *w = IMAGE_MIN_WIDTH;
    if(*h < IMAGE_MIN_HEIGHT) *h = IMAGE_MIN_HEIGHT;

    if(*w > IMAGE_MAX_WIDTH) *w = IMAGE_MAX_WIDTH;
    if(*h > IMAGE_MAX_HEIGHT) *h = IMAGE_MAX_HEIGHT;

    switch(id) {
    case PIXEL_FMT_YV12:
        *w = (*w + 7) & ~7;
        *h = (*h + 1) & ~1;
        pitchY = *w;
	pitchUV = *w >> 1;
	if(pitches) {
	  pitches[0] = pitchY;
	  pitches[1] = pitches[2] = pitchUV;
        }
	sizeY = pitchY * (*h);
	sizeUV = pitchUV * ((*h) >> 1);
	if(offsets) {	
          offsets[0] = 0;
          offsets[1] = sizeY;
          offsets[2] = sizeY + sizeUV;
        }
        size = sizeY + (sizeUV << 1);
	break;
    case PIXEL_FMT_YUY2:
    default:
        *w = (*w + 1) & ~1;
        pitchY = *w << 1;
	if(pitches) pitches[0] = pitchY;
	if(offsets) offsets[0] = 0;
	size = pitchY * (*h);
	break;
    }

    return size;
}

static void
SISBlockHandler (
    int i,
    pointer     blockData,
    pointer     pTimeout,
    pointer     pReadmask
){
    ScreenPtr   pScreen = screenInfo.screens[i];
    ScrnInfoPtr pScrn = xf86Screens[i];
    SISPtr      pSIS = SISPTR(pScrn);
    SISPortPrivPtr pPriv = GET_PORT_PRIVATE(pScrn);

    pScreen->BlockHandler = pSIS->BlockHandler;
    
    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);

    pScreen->BlockHandler = SISBlockHandler;

    if(pPriv->videoStatus & TIMER_MASK) {
	UpdateCurrentTime();
	if(pPriv->videoStatus & OFF_TIMER) {
	    if(pPriv->offTime < currentTime.milliseconds) {
		/* Turn off the overlay */
		close_overlay(pSIS, pPriv);
	
		pPriv->videoStatus = FREE_TIMER;
		pPriv->freeTime = currentTime.milliseconds + FREE_DELAY;
	    }
	} else {  /* FREE_TIMER */
	    if(pPriv->freeTime < currentTime.milliseconds) {
              if(pPriv->fbAreaPtr) {
                xf86FreeOffscreenArea(pPriv->fbAreaPtr);
                pPriv->fbAreaPtr = NULL;
                pPriv->fbSize = 0;
              }
	      pPriv->videoStatus = 0;
	    }
        }
    }
}
