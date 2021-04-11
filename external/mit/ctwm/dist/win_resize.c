/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * Copyright 1992 Claude Lecommandeur.
 */

/***********************************************************************
 *
 * $XConsortium: resize.c,v 1.80 91/05/11 17:35:42 dave Exp $
 *
 * window resizing borrowed from the "wm" window manager
 *
 * 11-Dec-87 Thomas E. LaStrange                File created
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 *
 ***********************************************************************/

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include "events.h"
#include "util.h"
#include "otp.h"
#include "functions_defs.h"
#include "add_window.h"
#include "colormaps.h"
#include "screen.h"
#include "drawing.h"
#include "win_decorations.h"
#include "win_ops.h"
#include "win_resize.h"
#include "win_utils.h"
#include "workspace_manager.h"
#include "iconmgr.h"

#define MINHEIGHT 0     /* had been 32 */
#define MINWIDTH 0      /* had been 60 */

static int dragx;       /* all these variables are used */
static int dragy;       /* in resize operations */
static unsigned int dragWidth;
static unsigned int dragHeight;

static int origx;
static int origy;
static int origWidth;
static int origHeight;

static int clampTop;
static int clampBottom;
static int clampLeft;
static int clampRight;
static int clampDX;
static int clampDY;

static int last_width;
static int last_height;

static unsigned int resizeGrabMask;

static void DisplaySize(TwmWindow *tmp_win, int width, int height);

static void do_auto_clamp(TwmWindow *tmp_win, XEvent *evp)
{
	Window junkRoot;
	int x, y, h, v, junkbw;
	unsigned int junkMask;

	switch(evp->type) {
		case ButtonPress:
			x = evp->xbutton.x_root;
			y = evp->xbutton.y_root;
			break;
		case KeyPress:
			x = evp->xkey.x_root;
			y = evp->xkey.y_root;
			break;
		default:
			if(!XQueryPointer(dpy, Scr->Root, &junkRoot, &junkRoot,
			                  &x, &y, &junkbw, &junkbw, &junkMask)) {
				return;
			}
	}

	/*
	 * Determine in which of the 9 "quadrants" of the window we are.
	 * Cast the values to signed int: if the numerator is negative
	 * we don't want them converted to unsigned due to the default
	 * promotion rules: that would produce a very large quotient.
	 */
	h = (int)(x - dragx) / (int)(dragWidth < 3 ? 1 : (dragWidth / 3));
	v = (int)(y - dragy - tmp_win->title_height) /
	    (int)(dragHeight < 3 ? 1 : (dragHeight / 3));

	if(h <= 0) {
		clampLeft = 1;
		clampDX = (x - dragx);
	}
	else if(h >= 2) {
		clampRight = 1;
		clampDX = (x - dragx - dragWidth);
	}

	if(v <= 0) {
		clampTop = 1;
		clampDY = (y - dragy);
	}
	else if(v >= 2) {
		clampBottom = 1;
		clampDY = (y - dragy - dragHeight);
	}
}

/***********************************************************************
 *
 *  Procedure:
 *      OpaqueResizeSize - determine if window should be resized opaquely.
 *
 *  Inputs:
 *      tmp_win - the TwmWindow pointer
 *
 ***********************************************************************
 */

void OpaqueResizeSize(TwmWindow *tmp_win)
{
	if(tmp_win->OpaqueResize) {
		/*
		 * OpaqueResize defaults to a thousand.  Assume that any number
		 * >= 1000 is "infinity" and don't bother calculating.
		 */
		if(Scr->OpaqueResizeThreshold >= 1000) {
			Scr->OpaqueResize = true;
		}
		else {
			/*
			 * scrsz will hold the number of pixels in your resolution,
			 * which can get big.  [signed] int may not cut it.
			 */
			const unsigned long winsz = tmp_win->frame_width
			                            * tmp_win->frame_height;
			const unsigned long scrsz = Scr->rootw  * Scr->rooth;
			if(winsz > (scrsz * (Scr->OpaqueResizeThreshold / 100.0))) {
				Scr->OpaqueResize = false;
			}
			else {
				Scr->OpaqueResize = true;
			}
		}
	}
	else {
		Scr->OpaqueResize = false;
	}
}


/***********************************************************************
 *
 *  Procedure:
 *      StartResize - begin a window resize operation
 *
 *  Inputs:
 *      ev      - the event structure (button press)
 *      tmp_win - the TwmWindow pointer
 *      fromtitlebar - action invoked from titlebar button
 *
 ***********************************************************************
 */

