/*
 * Copyright 1989 Massachusetts Institute of Technology
 * Copyright 1992 Claude Lecommandeur.
 */

/***********************************************************************
 *
 * $XConsortium: cursor.c,v 1.10 89/12/14 14:52:23 jim Exp $
 *
 * cursor creation code
 *
 * 05-Apr-89 Thomas E. LaStrange        File created
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 *
 ***********************************************************************/

#include "ctwm.h"

#include <stdio.h>

#include <X11/cursorfont.h>

#include "screen.h"
#include "cursor.h"
#include "image_bitmap.h"

static struct _CursorName {
	const char          *name;
	unsigned int        shape;
	Cursor              cursor;
} cursor_names[] = {

	{"X_cursor",            XC_X_cursor,            None},
	{"arrow",               XC_arrow,               None},
	{"based_arrow_down",    XC_based_arrow_down,    None},
	{"based_arrow_up",      XC_based_arrow_up,      None},
	{"boat",                XC_boat,                None},
	{"bogosity",            XC_bogosity,            None},
	{"bottom_left_corner",  XC_bottom_left_corner,  None},
	{"bottom_right_corner", XC_bottom_right_corner, None},
	{"bottom_side",         XC_bottom_side,         None},
	{"bottom_tee",          XC_bottom_tee,          None},
	{"box_spiral",          XC_box_spiral,          None},
	{"center_ptr",          XC_center_ptr,          None},
	{"circle",              XC_circle,              None},
	{"clock",               XC_clock,               None},
	{"coffee_mug",          XC_coffee_mug,          None},
	{"cross",               XC_cross,               None},
	{"cross_reverse",       XC_cross_reverse,       None},
	{"crosshair",           XC_crosshair,           None},
	{"diamond_cross",       XC_diamond_cross,       None},
	{"dot",                 XC_dot,                 None},
	{"dotbox",              XC_dotbox,              None},
	{"double_arrow",        XC_double_arrow,        None},
	{"draft_large",         XC_draft_large,         None},
	{"draft_small",         XC_draft_small,         None},
	{"draped_box",          XC_draped_box,          None},
	{"exchange",            XC_exchange,            None},
	{"fleur",               XC_fleur,               None},
	{"gobbler",             XC_gobbler,             None},
	{"gumby",               XC_gumby,               None},
	{"hand1",               XC_hand1,               None},
	{"hand2",               XC_hand2,               None},
	{"heart",               XC_heart,               None},
	{"icon",                XC_icon,                None},
	{"iron_cross",          XC_iron_cross,          None},
	{"left_ptr",            XC_left_ptr,            None},
	{"left_side",           XC_left_side,           None},
	{"left_tee",            XC_left_tee,            None},
	{"leftbutton",          XC_leftbutton,          None},
	{"ll_angle",            XC_ll_angle,            None},
	{"lr_angle",            XC_lr_angle,            None},
	{"man",                 XC_man,                 None},
	{"middlebutton",        XC_middlebutton,        None},
	{"mouse",               XC_mouse,               None},
	{"pencil",              XC_pencil,              None},
	{"pirate",              XC_pirate,              None},
	{"plus",                XC_plus,                None},
	{"question_arrow",      XC_question_arrow,      None},
	{"right_ptr",           XC_right_ptr,           None},
	{"right_side",          XC_right_side,          None},
	{"right_tee",           XC_right_tee,           None},
	{"rightbutton",         XC_rightbutton,         None},
	{"rtl_logo",            XC_rtl_logo,            None},
	{"sailboat",            XC_sailboat,            None},
	{"sb_down_arrow",       XC_sb_down_arrow,       None},
	{"sb_h_double_arrow",   XC_sb_h_double_arrow,   None},
	{"sb_left_arrow",       XC_sb_left_arrow,       None},
	{"sb_right_arrow",      XC_sb_right_arrow,      None},
	{"sb_up_arrow",         XC_sb_up_arrow,         None},
	{"sb_v_double_arrow",   XC_sb_v_double_arrow,   None},
	{"shuttle",             XC_shuttle,             None},
	{"sizing",              XC_sizing,              None},
	{"spider",              XC_spider,              None},
	{"spraycan",            XC_spraycan,            None},
	{"star",                XC_star,                None},
	{"target",              XC_target,              None},
	{"tcross",              XC_tcross,              None},
	{"top_left_arrow",      XC_top_left_arrow,      None},
	{"top_left_corner",     XC_top_left_corner,     None},
	{"top_right_corner",    XC_top_right_corner,    None},
	{"top_side",            XC_top_side,            None},
	{"top_tee",             XC_top_tee,             None},
	{"trek",                XC_trek,                None},
	{"ul_angle",            XC_ul_angle,            None},
	{"umbrella",            XC_umbrella,            None},
	{"ur_angle",            XC_ur_angle,            None},
	{"watch",               XC_watch,               None},
	{"xterm",               XC_xterm,               None},
};

