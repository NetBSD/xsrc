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
 * $XConsortium: gtpntrmppn.m,v 1.8 94/04/17 21:06:35 rws Exp $
 */
>>TITLE XGetPointerMapping CH07
int
xname()
Display	*display = Dsp;
unsigned char	*map_return = Map;
int 	nmap = MAPSIZE;
>>EXTERN

/*
 * The number of buttons should be between 1 and 5 -- allow more space
 * in the array than this.
 */
#define	MAPSIZE	20
static	unsigned char	Map[MAPSIZE];

>>ASSERTION Good B 3
>>#A call to xname
>>#returns the logical button numbers corresponding to the
>>#physical button i+1, where i is an index into the returned array
>>#.A map_return .
A call to xname
returns in map_return[i] the logical button numbers of the
physical buttons i+1.
>>STRATEGY
Call xname to get pointer mapping.
If extension available:
  Create window and map it.
  Select for ButtonPress events.
  Warp into window.
  For i in 1..nphysbuttons
    Simulate pressing button i.
    Check for incoming ButtonPress event.
    Check that event.xbutton.button is map_return[i-1].
    Release all buttons.
else
  UNTESTED touch test only.
>>CODE
int 	nbuttons;
int	i;
Window	win;

	nbuttons = XCALL;
	trace("Number of buttons reported as %d", nbuttons);

	if (noext(nbuttons)) {
		report("There is no reliable test method, but a touch test was performed");
		UNTESTED;
		return;
	} else
		CHECK;
	win = defwin(display);
	if (isdeleted()) {
		delete("Failed to create ButtonPress window.");
		return;
	} else
		CHECK;
	XSelectInput(display, win, ButtonPressMask);
	XSync(display, True);
	if (isdeleted()) {
		delete("Failed to select for ButtonPress. May indicate competing clients (e.g. window managers).");
		return;
	} else
		CHECK;
	(void) warppointer(display, win, 2, 2);
	for (i=0; i<nbuttons; i++) {
		XEvent ev;

		XSync(display, True); /* clear out event Q */
		_startcall(display); /* install and reset error handlers */
		buttonpress(display, (unsigned int)i+1);
		if (!XCheckWindowEvent(display, win, ButtonPressMask, &ev)) {
			delete("No event received for button %d.", i+1);
			relalldev();
			return;
		} else
			CHECK;
		relalldev();
		_endcall(display); /* back to catching unexpected ones */
		if (ev.xbutton.button != map_return[i]) {
			report("Mapping for button %d is %d not %d.",
				i+1, (int)ev.xbutton.button, (int)map_return[i]);
			FAIL;
		} else
			CHECK;
		trace("Mapping for button %d is %d, expecting %d.",
				i+1, (int)ev.xbutton.button, (int)map_return[i]);
	}
	CHECKPASS(3+2*nbuttons);

>>ASSERTION Good B 3
A call to xname
returns the number of physical buttons actually on the pointer.
>>STRATEGY
Call xname to get number of buttons.
Check this lies within the protocol limit of 1..5.
If extension available:
  Simulate the pressing of buttons 1..5 and check that
    we got Success for buttons in the range returned by xname, and
    we got BadValue for the rest.
  Release all buttons.
>>CODE
int	nbuttons;
int	i;

	nbuttons = XCALL;
	if (nbuttons < 1 || nbuttons > 5) {
		report("Protocol limit of 1..5 buttons exceeded (%d).", nbuttons);
		FAIL;
	} else
		CHECK;

	if (noext(nbuttons)) {
		report("There is no reliable test method, but a touch test was performed");
		UNTESTED;
		return;
	} else
		CHECK;

	for(i=0; i<5; i++) {
		_startcall(display); /* install and reset error handlers */
		buttonpress(display, (unsigned int)i+1);
		if (geterr() != ((i<nbuttons)?Success:BadValue)) {
			report("Apparently button %d could%s be pressed.",
				i+1, (i<nbuttons)?"n't":"");
			FAIL;
		} else
			CHECK;
		relalldev();
		_endcall(display); /* back to catching unexpected ones */
	}
	CHECKPASS(2+5);

>>ASSERTION Good A
When the
.A nmap
argument is less than the number of elements in the pointer mapping,
then only the first
.A nmap
elements are returned in
.A map_return .
>>STRATEGY
Set all elements of map_return to 255.
Set nmap to a value less than number of elements in the pointer mapping.
Call xname.
Verify that elements of map_return beyond nmap-1 are still 255.
>>CODE
int 	nbuttons;
int 	i;
#define	TEST_VAL	((unsigned char)255)

	nbuttons = XCALL;

	for (i = 0; i < MAPSIZE; i++)
		Map[i] = TEST_VAL;

	/*
	 * Its not clear that asking for 0 elements is really sensible so we
	 * only do it when there is only one button.
	 */
	if (nbuttons == 1)
		nmap = 0;
	else
		nmap = 1;

	XCALL;

	for (i = nmap; i < MAPSIZE; i++) {
		if (Map[i] != TEST_VAL) {
			report("An element beyond the first nmap was returned");
			report(" element %d was %u, expecting %u", i, (unsigned)Map[i],
				(unsigned)TEST_VAL);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(MAPSIZE-nmap);
