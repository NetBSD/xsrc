/*
 
Copyright (c) 1990, 1991, 1992  X Consortium

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
 * Copyright 1990, 1991, 1992 by UniSoft Group Limited.
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
 * $XConsortium: pixval.h,v 1.16 94/04/17 21:00:04 rws Exp $
 */
#ifndef _PIX_VAL_
#define _PIX_VAL_
/*************
*  Functions to mask during pixval generation runs
**************/

#ifdef GENERATE_PIXMAPS
#define XSync(a,b) 
#define	XPending(d)	(0)
#define XMapWindow(a,b) VMapWindow(a,b)
#define XMapRaised(a,b) do {VMapWindow(a,b);VRaiseWindow(a,b);} while(0)
#define XUnmapWindow(a,b) VUnmapWindow(a,b)
#define XClearWindow(a,b)  VClearWindow(a,b)
#define XClearArea(a,b,x,y,w,h,e) VClearArea(a,b,x,y,w,h,e) 
#define save_stat(a,b,c,d) REGR_NORMAL
#define chek_stat(a,b,c,d,e) REGR_NORMAL
#define badstat(a,b,c) REGR_NORMAL
#define r_init() vr_init()
#define r_close() vr_close()
#define step(str) vstep(str)
#define r_wait(a,b,c,d)
#define bufrdisp(a)
#define bufrgc(a,b)
#define signal_status arbitrary
#define XDrawPoint(a,b,c,d,e) VDrawPoint(a,b,c,d,e)
#define XDrawPoints(a,b,c,d,e,f) VDrawPoints(a,b,c,d,e,f)
#define XDrawLine(a,b,c,d,e,f,g) VDrawLine(a,b,c,d,e,f,g)
#define XDrawLines(a,b,c,d,e,f) VDrawLines(a,b,c,d,e,f)
#define XDrawSegments(a,b,c,d,e) VDrawSegments(a,b,c,d,e)
#define XDrawRectangle(a,b,c,d,e,f,g) VDrawRectangle(a,b,c,d,e,f,g)
#define XDrawRectangles(a,b,c,d,e) VDrawRectangles(a,b,c,d,e)
#define XFillRectangle(a,b,c,d,e,f,g) VFillRectangle(a,b,c,d,e,f,g)
#define XFillRectangles(a,b,c,d,e) VFillRectangles(a,b,c,d,e)
#define XFillPolygon(a,b,c,d,e,f,g) VFillPolygon(a,b,c,d,e,f,g)
#define XDrawArc(a,b,c,d,e,f,g,h,i) VDrawArc(a,b,c,d,e,f,g,h,i)
#define XDrawArcs(a,b,c,d,e) VDrawArcs(a,b,c,d,e)
#define XFillArc(a,b,c,d,e,f,g,h,i) VFillArc(a,b,c,d,e,f,g,h,i)
#define XFillArcs(a,b,c,d,e) VFillArcs(a,b,c,d,e)
#define XCloseDisplay(a) VCloseDisplay(a)
#define XCopyGC(d,gc,vm,xgcv) VCopyGC(d,gc,vm,xgcv)
#define XCreateGC(d,w,vm,xgcv) VCreateGC(d,w,vm,xgcv)
#define XCreatePixmap(a,b,c,d,e) VCreatePixmap(a,b,c,d,e)
#define XCreateSimpleWindow(a,b,c,d,e,f,g,h,i) VCreateSimpleWindow(a,b,c,d,e,f,g,h,i) 
#define XCreateWindow(a,b,c,d,e,f,g,h,i,j,k,l) VCreateWindow(a,b,c,d,e,f,g,h,i,j,k,l)
#define XChangeWindowAttributes(a,b,c,d)  VChangeWindowAttributes(a,b,c,d) 
#define XGetGeometry(display, drawable, root, x, y, width, height, b_w, depth) \
		VGetGeometry(display, drawable, root, x, y, width, height, b_w, depth)
