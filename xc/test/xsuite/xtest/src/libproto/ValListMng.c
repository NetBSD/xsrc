/*
 
Copyright (c) 1990, 1991  X Consortium

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
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: ValListMng.c,v 1.10 94/04/17 21:01:34 rws Exp $
 */
/*
 *	Purpose:  routines to edit value lists in requests
 *
 */

/*
 * Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989 
 * Sequent Computer Systems, Inc.   All rights reserved.
 *  
 * This software is furnished under a license and may be used
 * only in accordance with the terms of that license and with the
 * inclusion of the above copyright notice.   This software may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.
 */

#include "XstlibInt.h"
#include "DataMove.h"

static int Ones();
static xReq * _Add_Masked_Value();
static xReq * _Del_Masked_Value();

/*
 *	Routine: Clear_Masked_Value - clears mask and deallocates value list
 *
 *	Input: reqp - pointer to original request
 *
 *	Output: 
 *
 *	Returns: pointer to modified request
 *
 *	Globals used:
 *
 *	Side Effects: invalidates pointer to original request
 *
 *	Methods: 
 *
 */

xReq *
Clear_Masked_Value (reqp)
xReq * reqp;
{
    unsigned long   nominal_size;

    switch (reqp -> reqType) {
	case X_CreateWindow: 
	    nominal_size = sizeof (xCreateWindowReq);
	    ((xCreateWindowReq *) reqp) -> length = nominal_size / 4;
	    ((xCreateWindowReq *) reqp) -> mask = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ChangeWindowAttributes: 
	    nominal_size = sizeof (xChangeWindowAttributesReq);
	    ((xChangeWindowAttributesReq *) reqp) -> length = nominal_size / 4;
	    ((xChangeWindowAttributesReq *) reqp) -> valueMask = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ConfigureWindow: 
	    nominal_size = sizeof (xConfigureWindowReq);
	    ((xConfigureWindowReq *) reqp) -> length = nominal_size / 4;
	    ((xConfigureWindowReq *) reqp) -> mask = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_CreateGC: 
	    nominal_size = sizeof (xCreateGCReq);
	    ((xCreateGCReq *) reqp) -> length = nominal_size / 4;
	    ((xCreateGCReq *) reqp) -> mask = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ChangeGC: 
	    nominal_size = sizeof (xChangeGCReq);
	    ((xChangeGCReq *) reqp) -> length = nominal_size / 4;
	    ((xChangeGCReq *) reqp) -> mask = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ChangeKeyboardControl: 
	    nominal_size = sizeof (xChangeKeyboardControlReq);
	    ((xChangeKeyboardControlReq *) reqp) -> length = nominal_size / 4;
	    ((xChangeKeyboardControlReq *) reqp) -> mask = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	default: 
	    DEFAULT_ERROR;
	    break;
    }
    return(reqp);
}

/*
 *	Routine: Add_Masked_Value - appends a value to a value list
 *
 *	Input: reqp - pointer to original request
 *             mask - mask with one bit set to indicate which value to add
 *             value - the value to add
 *
 *	Output:
 *
 *	Returns: pointer to modified request
 *
 *	Globals used:
 *
 *	Side Effects: invalidates pointer to original request
 *                    ORs mask into mask field in request
 *
 *	Methods: 
 *
 */

