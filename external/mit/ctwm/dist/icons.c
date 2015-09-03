/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
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

/**********************************************************************
 *
 * $XConsortium: icons.c,v 1.22 91/07/12 09:58:38 dave Exp $
 *
 * Icon releated routines
 *
 * 10-Apr-89 Tom LaStrange        Initial Version.
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 *
 **********************************************************************/

#include <stdio.h>
#include "twm.h"
#include "screen.h"
#include "icons.h"
#include "list.h"
#include "parse.h"
#include "util.h"

extern void twmrc_error_prefix(void);
extern Bool AnimationPending;
extern Bool AnimationActive;
extern Bool MaybeAnimate;

#define iconWidth(w)	(w->icon->border_width * 2 + \
			Scr->ShrinkIconTitles ? w->icon->width : w->icon->w_width)
#define iconHeight(w)	(w->icon->border_width * 2 + w->icon->w_height)

static void splitEntry (IconEntry *ie, int grav1, int grav2, int w, int h)
{
    IconEntry	*new;

    switch (grav1) {
    case D_NORTH:
    case D_SOUTH:
	if (w != ie->w)
	    splitEntry (ie, grav2, grav1, w, ie->h);
	if (h != ie->h) {
	    new = (IconEntry *)malloc (sizeof (IconEntry));
	    new->twm_win = 0;
	    new->used = 0;
	    new->next = ie->next;
	    ie->next = new;
	    new->x = ie->x;
	    new->h = (ie->h - h);
	    new->w = ie->w;
	    ie->h = h;
	    if (grav1 == D_SOUTH) {
		new->y = ie->y;
		ie->y = new->y + new->h;
	    } else
		new->y = ie->y + ie->h;
	}
	break;
    case D_EAST:
    case D_WEST:
	if (h != ie->h)
	    splitEntry (ie, grav2, grav1, ie->w, h);
	if (w != ie->w) {
	    new = (IconEntry *)malloc (sizeof (IconEntry));
	    new->twm_win = 0;
	    new->used = 0;
	    new->next = ie->next;
	    ie->next = new;
	    new->y = ie->y;
	    new->w = (ie->w - w);
	    new->h = ie->h;
	    ie->w = w;
	    if (grav1 == D_EAST) {
		new->x = ie->x;
		ie->x = new->x + new->w;
	    } else
		new->x = ie->x + ie->w;
	}
	break;
    }
}

static int roundUp (int v, int multiple)
{
    return ((v + multiple - 1) / multiple) * multiple;
}

static void PlaceIcon(TwmWindow *tmp_win, int def_x, int def_y,
		      int *final_x, int *final_y)
{
    IconRegion	*ir, *oldir;
    IconEntry	*ie;
    int		w = 0, h = 0;

    /*
     * First, check to see if the window is in a region's client list
     */
    ie = 0;
    for (ir = Scr->FirstRegion; ir; ir = ir->next) {
	if (LookInList(ir->clientlist, tmp_win->full_name, &tmp_win->class))
	{
	    w = roundUp (iconWidth (tmp_win), ir->stepx);
	    h = roundUp (iconHeight (tmp_win), ir->stepy);
	    for (ie = ir->entries; ie; ie=ie->next) {
	        if (ie->used)
		    continue;
	        if (ie->w >= w && ie->h >= h)
		    break;
	    }
	    if (ie)
	        break;
	}
    }

    /*
     * If not found in any region's client list, place anywhere
     */
    if (!ie)
    {
        for (ir = Scr->FirstRegion; ir; ir = ir->next) {
	    w = roundUp (iconWidth (tmp_win), ir->stepx);
	    h = roundUp (iconHeight (tmp_win), ir->stepy);
	    for (ie = ir->entries; ie; ie=ie->next) {
	        if (ie->used)
		    continue;
	        if (ie->w >= w && ie->h >= h)
		    break;
	    }
	    if (ie)
	        break;
        }
    }
    oldir = tmp_win->icon->ir;
    if (ie) {
	splitEntry (ie, ir->grav1, ir->grav2, w, h);
	ie->used = 1;
	ie->twm_win = tmp_win;
	switch (ir->Justification) {
	    case J_LEFT :
		*final_x = ie->x;
		break;
	    case J_UNDEF :
	    case J_CENTER :
		*final_x = ie->x + (ie->w - iconWidth (tmp_win)) / 2;
		break;
	    case J_RIGHT :
		*final_x = ie->x + ie->w - iconWidth (tmp_win);
		break;
	    case J_BORDER :
		if (ir->grav2 == D_EAST)
		    *final_x = ie->x + ie->w - iconWidth (tmp_win);
		else
		    *final_x = ie->x;
		break;
	}
	switch (ir->Alignement) {
	    case J_TOP :
		*final_y = ie->y;
		break;
	    case J_UNDEF :
	    case J_CENTER :
		*final_y = ie->y + (ie->h - iconHeight (tmp_win)) / 2;
		break;
	    case J_BOTTOM :
		*final_y = ie->y + ie->h - iconHeight (tmp_win);
		break;
	    case J_BORDER :
		if (ir->grav1 == D_SOUTH)
		    *final_y = ie->y + ie->h - iconHeight (tmp_win);
		else
		    *final_y = ie->y;
		break;
	}
	tmp_win->icon->ir = ir;
    } else {
	*final_x = def_x;
	*final_y = def_y;
	tmp_win->icon->ir = (IconRegion*)0;
	return;
    }
    if (Scr->ShrinkIconTitles && tmp_win->icon->has_title) {
	*final_x -= GetIconOffset (tmp_win->icon);
	if (tmp_win->icon->ir != oldir) ReshapeIcon (tmp_win->icon);
    }
    return;
}