#define XGetWindowAttributes(a,b,c)  VGetWindowAttributes(a,b,c) 
#define XQueryTree(di,w,rr,pr,cr,n) VQueryTree(di,w,rr,pr,cr,n)
#define XDestroyWindow(a,b) VDestroyWindow(a,b) 
#define XFreeGC(a,b) VFreeGC(a,b)
#define XFreePixmap(a,b) VFreePixmap(a,b)
#define XOpenDisplay(a) VOpenDisplay(a)
#define XSetBackground(a,b,c) VSetBackground(a,b,c)
#define XSetForeground(a,b,c) VSetForeground(a,b,c)
#define XSetFunction(a,b,c) VSetFunction(a,b,c)
#define XSetState(a,b,c,d,e,f) VSetState(a,b,c,d,e,f)
#define XSetLineAttributes(disp,gc,a,b,c,d) VSetLineAttributes(disp,gc,a,b,c,d)
#define XSetDashes(disp,gc,do,dl,n) VSetDashes(disp,gc,do,dl,n)
#define XSetFillStyle(disp,gc,fs) VSetFillStyle(disp,gc,fs)
#define XSetFillRule(disp,gc,fr) VSetFillRule(disp,gc,fr)
#define XSetArcMode(disp,gc,am) VSetArcMode(disp,gc,am)
#define XSetStipple(disp,gc,s) VSetStipple(disp,gc,s)
#define XSetTile(disp,gc,t) VSetTile(disp,gc,t)
#define XSetClipMask(disp,gc,cm) VSetClipMask(disp,gc,cm)
#define XSetTSOrigin(disp,gc,tsx,tsy) VSetTSOrigin(disp,gc,tsx,tsy)
#define XSetClipOrigin(disp,gc,clx,cly) VSetClipOrigin(disp,gc,clx,cly)
#define XSetWindowBackground(a,b,c) VSetWindowBackground(a,b,c)
#define XSetWindowBackgroundPixmap(a,b,c) VSetWindowBackgroundPixmap(a,b,c)
#define	XConfigureWindow(d,w,a,b) VConfigureWindow(d,w,a,b)
#define XMoveWindow(d,w,x,y) VMoveWindow(d,w,x,y)
#define XResizeWindow(d,w,wd,h) VResizeWindow(d,w,wd,h)
#define XRaiseWindow(d,w) VRaiseWindow(d,w)
#define XLoadFont(d,fn) VLoadFont(d,fn)
#define XQueryFont(d,fid) VQueryFont(d,fid)
#define XLoadQueryFont(d,fn) VLoadQueryFont(d,fn)
#define XSetFont(d,gc,f) VSetFont(d,gc,f)
#define XTextExtents(fs,s,n,dr,ar,dtr,or) VTextExtents(fs,s,n,dr,ar,dtr,or)
#define XTextExtents16(fs,s,n,dr,ar,dtr,or) VTextExtents16(fs,s,n,dr,ar,dtr,or)
#define XTextWidth(fs,s,len) VTextWidth(fs,s,len)
#define XTextWidth16(fs,s,len) VTextWidth16(fs,s,len)
#define XUnloadFont(d,f) VUnloadFont(d,f)
#define XFreeFont(d,fs) VFreeFont(d,fs)
#define XDrawImageString(a,b,c,d,e,f,g) VDrawImageString(a,b,c,d,e,f,g)
#define XDrawString(a,b,c,d,e,f,g) VDrawString(a,b,c,d,e,f,g)
#define XDrawImageString16(a,b,c,d,e,f,g) VDrawImageString16(a,b,c,d,e,f,g)
#define XDrawString16(a,b,c,d,e,f,g) VDrawString16(a,b,c,d,e,f,g)
#define XDrawText(di,dr,gc,x,y,items,nitems) VDrawText(di,dr,gc,x,y,items,nitems)
#define XDrawText16(di,dr,gc,x,y,items,nitems) VDrawText16(di,dr,gc,x,y,items,nitems)
#define XDestroySubwindows(di,wi) VDestroySubwindows(di,wi) 
#define XMapSubwindows(di,wi) VMapSubwindows(di,wi) 
#define XMoveResizeWindow(di,wi,x,y,w,h) VMoveResizeWindow(di,wi,x,y,w,h)
#define XSetWindowBorder(di,wi,b) VSetWindowBorder(di,wi,b)
#define XSetWindowBorderPixmap(di,wi,p) VSetWindowBorderPixmap(di,wi,p)
#define XSetWindowBorderWidth(di,wi,w) VSetWindowBorderWidth(di,wi,w)
#define XCreateColormap(disp,w,vis,alloc) NULL
#define XFreeColormap(disp,colormap)
#define XInstallColormap(disp,colormap)
#define XSetWindowColormap(disp,w,colormap)
#define XStoreColor(disp,colormap,color)
#define XCreateRegion() NULL
#define XEmptyRegion(anything) True
#define XEqualRegion(aand, b) True
#define XIntersectRegion(three, para, meters)
#define XNextEvent(has, twoparameters)
#define XSelectInput(hasone, two, threethreeparameters)
#define XSubtractRegion(one, two, three)
#define XUnionRectWithRegion(onetwo, three, also)
#define XUnionRegion(has, threeparameters, howboutthat)
#define XResetScreenSaver(dsp)

