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
 * $XConsortium: gtwdwattrb.m,v 1.10 94/04/17 21:03:32 rws Exp $
 */
>>TITLE XGetWindowAttributes CH04
Status

Display *display = Dsp;
Window w;
XWindowAttributes *window_attributes_return = &w_a;
>>EXTERN
XWindowAttributes w_a;
>># There is only one assertion here. The rational for this is that
>># this is a fundemental function, and that many of our tests rely
>># on the correct working of this to succeed. If there is
>># a problem with the function it will manifest itself elsewhere.
>>#							-stuart.
>>ASSERTION Good A
A call to xname on an existing window
.A w
returns
.S True ,
and the current attributes for the specified window
.A w
to the
.A window_attributes_return
structure.
>>STRATEGY
Create a colormap.
Install a colormap.
Create a window.
Call xname to obtain the window attributes.
Verify the window attributes were returned as expected.
>>CODE
Window parent;
int x, y, depth;
unsigned int width, height, border_width, class;
XVisualInfo *vinf;
Visual *visual;
unsigned long valuemask;
XSetWindowAttributes attributes;

/* Create a window. */
	resetvinf(VI_WIN);
	if( !nextvinf(&vinf) ) {
		delete("nextvinf call returned false");
		return;
	} else
		CHECK;

	parent = DefaultRootWindow(Dsp);
	x = 5;
	y = 10;
	width = 15;
	height = 20;
	border_width = 3;
	depth = vinf->depth;
	class = InputOutput;
	visual= vinf->visual;
	valuemask = CWBitGravity | CWWinGravity | CWBackingStore |
		CWBackingPlanes | CWBackingPixel | CWSaveUnder | CWColormap |
		CWOverrideRedirect | CWBorderPixel;

	attributes.bit_gravity = NorthGravity;
	attributes.win_gravity = SouthGravity;
	attributes.backing_store = WhenMapped;
	attributes.backing_planes= 3;
	attributes.backing_pixel = 1;
	attributes.save_under = True;

        /*
         * Create a colormap of the correct visual type, since there are
         * no guarantees that the selected visual is the default visual.
	 * Install the colormap to ensure that we can test the
	 * map_installed value
         */
	attributes.colormap = makecolmap(display, vinf->visual, AllocNone);
	XInstallColormap( display, attributes.colormap );

        /*
         * Set the window border pixel also, since it is possible that the
         * window visual does not match that of it's parent (the root),
         * causing a BadMatch error since the default border pixmap is
         * CopyFromParent.
         */
        attributes.border_pixel = 0;

	attributes.override_redirect = True;

	w = XCreateWindow(display, parent, x, y, width, height, border_width,
			depth, class, visual, valuemask, &attributes);
	regid(display, (union regtypes *)&w, REG_WINDOW);

/* Call xname to obtain the window attributes. */
	XCALL;

/* Verify the window attributes were returned as expected. */
#ifdef TESTING
	x--; y--; width--; height--; class--; depth--;
	attributes.bit_gravity = EastGravity;
	attributes.win_gravity = WestGravity;
	attributes.backing_store = NotUseful;
	attributes.backing_planes= 5;
	attributes.backing_pixel = 0;
	attributes.save_under = False;
	attributes.colormap = CopyFromParent;
	attributes.all_event_masks = StructureNotifyMask;
	attributes.your_event_mask = StructureNotifyMask;
	attributes.do_not_propagate_mask = StructureNotifyMask;
	attributes.override_redirect = False;
