/* $XFree86: xc/programs/Xserver/hw/xfree86/XF86Setup/tclvidmode.c,v 3.4 1996/08/24 12:51:02 dawes Exp $ */
/*
 * Copyright 1996 by Joseph V. Moss <joe@XFree86.Org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Joseph Moss not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Joseph Moss makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * JOSEPH MOSS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL JOSEPH MOSS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


/*

  This file contains Tcl bindings to the XFree86-VidModeExtension

 */

#define NOT_YET_IMPLEMENTED 0

#include <X11/Intrinsic.h>
#include <X11/Xmd.h>
#include <X11/extensions/xf86vmode.h>
#include <tcl.h>
#include <tk.h>
#include "tclvidmode.h"

/* Mode flags -- ignore flags not in V_FLAG_MASK */
#define V_FLAG_MASK	0x1FF;
#define V_PHSYNC	0x001 
#define V_NHSYNC	0x002
#define V_PVSYNC	0x004
#define V_NVSYNC	0x008
#define V_INTERLACE	0x010 
#define V_DBLSCAN	0x020
#define V_CSYNC		0x040
#define V_PCSYNC	0x080
#define V_NCSYNC	0x100

static int (*savErrorFunc)();
static int errorOccurred;
static char errMsgBuf[512];

/*
  Simple error handler
*/
static int vidError(dis, err)
Display *dis;
XErrorEvent *err;
{
	XGetErrorText(dis, err->error_code, errMsgBuf, 512);
	errorOccurred = TRUE;
	return 0;
}

/*
   Adds all the vidmode specific commands to the Tcl interpreter
*/

int
XF86vid_Init(interp)
    Tcl_Interp	*interp;
{
     Tcl_CreateCommand(interp, "xf86vid_getversion",
	     TCL_XF86VidModeQueryVersion, (ClientData) NULL,
	     (void (*)()) NULL);

     Tcl_CreateCommand(interp, "xf86vid_getbasevals",
	     TCL_XF86VidModeQueryExtension, (ClientData) NULL,
	     (void (*)()) NULL);

     Tcl_CreateCommand(interp, "xf86vid_getmodeline",
	     TCL_XF86VidModeGetModeLine, (ClientData) NULL,
	     (void (*)()) NULL);

     Tcl_CreateCommand(interp, "xf86vid_getallmodelines",
	     TCL_XF86VidModeGetAllModeLines, (ClientData) NULL,
	     (void (*)()) NULL);

     Tcl_CreateCommand(interp, "xf86vid_lockmodeswitch",
	     TCL_XF86VidModeLockModeSwitch, (ClientData) NULL,
	     (void (*)()) NULL);

     Tcl_CreateCommand(interp, "xf86vid_switchmode",
	     TCL_XF86VidModeSwitchMode, (ClientData) NULL,
	     (void (*)()) NULL);

     Tcl_CreateCommand(interp, "xf86vid_getmonitor",
	     TCL_XF86VidModeGetMonitor, (ClientData) NULL,
	     (void (*)()) NULL);

     return TCL_OK;
}

/*
   Implements the xf86vid_getversion command which
   returns (in interp->result) the version of the
   XFree86-VidModeExtension that is built into the X server
   The version is returned simple floating point number (e.g. 0.4)
*/

int
TCL_XF86VidModeQueryVersion(clientData, interp, argc, argv)
    ClientData	clientData;
    Tcl_Interp	*interp;
    int		argc;
    char	*argv[];
{
	int MajorVersion, MinorVersion;
	Tk_Window tkwin;
	char tmpbuf[16];

        if (argc != 1) {
                Tcl_SetResult(interp, "Usage: xf86vid_getversion", TCL_STATIC);
                return TCL_ERROR;
        }

	if ((tkwin = Tk_MainWindow(interp)) == NULL)
		return TCL_ERROR;
	if (!XF86VidModeQueryVersion(Tk_Display(tkwin), &MajorVersion, &MinorVersion))
	{
		Tcl_AppendResult(interp,
			"Could not query vidmode extension version",
			(char *) NULL);
		return TCL_ERROR;
	} else {
		sprintf(tmpbuf, "%d.%d", MajorVersion, MinorVersion);
		Tcl_AppendResult(interp, tmpbuf, (char *) NULL);
		return TCL_OK;
	}
}