void StartResize(XEvent *evp, TwmWindow *tmp_win,
                 bool fromtitlebar, bool from3dborder)
{
	Window      junkRoot, grabwin;
	unsigned int junkbw, junkDepth;
	Cursor      cursor;

	cursor = (Scr->BorderCursors
	          && tmp_win->curcurs) ? tmp_win->curcurs : Scr->ResizeCursor;
	ResizeWindow = tmp_win->frame;
	if(! Scr->OpaqueResize || resizeWhenAdd) {
		XGrabServer(dpy);
	}
	resizeGrabMask = ButtonPressMask | ButtonReleaseMask |
	                 ButtonMotionMask | PointerMotionHintMask;

	grabwin = Scr->Root;
	if(tmp_win->winbox) {
		grabwin = tmp_win->winbox->window;
	}
	XGrabPointer(dpy, grabwin, True, resizeGrabMask,
	             GrabModeAsync, GrabModeAsync, grabwin, cursor, CurrentTime);

	XGetGeometry(dpy, (Drawable) tmp_win->frame, &junkRoot,
	             &dragx, &dragy, &dragWidth, &dragHeight, &junkbw,
	             &junkDepth);
	dragx += tmp_win->frame_bw;
	dragy += tmp_win->frame_bw;
	origx = dragx;
	origy = dragy;
	origWidth = dragWidth;
	origHeight = dragHeight;
	clampTop = clampBottom = clampLeft = clampRight = clampDX = clampDY = 0;

	if(Scr->AutoRelativeResize && (from3dborder || !fromtitlebar)) {
		do_auto_clamp(tmp_win, evp);
	}

	Scr->SizeStringOffset = SIZE_HINDENT;
	XResizeWindow(dpy, Scr->SizeWindow,
	              Scr->SizeStringWidth + SIZE_HINDENT * 2,
	              Scr->SizeFont.height + SIZE_VINDENT * 2);
	XMapRaised(dpy, Scr->SizeWindow);
	InstallRootColormap();
	last_width = 0;
	last_height = 0;
	DisplaySize(tmp_win, origWidth, origHeight);

	if(! Scr->OpaqueResize || resizeWhenAdd)
		MoveOutline(Scr->Root, dragx - tmp_win->frame_bw,
		            dragy - tmp_win->frame_bw, dragWidth + 2 * tmp_win->frame_bw,
		            dragHeight + 2 * tmp_win->frame_bw,
		            tmp_win->frame_bw, tmp_win->title_height + tmp_win->frame_bw3D);
}


void MenuStartResize(TwmWindow *tmp_win, int x, int y, int w, int h)
{
	if(! Scr->OpaqueResize) {
		XGrabServer(dpy);
	}
	resizeGrabMask = ButtonPressMask | ButtonMotionMask | PointerMotionMask;
	XGrabPointer(dpy, Scr->Root, True, resizeGrabMask,
	             GrabModeAsync, GrabModeAsync,
	             Scr->Root, Scr->ResizeCursor, CurrentTime);
	dragx = x + tmp_win->frame_bw;
	dragy = y + tmp_win->frame_bw;
	origx = dragx;
	origy = dragy;
	dragWidth = origWidth = w;
	dragHeight = origHeight = h;
	clampTop = clampBottom = clampLeft = clampRight = clampDX = clampDY = 0;
	last_width = 0;
	last_height = 0;
	Scr->SizeStringOffset = SIZE_HINDENT;
	XResizeWindow(dpy, Scr->SizeWindow,
	              Scr->SizeStringWidth + SIZE_HINDENT * 2,
	              Scr->SizeFont.height + SIZE_VINDENT * 2);
	XMapRaised(dpy, Scr->SizeWindow);
	DisplaySize(tmp_win, origWidth, origHeight);
	if(! Scr->OpaqueResize)
		MoveOutline(Scr->Root, dragx - tmp_win->frame_bw,
		            dragy - tmp_win->frame_bw,
		            dragWidth + 2 * tmp_win->frame_bw,
		            dragHeight + 2 * tmp_win->frame_bw,
		            tmp_win->frame_bw, tmp_win->title_height + tmp_win->frame_bw3D);
}

/***********************************************************************
 *
 *  Procedure:
 *      AddStartResize - begin a windorew resize operation from AddWindow
 *
 *  Inputs:
 *      tmp_win - the TwmWindow pointer
 *
 ***********************************************************************
 */

void AddStartResize(TwmWindow *tmp_win, int x, int y, int w, int h)
{
	XGrabServer(dpy);
	resizeGrabMask = ButtonReleaseMask | ButtonMotionMask | PointerMotionHintMask;
	XGrabPointer(dpy, Scr->Root, True, resizeGrabMask,
	             GrabModeAsync, GrabModeAsync,
	             Scr->Root, Scr->ResizeCursor, CurrentTime);

	dragx = x + tmp_win->frame_bw;
	dragy = y + tmp_win->frame_bw;
	origx = dragx;
	origy = dragy;
	dragWidth = origWidth = w - 2 * tmp_win->frame_bw;
	dragHeight = origHeight = h - 2 * tmp_win->frame_bw;
	clampTop = clampBottom = clampLeft = clampRight = clampDX = clampDY = 0;
	last_width = 0;
	last_height = 0;
	DisplaySize(tmp_win, origWidth, origHeight);
}