static IconEntry *FindIconEntry (TwmWindow *tmp_win, IconRegion **irp)
{
    IconRegion	*ir;
    IconEntry	*ie;

    for (ir = Scr->FirstRegion; ir; ir = ir->next) {
	for (ie = ir->entries; ie; ie=ie->next)
	    if (ie->twm_win == tmp_win) {
		if (irp)
		    *irp = ir;
		return ie;
	    }
    }
    return 0;
}

int IconUp (TwmWindow *tmp_win)
{
    int		x, y;
    int		defx, defy;
    struct IconRegion *ir;

    /*
     * If the client specified a particular location, let's use it (this might
     * want to be an option at some point).  Otherwise, try to fit within the
     * icon region.
     */
    if (tmp_win->wmhints && (tmp_win->wmhints->flags & IconPositionHint))
      return (0);

    if (tmp_win->icon_moved) {
	if (!XGetGeometry (dpy, tmp_win->icon->w, &JunkRoot, &defx, &defy,
			   &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth))
	  return (1);

	x = defx + ((int) JunkWidth) / 2;
	y = defy + ((int) JunkHeight) / 2;

	for (ir = Scr->FirstRegion; ir; ir = ir->next) {
	    if (x >= ir->x && x < (ir->x + ir->w) &&
		y >= ir->y && y < (ir->y + ir->h))
	      break;
	}
	if (!ir) return (0);		/* outside icon regions, leave alone */
    }

    defx = -100;
    defy = -100;
    PlaceIcon(tmp_win, defx, defy, &x, &y);
    if (x != defx || y != defy) {
	XMoveWindow (dpy, tmp_win->icon->w, x, y);
	tmp_win->icon_moved = FALSE;	/* since we've restored it */
    }
    MaybeAnimate = True;
    return (0);
}

static IconEntry *prevIconEntry (IconEntry *ie, IconRegion *ir)
{
    IconEntry	*ip;

    if (ie == ir->entries)
	return 0;
    for (ip = ir->entries; ip->next != ie; ip=ip->next)
	;
    return ip;
}

/* old is being freed; and is adjacent to ie.  Merge
 * regions together
 */

static void mergeEntries (IconEntry *old, IconEntry *ie)
{
    if (old->y == ie->y) {
	ie->w = old->w + ie->w;
	if (old->x < ie->x)
	    ie->x = old->x;
    } else {
	ie->h = old->h + ie->h;
	if (old->y < ie->y)
	    ie->y = old->y;
    }
}

void IconDown (TwmWindow *tmp_win)
{
    IconEntry	*ie, *ip, *in;
    IconRegion	*ir;

    ie = FindIconEntry (tmp_win, &ir);
    if (ie) {
	ie->twm_win = 0;
	ie->used = 0;
	ip = prevIconEntry (ie, ir);
	in = ie->next;
	for (;;) {
	    if (ip && ip->used == 0 &&
	       ((ip->x == ie->x && ip->w == ie->w) ||
	        (ip->y == ie->y && ip->h == ie->h)))
	    {
	    	ip->next = ie->next;
	    	mergeEntries (ie, ip);
	    	free ((char *) ie);
		ie = ip;
	    	ip = prevIconEntry (ip, ir);
	    } else if (in && in->used == 0 &&
	       ((in->x == ie->x && in->w == ie->w) ||
	        (in->y == ie->y && in->h == ie->h)))
	    {
	    	ie->next = in->next;
	    	mergeEntries (in, ie);
	    	free ((char *) in);
	    	in = ie->next;
	    } else
		break;
	}
    }
}

