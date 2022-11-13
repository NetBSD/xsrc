/*

Copyright 1988, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

/*
 * xkill - simple program for destroying unwanted clients
 * Author:  Jim Fulton, MIT X Consortium; Dana Chee, Bellcore
 */

/*
 * Warning, this is a very dangerous client....
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xproto.h>

#include <X11/Xmu/WinUtil.h>

static char *ProgramName;

#define SelectButtonAny (-1)
#define SelectButtonFirst (-2)

static int parse_button ( const char *s, int *buttonp );
static XID get_window_id ( Display *dpy, int screen, int button, const char *msg );
static int catch_window_errors ( Display *dpy, XErrorEvent *ev );
static int kill_all_windows ( Display *dpy, int screenno, Bool top );
static int verify_okay_to_kill ( Display *dpy, int screenno );
static Bool wm_state_set ( Display *dpy, Window win );
static Bool wm_running ( Display *dpy, int screenno );

static void _X_NORETURN
Exit(int code, Display *dpy)
{
    if (dpy) {
	XCloseDisplay (dpy);
    }
    exit (code);
}

static void _X_NORETURN
usage(const char *errmsg)
{
    const char *options =
"where options include:\n"
"    -display displayname    X server to contact\n"
"    -id resource            resource whose client is to be killed\n"
"    -frame                  don't ignore window manager frames\n"
"    -button number          specific button to be pressed to select window\n"
"    -all                    kill all clients with top level windows\n"
"    -version                print version and exit\n"
"\n";

    if (errmsg != NULL)
        fprintf (stderr, "%s: %s\n\n", ProgramName, errmsg);

    fprintf (stderr, "usage:  %s [-option ...]\n%s",
	     ProgramName, options);
    Exit (1, NULL);
}

int
main(int argc, char *argv[])
{
    Display *dpy = NULL;
    char *displayname = NULL;		/* name of server to contact */
    int screenno;			/* screen number of dpy */
    XID id = None;			/* resource to kill */
    char *button_name = NULL;		/* name of button for window select */
    int button;				/* button number or negative for all */
    Bool kill_all = False;
    Bool top = False;

    ProgramName = argv[0];
    button = SelectButtonFirst;

    for (int i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case 'd':			/* -display displayname */
		if (++i >= argc) usage ("-display requires an argument");
		displayname = argv[i];
		continue;
	      case 'i':			/* -id resourceid */
		if (++i >= argc) usage ("-id requires an argument");
		id = strtoul (argv[i], NULL, 0);
		if (id == 0 || id >= 0xFFFFFFFFU) {
		    fprintf (stderr, "%s:  invalid id \"%s\"\n",
			     ProgramName, argv[i]);
		    Exit (1, dpy);
		}
		continue;
	      case 'b':			/* -button number */
		if (++i >= argc) usage ("-button requires an argument");
		button_name = argv[i];
		continue;
	      case 'f':			/* -frame */
		top = True;
		continue;
	      case 'a':			/* -all */
		kill_all = True;
		continue;
              case 'v':
                puts(PACKAGE_STRING);
                exit(0);
	      default:
                fprintf(stderr, "%s: unrecognized argument %s\n\n",
                        ProgramName, arg);
		usage (NULL);
	    }
	} else {
            fprintf(stderr, "%s: unrecognized argument %s\n\n",
                    ProgramName, arg);
            usage (NULL);
	}
    }					/* end for */

    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display \"%s\"\n",
		 ProgramName, XDisplayName (displayname));
	Exit (1, dpy);
    }
    screenno = DefaultScreen (dpy);

    if (kill_all) {
	if (verify_okay_to_kill (dpy, screenno)) 
	  kill_all_windows (dpy, screenno, top);
	Exit (0, dpy);
    }

    /*
     * if no id was given, we need to choose a window
     */

    if (id == None) {
	if (!button_name)
	    button_name = XGetDefault (dpy, ProgramName, "Button");

	if (button_name && !parse_button (button_name, &button)) {
	    fprintf (stderr, "%s:  invalid button specification \"%s\"\n",
		     ProgramName, button_name);
	    Exit (1, dpy);
	}

	if (button >= 0 || button == SelectButtonFirst) {
	    unsigned char pointer_map[256];	 /* 8 bits of pointer num */
	    int count;
	    unsigned int ub = (unsigned int) button;


	    count = XGetPointerMapping (dpy, pointer_map, 256);
	    if (count <= 0) {
		fprintf (stderr, 
			 "%s:  no pointer mapping, can't select window\n",
			 ProgramName);
		Exit (1, dpy);
	    }

	    if (button >= 0) {			/* check button */
		int j;

		for (j = 0; j < count; j++) {
		    if (ub == (unsigned int) pointer_map[j]) break;
		}
		if (j == count) {
		    fprintf (stderr,
	 "%s:  no button number %u in pointer map, can't select window\n",
			     ProgramName, ub);
		    Exit (1, dpy);
	        }
	    } else {				/* get first entry */
		button = (int) ((unsigned int) pointer_map[0]);
	    }
	}
	if ((id = get_window_id (dpy, screenno, button,
				"the window whose client you wish to kill"))) {
	    if (id == RootWindow(dpy,screenno)) id = None;
	    else if (!top) {
		XID indicated = id;
		if ((id = XmuClientWindow(dpy, indicated)) == indicated) {
		    
		    /* Try not to kill the window manager when the user
		     * indicates an icon to xkill.
		     */

		    if (! wm_state_set(dpy, id) && wm_running(dpy, screenno))
			id = None;

		} 
	    }
	}
    }

    if (id != None) {
	printf ("%s:  killing creator of resource 0x%lx\n", ProgramName, id);
	XSync (dpy, 0);			/* give xterm a chance */
	XKillClient (dpy, id);
	XSync (dpy, 0);
    }

    Exit (0, dpy);
    /*NOTREACHED*/
    return 0;
}

