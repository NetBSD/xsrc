/*
 *  Permedia 3 Xv Driver
 *
 *  Copyright (C) 2001 Sven Luther <luther@dpt-info.u-strasbg.fr>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *
 *  Based on work of Michael H. Schimek <m.schimek@netway.at>
 */
 
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/pm3_video.c,v 1.6 2001/05/08 19:31:22 alanh Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Pci.h"
#include "xf86PciInfo.h"
#include "xf86Xinput.h"
#include "xf86fbman.h"
#include "xf86xv.h"
#include "Xv.h"
#include "regionstr.h"
#include "xaa.h"
#include "xaalocal.h"

#include "glint_regs.h"
#include "pm3_regs.h"
#include "glint.h"

#define DEBUG(x)
#define USE_HARDWARE_COPY	1
#define SUPPORT_CLIPPING	1
#define BLACKNESS_WORKAROUND	1

#ifndef XvExtension

void Permedia3VideoInit(ScreenPtr pScreen) {}
void Permedia3VideoUninit(ScrnInfoPtr pScrn) {}
void Permedia3VideoEnterVT(ScrnInfoPtr pScrn) {}
void Permedia3VideoLeaveVT(ScrnInfoPtr pScrn) {}

#else

#undef MIN
#undef ABS
#undef CLAMP
#undef ENTRIES

#define MIN(a, b) (((a) < (b)) ? (a) : (b)) 
#define ABS(n) (((n) < 0) ? -(n) : (n))
#define CLAMP(v, min, max) (((v) < (min)) ? (min) : MIN(v, max))
#define ENTRIES(array) (sizeof(array) / sizeof((array)[0]))

enum {
    OVERLAY_DATA_NONE,
    OVERLAY_DATA_COLORKEY,
    OVERLAY_DATA_ALPHAKEY,
    OVERLAY_DATA_ALPHABLEND
} ;

#define MAX_BUFFERS 3

typedef struct _PortPrivRec {
    struct _AdaptorPrivRec *    pAdaptor;

    /* Sync function */
    void (*Sync) (ScrnInfoPtr pScrn);

    /* Attributes */
    INT32			ColorKey;
    INT32			OverlayAlpha;
    INT32			OverlayMode;
    INT32			Attribute[3];

    /* Clipping */
    RegionRec			clip;

#if 0 /* Adding this cause the server to crash if we minimize the video */
    /* Frame counter */
    char			Frames;
#endif

    /* Ramdac save values, ... */
    INT32			ramdac_x, ramdac_w;
    INT32			ramdac_y, ramdac_h;
    Bool			ramdac_on;

    /* Buffers */
    int				Id, Format;
    int				FB_Shift, Video_Shift;
    short			display, copy;
    FBAreaPtr			Buffer[MAX_BUFFERS];
    CARD32			BufferBase[MAX_BUFFERS];

    /* Buffer and Drawable size and position */
    INT32			vx, vy, vw, vh;			/* 12.10 fp */
    int				dx, dy, dw, dh;

    /* Timer stuff */
    OsTimerPtr			Timer;
    Bool			TimerInUse;
    int				Delay, Instant, StopDelay;

} PortPrivRec, *PortPrivPtr;

typedef struct _AdaptorPrivRec {
    struct _AdaptorPrivRec *	Next;
    ScrnInfoPtr			pScrn;
    PortPrivPtr                 pPort;
} AdaptorPrivRec, *AdaptorPrivPtr;

static AdaptorPrivPtr AdaptorPrivList = NULL;

/*
 *  Proprietary Attributes
 */
 
#define XV_FILTER		"XV_FILTER"
/* We support 3 sorts of filters :
 * 0 : None.
 * 1 : Partial (only in the X directrion).
 * 2 : Full (incompatible with X mirroring).
 */

#define XV_MIRROR		"XV_MIRROR"	
/* We also support mirroring of the image :
 * bit 0 : if set, will mirror in the X direction
 *         (incompatible with full filtering).
 * bit 1 : if set, will mirror in the Y direction.
 */

#define XV_ALPHA		"XV_ALPHA"
/* We support the following alpha blend factors :
 * 0 ->   0% Video, 100% Framebuffer
 * 1 ->  25% Video,  75% Framebuffer
 * 2 ->  75% Video,  25% Framebuffer
 * 3 -> 100% Video,   0% Framebuffer 
 */

static XF86AttributeRec
ScalerAttributes[] =
{
    { XvSettable | XvGettable, 0, 2, XV_FILTER },
    { XvSettable | XvGettable, 0, 3, XV_MIRROR },
    { XvSettable | XvGettable, 0, 3, XV_ALPHA },
};

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

static Atom xvFilter, xvMirror, xvAlpha;


/* Scaler */

static XF86VideoEncodingRec
ScalerEncodings[] =
{
    { 0, "XV_IMAGE", 2047, 2047, { 1, 1 }},
};

static XF86VideoFormatRec
ScalerVideoFormats[] =
{
    { 8,  TrueColor }, /* Dithered */
    { 15, TrueColor },
    { 16, TrueColor },
    { 24, TrueColor },
};

/*
 *  FOURCC from http://www.webartz.com/fourcc
 *  Generic GUID for legacy FOURCC XXXXXXXX-0000-0010-8000-00AA00389B71
 */
#define LE4CC(a,b,c,d) (((CARD32)(a)&0xFF)|(((CARD32)(b)&0xFF)<<8)|(((CARD32)(c)&0xFF)<<16)|(((CARD32)(d)&0xFF)<<24))
#define GUID4CC(a,b,c,d) { a,b,c,d,0,0,0,0x10,0x80,0,0,0xAA,0,0x38,0x9B,0x71 }

#define NoOrder LSBFirst

