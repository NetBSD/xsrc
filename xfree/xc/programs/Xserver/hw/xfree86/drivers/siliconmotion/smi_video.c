/* Header:   //Mercury/Projects/archives/XFree86/4.0/smi_video.c.-arc   1.14   30 Nov 2000 16:51:40   Frido  $ */

/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from the XFree86 Project and silicon Motion.
*/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/siliconmotion/smi_video.c,v 1.4 2001/03/03 22:26:13 tsi Exp $ */

#include "smi.h"
#include "smi_video.h"

#define nElems(x)		(sizeof(x) / sizeof(x[0]))
#define MAKE_ATOM(a)	MakeAtom(a, sizeof(a) - 1, TRUE)

#if defined(XvExtension) && SMI_USE_VIDEO

#include "dixstruct.h"
#include "xaa.h"
#include "xaalocal.h"

static XF86VideoAdaptorPtr SMI_SetupVideo(ScreenPtr pScreen);
static void SMI_ResetVideo(ScrnInfoPtr pScrn);

#if SMI_USE_CAPTURE
static int SMI_GetVideo(ScrnInfoPtr pScrn,
		short vid_x, short vid_y, short drw_x, short drw_y,
		short vid_w, short vid_h, short drw_w, short drw_h,
		RegionPtr clipBoxes, pointer data);
#endif
static void SMI_StopVideo(ScrnInfoPtr pScrn, pointer data, Bool exit);
static int SMI_SetPortAttribute(ScrnInfoPtr pScrn, Atom attribute,
		INT32 value, pointer data);
static int SMI_GetPortAttribute(ScrnInfoPtr pScrn, Atom attribute,
		INT32 *value, pointer data);
static void SMI_QueryBestSize(ScrnInfoPtr pScrn, Bool motion,
		short vid_w, short vid_h, short drw_w, short drw_h,
		unsigned int *p_w, unsigned int *p_h, pointer data);
static int SMI_PutImage(ScrnInfoPtr pScrn,
		short src_x, short src_y, short drw_x, short drw_y,
		short src_w, short src_h, short drw_w, short drw_h,
		int id, unsigned char *buf, short width, short height, Bool sync,
		RegionPtr clipBoxes, pointer data);
static int SMI_QueryImageAttributes(ScrnInfoPtr pScrn,
		int id, unsigned short *width, unsigned short *height,
		int *picthes, int *offsets);

static Bool RegionsEqual(RegionPtr A, RegionPtr B);
static Bool SMI_ClipVideo(ScrnInfoPtr pScrn, BoxPtr dst,
		INT32 *x1, INT32 *y1, INT32 *x2, INT32 *y2,
		RegionPtr reg, INT32 width, INT32 height);
static void SMI_DisplayVideo(ScrnInfoPtr pScrn, int id, int offset,
		short width, short height, int pitch, int x1, int y1, int x2, int y2,
		BoxPtr dstBox, short vid_w, short vid_h, short drw_w, short drw_h);
static void SMI_BlockHandler(int i, pointer blockData, pointer pTimeout,
		pointer pReadMask);
static void SMI_WaitForSync(ScrnInfoPtr pScrn);
static int SMI_SendI2C(ScrnInfoPtr pScrn, CARD8 device, char *devName,
		SMI_I2CDataPtr i2cData);

static void SMI_InitOffscreenImages(ScreenPtr pScreen);
static FBAreaPtr SMI_AllocateMemory(ScrnInfoPtr pScrn, FBAreaPtr area,
		int numLines);
static void SMI_CopyData(unsigned char *src, unsigned char *dst, int srcPitch,
		int dstPitch, int height, int width);
static void SMI_CopyYV12Data(unsigned char *src1, unsigned char *src2,
		unsigned char *src3, unsigned char *dst, int srcPitch1, int srcPitch2,
		int dstPitch, int height, int width);

static int SMI_AllocSurface(ScrnInfoPtr pScrn,
		int id, unsigned short width, unsigned short height,
		XF86SurfacePtr surface);
static int SMI_FreeSurface(XF86SurfacePtr surface);
static int SMI_DisplaySurface(XF86SurfacePtr surface,
		short vid_x, short vid_y, short drw_x, short drw_y,
		short vid_w, short vid_h, short drw_w, short drw_h,
		RegionPtr clipBoxes);
static int SMI_StopSurface(XF86SurfacePtr surface);
static int SMI_GetSurfaceAttribute(ScrnInfoPtr pScrn, Atom attr, INT32 *value);
static int SMI_SetSurfaceAttribute(ScrnInfoPtr pScrn, Atom attr, INT32 value);

static Atom xvColorKey, xvBrightness;

/******************************************************************************\
**																			  **
**					X V E X T E N S I O N   I N T E R F A C E				  **
**																			  **
\******************************************************************************/

void SMI_InitVideo(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	SMIPtr psmi = SMIPTR(pScrn);
	XF86VideoAdaptorPtr *ptrAdaptors, *newAdaptors = NULL;
	XF86VideoAdaptorPtr newAdaptor = NULL;
	int numAdaptors;

	ENTER_PROC("SMI_InitVideo");

	numAdaptors = xf86XVListGenericAdaptors(pScrn, &ptrAdaptors);

	if (   (psmi->rotate == 0)
		&& !psmi->NoAccel
	)
	{
		newAdaptor = SMI_SetupVideo(pScreen);
		SMI_InitOffscreenImages(pScreen);
	}

	if (newAdaptor != NULL)
	{
		if (numAdaptors == 0)
		{
			numAdaptors = 1;
			ptrAdaptors = &newAdaptor;
		}
		else
		{
			newAdaptors = xalloc((numAdaptors + 1) *
					sizeof(XF86VideoAdaptorPtr*));
			if (newAdaptors != NULL)
			{
				memcpy(newAdaptors, ptrAdaptors,
						numAdaptors * sizeof(XF86VideoAdaptorPtr));
				newAdaptors[numAdaptors++] = newAdaptor;
				ptrAdaptors = newAdaptors;
			}
		}
	}

	if (numAdaptors != 0)
	{
		xf86XVScreenInit(pScreen, ptrAdaptors, numAdaptors);
	}

	if (newAdaptors != NULL)
	{
		xfree(newAdaptors);
	}

	LEAVE_PROC("SMI_InitVideo");
}

/******************************************************************************\
**																			  **
**							 C A P A B I L I T I E S						  **
**																			  **
\******************************************************************************/

static XF86VideoEncodingRec SMI_VideoEncodings[] =
{
#if SMI_USE_CAPTURE
	{
		SMI_VIDEO_VIDEO,				/* id						*/
		"XV_VIDEO",						/* name						*/
		1024, 1024,						/* width, height			*/
		{1, 1}							/* rate						*/
	},
#endif
	{
		SMI_VIDEO_IMAGE,				/* id						*/
		"XV_IMAGE",						/* name						*/
		1024, 1024,						/* width, height			*/
		{1, 1}							/* rate						*/
	},
};

static XF86VideoFormatRec SMI_VideoFormats[] =
{
	{ 15, TrueColor },					/* depth, class				*/
	{ 16, TrueColor },					/* depth, class				*/
	{ 24, TrueColor },					/* depth, class				*/
};