name_list **AddIconRegion(char *geom,
			  int grav1, int grav2,
			  int stepx, int stepy,
			  char *ijust, char *just, char *align)
{
    IconRegion *ir;
    int mask, tmp;

    ir = (IconRegion *)malloc(sizeof(IconRegion));
    ir->next = NULL;

    if (Scr->LastRegion) Scr->LastRegion->next = ir;
    Scr->LastRegion = ir;
    if (!Scr->FirstRegion) Scr->FirstRegion = ir;

    ir->entries = NULL;
    ir->clientlist = NULL;
    ir->grav1 = grav1;
    ir->grav2 = grav2;
    if (stepx <= 0)
	stepx = 1;
    if (stepy <= 0)
	stepy = 1;
    ir->stepx = stepx;
    ir->stepy = stepy;
    ir->x = ir->y = ir->w = ir->h = 0;

    mask = XParseGeometry(geom, &ir->x, &ir->y, (unsigned int *)&ir->w, (unsigned int *)&ir->h);

    if (mask & XNegative) ir->x += Scr->rootw - ir->w;
    if (mask & YNegative) ir->y += Scr->rooth - ir->h;

    ir->entries = (IconEntry *)malloc(sizeof(IconEntry));
    ir->entries->next = 0;
    ir->entries->x = ir->x;
    ir->entries->y = ir->y;
    ir->entries->w = ir->w;
    ir->entries->h = ir->h;
    ir->entries->twm_win = 0;
    ir->entries->used = 0;

    tmp = ParseJustification (ijust);
    if ((tmp < 0) || (tmp == J_BORDER)) {
	twmrc_error_prefix();
	fprintf (stderr, "ignoring invalid IconRegion argument \"%s\"\n", ijust);
	tmp = J_UNDEF;
    }
    ir->TitleJustification = tmp;

    tmp = ParseJustification (just);
    if ((tmp = ParseJustification (just)) < 0) {
	twmrc_error_prefix();
	fprintf (stderr, "ignoring invalid IconRegion argument \"%s\"\n", just);
	tmp = J_UNDEF;
    }
    ir->Justification = tmp;

    if ((tmp = ParseAlignement (align)) < 0) {
	twmrc_error_prefix();
	fprintf (stderr, "ignoring invalid IconRegion argument \"%s\"\n", align);
	tmp = J_UNDEF;
    }
    ir->Alignement = tmp;

    return(&(ir->clientlist));
}

#ifdef comment
FreeIconEntries (ir)
    IconRegion	*ir;
{
    IconEntry	*ie, *tmp;

    for (ie = ir->entries; ie; ie=tmp)
    {
	tmp = ie->next;
	free ((char *) ie);
    }
}
FreeIconRegions()
{
    IconRegion *ir, *tmp;

    for (ir = Scr->FirstRegion; ir != NULL;)
    {
	tmp = ir;
	FreeIconEntries (ir);
	ir = ir->next;
	free((char *) tmp);
    }
    Scr->FirstRegion = NULL;
    Scr->LastRegion = NULL;
}
#endif