/*
   Implements the xf86vid_getbasevals command which
   returns (in interp->result) a list containing two elements.
   The first element is the EventBase and the second is the ErrorBase
*/

int
TCL_XF86VidModeQueryExtension(clientData, interp, argc, argv)
    ClientData	clientData;
    Tcl_Interp	*interp;
    int		argc;
    char	*argv[];
{
	int EventBase, ErrorBase;
	Tk_Window tkwin;
	char tmpbuf[16];

        if (argc != 1) {
                Tcl_SetResult(interp, "Usage: xf86vid_getbasevals", TCL_STATIC);
                return TCL_ERROR;
        }

	if ((tkwin = Tk_MainWindow(interp)) == NULL)
		return TCL_ERROR;
	if (!XF86VidModeQueryExtension(Tk_Display(tkwin), &EventBase, &ErrorBase)) {
		Tcl_AppendResult(interp,
			"Unable to query video extension information",
			(char *) NULL);
		return TCL_ERROR;
	} else {
		sprintf(tmpbuf, "%d %d", EventBase, ErrorBase);
		Tcl_AppendResult(interp, tmpbuf, (char *) NULL);
		return TCL_OK;
	}
}


/*
   Implements the xf86vid_getmodeline command which
   returns (in interp->result) a list containing the
   various video mode parameters (including any flags)
*/

int
TCL_XF86VidModeGetModeLine(clientData, interp, argc, argv)
    ClientData	clientData;
    Tcl_Interp	*interp;
    int		argc;
    char	*argv[];
{
	int dot_clock, mode_flags;
	Tk_Window tkwin;
	XF86VidModeModeLine mode_line;
	char tmpbuf[200];

        if (argc != 1) {
                Tcl_SetResult(interp, "Usage: xf86vid_getmodeline", TCL_STATIC);
                return TCL_ERROR;
        }

	if ((tkwin = Tk_MainWindow(interp)) == NULL)
		return TCL_ERROR;
	if (!XF86VidModeGetModeLine(Tk_Display(tkwin),
			Tk_ScreenNumber(tkwin),
			&dot_clock, &mode_line)) {
		Tcl_AppendResult(interp,
			"Unable to get mode line information",
			(char *) NULL);
		return TCL_ERROR;
	} else {
		sprintf(tmpbuf, "%6.2f %d %d %d %d %d %d %d %d",
		    dot_clock/1000.0,
		    mode_line.hdisplay, mode_line.hsyncstart,
		    mode_line.hsyncend, mode_line.htotal,
		    mode_line.vdisplay, mode_line.vsyncstart,
		    mode_line.vsyncend, mode_line.vtotal);
		mode_flags = mode_line.flags;
		if (mode_flags & V_PHSYNC)    strcat(tmpbuf, " +hsync");
		if (mode_flags & V_NHSYNC)    strcat(tmpbuf, " -hsync");
		if (mode_flags & V_PVSYNC)    strcat(tmpbuf, " +vsync");
		if (mode_flags & V_NVSYNC)    strcat(tmpbuf, " -vsync");
		if (mode_flags & V_INTERLACE) strcat(tmpbuf, " interlace");
		if (mode_flags & V_CSYNC)     strcat(tmpbuf, " composite");
		if (mode_flags & V_PCSYNC)    strcat(tmpbuf, " +csync");
		if (mode_flags & V_PCSYNC)    strcat(tmpbuf, " -csync");
		if (mode_flags & V_DBLSCAN)   strcat(tmpbuf, " doublescan");
		Tcl_AppendResult(interp, tmpbuf, (char *) NULL);
		XtFree((char *) mode_line.private);
		return TCL_OK;
	}
}

/*
   Implements the xf86vid_getallmodelines command which
   returns (in interp->result) a list containing lists of the
   various video mode parameters (including any flags)
*/