static XF86ImageRec
ScalerImages[] =
{
    /* Planar YVU 4:2:0 (emulated) */
    { LE4CC('Y','V','1','2'), XvYUV, NoOrder, GUID4CC('Y','V','1','2'),
      12, XvPlanar, 3, 0, 0, 0, 0,
      8, 8, 8,  1, 2, 2,  1, 2, 2, "YVU", XvTopToBottom },

    /* Packed YUYV 4:2:2 */
    { LE4CC('Y','U','Y','2'), XvYUV, NoOrder, GUID4CC('Y','U','Y','2'),
      16, XvPacked, 1, 0, 0, 0, 0,
      8, 8, 8,  1, 2, 2,  1, 1, 1, "YUYV", XvTopToBottom },

    /* Packed UYVY 4:2:2 */
    { LE4CC('U','Y','V','Y'), XvYUV, NoOrder, GUID4CC('U','Y','V','Y'),
      16, XvPacked, 1, 0, 0, 0, 0,
      8, 8, 8,  1, 2, 2,  1, 1, 1, "UYVY", XvTopToBottom },

    /* Packed YUVA 4:4:4 */
    { LE4CC('Y','U','V','A') /* XXX not registered */, XvYUV, LSBFirst, { 0 },
      32, XvPacked, 1, 0, 0, 0, 0,
      8, 8, 8,  1, 1, 1,  1, 1, 1, "YUVA", XvTopToBottom },

    /* Packed VUYA 4:4:4 */
    { LE4CC('V','U','Y','A') /* XXX not registered */, XvYUV, LSBFirst, { 0 },
      32, XvPacked, 1, 0, 0, 0, 0,
      8, 8, 8,  1, 1, 1,  1, 1, 1, "VUYA", XvTopToBottom },

    /* RGBA 8:8:8:8 */
    { 0x41, XvRGB, LSBFirst, { 0 },
      32, XvPacked, 1, 24, 0x0000FF, 0x00FF00, 0xFF0000, 
      0, 0, 0,  0, 0, 0,  0, 0, 0, "RGBA", XvTopToBottom },

    /* RGB 5:6:5 */
    { 0x42, XvRGB, LSBFirst, { 0 },
      16, XvPacked, 1, 16, 0x001F, 0x07E0, 0xF800, 
      0, 0, 0,  0, 0, 0,  0, 0, 0, "RGB", XvTopToBottom },

    /* RGBA 5:5:5:1 */
    { 0x43, XvRGB, LSBFirst, { 0 },
      16, XvPacked, 1, 15, 0x001F, 0x03E0, 0x7C00, 
      0, 0, 0,  0, 0, 0,  0, 0, 0, "RGBA", XvTopToBottom },

    /* RGBA 4:4:4:4 */
    { 0x44, XvRGB, LSBFirst, { 0 },
      16, XvPacked, 1, 12, 0x000F, 0x00F0, 0x0F00, 
      0, 0, 0,  0, 0, 0,  0, 0, 0, "RGBA", XvTopToBottom },

    /* RGB 3:3:2 */
    { 0x46, XvRGB, NoOrder, { 0 },
      8, XvPacked, 1, 8, 0x07, 0x38, 0xC0, 
      0, 0, 0,  0, 0, 0,  0, 0, 0, "RGB", XvTopToBottom },

    /* BGRA 8:8:8:8 */
    { 0x47, XvRGB, LSBFirst, { 0 },
      32, XvPacked, 1, 24, 0xFF0000, 0x00FF00, 0x0000FF,
      0, 0, 0,  0, 0, 0,  0, 0, 0, "BGRA", XvTopToBottom },

    /* BGR 5:6:5 */
    { 0x48, XvRGB, LSBFirst, { 0 },
      16, XvPacked, 1, 16, 0xF800, 0x07E0, 0x001F,
      0, 0, 0,  0, 0, 0,  0, 0, 0, "BGR", XvTopToBottom },

    /* BGRA 5:5:5:1 */
    { 0x49, XvRGB, LSBFirst, { 0 },
      16, XvPacked, 1, 15, 0x7C00, 0x03E0, 0x001F,
      0, 0, 0,  0, 0, 0,  0, 0, 0, "BGRA", XvTopToBottom },

    /* BGRA 4:4:4:4 */
    { 0x4A, XvRGB, LSBFirst, { 0 },
      16, XvPacked, 1, 12, 0x0F00, 0x00F0, 0x000F,
      0, 0, 0,  0, 0, 0,  0, 0, 0, "BGRA", XvTopToBottom },

    /* BGR 2:3:3 */
    { 0x4C, XvRGB, NoOrder, { 0 },
      8, XvPacked, 1, 8, 0xC0, 0x38, 0x07,
      0, 0, 0,  0, 0, 0,  0, 0, 0, "BGR", XvTopToBottom },
};
/*
 *  Buffer management
 */

static void
RemoveBufferCallback(FBAreaPtr Buffer)
{
    PortPrivPtr pPPriv = (PortPrivPtr) Buffer->devPrivate.ptr;
    int i = -1;

    /* Find the buffer that is being removed */
    for (i = 0; i < MAX_BUFFERS && pPPriv->Buffer[i] != Buffer; i++);
    if (i >= MAX_BUFFERS) return;
	
    if (i == pPPriv->display) pPPriv->display = -1;
    if (i == pPPriv->copy) pPPriv->copy = -1;
    pPPriv->Buffer[i] = NULL;
}

static void
FreeBuffers(PortPrivPtr pPPriv, Bool from_timer)
{
    int i = -1;

    if (!from_timer) {
	if (pPPriv->TimerInUse) {
	    pPPriv->TimerInUse = FALSE;
	    TimerCancel(pPPriv->Timer);
	}
    }

    pPPriv->display = -1;
    pPPriv->copy = -1;
    for (i=0; i < MAX_BUFFERS; i++)
	if (pPPriv->Buffer[i]) {
	    xf86FreeOffscreenArea (pPPriv->Buffer[i]);
	    pPPriv->Buffer[i] = NULL;
	}
}

static CARD32
TimerCallback(OsTimerPtr pTim, CARD32 now, pointer p)
{
    PortPrivPtr pPPriv = (PortPrivPtr) p;

    if (pPPriv->StopDelay >= 0) {
	if (!(pPPriv->StopDelay--)) {
	    FreeBuffers(pPPriv, TRUE);
	    pPPriv->TimerInUse = FALSE;
	}
    }

    if (pPPriv->TimerInUse)
	return pPPriv->Instant;

    return 0; /* Cancel */
}

static int
AllocateBuffers(PortPrivPtr pPPriv, int w_bpp, int h)
{
    AdaptorPrivPtr pAPriv = pPPriv->pAdaptor;
    ScrnInfoPtr pScrn = pAPriv->pScrn;
    int i = -1;

    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"We try to allocate a %dx%d buffer.\n", w_bpp, h));
    /* we start a timer to free the buffers if they are nto used within
     * 5 seconds (pPPriv->Delay * pPPriv->Instant) */
    pPPriv->StopDelay = pPPriv->Delay;
    if (!pPPriv->TimerInUse) {
	pPPriv->TimerInUse = TRUE;
	TimerSet(pPPriv->Timer, 0, 80, TimerCallback, pAPriv);
    }

    for (i=0; i < MAX_BUFFERS
	&& (i == pPPriv->display || i == pPPriv->copy); i++);

    if (pPPriv->Buffer[i]) {
	DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	    "Buffer %d exists.\n", i));
	if ((pPPriv->Buffer[i]->box.x2 - pPPriv->Buffer[i]->box.x1) == w_bpp &&
	    (pPPriv->Buffer[i]->box.y2 - pPPriv->Buffer[i]->box.y1) == h) {
	    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
		"Buffer %d is of the good size, let's use it.\n", i));
	    return (pPPriv->copy = i);
	}
	else if (xf86ResizeOffscreenArea (pPPriv->Buffer[i], w_bpp, h)) {
		DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
		    "I was able to resize buffer %d, let's use it.\n", i));
		pPPriv->BufferBase[i] =
		    ((pPPriv->Buffer[i]->box.y1 * pScrn->displayWidth) +
		     pPPriv->Buffer[i]->box.x1)<<pPPriv->FB_Shift;
	    	return (pPPriv->copy = i);
	    }
	    else {
		DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
		    "I was not able to resize buffer %d.\n", i));
		xf86FreeOffscreenArea (pPPriv->Buffer[i]);
		pPPriv->Buffer[i] = NULL;
	    }
    }
    if ((pPPriv->Buffer[i] = xf86AllocateOffscreenArea (pScrn->pScreen,
	w_bpp, h, 4 >> pPPriv->FB_Shift, NULL, NULL, (pointer) pPPriv))) {
	DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	    "Sucessfully allocated buffer %d, let's use it.\n", i));
	pPPriv->BufferBase[i] =
	    ((pPPriv->Buffer[i]->box.y1 * pScrn->displayWidth) +
	     pPPriv->Buffer[i]->box.x1)<<pPPriv->FB_Shift;
	return (pPPriv->copy = i);
    }
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"Unable to allocate a buffer.\n"));
    return -1;
}

/*
 *  Xv interface
 */