static XF86AttributeRec SMI_VideoAttributes[] =
{
	{
		XvSettable | XvGettable,		/* flags					*/
		0x000000, 0xFFFFFF,				/* min_value, max_value		*/
		"XV_COLORKEY"					/* name						*/
	},
	{
		XvSettable | XvGettable,		/* flags					*/
		-128, 127,						/* min_value, max_value		*/
		"XV_BRIGHTNESS"					/* name						*/
	},
};

static XF86ImageRec SMI_VideoImages[] =
{
	XVIMAGE_YUY2,
	XVIMAGE_YV12,
	XVIMAGE_I420,
	{
		FOURCC_RV15,					/* id						*/
		XvRGB,							/* type						*/
		LSBFirst,						/* byte_order				*/
		{ 'R', 'V' ,'1', '5',
		  0x00, '5',  0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00 },		/* guid						*/
		16,								/* bits_per_pixel			*/
		XvPacked,						/* format					*/
		1,								/* num_planes				*/
		15,								/* depth					*/
		0x001F, 0x03E0, 0x7C00,			/* red_mask, green, blue	*/
		0, 0, 0,						/* y_sample_bits, u, v		*/
		0, 0, 0,						/* horz_y_period, u, v		*/
		0, 0, 0,						/* vert_y_period, u, v		*/
		{ 'R', 'V', 'B' },				/* component_order			*/
		XvTopToBottom					/* scaline_order			*/
	},
	{
		FOURCC_RV16,					/* id						*/
		XvRGB,							/* type						*/
		LSBFirst,						/* byte_order				*/
		{ 'R', 'V' ,'1', '6',
		  0x00, 0x00, 0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00 },		/* guid						*/
		16,								/* bits_per_pixel			*/
		XvPacked,						/* format					*/
		1,								/* num_planes				*/
		16,								/* depth					*/
		0x001F, 0x07E0, 0xF800,			/* red_mask, green, blue	*/
		0, 0, 0,						/* y_sample_bits, u, v		*/
		0, 0, 0,						/* horz_y_period, u, v		*/
		0, 0, 0,						/* vert_y_period, u, v		*/
		{ 'R', 'V', 'B' },				/* component_order			*/
		XvTopToBottom					/* scaline_order			*/
	},
	{
		FOURCC_RV24,					/* id						*/
		XvRGB,							/* type						*/
		LSBFirst,						/* byte_order				*/
		{ 'R', 'V' ,'2', '4',
		  0x00, 0x00, 0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00 },		/* guid						*/
		24,								/* bits_per_pixel			*/
		XvPacked,						/* format					*/
		1,								/* num_planes				*/
		24,								/* depth					*/
		0x0000FF, 0x00FF00, 0xFF0000,	/* red_mask, green, blue	*/
		0, 0, 0,						/* y_sample_bits, u, v		*/
		0, 0, 0,						/* horz_y_period, u, v		*/
		0, 0, 0,						/* vert_y_period, u, v		*/
		{ 'R', 'V', 'B' },				/* component_order			*/
		XvTopToBottom					/* scaline_order			*/
	},
	{
		FOURCC_RV32,					/* id						*/
		XvRGB,							/* type						*/
		LSBFirst,						/* byte_order				*/
		{ 'R', 'V' ,'3', '2',
		  0x00, 0x00, 0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00 },		/* guid						*/
		32,								/* bits_per_pixel			*/
		XvPacked,						/* format					*/
		1,								/* num_planes				*/
		24,								/* depth					*/
		0x0000FF, 0x00FF00, 0xFF0000,	/* red_mask, green, blue	*/
		0, 0, 0,						/* y_sample_bits, u, v		*/
		0, 0, 0,						/* horz_y_period, u, v		*/
		0, 0, 0,						/* vert_y_period, u, v		*/
		{ 'R', 'V', 'B' },				/* component_order			*/
		XvTopToBottom					/* scaline_order			*/
	},
};

SMI_I2CDataRec data_SAA7110[] =
{
	/* Configuration */
	{ 0x00, 0x4C },	{ 0x01, 0x3C }, { 0x02, 0x00 }, { 0x03, 0xEF },
	{ 0x04, 0xBD }, { 0x05, 0xE2 }, { 0x06, 0x00 }, { 0x07, 0x00 },
	{ 0x08, 0xF8 }, { 0x09, 0xF8 }, { 0x0A, 0x60 }, { 0x0B, 0x60 },
	{ 0x0C, 0x00 }, { 0x0D, 0x80 }, { 0x0E, 0x18 }, { 0x0F, 0xD9 },
	{ 0x10, 0x00 }, { 0x11, 0x2B }, { 0x12, 0x40 }, { 0x13, 0x40 },
	{ 0x14, 0x42 }, { 0x15, 0x1A }, { 0x16, 0xFF }, { 0x17, 0xDA },
	{ 0x18, 0xE6 }, { 0x19, 0x90 }, { 0x20, 0xD9 }, { 0x21, 0x16 },
	{ 0x22, 0x40 }, { 0x23, 0x40 }, { 0x24, 0x80 }, { 0x25, 0x40 },
	{ 0x26, 0x80 }, { 0x27, 0x4F }, { 0x28, 0xFE }, { 0x29, 0x01 },
	{ 0x2A, 0xCF }, { 0x2B, 0x0F }, { 0x2C, 0x03 }, { 0x2D, 0x01 },
	{ 0x2E, 0x83 }, { 0x2F, 0x03 }, { 0x30, 0x40 }, { 0x31, 0x35 },
	{ 0x32, 0x02 }, { 0x33, 0x8C }, { 0x34, 0x03 },

	/* NTSC */
	{ 0x11, 0x2B }, { 0x0F, 0xD9 },

	/* RCA input connector */
	{ 0x06, 0x00 }, { 0x0E, 0x18 }, { 0x20, 0xD9 }, { 0x21, 0x16 },
	{ 0x22, 0x40 }, { 0x2C, 0x03 },

	{ 0xFF, 0xFF }
};

/******************************************************************************\
**																			  **
**						 V I D E O   M A N A G E M E N T					  **
**																			  **
\******************************************************************************/

static XF86VideoAdaptorPtr
SMI_SetupVideo(
	ScreenPtr	pScreen
)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	SMIPtr pSmi = SMIPTR(pScrn);
	SMI_PortPtr smiPortPtr;
	XF86VideoAdaptorPtr ptrAdaptor;
	int i;

	ENTER_PROC("SMI_SetupVideo");

	ptrAdaptor = xcalloc(1, sizeof(XF86VideoAdaptorRec) +
			sizeof(DevUnion) + sizeof(SMI_PortRec));
	if (ptrAdaptor == NULL)
	{
		LEAVE_PROC("SMI_SetupVideo");
		return(NULL);
	}

	ptrAdaptor->type = XvInputMask
#if SMI_USE_CAPTURE
					 | XvOutputMask
					 | XvVideoMask
