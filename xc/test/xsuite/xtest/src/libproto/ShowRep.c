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
 * $XConsortium: ShowRep.c,v 1.9 94/04/17 21:01:28 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

#include "XstlibInt.h"
#include "DataMove.h"

void
Show_Rep(mmp,type,bytes_given)
xReply *mmp;
int type;
long bytes_given;
{
	unsigned long rep_says = (long)(mmp->generic.length<<2) + sizeof(xReply);
	unsigned long bytes_needed = (unsigned long)bytes_given;
	xReply *mp;
	char *valuePtr, *endPtr;
	int free_it = False;
	
	if (rep_says != bytes_needed) { /* ensure we've got enough room */
		unsigned int reqd = max(rep_says, bytes_needed);
		unsigned int cpyd = min(rep_says, bytes_needed);

		mp = (xReply *)Xstmalloc(reqd);
		bcopy((char *)mmp, (char *)mp, cpyd);
		bytes_needed = cpyd;
		free_it = True;
	} else
		mp = mmp;

	valuePtr = (char *) ((char *) mp + sizeof(xReply));
	endPtr = ((char *) mp) + bytes_needed;

	if (type > X_NoOperation) {
	    Show_Ext_Rep(mp, type, bytes_given);
	    if (free_it)
		Free_Reply(mp);
	    return;
	}

	switch (type) {
	case X_GetWindowAttributes:
		BPRINTF1("GetWindowAttributes:\n");
		BPRINTF2("\tbackingStore = %ld\n",(long) ((xGetWindowAttributesReply *)mp)->backingStore);
		BPRINTF2("\tsequenceNumber = %d\n",((xGetWindowAttributesReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetWindowAttributesReply *)mp)->length);
		BPRINTF2("\tvisualID = %ld\n",((xGetWindowAttributesReply *)mp)->visualID);
		BPRINTF2("\tclass = %ld\n",(long) ((xGetWindowAttributesReply *)mp)->class);
		BPRINTF2("\tbitGravity = %d\n",((xGetWindowAttributesReply *)mp)->bitGravity);
		BPRINTF2("\twinGravity = %d\n",((xGetWindowAttributesReply *)mp)->winGravity);
		BPRINTF2("\tbackingBitPlanes = %ld\n",((xGetWindowAttributesReply *)mp)->backingBitPlanes);
		BPRINTF2("\tbackingPixel = %ld\n",((xGetWindowAttributesReply *)mp)->backingPixel);
		BPRINTF2("\tsaveUnder = %d\n",((xGetWindowAttributesReply *)mp)->saveUnder);
		BPRINTF2("\tmapInstalled = %d\n",((xGetWindowAttributesReply *)mp)->mapInstalled);
		BPRINTF2("\tmapState = %ld\n",(long) ((xGetWindowAttributesReply *)mp)->mapState);
		BPRINTF2("\toverride = %d\n",((xGetWindowAttributesReply *)mp)->override);
		BPRINTF2("\tcolormap = %ld\n",((xGetWindowAttributesReply *)mp)->colormap);
		BPRINTF2("\tallEventMasks = 0x%08x\n",((xGetWindowAttributesReply *)mp)->allEventMasks);
		BPRINTF2("\tyourEventMask = 0x%08x\n",((xGetWindowAttributesReply *)mp)->yourEventMask);
		BPRINTF2("\tdoNotPropagateMask = 0x08x\n",((xGetWindowAttributesReply *)mp)->doNotPropagateMask);
		break;
	case X_GetGeometry:
		BPRINTF1("GetGeometry:\n");
		BPRINTF2("\tdepth = %d\n",((xGetGeometryReply *)mp)->depth);
		BPRINTF2("\tsequenceNumber = %d\n",((xGetGeometryReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetGeometryReply *)mp)->length);
		BPRINTF2("\troot = %ld\n",((xGetGeometryReply *)mp)->root);
		BPRINTF2("\tx = %d\n",((xGetGeometryReply *)mp)->x);
		BPRINTF2("\ty = %d\n",((xGetGeometryReply *)mp)->y);
		BPRINTF2("\twidth = %d\n",((xGetGeometryReply *)mp)->width);
		BPRINTF2("\theight = %d\n",((xGetGeometryReply *)mp)->height);
		BPRINTF2("\tborderWidth = %d\n",((xGetGeometryReply *)mp)->borderWidth);
		break;
	case X_QueryTree:
		BPRINTF1("QueryTree:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xQueryTreeReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryTreeReply *)mp)->length);
		BPRINTF2("\troot = %ld\n",((xQueryTreeReply *)mp)->root);
		BPRINTF2("\tparent = %ld\n",((xQueryTreeReply *)mp)->parent);
		BPRINTF2("\tnChildren = %ld\n",(long) ((xQueryTreeReply *)mp)->nChildren);
		Show_Value_List_Rep(mp,sizeof(xQueryTreeReply),FORMAT32);
		break;
	case X_InternAtom:
		BPRINTF1("InternAtom:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xInternAtomReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xInternAtomReply *)mp)->length);
		BPRINTF2("\tatom = %ld\n",((xInternAtomReply *)mp)->atom);
		break;
	case X_GetAtomName:
		BPRINTF1("GetAtomName:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xGetAtomNameReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetAtomNameReply *)mp)->length);
		BPRINTF2("\tnameLength = %ld\n",(long) ((xGetAtomNameReply *)mp)->nameLength);
		Show_String8(mp,sizeof(xGetAtomNameReply),((xGetAtomNameReply *)mp)->nameLength);
		break;
	case X_GetProperty:
		BPRINTF1("GetProperty:\n");
		BPRINTF2("\tformat = %d\n",((xGetPropertyReply *)mp)->format);
		BPRINTF2("\tsequenceNumber = %d\n",((xGetPropertyReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetPropertyReply *)mp)->length);
		BPRINTF2("\tbytesAfter = %ld\n",((xGetPropertyReply *)mp)->bytesAfter);
		BPRINTF2("\tnItems = %ld\n",((xGetPropertyReply *)mp)->nItems);
		Show_Value_List_Rep(mp,sizeof(xGetPropertyReply),((xGetPropertyReply *)mp)->format);
		break;
	case X_ListProperties:
		BPRINTF1("ListProperties:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xListPropertiesReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xListPropertiesReply *)mp)->length);
		BPRINTF2("\tnProperties = %ld\n",(long) ((xListPropertiesReply *)mp)->nProperties);
		Show_Value_List_Rep(mp,sizeof(xListPropertiesReply),FORMAT32);
		break;
	case X_GetSelectionOwner:
		BPRINTF1("GetSelectionOwner:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xGetSelectionOwnerReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetSelectionOwnerReply *)mp)->length);
		BPRINTF2("\towner = %ld\n",((xGetSelectionOwnerReply *)mp)->owner);
		break;
	case X_GrabPointer:
		BPRINTF1("GrabPointer:\n");
		BPRINTF2("\tstatus = %ld\n",(long) ((xGrabPointerReply *)mp)->status);
		BPRINTF2("\tsequenceNumber = %d\n",((xGrabPointerReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGrabPointerReply *)mp)->length);
		break;
	case X_GrabKeyboard:
		BPRINTF1("GrabKeyboard:\n");
		BPRINTF2("\tstatus = %ld\n",(long) ((xGrabKeyboardReply *)mp)->status);
		BPRINTF2("\tsequenceNumber = %d\n",((xGrabKeyboardReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGrabKeyboardReply *)mp)->length);
		break;
	case X_QueryPointer:
		BPRINTF1("QueryPointer:\n");
		BPRINTF2("\tsameScreen = %d\n",((xQueryPointerReply *)mp)->sameScreen);
		BPRINTF2("\tsequenceNumber = %d\n",((xQueryPointerReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryPointerReply *)mp)->length);
		BPRINTF2("\troot = %ld\n",((xQueryPointerReply *)mp)->root);
		BPRINTF2("\tchild = %ld\n",((xQueryPointerReply *)mp)->child);
		BPRINTF2("\trootX = %d\n",((xQueryPointerReply *)mp)->rootX);
		BPRINTF2("\trootY = %d\n",((xQueryPointerReply *)mp)->rootY);
		BPRINTF2("\twinX = %d\n",((xQueryPointerReply *)mp)->winX);
		BPRINTF2("\twinY = %d\n",((xQueryPointerReply *)mp)->winY);
		BPRINTF2("\tmask = 0x%04x\n",((xQueryPointerReply *)mp)->mask);
		break;
	case X_GetMotionEvents:
		BPRINTF1("GetMotionEvents:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xGetMotionEventsReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetMotionEventsReply *)mp)->length);
		BPRINTF2("\tnEvents = %ld\n",(long) ((xGetMotionEventsReply *)mp)->nEvents);
		Show_Value_List_Rep((xReq *)mp,sizeof(xGetMotionEventsReply), FORMATtimecoord);
		break;
	case X_TranslateCoords:
		BPRINTF1("TranslateCoords:\n");
		BPRINTF2("\tsameScreen = %d\n",((xTranslateCoordsReply *)mp)->sameScreen);
		BPRINTF2("\tsequenceNumber = %d\n",((xTranslateCoordsReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xTranslateCoordsReply *)mp)->length);
		BPRINTF2("\tchild = %ld\n",((xTranslateCoordsReply *)mp)->child);
		BPRINTF2("\tdstX = %d\n",((xTranslateCoordsReply *)mp)->dstX);
		BPRINTF2("\tdstY = %d\n",((xTranslateCoordsReply *)mp)->dstY);
		break;
	case X_GetInputFocus:
		BPRINTF1("GetInputFocus:\n");
		BPRINTF2("\trevertTo = %ld\n",(long) ((xGetInputFocusReply *)mp)->revertTo);
		BPRINTF2("\tsequenceNumber = %d\n",((xGetInputFocusReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetInputFocusReply *)mp)->length);
		BPRINTF2("\tfocus = %ld\n",((xGetInputFocusReply *)mp)->focus);
		break;
	case X_QueryKeymap:
		BPRINTF1("QueryKeymap:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xQueryKeymapReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryKeymapReply *)mp)->length);
		Show_Value_List_Rep(mp,sizeof(xQueryKeymapReply)-sizeof(((xQueryKeymapReply *)mp)->map),FORMAT8);
		break;
	case X_QueryFont: {
	        long nfontprops;
	        long ncharinfos;

		BPRINTF1("QueryFont:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xQueryFontReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryFontReply *)mp)->length);
		/* print the CHARINFO structure found at min-bounds */

		BPRINTF1("\tmin-bounds:\n");
		BPRINTF2("\t\tleft-side-bearing = %d\n",((xQueryFontReply *)mp)->minBounds.leftSideBearing);
		BPRINTF2("\t\tright-side-bearing = %d\n",((xQueryFontReply *)mp)->minBounds.rightSideBearing);
		BPRINTF2("\t\tcharacter-width = %d\n",((xQueryFontReply *)mp)->minBounds.characterWidth);
		BPRINTF2("\t\tascent = %d\n",((xQueryFontReply *)mp)->minBounds.ascent);
		BPRINTF2("\t\tdescent = %d\n",((xQueryFontReply *)mp)->minBounds.descent);
		BPRINTF2("\t\tattributes = 0x%x\n",((xQueryFontReply *)mp)->minBounds.attributes);

		/* end of CHARINFO structure */
		/* print the CHARINFO structure found at max-bounds */

		BPRINTF1("\tmax-bounds:\n");
		BPRINTF2("\t\tleft-side-bearing = %d\n",((xQueryFontReply *)mp)->maxBounds.leftSideBearing);
		BPRINTF2("\t\tright-side-bearing = %d\n",((xQueryFontReply *)mp)->maxBounds.rightSideBearing);
		BPRINTF2("\t\tcharacter-width = %d\n",((xQueryFontReply *)mp)->maxBounds.characterWidth);
		BPRINTF2("\t\tascent = %d\n",((xQueryFontReply *)mp)->maxBounds.ascent);
		BPRINTF2("\t\tdescent = %d\n",((xQueryFontReply *)mp)->maxBounds.descent);
		BPRINTF2("\t\tattributes = 0x%x\n",((xQueryFontReply *)mp)->maxBounds.attributes);

		/* end of CHARINFO structure */
		BPRINTF2("\tminCharOrByte2 = %d\n",((xQueryFontReply *)mp)->minCharOrByte2);
		BPRINTF2("\tmaxCharOrByte2 = %d\n",((xQueryFontReply *)mp)->maxCharOrByte2);
		BPRINTF2("\tdefaultChar = %d\n",((xQueryFontReply *)mp)->defaultChar);
		BPRINTF2("\tnFontProps = %ld\n",(long) ((xQueryFontReply *)mp)->nFontProps);
		BPRINTF2("\tdrawDirection = %ld\n",(long) ((xQueryFontReply *)mp)->drawDirection);
		BPRINTF2("\tminByte1 = %d\n",((xQueryFontReply *)mp)->minByte1);
		BPRINTF2("\tmaxByte1 = %d\n",((xQueryFontReply *)mp)->maxByte1);
		BPRINTF2("\tallCharsExist = %d\n",((xQueryFontReply *)mp)->allCharsExist);
		BPRINTF2("\tfontAscent = %d\n",((xQueryFontReply *)mp)->fontAscent);
		BPRINTF2("\tfontDescent = %d\n",((xQueryFontReply *)mp)->fontDescent);
		BPRINTF2("\tnCharInfos = %ld\n",(long) ((xQueryFontReply *)mp)->nCharInfos);
	        nfontprops = (long) ((xQueryFontReply *)mp)->nFontProps;
	        ncharinfos = (long) ((xQueryFontReply *)mp)->nCharInfos;
		Show_Value_List_nRep ( mp, nfontprops, sizeof(xQueryFontReply), FORMATfontprop);
		Show_Value_List_nRep ( mp, ncharinfos, sizeof(xQueryFontReply) + (nfontprops * 8), FORMATcharinfo);
	}
		break;
	case X_QueryTextExtents:
		BPRINTF1("QueryTextExtents:\n");
		BPRINTF2("\tdrawDirection = %ld\n",(long) ((xQueryTextExtentsReply *)mp)->drawDirection);
		BPRINTF2("\tsequenceNumber = %d\n",((xQueryTextExtentsReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryTextExtentsReply *)mp)->length);
		BPRINTF2("\tfontAscent = %d\n",((xQueryTextExtentsReply *)mp)->fontAscent);
		BPRINTF2("\tfontDescent = %d\n",((xQueryTextExtentsReply *)mp)->fontDescent);
		BPRINTF2("\toverallAscent = %d\n",((xQueryTextExtentsReply *)mp)->overallAscent);
		BPRINTF2("\toverallDescent = %d\n",((xQueryTextExtentsReply *)mp)->overallDescent);
		BPRINTF2("\toverallWidth = %ld\n",((xQueryTextExtentsReply *)mp)->overallWidth);
		BPRINTF2("\toverallLeft = %ld\n",((xQueryTextExtentsReply *)mp)->overallLeft);
		BPRINTF2("\toverallRight = %ld\n",((xQueryTextExtentsReply *)mp)->overallRight);
		break;
	case X_ListFonts:
		BPRINTF1("ListFonts:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xListFontsReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xListFontsReply *)mp)->length);
		BPRINTF2("\tnFonts = %d\n",((xListFontsReply *)mp)->nFonts);
		Show_String8(mp,sizeof(xListFontsReply),((xListFontsReply *)mp)->nFonts);
		break;
	case X_ListFontsWithInfo: {
	    long nfontprops;

		BPRINTF1("ListFontsWithInfo:\n");
		BPRINTF2("\tnameLength = %ld\n",(long) ((xListFontsWithInfoReply *)mp)->nameLength);
		    
		BPRINTF2("\tsequenceNumber = %d\n",((xListFontsWithInfoReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xListFontsWithInfoReply *)mp)->length);
		if (((xListFontsWithInfoReply *)mp)->nameLength == 0)
		    break;  /* last reply, so rest of structure unused */

		/* print the CHARINFO structure found at min-bounds */

		BPRINTF1("\tmin-bounds, ");
		BPRINTF2("left-side-bearing = %d, ",((xListFontsWithInfoReply *)mp)->minBounds.leftSideBearing);
		BPRINTF2("right-side-bearing = %d, ",((xListFontsWithInfoReply *)mp)->minBounds.rightSideBearing);
		BPRINTF2("character-width = %d, ",((xListFontsWithInfoReply *)mp)->minBounds.characterWidth);
		BPRINTF2("ascent = %d, ",((xListFontsWithInfoReply *)mp)->minBounds.ascent);
		BPRINTF2("descent = %d, ",((xListFontsWithInfoReply *)mp)->minBounds.descent);
		BPRINTF2("attributes = 0x%x\n",((xListFontsWithInfoReply *)mp)->minBounds.attributes);

		/* end of CHARINFO structure */

		/* print the CHARINFO structure found at max-bounds */

		BPRINTF1("\tmax-bounds, ");
		BPRINTF2("left-side-bearing = %d, ",((xListFontsWithInfoReply *)mp)->maxBounds.leftSideBearing);
		BPRINTF2("right-side-bearing = %d, ",((xListFontsWithInfoReply *)mp)->maxBounds.rightSideBearing);
		BPRINTF2("character-width = %d, ",((xListFontsWithInfoReply *)mp)->maxBounds.characterWidth);
		BPRINTF2("ascent = %d, ",((xListFontsWithInfoReply *)mp)->maxBounds.ascent);
		BPRINTF2("descent = %d, ",((xListFontsWithInfoReply *)mp)->maxBounds.descent);
		BPRINTF2("attributes = 0x%x\n",((xListFontsWithInfoReply *)mp)->maxBounds.attributes);

		/* end of CHARINFO structure */

		BPRINTF2("\tminCharOrByte2 = %d\n",((xListFontsWithInfoReply *)mp)->minCharOrByte2);
		BPRINTF2("\tmaxCharOrByte2 = %d\n",((xListFontsWithInfoReply *)mp)->maxCharOrByte2);
		BPRINTF2("\tdefaultChar = %d\n",((xListFontsWithInfoReply *)mp)->defaultChar);
		BPRINTF2("\tnFontProps = %ld\n",(long) ((xListFontsWithInfoReply *)mp)->nFontProps);
		BPRINTF2("\tdrawDirection = %ld\n",(long) ((xListFontsWithInfoReply *)mp)->drawDirection);
		BPRINTF2("\tminByte1 = %d\n",((xListFontsWithInfoReply *)mp)->minByte1);
		BPRINTF2("\tmaxByte1 = %d\n",((xListFontsWithInfoReply *)mp)->maxByte1);
		BPRINTF2("\tallCharsExist = %d\n",((xListFontsWithInfoReply *)mp)->allCharsExist);
		BPRINTF2("\tfontAscent = %d\n",((xListFontsWithInfoReply *)mp)->fontAscent);
		BPRINTF2("\tfontDescent = %d\n",((xListFontsWithInfoReply *)mp)->fontDescent);
		BPRINTF2("\tnReplies = %ld\n",((xListFontsWithInfoReply *)mp)->nReplies);
	        nfontprops = (long) ((xListFontsWithInfoReply *)mp)->nFontProps;
		Show_Value_List_nRep ( mp, nfontprops, sizeof(xListFontsWithInfoReply), FORMATfontprop);
		Show_String8 ( mp, sizeof(xListFontsWithInfoReply) + (nfontprops * 8), ((xListFontsWithInfoReply *)mp)->nameLength);
	}
		break;
	case X_GetFontPath:
		BPRINTF1("GetFontPath:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xGetFontPathReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetFontPathReply *)mp)->length);
		BPRINTF2("\tnPaths = %d\n",((xGetFontPathReply *)mp)->nPaths);
		Show_Strs(valuePtr,((xGetFontPathReply *)mp)->nPaths,
		    ((xGetFontPathReply *)mp)->length<<2,"path");
		break;
	case X_GetImage:
		BPRINTF1("GetImage:\n");
		BPRINTF2("\tdepth = %d\n",((xGetImageReply *)mp)->depth);
		BPRINTF2("\tsequenceNumber = %d\n",((xGetImageReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetImageReply *)mp)->length);
		BPRINTF2("\tvisual = %ld\n",((xGetImageReply *)mp)->visual);
		Show_Value_List_Rep(mp,sizeof(xGetImageReply),FORMAT8);
		break;
	case X_ListInstalledColormaps:
		BPRINTF1("ListInstalledColormaps:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xListInstalledColormapsReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xListInstalledColormapsReply *)mp)->length);
		BPRINTF2("\tnColormaps = %ld\n",(long) ((xListInstalledColormapsReply *)mp)->nColormaps);
		Show_Value_List_Rep(mp,sizeof(xListInstalledColormapsReply),FORMAT32);
		break;
	case X_AllocColor:
		BPRINTF1("AllocColor:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xAllocColorReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xAllocColorReply *)mp)->length);
		BPRINTF2("\tred = %d\n",((xAllocColorReply *)mp)->red);
		BPRINTF2("\tgreen = %d\n",((xAllocColorReply *)mp)->green);
		BPRINTF2("\tblue = %d\n",((xAllocColorReply *)mp)->blue);
		BPRINTF2("\tpixel = %ld\n",((xAllocColorReply *)mp)->pixel);
		break;
	case X_AllocNamedColor:
		BPRINTF1("AllocNamedColor:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xAllocNamedColorReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xAllocNamedColorReply *)mp)->length);
		BPRINTF2("\tpixel = %ld\n",((xAllocNamedColorReply *)mp)->pixel);
		BPRINTF2("\texactRed = %d\n",((xAllocNamedColorReply *)mp)->exactRed);
		BPRINTF2("\texactGreen = %d\n",((xAllocNamedColorReply *)mp)->exactGreen);
		BPRINTF2("\texactBlue = %d\n",((xAllocNamedColorReply *)mp)->exactBlue);
		BPRINTF2("\tscreenRed = %d\n",((xAllocNamedColorReply *)mp)->screenRed);
		BPRINTF2("\tscreenGreen = %d\n",((xAllocNamedColorReply *)mp)->screenGreen);
		BPRINTF2("\tscreenBlue = %d\n",((xAllocNamedColorReply *)mp)->screenBlue);
		break;
	case X_AllocColorCells:
		BPRINTF1("AllocColorCells:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xAllocColorCellsReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xAllocColorCellsReply *)mp)->length);
		BPRINTF2("\tnPixels = %ld\n",(long) ((xAllocColorCellsReply *)mp)->nPixels);
		BPRINTF2("\tnMasks = %ld\n",(long) ((xAllocColorCellsReply *)mp)->nMasks);
		BPRINTF1("\tPixels and Masks:\n");
		Show_Value_List_Rep( mp, sizeof(xAllocColorCellsReply), FORMAT32);
		break;
	case X_AllocColorPlanes:
		BPRINTF1("AllocColorPlanes:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xAllocColorPlanesReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xAllocColorPlanesReply *)mp)->length);
		BPRINTF2("\tnPixels = %ld\n",(long) ((xAllocColorPlanesReply *)mp)->nPixels);
		BPRINTF2("\tredMask = %ld\n",((xAllocColorPlanesReply *)mp)->redMask);
		BPRINTF2("\tgreenMask = %ld\n",((xAllocColorPlanesReply *)mp)->greenMask);
		BPRINTF2("\tblueMask = %ld\n",((xAllocColorPlanesReply *)mp)->blueMask);
		Show_Value_List_Rep(mp,sizeof(xAllocColorPlanesReply),FORMAT32);
		break;
	case X_QueryColors:
		BPRINTF1("QueryColors:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xQueryColorsReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryColorsReply *)mp)->length);
		BPRINTF2("\tnColors = %ld\n",(long) ((xQueryColorsReply *)mp)->nColors);
		Show_Value_List_Rep((xReq *)mp,sizeof(xQueryColorsReply),FORMATrgb);
		break;
	case X_LookupColor:
		BPRINTF1("LookupColor:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xLookupColorReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xLookupColorReply *)mp)->length);
		BPRINTF2("\texactRed = %d\n",((xLookupColorReply *)mp)->exactRed);
		BPRINTF2("\texactGreen = %d\n",((xLookupColorReply *)mp)->exactGreen);
		BPRINTF2("\texactBlue = %d\n",((xLookupColorReply *)mp)->exactBlue);
		BPRINTF2("\tscreenRed = %d\n",((xLookupColorReply *)mp)->screenRed);
		BPRINTF2("\tscreenGreen = %d\n",((xLookupColorReply *)mp)->screenGreen);
		BPRINTF2("\tscreenBlue = %d\n",((xLookupColorReply *)mp)->screenBlue);
		break;
	case X_QueryBestSize:
		BPRINTF1("QueryBestSize:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xQueryBestSizeReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryBestSizeReply *)mp)->length);
		BPRINTF2("\twidth = %d\n",((xQueryBestSizeReply *)mp)->width);
		BPRINTF2("\theight = %d\n",((xQueryBestSizeReply *)mp)->height);
		break;
	case X_QueryExtension:
		BPRINTF1("QueryExtension:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xQueryExtensionReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xQueryExtensionReply *)mp)->length);
		BPRINTF2("\tpresent = %d\n",((xQueryExtensionReply *)mp)->present);
		BPRINTF2("\tmajor_opcode = %d\n",((xQueryExtensionReply *)mp)->major_opcode);
		BPRINTF2("\tfirst_event = %d\n",((xQueryExtensionReply *)mp)->first_event);
		BPRINTF2("\tfirst_error = %d\n",((xQueryExtensionReply *)mp)->first_error);
		break;
	case X_ListExtensions:
		BPRINTF1("ListExtensions:\n");
		BPRINTF2("\tnExtensions = %d\n",((xListExtensionsReply *)mp)->nExtensions);
		BPRINTF2("\tsequenceNumber = %d\n",((xListExtensionsReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xListExtensionsReply *)mp)->length);
		Show_Strs(valuePtr,((xListExtensionsReply *)mp)->nExtensions,
		    ((xListExtensionsReply *)mp)->length<<2,"extension");
		break;
	case X_GetKeyboardMapping:
		BPRINTF1("GetKeyboardMapping:\n");
		BPRINTF2("\tkeySymsPerKeyCode = %ld\n",(long) ((xGetKeyboardMappingReply *)mp)->keySymsPerKeyCode);
		BPRINTF2("\tsequenceNumber = %d\n",((xGetKeyboardMappingReply *)mp)->sequenceNumber);
		BPRINTF2("\treply length = %ld\n",(long) ((xGetKeyboardMappingReply *)mp)->length);
		Show_Value_List_Rep(mp,sizeof(xGetKeyboardMappingReply),FORMAT32);
		break;
	case X_GetKeyboardControl:
		BPRINTF1("GetKeyboardControl:\n");
		BPRINTF2("\tglobalAutoRepeat = %ld\n",(long) ((xGetKeyboardControlReply *)mp)->globalAutoRepeat);
		BPRINTF2("\tsequenceNumber = %d\n",((xGetKeyboardControlReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetKeyboardControlReply *)mp)->length);
		BPRINTF2("\tledMask = %ld\n",((xGetKeyboardControlReply *)mp)->ledMask);
		BPRINTF2("\tkeyClickPercent = %d\n",((xGetKeyboardControlReply *)mp)->keyClickPercent);
		BPRINTF2("\tbellPercent = %d\n",((xGetKeyboardControlReply *)mp)->bellPercent);
		BPRINTF2("\tbellPitch = %d\n",((xGetKeyboardControlReply *)mp)->bellPitch);
		BPRINTF2("\tbellDuration = %d\n",((xGetKeyboardControlReply *)mp)->bellDuration);
		BPRINTF1("\tauto-repeats:\n");
		Show_Value_List_Rep( mp, sizeof(xGetKeyboardControlReply)-sizeof(((xGetKeyboardControlReply *)mp)->map), FORMAT8);
		break;
	case X_GetPointerControl:
		BPRINTF1("GetPointerControl:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xGetPointerControlReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetPointerControlReply *)mp)->length);
		BPRINTF2("\taccelNumerator = %d\n",((xGetPointerControlReply *)mp)->accelNumerator);
		BPRINTF2("\taccelDenominator = %d\n",((xGetPointerControlReply *)mp)->accelDenominator);
		BPRINTF2("\tthreshold = %d\n",((xGetPointerControlReply *)mp)->threshold);
		break;
	case X_GetScreenSaver:
		BPRINTF1("GetScreenSaver:\n");
		BPRINTF2("\tsequenceNumber = %d\n",((xGetScreenSaverReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetScreenSaverReply *)mp)->length);
		BPRINTF2("\ttimeout = %d\n",((xGetScreenSaverReply *)mp)->timeout);
		BPRINTF2("\tinterval = %d\n",((xGetScreenSaverReply *)mp)->interval);
		BPRINTF2("\tpreferBlanking = %ld\n",(long) ((xGetScreenSaverReply *)mp)->preferBlanking);
		BPRINTF2("\tallowExposures = %ld\n",(long) ((xGetScreenSaverReply *)mp)->allowExposures);
		break;
	case X_ListHosts:
		{
		    unsigned short nhosts;
		    xHostEntry *hp;
		    char scratch[132];
		    unsigned long hostname;
		    int i;

		    BPRINTF1("ListHosts:\n");
		    BPRINTF2("\tenabled = %ld\n",(long) ((xListHostsReply *)mp)->enabled);
		    BPRINTF2("\tsequenceNumber = %d\n",((xListHostsReply *)mp)->sequenceNumber);
		    BPRINTF2("\tlength = %ld\n",(long) ((xListHostsReply *)mp)->length);
		    BPRINTF2("\tnHosts = %d\n",((xListHostsReply *)mp)->nHosts);
		    nhosts = ((xListHostsReply *)mp)->nHosts;
		    for(i=0;i<(int)nhosts;i++) {
			if (valuePtr + sizeof(xHostEntry) > endPtr)
				break;
			hp = (xHostEntry *) valuePtr;
			if (valuePtr + hp->length > endPtr)
				break;
			BPRINTF2("\thost[%d]:\t",i);
			BPRINTF2("family = %d\t",hp->family);
			BPRINTF2("length = %d\t",hp->length);
			valuePtr += sizeof(xHostEntry);
			hostname = 0x0;
			bcopy(valuePtr, (char *)&hostname,
				min(sizeof(unsigned long),hp->length));
			BPRINTF2("\tFirst 4 bytes: 0x%08x\n",hostname);
			valuePtr += padup((int)hp->length);
		    }
		    if (i < (int)nhosts)
			BPRINTF2("\t.... INCOMPLETE, another %d entries expected\n", nhosts - i);
		}
		break;
	case X_SetPointerMapping:
		BPRINTF1("SetPointerMapping:\n");
		BPRINTF2("\tsuccess = %ld\n",(long) ((xSetPointerMappingReply *)mp)->success);
		BPRINTF2("\tsequenceNumber = %d\n",((xSetPointerMappingReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetPointerMappingReply *)mp)->length);
		break;
	case X_GetPointerMapping:
		BPRINTF1("GetPointerMapping:\n");
		BPRINTF2("\tnElts = %ld\n",(long) ((xGetPointerMappingReply *)mp)->nElts);
		BPRINTF2("\tsequenceNumber = %d\n",((xGetPointerMappingReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetPointerMappingReply *)mp)->length);
		Show_Value_List_Rep(mp,sizeof(xGetPointerMappingReply),FORMAT8);
		break;
	case X_SetModifierMapping:
		BPRINTF1("SetModifierMapping:\n");
		BPRINTF2("\tsuccess = %ld\n",(long) ((xSetModifierMappingReply *)mp)->success);
		BPRINTF2("\tsequenceNumber = %d\n",((xSetModifierMappingReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xSetModifierMappingReply *)mp)->length);
		break;
	case X_GetModifierMapping:
		BPRINTF1("GetModifierMapping:\n");
		BPRINTF2("\tnumKeyPerModifier = %ld\n",(long) ((xGetModifierMappingReply *)mp)->numKeyPerModifier);
		BPRINTF2("\tsequenceNumber = %d\n",((xGetModifierMappingReply *)mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n",(long) ((xGetModifierMappingReply *)mp)->length);
		Show_Value_List_Rep(mp,sizeof(xGetModifierMappingReply),FORMAT32);
		break;
	default:
		BPRINTF1("UNKNOWN REPLY TYPE:\n");
		BPRINTF2("\tstated type = %d\n",type);
		BPRINTF2("\tdata1 = %ld\n", (long) ((xGenericReply *) mp)->data1);
		BPRINTF2("\tsequenceNumber = %ld\n", (long) ((xGenericReply *) mp)->sequenceNumber);
		BPRINTF2("\tlength = %ld\n", (long) ((xGenericReply *) mp)->length);
		BPRINTF2("\tdata00 = %ld\n", (long) ((xGenericReply *) mp)->data00);
		BPRINTF2("\tdata01 = %ld\n", (long) ((xGenericReply *) mp)->data01);
		BPRINTF2("\tdata02 = %ld\n", (long) ((xGenericReply *) mp)->data02);
		BPRINTF2("\tdata03 = %ld\n", (long) ((xGenericReply *) mp)->data03);
		BPRINTF2("\tdata04 = %ld\n", (long) ((xGenericReply *) mp)->data04);
		BPRINTF2("\tdata05 = %ld\n", (long) ((xGenericReply *) mp)->data05);
		break;
	}
	if (free_it)
		Free_Reply(mp);
}