#if USE_HARDWARE_COPY
static void
HWCopySetup(PortPrivPtr pPPriv, int x, int y, int w, int h)
{
    AdaptorPrivPtr pAPriv = pPPriv->pAdaptor;
    ScrnInfoPtr pScrn = pAPriv->pScrn;
    GLINTPtr pGlint = GLINTPTR(pScrn);
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	"x = %d, y = %d, w = %d, h = %d.\n", x, y, w, h));

    GLINT_WAIT(4);
    GLINT_WRITE_REG(0xffffffff, FBHardwareWriteMask);
    GLINT_WRITE_REG(
	PM3Config2D_ForegroundROPEnable |
	PM3Config2D_ForegroundROP(GXcopy) |
	PM3Config2D_FBWriteEnable,
	PM3Config2D);
    GLINT_WRITE_REG(
	PM3RectanglePosition_XOffset(x) |
	PM3RectanglePosition_YOffset(y),
	PM3RectanglePosition);
    GLINT_WRITE_REG(
	PM3Render2D_SpanOperation |
	PM3Render2D_XPositive |
	PM3Render2D_YPositive |
	PM3Render2D_Operation_SyncOnHostData |
	PM3Render2D_Width(w) | PM3Render2D_Height(h),
	PM3Render2D);
}
static void
HWCopyYV12(PortPrivPtr pPPriv, CARD8 *Y, int w, int h)
{
    AdaptorPrivPtr pAPriv = pPPriv->pAdaptor;
    ScrnInfoPtr pScrn = pAPriv->pScrn;
    GLINTPtr pGlint = GLINTPTR(pScrn);
    int Y_size = w * h;
    CARD8 *V = Y + Y_size;
    CARD8 *U = V + (Y_size >> 2);
    CARD32 *dst;
    int dwords, i, x;

    dwords = Y_size >> 1;

    x = 0;
    while (dwords >= pGlint->FIFOSize) {
	dst = (CARD32*)((char*)pGlint->IOBase + OutputFIFO + 4);
	GLINT_WAIT(pGlint->FIFOSize);
	/* (0x15 << 4) | 0x05 is the TAG for FBSourceData */
	GLINT_WRITE_REG(((pGlint->FIFOSize - 2) << 16) | (0x15 << 4) |
	    0x05, OutputFIFO);
	for (i = pGlint->FIFOSize - 1; i; i--, Y += 2, U++, V++, dst++, x++) {
	/* mmm, i don't know if this is really needed, as we perform
	 * endianess inversion as usual, let's check it before removing */
#if X_BYTE_ORDER == X_BIG_ENDIAN
	    *dst = V[0] + (Y[1] << 8) + (U[0] << 16) + (Y[0] << 24);
#else
	    *dst = Y[0] + (U[0] << 8) + (Y[1] << 16) + (V[0] << 24);
#endif
	    if (x == w>>1) { U -= w>>1; V -= w>>1; }
	    if (x == w) x = 0;
	}
	dwords -= pGlint->FIFOSize - 1;
    }
    if (dwords) {
	dst = (CARD32*)((char*)pGlint->IOBase + OutputFIFO + 4);
	GLINT_WAIT(dwords + 1);
	/* (0x15 << 4) | 0x05 is the TAG for FBSourceData */
	GLINT_WRITE_REG(((dwords - 1) << 16) | (0x15 << 4) |
	    0x05, OutputFIFO);
	for (i = dwords; i; i--, Y += 2, U++, V++, dst++, x++) {
	/* mmm, i don't know if this is really needed, as we perform
	 * endianess inversion as usual, let's check it before removing */
#if X_BYTE_ORDER == X_BIG_ENDIAN
	    *dst = V[0] + (Y[1] << 8) + (U[0] << 16) + (Y[0] << 24);
#else
	    *dst = Y[0] + (U[0] << 8) + (Y[1] << 16) + (V[0] << 24);
#endif
	    if (x == w>>1) { U -= w>>1; V -= w>>1; }
	    if (x == w) x = 0;
	}
    }
}
static void
HWCopyFlat(PortPrivPtr pPPriv, CARD8 *src, int w, int h)
{
    AdaptorPrivPtr pAPriv = pPPriv->pAdaptor;
    ScrnInfoPtr pScrn = pAPriv->pScrn;
    GLINTPtr pGlint = GLINTPTR(pScrn);
    int size = w * h;
    int pitch = pScrn->displayWidth<<pPPriv->FB_Shift;
    CARD32 *dst;
    CARD8 *tmp_src;
    int dwords, i;

    if (w == pitch) {
	DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	    "HWCopyFlat : src = %08x, w = pitch = %d, h = %d.\n",
	    src, w, h));
	dwords = size >> pPPriv->Video_Shift;
	while (dwords >= pGlint->FIFOSize) {
	    dst = (CARD32*)((char*)pGlint->IOBase + OutputFIFO + 4);
	    GLINT_WAIT(pGlint->FIFOSize);
	    GLINT_WRITE_REG(((pGlint->FIFOSize - 2) << 16) | (0x15 << 4) |
		0x05, OutputFIFO);
	    for (i = pGlint->FIFOSize - 1; i; i--, dst++, src++) *dst = *src;
	    dwords -= pGlint->FIFOSize - 1;
	}
	if (dwords) {
	    dst = (CARD32*)((char*)pGlint->IOBase + OutputFIFO + 4);
	    GLINT_WAIT(dwords + 1);
	    GLINT_WRITE_REG(((dwords - 1) << 16) | (0x15 << 4) |
		0x05, OutputFIFO);
	    for (i = dwords; i; i--, dst++, src++) *dst = *src;
	}
    } else {
	DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	    "HWCopyFlat : src = %08x, w = %d, pitch = %d, h = %d.\n",
	    src, w, pitch, h));
	while (h) {
	    tmp_src = src;
	    dwords = w >> pPPriv->Video_Shift;
	    while (dwords >= pGlint->FIFOSize) {
		dst = (CARD32*)((char*)pGlint->IOBase + OutputFIFO + 4);
		GLINT_WAIT(pGlint->FIFOSize);
		GLINT_WRITE_REG(((pGlint->FIFOSize - 2) << 16) | (0x15 << 4) |
		    0x05, OutputFIFO);
		for (i = pGlint->FIFOSize - 1; i; i--, dst++, src++) *dst = *src;
		dwords -= pGlint->FIFOSize - 1;
	    }
	    if (dwords) {
		dst = (CARD32*)((char*)pGlint->IOBase + OutputFIFO + 4);
		GLINT_WAIT(dwords + 1);
		GLINT_WRITE_REG(((dwords - 1) << 16) | (0x15 << 4) |
		    0x05, OutputFIFO);
		for (i = dwords; i; i--, dst++, src++) *dst = *src;
	    }
	    src = tmp_src + pitch;
	}
    }
}
#else
static void
CopyYV12(CARD8 *Y, CARD32 *dst, int width, int height, int pitch)
{
    int Y_size = width * height;
    CARD8 *V = Y + Y_size;
    CARD8 *U = V + (Y_size >> 2);
    int pad = (pitch >> 2) - (width >> 1);
    int x;

    width >>= 1;

    for (height >>= 1; height > 0; height--) {
	for (x = 0; x < width; Y += 2, x++)
#if X_BYTE_ORDER == X_BIG_ENDIAN
	    *dst++ = V[x] + (Y[1] << 8) + (U[x] << 16) + (Y[0] << 24);
#else
	    *dst++ = Y[0] + (U[x] << 8) + (Y[1] << 16) + (V[x] << 24);
#endif
	dst += pad;
	for (x = 0; x < width; Y += 2, x++)
#if X_BYTE_ORDER == X_BIG_ENDIAN
	    *dst++ = V[x] + (Y[1] << 8) + (U[x] << 16) + (Y[0] << 24);
#else
	    *dst++ = Y[0] + (U[x] << 8) + (Y[1] << 16) + (V[x] << 24);
#endif
	dst += pad;
	U += width;
	V += width;
    }
}

static void
CopyFlat(CARD8 *src, CARD8 *dst, int width, int height, int pitch)
{
    if (width == pitch) {
	memcpy(dst, src, width * height);
	return;
    }

    while (height > 0) {
	memcpy(dst, src, width);
	dst += pitch;
	src += width;
	height--;
    }
}
#endif