int CreateIconWindow(TwmWindow *tmp_win, int def_x, int def_y)
{
    unsigned long event_mask;
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    int final_x, final_y;
    int x;
    Icon	*icon;
    Image	*image = None;

    icon = (Icon*) malloc (sizeof (struct Icon));

    icon->border	= Scr->IconBorderColor;
    icon->iconc.fore	= Scr->IconC.fore;
    icon->iconc.back	= Scr->IconC.back;

    GetColorFromList(Scr->IconBorderColorL, tmp_win->full_name, &tmp_win->class,
	&icon->border);
    GetColorFromList(Scr->IconForegroundL, tmp_win->full_name, &tmp_win->class,
	&icon->iconc.fore);
    GetColorFromList(Scr->IconBackgroundL, tmp_win->full_name, &tmp_win->class,
	&icon->iconc.back);
    if (Scr->use3Diconmanagers && !Scr->BeNiceToColormap) GetShadeColors (&icon->iconc);

    FB(icon->iconc.fore, icon->iconc.back);

    icon->match   = match_none;
    icon->pattern = NULL;
    icon->image   = None;
    icon->ir      = (IconRegion*) 0;

    tmp_win->forced = FALSE;
    tmp_win->icon_not_ours = FALSE;

    /* now go through the steps to get an icon window,  if ForceIcon is 
     * set, then no matter what else is defined, the bitmap from the
     * .twmrc file is used
     */
    if (Scr->ForceIcon) {
	char *icon_name;

	icon_name = LookInNameList (Scr->IconNames, tmp_win->icon_name);
        if (icon_name != NULL) {
	    icon->pattern = LookPatternInNameList (Scr->IconNames, tmp_win->icon_name);
	    icon->match = match_icon;
	}
        if (icon->match == match_none)
	    icon_name = LookInNameList(Scr->IconNames, tmp_win->full_name);
        if ((icon->match == match_none) && (icon_name != NULL)) {
	    icon->pattern = LookPatternInNameList (Scr->IconNames, tmp_win->full_name);
	    icon->match = match_name;
	}
        if (icon->match == match_none)
	    icon_name = LookInList(Scr->IconNames, tmp_win->full_name, &tmp_win->class);
        if ((icon->match == match_none) && (icon_name != NULL)) {
	    icon->pattern = LookPatternInList (Scr->IconNames, tmp_win->full_name, &tmp_win->class);
	    icon->match = match_class;
	}
	if ((image  = GetImage (icon_name, icon->iconc)) != None) {
	    icon->image  = image;
	    icon->width  = image->width;
	    icon->height = image->height;
	    tmp_win->forced = TRUE;
	}
    }

    /* if the pixmap is still NULL, we didn't get one from the above code,
     * that could mean that ForceIcon was not set, or that the window
     * was not in the Icons list, now check the WM hints for an icon
     */
    if (image == None && tmp_win->wmhints &&
	tmp_win->wmhints->flags & IconPixmapHint) {
	if (XGetGeometry(dpy, tmp_win->wmhints->icon_pixmap,
		&JunkRoot, &JunkX, &JunkY, &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth)) {
	    image = (Image*) malloc (sizeof (struct _Image));
	    image->width  = JunkWidth;
	    image->height = JunkHeight;
	    image->pixmap = XCreatePixmap (dpy, Scr->Root, image->width,
					image->height, Scr->d_depth);
	    image->mask   = None;
	    image->next   = None;
	    if (JunkDepth == Scr->d_depth) 
		XCopyArea  (dpy, tmp_win->wmhints->icon_pixmap, image->pixmap, Scr->NormalGC,
			0, 0, image->width, image->height, 0, 0);
	    else
		XCopyPlane (dpy, tmp_win->wmhints->icon_pixmap, image->pixmap, Scr->NormalGC,
			0, 0, image->width, image->height, 0, 0, 1 );

	    icon->width   = image->width;
	    icon->height  = image->height;

	    if ((tmp_win->wmhints->flags & IconMaskHint) &&
		XGetGeometry(dpy, tmp_win->wmhints->icon_mask,
		    &JunkRoot, &JunkX, &JunkY, &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) &&
		(JunkDepth == 1)) {
		GC gc;

		image->mask = XCreatePixmap (dpy, Scr->Root, JunkWidth, JunkHeight, 1);
		if (image->mask) {
		    gc = XCreateGC (dpy, image->mask, 0, NULL);
		    if (gc) {
			XCopyArea (dpy, tmp_win->wmhints->icon_mask, image->mask, gc,
				0, 0, JunkWidth, JunkHeight, 0, 0);
			XFreeGC (dpy, gc);
		    }
		}
	    }
	    icon->image = image;
	}
    }

    /* if we still haven't got an icon, let's look in the Icon list 
     * if ForceIcon is not set
     */
    if (image == None && !Scr->ForceIcon) {
	char *icon_name;

	icon->match   = match_none;
	icon->pattern = NULL;
	icon_name = LookInNameList(Scr->IconNames, tmp_win->icon_name);
        if (icon_name != NULL) {
	    icon->pattern = LookPatternInNameList (Scr->IconNames, tmp_win->icon_name);
	    icon->match = match_icon;
	}
        if (icon->match == match_none)
	    icon_name = LookInNameList(Scr->IconNames, tmp_win->full_name);
        if ((icon->match == match_none) && (icon_name != NULL)) {
	    icon->pattern = LookPatternInNameList (Scr->IconNames, tmp_win->full_name);
	    icon->match = match_name;
	}
        if (icon->match == match_none)
	    icon_name = LookInList(Scr->IconNames, tmp_win->full_name, &tmp_win->class);
        if ((icon->match == match_none) && (icon_name != NULL)) {
	    icon->pattern = LookPatternInList (Scr->IconNames,
				tmp_win->full_name, &tmp_win->class);
	    icon->match = match_class;
	}
	if ((image = GetImage (icon_name, icon->iconc)) != None) {
	    icon->image  = image;
	    icon->width  = image->width;
	    icon->height = image->height;
	    tmp_win->forced = TRUE;
	}
    }

    /* if we still don't have an icon, assign the UnknownIcon */
    if (image == None && Scr->UnknownImage != None)
    {
	image = Scr->UnknownImage;
	icon->width   = image->width;
	icon->height  = image->height;
	icon->image   = image;
    }

    if (image == None)
    {
	icon->height = 0;
	icon->width  = 0;
	valuemask    = 0;
    }
    else
    {
	valuemask = CWBackPixmap;
	attributes.background_pixmap = image->pixmap;
    }

    icon->border_width = Scr->IconBorderWidth;
    if (Scr->NoIconTitlebar ||
	LookInNameList (Scr->NoIconTitle, tmp_win->icon_name) ||
	LookInList (Scr->NoIconTitle, tmp_win->full_name, &tmp_win->class))
    {
	icon->w_width  = icon->width;
	icon->w_height = icon->height;
	icon->x = 0;
	icon->y = 0;
	icon->has_title = False;
    }
    else {
	XRectangle inc_rect;
	XRectangle logical_rect;

	XmbTextExtents(Scr->IconFont.font_set,
		       tmp_win->icon_name, strlen (tmp_win->icon_name),
		       &inc_rect, &logical_rect);
	icon->w_width = logical_rect.width;

	icon->w_width += 2 * Scr->IconManagerShadowDepth + 6;
	if (icon->w_width > Scr->MaxIconTitleWidth) icon->w_width = Scr->MaxIconTitleWidth;
	if (icon->w_width < icon->width)
	{
	    icon->x  = (icon->width - icon->w_width) / 2;
	    icon->x += Scr->IconManagerShadowDepth + 3;
	    icon->w_width = icon->width;
	}
	else
	{
	    icon->x = Scr->IconManagerShadowDepth + 3;
	}
	icon->y = icon->height + Scr->IconFont.height + Scr->IconManagerShadowDepth;
	icon->w_height = icon->height + Scr->IconFont.height +
			 2 * Scr->IconManagerShadowDepth + 6;
	icon->has_title = True;
	if (icon->height) icon->border_width = 0;
    }

    event_mask = 0;
    if (tmp_win->wmhints && tmp_win->wmhints->flags & IconWindowHint)
    {
	icon->w = tmp_win->wmhints->icon_window;
	if (tmp_win->forced ||
	    XGetGeometry(dpy, icon->w, &JunkRoot, &JunkX, &JunkY,
		     (unsigned int *)&icon->w_width, (unsigned int *)&icon->w_height,
		     &JunkBW, &JunkDepth) == 0)
	{
	    icon->w = None;
	    tmp_win->wmhints->flags &= ~IconWindowHint;
	}
	else
	{
	    tmp_win->icon_not_ours = TRUE;
	    image = None;
	    icon->width  = icon->w_width;
	    icon->height = icon->w_height;
	    icon->image  = image;
	    icon->has_title = False;
	    event_mask = 0;
	}
    }
    else
    {
	icon->w = None;
    }

    if ((image != None) &&
	 image->mask != None &&
	 (! (tmp_win->wmhints && tmp_win->wmhints->flags & IconWindowHint))) {
	    icon->border_width = 0;
    }
    if (icon->w == None)
    {
	icon->w = XCreateSimpleWindow(dpy, Scr->Root,
	    0,0,
	    icon->w_width, icon->w_height,
	    icon->border_width, icon->border, icon->iconc.back);
	event_mask = ExposureMask;
    }

    if (Scr->AutoRaiseIcons || Scr->ShrinkIconTitles)
	event_mask |= EnterWindowMask | LeaveWindowMask;
    event_mask |= KeyPressMask | ButtonPressMask | ButtonReleaseMask;

    if (tmp_win->icon_not_ours) {
	XWindowAttributes wattr;

        XGetWindowAttributes(dpy, icon->w, &wattr);
        if (wattr.all_event_masks & ButtonPressMask) {
            event_mask &= ~ButtonPressMask;
	}
    }
    XSelectInput (dpy, icon->w, event_mask);

    if (icon->width == 0) icon->width = icon->w_width;
    icon->bm_w = None;
    if (image && (! (tmp_win->wmhints && tmp_win->wmhints->flags & IconWindowHint))) {
	XRectangle rect;

	x = GetIconOffset (icon);
	icon->bm_w = XCreateWindow (dpy, icon->w, x, 0,
					    (unsigned int)icon->width,
					    (unsigned int)icon->height,
					    (unsigned int) 0, Scr->d_depth,
					    (unsigned int) CopyFromParent,
					    Scr->d_visual, valuemask,
					    &attributes);
	if (image->mask) {
	    XShapeCombineMask (dpy, icon->bm_w, ShapeBounding, 0, 0, image->mask, ShapeSet);
	    XShapeCombineMask (dpy, icon->w,    ShapeBounding, x, 0, image->mask, ShapeSet);
	} else if (icon->has_title) {
	    rect.x      = x;
	    rect.y      = 0;
	    rect.width  = icon->width;
	    rect.height = icon->height;
	    XShapeCombineRectangles (dpy, icon->w, ShapeBounding,
			0, 0, &rect, 1, ShapeSet, 0);
	}
	if (icon->has_title) {
	    if (Scr->ShrinkIconTitles) {
		rect.x      = x;
		rect.y      = icon->height;
		rect.width  = icon->width;
		rect.height = icon->w_height - icon->height;
		icon->title_shrunk = True;
	    } else {
		rect.x      = 0;
		rect.y      = icon->height;
		rect.width  = icon->w_width;
		rect.height = icon->w_height - icon->height;
		icon->title_shrunk = False;
	    }
	    XShapeCombineRectangles (dpy, icon->w, ShapeBounding,
			0, 0, &rect, 1, ShapeUnion, 0);
	}
    }

    if (icon->match != match_none)
	AddToList (&tmp_win->iconslist, icon->pattern, (char*) icon);

    tmp_win->icon = icon;
    /* I need to figure out where to put the icon window now, because 
     * getting here means that I am going to make the icon visible
     */
    if (tmp_win->wmhints &&
	tmp_win->wmhints->flags & IconPositionHint)
    {
	final_x = tmp_win->wmhints->icon_x;
	final_y = tmp_win->wmhints->icon_y;
    }
    else
    {
      if (visible (tmp_win))
	PlaceIcon(tmp_win, def_x, def_y, &final_x, &final_y);
    }

  if (visible (tmp_win) ||
      (tmp_win->wmhints && tmp_win->wmhints->flags & IconPositionHint)) {
    if (final_x > Scr->rootw)
	final_x = Scr->rootw - icon->w_width - (2 * Scr->IconBorderWidth);
    if (Scr->ShrinkIconTitles && icon->bm_w) {
	if (final_x + (icon->w_width - icon->width) < 0) final_x = 0;
    } else {
	if (final_x < 0) final_x = 0;
    }
    if (final_y > Scr->rooth)
	final_y = Scr->rooth - icon->height -
	    Scr->IconFont.height - 6 - (2 * Scr->IconBorderWidth);
    if (final_y < 0) final_y = 0;

    XMoveWindow(dpy, icon->w, final_x, final_y);
  }
    tmp_win->iconified = TRUE;

    XMapSubwindows(dpy, icon->w);
    XSaveContext(dpy, icon->w, TwmContext, (XPointer)tmp_win);
    XSaveContext(dpy, icon->w, ScreenContext, (XPointer)Scr);
    XDefineCursor(dpy, icon->w, Scr->IconCursor);
    MaybeAnimate = True;
    return (0);
}