#endif
					 | XvImageMask
					 | XvWindowMask
					 ;

	ptrAdaptor->flags = VIDEO_OVERLAID_IMAGES
					  | VIDEO_CLIP_TO_VIEWPORT
					  ;

	ptrAdaptor->name = "Silicon Motion Lynx Series Video Engine";

	ptrAdaptor->nEncodings = nElems(SMI_VideoEncodings);
	ptrAdaptor->pEncodings = SMI_VideoEncodings;
	for (i = 0; i < nElems(SMI_VideoEncodings); i++)
	{
		SMI_VideoEncodings[i].width = pSmi->lcdWidth;
		SMI_VideoEncodings[i].height = pSmi->lcdHeight;
	}

	ptrAdaptor->nFormats = nElems(SMI_VideoFormats);
	ptrAdaptor->pFormats = SMI_VideoFormats;

	ptrAdaptor->nPorts = 1;
	ptrAdaptor->pPortPrivates = (DevUnion*) &ptrAdaptor[1];
	ptrAdaptor->pPortPrivates[0].ptr = (pointer) &ptrAdaptor->pPortPrivates[1];

	ptrAdaptor->nAttributes = nElems(SMI_VideoAttributes);
	ptrAdaptor->pAttributes = SMI_VideoAttributes;

	ptrAdaptor->nImages = nElems(SMI_VideoImages);
	ptrAdaptor->pImages = SMI_VideoImages;

#if SMI_USE_CAPTURE
	ptrAdaptor->PutVideo = NULL;
	ptrAdaptor->PutStill = NULL;
	ptrAdaptor->GetVideo = SMI_GetVideo;
	ptrAdaptor->GetStill = NULL;
#else
	ptrAdaptor->PutVideo = NULL;
	ptrAdaptor->PutStill = NULL;
	ptrAdaptor->GetVideo = NULL;
	ptrAdaptor->GetStill = NULL;
#endif
	ptrAdaptor->StopVideo = SMI_StopVideo;
	ptrAdaptor->SetPortAttribute = SMI_SetPortAttribute;
	ptrAdaptor->GetPortAttribute = SMI_GetPortAttribute;
	ptrAdaptor->QueryBestSize = SMI_QueryBestSize;
	ptrAdaptor->PutImage = SMI_PutImage;
	ptrAdaptor->QueryImageAttributes = SMI_QueryImageAttributes;

	smiPortPtr = (SMI_PortPtr) ptrAdaptor->pPortPrivates[0].ptr;
	smiPortPtr->colorKey = pSmi->videoKey;
	smiPortPtr->videoStatus = 0;
	smiPortPtr->brightness = 0;
	REGION_INIT(pScreen, &smiPortPtr->clip, NullBox, 0);

	pSmi->ptrAdaptor = ptrAdaptor;
	pSmi->BlockHandler = pScreen->BlockHandler;
	pScreen->BlockHandler = SMI_BlockHandler;

	xvBrightness = MAKE_ATOM("XV_BRIGHTNESS");
	xvColorKey = MAKE_ATOM("XV_COLORKEY");

	SMI_ResetVideo(pScrn);

	LEAVE_PROC("SMI_SetupVideo");
	return(ptrAdaptor);
}

static void
SMI_ResetVideo(
	ScrnInfoPtr	pScrn
)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	SMI_PortPtr ptrPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;
	int r, g, b;

	ENTER_PROC("SMI_ResetVideo");

	switch (pScrn->depth)
	{
		case 8:
			WRITE_VPR(pSmi, 0x04, ptrPort->colorKey & 0x00FF);
			WRITE_VPR(pSmi, 0x08, 0);
			break;

		case 15:
		case 16:
			WRITE_VPR(pSmi, 0x04, ptrPort->colorKey & 0xFFFF);
			WRITE_VPR(pSmi, 0x08, 0);
			break;

		default:
			r = (ptrPort->colorKey & pScrn->mask.red) >> pScrn->offset.red;
			g = (ptrPort->colorKey & pScrn->mask.green) >> pScrn->offset.green;
			b = (ptrPort->colorKey & pScrn->mask.blue) >> pScrn->offset.blue;
			WRITE_VPR(pSmi, 0x04, ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
			WRITE_VPR(pSmi, 0x08, 0);
			break;
	}

	WRITE_VPR(pSmi, 0x5C, 0xEDEDED | (ptrPort->brightness << 24));

	LEAVE_PROC("SMI_ResetVideo");
}

