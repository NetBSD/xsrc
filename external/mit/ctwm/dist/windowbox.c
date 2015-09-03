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
#include "twm.h"
#ifdef VMS
#include <decw$include/Xatom.h>
#else
#include <X11/Xatom.h>
#endif
#include "screen.h"
#include "add_window.h"
#include "resize.h"
#include "windowbox.h"

name_list **addWindowBox (char *boxname, char *geometry)
{
    WindowBox *winbox;

#if 0
    printf ("addWindowBox : name = %s, geometry = %s\n", boxname, geometry);
#endif
    winbox = (WindowBox*) malloc (sizeof (WindowBox));
    winbox->next     = NULL;
    winbox->name     = strdup (boxname);
    winbox->geometry = strdup (geometry);
    winbox->winlist  = NULL;
    if (!Scr->FirstWindowBox) Scr->FirstWindowBox = winbox;
    return (&(winbox->winlist));
}

void createWindowBoxes (void)
{
    WindowBox *winbox;
    char title [128];
    XWMHints	  wmhints;
    XSizeHints	  sizehints;

    for (winbox = Scr->FirstWindowBox; winbox; winbox = winbox->next) {
	int mask, x, y, gravity;
	unsigned int w, h;
	Window win;

	mask = XParseGeometry (winbox->geometry, &x, &y, &w, &h);
	if (mask & XNegative) {
	    x += Scr->rootw  - w;
	    gravity = (mask & YNegative) ? SouthEastGravity : NorthEastGravity;
	} else {
	    gravity = (mask & YNegative) ? SouthWestGravity : NorthWestGravity;
	}
	if (mask & YNegative) y += Scr->rooth - h;

	win = XCreateSimpleWindow (dpy, Scr->Root, x, y, w, h, 0, Scr->Black, Scr->White);
	/*printf ("createWindowBoxes : name = %s, win = 0x%x, x = %d, y = %d, w = %d, h = %d\n",
		winbox->name, win, x, y, w, h); */
	sprintf (title, "%s", winbox->name);
	XSetStandardProperties (dpy, win, title, title, None, NULL, 0, NULL);
	sizehints.flags  = USPosition | USSize | PWinGravity;
	sizehints.x      = x;
	sizehints.y      = y;
	sizehints.width  = w;
	sizehints.height = h;
	sizehints.win_gravity = gravity;
	XSetWMSizeHints (dpy, win, &sizehints, XA_WM_NORMAL_HINTS);

	wmhints.initial_state = NormalState;
	wmhints.input         = True;
	wmhints.flags         = InputHint | StateHint;
	XSetWMHints (dpy, win, &wmhints);

	winbox->window = win;
	winbox->twmwin = AddWindow (win, 2, NULL);
	if (!winbox->twmwin) {
	    fprintf (stderr, "cannot create %s window box, exiting...\n", winbox->name);
	    exit (1);
	}
	winbox->twmwin->iswinbox = TRUE;
	XMapWindow (dpy, win);
    }
}

WindowBox *findWindowBox (TwmWindow *twmwin)
{
    WindowBox *winbox;
    if (twmwin->iswinbox) return ((WindowBox*)0);
    if (!Scr->FirstWindowBox) return ((WindowBox*)0);
    for (winbox = Scr->FirstWindowBox; winbox; winbox = winbox->next) {
	if (LookInList (winbox->winlist, twmwin->full_name, &twmwin->class)) {
	    if (visible (winbox->twmwin)) {
		twmwin->winbox = winbox;
		return (winbox);
	    }
	}
    }
    return ((WindowBox*)0);
}

void ConstrainedToWinBox (TwmWindow *twmwin, int x, int y, int *nx, int *ny)
{
    XWindowAttributes attr;

    *nx = x; *ny = y;
    XGetWindowAttributes (dpy, twmwin->winbox->window, &attr);
    if (x < 0) *nx = 0;
    if (y < 0) *ny = 0;
    if (x >  attr.width - 1) *nx = attr.width - 1;
    if (y > attr.height - 1) *ny = attr.height - 1;
}

void fittocontent (TwmWindow *twmwin)
{
    TwmWindow	*t;
    int minx, miny, maxx, maxy, x, y, w, h;
    minx = Scr->rootw;
    miny = Scr->rooth;
    maxx = 0;
    maxy = 0;
    for (t = Scr->FirstWindow; t != NULL; t = t->next) {
	if (t->winbox && (t->winbox->twmwin == twmwin)) {
	    if (t->frame_x < minx) minx = t->frame_x;
	    if (t->frame_y < miny) miny = t->frame_y;
	    w = t->frame_width  + 2 * t->frame_bw;
	    h = t->frame_height + 2 * t->frame_bw;
	    if (t->frame_x + w > maxx) maxx = t->frame_x + w;
	    if (t->frame_y + h > maxy) maxy = t->frame_y + h;
	}
    }
    x = twmwin->frame_x + minx;
    y = twmwin->frame_y + miny;
    w = maxx - minx + 2 * twmwin->frame_bw3D;
    h = maxy - miny + 2 * twmwin->frame_bw3D;
    SetupWindow (twmwin, x, y, w, h, -1);
    for (t = Scr->FirstWindow; t != NULL; t = t->next) {
	if (t->winbox && (t->winbox->twmwin == twmwin)) {
	    SetupWindow (t, t->frame_x - minx, t->frame_y - miny,
			t->frame_width, t->frame_height, -1);
	}
    }
}