int
TCL_XF86VidModeGetAllModeLines(clientData, interp, argc, argv)
    ClientData	clientData;
    Tcl_Interp	*interp;
    int		argc;
    char	*argv[];
{
	int i, modecount, mode_flags;
	Tk_Window topwin, tkwin;
	XF86VidModeModeInfo **modelines;
	char tmpbuf[200];

        if (argc != 1 && !(argc==3 && !strcmp(argv[1],"-displayof"))) {
                Tcl_SetResult(interp,
			"Usage: xf86vid_getallmodelines [-displayof <window>]",
			TCL_STATIC);
                return TCL_ERROR;
        }

	if ((topwin = Tk_MainWindow(interp)) == NULL)
		return TCL_ERROR;
	if (argc == 3) {
		tkwin = Tk_NameToWindow(interp, argv[2], topwin);
	} else
		tkwin = topwin;
	if (!XF86VidModeGetAllModeLines(Tk_Display(tkwin),
					Tk_ScreenNumber(tkwin),
					&modecount, &modelines)) {
		Tcl_AppendResult(interp,
			"Unable to get mode line information",
			(char *) NULL);
		return TCL_ERROR;
	} else {
	    for (i = 0; i < modecount; i++) {
		sprintf(tmpbuf, "%6.2f %d %d %d %d %d %d %d %d",
		    (float) modelines[i]->dotclock/1000.0,
		    modelines[i]->hdisplay, modelines[i]->hsyncstart,
		    modelines[i]->hsyncend, modelines[i]->htotal,
		    modelines[i]->vdisplay, modelines[i]->vsyncstart,
		    modelines[i]->vsyncend, modelines[i]->vtotal);
		mode_flags = modelines[i]->flags;
		if (mode_flags & V_PHSYNC)    strcat(tmpbuf, " +hsync");
		if (mode_flags & V_NHSYNC)    strcat(tmpbuf, " -hsync");
		if (mode_flags & V_PVSYNC)    strcat(tmpbuf, " +vsync");
		if (mode_flags & V_NVSYNC)    strcat(tmpbuf, " -vsync");
		if (mode_flags & V_INTERLACE) strcat(tmpbuf, " interlace");
		if (mode_flags & V_CSYNC)     strcat(tmpbuf, " composite");
		if (mode_flags & V_PCSYNC)    strcat(tmpbuf, " +csync");
		if (mode_flags & V_PCSYNC)    strcat(tmpbuf, " -csync");
		if (mode_flags & V_DBLSCAN)   strcat(tmpbuf, " doublescan");
		Tcl_AppendElement(interp, tmpbuf);
	    }
	    XtFree((char *) modelines);
	    return TCL_OK;
	}
}

/*
   Returns the monitor's manufacturer and model names and its
   horiz and vert sync rates,
*/

int
TCL_XF86VidModeGetMonitor(clientData, interp, argc, argv)
    ClientData	clientData;
    Tcl_Interp	*interp;
    int		argc;
    char	*argv[];
{
#define MNHSync ((int) monitor.nhsync)
#define MNVSync ((int) monitor.nvsync)

	XF86VidModeMonitor monitor;
	Tk_Window tkwin;
	char *Hsyncbuf, *Vsyncbuf, *tmpptr, *av[5];
	int i;

        if (argc != 1) {
                Tcl_SetResult(interp, "Usage: xf86vid_getmonitor", TCL_STATIC);
                return TCL_ERROR;
        }

	if ((tkwin = Tk_MainWindow(interp)) == NULL)
		return TCL_ERROR;
	if (!XF86VidModeGetMonitor(Tk_Display(tkwin),
			Tk_ScreenNumber(tkwin), &monitor))
	{
		Tcl_AppendResult(interp,
			"Could not get monitor information",
			(char *) NULL);
		return TCL_ERROR;
	} else {
		av[0] = monitor.vendor;
		av[1] = monitor.model;

		tmpptr = Hsyncbuf = XtMalloc(MNHSync*14);
		for (i = 0; i < MNHSync; i++) {
			sprintf(tmpptr, "%s%.5g-%.5g", (i? ",": ""),
				monitor.hsync[i].lo, monitor.hsync[i].hi);
			tmpptr += strlen(tmpptr);
		}
		av[2] = Hsyncbuf;

		tmpptr = Vsyncbuf = XtMalloc(MNVSync*14);
		for (i = 0; i < MNVSync; i++) {
			sprintf(tmpptr, "%s%.5g-%.5g", (i? ",": ""),
				monitor.vsync[i].lo, monitor.vsync[i].hi);
			tmpptr += strlen(tmpptr);
		}
		av[3] = Vsyncbuf;
		av[4] = NULL;

		Tcl_SetResult(interp, Tcl_Merge(4, av), TCL_DYNAMIC);
		XtFree(Hsyncbuf);
		XtFree(Vsyncbuf);
		XtFree(monitor.vendor);
		XtFree(monitor.model);
		XtFree((char *) monitor.hsync);
		XtFree((char *) monitor.vsync);
		return TCL_OK;
	}
#undef MNHSync
#undef MNVSync
}