void MenuDoResize(int x_root, int y_root, TwmWindow *tmp_win)
{
	int action;
	Cursor cursor = 0;

	action = 0;

	x_root -= clampDX;
	y_root -= clampDY;

	if(clampTop) {
		int         delta = y_root - dragy;
		if((int)(dragHeight - delta) < MINHEIGHT) {
			delta = dragHeight - MINHEIGHT;
			clampTop = 0;
		}
		dragy += delta;
		dragHeight -= delta;
		action = 1;
		cursor = TopCursor;
	}
	else if(y_root <= dragy) {
		dragy = y_root;
		dragHeight = origy + origHeight -
		             y_root;
		clampBottom = 0;
		clampTop = 1;
		clampDY = 0;
		action = 1;
		cursor = TopCursor;
	}
	if(clampLeft) {
		int         delta = x_root - dragx;
		if((int)(dragWidth - delta) < MINWIDTH) {
			delta = dragWidth - MINWIDTH;
			clampLeft = 0;
		}
		dragx += delta;
		dragWidth -= delta;
		action = 1;
		cursor = clampTop ? TopLeftCursor : LeftCursor;
	}
	else if(x_root <= dragx) {
		dragx = x_root;
		dragWidth = origx + origWidth -
		            x_root;
		clampRight = 0;
		clampLeft = 1;
		clampDX = 0;
		action = 1;
		cursor = clampTop ? TopLeftCursor : LeftCursor;
	}
	if(clampBottom) {
		int         delta = y_root - dragy - dragHeight;
		if((int)(dragHeight + delta) < MINHEIGHT) {
			delta = MINHEIGHT - dragHeight;
			clampBottom = 0;
		}
		dragHeight += delta;
		action = 1;
		cursor = clampLeft ? BottomLeftCursor : BottomCursor;
	}
	else if(y_root >= dragy + dragHeight) {
		dragy = origy;
		dragHeight = 1 + y_root - dragy;
		clampTop = 0;
		clampBottom = 1;
		clampDY = 0;
		action = 1;
		cursor = clampLeft ? BottomLeftCursor : BottomCursor;
	}
	if(clampRight) {
		int         delta = x_root - dragx - dragWidth;
		if((int)(dragWidth + delta) < MINWIDTH) {
			delta = MINWIDTH - dragWidth;
			clampRight = 0;
		}
		dragWidth += delta;
		action = 1;
		cursor = clampBottom ? BottomRightCursor : RightCursor;
		cursor = clampTop ? TopRightCursor : cursor;
	}
	else if(x_root >= dragx + dragWidth) {
		dragx = origx;
		dragWidth = 1 + x_root - origx;
		clampLeft = 0;
		clampRight = 1;
		clampDX = 0;
		action = 1;
		cursor = clampBottom ? BottomRightCursor : RightCursor;
		cursor = clampTop ? TopRightCursor : cursor;
	}

	if(action) {
		ConstrainSize(tmp_win, &dragWidth, &dragHeight);
		if(clampLeft) {
			dragx = origx + origWidth - dragWidth;
		}
		if(clampTop) {
			dragy = origy + origHeight - dragHeight;
		}
		if(Scr->OpaqueResize)
			SetupWindow(tmp_win, dragx - tmp_win->frame_bw, dragy - tmp_win->frame_bw,
			            dragWidth, dragHeight, -1);
		else
			MoveOutline(Scr->Root,
			            dragx - tmp_win->frame_bw,
			            dragy - tmp_win->frame_bw,
			            dragWidth + 2 * tmp_win->frame_bw,
			            dragHeight + 2 * tmp_win->frame_bw,
			            tmp_win->frame_bw, tmp_win->title_height + tmp_win->frame_bw3D);
		if(Scr->BorderCursors && (cursor != tmp_win->curcurs)) {
			tmp_win->curcurs = cursor;
			XChangeActivePointerGrab(dpy, resizeGrabMask, cursor, CurrentTime);
		}
	}

	DisplaySize(tmp_win, dragWidth, dragHeight);
}

/***********************************************************************
 *
 *  Procedure:
 *      DoResize - move the rubberband around.  This is called for
 *                 each motion event when we are resizing
 *
 *  Inputs:
 *      x_root  - the X corrdinate in the root window
 *      y_root  - the Y corrdinate in the root window
 *      tmp_win - the current twm window
 *
 ***********************************************************************
 */

void DoResize(int x_root, int y_root, TwmWindow *tmp_win)
{
	int action;
	Cursor cursor = 0;

	action = 0;

	x_root -= clampDX;
	y_root -= clampDY;

	if(clampTop) {
		int         delta = y_root - dragy;
		if((int)(dragHeight - delta) < MINHEIGHT) {
			delta = dragHeight - MINHEIGHT;
			clampTop = 0;
		}
		dragy += delta;
		dragHeight -= delta;
		action = 1;
		cursor = TopCursor;
	}
	else if(y_root <= dragy) {
		dragy = y_root;
		dragHeight = origy + origHeight -
		             y_root;
		clampBottom = 0;
		clampTop = 1;
		clampDY = 0;
		action = 1;
		cursor = TopCursor;
	}
	if(clampLeft) {
		int         delta = x_root - dragx;
		if((int)(dragWidth - delta) < MINWIDTH) {
			delta = dragWidth - MINWIDTH;
			clampLeft = 0;
		}
		dragx += delta;
		dragWidth -= delta;
		action = 1;
		cursor = clampTop ? TopLeftCursor : LeftCursor;
	}
	else if(x_root <= dragx) {
		dragx = x_root;
		dragWidth = origx + origWidth -
		            x_root;
		clampRight = 0;
		clampLeft = 1;
		clampDX = 0;
		action = 1;
		cursor = clampTop ? TopLeftCursor : LeftCursor;
	}
	if(clampBottom) {
		int         delta = y_root - dragy - dragHeight;
		if((int)(dragHeight + delta) < MINHEIGHT) {
			delta = MINHEIGHT - dragHeight;
			clampBottom = 0;
		}
		dragHeight += delta;
		action = 1;
		cursor = clampLeft ? BottomLeftCursor : BottomCursor;
	}
	else if(y_root >= dragy + dragHeight - 1) {
		dragy = origy;
		dragHeight = 1 + y_root - dragy;
		clampTop = 0;
		clampBottom = 1;
		clampDY = 0;
		action = 1;
		cursor = clampLeft ? BottomLeftCursor : BottomCursor;
	}
	if(clampRight) {
		int         delta = x_root - dragx - dragWidth;
		if((int)(dragWidth + delta) < MINWIDTH) {
			delta = MINWIDTH - dragWidth;
			clampRight = 0;
		}
		dragWidth += delta;
		action = 1;
		cursor = clampBottom ? BottomRightCursor : RightCursor;
		cursor = clampTop ? TopRightCursor : cursor;
	}
	else if(x_root >= dragx + dragWidth - 1) {
		dragx = origx;
		dragWidth = 1 + x_root - origx;
		clampLeft = 0;
		clampRight = 1;
		clampDX = 0;
		action = 1;
		cursor = clampBottom ? BottomRightCursor : RightCursor;
		cursor = clampTop ? TopRightCursor : cursor;
	}

	if(action) {
		ConstrainSize(tmp_win, &dragWidth, &dragHeight);
		if(clampLeft) {
			dragx = origx + origWidth - dragWidth;
		}
		if(clampTop) {
			dragy = origy + origHeight - dragHeight;
		}
		if(Scr->OpaqueResize && ! resizeWhenAdd) {
			SetupWindow(tmp_win, dragx - tmp_win->frame_bw, dragy - tmp_win->frame_bw,
			            dragWidth, dragHeight, -1);
		}
		else {
			MoveOutline(Scr->Root,
			            dragx - tmp_win->frame_bw,
			            dragy - tmp_win->frame_bw,
			            dragWidth + 2 * tmp_win->frame_bw,
			            dragHeight + 2 * tmp_win->frame_bw,
			            tmp_win->frame_bw, tmp_win->title_height + tmp_win->frame_bw3D);
		}
		if(Scr->BorderCursors && (cursor != tmp_win->curcurs)) {
			tmp_win->curcurs = cursor;
			XChangeActivePointerGrab(dpy, resizeGrabMask, cursor, CurrentTime);
		}
	}

	DisplaySize(tmp_win, dragWidth, dragHeight);
}