#if SMI_USE_CAPTURE
static int
SMI_GetVideo(
	ScrnInfoPtr	pScrn,
	short		vid_x,
	short		vid_y,
	short		drw_x,
	short		drw_y,
	short		vid_w,
	short		vid_h,
	short		drw_w,
	short		drw_h,
	RegionPtr	clipBoxes,
	pointer		data
)
{
	SMI_PortPtr ptrPort = (SMI_PortPtr) data;
	SMIPtr pSmi = SMIPTR(pScrn);
	CARD32 vid_pitch, vid_address;
	CARD32 vpr00, cpr00;
	int xscale, yscale;
	BoxRec dstBox;
	INT32 x1, y1, x2, y2;
	int areaHeight, width, height, fbPitch;
	int top, left;

	ENTER_PROC("SMI_GetVideo");

	x1 = vid_x;
	y1 = vid_y;
	x2 = vid_x + vid_w;
	y2 = vid_y + vid_h;

	width = vid_w;
	height = vid_h;

	dstBox.x1 = drw_x;
	dstBox.y1 = drw_y;
	dstBox.x2 = drw_x + drw_w;
	dstBox.y2 = drw_y + drw_h;

	if (!SMI_ClipVideo(pScrn, &dstBox, &x1, &y1, &x2, &y2, clipBoxes, width,
			height))
	{
		LEAVE_PROC("SMI_GetVideo");
		return(Success);
	}

	dstBox.x1 -= pScrn->frameX0;
	dstBox.y1 -= pScrn->frameY0;
	dstBox.x2 -= pScrn->frameX0;
	dstBox.y2 -= pScrn->frameY0;

	if (ptrPort->i2cDevice == 0)
	{
		if (SMI_SendI2C(pScrn, SAA7110, "SAA7110", data_SAA7110) == Success)
		{
			ptrPort->i2cDevice = SAA7110;
		}
		else
		{
			xf86FreeOffscreenArea(ptrPort->area);
			ptrPort->area = NULL;
			LEAVE_PROC("SMI_GetVideo");
			return(BadAlloc);
		}
	}

	vid_pitch = (vid_w * 2 + 7) & ~7;

	vpr00 = READ_VPR(pSmi, 0x00) & ~0x0FF000FF;
	cpr00 = READ_CPR(pSmi, 0x00) & ~0x000FFF00;

	vpr00 |= 0x0110000E;
	cpr00 |= 0x00000001;
	if (pSmi->ByteSwap)
		cpr00 |= 0x00004000;

	fbPitch = pSmi->Stride;
	if (pSmi->Bpp != 3)
	{
		fbPitch *= pSmi->Bpp;
	}

	if (vid_w <= drw_w)
	{
		xscale = (256 * vid_w / drw_w) & 0xFF;
	}
	else if (vid_w / 2 <= drw_w)
	{
		xscale = (128 * vid_w / drw_w) & 0xFF;
		width /= 2;
		vid_pitch /= 2;
		cpr00 |= 0x00010000;
	}
	else if (vid_w / 4 <= drw_w)
	{
		xscale = (64 * vid_w / drw_w) & 0xFF;
		width /= 4;
		vid_pitch /= 4;
		cpr00 |= 0x00020000;
	}
	else
	{
		xscale = 0;
		width /= 4;
		vid_pitch /= 4;
		cpr00 |= 0x00020000;
	}

	if (vid_h <= drw_h)
	{
		yscale = (256 * vid_h / drw_h) & 0xFF;
	}
	else if (vid_h / 2 <= drw_h)
	{
		yscale = (128 * vid_h / drw_h) & 0xFF;
		height /= 2;
		cpr00 |= 0x00040000;
	}
	else if (vid_h / 4 <= drw_h)
	{
		yscale = (64 * vid_h / drw_h) & 0xFF;
		height /= 4;
		cpr00 |= 0x00080000;
	}
	else
	{
		yscale = 0;
		height /= 4;
		cpr00 |= 0x00080000;
	}

	do
	{
		areaHeight = (vid_pitch * height + fbPitch - 1) / fbPitch;
		ptrPort->area = SMI_AllocateMemory(pScrn, ptrPort->area, areaHeight);
		if (ptrPort->area == NULL)
		{
			if ((cpr00 & 0x000C0000) == 0)
			{
				/* height -> 1/2 height */
				yscale = (128 * vid_h / drw_h) & 0xFF;
				height = vid_h / 2;
				cpr00 |= 0x00040000;
			}
			else if (cpr00 & 0x00040000)
			{
				/* 1/2 height -> 1/4 height */
				yscale = (64 * vid_h / drw_h) & 0xFF;
				height = vid_h / 4;
				cpr00 ^= 0x000C0000;
			}
			else
			{
				/* 1/4 height */
				if ((cpr00 & 0x00030000) == 0)
				{
					/* width -> 1/2 width */
					xscale = (128 * vid_w / drw_w) & 0xFF;
					width = vid_w / 2;
					cpr00 |= 0x00010000;
				}
				else if (cpr00 & 0x00010000)
				{
					/* 1/2 width -> 1/4 width */
					xscale = (64 * vid_w / drw_w) & 0xFF;
					width = vid_w / 4;
					cpr00 ^= 0x00030000;
				}
				else
				{
					LEAVE_PROC("SMI_GetVideo");
					return(BadAlloc);
				}
			}
		}
	}
	while (ptrPort->area == NULL);

	vid_address = (ptrPort->area->box.y1 * fbPitch) + ((y1 >> 16) * vid_pitch);

	if (!RegionsEqual(&ptrPort->clip, clipBoxes))
	{
		REGION_COPY(pScreen, &ptrPort->clip, clipBoxes);
		XAAFillSolidRects(pScrn, ptrPort->colorKey, GXcopy, ~0,
				REGION_NUM_RECTS(clipBoxes), REGION_RECTS(clipBoxes));
	}

	left = x1 >> 16;
	top = y1 >> 16;
	width = (x2 - x1) >> 16;
	height = (y2 - y1) >> 16;
	if (ptrPort->i2cDevice == SAA7110)
	{
		left += 79;
		top += 12;
	}

	OUT_SEQ(pSmi, 0x21, IN_SEQ(pSmi, 0x21) & ~0x04);
	WRITE_VPR(pSmi, 0x54, READ_VPR(pSmi, 0x54) | 0x00200000);

	SMI_WaitForSync(pScrn);

	WRITE_VPR(pSmi, 0x14, dstBox.x1 + (dstBox.y1 << 16));
	WRITE_VPR(pSmi, 0x18, dstBox.x2 + (dstBox.y2 << 16));
	WRITE_VPR(pSmi, 0x20, (vid_pitch / 8) + ((vid_pitch / 8) << 16));
	WRITE_VPR(pSmi, 0x24, (xscale << 8) + yscale);

	WRITE_CPR(pSmi, 0x04, left + (top << 16));
	WRITE_CPR(pSmi, 0x08, width + (height << 16));
	WRITE_CPR(pSmi, 0x0C, vid_address / 8);
	WRITE_CPR(pSmi, 0x10, vid_address / 8);
	WRITE_CPR(pSmi, 0x14, (vid_pitch / 8) + ((vid_pitch / 8) << 16));

	WRITE_CPR(pSmi, 0x00, cpr00);
	WRITE_VPR(pSmi, 0x00, vpr00);

	ptrPort->videoStatus = CLIENT_VIDEO_ON;
	LEAVE_PROC("SMI_GetVideo");
	return(Success);
}
#endif

static void
SMI_StopVideo(
	ScrnInfoPtr	pScrn,
	pointer		data,
	Bool		exit
)
{
	SMI_PortPtr ptrPort = (SMI_PortPtr) data;
	SMIPtr pSmi = SMIPTR(pScrn);

	ENTER_PROC("SMI_StopVideo");

	REGION_EMPTY(pScrn->pScreen, &ptrPort->clip);

	if (exit)
	{
		if (ptrPort->videoStatus & CLIENT_VIDEO_ON)
		{
			WRITE_VPR(pSmi, 0x00, READ_VPR(pSmi, 0x00) & ~0x01000008);
			#if SMI_USE_CAPTURE
			WRITE_CPR(pSmi, 0x00, READ_CPR(pSmi, 0x00) & ~0x00000001);
			WRITE_VPR(pSmi, 0x54, READ_VPR(pSmi, 0x54) & ~0x00F00000);
/* #864		OUT_SEQ(pSmi, 0x21, IN_SEQ(pSmi, 0x21) | 0x04); */
			#endif
		}
		if (ptrPort->area != NULL)
		{
			xf86FreeOffscreenArea(ptrPort->area);
			ptrPort->area = NULL;
		}
		ptrPort->videoStatus = 0;
		ptrPort->i2cDevice = 0;
	}
	else
	{
		if (ptrPort->videoStatus & CLIENT_VIDEO_ON)
		{
			ptrPort->videoStatus |= OFF_TIMER;
			ptrPort->offTime = currentTime.milliseconds + OFF_DELAY;
		}
	}

	LEAVE_PROC("SMI_StopVideo");
}

