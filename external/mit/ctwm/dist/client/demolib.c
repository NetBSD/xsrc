/*
 * Copyright 1993 Claude Lecommandeur.
 */
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "ctwm_client.h"

Window awindow = 0x5c0000d;
char *awspc1 = "lecom", *awspc2 = "root";

int
main(int argc, char *argv[])
{
	Display *dpy;
	char   **wlist, **wl, **occupation;
	char   *cur;
	int    status;

	dpy = XOpenDisplay(NULL);
	if(dpy == NULL) {
		fprintf(stderr, "Can't open display\n");
		exit(1);
	}

	/****************************************************************/

	if(! CtwmIsRunning(dpy, 0)) {
		fprintf(stderr, "ctwm is not running\n");
		exit(1);
	}

	/****************************************************************/

	wlist = CtwmListWorkspaces(dpy, 0);
	if(wlist == NULL) {
		fprintf(stderr, "cannot obtain workspaces list\n");
		exit(1);
	}
	printf("list of workspaces : ");
	wl = wlist;
	while(*wl) {
		printf("\"%s\" ", *wl++);
	}
	printf("\n");

	/****************************************************************/

	cur = CtwmCurrentWorkspace(dpy, 0);
	if(cur == NULL) {
		fprintf(stderr, "cannot obtain current workspace\n");
		exit(1);
	}
	printf("current workspace : %s\n", cur);

	/****************************************************************/

	status = CtwmChangeWorkspace(dpy, 0, awspc1);
	if(! status) {
		fprintf(stderr, "cannot change the current workspace\n");
		exit(1);
	}

	/****************************************************************/

	wlist = CtwmCurrentOccupation(dpy, awindow);
	if(wlist == NULL) {
		fprintf(stderr, "cannot obtain occupation of window %lu\n", awindow);
		exit(1);
	}
	printf("Occupation of window %lu: ", awindow);
	wl = wlist;
	while(*wl) {
		printf("\"%s\" ", *wl++);
	}
	printf("\n");

	/****************************************************************/

	occupation = calloc(3, sizeof(char *));
	occupation [0] = awspc1;
	occupation [1] = awspc2;
	occupation [2] = NULL;
	status = CtwmSetOccupation(dpy, awindow, occupation);
	if(! status) {
		fprintf(stderr, "cannot change the occupation of window %lu\n", awindow);
	}
	printf("occupation of window %lu changed to 'lecom', 'root'\n", awindow);

	/****************************************************************/
	status = CtwmAddToCurrentWorkspace(dpy, awindow);
	if(! status) {
		fprintf(stderr, "cannot change the occupation of window %lu\n", awindow);
	}
	printf("window %lu now occupy the current workspace\n", awindow);
}

