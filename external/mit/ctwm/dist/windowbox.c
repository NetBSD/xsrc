/*
 * Copyright 1992 Claude Lecommandeur.
 */


#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xatom.h>

#include "screen.h"
#include "add_window.h"
#include "list.h"
#include "windowbox.h"
#include "win_decorations.h"
#include "win_resize.h"
#include "win_utils.h"
#include "xparsegeometry.h"

name_list **addWindowBox(char *boxname, char *geometry)
{
	WindowBox *winbox;

#if 0
	printf("addWindowBox : name = %s, geometry = %s\n", boxname, geometry);
#endif
	winbox = malloc(sizeof(WindowBox));
	winbox->next     = NULL;
	winbox->name     = strdup(boxname);
	winbox->geometry = strdup(geometry);
	winbox->winlist  = NULL;
	if(!Scr->FirstWindowBox) {
		Scr->FirstWindowBox = winbox;
	}
	return (&(winbox->winlist));
}

void createWindowBoxes(void)
{
	WindowBox *winbox;
	char title [128];
	XWMHints      wmhints;
	XSizeHints    sizehints;

	for(winbox = Scr->FirstWindowBox; winbox; winbox = winbox->next) {
		int mask, x, y, gravity;
		unsigned int w, h;
		Window win;

		mask = RLayoutXParseGeometry(Scr->Layout, winbox->geometry,
		                             &x, &y, &w, &h);
		if(mask & XNegative) {
			x += Scr->rootw  - w;
			gravity = (mask & YNegative) ? SouthEastGravity : NorthEastGravity;
		}
		else {
			gravity = (mask & YNegative) ? SouthWestGravity : NorthWestGravity;
		}
		if(mask & YNegative) {
			y += Scr->rooth - h;
		}

		win = XCreateSimpleWindow(dpy, Scr->Root, x, y, w, h, 0, Scr->Black,
		                          Scr->White);
#if 0
		printf("createWindowBoxes : name = %s, win = 0x%x, x = %d, y = %d, w = %d, h = %d\n",
		       winbox->name, win, x, y, w, h);
#endif
		sprintf(title, "%s", winbox->name);

		sizehints.flags  = USPosition | USSize | PWinGravity;
		sizehints.x      = x;
		sizehints.y      = y;
		sizehints.width  = w;
		sizehints.height = h;
		sizehints.win_gravity = gravity;

		wmhints.initial_state = NormalState;
		wmhints.input         = True;
		wmhints.flags         = InputHint | StateHint;

		XmbSetWMProperties(dpy, win, title, title, NULL, 0,
		                   &sizehints, &wmhints, NULL);

		winbox->window = win;
		winbox->twmwin = AddWindow(win, AWT_WINDOWBOX, NULL, Scr->currentvs);
		if(!winbox->twmwin) {
			fprintf(stderr, "cannot create %s window box, exiting...\n", winbox->name);
			exit(1);
		}
		winbox->twmwin->iswinbox = true;
		XMapWindow(dpy, win);
	}
}

WindowBox *findWindowBox(TwmWindow *twmwin)
{
	WindowBox *winbox;
	if(twmwin->iswinbox) {
		return NULL;
	}
	if(!Scr->FirstWindowBox) {
		return NULL;
	}
	for(winbox = Scr->FirstWindowBox; winbox; winbox = winbox->next) {
		if(LookInList(winbox->winlist, twmwin->name, &twmwin->class)) {
			if(visible(winbox->twmwin)) {
				twmwin->winbox = winbox;
				return winbox;
			}
		}
	}
	return NULL;
}

void ConstrainedToWinBox(TwmWindow *twmwin, int x, int y, int *nx, int *ny)
{
	XWindowAttributes attr;

	*nx = x;
	*ny = y;
	XGetWindowAttributes(dpy, twmwin->winbox->window, &attr);
	if(x < 0) {
		*nx = 0;
	}
	if(y < 0) {
		*ny = 0;
	}
	if(x >  attr.width - 1) {
		*nx = attr.width - 1;
	}
	if(y > attr.height - 1) {
		*ny = attr.height - 1;
	}
}

void fittocontent(TwmWindow *twmwin)
{
	TwmWindow   *t;
	int minx, miny, maxx, maxy, x, y, w, h;
	minx = Scr->rootw;
	miny = Scr->rooth;
	maxx = 0;
	maxy = 0;
	for(t = Scr->FirstWindow; t != NULL; t = t->next) {
		if(t->winbox && (t->winbox->twmwin == twmwin)) {
			if(t->frame_x < minx) {
				minx = t->frame_x;
			}
			if(t->frame_y < miny) {
				miny = t->frame_y;
			}
			w = t->frame_width  + 2 * t->frame_bw;
			h = t->frame_height + 2 * t->frame_bw;
			if(t->frame_x + w > maxx) {
				maxx = t->frame_x + w;
			}
			if(t->frame_y + h > maxy) {
				maxy = t->frame_y + h;
			}
		}
	}
	x = twmwin->frame_x + minx;
	y = twmwin->frame_y + miny;
	w = maxx - minx + 2 * twmwin->frame_bw3D;
	h = maxy - miny + 2 * twmwin->frame_bw3D;
	SetupWindow(twmwin, x, y, w, h, -1);
	for(t = Scr->FirstWindow; t != NULL; t = t->next) {
		if(t->winbox && (t->winbox->twmwin == twmwin)) {
			SetupWindow(t, t->frame_x - minx, t->frame_y - miny,
			            t->frame_width, t->frame_height, -1);
		}
	}
}