static int
SMI_SetPortAttribute(
	ScrnInfoPtr	pScrn,
	Atom		attribute,
	INT32		value,
	pointer		data
)
{
	SMI_PortPtr ptrPort = (SMI_PortPtr) data;
	SMIPtr pSmi = SMIPTR(pScrn);

	ENTER_PROC("SMI_SetPortAttribute");

	if (attribute == xvColorKey)
	{
		int r, g, b;

		ptrPort->colorKey = value;
		switch (pScrn->depth)
		{
			case 8:
				WRITE_VPR(pSmi, 0x04, value & 0x00FF);
				break;

			case 15:
			case 16:
				WRITE_VPR(pSmi, 0x04, value & 0xFFFF);
				break;

			default:
				r = (value & pScrn->mask.red) >> pScrn->offset.red;
				g = (value & pScrn->mask.green) >> pScrn->offset.green;
				b = (value & pScrn->mask.blue) >> pScrn->offset.blue;
				WRITE_VPR(pSmi, 0x04,
						((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
				break;
		}
	}

	else if (attribute == xvBrightness)
	{
		if ((value < -128) || (value > 127))
		{
			LEAVE_PROC("SMI_SetPortAttribute");
			return(BadValue);
		}
		ptrPort->brightness = value;
		WRITE_VPR(pSmi, 0x5C, 0xEDEDED | (value << 24));
	}

	else
	{
		LEAVE_PROC("SMI_SetPortAttribute");
		return(BadMatch);
	}

	LEAVE_PROC("SMI_SetPortAttribute");
	return(Success);
}

static int
SMI_GetPortAttribute(
	ScrnInfoPtr	pScrn,
	Atom		attribute,
	INT32		*value,
	pointer		data
)
{
	SMI_PortPtr ptrPort = (SMI_PortPtr) data;

	ENTER_PROC("SMI_GetPortAttribute");

	if (attribute == xvColorKey)
	{
		*value = ptrPort->colorKey;
	}

	else if (attribute == xvBrightness)
	{
		*value = ptrPort->brightness;
	}

	else
	{
		LEAVE_PROC("SMI_GetPortAttribute");
		return(BadMatch);
	}

	LEAVE_PROC("SMI_GetPortAttribute");
	return(Success);
}

static void
SMI_QueryBestSize(
	ScrnInfoPtr		pScrn,
	Bool			motion,
	short			vid_w,
	short			vid_h,
	short			drw_w,
	short			drw_h,
	unsigned int	*p_w,
	unsigned int	*p_h,
	pointer			data
)
{
	SMIPtr pSmi = SMIPTR(pScrn);

	ENTER_PROC("SMI_QueryBestSize");

	*p_w = min(drw_w, pSmi->lcdWidth);
	*p_h = min(drw_h, pSmi->lcdHeight);

	LEAVE_PROC("SMI_QueryBestSize");
}

static int
SMI_PutImage(
	ScrnInfoPtr		pScrn,
	short			src_x,
	short			src_y,
	short			drw_x,
	short			drw_y,
	short			src_w,
	short			src_h,
	short			drw_w,
	short			drw_h,
	int				id,
	unsigned char	*buf,
	short			width,
	short			height,
	Bool			sync,
	RegionPtr		clipBoxes,
	pointer			data
)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	SMI_PortPtr ptrPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;
	INT32 x1, y1, x2, y2;
	int bpp = 0;
	int fbPitch, srcPitch, srcPitch2 = 0, dstPitch, areaHeight;
	BoxRec dstBox;
	CARD32 offset, offset2 = 0, offset3 = 0, tmp;
	int left, top, nPixels, nLines;
	unsigned char *dstStart;

	ENTER_PROC("SMI_PutImage");

	x1 = src_x;
	y1 = src_y;
	x2 = src_x + src_w;
	y2 = src_y + src_h;

	dstBox.x1 = drw_x;
	dstBox.y1 = drw_y;
	dstBox.x2 = drw_x + drw_w;
	dstBox.y2 = drw_y + drw_h;

	if (!SMI_ClipVideo(pScrn, &dstBox, &x1, &y1, &x2, &y2, clipBoxes, width,
			height))
	{
		LEAVE_PROC("SMI_PutImage");
		return(Success);
	}

	dstBox.x1 -= pScrn->frameX0;
	dstBox.y1 -= pScrn->frameY0;
	dstBox.x2 -= pScrn->frameX0;
	dstBox.y2 -= pScrn->frameY0;

	if (pSmi->Bpp == 3)
	{
		fbPitch = pSmi->Stride;
	}
	else
	{
		fbPitch = pSmi->Stride * pSmi->Bpp;
	}

	switch (id)
	{
		case FOURCC_YV12:
			srcPitch  = (width + 3) & ~3;
			offset2   = srcPitch * height;
			srcPitch2 = ((width >> 1) + 3) & ~3;
			offset3   = offset2 + (srcPitch2 * (height >> 1));
			dstPitch  = ((width << 1) + 15) & ~15;
			break;

		case FOURCC_I420:
			srcPitch  = (width + 3) & ~3;
			offset3   = srcPitch * height;
			srcPitch2 = ((width >> 1) + 3) & ~3;
			offset2   = offset3 + (srcPitch2 * (height >> 1));
			dstPitch  = ((width << 1) + 15) & ~15;
			break;

		case FOURCC_RV24:
			bpp = 3;
			srcPitch = width * bpp;
			dstPitch = (srcPitch + 15) & ~15;
			break;

		case FOURCC_RV32:
			bpp = 4;
			srcPitch = width * bpp;
			dstPitch = (srcPitch + 15) & ~15;
			break;

		case FOURCC_YUY2:
		case FOURCC_RV15:
		case FOURCC_RV16:
		default:
			bpp = 2;
			srcPitch = width * bpp;
			dstPitch = (srcPitch + 15) & ~15;
			break;
	}

	areaHeight = ((dstPitch * height) + fbPitch - 1) / fbPitch;
	ptrPort->area = SMI_AllocateMemory(pScrn, ptrPort->area, areaHeight);
	if (ptrPort->area == NULL)
	{
		LEAVE_PROC("SMI_PutImage");
		return(BadAlloc);
	}

	top = y1 >> 16;
	left = (x1 >> 16) & ~1;
	nPixels = ((((x2 + 0xFFFF) >> 16) + 1) & ~1) - left;
	left *= bpp;

	offset = (ptrPort->area->box.y1 * fbPitch) + (top * dstPitch);
	dstStart = pSmi->FBBase + offset + left;

	switch (id)
	{
		case FOURCC_YV12:
		case FOURCC_I420:
			top &= ~1;
			tmp = ((top >> 1) * srcPitch2) + (left >> 2);
			offset2 += tmp;
			offset3 += tmp;
			nLines = ((((y2 + 0xFFFF) >> 16) + 1) & ~1) - top;
			SMI_CopyYV12Data(buf + (top * srcPitch) + (left >> 1),
					buf + offset2, buf + offset3, dstStart, srcPitch, srcPitch2,
					dstPitch, nLines, nPixels);
			break;

		default:
			buf += (top * srcPitch) + left;
			nLines = ((y2 + 0xFFFF) >> 16) - top;
			SMI_CopyData(buf, dstStart, srcPitch, dstPitch, nLines,
					nPixels * bpp);
			break;
	}

	if (!RegionsEqual(&ptrPort->clip, clipBoxes))
	{
		REGION_COPY(pScreen, &ptrPort->clip, clipBoxes);
		XAAFillSolidRects(pScrn, ptrPort->colorKey, GXcopy, ~0,
				REGION_NUM_RECTS(clipBoxes), REGION_RECTS(clipBoxes));
	}

	SMI_DisplayVideo(pScrn, id, offset, width, height, dstPitch, x1, y1, x2, y2,
			&dstBox, src_w, src_h, drw_w, drw_h);

	ptrPort->videoStatus = CLIENT_VIDEO_ON;
	LEAVE_PROC("SMI_PutImage");
	return(Success);
	
}

static int
SMI_QueryImageAttributes(
	ScrnInfoPtr		pScrn,
	int				id,
	unsigned short	*width,
	unsigned short	*height,
	int				*pitches,
	int				*offsets
)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	int size, tmp;

	ENTER_PROC("SMI_QueryImageAttributes");

	if (*width > pSmi->lcdWidth)
	{
		*width = pSmi->lcdWidth;
	}
	if (*height > pSmi->lcdHeight)
	{
		*height = pSmi->lcdHeight;
	}

	*width = (*width + 1) & ~1;
	if (offsets != NULL)
	{
		offsets[0] = 0;
	}

	switch (id)
	{
		case FOURCC_YV12:
		case FOURCC_I420:
			*height = (*height + 1) & ~1;
			size = (*width + 3) & ~3;
			if (pitches != NULL)
			{
				pitches[0] = size;
			}
			size *= *height;
			if (offsets != NULL)
			{
				offsets[1] = size;
			}
			tmp = ((*width >> 1) + 3) & ~3;
			if (pitches != NULL)
			{
				pitches[1] = pitches[2] = tmp;
			}
			tmp *= (*height >> 1);
			size += tmp;
			if (offsets != NULL)
			{
				offsets[2] = size;
			}
			size += tmp;
			break;

		case FOURCC_YUY2:
		case FOURCC_RV15:
		case FOURCC_RV16:
		default:
			size = *width * 2;
			if (pitches != NULL)
			{
				pitches[0] = size;
			}
			size *= *height;
			break;

		case FOURCC_RV24:
			size = *width * 3;
			if (pitches != NULL)
			{
				pitches[0] = size;
			}
			size *= *height;
			break;

		case FOURCC_RV32:
			size = *width * 4;
			if (pitches != NULL)
			{
				pitches[0] = size;
			}
			size *= *height;
			break;
	}

	LEAVE_PROC("SMI_QueryImageAttributes");
	return(size);
}

/******************************************************************************\
**																			  **
**						S U P P O R T   F U N C T I O N S					  **
**																			  **
\******************************************************************************/

static void
SMI_WaitForSync(
	ScrnInfoPtr	pScrn
)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	vgaHWPtr hwp = VGAHWPTR(pScrn);
	int vgaIOBase  = hwp->IOBase;
	int vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
	int vgaCRData  = vgaIOBase + VGA_CRTC_DATA_OFFSET;

	VerticalRetraceWait();
}