/***********************************************************************
 *
 *  Procedure:
 *      DisplaySize - display the size in the dimensions window
 *
 *  Inputs:
 *      tmp_win - the current twm window
 *      width   - the width of the rubber band
 *      height  - the height of the rubber band
 *
 ***********************************************************************
 */

static void DisplaySize(TwmWindow *tmp_win, int width, int height)
{
	char str[100];
	int dwidth;
	int dheight;

	if(last_width == width && last_height == height) {
		return;
	}

	last_width = width;
	last_height = height;

	dheight = height - tmp_win->title_height - 2 * tmp_win->frame_bw3D;
	dwidth = width - 2 * tmp_win->frame_bw3D;

	/*
	 * ICCCM says that PMinSize is the default is no PBaseSize is given,
	 * and vice-versa.
	 */
	if(tmp_win->hints.flags & (PMinSize | PBaseSize)
	                && tmp_win->hints.flags & PResizeInc) {
		if(tmp_win->hints.flags & PBaseSize) {
			dwidth -= tmp_win->hints.base_width;
			dheight -= tmp_win->hints.base_height;
		}
		else {
			dwidth -= tmp_win->hints.min_width;
			dheight -= tmp_win->hints.min_height;
		}
	}

	if(tmp_win->hints.flags & PResizeInc) {
		dwidth /= tmp_win->hints.width_inc;
		dheight /= tmp_win->hints.height_inc;
	}

	sprintf(str, " %4d x %-4d ", dwidth, dheight);
	XRaiseWindow(dpy, Scr->SizeWindow);

	Draw3DBorder(Scr->SizeWindow, 0, 0,
	             Scr->SizeStringOffset + Scr->SizeStringWidth + SIZE_HINDENT,
	             Scr->SizeFont.height + SIZE_VINDENT * 2,
	             2, Scr->DefaultC, off, false, false);

	FB(Scr->DefaultC.fore, Scr->DefaultC.back);
	XmbDrawImageString(dpy, Scr->SizeWindow, Scr->SizeFont.font_set,
	                   Scr->NormalGC, Scr->SizeStringOffset,
	                   Scr->SizeFont.ascent + SIZE_VINDENT, str, 13);
}

/***********************************************************************
 *
 *  Procedure:
 *      EndResize - finish the resize operation
 *
 ***********************************************************************
 */

void EndResize(void)
{
	TwmWindow *tmp_win;

#ifdef DEBUG
	fprintf(stderr, "EndResize\n");
#endif

	MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
	XUnmapWindow(dpy, Scr->SizeWindow);

	tmp_win = GetTwmWindow(ResizeWindow);
	if(!tmp_win) {
		return;
	}

	ConstrainSize(tmp_win, &dragWidth, &dragHeight);

	if(dragWidth != tmp_win->frame_width ||
	                dragHeight != tmp_win->frame_height) {
		unzoom(tmp_win);
	}

	SetupWindow(tmp_win, dragx - tmp_win->frame_bw, dragy - tmp_win->frame_bw,
	            dragWidth, dragHeight, -1);

	if(tmp_win->isiconmgr) {
		int ncols = tmp_win->iconmgrp->cur_columns;
		if(ncols == 0) {
			ncols = 1;
		}

		tmp_win->iconmgrp->width = (int)(((dragWidth - 2 * tmp_win->frame_bw3D) *
		                                  (long) tmp_win->iconmgrp->columns)
		                                 / ncols);
		PackIconManager(tmp_win->iconmgrp);
	}

	if(!Scr->NoRaiseResize) {
		OtpRaise(tmp_win, WinWin);
		WMapRaise(tmp_win);
	}

	UninstallRootColormap();

	ResizeWindow = None;
}

