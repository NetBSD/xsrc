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
 * $XConsortium: startup.c,v 1.29 94/04/17 21:01:06 rws Exp $
 */

#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "tet_api.h"
#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include "extensions/XInput.h"
#include "xtestlib.h"
#include "pixval.h"

/*
 * Generic startup routine for tests.  All tests in general call
 * this routine unless there is a good reason not to.
 * 
 */

#define LINELEN 1024
char    buf[LINELEN];

extern  struct valname S_XIerror[];
extern  struct valname XI_event[];
extern	int 	ntests;
extern	int 	NXI_event;
int XInputMajorOpcode;
int XInputFirstError;
int XInputFirstEvent;
XID baddevice;

Display	*Dsp;
Window	Win;

static	Window	ofocus;
static	int	revert_to;
static	char	**odir_array;
static	int	odirs;

Window	ErrdefWindow;
Drawable ErrdefDrawable;
GC		ErrdefGC;
Colormap ErrdefColormap;

/* NOT DONE YET */
Pixmap	ErrdefPixmap;
Atom	ErrdefAtom;
Cursor	ErrdefCursor;
Font	ErrdefFont;

void	aborttest();

void
startup()
{
int 	i;
char	*disp;
char	*cp;
extern	char	*TestName;
extern	struct	tet_testlist	tet_testlist[];
#if TEST_ANSI
extern	int 	unexp_err(Display *d, XErrorEvent *ev);
extern	int 	io_err(Display *d);
#else
extern	int 	unexp_err();
extern	int 	io_err();
#endif

	/*
	 * Set the debug level first (it is used in initconfig).
	 */
	if ((cp = tet_getvar("XT_DEBUG")) != NULL) {
	extern	int 	DebugLevel;

		setdblev(atov(cp));
	} else {
		setdblev(0);
	}

	/*
	 * Obtain and check all configuration parameters.
	 */
	initconfig();

	/*
	 * Put out the NAME info line for the report generator.
	 */
	(void) sprintf(buf, "TRACE:NAME: %s", TestName);
	tet_infoline(buf);

	/*
	 * Pause a while in case the X server is resetting.
	 */
	reset_delay();

	/*
	 * Get the display to use and open it.
	 */
	disp = tet_getvar("XT_DISPLAY");

	if (disp == (char*)0) {
		report("XT_DISPLAY was not set.  Can not continue.");
		for (i = 0; i < ntests; i++)
			tet_testlist[i].testfunc = aborttest;
		return;
	}

#ifdef XTEST_THREADS
	XInitThreads();
#endif
	Dsp = XOpenDisplay(disp);

	if (Dsp == (Display *)0) {
		report("Could not open display.  Can not continue.");
		for (i = 0; i < ntests; i++)
			tet_testlist[i].testfunc = aborttest;
		return;
	}

#ifdef notyet
	/* Get a window for general use */
	Win = makewin(Dsp);
#endif

	/* Make sure that screen saver hasn't cut in */
	XResetScreenSaver(Dsp);

	/*
	 * Set up the error handlers.
	 */
	(void) XSetErrorHandler(unexp_err);
	(void) XSetIOErrorHandler(io_err);

	/*
	 * Initialize the errors for the input device extension.
	 */

	 (void) init_xinput(Dsp);

	/*
	 * Set up the default resources for error tests. At present
	 * just use the root window (could create a window specially)
	 */
	ErrdefWindow = DefaultRootWindow(Dsp);
	ErrdefDrawable = DefaultRootWindow(Dsp);
	ErrdefGC = DefaultGC(Dsp, DefaultScreen(Dsp));
	ErrdefColormap = DefaultColormap(Dsp, DefaultScreen(Dsp));
	ErrdefPixmap = maketile(Dsp, DefaultRootWindow(Dsp));
	ErrdefAtom = XInternAtom(Dsp, "XT_ERRDEFATOM", False);
	/* ErrdefCursor; */
	/* ErrdefFont; */

	/*
	 * Sync and clear out queue.
	 */
	XSync(Dsp, True);

}

/*
 * Cleanup functions called at the end of the test purposes.
 */
void
cleanup()
{
	if (Dsp) {
#ifndef GENERATE_PIXMAPS
		/* At present this causes needless problems... */
		XCloseDisplay(Dsp);
#endif
	}
}

/*
 * Startup functions called at the end of the test purposes.
 * Version to set fontpath.
 */
void
fontstartup()
{
	startup();
	if (Dsp == (Display *)0) 
		return;

	setxtfontpath();
}