xReq *
Add_Masked_Value (reqp, mask, value)
xReq * reqp;
unsigned long mask;
unsigned long value;
{
    unsigned long   nominal_size;

    if (Ones(mask) != 1) {
	Log_Msg("Add_Masked_Value called with bad mask = 0x%08x\n",mask);
	Abort();
    }

    switch (reqp -> reqType) {
	case X_CreateWindow: 
	    nominal_size = sizeof (xCreateWindowReq);
	    reqp = _Add_Masked_Value(reqp,nominal_size,
		&((xCreateWindowReq *)reqp)->mask, NULL,
		mask,value);
	    break;
	case X_ChangeWindowAttributes: 
	    nominal_size = sizeof (xChangeWindowAttributesReq);
	    reqp = _Add_Masked_Value(reqp,nominal_size,
		&((xChangeWindowAttributesReq *)reqp)->valueMask, NULL,
		mask,value);
	    break;
	case X_ConfigureWindow: 			/* 16-bit mask! */
	    nominal_size = sizeof (xConfigureWindowReq);
	    reqp = _Add_Masked_Value(reqp,nominal_size,
		NULL, &((xConfigureWindowReq *)reqp)->mask,
		mask,value);
	    break;
	case X_CreateGC: 
	    nominal_size = sizeof (xCreateGCReq);
	    reqp = _Add_Masked_Value(reqp,nominal_size,
		&((xCreateGCReq *)reqp)->mask, NULL,
		mask,value);
	    break;
	case X_ChangeGC: 
	    nominal_size = sizeof (xChangeGCReq);
	    reqp = _Add_Masked_Value(reqp,nominal_size,
		&((xChangeGCReq *)reqp)->mask, NULL,
		mask,value);
	    break;
	case X_ChangeKeyboardControl: 
	    nominal_size = sizeof (xChangeKeyboardControlReq);
	    reqp = _Add_Masked_Value(reqp,nominal_size,
		&((xChangeKeyboardControlReq *)reqp)->mask, NULL,
		mask,value);
	    break;
	default: 
	    DEFAULT_ERROR;
	    break;
    }
    return(reqp);
}

/*
 *	Routine: _Add_Masked_Value - generic routine to add a single value
 *               to any request
 *
 *	Input: reqp - pointer to request
 *             nominal_size - size of the basic request (without value list)
 *             rmaskp32 - pointer to 32 bit mask of interest in request
 *             rmaskp16 - pointer to 16 bit mask in interest in request
 *             mask - mask with one bit set to indicate which value to add
 *             value - the value to add
 *
 *	Output:
 *
 *	Returns: pointer to modified request
 *
 *	Globals used:
 *
 *	Side Effects: invalidates pointer to original request
 *
 *	Methods: note: can only pass in ONE OF rmaskp32, rmaskp16
 *
 */

static xReq *
_Add_Masked_Value(reqp,nominal_size,rmaskp32,rmaskp16,mask,value)
xReq *reqp;
unsigned long nominal_size;
CARD32 *rmaskp32;
CARD16 *rmaskp16;
unsigned long mask;
unsigned long value;
{
    unsigned long rmask;
    unsigned long new_size;
    CARD32 *valuePtr;
    int before;		/* number of values before this one */
    int after;		/* number of values after this one */
    unsigned long bmask;	/* mask for the values before */
    unsigned long amask;	/* mask for the values after */

    rmask = (rmaskp32 != NULL) ? *rmaskp32 : (CARD32) *rmaskp16;

    bmask = mask - 1;
    before = Ones(rmask&bmask);

    amask = ~(mask | bmask);
    after = Ones(rmask&amask);

    if ((rmask&mask)==0) { /* this mask not there now */
	reqp->length += 1;	/* we're adding a new value */
	if (rmaskp32 != NULL)	*rmaskp32 |= mask;
	else			*rmaskp16 |= (CARD16) mask;

	reqp = (xReq *) Xstrealloc((char *)reqp,reqp->length<<2);

	valuePtr = (CARD32 *) (((char *) reqp) + nominal_size);
	valuePtr += before;		/* index down to this position */
	bcopy((char *)valuePtr,(char *)(valuePtr+1),(after<<2));
    }

    valuePtr = (CARD32 *) (((char *) reqp) + nominal_size);
    valuePtr += before;		/* index down to this position */
    *valuePtr = value;
    return(reqp);
}

/*
 *	Routine: Del_Masked_Value - deletes a value from a value list
 *
 *	Input: reqp - pointer to original request
 *             mask - mask with one bit set to indicate which value to delete
 *
 *	Output:
 *
 *	Returns: pointer to modified request
 *
 *	Globals used:
 *
 *	Side Effects: invalidates pointer to original request
 *
 *	Methods: 
 *
 */