static Bool
RegionsEqual(
	RegionPtr	A,
	RegionPtr	B
)
{
	int *dataA, *dataB;
	int num;

	ENTER_PROC("RegionsEqual");

	num = REGION_NUM_RECTS(A);
	if (num != REGION_NUM_RECTS(B))
	{
		LEAVE_PROC("RegionsEqual");
		return(FALSE);
	}

	if (   (A->extents.x1 != B->extents.x1)
		|| (A->extents.y1 != B->extents.y1)
		|| (A->extents.x2 != B->extents.x2)
		|| (A->extents.y2 != B->extents.y2)
	)
	{
		LEAVE_PROC("RegionsEqual");
		return(FALSE);
	}

	dataA = (int*) REGION_RECTS(A);
	dataB = (int*) REGION_RECTS(B);

	while (num--)
	{
		if ((dataA[0] != dataB[0]) || (dataA[1] != dataB[1]))
		{
			return(FALSE);
		}
		dataA += 2;
		dataB += 2;
	}

	LEAVE_PROC("RegionsEqual");
	return(TRUE);
}

static Bool
SMI_ClipVideo(
	ScrnInfoPtr	pScrn,
	BoxPtr		dst,
	INT32		*x1,
	INT32		*y1,
	INT32		*x2,
	INT32		*y2,
	RegionPtr	reg,
	INT32		width,
	INT32		height
)
{
	INT32 vscale, hscale, delta;
	BoxPtr extents = REGION_EXTENTS(pScrn, reg);
	int diff;

	ENTER_PROC("SMI_ClipVideo");

	/* PDR#941 */
	extents->x1 = max(extents->x1, pScrn->frameX0);
	extents->y1 = max(extents->y1, pScrn->frameY0);

	hscale = ((*x2 - *x1) << 16) / (dst->x2 - dst->x1);
	vscale = ((*y2 - *y1) << 16) / (dst->y2 - dst->y1);

	*x1 <<= 16; *y1 <<= 16;
	*x2 <<= 16; *y2 <<= 16;

	diff = extents->x1 - dst->x1;
	if (diff > 0)
	{
		dst->x1 = extents->x1;
		*x1 += diff * hscale;
	}

	diff = extents->y1 - dst->y1;
	if (diff > 0)
	{
		dst->y1 = extents->y1;
		*y1 += diff * vscale;
	}

	diff = dst->x2 - extents->x2;
	if (diff > 0)
	{
		dst->x2 = extents->x2; /* PDR#687 */
		*x2 -= diff * hscale;
	}

	diff = dst->y2 - extents->y2;
	if (diff > 0)
	{
		dst->y2 = extents->y2;
		*y2 -= diff * vscale;
	}

	if (*x1 < 0)
	{
		diff = (-*x1 + hscale - 1) / hscale;
		dst->x1 += diff;
		*x1 += diff * hscale;
	}

	if (*y1 < 0)
	{
		diff = (-*y1 + vscale - 1) / vscale;
		dst->y1 += diff;
		*y1 += diff * vscale;
	}

	delta = *x2 - (width << 16);
	if (delta > 0)
	{
		diff = (delta + hscale - 1) / hscale;
		dst->x2 -= diff;
		*x2 -= diff * hscale;
	}

	delta = *y2 - (height << 16);
	if (delta > 0)
	{
		diff = (delta + vscale - 1) / vscale;
		dst->y2 -= diff;
		*y2 -= diff * vscale;
	}

	if ((*x1 >= *x2) || (*y1 >= *y2))
	{
		LEAVE_PROC("SMI_ClipVideo");
		return(FALSE);
	}

	if (   (dst->x1 != extents->x1) || (dst->y1 != extents->y1)
		|| (dst->x2 != extents->x2) || (dst->y2 != extents->y2)
	)
	{
		RegionRec clipReg;
		REGION_INIT(pScrn, &clipReg, dst, 1);
		REGION_INTERSECT(pScrn, reg, reg, &clipReg);
		REGION_UNINIT(pScrn, &clipReg);
	}

	LEAVE_PROC("SMI_ClipVideo");
	return(TRUE);
}

static void
SMI_DisplayVideo(
	ScrnInfoPtr	pScrn,
	int			id,
	int			offset,
	short		width,
	short		height,
	int			pitch,
	int			x1,
	int			y1,
	int			x2,
	int			y2,
	BoxPtr		dstBox,
	short		vid_w,
	short		vid_h,
	short		drw_w,
	short		drw_h
)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	CARD32 vpr00;
	int hstretch, vstretch;

	ENTER_PROC("SMI_DisplayVideo");

	vpr00 = READ_VPR(pSmi, 0x00) & ~0x0CB800FF;

	switch (id)
	{
		case FOURCC_YV12:
		case FOURCC_I420:
		case FOURCC_YUY2:
			vpr00 |= 0x6;
			break;

		case FOURCC_RV15:
			vpr00 |= 0x1;
			break;

		case FOURCC_RV16:
			vpr00 |= 0x2;
			break;

		case FOURCC_RV24:
			vpr00 |= 0x4;
			break;

		case FOURCC_RV32:
			vpr00 |= 0x3;
			break;
	}

	
	if (drw_w > vid_w)
	{
		hstretch = (2560 * vid_w / drw_w + 5) / 10;
	}
	else
	{
		hstretch = 0;
	}

	if (drw_h > vid_h)
	{
		vstretch = (2560 * vid_h / drw_h + 5) / 10;
		vpr00 |= 1 << 21;
	}
	else
	{
		vstretch = 0;
	}

	SMI_WaitForSync(pScrn);

	WRITE_VPR(pSmi, 0x00, vpr00 | (1 << 3) | (1 << 20));
	WRITE_VPR(pSmi, 0x14, (dstBox->x1) | (dstBox->y1 << 16));
	WRITE_VPR(pSmi, 0x18, (dstBox->x2) | (dstBox->y2 << 16));
	WRITE_VPR(pSmi, 0x1C, offset >> 3);
	WRITE_VPR(pSmi, 0x20, (pitch >> 3) | ((pitch >> 3) << 16));
	WRITE_VPR(pSmi, 0x24, (hstretch << 8) | vstretch);

	LEAVE_PROC("SMI_DisplayVideo");
}