static int 
parse_button(const char *s, int *buttonp)
{
    if (strcasecmp (s, "any") == 0) {
	*buttonp = SelectButtonAny;
	return (1);
    }

    /* check for non-numeric input */
    for (const char *cp = s; *cp; cp++) {
	if (!(isascii (*cp) && isdigit (*cp))) return (0);  /* bogus name */
    }

    *buttonp = atoi (s);
    return (1);
}

static XID 
get_window_id(Display *dpy, int screen, int button, const char *msg)
{
    Cursor cursor;		/* cursor to use when selecting */
    Window root;		/* the current root */
    Window retwin = None;	/* the window that got selected */
    int retbutton = -1;		/* button used to select window */
    int pressed = 0;		/* count of number of buttons pressed */

#define MASK (ButtonPressMask | ButtonReleaseMask)

    root = RootWindow (dpy, screen);
    cursor = XCreateFontCursor (dpy, XC_pirate);
    if (cursor == None) {
	fprintf (stderr, "%s:  unable to create selection cursor\n",
		 ProgramName);
	Exit (1, dpy);
    }

    printf ("Select %s with ", msg);
    if (button == -1)
      printf ("any button");
    else
      printf ("button %d", button);
    printf ("....\n");
    XSync (dpy, 0);			/* give xterm a chance */

    if (XGrabPointer (dpy, root, False, MASK, GrabModeSync, GrabModeAsync, 
    		      None, cursor, CurrentTime) != GrabSuccess) {
	fprintf (stderr, "%s:  unable to grab cursor\n", ProgramName);
	Exit (1, dpy);
    }

    /* from dsimple.c in xwininfo */
    while (retwin == None || pressed != 0) {
	XEvent event;

	XAllowEvents (dpy, SyncPointer, CurrentTime);
	XWindowEvent (dpy, root, MASK, &event);
	switch (event.type) {
	  case ButtonPress:
	    if (retwin == None) {
		retbutton = event.xbutton.button;
		retwin = ((event.xbutton.subwindow != None) ?
			  event.xbutton.subwindow : root);
	    }
	    pressed++;
	    continue;
	  case ButtonRelease:
	    if (pressed > 0) pressed--;
	    continue;
	}					/* end switch */
    }						/* end for */

    XUngrabPointer (dpy, CurrentTime);
    XFreeCursor (dpy, cursor);
    XSync (dpy, 0);

    return ((button == -1 || retbutton == button) ? retwin : None);
}


static int 
catch_window_errors(_X_UNUSED Display *dpy, _X_UNUSED XErrorEvent *ev)
{
    return 0;
}

static int 
kill_all_windows(Display *dpy, int screenno, Bool top)
{
    Window root = RootWindow (dpy, screenno);
    Window dummywindow;
    Window *children;
    unsigned int nchildren;
    unsigned int i;
    XWindowAttributes attr;

    XSync (dpy, 0);
    XSetErrorHandler (catch_window_errors);

    nchildren = 0;
    XQueryTree (dpy, root, &dummywindow, &dummywindow, &children, &nchildren);
    if (!top) {
	for (i = 0; i < nchildren; i++) {
	    if (XGetWindowAttributes(dpy, children[i], &attr) &&
		(attr.map_state == IsViewable))
		children[i] = XmuClientWindow(dpy, children[i]);
	    else
		children[i] = 0;
	}
    }
    for (i = 0; i < nchildren; i++) {
	if (children[i])
	    XKillClient (dpy, children[i]);
    }
    XFree ((char *)children);

    XSync (dpy, 0);
    XSetErrorHandler (NULL);		/* pretty stupid way to do things... */

    return 0;
}

/*
 * ask the user to press in the root with each button in succession
 */
static int 
verify_okay_to_kill(Display *dpy, int screenno)
{
    unsigned char pointer_map[256];
    int count = XGetPointerMapping (dpy, pointer_map, 256);
    const char *msg = "the root window";
    Window root = RootWindow (dpy, screenno);
    int okay = 0;

    for (int i = 0; i < count; i++) {
	int button = (int) pointer_map[i];
	if (button == 0) continue;	/* disabled */
	if (get_window_id (dpy, screenno, button, msg) != root) {
	    okay = 0;
	    break;
	}
	okay++;				/* must have at least one button */
    }
    if (okay) {
	return 1;
    } else {
	printf ("Aborting.\n");
	return 0;
    }
}

/* Return True if the property WM_STATE is set on the window, otherwise
 * return False.
 */
static Bool 
wm_state_set(Display *dpy, Window win) 
{
    Atom wm_state;
    Atom actual_type;
    int success;
    int actual_format;
    unsigned long nitems, remaining;
    unsigned char* prop = NULL;

    wm_state = XInternAtom(dpy, "WM_STATE", True);
    if (wm_state == None) return False;
    success = XGetWindowProperty(dpy, win, wm_state, 0L, 0L, False, 
				 AnyPropertyType, &actual_type, &actual_format,
				 &nitems, &remaining, &prop);
    if (prop) XFree((char *) prop);
    return (success == Success && actual_type != None && actual_format);
}

/* Using a heuristic method, return True if a window manager is running,
 * otherwise, return False.
 */

static Bool 
wm_running(Display *dpy, int screenno)
{
    XWindowAttributes	xwa;
    Status		status;

    status = XGetWindowAttributes(dpy, RootWindow(dpy, screenno), &xwa);
    return (status &&
	    ((xwa.all_event_masks & SubstructureRedirectMask) ||
	     (xwa.all_event_masks & SubstructureNotifyMask)));
}