xReq *
Del_Masked_Value (reqp, mask)
xReq * reqp;
unsigned long mask;
{
    unsigned long   nominal_size;

    if (Ones(mask) != 1) {
	Log_Msg("Del_Masked_Value called with bad mask = 0x%08x\n",mask);
	Abort();
    }

    switch (reqp -> reqType) {
	case X_CreateWindow: 
	    nominal_size = sizeof (xCreateWindowReq);
	    _Del_Masked_Value(reqp,nominal_size,
		&((xCreateWindowReq *)reqp)->mask, NULL, mask);
	    break;
	case X_ChangeWindowAttributes: 
	    nominal_size = sizeof (xChangeWindowAttributesReq);
	    _Del_Masked_Value(reqp,nominal_size,
		&((xChangeWindowAttributesReq *)reqp)->valueMask, NULL, mask);
	    break;
	case X_ConfigureWindow: 			/* 16-bit mask! */
	    nominal_size = sizeof (xConfigureWindowReq);
	    _Del_Masked_Value(reqp,nominal_size,
		NULL, &((xConfigureWindowReq *)reqp)->mask, mask);
	    break;
	case X_CreateGC: 
	    nominal_size = sizeof (xCreateGCReq);
	    _Del_Masked_Value(reqp,nominal_size,
		&((xCreateGCReq *)reqp)->mask, NULL, mask);
	    break;
	case X_ChangeGC: 
	    nominal_size = sizeof (xChangeGCReq);
	    _Del_Masked_Value(reqp,nominal_size,
		&((xChangeGCReq *)reqp)->mask, NULL, mask);
	    break;
	case X_ChangeKeyboardControl: 
	    nominal_size = sizeof (xChangeKeyboardControlReq);
	    _Del_Masked_Value(reqp,nominal_size,
		&((xChangeKeyboardControlReq *)reqp)->mask, NULL, mask);
	    break;
	default: 
	    DEFAULT_ERROR;
	    break;
    }
    return(reqp);
}

/*
 *	Routine: _Del_Masked_Value - generic routine to delete a single value
 *               from any request
 *
 *	Input: reqp - pointer to original request
 *             nominal_size - size of the basic request (without value list)
 *             rmaskp32 - pointer to 32 bit mask of interest in request
 *             rmaskp16 - pointer to 16 bit mask of interest in request
 *             mask - mask with one bit set to indicate which value to delete
 *
 *	Output:
 *
 *	Returns: pointer to modified request
 *
 *	Globals used:
 *
 *	Side Effects: invalidates pointer to original request
 *
 *	Methods: note: can only pass in ONE OF rmaskp32, rmaskp16
 *
 */

static xReq *
_Del_Masked_Value(reqp,nominal_size,rmaskp32,rmaskp16,mask)
xReq *reqp;
unsigned long nominal_size;
CARD32 *rmaskp32;
CARD16 *rmaskp16;
unsigned long mask;
{
    unsigned long rmask;
    unsigned long   new_size;
    CARD32 *valuePtr;
    int before;		/* number of values before this one */
    int after;		/* number of values after this one */
    unsigned long bmask;	/* mask for the values before */
    unsigned long amask;	/* mask for the values after */

    rmask = (rmaskp32 != NULL) ? *rmaskp32 : (CARD32) *rmaskp16;

    if ((rmask&mask)==0) {	/* not there to zap */
	return(reqp);
    }

    bmask = mask - 1;
    before = Ones(rmask&bmask);

    amask = ~(mask | bmask);
    after = Ones(rmask&amask);

    reqp->length -= 1;	/* we're deleting a value */
    if (rmaskp32 != NULL)	*rmaskp32 &= ~mask;
    else			*rmaskp16 &= ~((CARD16) mask);

    valuePtr = (CARD32 *) (((char *) reqp) + nominal_size);
    valuePtr += before;		/* index down to this position */
    bcopy((char *)(valuePtr+1),(char *)valuePtr,(after<<2));
    reqp = (xReq *) Xstrealloc((char *)reqp,reqp->length<<2);
    return(reqp);
}

