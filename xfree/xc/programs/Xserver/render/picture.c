/*
 * $XFree86: xc/programs/Xserver/render/picture.c,v 1.12 2000/12/07 23:54:04 keithp Exp $
 *
 * Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#include "misc.h"
#include "scrnintstr.h"
#include "os.h"
#include "regionstr.h"
#include "validate.h"
#include "windowstr.h"
#include "input.h"
#include "resource.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "servermd.h"
#include "picturestr.h"

int		PictureScreenPrivateIndex = -1;
int		PictureWindowPrivateIndex;
int		PictureGeneration;
RESTYPE		PictureType;
RESTYPE		PictFormatType;
RESTYPE		GlyphSetType;

Bool
PictureDestroyWindow (WindowPtr pWindow)
{
    ScreenPtr		pScreen = pWindow->drawable.pScreen;
    PicturePtr		pPicture;
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    Bool		ret;

    while ((pPicture = GetPictureWindow(pWindow)))
    {
	SetPictureWindow(pWindow, pPicture->pNext);
	FreeResource (pPicture->id, PictureType);
	FreePicture ((pointer) pPicture, pPicture->id);
    }
    pScreen->DestroyWindow = ps->DestroyWindow;
    ret = (*pScreen->DestroyWindow) (pWindow);
    ps->DestroyWindow = pScreen->DestroyWindow;
    pScreen->DestroyWindow = PictureDestroyWindow;
    return ret;
}

Bool
PictureCloseScreen (int index, ScreenPtr pScreen)
{
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    Bool                ret;

    pScreen->CloseScreen = ps->CloseScreen;
    ret = (*pScreen->CloseScreen) (index, pScreen);
    SetPictureScreen(pScreen, 0);
    xfree (ps->formats);
    xfree (ps);
    return ret;
}

PictFormatPtr
PictureCreateDefaultFormats (ScreenPtr pScreen, int *nformatp)
{
    int		    nformats;
    PictFormatPtr   pFormats;
    int		    i;

    nformats = 7;
    pFormats = (PictFormatPtr) xalloc (nformats * sizeof (PictFormatRec));
    if (!pFormats)
	return 0;
    i = 0;
    pFormats[i].id = FakeClientID (0);
    pFormats[i].type = PictTypeDirect;
    pFormats[i].depth = 32;
    pFormats[i].direct.red = 16;
    pFormats[i].direct.redMask = 0xff;
    pFormats[i].direct.green = 8;
    pFormats[i].direct.greenMask = 0xff;
    pFormats[i].direct.blue = 0;
    pFormats[i].direct.blueMask = 0xff;
    pFormats[i].direct.alpha = 24;
    pFormats[i].direct.alphaMask = 0xff;
    pFormats[i].pColormap = 0;
    i++;
    pFormats[i].id = FakeClientID (0);
    pFormats[i].type = PictTypeDirect;
    pFormats[i].depth = 8;
    pFormats[i].direct.red = 0;
    pFormats[i].direct.redMask = 0;
    pFormats[i].direct.green = 0;
    pFormats[i].direct.greenMask = 0;
    pFormats[i].direct.blue = 0;
    pFormats[i].direct.blueMask = 0;
    pFormats[i].direct.alpha = 0;
    pFormats[i].direct.alphaMask = 0xff;
    pFormats[i].pColormap = 0;
    i++;
    pFormats[i].id = FakeClientID (0);
    pFormats[i].type = PictTypeDirect;
    pFormats[i].depth = 24;
    pFormats[i].direct.red = 16;
    pFormats[i].direct.redMask = 0xff;
    pFormats[i].direct.green = 8;
    pFormats[i].direct.greenMask = 0xff;
    pFormats[i].direct.blue = 0;
    pFormats[i].direct.blueMask = 0xff;
    pFormats[i].direct.alpha = 0;
    pFormats[i].direct.alphaMask = 0x0;
    pFormats[i].pColormap = 0;
    i++;
    pFormats[i].id = FakeClientID (0);
    pFormats[i].type = PictTypeDirect;
    pFormats[i].depth = 16;
    pFormats[i].direct.red = 11;
    pFormats[i].direct.redMask = 0x1f;
    pFormats[i].direct.green = 5;
    pFormats[i].direct.greenMask = 0x3f;
    pFormats[i].direct.blue = 0;
    pFormats[i].direct.blueMask = 0x1f;
    pFormats[i].direct.alpha = 0;
    pFormats[i].direct.alphaMask = 0x0;
    pFormats[i].pColormap = 0;
    i++;
    pFormats[i].id = FakeClientID (0);
    pFormats[i].type = PictTypeDirect;
    pFormats[i].depth = 15;
    pFormats[i].direct.red = 10;
    pFormats[i].direct.redMask = 0x1f;
    pFormats[i].direct.green = 5;
    pFormats[i].direct.greenMask = 0x1f;
    pFormats[i].direct.blue = 0;
    pFormats[i].direct.blueMask = 0x1f;
    pFormats[i].direct.alpha = 0;
    pFormats[i].direct.alphaMask = 0x0;
    pFormats[i].pColormap = 0;
    i++;
    pFormats[i].id = FakeClientID (0);
    pFormats[i].type = PictTypeDirect;
    pFormats[i].depth = 16;
    pFormats[i].direct.red = 10;
    pFormats[i].direct.redMask = 0x1f;
    pFormats[i].direct.green = 5;
    pFormats[i].direct.greenMask = 0x1f;
    pFormats[i].direct.blue = 0;
    pFormats[i].direct.blueMask = 0x1f;
    pFormats[i].direct.alpha = 15;
    pFormats[i].direct.alphaMask = 0x1;
    pFormats[i].pColormap = 0;
    i++;
    pFormats[i].id = FakeClientID (0);
    pFormats[i].type = PictTypeDirect;
    pFormats[i].depth = 1;
    pFormats[i].direct.red = 0;
    pFormats[i].direct.redMask = 0;
    pFormats[i].direct.green = 0;
    pFormats[i].direct.greenMask = 0;
    pFormats[i].direct.blue = 0;
    pFormats[i].direct.blueMask = 0;
    pFormats[i].direct.alpha = 0;
    pFormats[i].direct.alphaMask = 0x1;
    pFormats[i].pColormap = 0;
    i++;
    *nformatp = i;
    return pFormats;
}

PictFormatPtr
PictureMatchVisual (ScreenPtr pScreen, int depth, VisualPtr pVisual)
{
    PictureScreenPtr    ps = GetPictureScreenIfSet(pScreen);
    PictFormatPtr	format;
    int			nformat;
    int			type;

    if (!ps)
	return 0;
    format = ps->formats;
    nformat = ps->nformats;
    switch (pVisual->class) {
    case StaticGray:
    case GrayScale:
    case StaticColor:
    case PseudoColor:
	type = PictTypeIndexed;
	break;
    case TrueColor:
	type = PictTypeDirect;
	break;
    case DirectColor:
    default:
	return 0;
    }
    while (nformat--)
    {
	if (format->depth == depth && format->type == type)
	{
	    if (type == PictTypeIndexed)
	    {
		if (format->pColormap && format->pColormap->pVisual == pVisual)
		    return format;
	    }
	    else
	    {
		if (format->direct.redMask << format->direct.red == 
		    pVisual->redMask &&
		    format->direct.greenMask << format->direct.green == 
		    pVisual->greenMask &&
		    format->direct.blueMask << format->direct.blue == 
		    pVisual->blueMask)
		{
		    return format;
		}
	    }
	}
	format++;
    }
    return 0;
}

PictFormatPtr
PictureMatchFormat (ScreenPtr pScreen, int depth, CARD32 f)
{
    PictureScreenPtr    ps = GetPictureScreenIfSet(pScreen);
    PictFormatPtr	format;
    int			nformat;

    if (!ps)
	return 0;
    format = ps->formats;
    nformat = ps->nformats;
    while (nformat--)
    {
	if (format->depth == depth && format->format == (f & 0xffffff))
	    return format;
	format++;
    }
    return 0;
}

Bool
PictureInit (ScreenPtr pScreen, PictFormatPtr formats, int nformats)
{
    PictureScreenPtr	ps;
    int			n;
    CARD32		type, a, r, g, b;
    
    if (PictureGeneration != serverGeneration)
    {
	PictureType = CreateNewResourceType (FreePicture);
	if (!PictureType)
	    return FALSE;
	PictFormatType = CreateNewResourceType (FreePictFormat);
	if (!PictFormatType)
	    return FALSE;
	GlyphSetType = CreateNewResourceType (FreeGlyphSet);
	if (!GlyphSetType)
	    return FALSE;
	PictureScreenPrivateIndex = AllocateScreenPrivateIndex();
	if (PictureScreenPrivateIndex < 0)
	    return FALSE;
	PictureWindowPrivateIndex = AllocateWindowPrivateIndex();
	PictureGeneration = serverGeneration;
    }
    if (!AllocateWindowPrivate (pScreen, PictureWindowPrivateIndex, 0))
	return FALSE;
    
    if (!formats)
    {
	formats = PictureCreateDefaultFormats (pScreen, &nformats);
	if (!formats)
	    return FALSE;
    }
    for (n = 0; n < nformats; n++)
    {
	if (!AddResource (formats[n].id, PictFormatType, (pointer) (formats+n)))
	{
	    xfree (formats);
	    return FALSE;
	}
	if (formats[n].type == PictTypeIndexed)
	{
	    type = PICT_TYPE_INDEX;
	    a = r = g = b = 0;
	}
	else
	{
	    if ((formats[n].direct.redMask|
		 formats[n].direct.blueMask|
		 formats[n].direct.greenMask) == 0)
		type = PICT_TYPE_A;
	    else if (formats[n].direct.red > formats[n].direct.blue)
		type = PICT_TYPE_ARGB;
	    else
		type = PICT_TYPE_ABGR;
	    a = Ones (formats[n].direct.alphaMask);
	    r = Ones (formats[n].direct.redMask);
	    g = Ones (formats[n].direct.greenMask);
	    b = Ones (formats[n].direct.blueMask);
	}
	formats[n].format = PICT_FORMAT(0,type,a,r,g,b);
    }
    ps = (PictureScreenPtr) xalloc (sizeof (PictureScreenRec));
    if (!ps)
    {
	xfree (formats);
	return FALSE;
    }
    SetPictureScreen(pScreen, ps);
    if (!GlyphInit (pScreen))
    {
	SetPictureScreen(pScreen, 0);
	xfree (formats);
	xfree (ps);
	return FALSE;
    }

    ps->totalPictureSize = sizeof (PictureRec);
    ps->PicturePrivateSizes = 0;
    ps->PicturePrivateLen = 0;
    
    ps->formats = formats;
    ps->fallback = formats;
    ps->nformats = nformats;
    
    ps->CloseScreen = pScreen->CloseScreen;
    ps->DestroyWindow = pScreen->DestroyWindow;
    pScreen->DestroyWindow = PictureDestroyWindow;
    pScreen->CloseScreen = PictureCloseScreen;

    return TRUE;
}

void
SetPictureToDefaults (PicturePtr    pPicture)
{
    pPicture->refcnt = 1;
    pPicture->repeat = 0;
    pPicture->graphicsExposures = FALSE;
    pPicture->subWindowMode = ClipByChildren;
    pPicture->polyEdge = PolyEdgeSharp;
    pPicture->polyMode = PolyModePrecise;
    pPicture->freeCompClip = FALSE;
    pPicture->clientClipType = CT_NONE;

    pPicture->alphaMap = 0;
    pPicture->alphaOrigin.x = 0;
    pPicture->alphaOrigin.y = 0;

    pPicture->clipOrigin.x = 0;
    pPicture->clipOrigin.y = 0;
    pPicture->clientClip = 0;

    pPicture->dither = None;
    pPicture->serialNumber = GC_CHANGE_SERIAL_BIT;
    pPicture->stateChanges = (1 << (CPLastBit+1)) - 1;
}

PicturePtr
AllocatePicture (ScreenPtr  pScreen)
{
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    PicturePtr		pPicture;
    char		*ptr;
    DevUnion		*ppriv;
    unsigned int    	*sizes;
    unsigned int    	size;
    int			i;

    pPicture = (PicturePtr) xalloc (ps->totalPictureSize);
    if (!pPicture)
	return 0;
    ppriv = (DevUnion *)(pPicture + 1);
    pPicture->devPrivates = ppriv;
    sizes = ps->PicturePrivateSizes;
    ptr = (char *)(ppriv + ps->PicturePrivateLen);
    for (i = ps->PicturePrivateLen; --i >= 0; ppriv++, sizes++)
    {
	if ( (size = *sizes) )
	{
	    ppriv->ptr = (pointer)ptr;
	    ptr += size;
	}
	else
	    ppriv->ptr = (pointer)NULL;
    }
    return pPicture;
}

PicturePtr
CreatePicture (Picture		pid,
	       DrawablePtr	pDrawable,
	       PictFormatPtr	pFormat,
	       Mask		vmask,
	       XID		*vlist,
	       ClientPtr	client,
	       int		*error)
{
    PicturePtr		pPicture;
    PictureScreenPtr	ps = GetPictureScreen(pDrawable->pScreen);

    pPicture = AllocatePicture (pDrawable->pScreen);
    if (!pPicture)
    {
	*error = BadAlloc;
	return 0;
    }

    pPicture->id = pid;
    pPicture->pDrawable = pDrawable;
    pPicture->pFormat = pFormat;
    pPicture->format = pFormat->format | (pDrawable->bitsPerPixel << 24);
    if (pDrawable->type == DRAWABLE_PIXMAP)
    {
	++((PixmapPtr)pDrawable)->refcnt;
	pPicture->pNext = 0;
    }
    else
    {
	pPicture->pNext = GetPictureWindow(((WindowPtr) pDrawable));
	SetPictureWindow(((WindowPtr) pDrawable), pPicture);
    }

    SetPictureToDefaults (pPicture);
    
    if (vmask)
	*error = ChangePicture (pPicture, vmask, vlist, 0, client);
    else
	*error = Success;
    if (*error == Success)
	*error = (*ps->CreatePicture) (pPicture);
    if (*error != Success)
    {
	FreePicture (pPicture, (XID) 0);
	pPicture = 0;
    }
    return pPicture;
}

#define NEXT_VAL(_type) (vlist ? (_type) *vlist++ : (_type) ulist++->val)

#define NEXT_PTR(_type) ((_type) ulist++->ptr)

int
ChangePicture (PicturePtr	pPicture,
	       Mask		vmask,
	       XID		*vlist,
	       DevUnion		*ulist,
	       ClientPtr	client)
{
    ScreenPtr		pScreen = pPicture->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    BITS32		index2;
    int			error = 0;
    BITS32		maskQ;
    
    pPicture->serialNumber |= GC_CHANGE_SERIAL_BIT;
    maskQ = vmask;
    while (vmask && !error)
    {
	index2 = (BITS32) lowbit (vmask);
	vmask &= ~index2;
	pPicture->stateChanges |= index2;
	switch (index2)
	{
	case CPRepeat:
	    {
		unsigned int	newr;
		newr = NEXT_VAL(unsigned int);
		if (newr <= xTrue)
		    pPicture->repeat = newr;
		else
		{
		    client->errorValue = newr;
		    error = BadValue;
		}
	    }
	    break;
	case CPAlphaMap:
	    {
		PicturePtr  pAlpha;
		
		if (vlist)
		{
		    Picture	pid = NEXT_VAL(Picture);

		    if (pid == None)
			pAlpha = 0;
		    else
		    {
			pAlpha = (PicturePtr) SecurityLookupIDByType(client,
								     pid, 
								     PictureType, 
								     SecurityWriteAccess|SecurityReadAccess);
			if (!pAlpha)
			{
			    client->errorValue = pid;
			    error = BadPixmap;
			    break;
			}
			if (pAlpha->pDrawable->type != DRAWABLE_PIXMAP)
			{
			    client->errorValue = pid;
			    error = BadMatch;
			    break;
			}
		    }
		}
		else
		    pAlpha = NEXT_PTR(PicturePtr);
		if (!error)
		{
		    if (pAlpha && pAlpha->pDrawable->type == DRAWABLE_PIXMAP)
			pAlpha->refcnt++;
		    if (pPicture->alphaMap)
			FreePicture ((pointer) pPicture->alphaMap, (XID) 0);
		    pPicture->alphaMap = pAlpha;
		}
	    }
	    break;
	case CPAlphaXOrigin:
	    pPicture->alphaOrigin.x = NEXT_VAL(INT16);
	    break;
	case CPAlphaYOrigin:
	    pPicture->alphaOrigin.y = NEXT_VAL(INT16);
	    break;
	case CPClipXOrigin:
	    pPicture->clipOrigin.x = NEXT_VAL(INT16);
	    break;
	case CPClipYOrigin:
	    pPicture->clipOrigin.y = NEXT_VAL(INT16);
	    break;
	case CPClipMask:
	    {
		Pixmap	    pid;
		PixmapPtr   pPixmap;
		int	    clipType;

		if (vlist)
		{
		    pid = NEXT_VAL(Pixmap);
		    if (pid == None)
		    {
			clipType = CT_NONE;
			pPixmap = NullPixmap;
		    }
		    else
		    {
			clipType = CT_PIXMAP;
			pPixmap = (PixmapPtr)SecurityLookupIDByType(client,
								    pid, 
								    RT_PIXMAP,
								    SecurityReadAccess);
			if (!pPixmap)
			{
			    client->errorValue = pid;
			    error = BadPixmap;
			    break;
			}
		    }
		}
		else
		{
		    pPixmap = NEXT_PTR(PixmapPtr);
		    if (pPixmap)
			clipType = CT_PIXMAP;
		    else
			clipType = CT_NONE;
		}

		if (pPixmap)
		{
		    if ((pPixmap->drawable.depth != 1) ||
			(pPixmap->drawable.pScreen != pScreen))
		    {
			error = BadMatch;
			break;
		    }
		    else
		    {
			clipType = CT_PIXMAP;
			pPixmap->refcnt++;
		    }
		}
		error = (*ps->ChangePictureClip)(pPicture, clipType,
						 (pointer)pPixmap, 0);
		break;
	    }
	case CPGraphicsExposure:
	    {
		unsigned int	newe;
		newe = NEXT_VAL(unsigned int);
		if (newe <= xTrue)
		    pPicture->graphicsExposures = newe;
		else
		{
		    client->errorValue = newe;
		    error = BadValue;
		}
	    }
	    break;
	case CPSubwindowMode:
	    {
		unsigned int	news;
		news = NEXT_VAL(unsigned int);
		if (news == ClipByChildren || news == IncludeInferiors)
		    pPicture->subWindowMode = news;
		else
		{
		    client->errorValue = news;
		    error = BadValue;
		}
	    }
	    break;
	case CPPolyEdge:
	    {
		unsigned int	newe;
		newe = NEXT_VAL(unsigned int);
		if (newe == PolyEdgeSharp || newe == PolyEdgeSmooth)
		    pPicture->polyEdge = newe;
		else
		{
		    client->errorValue = newe;
		    error = BadValue;
		}
	    }
	    break;
	case CPPolyMode:
	    {
		unsigned int	newm;
		newm = NEXT_VAL(unsigned int);
		if (newm == PolyModePrecise || newm == PolyModeImprecise)
		    pPicture->polyMode = newm;
		else
		{
		    client->errorValue = newm;
		    error = BadValue;
		}
	    }
	    break;
	case CPDither:
	    pPicture->dither = NEXT_VAL(Atom);
	    break;
	case CPComponentAlpha:
	    {
		unsigned int	newca;

		newca = NEXT_VAL (unsigned int);
		if (newca <= xTrue)
		    pPicture->componentAlpha = newca;
		else
		{
		    client->errorValue = newca;
		    error = BadValue;
		}
	    }
	    break;
	default:
	    client->errorValue = maskQ;
	    error = BadValue;
	    break;
	}
    }
    (*ps->ChangePicture) (pPicture, maskQ);
    return error;
}

int
SetPictureClipRects (PicturePtr	pPicture,
		     int	xOrigin,
		     int	yOrigin,
		     int	nRect,
		     xRectangle	*rects)
{
    ScreenPtr		pScreen = pPicture->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    RegionPtr		clientClip;
    int			result;

    clientClip = RECTS_TO_REGION(pScreen,
				 nRect, rects, CT_UNSORTED);
    if (!clientClip)
	return BadAlloc;
    result =(*ps->ChangePictureClip) (pPicture, CT_REGION, 
				      (pointer) clientClip, 0);
    if (result == Success)
    {
	pPicture->clipOrigin.x = xOrigin;
	pPicture->clipOrigin.y = yOrigin;
	pPicture->stateChanges |= CPClipXOrigin|CPClipYOrigin|CPClipMask;
	pPicture->serialNumber |= GC_CHANGE_SERIAL_BIT;
    }
    return result;
}

void
ValidatePicture(PicturePtr pPicture)
{
    if (pPicture->serialNumber != pPicture->pDrawable->serialNumber)
    {
	PictureScreenPtr    ps = GetPictureScreen(pPicture->pDrawable->pScreen);

	(*ps->ValidatePicture) (pPicture, pPicture->stateChanges);
	pPicture->stateChanges = 0;
	pPicture->serialNumber = pPicture->pDrawable->serialNumber;
    }
}

int
FreePicture (pointer	value,
	     XID	pid)
{
    PicturePtr	pPicture = (PicturePtr) value;

    if (--pPicture->refcnt == 0)
    {
	ScreenPtr	    pScreen = pPicture->pDrawable->pScreen;
	PictureScreenPtr    ps = GetPictureScreen(pScreen);
	
	if (pPicture->alphaMap)
	    FreePicture ((pointer) pPicture->alphaMap, (XID) 0);
	(*ps->DestroyPicture) (pPicture);
	if (pPicture->pDrawable->type == DRAWABLE_WINDOW)
	{
	    WindowPtr	pWindow = (WindowPtr) pPicture->pDrawable;
	    PicturePtr	*pPrev;

	    for (pPrev = (PicturePtr *) &((pWindow)->devPrivates[PictureWindowPrivateIndex].ptr);
		 *pPrev;
		 pPrev = &(*pPrev)->pNext)
	    {
		if (*pPrev == pPicture)
		{
		    *pPrev = pPicture->pNext;
		    break;
		}
	    }
	}
	else if (pPicture->pDrawable->type == DRAWABLE_PIXMAP)
	{
	    (*pScreen->DestroyPixmap) ((PixmapPtr)pPicture->pDrawable);
	}
	xfree (pPicture);
    }
    return Success;
}

int
FreePictFormat (pointer	pPictFormat,
		XID     pid)
{
    return Success;
}

void
CompositePicture (CARD8		op,
		  PicturePtr	pSrc,
		  PicturePtr	pMask,
		  PicturePtr	pDst,
		  INT16		xSrc,
		  INT16		ySrc,
		  INT16		xMask,
		  INT16		yMask,
		  INT16		xDst,
		  INT16		yDst,
		  CARD16	width,
		  CARD16	height)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pSrc);
    if (pMask)
	ValidatePicture (pMask);
    ValidatePicture (pDst);
    (*ps->Composite) (op,
		       pSrc,
		       pMask,
		       pDst,
		       xSrc,
		       ySrc,
		       xMask,
		       yMask,
		       xDst,
		       yDst,
		       width,
		       height);
}

void
CompositeGlyphs (CARD8		op,
		 PicturePtr	pSrc,
		 PicturePtr	pDst,
		 PictFormatPtr	maskFormat,
		 INT16		xSrc,
		 INT16		ySrc,
		 int		nlist,
		 GlyphListPtr	lists,
		 GlyphPtr	*glyphs)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pSrc);
    ValidatePicture (pDst);
    (*ps->Glyphs) (op, pSrc, pDst, maskFormat, xSrc, ySrc, nlist, lists, glyphs);
}

void
CompositeRects (CARD8		op,
		PicturePtr	pDst,
		xRenderColor	*color,
		int		nRect,
		xRectangle      *rects)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pDst);
    (*ps->CompositeRects) (op, pDst, color, nRect, rects);
}