void MenuEndResize(TwmWindow *tmp_win)
{
	MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
	XUnmapWindow(dpy, Scr->SizeWindow);
	ConstrainSize(tmp_win, &dragWidth, &dragHeight);
	AddingX = dragx - tmp_win->frame_bw;
	AddingY = dragy - tmp_win->frame_bw;
	AddingW = dragWidth;
	AddingH = dragHeight;
	SetupWindow(tmp_win, AddingX, AddingY, AddingW, AddingH, -1);
}


/***********************************************************************
 *
 *  Procedure:
 *      AddEndResize - finish the resize operation for AddWindo<w
 *
 ***********************************************************************
 */

void AddEndResize(TwmWindow *tmp_win)
{

#ifdef DEBUG
	fprintf(stderr, "AddEndResize\n");
#endif

	ConstrainSize(tmp_win, &dragWidth, &dragHeight);
	AddingX = dragx;
	AddingY = dragy;
	AddingW = dragWidth + (2 * tmp_win->frame_bw);
	AddingH = dragHeight + (2 * tmp_win->frame_bw);
}

/***********************************************************************
 *
 *  Procedure:
 *      ConstrainSize - adjust the given width and height to account for the
 *              constraints imposed by size hints
 *
 *      The general algorithm, especially the aspect ratio stuff, is
 *      borrowed from uwm's CheckConsistency routine.
 *
 ***********************************************************************/

void ConstrainSize(TwmWindow *tmp_win,
                   unsigned int *widthp, unsigned int *heightp)
{
#define makemult(a,b) ((b==1) ? (a) : (((int)((a)/(b))) * (b)) )
#define _min(a,b) (((a) < (b)) ? (a) : (b))

	int minWidth, minHeight, maxWidth, maxHeight, xinc, yinc, delta;
	int baseWidth, baseHeight;
	int dwidth = *widthp, dheight = *heightp;


	dwidth  -= 2 * tmp_win->frame_bw3D;
	dheight -= (tmp_win->title_height + 2 * tmp_win->frame_bw3D);

	if(tmp_win->hints.flags & PMinSize) {
		minWidth = tmp_win->hints.min_width;
		minHeight = tmp_win->hints.min_height;
	}
	else if(tmp_win->hints.flags & PBaseSize) {
		minWidth = tmp_win->hints.base_width;
		minHeight = tmp_win->hints.base_height;
	}
	else {
		minWidth = minHeight = 1;
	}

	if(tmp_win->hints.flags & PBaseSize) {
		baseWidth = tmp_win->hints.base_width;
		baseHeight = tmp_win->hints.base_height;
	}
	else if(tmp_win->hints.flags & PMinSize) {
		baseWidth = tmp_win->hints.min_width;
		baseHeight = tmp_win->hints.min_height;
	}
	else {
		baseWidth = baseHeight = 0;
	}


	if(tmp_win->hints.flags & PMaxSize) {
		maxWidth = _min(Scr->MaxWindowWidth, tmp_win->hints.max_width);
		maxHeight = _min(Scr->MaxWindowHeight, tmp_win->hints.max_height);
	}
	else {
		maxWidth = Scr->MaxWindowWidth;
		maxHeight = Scr->MaxWindowHeight;
	}

	if(tmp_win->hints.flags & PResizeInc) {
		xinc = tmp_win->hints.width_inc;
		yinc = tmp_win->hints.height_inc;
		if(xinc == 0) {
			xinc = 1;
		}
		if(yinc == 0) {
			yinc = 1;
		}
	}
	else {
		xinc = yinc = 1;
	}

	/*
	 * First, clamp to min and max values
	 */
	if(dwidth < minWidth) {
		dwidth = minWidth;
	}
	if(dheight < minHeight) {
		dheight = minHeight;
	}

	if(dwidth > maxWidth) {
		dwidth = maxWidth;
	}
	if(dheight > maxHeight) {
		dheight = maxHeight;
	}


	/*
	 * Second, fit to base + N * inc
	 */
	dwidth = ((dwidth - baseWidth) / xinc * xinc) + baseWidth;
	dheight = ((dheight - baseHeight) / yinc * yinc) + baseHeight;


	/*
	 * Third, adjust for aspect ratio
	 */
	/*
	 * The math looks like this:
	 *
	 * minAspectX    dwidth     maxAspectX
	 * ---------- <= ------- <= ----------
	 * minAspectY    dheight    maxAspectY
	 *
	 * If that is multiplied out, then the width and height are
	 * invalid in the following situations:
	 *
	 * minAspectX * dheight > minAspectY * dwidth
	 * maxAspectX * dheight < maxAspectY * dwidth
	 *
	 */

	if(tmp_win->hints.flags & PAspect) {
		int minAspectX = tmp_win->hints.min_aspect.x;
		int minAspectY = tmp_win->hints.min_aspect.y;
		int maxAspectX = tmp_win->hints.max_aspect.x;
		int maxAspectY = tmp_win->hints.max_aspect.y;

		if(minAspectX && minAspectY && maxAspectX && maxAspectY) {
			if(minAspectX * dheight > minAspectY * dwidth) {
				delta = makemult(minAspectX * dheight / minAspectY - dwidth,
				                 xinc);
				if(dwidth + delta <= maxWidth) {
					dwidth += delta;
				}
				else {
					delta = makemult(dheight - dwidth * minAspectY / minAspectX,
					                 yinc);
					if(dheight - delta >= minHeight) {
						dheight -= delta;
					}
				}
			}

			if(maxAspectX * dheight < maxAspectY * dwidth) {
				delta = makemult(dwidth * maxAspectY / maxAspectX - dheight,
				                 yinc);
				if(dheight + delta <= maxHeight) {
					dheight += delta;
				}
				else {
					delta = makemult(dwidth - maxAspectX * dheight / maxAspectY,
					                 xinc);
					if(dwidth - delta >= minWidth) {
						dwidth -= delta;
					}
				}
			}
		}
	}


	/*
	 * Fourth, account for border width and title height
	 */
	*widthp = dwidth + 2 * tmp_win->frame_bw3D;
	*heightp = dheight + tmp_win->title_height + 2 * tmp_win->frame_bw3D;
}