/* Section 3.8 functions */
#define	XCirculateSubwindows(di,wi,dir)	VCirculateSubwindows(di,wi,dir)
#define	XCirculateSubwindowsUp(di,wi)	VCirculateSubwindowsUp(di,wi)
#define	XCirculateSubwindowsDown(di,wi)	VCirculateSubwindowsDown(di,wi)
#define	XLowerWindow(di,wi)	VLowerWindow(di,wi)
#define XRestackWindows(di,wi,nwi)	VRestackWindows(di,wi,nwi)

/* kieron new stuff */
#define XT_FONTDIR	"XT_FONTDIR"	/* used in tet_getvar() call in vfonts */
#define XCopyArea(display, source, dest, gc, src_x, src_y, width, height, dest_x, dest_y) \
	VCopyArea(display, source, dest, gc, src_x, src_y, width, height, dest_x, dest_y)
#define XCopyPlane(display, source, dest, gc, src_x, src_y, width, height, dest_x, dest_y,plane) \
	VCopyPlane(display, source, dest, gc, src_x, src_y, width, height, dest_x, dest_y,plane)

#define XSetPlaneMask(a,b,c) VSetPlaneMask(a,b,c)
#define XSetSubwindowMode(a,b,c) VSetSubwindowMode(a,b,c)
#define XSetGraphicsExposures(a,b,c) VSetGraphicsExposures(a,b,c)
#define XGetAtomName(dpy, atom) atomname(atom)
#define XSetErrorHandler(a) VSetErrorHandler(a)
#define XSetIOErrorHandler(a) VSetIOErrorHandler(a)
#define XSetFontPath(dpy, dirs, ndirs) VSetFontPath(dpy, dirs, ndirs)
#define XGetFontPath(dpy, ndirs_return) VGetFontPath(dpy, ndirs_return)
#define XFreeFontPath(dirs) VFreeFontPath(dirs)
#define	XChangeGC(a,b,c,d) VChangeGC(a,b,c,d)
#define	XGetGCValues(dsp,gc,val,vp) VGetGCValues(dsp,gc,val,vp)
#define _XFlushGCCache(d,g)
#define XWindowEvent(a,b,c,d)
#define XCreateBitmapFromData(display, d, data, width, height) \
		VCreateBitmapFromData(display, d, data, width, height)
#define XCreatePixmapFromBitmapData(display,d,data,width,height,fg,bg,depth) \
		VCreatePixmapFromBitmapData(display,d,data,width,height,fg,bg,depth)
#define XPutImage(dpy, d, gc, image, req_xoffset, req_yoffset, x, y , req_width, req_height) \
		VPutImage (dpy, d, gc, image, req_xoffset, \
			req_yoffset, x, y , req_width, req_height)

#define XGetImage(dpy, d, x, y , width, height, planes, format) \
		VGetImage (dpy, d, x, y , width, height, planes, format)
#define XGetSubImage(dpy, d, x, y, width, height, plane_mask, format, dest_image, dest_x, dest_y) \
		VGetSubImage(dpy, d, x, y, width, height, plane_mask, \
			format, dest_image, dest_x, dest_y)

/****** ones to watch?
gettime.o:_XChangeProperty
gettime.o:_XCheckWindowEvent
gettime.o:_XInternAtom
issuppvis.o:_XFree
issuppvis.o:_XGetVisualInfo
nextvinf.o:_XGetVisualInfo
nextvinf.o:_XListDepths
*******/

#define	XInternAtom(a,b,c)	(1)

#define	XCreateFontCursor(dpy, shape)		((Cursor)-1)
#define	XFreeCursor(dpy, cursor)
#define	XQueryPointer(dpy,w,rootp,childp,rxp,ryp,xp,yp,maskp) False
#define	XWarpPointer(dpy, src_w,dst_w,x,y,w,h,dstx,dsty)
#define	XGetInputFocus(dpy, focus_ret, rev_to_ret)
#define	XSetInputFocus(dpy, focus, rev_to, time)

/**** versions of internal functions to interrogate display  - kieron ****/
/*	All to allow us to use Xlib image functions unaltered so we need
	to use our routines that go near displays rather than theirs, as
	their display layout may be different. That way we don't have to
	duplicate all of Xlib, just most of it!
*/

#define _XGetScanlinePad(dpy, depth) _VGetScanlinePad(dpy, depth)
#define _XGetBitsPerPixel(dpy, depth) _VGetBitsPerPixel(dpy, depth)


/* display macros */            