static void
SMI_BlockHandler(
	int		i,
	pointer	blockData,
	pointer	pTimeout,
	pointer	pReadMask
)
{
	ScreenPtr	pScreen = screenInfo.screens[i];
	ScrnInfoPtr	pScrn	= xf86Screens[i];
	SMIPtr		pSmi    = SMIPTR(pScrn);
	SMI_PortPtr	ptrPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;

	pScreen->BlockHandler = pSmi->BlockHandler;
	(*pScreen->BlockHandler)(i, blockData, pTimeout, pReadMask);
	pScreen->BlockHandler = SMI_BlockHandler;

	if (ptrPort->videoStatus & TIMER_MASK)
	{
		UpdateCurrentTime();
		if (ptrPort->videoStatus & OFF_TIMER)
		{
			if (ptrPort->offTime < currentTime.milliseconds)
			{
				WRITE_VPR(pSmi, 0x00, READ_VPR(pSmi, 0x00) & ~0x00000008);
				ptrPort->videoStatus = FREE_TIMER;
				ptrPort->freeTime = currentTime.milliseconds + FREE_DELAY;
			}
		}
		else
		{
			if (ptrPort->freeTime < currentTime.milliseconds)
			{
				xf86FreeOffscreenArea(ptrPort->area);
				ptrPort->area = NULL;
			}
			ptrPort->videoStatus = 0;
		}
	}
}

static int
SMI_SendI2C(
	ScrnInfoPtr		pScrn,
	CARD8			device,
	char			*devName,
	SMI_I2CDataPtr	i2cData
)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	I2CDevPtr dev;
	int status = Success;

	ENTER_PROC("SMI_SendI2C");

	if (pSmi->I2C == NULL)
	{
		LEAVE_PROC("SMI_SendI2C");
		return(BadAlloc);
	}

	dev = xf86CreateI2CDevRec();
	if (dev == NULL)
	{
		LEAVE_PROC("SMI_SendI2C");
		return(BadAlloc);
	}
	dev->DevName = devName;
	dev->SlaveAddr = device;
	dev->pI2CBus = pSmi->I2C;

	if (!xf86I2CDevInit(dev))
	{
		status = BadAlloc;
	}
	else
	{
		while (i2cData->address != 0xFF || i2cData->data != 0xFF) /* PDR#676 */
		{
			if (!xf86I2CWriteByte(dev, i2cData->address, i2cData->data))
			{
				status = BadAlloc;
				break;
			}
			i2cData++;
		}
	}

	xf86DestroyI2CDevRec(dev, TRUE);
	LEAVE_PROC("SMI_SendI2C");
	return(status);
}

/******************************************************************************\
**																			  **
**				 O F F S C R E E N   M E M O R Y   M A N A G E R			  **
**																			  **
\******************************************************************************/

static void
SMI_InitOffscreenImages(
	ScreenPtr	pScreen
)
{
	XF86OffscreenImagePtr offscreenImages;
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	SMIPtr pSmi = SMIPTR(pScrn);

	ENTER_PROC("SMI_InitOffscreenImages");

	offscreenImages = xalloc(sizeof(XF86OffscreenImageRec));
	if (offscreenImages == NULL)
	{
		LEAVE_PROC("SMI_InitOffscreenImages");
		return;
	}

	offscreenImages->image = SMI_VideoImages;
	offscreenImages->flags = VIDEO_OVERLAID_IMAGES
						   | VIDEO_CLIP_TO_VIEWPORT;
	offscreenImages->alloc_surface = SMI_AllocSurface;
	offscreenImages->free_surface = SMI_FreeSurface;
	offscreenImages->display = SMI_DisplaySurface;
	offscreenImages->stop = SMI_StopSurface;
	offscreenImages->getAttribute = SMI_GetSurfaceAttribute;
	offscreenImages->setAttribute = SMI_SetSurfaceAttribute;
	offscreenImages->max_width = pSmi->lcdWidth;
	offscreenImages->max_height = pSmi->lcdHeight;
	offscreenImages->num_attributes = nElems(SMI_VideoAttributes);
	offscreenImages->attributes = SMI_VideoAttributes;

	xf86XVRegisterOffscreenImages(pScreen, offscreenImages, 1);

	LEAVE_PROC("SMI_InitOffscreenImages");
}

static FBAreaPtr
SMI_AllocateMemory(
	ScrnInfoPtr	pScrn,
	FBAreaPtr	area,
	int			numLines
)
{
	ScreenPtr pScreen = screenInfo.screens[pScrn->scrnIndex];

	ENTER_PROC("SMI_AllocateMemory");

	if (area != NULL)
	{
		if ((area->box.y2 - area->box.y1) >= numLines)
		{
			LEAVE_PROC("SMI_AllocateMemory");
			return(area);
		}

		if (xf86ResizeOffscreenArea(area, pScrn->displayWidth, numLines))
		{
			LEAVE_PROC("SMI_AllocateMemory");
			return(area);
		}

		xf86FreeOffscreenArea(area);
	}

	area = xf86AllocateOffscreenArea(pScreen, pScrn->displayWidth, numLines, 0,
			NULL, NULL, NULL);

	if (area == NULL)
	{
		int maxW, maxH;

		xf86QueryLargestOffscreenArea(pScreen, &maxW, &maxH, 0,
				FAVOR_WIDTH_THEN_AREA, PRIORITY_EXTREME);

		if ((maxW < pScrn->displayWidth) || (maxH < numLines))
		{
			LEAVE_PROC("SMI_AllocateMemory");
			return(NULL);
		}

		xf86PurgeUnlockedOffscreenAreas(pScreen);
		area = xf86AllocateOffscreenArea(pScreen, pScrn->displayWidth, numLines,
				0, NULL, NULL, NULL);
	}

	LEAVE_PROC("SMI_AllocateMemory");
	return(area);
}

static void
SMI_CopyData(
	unsigned char	*src,
	unsigned char	*dst,
	int				srcPitch,
	int				dstPitch,
	int				height,
	int				width
)
{
	ENTER_PROC("SMI_CopyData");

	while (height-- > 0)
	{
		memcpy(dst, src, width);
		src += srcPitch;
		dst += dstPitch;
	}

	LEAVE_PROC("SMI_CopyData");
}

static void
SMI_CopyYV12Data(
	unsigned char	*src1,
	unsigned char	*src2,
	unsigned char	*src3,
	unsigned char	*dst,
	int				srcPitch1,
	int				srcPitch2,
	int				dstPitch,
	int				height,
	int				width
)
{
	CARD32 *pDst = (CARD32 *) dst;
	int i, j;

	ENTER_PROC("SMI_CopyYV12Data");

	for (j = 0; j < height; j++)
	{
		for (i =0; i < width; i++)
		{
			pDst[i] = src1[i << 1] | (src1[(i << 1) + 1] << 16) |
					(src3[i] << 8) | (src2[i] << 24);
		}
		pDst += dstPitch >> 2;
		src1 += srcPitch1;
		if (j & 1)
		{
			src2 += srcPitch2;
			src3 += srcPitch2;
		}
	}

	LEAVE_PROC("SMI_CopyYV12Data");
}