void DeleteIconsList(TwmWindow *tmp_win)
{
    /*
     * Only the list itself needs to be freed, since the pointers it
     * contains point into various lists that belong to Scr.
     */
    FreeList(&tmp_win->iconslist);
}

void ShrinkIconTitle (TwmWindow *tmp_win)
{
    Icon	*icon;
    XRectangle	rect;

    if (!tmp_win || !tmp_win->icon) return;
    icon = tmp_win->icon;
    if (!icon->has_title) return;
    if (icon->w_width == icon->width) return;
    if (icon->height  == 0) return;

    rect.x      = GetIconOffset (icon);
    rect.y      = 0;
    rect.width  = icon->width;
    rect.height = icon->w_height;
    XShapeCombineRectangles (dpy, icon->w, ShapeBounding, 0, 0, &rect, 1, ShapeIntersect, 0);
    icon->title_shrunk = True;
    XClearArea (dpy, icon->w, 0, icon->height, icon->w_width,
		icon->w_height - icon->height, True);
}

void ExpandIconTitle (TwmWindow *tmp_win)
{
    Icon	*icon;
    XRectangle	rect;

    if (!tmp_win || !tmp_win->icon) return;
    icon = tmp_win->icon;
    if (!icon->has_title) return;
    if (icon->w_width == icon->width) return;
    if (icon->height  == 0) return;

    rect.x      = 0;
    rect.y      = icon->height;
    rect.width  = icon->w_width;
    rect.height = icon->w_height - icon->height;
    XShapeCombineRectangles (dpy, icon->w, ShapeBounding, 0, 0, &rect, 1, ShapeUnion, 0);
    icon->title_shrunk = False;
    XClearArea (dpy, icon->w, 0, icon->height, icon->w_width,
		icon->w_height - icon->height, True);
}