/**********************************************************************
 *  Rutgers mod #1   - rocky.
 *  Procedure:
 *         fullzoom - zooms window to full height of screen or
 *                    to full height and width of screen. (Toggles
 *                    so that it can undo the zoom - even when switching
 *                    between fullzoom and vertical zoom.)
 *
 *  Inputs:
 *         tmp_win - the TwmWindow pointer
 *
 *
 **********************************************************************
 */

void fullzoom(TwmWindow *tmp_win, int func)
{
	Window      junkRoot;
	unsigned int junkbw, junkDepth;
	int basex, basey;
	int border_x, border_y;
	int frame_bw_times_2;
	int zwidth  = Scr->rootw;
	int zheight = Scr->rooth;
	int tmpX, tmpY, tmpW, tmpH;


	/*
	 * All our callers [need to] do this, so moving it here saves a few
	 * lines in some places around the calling, and when redundant it
	 * just wastes a comparison, so it's cheap.
	 */
	if(tmp_win->squeezed) {
		XBell(dpy, 0);
		return;
	}


	XGetGeometry(dpy, (Drawable) tmp_win->frame, &junkRoot,
	             &dragx, &dragy, (unsigned int *)&dragWidth, (unsigned int *)&dragHeight,
	             &junkbw,
	             &junkDepth);

	basex = Scr->BorderLeft;
	basey = Scr->BorderTop;

	border_x = Scr->BorderLeft + Scr->BorderRight;
	border_y = Scr->BorderTop + Scr->BorderBottom;

	/*
	 * Guard; if it was already not zoomed, and we're asking to unzoom
	 * it, just finish right away.  This saves us work, but also avoids
	 * really bad side effects in some cases.  e.g., if we try to
	 * ZOOM_NONE a window that's never been ZOOM'd, tmp_win->save_* will
	 * all be 0, so we'd wind up resizing it to a point.  It's possible
	 * for that to happen via e.g. an EWMH message removing a _FULLSCREEN
	 * or similar attribute; that can then call into us telling us not to
	 * zoom, on a window that's never been zoomed.
	 *
	 * This wouldn't protect us if somehow it was zoomed but hadn't set
	 * that data, but I don't see how that can happen.  Worry about that
	 * when it does.
	 */
	if(func == ZOOM_NONE && tmp_win->zoomed == ZOOM_NONE) {
		return;
	}

	if(tmp_win->winbox) {
		XWindowAttributes winattrs;
		if(XGetWindowAttributes(dpy, tmp_win->winbox->window, &winattrs)) {
			zwidth   = winattrs.width;
			zheight  = winattrs.height;
		}
		basex    = 0;
		basey    = 0;
		border_x = 0;
		border_y = 0;
	}
	if(tmp_win->zoomed == func) {
		/* It was already zoomed this way, unzoom it */
		dragHeight = tmp_win->save_frame_height;
		dragWidth = tmp_win->save_frame_width;
		dragx = tmp_win->save_frame_x;
		dragy = tmp_win->save_frame_y;

		unzoom(tmp_win);

		/* XXX _should_ it be falling through here? */
	}
	else {
		if(tmp_win->zoomed == ZOOM_NONE) {
			tmp_win->save_frame_x = dragx;
			tmp_win->save_frame_y = dragy;
			tmp_win->save_frame_width = dragWidth;
			tmp_win->save_frame_height = dragHeight;
		}
		tmp_win->zoomed = func;

		frame_bw_times_2 = 2 * tmp_win->frame_bw;

		switch(func) {
			case ZOOM_NONE:
				break;
			case F_ZOOM:
				dragHeight = zheight - border_y - frame_bw_times_2;
				dragy = basey;
				break;
			case F_HORIZOOM:
				dragx = basex;
				dragWidth = zwidth - border_x - frame_bw_times_2;
				break;
			case F_FULLZOOM:
				dragx = basex;
				dragy = basey;
				dragHeight = zheight - border_y - frame_bw_times_2;
				dragWidth = zwidth - border_x - frame_bw_times_2;
				break;
			case F_LEFTZOOM:
				dragx = basex;
				dragy = basey;
				dragHeight = zheight - border_y - frame_bw_times_2;
				dragWidth = (zwidth - border_x) / 2 - frame_bw_times_2;
				break;
			case F_RIGHTZOOM:
				dragx = basex + (zwidth - border_x) / 2;
				dragy = basey;
				dragHeight = zheight - border_y - frame_bw_times_2;
				dragWidth = (zwidth - border_x) / 2 - frame_bw_times_2;
				break;
			case F_TOPZOOM:
				dragx = basex;
				dragy = basey;
				dragHeight = (zheight - border_y) / 2 - frame_bw_times_2;
				dragWidth = zwidth - border_x - frame_bw_times_2;
				break;
			case F_BOTTOMZOOM:
				dragx = basex;
				dragy = basey + (zheight - border_y) / 2;
				dragHeight = (zheight - border_y) / 2 - frame_bw_times_2;
				dragWidth = zwidth - border_x - frame_bw_times_2;
				break;
			case F_FULLSCREENZOOM: {
				int bw3D = tmp_win->frame_bw3D;
				int bw3D_times_2 = 2 * bw3D;
				int bw = tmp_win->frame_bw + bw3D;

				dragx = -bw;
				dragy = -tmp_win->title_height - bw;
				dragHeight = zheight + tmp_win->title_height + bw3D_times_2;
				dragWidth = zwidth + bw3D_times_2;

				/* and should ignore aspect ratio and size increments... */
#ifdef EWMH
				/* x-ref HandleFocusIn() comments for why we need this */
				OtpSetAflag(tmp_win, OTP_AFLAG_FULLSCREEN);
				OtpRestackWindow(tmp_win);
				/* the OtpRaise below is effectively already done here... */
#endif
			}
		}
	}

	if(!Scr->NoRaiseResize && func != F_FULLSCREENZOOM) {
		OtpRaise(tmp_win, WinWin);
	}

	if(func != F_FULLSCREENZOOM) {
		ConstrainSize(tmp_win, &dragWidth, &dragHeight);
	}
#ifdef BETTERZOOM
	if(func == F_ZOOM) {
		if(dragy + dragHeight < tmp_win->save_frame_y + tmp_win->save_frame_height) {
			dragy = tmp_win->save_frame_y + tmp_win->save_frame_height - dragHeight;
		}
	}
#endif
	SetupWindow(tmp_win, dragx, dragy, dragWidth, dragHeight, -1);
	/* I don't understand the reason of this. Claude.
	    XUngrabPointer (dpy, CurrentTime);
	*/
	XUngrabServer(dpy);

	XQueryPointer(dpy,
	              tmp_win->w,
	              &junkRoot, &junkRoot,
	              &tmpX, &tmpY, &tmpW, &tmpH, &junkDepth);
	if(tmp_win->frame_x > tmpX ||
	                tmp_win->frame_x + tmp_win->frame_width < tmpX ||
	                tmp_win->frame_y > tmpY ||
	                tmp_win->frame_y + tmp_win->frame_height < tmpY) {
		XWarpPointer(dpy, Scr->Root, tmp_win->w, 0, 0, 0, 0, 0, 0);
	}

#ifdef EWMH
	/*
	 * Reset _NET_WM_STATE prop on the window.  It sets whichever state
	 * applies, not always the _MAXIMIZED_VERT we specify here.
	 */
	EwmhSet_NET_WM_STATE(tmp_win, EWMH_STATE_MAXIMIZED_VERT);
#endif
}