void
setxtfontpath()
{
char	*fpathlist;
char	*fpathtmp;
char	*ndir_array[MAX_DIRS];
int 	ndirs;
int 	i;

	/*
	 * Save font path. (Only the first time).
	 */
	if (!odir_array)
		odir_array = XGetFontPath(Dsp, &odirs);

	/*
	 * Set font path to that containing the test fonts.
	 */
	fpathlist = config.fontpath;
	if (fpathlist == NULL || *fpathlist == '\0') {
		for (i = 1; i <= ntests; i++)
			tet_delete(i, "XT_FONTPATH not set");
		return;
	}
	fpathtmp = (char *)calloc(strlen(fpathlist)+1, sizeof(char));
	strcpy(fpathtmp, fpathlist);

	for (i = 0; i < MAX_DIRS; i++) {
		ndir_array[i] = strtok((i==0)? fpathtmp: (char*)0, SEP);
		if (ndir_array[i] == NULL)
			break;
		debug(1, "ndir_array entry %d - '%s'", i, ndir_array[i]);
	}
	ndirs = i;

	if (ndirs <= 0) {
		for (i = 1; i <= ntests; i++)
			tet_delete(i, "XT_FONTPATH contains no components");
		return;
	}

	XSetFontPath(Dsp, ndir_array, ndirs);

	/*
	 * Sync and clear out queue.
	 */
	XSync(Dsp, True);
}

/*
 * Cleanup functions called at the end of the test purposes.
 * Version to reset fontpath.
 */
void
fontcleanup()
{
	if (Dsp) {
		XSetFontPath(Dsp, odir_array, odirs);
		XFreeFontPath(odir_array);
	}

	cleanup();
}

/*
 * Startup functions called at the end of the test purposes.
 * Version to set input focus.
 */
void
focusstartup()
{
	startup();
	if (Dsp == (Display *)0) 
		return;

	/*
	 * Set focus to known state.
	 */
	XGetInputFocus(Dsp, &ofocus, &revert_to);
	XSetInputFocus(Dsp, DefaultRootWindow(Dsp), RevertToPointerRoot, CurrentTime);

	/*
	 * Sync and clear out queue.
	 */
	XSync(Dsp, True);
}

/*
 * Cleanup functions called at the end of the test purposes.
 * Version to reset input focus.
 */
void
focuscleanup()
{
	if (Dsp)
		XSetInputFocus(Dsp, ofocus, revert_to, CurrentTime);

	cleanup();
}

/*
 * Startup functions called at the end of the test purposes.
 * Version to set input focus and fontpath.
 */
void
fontfocusstartup()
{
	fontstartup();
	if (Dsp == (Display *)0) 
		return;

	/*
	 * Set focus to known state.
	 */
	XGetInputFocus(Dsp, &ofocus, &revert_to);
	XSetInputFocus(Dsp, PointerRoot, RevertToPointerRoot, CurrentTime);

	/*
	 * Sync and clear out queue.
	 */
	XSync(Dsp, True);
}

/*
 * Cleanup functions called at the end of the test purposes.
 * Version to reset input focus and fontpath.
 */
void
fontfocuscleanup()
{
	if (Dsp)
		XSetInputFocus(Dsp, ofocus, revert_to, CurrentTime);

	fontcleanup();
}

/*
 * Startup functions called at the end of the test purposes.
 * Version to initialise the resource manager.
 */
void
rmstartup()
{
	startup();
	XrmInitialize();
}

/*
 * This is used to replace all test cases when the display can not be
 * opened.  It issues a result code that should abort the current test
 * case and the tcc.
 */
void
aborttest()
{
	tccabort("Could not open display %s",
		(config.display)? config.display: "<not set>");
}

/*
 * This is used to delay before opening the first connection to the server.
 * This is to allow the server to reset (the previous test case will most 
 * likely cause a reset, when its last connection is closed).
 */
void
reset_delay()
{
	sleep(config.reset_delay);
}

Bool
init_xinput(dpy)
Display *dpy;
{
struct valname *vp;

	if (!XQueryExtension (dpy, "XInputExtension", &XInputMajorOpcode, 
		&XInputFirstEvent, &XInputFirstError))
		return False;
	if (!XI_event[0].val)
	    for (vp = XI_event; vp < &XI_event[NXI_event]; vp++) {
		vp->val += XInputFirstEvent;
	}
	BadDevice(dpy,S_XIerror[0].val);
	BadDevice(dpy,baddevice);
	BadEvent(dpy,S_XIerror[1].val);
	BadMode(dpy,S_XIerror[2].val);
	DeviceBusy(dpy,S_XIerror[3].val);
	BadClass(dpy,S_XIerror[4].val);
	return True;
}