void ReshapeIcon (Icon *icon)
{
    int x;
    XRectangle	rect;

    if (!icon) return;
    x = GetIconOffset (icon);
    XMoveWindow (dpy, icon->bm_w, x, 0);

    if (icon->image && icon->image->mask) {
	XShapeCombineMask (dpy, icon->w, ShapeBounding, x, 0, icon->image->mask, ShapeSet);
    } else {
	rect.x      = x;
	rect.y      = 0;
	rect.width  = icon->width;
	rect.height = icon->height;
	XShapeCombineRectangles (dpy, icon->w, ShapeBounding, 0, 0, &rect, 1, ShapeSet, 0);
    }
    rect.x      = x;
    rect.y      = icon->height;
    rect.width  = icon->width;
    rect.height = icon->w_height - icon->height;
    XShapeCombineRectangles (dpy, icon->w, ShapeBounding, 0, 0, &rect, 1, ShapeUnion, 0);
}

int GetIconOffset (Icon *icon)
{
    short justif;

    if (!icon) return (0);

    justif = icon->ir ? icon->ir->TitleJustification : Scr->IconJustification;
    switch (justif) {
	case J_LEFT :
	    return (0);

	case J_CENTER :
	    return ((icon->w_width - icon->width) / 2);

	case J_RIGHT :
	    return (icon->w_width - icon->width);

	default :
	    return (0);
    }
}