#define XConnectionNumber(dpy) 	((dpy)->fd)
#define XRootWindow(dpy, scr) 	(((dpy)->screens[(scr)]).root)
#define XDefaultScreen(dpy) 	((dpy)->default_screen)
#define XDefaultRootWindow(dpy) (((dpy)->screens[(dpy)->default_screen]).root)
#define XDefaultVisual(dpy, scr) (((dpy)->screens[(scr)]).root_visual)
#define XDefaultGC(dpy, scr) 	(((dpy)->screens[(scr)]).default_gc)
#define XBlackPixel(dpy, scr) 	(((dpy)->screens[(scr)]).black_pixel)
#define XWhitePixel(dpy, scr) 	(((dpy)->screens[(scr)]).white_pixel)
#define XAllPlanes() 		(~0)
#define XQLength(dpy) 		((dpy)->qlen)
#define XDisplayWidth(dpy, scr) (((dpy)->screens[(scr)]).width)
#define XDisplayHeight(dpy, scr) (((dpy)->screens[(scr)]).height)
#define XDisplayWidthMM(dpy, scr)(((dpy)->screens[(scr)]).mwidth)
#define XDisplayHeightMM(dpy, scr)(((dpy)->screens[(scr)]).mheight)
#define XDisplayPlanes(dpy, scr) (((dpy)->screens[(scr)]).root_depth)
#define XDisplayCells(dpy, scr) (DefaultVisual((dpy), (scr))->map_entries)
#define XScreenCount(dpy) 	((dpy)->nscreens)
#define XServerVendor(dpy) 	((dpy)->vendor)
#define XProtocolVersion(dpy) 	((dpy)->proto_major_version)
#define XProtocolRevision(dpy) 	((dpy)->proto_minor_version)
#define XVendorRelease(dpy) 	((dpy)->release)
#define XDisplayString(dpy) 	((dpy)->display_name)
#define XDefaultDepth(dpy, scr) (((dpy)->screens[(scr)]).root_depth)
#define XDefaultColormap(dpy, scr)(((dpy)->screens[(scr)]).cmap)
#define XBitmapUnit(dpy) 	((dpy)->bitmap_unit)
#define XBitmapBitOrder(dpy) 	((dpy)->bitmap_bit_order)
#define XBitmapPad(dpy) 	((dpy)->bitmap_pad)
#define XImageByteOrder(dpy) 	((dpy)->byte_order)
#define XNextRequest(dpy)	((dpy)->request + 1)
#define XLastKnownRequestProcessed(dpy)	((dpy)->last_request_read)

/* macros for screen oriented applications (toolkit) */

#define XScreenOfDisplay(dpy, scr)(&((dpy)->screens[(scr)]))
#define XDefaultScreenOfDisplay(dpy) (&((dpy)->screens[(dpy)->default_screen]))
#define XDisplayOfScreen(s)	((s)->display)
#define XRootWindowOfScreen(s)	((s)->root)
#define XBlackPixelOfScreen(s)	((s)->black_pixel)
#define XWhitePixelOfScreen(s)	((s)->white_pixel)
#define XDefaultColormapOfScreen(s)((s)->cmap)
#define XDefaultDepthOfScreen(s)((s)->root_depth)
#define XDefaultGCOfScreen(s)	((s)->default_gc)
#define XDefaultVisualOfScreen(s)((s)->root_visual)
#define XWidthOfScreen(s)	((s)->width)
#define XHeightOfScreen(s)	((s)->height)
#define XWidthMMOfScreen(s)	((s)->mwidth)
#define XHeightMMOfScreen(s)	((s)->mheight)
#define XPlanesOfScreen(s)	((s)->root_depth)
#define XCellsOfScreen(s)	(DefaultVisualOfScreen((s))->map_entries)
#define XMinCmapsOfScreen(s)	((s)->min_maps)
#define XMaxCmapsOfScreen(s)	((s)->max_maps)
#define XDoesSaveUnders(s)	((s)->save_unders)
#define XDoesBackingStore(s)	((s)->backing_store)
#define XEventMaskOfScreen(s)	((s)->root_input_mask)

#define VCompareCompImage() VGenerateCompImage()

#ifdef DUMP_KNOWN_GOOD_IMAGES /* for debug purposes - dumps known good image at generate time to given server for viewing */
#define VCompareCompImage() VGenerateCompImage2()
#endif


#ifdef DUMP_PIXMAPS /* for debug purposes - dumps pixmap record to given server for viewing 1 at a time */
#define VCompareCompImage() VDumpCompImage()
#endif

extern int PVT_debug;

#endif /* ifdef GENERATE_PIXMAPS */

/* pick up function protos for Vxxxxxx routines - kieron */
#include "Vlib.h"

#endif /* _PIX_VAL_ */
