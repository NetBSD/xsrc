/* $XFree86: xc/programs/lbxproxy/di/gfx.h,v 1.1 2004/04/03 22:38:53 tsi Exp $ */

#ifndef _GFX_H_
#define _GFX_H_

extern int  ProcInitialConnection(ClientPtr client),
            ProcEstablishConnection(ClientPtr client);

extern int  ProcStandardRequest(ClientPtr client);

extern int  ProcLBXChangeWindowAttributes(ClientPtr client),
	    ProcLBXGetWindowAttributes(ClientPtr client),
	    ProcLBXGetGeometry(ClientPtr client),
	    ProcLBXInternAtom(ClientPtr client),
            ProcLBXGetAtomName(ClientPtr client),
            ProcLBXCreateColormap(ClientPtr client),
            ProcLBXFreeColormap(ClientPtr client),
            ProcLBXCopyColormapAndFree(ClientPtr client),
            ProcLBXFreeColors(ClientPtr client),
            ProcLBXLookupColor(ClientPtr client),
            ProcLBXAllocColor(ClientPtr client),
            ProcLBXAllocNamedColor(ClientPtr client),
            ProcLBXAllocColorCells(ClientPtr client),
            ProcLBXAllocColorPlanes(ClientPtr client),
            ProcLBXGetModifierMapping(ClientPtr client),
            ProcLBXGetKeyboardMapping(ClientPtr client),
            ProcLBXQueryFont(ClientPtr client),
            ProcLBXChangeProperty(ClientPtr client),
            ProcLBXGetProperty(ClientPtr client),
	    ProcLBXCopyArea(ClientPtr client),
	    ProcLBXCopyPlane(ClientPtr client),
            ProcLBXPolyPoint(ClientPtr client),
            ProcLBXPolyLine(ClientPtr client),
            ProcLBXPolySegment(ClientPtr client),
            ProcLBXPolyRectangle(ClientPtr client),
            ProcLBXPolyArc(ClientPtr client),
            ProcLBXFillPoly(ClientPtr client),
            ProcLBXPolyFillRectangle(ClientPtr client),
            ProcLBXPolyFillArc(ClientPtr client),
	    ProcLBXPolyText(ClientPtr client),
	    ProcLBXImageText(ClientPtr client),
            ProcLBXQueryExtension(ClientPtr client),
	    ProcLBXGetImage(ClientPtr client),
	    ProcLBXPutImage(ClientPtr client);

extern int  ProcBadRequest(ClientPtr client);

/* tables */
extern int (*InitialVector[3]) (ClientPtr client);

#endif /* _GFX_H_ */