Bool AnimateIcons (ScreenInfo *scr, Icon *icon)
{
    Image	*image;
    XRectangle	rect;
    XSetWindowAttributes attr;
    int		x;

    image = icon->image;
    attr.background_pixmap = image->pixmap;
    XChangeWindowAttributes (dpy, icon->bm_w, CWBackPixmap, &attr);

    if (image->mask != None) {
	x = GetIconOffset (icon);
	XShapeCombineMask (dpy, icon->bm_w, ShapeBounding, 0, 0, image->mask, ShapeSet);
	if (icon->has_title) {
	    rect.x      = 0;
	    rect.y      = icon->height;
	    rect.width  = icon->w_width;
	    rect.height = scr->IconFont.height + 6;

	    XShapeCombineShape (dpy, scr->ShapeWindow, ShapeBounding, x, 0, icon->bm_w,
				ShapeBounding, ShapeSet);
	    XShapeCombineRectangles (dpy, scr->ShapeWindow, ShapeBounding, 0, 0, &rect, 1,
				ShapeUnion, 0);
	    XShapeCombineShape (dpy, icon->w, ShapeBounding, 0, 0, scr->ShapeWindow,
				ShapeBounding, ShapeSet);
	}
	else
	    XShapeCombineShape (dpy, icon->w, ShapeBounding, x, 0, icon->bm_w,
				ShapeBounding, ShapeSet);
    }
    XClearWindow (dpy, icon->bm_w);
    icon->image  = image->next;
    return (True);
}