#define FORMAT_RGB8888	PM3VideoOverlayMode_COLORFORMAT_RGB8888 
#define FORMAT_RGB4444	PM3VideoOverlayMode_COLORFORMAT_RGB4444
#define FORMAT_RGB5551	PM3VideoOverlayMode_COLORFORMAT_RGB5551
#define FORMAT_RGB565	PM3VideoOverlayMode_COLORFORMAT_RGB565
#define FORMAT_RGB332	PM3VideoOverlayMode_COLORFORMAT_RGB332
#define FORMAT_BGR8888	PM3VideoOverlayMode_COLORFORMAT_BGR8888
#define FORMAT_BGR4444	PM3VideoOverlayMode_COLORFORMAT_BGR4444
#define FORMAT_BGR5551	PM3VideoOverlayMode_COLORFORMAT_BGR5551
#define FORMAT_BGR565	PM3VideoOverlayMode_COLORFORMAT_BGR565
#define FORMAT_BGR332	PM3VideoOverlayMode_COLORFORMAT_BGR332
#define FORMAT_CI8	PM3VideoOverlayMode_COLORFORMAT_CI8
#define FORMAT_VUY444	PM3VideoOverlayMode_COLORFORMAT_VUY444
#define FORMAT_YUV444	PM3VideoOverlayMode_COLORFORMAT_YUV444
#define FORMAT_VUY422	PM3VideoOverlayMode_COLORFORMAT_VUY422
#define FORMAT_YUV422	PM3VideoOverlayMode_COLORFORMAT_YUV422

#define	RAMDAC_WRITE(data,index)				\
do{                                                             \
	mem_barrier();						\
	GLINT_WAIT(3);						\
	mem_barrier();						\
	GLINT_WRITE_REG(((index)>>8)&0xff, PM3RD_IndexHigh);	\
	mem_barrier();						\
 	GLINT_WRITE_REG((index)&0xff, PM3RD_IndexLow);		\
	mem_barrier();						\
	GLINT_WRITE_REG(data, PM3RD_IndexedData);		\
	mem_barrier();						\
}while(0)

#define RAMDAC_WRITE_OLD(data,index)				\
	Permedia2vOutIndReg(pScrn, index, 0x00, data)

/* Notice, have to check that we dont overflow the deltas here ... */
static void
compute_scale_factor(
    unsigned int* src_w, unsigned int* dst_w,
    unsigned int* shrink_delta, unsigned int* zoom_delta)
{
    if (*src_w >= *dst_w) {
	*dst_w &= ~0x3;
	*shrink_delta = (((*src_w << 16) / *dst_w) & 0x0ffffff0) + 0x10;
	*zoom_delta = 1<<16;
    } else {
	if (*src_w & 0x3) *src_w = (*src_w & ~0x3) + 4;
	*shrink_delta = 1<<16;
	for (;*dst_w > *src_w; (*dst_w)--) {
	    *zoom_delta = (*src_w << 16) / *dst_w;
	    if (((((*zoom_delta&0xf)+1) * *dst_w * *dst_w) >> 16) < *src_w) {
		*zoom_delta = ((*zoom_delta & ~0xf) + 0x10) & 0x0001fff0;
		return;
	    }
	}
	*zoom_delta = 1<<16;
    }
}

static void
BeginOverlay(PortPrivPtr pPPriv, int display)
{
    AdaptorPrivPtr pAPriv = pPPriv->pAdaptor;
    ScrnInfoPtr pScrn = pAPriv->pScrn;
    GLINTPtr pGlint = GLINTPTR(pScrn);
    unsigned int src_x = pPPriv->vx, dst_x = pPPriv->dx;
    unsigned int src_y = pPPriv->vy, dst_y = pPPriv->dy;
    unsigned int src_w = pPPriv->vw, dst_w = pPPriv->dw;
    unsigned int src_h = pPPriv->vh, dst_h = pPPriv->dh;
    unsigned int shrink_delta, zoom_delta;
#if BLACKNESS_WORKAROUND
    static int Frames = 50;
#endif
    unsigned int stride =
	(pScrn->displayWidth << pPPriv->FB_Shift) >> pPPriv->Video_Shift;

    /* Let's overlay only to visible parts of the screen */
    if (pPPriv->dx < pScrn->frameX0) {
	dst_w = dst_w - pScrn->frameX0 + dst_x;
	dst_x = 0;
	src_w = dst_w * pPPriv->vw / pPPriv->dw;
	src_x = src_x + pPPriv->vw - src_w;
    } else if (pScrn->frameX0 > 0) dst_x = dst_x - pScrn->frameX0;
    if (pPPriv->dy < pScrn->frameY0) {
	dst_h = dst_h - pScrn->frameY0 + pPPriv->dy; 
	dst_y = 0;
	src_h = dst_h * pPPriv->vh / pPPriv->dh;
	src_y = src_y + pPPriv->vh - src_h;
    } else if (pScrn->frameY0 > 0) dst_y = dst_y - pScrn->frameY0;
    if (dst_x + dst_w > (pScrn->frameX1 - pScrn->frameX0)) {
	unsigned int old_w = dst_w;
	dst_w = pScrn->frameX1 - pScrn->frameX0 - dst_x;
	src_w = dst_w * src_w / old_w;
    }
    if (dst_y + dst_h > (pScrn->frameY1 - pScrn->frameY0)) {
	unsigned int old_h = dst_h;
	dst_h = pScrn->frameY1 - pScrn->frameY0 - dst_y;
	src_h = dst_h * src_h / old_h;
    }

    /* Let's adjust the width of source and dest to be compliant with 
     * the Permedia3 overlay unit requirement, and compute the X deltas. */
    compute_scale_factor(&src_w, &dst_w, &shrink_delta, &zoom_delta);

#if BLACKNESS_WORKAROUND
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"BeginOverlay %d (buffer %d)\n", Frames, display));
#else
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"BeginOverlay (buffer %d)\n", display));
#endif
    if (src_w != pPPriv->vw)
	DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	    "BeginOverlay : Padding video width to 4 pixels %d->%d.\n",
	    pPPriv->vw, src_w));
    if (dst_w != pPPriv->dw)
	DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	    "BeginOverlay : Scaling destination width from %d to %d.\n"
	    "\tThe scaling factor is to high, and may cause problems.\n",
	    pPPriv->dw, dst_w));

    if (display != -1) pPPriv->display = display;

#if BLACKNESS_WORKAROUND
    if (++Frames>25) {
	Frames = 0;
	DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	    "Registers (1) : %08x, %08x, %08x, %08x, %08x.\n",
	    GLINT_READ_REG(PM3VideoOverlayFifoControl),
	    GLINT_READ_REG(PM3VideoOverlayMode),
	    GLINT_READ_REG(PM3VideoOverlayBase0),
	    GLINT_READ_REG(PM3VideoOverlayBase1),
	    GLINT_READ_REG(PM3VideoOverlayBase2)));
	DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	    "Registers (2) : %08x, %08x, %08x, %08x.\n",
	    GLINT_READ_REG(PM3VideoOverlayStride),
	    GLINT_READ_REG(PM3VideoOverlayWidth),
	    GLINT_READ_REG(PM3VideoOverlayHeight),
	    GLINT_READ_REG(PM3VideoOverlayOrigin)));
	DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	    "Registers (3) : %08x, %08x, %08x, %08x.\n",
	    GLINT_READ_REG(PM3VideoOverlayYDelta),
	    GLINT_READ_REG(PM3VideoOverlayShrinkXDelta),
	    GLINT_READ_REG(PM3VideoOverlayZoomXDelta),
	    GLINT_READ_REG(PM3VideoOverlayIndex)));
	GLINT_WAIT(8);
	GLINT_WRITE_REG(PM3VideoOverlayMode_DISABLE,
	    PM3VideoOverlayMode);
    } else GLINT_WAIT(7);
#else
    GLINT_WAIT(7);
