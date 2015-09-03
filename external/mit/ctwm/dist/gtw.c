/* 
 *  [ ctwm ]
 *
 *  Copyright 1992 Claude Lecommandeur.
 *            
 * Permission to use, copy, modify  and distribute this software  [ctwm] and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above  copyright notice appear  in all copies and that both that
 * copyright notice and this permission notice appear in supporting documen-
 * tation, and that the name of  Claude Lecommandeur not be used in adverti-
 * sing or  publicity  pertaining to  distribution of  the software  without
 * specific, written prior permission. Claude Lecommandeur make no represen-
 * tations  about the suitability  of this software  for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * Claude Lecommandeur DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL  Claude Lecommandeur  BE LIABLE FOR ANY SPECIAL,  INDIRECT OR
 * CONSEQUENTIAL  DAMAGES OR ANY  DAMAGES WHATSOEVER  RESULTING FROM LOSS OF
 * USE, DATA  OR PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR
 * OTHER  TORTIOUS ACTION,  ARISING OUT OF OR IN  CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Claude Lecommandeur [ lecom@sic.epfl.ch ][ April 1992 ]
 */
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
/*
#include <X11/Xmu/CharSet.h>
#include <X11/Xresource.h>
*/
#ifdef macII
int strcmp(); /* missing from string.h in AUX 2.0 */
#endif

Atom     _XA_WM_CURRENTWORKSPACE, _XA_WM_OCCUPATION;
Display *dpy;

main (argc, argv)
int argc;
char **argv;
{
    Window w;

    dpy = XOpenDisplay (NULL);
    if (dpy == NULL) {
	fprintf (stderr, "Can't open display\n");
	exit (1);
    }

    switch (argc) {
	case 2:
	    gotoWorkspace (argv [1]);
	    break;

	case 3:
	    sscanf (argv [1], "%x", &w);
	    changeOccupation (w, argv [2]);
	    break;

	default:
	    fprintf (stderr, "usage %s name\n", argv [0]);
	    break;

    }
}

gotoWorkspace (name)
char *name;
{
    _XA_WM_CURRENTWORKSPACE = XInternAtom (dpy, "WM_CURRENTWORKSPACE", True);
    if (_XA_WM_CURRENTWORKSPACE == None) {
	fprintf (stderr, "Can't get WM_CURRENTWORKSPACE atom\n");
	exit (1);
    }

    XChangeProperty (dpy, RootWindow (dpy, 0), _XA_WM_CURRENTWORKSPACE, XA_STRING, 8, 
		     PropModeReplace, (unsigned char *) name, strlen (name));
    XFlush (dpy);
}

changeOccupation (w, occup)
Window w;
char *occup;
{
    _XA_WM_OCCUPATION = XInternAtom (dpy, "WM_OCCUPATION", True);
    if (_XA_WM_OCCUPATION == None) {
	fprintf (stderr, "Can't get WM_WORKSPACES atom\n");
	exit (1);
    }

    XChangeProperty (dpy, w, _XA_WM_OCCUPATION, XA_STRING, 8, 
		     PropModeReplace, (unsigned char *) occup, strlen (occup));
    XFlush (dpy);
}