/*
 *	Routine: Clear_Counted_Value - clears count of values in list and
 *               deallocates value list
 *
 *	Input: reqp - pointer to original request
 *
 *	Output:
 *
 *	Returns: pointer to modified request
 *
 *	Globals used:
 *
 *	Side Effects: invalidates pointer to original request
 *
 *	Methods:
 *
 */

xReq *
Clear_Counted_Value (reqp)
xReq * reqp;
{
    unsigned long   nominal_size;

    switch (reqp -> reqType) {
	case X_InternAtom: 
	    nominal_size = sizeof (xInternAtomReq);
	    ((xInternAtomReq *) reqp) -> length = nominal_size / 4;
	    ((xInternAtomReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ChangeProperty: 
	    nominal_size = sizeof (xChangePropertyReq);
	    ((xChangePropertyReq *) reqp) -> length = nominal_size / 4;
	    ((xChangePropertyReq *) reqp) -> nUnits = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_OpenFont: 
	    nominal_size = sizeof (xOpenFontReq);
	    ((xOpenFontReq *) reqp) -> length = nominal_size / 4;
	    ((xOpenFontReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_QueryTextExtents: 
	    nominal_size = sizeof (xQueryTextExtentsReq);
	    ((xQueryTextExtentsReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ListFonts: 
	    nominal_size = sizeof (xListFontsReq);
	    ((xListFontsReq *) reqp) -> length = nominal_size / 4;
	    ((xListFontsReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ListFontsWithInfo: 
	    nominal_size = sizeof (xListFontsWithInfoReq);
	    ((xListFontsWithInfoReq *) reqp) -> length = nominal_size / 4;
	    ((xListFontsWithInfoReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_SetFontPath: 
	    nominal_size = sizeof (xSetFontPathReq);
	    ((xSetFontPathReq *) reqp) -> length = nominal_size / 4;
	    ((xSetFontPathReq *) reqp) -> nFonts = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_SetDashes: 
	    nominal_size = sizeof (xSetDashesReq);
	    ((xSetDashesReq *) reqp) -> length = nominal_size / 4;
	    ((xSetDashesReq *) reqp) -> nDashes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_SetClipRectangles: 
	    nominal_size = sizeof (xSetClipRectanglesReq);
	    ((xSetClipRectanglesReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyPoint: 
	    nominal_size = sizeof (xPolyPointReq);
	    ((xPolyPointReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyLine: 
	    nominal_size = sizeof (xPolyLineReq);
	    ((xPolyLineReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolySegment: 
	    nominal_size = sizeof (xPolySegmentReq);
	    ((xPolySegmentReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyRectangle: 
	    nominal_size = sizeof (xPolyRectangleReq);
	    ((xPolyRectangleReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyArc: 
	    nominal_size = sizeof (xPolyArcReq);
	    ((xPolyArcReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_FillPoly: 
	    nominal_size = sizeof (xFillPolyReq);
	    ((xFillPolyReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyFillRectangle: 
	    nominal_size = sizeof (xPolyFillRectangleReq);
	    ((xPolyFillRectangleReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyFillArc: 
	    nominal_size = sizeof (xPolyFillArcReq);
	    ((xPolyFillArcReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PutImage: 
	    nominal_size = sizeof (xPutImageReq);
	    ((xPutImageReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyText8: 
	    nominal_size = sizeof (xPolyText8Req);
	    ((xPolyText8Req *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyText16: 
	    nominal_size = sizeof (xPolyText16Req);
	    ((xPolyText16Req *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ImageText8: 
	    nominal_size = sizeof (xImageText8Req);
	    ((xImageText8Req *) reqp) -> length = nominal_size / 4;
	    ((xImageText8Req *) reqp) -> nChars = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ImageText16: 
	    nominal_size = sizeof (xImageText16Req);
	    ((xImageText16Req *) reqp) -> length = nominal_size / 4;
	    ((xImageText16Req *) reqp) -> nChars = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_AllocNamedColor: 
	    nominal_size = sizeof (xAllocNamedColorReq);
	    ((xAllocNamedColorReq *) reqp) -> length = nominal_size / 4;
	    ((xAllocNamedColorReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_FreeColors: 
	    nominal_size = sizeof (xFreeColorsReq);
	    ((xFreeColorsReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_StoreColors: 
	    nominal_size = sizeof (xStoreColorsReq);
	    ((xStoreColorsReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_StoreNamedColor: 
	    nominal_size = sizeof (xStoreNamedColorReq);
	    ((xStoreNamedColorReq *) reqp) -> length = nominal_size / 4;
	    ((xStoreNamedColorReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_QueryColors: 
	    nominal_size = sizeof (xQueryColorsReq);
	    ((xQueryColorsReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_LookupColor: 
	    nominal_size = sizeof (xLookupColorReq);
	    ((xLookupColorReq *) reqp) -> length = nominal_size / 4;
	    ((xLookupColorReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_QueryExtension: 
	    nominal_size = sizeof (xQueryExtensionReq);
	    ((xQueryExtensionReq *) reqp) -> length = nominal_size / 4;
	    ((xQueryExtensionReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ChangeKeyboardMapping: 
	    nominal_size = sizeof (xChangeKeyboardMappingReq);
	    ((xChangeKeyboardMappingReq *) reqp) -> length = nominal_size / 4;
	    ((xChangeKeyboardMappingReq *) reqp) -> keyCodes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ChangeHosts: 
	    nominal_size = sizeof (xChangeHostsReq);
	    ((xChangeHostsReq *) reqp) -> length = nominal_size / 4;
	    ((xChangeHostsReq *) reqp) -> hostLength = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_RotateProperties: 
	    nominal_size = sizeof (xRotatePropertiesReq);
	    ((xRotatePropertiesReq *) reqp) -> length = nominal_size / 4;
	    ((xRotatePropertiesReq *) reqp) -> nAtoms = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_SetPointerMapping: 
	    nominal_size = sizeof (xSetPointerMappingReq);
	    ((xSetPointerMappingReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_SetModifierMapping: 
	    nominal_size = sizeof (xSetModifierMappingReq);
	    ((xSetModifierMappingReq *) reqp) -> length = nominal_size / 4;
	    ((xSetModifierMappingReq *) reqp) -> numKeyPerModifier = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	default: 
	    DEFAULT_ERROR;
	    break;
    }
    return(reqp);
}


/*
 *	Routine: Add_Counted_Value - increments count of values in list and
 *               adds one value to list
 *
 *	Input: reqp - pointer to original request
 *             value - the value to add
 *
 *	Output:
 *
 *	Returns: pointer to modified request
 *
 *	Globals used:
 *
 *	Side Effects: invalidates pointer to original request
 *
 *	Methods:
 *
 */

xReq *
Add_Counted_Value (reqp, value)
xReq * reqp;
unsigned long value;
{
    unsigned long   nominal_size;
    unsigned char   *valuePtr;
    unsigned long   valueLen;


    switch (reqp -> reqType) {
	case X_InternAtom: 
	    nominal_size = sizeof (xInternAtomReq);
	    ((xInternAtomReq *) reqp) -> nbytes++;
	    valueLen = ((xInternAtomReq *) reqp)->nbytes;

	    if (padup(nominal_size+valueLen) >
		(reqp->length<<2)) {	/* this causes overflow */
		reqp->length++;
		reqp =
		    (xReq *) Xstrealloc ((char *) reqp, reqp->length<<2);
	    }

	    valuePtr = (unsigned char *) (((char *) reqp) + nominal_size);
	    valuePtr += ((((xInternAtomReq *) reqp)->nbytes)-1);
	    *valuePtr = (unsigned char) value;
	    break;
	case X_ChangeProperty: 
	    nominal_size = sizeof (xChangePropertyReq);
	    ((xChangePropertyReq *) reqp) -> nUnits++;
	    valueLen = ((xChangePropertyReq *) reqp)->nUnits * 
		(((xChangePropertyReq *) reqp)->format/8);

	    if (padup(nominal_size+valueLen) >
		(reqp->length<<2)) {	/* this causes overflow */
		reqp->length++;
		reqp =
		    (xReq *) Xstrealloc ((char *) reqp, reqp->length<<2);
	    }

	    valuePtr = (unsigned char *) (((char *) reqp) + nominal_size);
	    valuePtr += (valueLen - (((xChangePropertyReq *)reqp)->format/8));
	    switch(((xChangePropertyReq *)reqp)->format) {
	    case 8:
		Set_Value1(&valuePtr,value);
		break;
	    case 16:
		Set_Value2(&valuePtr,value);
		break;
	    case 32:
		Set_Value4(&valuePtr,value);
		break;
	    }
	    break;
	case X_OpenFont: 
	    nominal_size = sizeof (xOpenFontReq);
	    ((xOpenFontReq *) reqp) -> nbytes++;
	    valueLen = ((xOpenFontReq *) reqp)->nbytes;
	    if (padup(nominal_size+valueLen) >
		(reqp->length<<2)) {	/* this causes overflow */
		reqp->length++;
		reqp =
		    (xReq *) Xstrealloc ((char *) reqp, reqp->length<<2);
	    }

	    valuePtr = (unsigned char *) (((char *) reqp) + nominal_size);
	    valuePtr += (valueLen - 1);
	    Set_Value1(&valuePtr,value);
	    break;
	case X_QueryTextExtents: 
	    nominal_size = sizeof (xQueryTextExtentsReq);
	    valueLen = (reqp->length<<2) - 
		(((xQueryTextExtentsReq *) reqp)->oddLength << 1);
	    valueLen += 2;

	    if (padup(nominal_size+valueLen) >
		(reqp->length<<2)) {	/* this causes overflow */
		reqp->length++;
		reqp =
		    (xReq *) Xstrealloc ((char *) reqp, reqp->length<<2);
	    }

	    valuePtr = (unsigned char *) (((char *) reqp) + nominal_size);
	    valuePtr += (valueLen - 2);
	    Set_Value2(&valuePtr,value);
	    if ((valueLen % 4) == 2) {
		((xQueryTextExtentsReq *) reqp)->oddLength = 1;
	    }
	    else {
		((xQueryTextExtentsReq *) reqp)->oddLength = 0;
	    }
	    break;
	case X_ListFonts: 
	    nominal_size = sizeof (xListFontsReq);
	    ((xListFontsReq *) reqp) -> nbytes++;
	    valueLen = ((xListFontsReq *) reqp)->nbytes;

	    if (padup(nominal_size+valueLen) >
		(reqp->length<<2)) {	/* this causes overflow */
		reqp->length++;
		reqp =
		    (xReq *) Xstrealloc ((char *) reqp, reqp->length<<2);
	    }

	    valuePtr = (unsigned char *) (((char *) reqp) + nominal_size);
	    valuePtr += (valueLen - 1);
	    Set_Value1(&valuePtr,value);
	    break;
	case X_ListFontsWithInfo: 
	    nominal_size = sizeof (xListFontsWithInfoReq);
	    ((xListFontsWithInfoReq *) reqp) -> nbytes++;
	    valueLen = ((xListFontsWithInfoReq *) reqp)->nbytes;

	    if (padup(nominal_size+valueLen) >
		(reqp->length<<2)) {	/* this causes overflow */
		reqp->length++;
		reqp =
		    (xReq *) Xstrealloc ((char *) reqp, reqp->length<<2);
	    }

	    valuePtr = (unsigned char *) (((char *) reqp) + nominal_size);
	    valuePtr += (valueLen - 1);
	    Set_Value1(&valuePtr,value);
	    break;
	case X_SetFontPath: 
	    /* BEWARE --- this leaves nFonts as a count of the CARD8's
		that were added. Caller has to reset nFonts to correct
		value.
	    */
	    nominal_size = sizeof (xSetFontPathReq);
	    ((xSetFontPathReq *) reqp) -> nFonts++;
	    valueLen = ((xSetFontPathReq *) reqp)->nFonts;

	    if (padup(nominal_size+valueLen) >
		(reqp->length<<2)) {	/* this causes overflow */
		reqp->length++;
		reqp =
		    (xReq *) Xstrealloc ((char *) reqp, reqp->length<<2);
	    }

	    valuePtr = (unsigned char *) (((char *) reqp) + nominal_size);
	    valuePtr += (valueLen - 1);
	    Set_Value1(&valuePtr,value);
	    break;
	case X_SetDashes: 
	    nominal_size = sizeof (xSetDashesReq);
	    ((xSetDashesReq *) reqp) -> nDashes++;
	    valueLen = ((xSetDashesReq *) reqp)->nDashes;

	    if (padup(nominal_size+valueLen) >
		(reqp->length<<2)) {	/* this causes overflow */
		reqp->length++;
		reqp =
		    (xReq *) Xstrealloc ((char *) reqp, reqp->length<<2);
	    }

	    valuePtr = (unsigned char *) (((char *) reqp) + nominal_size);
	    valuePtr += (valueLen - 1);
	    Set_Value1(&valuePtr,value);
	    break;
	case X_SetClipRectangles: 
	    nominal_size = sizeof (xSetClipRectanglesReq);
	    ((xSetClipRectanglesReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyPoint: 
	    nominal_size = sizeof (xPolyPointReq);
	    ((xPolyPointReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyLine: 
	    nominal_size = sizeof (xPolyLineReq);
	    ((xPolyLineReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolySegment: 
	    nominal_size = sizeof (xPolySegmentReq);
	    ((xPolySegmentReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyRectangle: 
	    nominal_size = sizeof (xPolyRectangleReq);
	    ((xPolyRectangleReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyArc: 
	    nominal_size = sizeof (xPolyArcReq);
	    ((xPolyArcReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_FillPoly: 
	    nominal_size = sizeof (xFillPolyReq);
	    ((xFillPolyReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyFillRectangle: 
	    nominal_size = sizeof (xPolyFillRectangleReq);
	    ((xPolyFillRectangleReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyFillArc: 
	    nominal_size = sizeof (xPolyFillArcReq);
	    ((xPolyFillArcReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PutImage: 
	    nominal_size = sizeof (xPutImageReq);
	    ((xPutImageReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyText8: 
	    nominal_size = sizeof (xPolyText8Req);
	    ((xPolyText8Req *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_PolyText16: 
	    nominal_size = sizeof (xPolyText16Req);
	    ((xPolyText16Req *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ImageText8: 
	    nominal_size = sizeof (xImageText8Req);
	    ((xImageText8Req *) reqp) -> length = nominal_size / 4;
	    ((xImageText8Req *) reqp) -> nChars = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ImageText16: 
	    nominal_size = sizeof (xImageText16Req);
	    ((xImageText16Req *) reqp) -> length = nominal_size / 4;
	    ((xImageText16Req *) reqp) -> nChars = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_AllocNamedColor: 
	    nominal_size = sizeof (xAllocNamedColorReq);
	    ((xAllocNamedColorReq *) reqp) -> length = nominal_size / 4;
	    ((xAllocNamedColorReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_FreeColors: 
	    nominal_size = sizeof (xFreeColorsReq);
	    ((xFreeColorsReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_StoreColors: 
	    nominal_size = sizeof (xStoreColorsReq);
	    ((xStoreColorsReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_StoreNamedColor: 
	    nominal_size = sizeof (xStoreNamedColorReq);
	    ((xStoreNamedColorReq *) reqp) -> length = nominal_size / 4;
	    ((xStoreNamedColorReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_QueryColors: 
	    nominal_size = sizeof (xQueryColorsReq);
	    ((xQueryColorsReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_LookupColor: 
	    nominal_size = sizeof (xLookupColorReq);
	    ((xLookupColorReq *) reqp) -> length = nominal_size / 4;
	    ((xLookupColorReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_QueryExtension: 
	    nominal_size = sizeof (xQueryExtensionReq);
	    ((xQueryExtensionReq *) reqp) -> length = nominal_size / 4;
	    ((xQueryExtensionReq *) reqp) -> nbytes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ChangeKeyboardMapping: 
	    nominal_size = sizeof (xChangeKeyboardMappingReq);
	    ((xChangeKeyboardMappingReq *) reqp) -> length = nominal_size / 4;
	    ((xChangeKeyboardMappingReq *) reqp) -> keyCodes = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_ChangeHosts: 
	    nominal_size = sizeof (xChangeHostsReq);
	    ((xChangeHostsReq *) reqp) -> length = nominal_size / 4;
	    ((xChangeHostsReq *) reqp) -> hostLength = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_RotateProperties: 
	    nominal_size = sizeof (xRotatePropertiesReq);
	    ((xRotatePropertiesReq *) reqp) -> length = nominal_size / 4;
	    ((xRotatePropertiesReq *) reqp) -> nAtoms = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_SetPointerMapping: 
	    nominal_size = sizeof (xSetPointerMappingReq);
	    ((xSetPointerMappingReq *) reqp) -> length = nominal_size / 4;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	case X_SetModifierMapping: 
	    nominal_size = sizeof (xSetModifierMappingReq);
	    ((xSetModifierMappingReq *) reqp) -> length = nominal_size / 4;
	    ((xSetModifierMappingReq *) reqp) -> numKeyPerModifier = 0;
	    reqp =
		(xReq *) Xstrealloc ((char *) reqp, nominal_size);
	    break;
	default: 
	    DEFAULT_ERROR;
	    break;
    }
    return(reqp);
}
/*
 *	swiped from the sample server code!!!
 *	
 *	obscure but useful
 */
/*
 *	Routine: Ones - figures out number of ones set in a mask
 *
 *	Input: mask - 
 *
 *	Output:
 *
 *	Returns: number of ones set
 *
 *	Globals used:
 *
 *	Side Effects:
 *
 *	Methods:
 *
 */

static int
Ones(mask)                /* HACKMEM 169 */
Mask mask;
{
    register int y;

    y = (mask >> 1) &033333333333;
    y = mask - y - ((y >>1) & 033333333333);
    return (((y + (y >> 3)) & 030707070707) % (unsigned int)077);
}


/*
 *	Routine: Add_Counted_Bytes - appends byte string to end of value
 *      list and stuffs length of byte string into appropriate field
 *      in request.
 *
 *	Input: reqp - pointer to original request
 *             bytep - pointer to the byte string
 *             nbytes - number of bytes in *bytep
 *
 *	Output:
 *
 *	Returns: pointer to modified request
 *
 *	Globals used:
 *
 *	Side Effects: invalidates pointer to original request
 *
 *	Methods:
 *
 */

xReq *
Add_Counted_Bytes (reqp, bytep, nbytes)
    xReq * reqp;
    unsigned char *bytep;
    int nbytes;
{
    unsigned long   nominal_size;
    unsigned char   *valuePtr;
    unsigned long   valueLen;

    switch (reqp -> reqType) {
	case X_ChangeHosts:
	    nominal_size = sizeof(xChangeHostsReq);
	    ((xChangeHostsReq *) reqp) -> hostLength = nbytes;

	    if (padup(nominal_size + nbytes) >
		(reqp->length<<2)) {	/* this causes overflow */
		reqp =
		    (xReq *) Xstrealloc ((char *) reqp, padup(nominal_size + nbytes));
	    }

	    reqp->length += (padup(nbytes))/4;
	    valuePtr = (unsigned char *) (((char *) reqp) + nominal_size);
	    bcopy(bytep, valuePtr, nbytes);
	    break;
	default: 
	    DEFAULT_ERROR;
	    break;
	}
    return(reqp);
}