/*
 * Forget about a window being zoomed.
 * This also needs to undo the special effects of F_FULLSCREENZOOM.
 */
void unzoom(TwmWindow *tmp_win)
{
	if(tmp_win->zoomed != ZOOM_NONE) {
#ifdef EWMH
		if(tmp_win->zoomed == F_FULLSCREENZOOM) {
			OtpClearAflag(tmp_win, OTP_AFLAG_FULLSCREEN);
			OtpRestackWindow(tmp_win);
		}
#endif

		tmp_win->zoomed = ZOOM_NONE;
	}
}

void savegeometry(TwmWindow *tmp_win)
{
	if(!tmp_win) {
		return;
	}
	tmp_win->savegeometry.x      = tmp_win->frame_x;
	tmp_win->savegeometry.y      = tmp_win->frame_y;
	tmp_win->savegeometry.width  = tmp_win->frame_width;
	tmp_win->savegeometry.height = tmp_win->frame_height;
}

void restoregeometry(TwmWindow *tmp_win)
{
	int x, y;
	unsigned int w, h;

	if(!tmp_win) {
		return;
	}
	if(tmp_win->savegeometry.width == (unsigned int) - 1) {
		return;
	}
	x = tmp_win->savegeometry.x;
	y = tmp_win->savegeometry.y;
	w = tmp_win->savegeometry.width;
	h = tmp_win->savegeometry.height;
	SetupWindow(tmp_win, x, y, w, h, -1);
}