#endif
	if (w_a.x != x || w_a.y != y) {
		FAIL;
		report("%s did not return expected x,y coordinates", TestName);
		trace("Expected x=%d, y=%d", x, y);
		trace("Returned x=%d, y=%d", w_a.x, w_a.y);
	} else
		CHECK;

	if (w_a.width != width) {
		FAIL;
		report("%s did not return expected width", TestName);
		trace("Expected width=%u", width);
		trace("Returned width=%u", w_a.width);
	} else
		CHECK;

	if (w_a.height != height) {
		FAIL;
		report("%s did not return expected height", TestName);
		trace("Expected height=%u", height);
		trace("Returned height=%u", w_a.height);
	} else
		CHECK;

	if (w_a.border_width != border_width) {
		FAIL;
		report("%s did not return expected border width", TestName);
		trace("Expected border_width=%u", border_width);
		trace("Returned border_width=%u", w_a.border_width);
	} else
		CHECK;

	if (w_a.depth != depth) {
		FAIL;
		report("%s did not return expected depth", TestName);
		trace("Expected depth=%d", depth);
		trace("Returned depth=%d", w_a.depth);
	} else
		CHECK;

	if (XVisualIDFromVisual(w_a.visual) != XVisualIDFromVisual(visual)) {
		FAIL;
		report("%s did not return expected visual", TestName);
		trace("Expected visual=%d", XVisualIDFromVisual(visual));
		trace("Returned visual=%d", XVisualIDFromVisual(w_a.visual));
	} else
		CHECK;

	if (w_a.root != parent) {
		FAIL;
		report("%s did not return expected root window", TestName);
		trace("Expected root window=%d", parent);
		trace("Returned root window=%d", w_a.root);
	} else
		CHECK;

	if (w_a.class != class) {
		FAIL;
		report("%s did not return expected class", TestName);
		trace("Expected class=%u", class);
		trace("Returned class=%u", w_a.class);
	} else
		CHECK;

	if  (w_a.bit_gravity != attributes.bit_gravity) {
		FAIL;
		report("%s did not return expected bit gravity", TestName);
		trace("Expected bit_gravity=%s",
			 bitgravityname(attributes.bit_gravity));
		trace("Returned bit_gravity=%s",
			 bitgravityname(w_a.bit_gravity));
	} else
		CHECK;

	if  (w_a.win_gravity != attributes.win_gravity) {
		FAIL;
		report("%s did not return expected win gravity", TestName);
		trace("Expected win_gravity=%s",
			 wingravityname(attributes.win_gravity));
		trace("Returned win_gravity=%s",
			 wingravityname(w_a.win_gravity));
	} else
		CHECK;

	if  (w_a.backing_store != attributes.backing_store) {
		FAIL;
		report("%s did not return expected backing store", TestName);
		trace("Expected backing_store=%s",
			 backingstorename(attributes.backing_store));
		trace("Returned backing_store=%s",
			 backingstorename(w_a.backing_store));
	} else
		CHECK;

	if (w_a.backing_planes != attributes.backing_planes) {
		FAIL;
		report("%s did not return expected backing planes", TestName);
		trace("Expected backing_planes=%u", attributes.backing_planes);
		trace("Returned backing_planes=%u", w_a.backing_planes);
	} else
		CHECK;

	if (w_a.backing_pixel != attributes.backing_pixel) {
		FAIL;
		report("%s did not return expected backing pixel", TestName);
		trace("Expected backing_pixel=%u", attributes.backing_pixel);
		trace("Returned backing_pixel=%u", w_a.backing_pixel);
	} else
		CHECK;

	if (w_a.save_under != attributes.save_under) {
		FAIL;
		report("%s did not return expected save under", TestName);
		trace("Expected save_under=%s",
			 boolname(attributes.save_under));
		trace("Returned save_under=%s",
			 boolname(w_a.save_under));
	} else
		CHECK;

	if (w_a.colormap != attributes.colormap) {
		FAIL;
		report("%s did not return expected colormap", TestName);
		trace("Expected colormap=%d", attributes.colormap);
		trace("Returned colormap=%d", w_a.colormap);
	} else
		CHECK;

	if (w_a.map_installed != True) {
		FAIL;
		report("%s did not return expected map_installed", TestName);
		trace("Expected map_installed=%s",
			 boolname(True));
		trace("Returned map_installed=%s",
			 boolname(w_a.map_installed));
	} else
		CHECK;

	if (w_a.map_state != IsUnmapped ) {
		FAIL;
		report("%s did not return expected map_state", TestName);
		trace("Expected map_state=%s",
			 mapstatename(IsUnmapped));
		trace("Returned map_state=%s",
			 mapstatename(w_a.map_state));
	} else
		CHECK;

	if (w_a.all_event_masks != NoEventMask ) {
		FAIL;
		report("%s did not return expected all_event_masks", TestName);
		trace("Expected all_event_masks=%s",
			 eventmaskname(NoEventMask));
		trace("Returned all_event_masks=%s",
			 eventmaskname((unsigned long)w_a.all_event_masks));
	} else
		CHECK;

	if (w_a.your_event_mask != NoEventMask ) {
		FAIL;
		report("%s did not return expected your_event_mask", TestName);
		trace("Expected your_event_mask=%s",
			 eventmaskname(NoEventMask));
		trace("Returned your_event_mask=%s",
			 eventmaskname((unsigned long)w_a.your_event_mask));
	} else
		CHECK;

	if (w_a.do_not_propagate_mask != NoEventMask ) {
		FAIL;
		report("%s did not return expected do_not_propagate_mask",
			TestName);
		trace("Expected do_not_propagate_mask=%s",
			 eventmaskname(NoEventMask));
		trace("Returned do_not_propagate_mask=%s",
			 eventmaskname((unsigned long)w_a.do_not_propagate_mask));
	} else
		CHECK;

	if (w_a.override_redirect != True) {
		FAIL;
		report("%s did not return expected override_redirect",
			TestName);
		trace("Expected override_redirect=%s",
			boolname(attributes.override_redirect));
		trace("Returned override_redirect=%s",
			boolname(w_a.override_redirect));
	} else
		CHECK;

	if (w_a.screen != DefaultScreenOfDisplay(display) ) {
		FAIL;
		report("%s did not return expected screen pointer",
			TestName);
	} else
		CHECK;
		
	CHECKPASS(23);

>>ASSERTION Bad B 1
>># The bad drawable could occur if the window is destroyed whilst the
>># request is being processed. The Prize for the most original, working 
>># implementation is two Milky Ways.....			-stuart.
When xname is called, then a 
.S BadDrawable
error can occur.
>>ASSERTION Bad A
.ER BadWindow