void NewFontCursor(Cursor *cp, const char *str)
{
	int i;
	const Display *ldpy = dpy;  // Give compiler help to hoist

	for(i = 0; i < sizeof(cursor_names) / sizeof(struct _CursorName); i++) {
		if(strcmp(str, cursor_names[i].name) == 0) {
			if(ldpy == NULL) {
				// No display connection, but we found it
				*cp = None;
				return;
			}
			if(cursor_names[i].cursor == None) {
				cursor_names[i].cursor = XCreateFontCursor(dpy,
				                         cursor_names[i].shape);
			}
			*cp = cursor_names[i].cursor;
			return;
		}
	}
	fprintf(stderr, "%s:  unable to find font cursor \"%s\"\n",
	        ProgramName, str);
}

int NewBitmapCursor(Cursor *cp, char *source, char *mask)
{
	XColor fore, back;
	int hotx, hoty;
	int sx, sy, mx, my;
	unsigned int sw, sh, mw, mh;
	Pixmap spm, mpm;
	Colormap cmap = Scr->RootColormaps.cwins[0]->colormap->c;

	if(dpy == NULL) {
		// Handle special cases like --cfgchk
		*cp = None;
		return 0;
	}

	fore.pixel = Scr->Black;
	XQueryColor(dpy, cmap, &fore);
	back.pixel = Scr->White;
	XQueryColor(dpy, cmap, &back);

	spm = GetBitmap(source);
	if((hotx = HotX) < 0) {
		hotx = 0;
	}
	if((hoty = HotY) < 0) {
		hoty = 0;
	}
	mpm = GetBitmap(mask);

	/* make sure they are the same size */

	XGetGeometry(dpy, spm, &JunkRoot, &sx, &sy, &sw, &sh, &JunkBW, &JunkDepth);
	XGetGeometry(dpy, mpm, &JunkRoot, &mx, &my, &mw, &mh, &JunkBW, &JunkDepth);
	if(sw != mw || sh != mh) {
		fprintf(stderr,
		        "%s:  cursor bitmaps \"%s\" and \"%s\" not the same size\n",
		        ProgramName, source, mask);
		return (1);
	}
	*cp = XCreatePixmapCursor(dpy, spm, mpm, &fore, &back, hotx, hoty);
	return (0);
}

Cursor MakeStringCursor(char *string)
{
	Cursor      cursor;
	XColor      black, white;
	Pixmap      bitmap;
	unsigned int width, height, middle;
	GC          gc;
	Colormap    cmap = Scr->RootColormaps.cwins[0]->colormap->c;
	MyFont      myfont = Scr->TitleBarFont;
	XRectangle inc_rect;
	XRectangle logical_rect;

	black.pixel = Scr->Black;
	XQueryColor(dpy, cmap, &black);
	white.pixel = Scr->White;
	XQueryColor(dpy, cmap, &white);

	XmbTextExtents(myfont.font_set, string, strlen(string),
	               &inc_rect, &logical_rect);
	width  = logical_rect.width  + 4;
	height = logical_rect.height + 2;
	middle = myfont.ascent;
	/*XQueryBestCursor (dpy, Scr->Root, width, height, &rwidth, &rheight);*/

	bitmap = XCreatePixmap(dpy, Scr->Root, width, height, 1);
	gc     = XCreateGC(dpy, bitmap, 0L, NULL);

	XSetForeground(dpy, gc, 0L);
	XFillRectangle(dpy, bitmap, gc, 0, 0, width, height);
	XSetForeground(dpy, gc, 1L);
	XDrawRectangle(dpy, bitmap, gc, 0, 0, width - 1, height - 1);

	XmbDrawString(dpy, bitmap, myfont.font_set,
	              gc, 2, middle, string, strlen(string));

	cursor = XCreatePixmapCursor(dpy, bitmap, None, &black, &white, 0, 0);
	XFreePixmap(dpy, bitmap);
	XFreeGC(dpy, gc);
	return (cursor);
}
