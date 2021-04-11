/*
 * Copyright 1992 Claude Lecommandeur.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

Atom     _XA_WM_CURRENTWORKSPACE, _XA_WM_OCCUPATION;
Display *dpy;

void gotoWorkspace(char *name);
void changeOccupation(Window w, char *occup);

int
main(int argc, char **argv)
{
	Window w;

	dpy = XOpenDisplay(NULL);
	if(dpy == NULL) {
		fprintf(stderr, "Can't open display\n");
		exit(1);
	}

	switch(argc) {
		case 2:
			gotoWorkspace(argv [1]);
			break;

		case 3:
			sscanf(argv [1], "%x", (unsigned int *)&w);
			changeOccupation(w, argv [2]);
			break;

		default:
			fprintf(stderr, "usage %s name\n", argv [0]);
			break;

	}
}

void
gotoWorkspace(char *name)
{
	_XA_WM_CURRENTWORKSPACE = XInternAtom(dpy, "WM_CURRENTWORKSPACE", True);
	if(_XA_WM_CURRENTWORKSPACE == None) {
		fprintf(stderr, "Can't get WM_CURRENTWORKSPACE atom\n");
		exit(1);
	}

	XChangeProperty(dpy, RootWindow(dpy, 0), _XA_WM_CURRENTWORKSPACE, XA_STRING, 8,
	                PropModeReplace, (unsigned char *) name, strlen(name));
	XFlush(dpy);
}

void
changeOccupation(Window w, char *occup)
{
	_XA_WM_OCCUPATION = XInternAtom(dpy, "WM_OCCUPATION", True);
	if(_XA_WM_OCCUPATION == None) {
		fprintf(stderr, "Can't get WM_WORKSPACES atom\n");
		exit(1);
	}

	XChangeProperty(dpy, w, _XA_WM_OCCUPATION, XA_STRING, 8,
	                PropModeReplace, (unsigned char *) occup, strlen(occup));
	XFlush(dpy);
}