#endif
    GLINT_WRITE_REG(3|(12<<16), PM3VideoOverlayFifoControl);
    /* Updating the Video Overlay Source Image Parameters */
    GLINT_WRITE_REG(
	pPPriv->BufferBase[pPPriv->display]>>pPPriv->Video_Shift,
	PM3VideoOverlayBase+(pPPriv->display*8));
    GLINT_WRITE_REG(pPPriv->Format |
	PM3VideoOverlayMode_BUFFERSYNC_MANUAL |
	PM3VideoOverlayMode_FLIP_VIDEO |
	/* Filtering & Mirroring Attributes */
	pPPriv->OverlayMode |
	PM3VideoOverlayMode_ENABLE,
	PM3VideoOverlayMode);
    /* Let's set the source stride. */
    GLINT_WRITE_REG(PM3VideoOverlayStride_STRIDE(stride),
	PM3VideoOverlayStride);
    /* Let's set the position and size of the visible part of the source. */
    GLINT_WRITE_REG(PM3VideoOverlayWidth_WIDTH(src_w),
	PM3VideoOverlayWidth);
    GLINT_WRITE_REG(PM3VideoOverlayHeight_HEIGHT(src_h),
	PM3VideoOverlayHeight);
    GLINT_WRITE_REG(
	PM3VideoOverlayOrigin_XORIGIN(src_x) |
	PM3VideoOverlayOrigin_YORIGIN(src_y),
	PM3VideoOverlayOrigin);

    GLINT_WAIT(5);
    /* Scale the source to the destinationsize */
    if (src_h == dst_h) {
	GLINT_WRITE_REG(
	    PM3VideoOverlayYDelta_NONE,
	    PM3VideoOverlayYDelta);
    } else {
	GLINT_WRITE_REG(
	    PM3VideoOverlayYDelta_DELTA(src_h,dst_h),
	    PM3VideoOverlayYDelta);
    }
    GLINT_WRITE_REG(shrink_delta, PM3VideoOverlayShrinkXDelta);
    GLINT_WRITE_REG(zoom_delta, PM3VideoOverlayZoomXDelta);
    GLINT_WRITE_REG(pPPriv->display, PM3VideoOverlayIndex);
    GLINT_WRITE_REG(PM3VideoOverlayUpdate_ENABLE,
	PM3VideoOverlayUpdate);


    /* Now set the ramdac video overlay region and mode */
    if ((pPPriv->ramdac_x != dst_x) || (pPPriv->ramdac_w != dst_w)) {
	RAMDAC_WRITE((dst_x&0xff), PM3RD_VideoOverlayXStartLow);
	RAMDAC_WRITE((dst_x&0xf00)>>8, PM3RD_VideoOverlayXStartHigh);
	RAMDAC_WRITE(((dst_x+dst_w)&0xff), PM3RD_VideoOverlayXEndLow);
	RAMDAC_WRITE(((dst_x+dst_w)&0xf00)>>8,PM3RD_VideoOverlayXEndHigh);
	pPPriv->ramdac_x = dst_x;
	pPPriv->ramdac_w = dst_w;
    }
    if ((pPPriv->ramdac_y != dst_y) || (pPPriv->ramdac_h != dst_h)) {
	RAMDAC_WRITE((dst_y&0xff), PM3RD_VideoOverlayYStartLow); 
	RAMDAC_WRITE((dst_y&0xf00)>>8, PM3RD_VideoOverlayYStartHigh);
	RAMDAC_WRITE(((dst_y+dst_h)&0xff), PM3RD_VideoOverlayYEndLow); 
	RAMDAC_WRITE(((dst_y+dst_h)&0xf00)>>8,PM3RD_VideoOverlayYEndHigh);
	pPPriv->ramdac_y = dst_y;
	pPPriv->ramdac_h = dst_h;
    }
    
    if (!pPPriv->ramdac_on) {
	if (pPPriv->OverlayAlpha<(3<<6)) {
	    RAMDAC_WRITE(pPPriv->OverlayAlpha, PM3RD_VideoOverlayBlend);
	    RAMDAC_WRITE(PM3RD_VideoOverlayControl_ENABLE |
		PM3RD_VideoOverlayControl_MODE_BLEND |
		PM3RD_VideoOverlayControl_BLENDSRC_REGISTER,
		PM3RD_VideoOverlayControl);
	} else {
#if SUPPORT_CLIPPING
	    switch (pScrn->depth) {
		case 8:
		case 16:
		    RAMDAC_WRITE((pPPriv->ColorKey&0xff0000)>>16,
			PM3RD_VideoOverlayKeyR);
		    RAMDAC_WRITE((pPPriv->ColorKey&0x00ff00)>>8,
			PM3RD_VideoOverlayKeyG);
		    RAMDAC_WRITE(pPPriv->ColorKey&0x0000ff,
			PM3RD_VideoOverlayKeyB);
		    RAMDAC_WRITE(PM3RD_VideoOverlayControl_ENABLE |
			PM3RD_VideoOverlayControl_MODE_MAINKEY |
			PM3RD_VideoOverlayControl_KEY_COLOR,
			PM3RD_VideoOverlayControl);
		    break;
		case 15:
		    RAMDAC_WRITE(0x1, PM3RD_VideoOverlayKeyR);
		    RAMDAC_WRITE(PM3RD_VideoOverlayControl_ENABLE |
			PM3RD_VideoOverlayControl_MODE_MAINKEY |
			PM3RD_VideoOverlayControl_KEY_ALPHA,
			PM3RD_VideoOverlayControl);
		    break;
 	    case 24:
		    RAMDAC_WRITE(0xff, PM3RD_VideoOverlayKeyR);
		    RAMDAC_WRITE(PM3RD_VideoOverlayControl_ENABLE |
			PM3RD_VideoOverlayControl_MODE_MAINKEY |
			PM3RD_VideoOverlayControl_KEY_ALPHA,
			PM3RD_VideoOverlayControl);
		    break;
	    }
#else
	RAMDAC_WRITE(PM3RD_VideoOverlayControl_ENABLE |
	    PM3RD_VideoOverlayControl_MODE_ALWAYS,
	    PM3RD_VideoOverlayControl);
#endif
	}
	pPPriv->ramdac_on = TRUE;
    }

    pPPriv->Buffer[pPPriv->display]->RemoveAreaCallback =
	RemoveBufferCallback;
    if (display != -1) pPPriv->copy = -1;
}

#if SUPPORT_CLIPPING

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
static void Clip (PortPrivPtr pPPriv, RegionPtr clipBoxes)
{
    AdaptorPrivPtr pAPriv = pPPriv->pAdaptor;
    ScrnInfoPtr pScrn = pAPriv->pScrn;

    /* Let's handle the clipping here. */
    if(!RegionsEqual(&pPPriv->clip, clipBoxes)) {
	REGION_COPY(pScrn->pScreen, &pPPriv->clip, clipBoxes);
	if (pPPriv->OverlayAlpha<(3<<6)) {
	     XAAFillSolidRects(pScrn, pPPriv->OverlayAlpha<<24, GXcopy,
		0xff000000, REGION_NUM_RECTS(clipBoxes),
		REGION_RECTS(clipBoxes));
	} else {
	    switch (pScrn->depth) {
		case 8:	/* CI8 */
		    XAAFillSolidRects(pScrn, pPPriv->ColorKey,
			GXcopy, 0xffffffff, REGION_NUM_RECTS(clipBoxes),
			REGION_RECTS(clipBoxes));
		    break;
		case 15:	/* RGB5551 */
		    XAAFillSolidRects(pScrn, 0xffffffff, GXcopy, 0x80008000,
			REGION_NUM_RECTS(clipBoxes), REGION_RECTS(clipBoxes));
		    break;
		case 16:	/* RGB565 */
		    XAAFillSolidRects(pScrn, pPPriv->ColorKey, GXcopy,
			0xffffffff, REGION_NUM_RECTS(clipBoxes),
			REGION_RECTS(clipBoxes));
		    break;
	        case 24:	/* RGB8888 */
		    XAAFillSolidRects(pScrn, 0xffffffff, GXcopy,
			0xff000000, REGION_NUM_RECTS(clipBoxes),
			REGION_RECTS(clipBoxes));
		    break;
	    }
	}
    }
}
#endif