/*
   Turn on/off video mode switching
*/

int
TCL_XF86VidModeLockModeSwitch(clientData, interp, argc, argv)
    ClientData	clientData;
    Tcl_Interp	*interp;
    int		argc;
    char	*argv[];
{
	int lock;
	Tk_Window tkwin;
	static char usagemsg[] = "Usage: xf86vid_lockmodeswitch lock|unlock";

        if (argc != 2) {
                Tcl_SetResult(interp, usagemsg, TCL_STATIC);
                return TCL_ERROR;
        }

	if (!strcmp(argv[1], "lock")) {
		lock = TRUE;
	} else if (!strcmp(argv[1], "unlock")) {
		lock = FALSE;
	} else {
                Tcl_SetResult(interp, usagemsg, TCL_STATIC);
                return TCL_ERROR;
        }

        if ((tkwin = Tk_MainWindow(interp)) == NULL)
                return TCL_ERROR;
 
	XSync(Tk_Display(tkwin), False);
	savErrorFunc = XSetErrorHandler(vidError);
	errorOccurred = 0;
	XF86VidModeLockModeSwitch(Tk_Display(tkwin),
				Tk_ScreenNumber(tkwin), lock);
	XSync(Tk_Display(tkwin), False);
	XSetErrorHandler(savErrorFunc);
	if (errorOccurred) {
		Tcl_AppendResult(interp, "Unable to ",
			(lock? "":"un"), "lock mode switching: ",
			errMsgBuf, (char *) NULL);
		return TCL_ERROR;
	}
	return TCL_OK;
}

/*
   Change to the previous/next video mode
*/

int
TCL_XF86VidModeSwitchMode(clientData, interp, argc, argv)
    ClientData	clientData;
    Tcl_Interp	*interp;
    int		argc;
    char	*argv[];
{
#define PREV -1
#define NEXT 1
	int direction;
	Tk_Window tkwin;
	static char usagemsg[] = "Usage: xf86vid_switchmode previous|next";

        if (argc != 2) {
                Tcl_SetResult(interp, usagemsg, TCL_STATIC);
                return TCL_ERROR;
        }

	if (!strncmp(argv[1], "prev", 4)) {
		direction = PREV;
	} else if (!strcmp(argv[1], "next")) {
		direction = NEXT;
	} else {
                Tcl_SetResult(interp, usagemsg, TCL_STATIC);
                return TCL_ERROR;
        }

        if ((tkwin = Tk_MainWindow(interp)) == NULL)
                return TCL_ERROR;
 
	XSync(Tk_Display(tkwin), False);
	savErrorFunc = XSetErrorHandler(vidError);
	errorOccurred = 0;
	XF86VidModeSwitchMode(Tk_Display(tkwin),
				Tk_ScreenNumber(tkwin), direction);
	XSync(Tk_Display(tkwin), False);
	XSetErrorHandler(savErrorFunc);
	if (errorOccurred) {
		Tcl_AppendResult(interp,
			"Unable to switch modes: ",
			errMsgBuf, (char *) NULL);
		return TCL_ERROR;
	}
	return TCL_OK;
#undef PREV
#undef NEXT
}