static int
SMI_AllocSurface(
	ScrnInfoPtr		pScrn,
	int				id,
	unsigned short	width,
	unsigned short	height,
	XF86SurfacePtr	surface
)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	int numLines, pitch, fbPitch, bpp;
	SMI_OffscreenPtr ptrOffscreen;
	FBAreaPtr area;

	ENTER_PROC("SMI_AllocSurface");

	if ((width > pSmi->lcdWidth) || (height > pSmi->lcdHeight))
	{
		LEAVE_PROC("SMI_AllocSurface");
		return(BadAlloc);
	}

	if (pSmi->Bpp == 3)
	{
		fbPitch = pSmi->Stride;
	}
	else
	{
		fbPitch = pSmi->Stride * pSmi->Bpp;
	}

	width = (width + 1) & ~1;
	switch (id)
	{
		case FOURCC_YV12:
		case FOURCC_I420:
		case FOURCC_YUY2:
		case FOURCC_RV15:
		case FOURCC_RV16:
			bpp = 2;
			break;

		case FOURCC_RV24:
			bpp = 3;
			break;

		case FOURCC_RV32:
			bpp = 4;
			break;

		default:
			LEAVE_PROC("SMI_AllocSurface");
			return(BadAlloc);
	}
	pitch = (width * bpp + 15) & ~15;

	numLines = ((height * pitch) + fbPitch - 1) / fbPitch;

	area = SMI_AllocateMemory(pScrn, NULL, numLines);
	if (area == NULL)
	{
		LEAVE_PROC("SMI_AllocSurface");
		return(BadAlloc);
	}

	surface->pitches = xalloc(sizeof(int));
	if (surface->pitches == NULL)
	{
		xf86FreeOffscreenArea(area);
		LEAVE_PROC("SMI_AllocSurface");
		return(BadAlloc);
	}
	surface->offsets = xalloc(sizeof(int));
	if (surface->offsets == NULL)
	{
		xfree(surface->pitches);
		xf86FreeOffscreenArea(area);
		LEAVE_PROC("SMI_AllocSurface");
		return(BadAlloc);
	}

	ptrOffscreen = xalloc(sizeof(SMI_OffscreenRec));
	if (ptrOffscreen == NULL)
	{
		xfree(surface->offsets);
		xfree(surface->pitches);
		xf86FreeOffscreenArea(area);
		LEAVE_PROC("SMI_AllocSurface");
		return(BadAlloc);
	}

	surface->pScrn = pScrn;
	surface->id = id;
	surface->width = width;
	surface->height = height;
	surface->pitches[0] = pitch;
	surface->offsets[0] = area->box.y1 * fbPitch;
	surface->devPrivate.ptr = (pointer) ptrOffscreen;

	ptrOffscreen->area = area;
	ptrOffscreen->isOn = FALSE;

	LEAVE_PROC("SMI_AllocSurface");
	return(Success);
}

static int
SMI_FreeSurface(
	XF86SurfacePtr	surface
)
{
	SMI_OffscreenPtr ptrOffscreen = (SMI_OffscreenPtr) surface->devPrivate.ptr;

	ENTER_PROC("SMI_FreeSurface");

	if (ptrOffscreen->isOn)
	{
		SMI_StopSurface(surface);
	}

	xf86FreeOffscreenArea(ptrOffscreen->area);
	xfree(surface->pitches);
	xfree(surface->offsets);
	xfree(surface->devPrivate.ptr);

	LEAVE_PROC("SMI_FreeSurface");
	return(Success);
}

static int
SMI_DisplaySurface(
	XF86SurfacePtr	surface,
	short			vid_x,
	short			vid_y,
	short			drw_x,
	short			drw_y,
	short			vid_w,
	short			vid_h,
	short			drw_w,
	short			drw_h,
	RegionPtr		clipBoxes
)
{
	SMI_OffscreenPtr ptrOffscreen = (SMI_OffscreenPtr) surface->devPrivate.ptr;
	SMIPtr pSmi = SMIPTR(surface->pScrn);
	SMI_PortPtr ptrPort = pSmi->ptrAdaptor->pPortPrivates[0].ptr;
	INT32 x1, y1, x2, y2;
	BoxRec dstBox;

	ENTER_PROC("SMI_DisplaySurface");

	x1 = vid_x;
	x2 = vid_x + vid_w;
	y1 = vid_y;
	y2 = vid_y + vid_h;

	dstBox.x1 = drw_x;
	dstBox.x2 = drw_x + drw_w;
	dstBox.y1 = drw_y;
	dstBox.y2 = drw_y + drw_h;

	if (!SMI_ClipVideo(surface->pScrn, &dstBox, &x1, &y1, &x2, &y2, clipBoxes,
			surface->width, surface->height))
	{
		LEAVE_PROC("SMI_DisplaySurface");
		return(Success);
	}

	dstBox.x1 -= surface->pScrn->frameX0;
	dstBox.y1 -= surface->pScrn->frameY0;
	dstBox.x2 -= surface->pScrn->frameX0;
	dstBox.y2 -= surface->pScrn->frameY0;

	XAAFillSolidRects(surface->pScrn, ptrPort->colorKey, GXcopy, ~0,
			REGION_NUM_RECTS(clipBoxes), REGION_RECTS(clipBoxes));

	SMI_ResetVideo(surface->pScrn);
	SMI_DisplayVideo(surface->pScrn, surface->id, surface->offsets[0],
			surface->width, surface->height, surface->pitches[0], x1, y1, x2,
			y2, &dstBox, vid_w, vid_h, drw_w, drw_h);

	ptrOffscreen->isOn = TRUE;
	if (ptrPort->videoStatus & CLIENT_VIDEO_ON)
	{
		REGION_EMPTY(pScrn->pScreen, &ptrPort->clip);
		UpdateCurrentTime();
		ptrPort->videoStatus = FREE_TIMER;
		ptrPort->freeTime = currentTime.milliseconds + FREE_DELAY;
	}

	LEAVE_PROC("SMI_DisplaySurface");
	return(Success);
}

static int
SMI_StopSurface(
	XF86SurfacePtr	surface
)
{
	SMI_OffscreenPtr ptrOffscreen = (SMI_OffscreenPtr) surface->devPrivate.ptr;

	ENTER_PROC("SMI_StopSurface");

	if (ptrOffscreen->isOn)
	{
		SMIPtr pSmi = SMIPTR(surface->pScrn);
		WRITE_VPR(pSmi, 0x00, READ_VPR(pSmi, 0x00) & ~0x00000008);
		ptrOffscreen->isOn = FALSE;
	}

	LEAVE_PROC("SMI_StopSurface");
	return(Success);
}

static int
SMI_GetSurfaceAttribute(
	ScrnInfoPtr	pScrn,
	Atom		attr,
	INT32		*value
)
{
	SMIPtr pSmi = SMIPTR(pScrn);

	return(SMI_GetPortAttribute(pScrn, attr, value,
			(pointer) pSmi->ptrAdaptor->pPortPrivates[0].ptr));
}

static int
SMI_SetSurfaceAttribute(
	ScrnInfoPtr	pScrn,
	Atom		attr,
	INT32		value
)
{
	SMIPtr pSmi = SMIPTR(pScrn);

	return(SMI_SetPortAttribute(pScrn, attr, value,
			(pointer) pSmi->ptrAdaptor->pPortPrivates[0].ptr));
}
#else /* XvExtension */
void SMI_InitVideo(ScreenPtr pScreen) {}
#endif