static void
StopOverlay(PortPrivPtr pPPriv, int cleanup)
{
    AdaptorPrivPtr pAPriv = pPPriv->pAdaptor;
    ScrnInfoPtr pScrn = pAPriv->pScrn;
    GLINTPtr pGlint = GLINTPTR(pScrn);

    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3, "StopOverlay.\n"));
    /* Stop the Video Overlay in the RAMDAC */
    if (pPPriv->ramdac_on) {
	RAMDAC_WRITE(PM3RD_VideoOverlayControl_DISABLE,
	    PM3RD_VideoOverlayControl);
	pPPriv->ramdac_on = FALSE;
    }
    /* Stop the Video Overlay in the Video Overlay Unit */
    GLINT_WAIT(1);
    GLINT_WRITE_REG(PM3VideoOverlayMode_DISABLE,
	PM3VideoOverlayMode);
}
/* ReputImage is used if only the destination position or
 * the clipboxes change. */
static int
Permedia3ReputImage(ScrnInfoPtr pScrn,
    short drw_x, short drw_y,
    RegionPtr clipBoxes, pointer data)
{
    PortPrivPtr pPPriv = (PortPrivPtr) data;  

    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"ReputImage %d,%d.\n", drw_x, drw_y));

#if !SUPPORT_CLIPPING
    /* If the clip region is not a rectangle */
    if (REGION_SIZE(clipBoxes) != 0) {
	StopOverlay (pPPriv, FALSE);
	return Success;
    }
#endif

    /* If the buffer was freed, we cannot overlay it. */
    if (pPPriv->display == -1) {
	StopOverlay (pPPriv, FALSE);
	return Success;
    }

    /* Check that the dst area is some part of the visible screen. */
    if ((drw_x + pPPriv->dw) < pScrn->frameX0 ||
        (drw_y + pPPriv->dh) < pScrn->frameY0 ||
	drw_x > pScrn->frameX1 || drw_y > pScrn->frameY1) {
        return Success;
    }
    /* Copy the destinations coordinates */
    pPPriv->dx = drw_x;
    pPPriv->dy = drw_y;

#if SUPPORT_CLIPPING
    /* Clipping */
    Clip (pPPriv, clipBoxes);
#endif

    /* Restart the overlay */
    BeginOverlay(pPPriv, -1);

    return Success;
}

static int
Permedia3PutImage(ScrnInfoPtr pScrn,
    short src_x, short src_y, short drw_x, short drw_y,
    short src_w, short src_h, short drw_w, short drw_h,
    int id, unsigned char *buf, short width, short height,
    Bool sync, RegionPtr clipBoxes, pointer data)
{
    PortPrivPtr pPPriv = (PortPrivPtr) data;  
#if !USE_HARDWARE_COPY
    GLINTPtr pGlint = GLINTPTR(pScrn);
#endif
    int copy = -1;
    Bool copy_flat = TRUE;
    int w_bpp;

    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"PutImage %d,%d,%d,%d -> %d,%d,%d,%d "
	"id=0x%08x buf=%p w=%d h=%d sync=%d\n",
	src_x, src_y, src_w, src_h, drw_x, drw_y, drw_w, drw_h,
	id, buf, width, height, sync));

#if !SUPPORT_CLIPPING
    /* If the clip region is not a rectangle */
    if (REGION_SIZE(clipBoxes) != 0) {
	StopOverlay (pPPriv, FALSE);
	return Success;
    }
#endif

    /* Check that the src area to put is included in the buffer. */
    if ((src_x + src_w) > width ||
        (src_y + src_h) > height ||
	src_x < 0 || src_y < 0) {
	StopOverlay(pPPriv, FALSE);
        return BadValue;
    }

    /* Check that the dst area is some part of the visible screen. */
    if ((drw_x + drw_w) < pScrn->frameX0 ||
        (drw_y + drw_h) < pScrn->frameY0 ||
	drw_x > pScrn->frameX1 || drw_y > pScrn->frameY1) {
	StopOverlay(pPPriv, FALSE);
        return Success;
    }

    /* Copy the source and destinations coordinates and size */
    pPPriv->vx = src_x;
    pPPriv->vy = src_y;
    pPPriv->vw = src_w;
    pPPriv->vh = src_h;

    pPPriv->dx = drw_x;
    pPPriv->dy = drw_y;
    pPPriv->dw = drw_w;
    pPPriv->dh = drw_h;

    /* If the image format changed since a previous call,
     * let's check if it is supported. By default, we suppose that
     * the previous image format was ScalerImages[0].id */
    if (id != pPPriv->Id) {
	int i;
	for (i = 0; i < ENTRIES(ScalerImages); i++)
	    if (id == ScalerImages[i].id)
		break;
	if (i >= ENTRIES(ScalerImages))
	    return XvBadAlloc;
	pPPriv->Id = id;
    }

    /* Let's find the image format and Video_Shift values */
    switch (id) {
	case LE4CC('Y','V','1','2'):
	    pPPriv->Format = FORMAT_YUV422;
	    pPPriv->Video_Shift = 1;
	    copy_flat = FALSE;
	    break;
	case LE4CC('Y','U','Y','2'):
	    pPPriv->Format = FORMAT_YUV422;
	    pPPriv->Video_Shift = 1;
	    break;
	case LE4CC('U','Y','V','Y'):
	    pPPriv->Format = FORMAT_VUY422;
	    pPPriv->Video_Shift = 1;
	    break;
	case LE4CC('Y','U','V','A'):
	    pPPriv->Format = FORMAT_YUV444;
	    pPPriv->Video_Shift = 2;
	    break;
	case LE4CC('V','U','Y','A'):
	    pPPriv->Format = FORMAT_VUY444;
	    pPPriv->Video_Shift = 2;
	    break;
	case 0x41: /* RGBA 8:8:8:8 */
	    pPPriv->Format = FORMAT_RGB8888;
	    pPPriv->Video_Shift = 2;
	    break;
	case 0x42: /* RGB 5:6:5 */
	    pPPriv->Format = FORMAT_RGB565;
	    pPPriv->Video_Shift = 1;
	    break;
	case 0x43: /* RGB 1:5:5:5 */
	    pPPriv->Format = FORMAT_RGB5551;
	    pPPriv->Video_Shift = 1;
	    break;
	case 0x44: /* RGB 4:4:4:4 */
	    pPPriv->Format = FORMAT_RGB4444;
	    pPPriv->Video_Shift = 1;
	    break;
	case 0x46: /* RGB 2:3:3 */
	    pPPriv->Format = FORMAT_RGB332;
	    pPPriv->Video_Shift = 0;
	    break;
	case 0x47: /* BGRA 8:8:8:8 */
	    pPPriv->Format = FORMAT_BGR8888;
	    pPPriv->Video_Shift = 2;
	    break;
	case 0x48: /* BGR 5:6:5 */
	    pPPriv->Format = FORMAT_BGR565;
	    pPPriv->Video_Shift = 1;
	    break;
	case 0x49: /* BGR 1:5:5:5 */
	    pPPriv->Format = FORMAT_BGR5551;
	    pPPriv->Video_Shift = 1;
	    break;
	case 0x4A: /* BGR 4:4:4:4 */
	    pPPriv->Format = FORMAT_BGR4444;
	    pPPriv->Video_Shift = 1;
	    break;
	case 0x4C: /* BGR 2:3:3 */
	    pPPriv->Format = FORMAT_BGR332;
	    pPPriv->Video_Shift = 0;
	    break;
	    default:
	    return XvBadAlloc;
    }

    /* Now we allocate a buffer, if it is needed */
    w_bpp = (width << pPPriv->Video_Shift) >> pPPriv->FB_Shift;
    if ((copy = AllocateBuffers(pPPriv, w_bpp, height)) == -1)
	return XvBadAlloc;
    
    /* Let's copy the image to the framebuffer */
#if USE_HARDWARE_COPY
    /* Erm, ... removing this log message will make the server crash. */
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"Hardware image upload.\n"));
    HWCopySetup(pPPriv, pPPriv->Buffer[copy]->box.x1,
	pPPriv->Buffer[copy]->box.y1, w_bpp, height);
    if (copy_flat) HWCopyFlat(pPPriv, buf, width, height);
    else HWCopyYV12(pPPriv, buf, width, height);