void ChangeSize(char *in_string, TwmWindow *tmp_win)
{
	int change = 0, size = 0;
	char *endptr;
	int rx, ry, wx, wy, mr;
	Window  rr, cr;

	if(Isdigit(in_string[0])) {
		/* Handle the case f.changesize "640x480" */
		wx = strtol(in_string, &endptr, 10);
		if(*endptr++ != 'x') {
			fprintf(stderr,
			        "%s: Bad argument to f.changesize: \"%s\" (pattern \"640x480\")\n",
			        ProgramName, in_string);
			return;
		}
		wy = strtol(endptr, &endptr, 10);

		if(wy < tmp_win->title_height + 1) {
			wy = tmp_win->title_height + 1;
		}

		SetupWindow(tmp_win, tmp_win->frame_x, tmp_win->frame_y,
		            wx, wy + tmp_win->title_height, -1);
	}
	else {
		/* Handle the cases like f.changesize "right +10" */
		int cmdlen = 0;

		while(in_string[cmdlen] != ' ' && in_string[cmdlen] != '\0') {
			cmdlen++;
		}

		if(in_string[cmdlen] != ' ') {
			fprintf(stderr,
			        "%s: Bad argument to f.changesize: \"%s\" (sizechange missing)\n",
			        ProgramName, in_string);
			return;
		}

		change = strtol(in_string + cmdlen + 1, &endptr, 10);
		if(*endptr != 0) {
			fprintf(stderr,
			        "%s: Bad argument to f.changesize: \"%s\" (sizechange not a number)\n",
			        ProgramName, in_string);
			return;
		}

		if(strncmp("bottom", in_string, cmdlen) == 0) {
			size = tmp_win->frame_height + change;

			if(size < (tmp_win->title_height + 1)) {
				size = tmp_win->title_height + 1;
			}

			SetupWindow(tmp_win, tmp_win->frame_x, tmp_win->frame_y,
			            tmp_win->frame_width, size,
			            -1);

			XQueryPointer(dpy, tmp_win->w, &rr, &cr, &rx, &ry, &wx, &wy,
			              (unsigned int *)&mr);

			if((wy + tmp_win->title_height) > size) {
				XWarpPointer(dpy, None, tmp_win->w, 0, 0, 0, 0, 0, 0);
			}
		}
		else if(strncmp("top", in_string, cmdlen) == 0) {
			size = tmp_win->frame_height + change;

			if(size < (tmp_win->title_height + 1)) {
				size = tmp_win->title_height + 1;
			}

			SetupWindow(tmp_win, tmp_win->frame_x, (tmp_win->frame_y - change),
			            tmp_win->frame_width, size,
			            -1);

			XQueryPointer(dpy, tmp_win->w, &rr, &cr, &rx, &ry, &wx, &wy,
			              (unsigned int *)&mr);

			if((wy + tmp_win->title_height) > size) {
				XWarpPointer(dpy, None, tmp_win->w, 0, 0, 0, 0, 0, 0);
			}


		}
		else if(strncmp("left", in_string, cmdlen) == 0) {
			size = tmp_win->frame_width + change;

			if(size < 1) {
				size = 1;
			}

			SetupWindow(tmp_win, (tmp_win->frame_x - change), tmp_win->frame_y,
			            size, tmp_win->frame_height,
			            -1);

			XQueryPointer(dpy, tmp_win->w, &rr, &cr, &rx, &ry, &wx, &wy,
			              (unsigned int *)&mr);

			if(wx > size) {
				XWarpPointer(dpy, None, tmp_win->w, 0, 0, 0, 0, 0, 0);
			}


		}
		else if(strncmp("right", in_string, cmdlen) == 0) {
			size = tmp_win->frame_width + change;

			if(size < 1) {
				size = 1;
			}

			SetupWindow(tmp_win, tmp_win->frame_x, tmp_win->frame_y,
			            size, tmp_win->frame_height,
			            -1);

			XQueryPointer(dpy, tmp_win->w, &rr, &cr, &rx, &ry, &wx, &wy,
			              (unsigned int *)&mr);

			if(wx > size) {
				XWarpPointer(dpy, None, tmp_win->w, 0, 0, 0, 0, 0, 0);
			}

		}
		else {
			/* error */
			fprintf(stderr, "%s: Bad argument to f.changesize: \"%s\"\n (unknown border)",
			        ProgramName, in_string);
			return;
		}
	}
}


/***********************************************************************
 *
 *  Procedure:
 *      resizeFromCenter -
 *
 ***********************************************************************
 */
void
resizeFromCenter(Window w, TwmWindow *tmp_win)
{
	int lastx, lasty, bw2;

	bw2 = tmp_win->frame_bw * 2;
	AddingW = tmp_win->attr.width + bw2 + 2 * tmp_win->frame_bw3D;
	AddingH = tmp_win->attr.height + tmp_win->title_height + bw2 + 2 *
	          tmp_win->frame_bw3D;

	XGetGeometry(dpy, w, &JunkRoot, &origDragX, &origDragY,
	             &DragWidth, &DragHeight,
	             &JunkBW, &JunkDepth);

	XWarpPointer(dpy, None, w,
	             0, 0, 0, 0, DragWidth / 2, DragHeight / 2);
	XQueryPointer(dpy, Scr->Root, &JunkRoot,
	              &JunkChild, &JunkX, &JunkY,
	              &AddingX, &AddingY, &JunkMask);

	lastx = -10000;
	lasty = -10000;

	MenuStartResize(tmp_win, origDragX, origDragY, DragWidth, DragHeight);
	while(1) {
		XMaskEvent(dpy,
		           ButtonPressMask | PointerMotionMask | ExposureMask, &Event);

		if(Event.type == MotionNotify) {
			/* discard any extra motion events before a release */
			while(XCheckMaskEvent(dpy,
			                      ButtonMotionMask | ButtonPressMask, &Event))
				if(Event.type == ButtonPress) {
					break;
				}
		}

		if(Event.type == ButtonPress) {
			MenuEndResize(tmp_win);
			// Next line should be unneeded, done by MenuEndResize() ?
			XMoveResizeWindow(dpy, w, AddingX, AddingY, AddingW, AddingH);
			break;
		}

		if(Event.type != MotionNotify) {
			DispatchEvent2();
			if(Cancel) {
				// ...
				MenuEndResize(tmp_win);
				return;
			}
			continue;
		}

		/*
		 * XXX - if we are going to do a loop, we ought to consider
		 * using multiple GXxor lines so that we don't need to
		 * grab the server.
		 */
		XQueryPointer(dpy, Scr->Root, &JunkRoot, &JunkChild,
		              &JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

		if(lastx != AddingX || lasty != AddingY) {
			MenuDoResize(AddingX, AddingY, tmp_win);

			lastx = AddingX;
			lasty = AddingY;
		}

	}
}