#else
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"Software image upload (1).\n"));
    pPPriv->Sync(pScrn);
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"Software image upload (2).\n"));
    if (copy_flat) CopyFlat(buf,
	(CARD8 *) pGlint->FbBase + pPPriv->BufferBase[copy],
	width << pPPriv->FB_Shift, height,
	pScrn->displayWidth << pPPriv->FB_Shift);
    else CopyYV12(buf,
	(CARD32 *)((CARD8 *) pGlint->FbBase + pPPriv->BufferBase[copy]),
	width, height, pScrn->displayWidth << pPPriv->FB_Shift);
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"Software image upload (3).\n"));
    pPPriv->Sync(pScrn);
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"Software image upload (4).\n"));
#endif

#if SUPPORT_CLIPPING
    /* Clipping*/
    Clip (pPPriv, clipBoxes);
#endif

    /* We start the overlay */
    BeginOverlay(pPPriv, copy);	

    /* We sync the chip again (if needed). */
    if (sync) pPPriv->Sync(pScrn);

    return Success;
}

static void
Permedia3StopVideo(ScrnInfoPtr pScrn, pointer data, Bool cleanup)
{
    PortPrivPtr pPPriv = (PortPrivPtr) data;  

    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"StopVideo : exit=%d\n", cleanup));

    REGION_EMPTY(pScrn->pScreen, &pPPriv->clip);
    StopOverlay(pPPriv, cleanup);

    if (cleanup) {
	FreeBuffers(pPPriv, FALSE);
    }
}

static int
Permedia3SetPortAttribute(ScrnInfoPtr pScrn,
    Atom attribute, INT32 value, pointer data)
{
    PortPrivPtr pPPriv = (PortPrivPtr) data;

    if (attribute == xvFilter) {
	switch (value) {
	    case 0:	/* No Filtering */
		pPPriv->OverlayMode =
		    (pPPriv->OverlayMode &
		      ~PM3VideoOverlayMode_FILTER_MASK) |
		    PM3VideoOverlayMode_FILTER_OFF;
		break;
	    case 1:	/* Partial Filtering (X axis only) */ 
		pPPriv->OverlayMode =
		    (pPPriv->OverlayMode &
		      ~PM3VideoOverlayMode_FILTER_MASK) |
		    PM3VideoOverlayMode_FILTER_PARTIAL;
		break;
	    case 2:	/* Full Bilinear Filtering */
		/* We have to disable X mirroring also */
		pPPriv->OverlayMode =
		    (pPPriv->OverlayMode &
		     ~(PM3VideoOverlayMode_FILTER_MASK |
		       PM3VideoOverlayMode_MIRRORX_ON)) |
		    PM3VideoOverlayMode_FILTER_FULL;
		pPPriv->Attribute[1] &= 2;
		break;
	    default:
		return BadValue;
	}
	pPPriv->Attribute[0] = value;
    }
    else if (attribute == xvMirror) {
	switch (value) {
	    case 0:	/* No Mirroring */
		pPPriv->OverlayMode =
		    (pPPriv->OverlayMode &
		      ~PM3VideoOverlayMode_MIRROR_MASK) |
		    PM3VideoOverlayMode_MIRRORX_OFF |
		    PM3VideoOverlayMode_MIRRORY_OFF;
		break;
	    case 1:	/* X Axis Mirroring */
		/* If full filtering was enabled, rever to partial. */
		if (pPPriv->Attribute[0] == 2) {
		    pPPriv->OverlayMode =
			(pPPriv->OverlayMode &
			 ~(PM3VideoOverlayMode_MIRROR_MASK |
			   PM3VideoOverlayMode_FILTER_MASK)) |
			PM3VideoOverlayMode_MIRRORX_ON |
			PM3VideoOverlayMode_MIRRORY_OFF |
			PM3VideoOverlayMode_FILTER_PARTIAL;
		    pPPriv->Attribute[0] = 1; 
		} else {
		    pPPriv->OverlayMode =
			(pPPriv->OverlayMode &
			 ~PM3VideoOverlayMode_MIRROR_MASK) |
			PM3VideoOverlayMode_MIRRORX_ON |
			PM3VideoOverlayMode_MIRRORY_OFF;
		}
		break;
	    case 2:	/* Y Axis Mirroring */
		pPPriv->OverlayMode =
		    (pPPriv->OverlayMode &
		      ~PM3VideoOverlayMode_MIRROR_MASK) |
		    PM3VideoOverlayMode_MIRRORX_OFF |
		    PM3VideoOverlayMode_MIRRORY_ON;
		break;
	    case 3:	/* X and Y Axis Mirroring */
		/* If full filtering was enabled, rever to partial. */
		if (pPPriv->Attribute[0] == 2) {
		    pPPriv->OverlayMode =
			(pPPriv->OverlayMode &
			 ~(PM3VideoOverlayMode_MIRROR_MASK |
			   PM3VideoOverlayMode_FILTER_MASK)) |
			PM3VideoOverlayMode_MIRRORX_ON |
			PM3VideoOverlayMode_MIRRORY_ON |
			PM3VideoOverlayMode_FILTER_PARTIAL;
		    pPPriv->Attribute[0] = 1; 
		} else {
		    pPPriv->OverlayMode =
			(pPPriv->OverlayMode &
			 ~PM3VideoOverlayMode_MIRROR_MASK) |
			PM3VideoOverlayMode_MIRRORX_ON |
			PM3VideoOverlayMode_MIRRORY_ON;
		}
		break;
	    default:
		return BadValue;
	}
	pPPriv->Attribute[1] = value;
    }
    else if (attribute == xvAlpha) {
	if (value >= 0 && value <= 3) {
	    pPPriv->OverlayAlpha = value << 6;
	} else return BadValue;
	pPPriv->Attribute[2] = value;
    }
    else return BadMatch;

    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"SPA attr=%d val=%d\n",
	attribute, value));

    return Success;
}

static int
Permedia3GetPortAttribute(ScrnInfoPtr pScrn, 
    Atom attribute, INT32 *value, pointer data)
{
    PortPrivPtr pPPriv = (PortPrivPtr) data;

    if (attribute == xvFilter)
	*value = pPPriv->Attribute[0];
    else if (attribute == xvMirror)
	*value = pPPriv->Attribute[1];
    else if (attribute == xvAlpha)
	*value = pPPriv->Attribute[2];
    else return BadMatch;

    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 3,
	"GPA attr=%d val=%d\n",
	attribute, *value));

    return Success;
}

static void
Permedia3QueryBestSize(ScrnInfoPtr pScrn, Bool motion,
    short vid_w, short vid_h, short drw_w, short drw_h,
    unsigned int *p_w, unsigned int *p_h, pointer data)
{
    unsigned int zoom_delta, shrink_delta, src_w;

    *p_w = drw_w;
    compute_scale_factor (&src_w, p_w, &shrink_delta, &zoom_delta);
    *p_h = drw_h;
}

static int
Permedia3QueryImageAttributes(ScrnInfoPtr pScrn,
    int id, unsigned short *width, unsigned short *height,
    int *pitches, int *offsets)
{
    int i, pitch;

    *width = CLAMP(*width, 1, 2047);
    *height = CLAMP(*height, 1, 2047);

    if (offsets)
	offsets[0] = 0;

    switch (id) {
    case LE4CC('Y','V','1','2'): /* Planar YVU 4:2:0 (emulated) */
	*width = CLAMP((*width + 1) & ~1, 2, 2046);
	*height = CLAMP((*height + 1) & ~1, 2, 2046);

	pitch = *width; /* luma component */

	if (offsets) {
	    offsets[1] = pitch * *height;
	    offsets[2] = offsets[1] + (offsets[1] >> 2);
	}

	if (pitches) {
	    pitches[0] = pitch;
	    pitches[1] = pitches[2] = pitch >> 1;
	}

	return pitch * *height * 3 / 2;

    case LE4CC('Y','U','Y','2'): /* Packed YUYV 4:2:2 */
    case LE4CC('U','Y','V','Y'): /* Packed UYVY 4:2:2 */
	*width = CLAMP((*width + 1) & ~1, 2, 2046);

	pitch = *width * 2;

	if (pitches)
	    pitches[0] = pitch;

	return pitch * *height;

    default:
	for (i = 0; i < ENTRIES(ScalerImages); i++)
	    if (ScalerImages[i].id == id)
		break;

	if (i >= ENTRIES(ScalerImages))
	    break;

	pitch = *width * (ScalerImages[i].bits_per_pixel >> 3);

	if (pitches)
	    pitches[0] = pitch;

	return pitch * *height;
    }

    return 0;
}

static void
DeleteAdaptorPriv(AdaptorPrivPtr pAPriv)
{
    FreeBuffers(pAPriv->pPort, FALSE);

    TimerFree(pAPriv->pPort->Timer);

    xfree(pAPriv);
}

static AdaptorPrivPtr
NewAdaptorPriv(ScrnInfoPtr pScrn)
{
    AdaptorPrivPtr pAPriv = (AdaptorPrivPtr) xcalloc(1, sizeof(AdaptorPrivRec));
    PortPrivPtr pPPriv = (PortPrivPtr) xcalloc(1, sizeof(PortPrivRec));
    GLINTPtr pGlint = GLINTPTR(pScrn);
    int i;

    if (!pAPriv) return NULL;
    pAPriv->pScrn = pScrn;
    if (!pPPriv) return NULL;
    pAPriv->pPort = pPPriv;

    
    /* We allocate a timer */
    if (!(pPPriv->Timer = TimerSet(NULL, 0, 0, TimerCallback, pPPriv))) {
	DeleteAdaptorPriv(pAPriv);
	return NULL;
    }

    pPPriv->pAdaptor = pAPriv;
    /* Sync */
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 2,
	"Sync is using : %sPermedia3Sync.\n",
	pGlint->MultiAperture?"Dual":""));
    if (pGlint->MultiAperture) pPPriv->Sync = DualPermedia3Sync;
    else pPPriv->Sync = Permedia3Sync;

    /* Framebuffer bpp shift */
    pPPriv->FB_Shift = pScrn->bitsPerPixel >> 4;

    /* Attributes */
    pPPriv->Attribute[0] = 2;	/* Full filtering enabled */
    pPPriv->Attribute[1] = 0;	/* No mirroring */
    pPPriv->Attribute[2] = 3;	/* Opaque overlay mode */
    pPPriv->ColorKey = 0;
    pPPriv->OverlayAlpha =
	PM3RD_VideoOverlayBlend_FACTOR_100_PERCENT;
    pPPriv->OverlayMode =
	PM3VideoOverlayMode_FILTER_FULL |
	PM3VideoOverlayMode_MIRRORX_OFF |
	PM3VideoOverlayMode_MIRRORY_OFF;

    /* Clipping */
    REGION_EMPTY(pScrn->pScreen, &pPPriv->clip);

    /* RAMDAC saved values */
    pPPriv->ramdac_x = 0;
    pPPriv->ramdac_w = 0;
    pPPriv->ramdac_y = 0;
    pPPriv->ramdac_h = 0;
    pPPriv->ramdac_on = FALSE;

    /* Buffers */
    pPPriv->Id = ScalerImages[0].id;
    pPPriv->copy = -1;
    pPPriv->display = -1;
    for (i = 0; i < MAX_BUFFERS; i++)
	pPPriv->Buffer[i] = NULL;

    /* Timer */
    pPPriv->StopDelay = -1;
    pPPriv->Delay = 125;
    pPPriv->Instant = 1000 / 25;

    return pAPriv;
}

/*
 *  Glint interface
 */

void
Permedia3VideoEnterVT(ScrnInfoPtr pScrn)
{
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 2, "Xv enter VT\n"));
}

void
Permedia3VideoLeaveVT(ScrnInfoPtr pScrn)
{
    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 2, "Xv leave VT\n"));
}

void
Permedia3VideoUninit(ScrnInfoPtr pScrn)
{
    AdaptorPrivPtr pAPriv, *ppAPriv;

    for (ppAPriv = &AdaptorPrivList; (pAPriv = *ppAPriv); ppAPriv = &(pAPriv->Next))
	if (pAPriv->pScrn == pScrn) {
	    *ppAPriv = pAPriv->Next;
	    DeleteAdaptorPriv(pAPriv);
	    break;
	}

    DEBUG(xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 2, "Xv cleanup\n"));
}

void
Permedia3VideoInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    GLINTPtr pGlint = GLINTPTR(pScrn);
    AdaptorPrivPtr pAPriv;
    DevUnion Private[1];
    XF86VideoAdaptorRec VAR;
    XF86VideoAdaptorPtr VARPtrs;

    switch (pGlint->Chipset) {
	case PCI_VENDOR_3DLABS_CHIP_PERMEDIA3:
	    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 1,
		"Using the Permedia3 chipset.\n");
            break;
	case PCI_VENDOR_3DLABS_CHIP_GAMMA:
	    if (pGlint->MultiChip == PCI_CHIP_PERMEDIA3) {
		xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 1,
		    "Using the Gamma chipset.\n");
		break;
	    }
	default:
	    xf86DrvMsgVerb(pScrn->scrnIndex, X_ERROR, 1,
		"No Xv support for chipset %d.\n", pGlint->Chipset);
            return;
    }

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 1,
	"Initializing Permedia3 Xv driver rev. 1\n");

    if (pGlint->NoAccel) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	    "Xv initialization failed : XAA is needed\n");
	return;
    }

    if (!(pAPriv = NewAdaptorPriv(pScrn))) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Xv initialization failed\n");
	return;
    }

    memset(&VAR, 0, sizeof(VAR));

    Private[0].ptr = (pointer) pAPriv->pPort;

    VARPtrs = &VAR;

    VAR.type = XvInputMask | XvWindowMask | XvImageMask;
    VAR.flags = VIDEO_OVERLAID_IMAGES | VIDEO_CLIP_TO_VIEWPORT;
    VAR.name = "Permedia 3 Frontend Scaler";
    VAR.nEncodings = ENTRIES(ScalerEncodings);
    VAR.pEncodings = ScalerEncodings;
    VAR.nFormats	= ENTRIES(ScalerVideoFormats);
    VAR.pFormats	= ScalerVideoFormats;
    VAR.nPorts = 1;
    VAR.pPortPrivates = &Private[0];
    VAR.nAttributes = ENTRIES(ScalerAttributes);
    VAR.pAttributes = ScalerAttributes;
    VAR.nImages	= ENTRIES(ScalerImages);
    VAR.pImages	= ScalerImages;

    VAR.PutVideo = NULL;
    VAR.PutStill = NULL;
    VAR.GetVideo = NULL;
    VAR.GetStill = NULL;
    VAR.StopVideo = Permedia3StopVideo;
    VAR.SetPortAttribute = Permedia3SetPortAttribute;
    VAR.GetPortAttribute = Permedia3GetPortAttribute;
    VAR.QueryBestSize = Permedia3QueryBestSize;
    VAR.PutImage = Permedia3PutImage;
    VAR.ReputImage = Permedia3ReputImage;
    VAR.QueryImageAttributes = Permedia3QueryImageAttributes;

    if (xf86XVScreenInit(pScreen, &VARPtrs, 1)) {
	xvFilter	= MAKE_ATOM(XV_FILTER);
	xvMirror	= MAKE_ATOM(XV_MIRROR);
	xvAlpha		= MAKE_ATOM(XV_ALPHA);

#if USE_HARDWARE_COPY
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	    "Xv frontend scaler enabled (HW)\n");
#else
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	    "Xv frontend scaler enabled (SW)\n");
#endif
    } else {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Xv initialization failed\n");
	DeleteAdaptorPriv(pAPriv);
    }
}

#endif /* XvExtension */
