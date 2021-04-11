/*
 * Copyright 1989 Massachusetts Institute of Technology
 * Copyright 1992 Claude Lecommandeur.
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

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/extensions/shape.h>

#include "drawing.h"
#include "screen.h"
#include "iconmgr.h"
#include "icons.h"
#include "otp.h"
#include "list.h"
#include "parse.h"
#include "util.h"
#include "animate.h"
#include "image.h"
#include "win_utils.h"
#include "workspace_manager.h"

static void splitIconRegionEntry(IconEntry *ie, RegGravity grav1,
                                 RegGravity grav2, int w, int h);
static void PlaceIcon(TwmWindow *tmp_win, int def_x, int def_y,
                      int *final_x, int *final_y);
static IconEntry *FindIconEntry(TwmWindow *tmp_win, IconRegion **irp);
static IconEntry *prevIconEntry(IconEntry *ie, IconRegion *ir);
static void mergeEntries(IconEntry *old, IconEntry *ie);
static void ReshapeIcon(Icon *icon);
static int roundUp(int v, int multiple);
static Image *LookupIconNameOrClass(TwmWindow *tmp_win, Icon *icon,
                                    char **pattern);



/*
 ****************************************************************
 *
 * First some bits related to figuring out where icons go.  Lots of
 * IconRegion handling stuff, handling of IconEntry tracking, etc.
 *
 ****************************************************************
 */


/*
 * This function operates in very weird and obtuse ways, especially in
 * how it handles vertical vs. horizontal in weird recursive calls.  Part
 * of this is what previously allowed specs with "hgrav vgrav" instead of
 * the proper "vgrav hgrav" to sorta-work.  This should be broken up at
 * some point into clean h/v functions, but because of the recursion it's
 * not exactly trivial.  The parsing code now enforces v/h, so at least
 * things can be known to come in in the right order initially.  Revisit
 * someday.
 */
static void
splitIconRegionEntry(IconEntry *ie, RegGravity grav1, RegGravity grav2,
                     int w, int h)
{
	switch(grav1) {
		case GRAV_NORTH:
		case GRAV_SOUTH:
			if(w != ie->w) {
				splitIconRegionEntry(ie, grav2, grav1, w, ie->h);
			}
			if(h != ie->h) {
				IconEntry *new = calloc(1, sizeof(IconEntry));
				new->next = ie->next;
				ie->next = new;
				new->x = ie->x;
				new->h = (ie->h - h);
				new->w = ie->w;
				ie->h = h;
				if(grav1 == GRAV_SOUTH) {
					new->y = ie->y;
					ie->y = new->y + new->h;
				}
				else {
					new->y = ie->y + ie->h;
				}
			}
			break;
		case GRAV_EAST:
		case GRAV_WEST:
			if(h != ie->h) {
				splitIconRegionEntry(ie, grav2, grav1, ie->w, h);
			}
			if(w != ie->w) {
				IconEntry *new = calloc(1, sizeof(IconEntry));
				new->next = ie->next;
				ie->next = new;
				new->y = ie->y;
				new->w = (ie->w - w);
				new->h = ie->h;
				ie->w = w;
				if(grav1 == GRAV_EAST) {
					new->x = ie->x;
					ie->x = new->x + new->w;
				}
				else {
					new->x = ie->x + ie->w;
				}
			}
			break;
	}
}


/*
 * Backend for parsing IconRegion config
 */
name_list **
AddIconRegion(const char *geom, RegGravity grav1, RegGravity grav2,
              int stepx, int stepy,
              const char *ijust, const char *just, const char *align)
{
	IconRegion *ir;
	int mask, tmp;

	ir = malloc(sizeof(IconRegion));
	ir->next = NULL;

	if(Scr->LastRegion) {
		Scr->LastRegion->next = ir;
	}
	Scr->LastRegion = ir;
	if(!Scr->FirstRegion) {
		Scr->FirstRegion = ir;
	}

	ir->entries = NULL;
	ir->clientlist = NULL;
	ir->grav1 = grav1;
	ir->grav2 = grav2;
	if(stepx <= 0) {
		stepx = 1;
	}
	if(stepy <= 0) {
		stepy = 1;
	}
	ir->stepx = stepx;
	ir->stepy = stepy;
	ir->x = ir->y = ir->w = ir->h = 0;

	mask = XParseGeometry(geom, &ir->x, &ir->y, (unsigned int *)&ir->w,
	                      (unsigned int *)&ir->h);

	if(mask & XNegative) {
		ir->x += Scr->rootw - ir->w;
	}
	if(mask & YNegative) {
		ir->y += Scr->rooth - ir->h;
	}

	ir->entries = calloc(1, sizeof(IconEntry));
	ir->entries->x = ir->x;
	ir->entries->y = ir->y;
	ir->entries->w = ir->w;
	ir->entries->h = ir->h;

	if((tmp = ParseTitleJustification(ijust)) < 0) {
		twmrc_error_prefix();
		fprintf(stderr, "ignoring invalid IconRegion argument \"%s\"\n", ijust);
		tmp = TJ_UNDEF;
	}
	ir->TitleJustification = tmp;

	if((tmp = ParseIRJustification(just)) < 0) {
		twmrc_error_prefix();
		fprintf(stderr, "ignoring invalid IconRegion argument \"%s\"\n", just);
		tmp = IRJ_UNDEF;
	}
	ir->Justification = tmp;

	if((tmp = ParseAlignement(align)) < 0) {
		twmrc_error_prefix();
		fprintf(stderr, "ignoring invalid IconRegion argument \"%s\"\n", align);
		tmp = IRA_UNDEF;
	}
	ir->Alignement = tmp;

	return(&(ir->clientlist));
}


/*
 * Figure out where to put a window's icon based on the IconRegion
 * specifications given in config.  Passed def_[xy] which are used
 * if we don't find a better location ourselves.  Returns the chosen
 * location in final_[xy], and also sets the IconRegion in tmp_win->icon
 * if we chose one.
 */
static void
PlaceIcon(TwmWindow *tmp_win, int def_x, int def_y,
          int *final_x, int *final_y)
{
	IconRegion  *ir, *oldir;
	IconEntry   *ie;
	int         w, h;

	const int iconWidth = tmp_win->icon->border_width * 2
	                      + (Scr->ShrinkIconTitles ? tmp_win->icon->width
	                         : tmp_win->icon->w_width);
	const int iconHeight = tmp_win->icon->border_width * 2
	                       + tmp_win->icon->w_height;

	/*
	 * First, check to see if the window is in a region's client list
	 * (i.e., the win-list on an IconRegion specifier in the config).
	 */
	ie = NULL;
	for(ir = Scr->FirstRegion; ir; ir = ir->next) {
		if(LookInList(ir->clientlist, tmp_win->name, &tmp_win->class)) {
			/*
			 * Found one that claims it.  Figure the necessary local
			 * size, based on the icon's side itself and the grid for
			 * this IR.
			 */
			w = roundUp(iconWidth, ir->stepx);
			h = roundUp(iconHeight, ir->stepy);

			/* Find a currently-unused region that's big enough */
			for(ie = ir->entries; ie; ie = ie->next) {
				if(ie->used) {
					continue;
				}
				if(ie->w >= w && ie->h >= h) {
					/* Bingo */
					break;
				}
			}

			/* If we found one, we're done here */
			if(ie) {
				break;
			}
		}
	}


	/*
	 * If we found a slot in a region claiming it, ie is set to the
	 * IconEntry.  If not, start over and find the first available berth.
	 */
	if(!ie) {
		for(ir = Scr->FirstRegion; ir; ir = ir->next) {
			w = roundUp(iconWidth, ir->stepx);
			h = roundUp(iconHeight, ir->stepy);
			for(ie = ir->entries; ie; ie = ie->next) {
				if(ie->used) {
					continue;
				}
				if(ie->w >= w && ie->h >= h) {
					/* Bingo */
					break;
				}
			}
			if(ie) {
				break;
			}
		}
	}

	/* Stash for comparison */
	oldir = tmp_win->icon->ir;

	/*
	 * If we found an appropriate region, use it.  Else, we have no
	 * better idea, so use the x/y coords the caller passed us as our
	 * basis.
	 */
	if(ie) {
		/* XXX whatever sIRE() does */
		splitIconRegionEntry(ie, ir->grav1, ir->grav2, w, h);

		/* Adjust horizontal positioning based on IconRegionJustification */
		switch(ir->Justification) {
			case IRJ_LEFT:
				*final_x = ie->x;
				break;
			case IRJ_UNDEF:
			case IRJ_CENTER:
				*final_x = ie->x + (ie->w - iconWidth) / 2;
				break;
			case IRJ_RIGHT:
				*final_x = ie->x + ie->w - iconWidth;
				break;
			case IRJ_BORDER:
				if(ir->grav2 == GRAV_EAST) {
					*final_x = ie->x + ie->w - iconWidth;
				}
				else {
					*final_x = ie->x;
				}
				break;
		}

		/* And vertical based on IconRegionAlignement */
		switch(ir->Alignement) {
			case IRA_TOP :
				*final_y = ie->y;
				break;
			case IRA_UNDEF :
			case IRA_CENTER :
				*final_y = ie->y + (ie->h - iconHeight) / 2;
				break;
			case IRA_BOTTOM :
				*final_y = ie->y + ie->h - iconHeight;
				break;
			case IRA_BORDER :
				if(ir->grav1 == GRAV_SOUTH) {
					*final_y = ie->y + ie->h - iconHeight;
				}
				else {
					*final_y = ie->y;
				}
				break;
		}

		/* Tell the win/icon what region it's in, and the entry what's in it */
		tmp_win->icon->ir = ir;
		ie->used = true;
		ie->twm_win = tmp_win;
	}
	else {
		/* No better idea, tell caller to use theirs */
		*final_x = def_x;
		*final_y = def_y;
		tmp_win->icon->ir = NULL;
		return;
		/* XXX Should we be doing the below in this case too? */
	}

	/* Alterations if ShrinkIconTitles is set */
	if(Scr->ShrinkIconTitles && tmp_win->icon->has_title) {
		*final_x -= GetIconOffset(tmp_win->icon);
		if(tmp_win->icon->ir != oldir) {
			ReshapeIcon(tmp_win->icon);
		}
	}

	return;
}


/*
 * Look up an IconEntry holding the icon for a given window, and
 * optionally stash its IconRegion in irp.  Used internally in
 * IconDown().
 */
static IconEntry *
FindIconEntry(TwmWindow *tmp_win, IconRegion **irp)
{
	IconRegion  *ir;
	IconEntry   *ie;

	for(ir = Scr->FirstRegion; ir; ir = ir->next) {
		for(ie = ir->entries; ie; ie = ie->next)
			if(ie->twm_win == tmp_win) {
				if(irp) {
					*irp = ir;
				}
				return ie;
			}
	}
	return NULL;
}


/*
 * Find prior IE in list.  Used internally in IconDown().
 */
static IconEntry *
prevIconEntry(IconEntry *ie, IconRegion *ir)
{
	IconEntry   *ip;

	if(ie == ir->entries) {
		return NULL;
	}
	for(ip = ir->entries; ip->next != ie; ip = ip->next)
		;
	return ip;
}


/*
 * Merge two adjacent IconEntry's.  old is being freed; and is adjacent
 * to ie.  Merge regions together.
 */
static void
mergeEntries(IconEntry *old, IconEntry *ie)
{
	if(old->y == ie->y) {
		ie->w = old->w + ie->w;
		if(old->x < ie->x) {
			ie->x = old->x;
		}
	}
	else {
		ie->h = old->h + ie->h;
		if(old->y < ie->y) {
			ie->y = old->y;
		}
	}
}




/*
 ****************************************************************
 *
 * Next, the bits related to creating and putting together the icon
 * windows, as well as destroying them.
 *
 ****************************************************************
 */


/*
 * Create the window scaffolding for an icon.  Called when we need to
 * make one, e.g. the first time a window is iconified.
 */
void
CreateIconWindow(TwmWindow *tmp_win, int def_x, int def_y)
{
	unsigned long event_mask;
	unsigned long valuemask;            /* mask for create windows */
	XSetWindowAttributes attributes;    /* attributes for create windows */
	int final_x, final_y;
	int x;
	Icon        *icon;
	Image       *image = NULL;
	char        *pattern;

	icon = malloc(sizeof(struct Icon));

	icon->otp           = NULL;
	icon->border        = Scr->IconBorderColor;
	icon->iconc.fore    = Scr->IconC.fore;
	icon->iconc.back    = Scr->IconC.back;
	icon->title_shrunk  = false;

	GetColorFromList(Scr->IconBorderColorL, tmp_win->name, &tmp_win->class,
	                 &icon->border);
	GetColorFromList(Scr->IconForegroundL, tmp_win->name, &tmp_win->class,
	                 &icon->iconc.fore);
	GetColorFromList(Scr->IconBackgroundL, tmp_win->name, &tmp_win->class,
	                 &icon->iconc.back);
	if(Scr->use3Diconmanagers && !Scr->BeNiceToColormap) {
		GetShadeColors(&icon->iconc);
	}

	FB(icon->iconc.fore, icon->iconc.back);

	icon->match   = match_none;
	icon->image   = NULL;
	icon->ir      = NULL;

	tmp_win->forced = false;
	icon->w_not_ours = false;

	pattern = NULL;

	/* now go through the steps to get an icon window,  if ForceIcon is
	 * set, then no matter what else is defined, the bitmap from the
	 * .twmrc file is used
	 */
	if(Scr->ForceIcon) {
		image = LookupIconNameOrClass(tmp_win, icon, &pattern);
	}

#ifdef EWMH
	/*
	 * Look to see if there is a _NET_WM_ICON property to provide an icon.
	 */
	if(image == NULL) {
		image = EwmhGetIcon(Scr, tmp_win);
		if(image != NULL) {
			icon->match   = match_net_wm_icon;
			icon->width   = image->width;
			icon->height  = image->height;
			icon->image   = image;
		}
	}
#endif /* EWMH */

	/* if the pixmap is still NULL, we didn't get one from the above code,
	 * that could mean that ForceIcon was not set, or that the window
	 * was not in the Icons list, now check the WM hints for an icon
	 */
	if(image == NULL && tmp_win->wmhints->flags & IconPixmapHint) {
		unsigned int IconDepth, IconWidth, IconHeight;

		if(XGetGeometry(dpy, tmp_win->wmhints->icon_pixmap,
		                &JunkRoot, &JunkX, &JunkY, &IconWidth, &IconHeight, &JunkBW, &IconDepth)) {
			image = AllocImage();
			image->width  = IconWidth;
			image->height = IconHeight;
			image->pixmap = XCreatePixmap(dpy, Scr->Root, image->width,
			                              image->height, Scr->d_depth);
			if(IconDepth == Scr->d_depth)
				XCopyArea(dpy, tmp_win->wmhints->icon_pixmap, image->pixmap, Scr->NormalGC,
				          0, 0, image->width, image->height, 0, 0);
			else
				XCopyPlane(dpy, tmp_win->wmhints->icon_pixmap, image->pixmap, Scr->NormalGC,
				           0, 0, image->width, image->height, 0, 0, 1);

			icon->width   = image->width;
			icon->height  = image->height;
			icon->match   = match_icon_pixmap_hint;

			if((tmp_win->wmhints->flags & IconMaskHint) &&
			                XGetGeometry(dpy, tmp_win->wmhints->icon_mask,
			                             &JunkRoot, &JunkX, &JunkY, &IconWidth, &IconHeight, &JunkBW, &IconDepth) &&
			                (IconDepth == 1)) {
				GC gc;

				image->mask = XCreatePixmap(dpy, Scr->Root, IconWidth, IconHeight, 1);
				if(image->mask) {
					gc = XCreateGC(dpy, image->mask, 0, NULL);
					if(gc) {
						XCopyArea(dpy, tmp_win->wmhints->icon_mask, image->mask, gc,
						          0, 0, IconWidth, IconHeight, 0, 0);
						XFreeGC(dpy, gc);
					}
				}
			}
			icon->image = image;
		}
	}

	/* if we still haven't got an icon, let's look in the Icon list
	 * if ForceIcon is not set
	 */
	if(image == NULL && !Scr->ForceIcon) {
		image = LookupIconNameOrClass(tmp_win, icon, &pattern);
	}

	/* if we still don't have an icon, assign the UnknownIcon */
	if(image == NULL && Scr->UnknownImage != NULL) {
		image = Scr->UnknownImage;
		icon->match   = match_unknown_default;
		icon->width   = image->width;
		icon->height  = image->height;
		icon->image   = image;
	}

	if(image == NULL) {
		icon->height = 0;
		icon->width  = 0;
		valuemask    = 0;
	}
	else {
		valuemask = CWBackPixmap;
		attributes.background_pixmap = image->pixmap;
	}

	icon->border_width = Scr->IconBorderWidth;
	if(Scr->NoIconTitlebar ||
	                LookInNameList(Scr->NoIconTitle, tmp_win->icon_name) ||
	                LookInList(Scr->NoIconTitle, tmp_win->name, &tmp_win->class)) {
		icon->w_width  = icon->width;
		icon->w_height = icon->height;
		icon->x = 0;
		icon->y = 0;
		icon->has_title = false;
	}
	else {
		XRectangle inc_rect;
		XRectangle logical_rect;

		XmbTextExtents(Scr->IconFont.font_set,
		               tmp_win->icon_name, strlen(tmp_win->icon_name),
		               &inc_rect, &logical_rect);
		icon->w_width = logical_rect.width;

		icon->w_width += 2 * (Scr->IconManagerShadowDepth + ICON_MGR_IBORDER);
		if(icon->w_width > Scr->MaxIconTitleWidth) {
			icon->w_width = Scr->MaxIconTitleWidth;
		}
		if(icon->w_width < icon->width) {
			icon->x  = (icon->width - icon->w_width) / 2;
			icon->x += Scr->IconManagerShadowDepth + ICON_MGR_IBORDER;
			icon->w_width = icon->width;
		}
		else {
			icon->x = Scr->IconManagerShadowDepth + ICON_MGR_IBORDER;
		}
		icon->y = icon->height + Scr->IconFont.height + Scr->IconManagerShadowDepth;
		icon->w_height = icon->height + Scr->IconFont.height +
		                 2 * (Scr->IconManagerShadowDepth + ICON_MGR_IBORDER);
		icon->has_title = true;
		if(icon->height) {
			icon->border_width = 0;
		}
	}

	event_mask = 0;
	if(tmp_win->wmhints->flags & IconWindowHint) {
		icon->w = tmp_win->wmhints->icon_window;
		if(tmp_win->forced ||
		                XGetGeometry(dpy, icon->w, &JunkRoot, &JunkX, &JunkY,
		                             (unsigned int *)&icon->w_width, (unsigned int *)&icon->w_height,
		                             &JunkBW, &JunkDepth) == 0) {
			icon->w = None;
			tmp_win->wmhints->flags &= ~IconWindowHint;
		}
		else {
			image = NULL;
			icon->w_not_ours = true;
			icon->width  = icon->w_width;
			icon->height = icon->w_height;
			icon->image  = image;
			icon->has_title = false;
			event_mask = 0;
		}
	}
	else {
		icon->w = None;
	}

	if((image != NULL) &&
	                image->mask != None &&
	                !(tmp_win->wmhints->flags & IconWindowHint)) {
		icon->border_width = 0;
	}
	if(icon->w == None) {
		icon->w = XCreateSimpleWindow(dpy, Scr->Root,
		                              0, 0,
		                              icon->w_width, icon->w_height,
		                              icon->border_width, icon->border, icon->iconc.back);
		event_mask = ExposureMask;
	}

	if(Scr->AutoRaiseIcons || Scr->ShrinkIconTitles) {
		event_mask |= EnterWindowMask | LeaveWindowMask;
	}
	event_mask |= KeyPressMask | ButtonPressMask | ButtonReleaseMask;

	if(icon->w_not_ours) {
		XWindowAttributes wattr;

		XGetWindowAttributes(dpy, icon->w, &wattr);
		if(wattr.all_event_masks & ButtonPressMask) {
			event_mask &= ~ButtonPressMask;
		}
	}
	XSelectInput(dpy, icon->w, event_mask);

	if(icon->width == 0) {
		icon->width = icon->w_width;
	}
	icon->bm_w = None;
	if(image && !(tmp_win->wmhints->flags & IconWindowHint)) {
		XRectangle rect;

		x = GetIconOffset(icon);
		icon->bm_w = XCreateWindow(dpy, icon->w, x, 0,
		                           icon->width,
		                           icon->height,
		                           0, Scr->d_depth,
		                           CopyFromParent,
		                           Scr->d_visual, valuemask,
		                           &attributes);
		if(image->mask) {
			XShapeCombineMask(dpy, icon->bm_w, ShapeBounding, 0, 0, image->mask, ShapeSet);
			XShapeCombineMask(dpy, icon->w,    ShapeBounding, x, 0, image->mask, ShapeSet);
		}
		else if(icon->has_title) {
			rect.x      = x;
			rect.y      = 0;
			rect.width  = icon->width;
			rect.height = icon->height;
			XShapeCombineRectangles(dpy, icon->w, ShapeBounding,
			                        0, 0, &rect, 1, ShapeSet, 0);
		}
		if(icon->has_title) {
			if(Scr->ShrinkIconTitles) {
				rect.x      = x;
				rect.y      = icon->height;
				rect.width  = icon->width;
				rect.height = icon->w_height - icon->height;
				icon->title_shrunk = true;
			}
			else {
				rect.x      = 0;
				rect.y      = icon->height;
				rect.width  = icon->w_width;
				rect.height = icon->w_height - icon->height;
				icon->title_shrunk = false;
			}
			XShapeCombineRectangles(dpy, icon->w, ShapeBounding,
			                        0, 0, &rect, 1, ShapeUnion, 0);
		}
	}

	if(pattern != NULL) {
		AddToList(&tmp_win->iconslist, pattern, icon);
	}

	tmp_win->icon = icon;
	/* I need to figure out where to put the icon window now, because
	 * getting here means that I am going to make the icon visible
	 */
	final_x = final_y = 0;
	if(tmp_win->wmhints->flags & IconPositionHint) {
		final_x = tmp_win->wmhints->icon_x;
		final_y = tmp_win->wmhints->icon_y;
	}
	else {
		if(visible(tmp_win)) {
			PlaceIcon(tmp_win, def_x, def_y, &final_x, &final_y);
		}
	}

	if(visible(tmp_win) || (tmp_win->wmhints->flags & IconPositionHint)) {
		if(final_x > Scr->rootw) {
			final_x = Scr->rootw - icon->w_width - (2 * Scr->IconBorderWidth);
		}
		if(Scr->ShrinkIconTitles && icon->bm_w) {
			if(final_x + (icon->w_width - icon->width) < 0) {
				final_x = 0;
			}
		}
		else {
			if(final_x < 0) {
				final_x = 0;
			}
		}
		if(final_y > Scr->rooth)
			final_y = Scr->rooth - icon->height -
			          Scr->IconFont.height - 6 - (2 * Scr->IconBorderWidth);
		if(final_y < 0) {
			final_y = 0;
		}

		XMoveWindow(dpy, icon->w, final_x, final_y);
		icon->w_x = final_x;
		icon->w_y = final_y;
	}
	tmp_win->iconified = true;
	OtpAdd(tmp_win, IconWin);

	XMapSubwindows(dpy, icon->w);
	XSaveContext(dpy, icon->w, TwmContext, (XPointer)tmp_win);
	XSaveContext(dpy, icon->w, ScreenContext, (XPointer)Scr);
	XDefineCursor(dpy, icon->w, Scr->IconCursor);
	MaybeAnimate = true;
}


/*
 * Delete TwmWindow.iconslist.
 * Call it before deleting TwmWindow.icon, since we need to check
 * that we're not deleting that Icon.
 */
void
DeleteIconsList(TwmWindow *tmp_win)
{
	/*
	 * Only the list itself needs to be freed, since the pointers it
	 * contains point into various lists that belong to Scr.
	 *
	 * Rhialto: Hmmmm not quite sure about that! CreateIconWindow() above
	 * always allocates a struct Icon, and doesn't attach it to Scr...
	 * It is probably correct for the Image pointers inside those Icons though.
	 */
	name_list *nptr;
	name_list *next;

	for(nptr = tmp_win->iconslist; nptr != NULL;) {
		next = nptr->next;
		Icon *icon = (Icon *)nptr->ptr;
		if(icon != tmp_win->icon) {
			DeleteIcon(icon);
		}
		free(nptr->name);
		free(nptr);
		nptr = next;
	}
	tmp_win->iconslist = NULL;
}


/*
 * Delete a single Icon.  Called iteratively from DeleteIconList(), and
 * directly during window destruction.
 */
void
DeleteIcon(Icon *icon)
{
	if(icon->w && !icon->w_not_ours) {
		XDestroyWindow(dpy, icon->w);
	}
	ReleaseIconImage(icon);
	free(icon);
}


/*
 * Delete the Image from an icon, if it is not a shared one.  match_list
 * ands match_unknown_default need not be freed.
 *
 * Formerly ReleaseImage()
 */
void
ReleaseIconImage(Icon *icon)
{
	if(icon->match == match_icon_pixmap_hint ||
	                icon->match == match_net_wm_icon) {
		FreeImage(icon->image);
	}
}




/*
 ****************************************************************
 *
 * Bringing an icon up or down.
 *
 ****************************************************************
 */


/*
 * Show up an icon.  Note that neither IconUp nor IconDown actually map
 * or unmap the icon window; that's handled by the callers.  These
 * functions limit themselves to figuring out where it should be, moving
 * it (still unmapped) there, and linking/unlinking it from the iconentry
 * lists.
 */
void
IconUp(TwmWindow *tmp_win)
{
	int         x, y;
	int         defx, defy;

	/*
	 * If the client specified a particular location, let's use it (this might
	 * want to be an option at some point).  Otherwise, try to fit within the
	 * icon region.
	 */
	if(tmp_win->wmhints->flags & IconPositionHint) {
		return;
	}

	if(tmp_win->icon_moved) {
		struct IconRegion *ir;
		unsigned int iww, iwh;

		if(!XGetGeometry(dpy, tmp_win->icon->w, &JunkRoot, &defx, &defy,
		                 &iww, &iwh, &JunkBW, &JunkDepth)) {
			return;
		}

		x = defx + ((int) iww) / 2;
		y = defy + ((int) iwh) / 2;

		for(ir = Scr->FirstRegion; ir; ir = ir->next) {
			if(x >= ir->x && x < (ir->x + ir->w) &&
			                y >= ir->y && y < (ir->y + ir->h)) {
				break;
			}
		}
		if(!ir) {
			return;        /* outside icon regions, leave alone */
		}
	}

	defx = -100;
	defy = -100;
	PlaceIcon(tmp_win, defx, defy, &x, &y);
	if(x != defx || y != defy) {
		XMoveWindow(dpy, tmp_win->icon->w, x, y);
		tmp_win->icon->w_x = x;
		tmp_win->icon->w_y = y;
		tmp_win->icon_moved = false;    /* since we've restored it */
	}
	MaybeAnimate = true;
	return;
}


/*
 * Remove an icon from its displayed IconEntry.  x-ref comment on
 * IconUp().
 */
void
IconDown(TwmWindow *tmp_win)
{
	IconEntry   *ie, *ip, *in;
	IconRegion  *ir;

	ie = FindIconEntry(tmp_win, &ir);
	if(ie) {
		ie->twm_win = NULL;
		ie->used = false;
		ip = prevIconEntry(ie, ir);
		in = ie->next;
		for(;;) {
			if(ip && ip->used == false &&
			                ((ip->x == ie->x && ip->w == ie->w) ||
			                 (ip->y == ie->y && ip->h == ie->h))) {
				ip->next = ie->next;
				mergeEntries(ie, ip);
				free(ie);
				ie = ip;
				ip = prevIconEntry(ip, ir);
			}
			else if(in && in->used == false &&
			                ((in->x == ie->x && in->w == ie->w) ||
			                 (in->y == ie->y && in->h == ie->h))) {
				ie->next = in->next;
				mergeEntries(in, ie);
				free(in);
				in = ie->next;
			}
			else {
				break;
			}
		}
	}
}




/*
 ****************************************************************
 *
 * Funcs related to drawing the icon.
 *
 ****************************************************************
 */


/*
 * Slightly misnamed: draws the text label under an icon.
 */
void
PaintIcon(TwmWindow *tmp_win)
{
	int         width, twidth, mwidth, len, x;
	Icon        *icon;
	XRectangle ink_rect;
	XRectangle logical_rect;

	if(!tmp_win || !tmp_win->icon) {
		return;
	}
	icon = tmp_win->icon;
	if(!icon->has_title) {
		return;
	}

	x     = 0;
	width = icon->w_width;
	if(Scr->ShrinkIconTitles && icon->title_shrunk) {
		x     = GetIconOffset(icon);
		width = icon->width;
	}
	len    = strlen(tmp_win->icon_name);
	XmbTextExtents(Scr->IconFont.font_set,
	               tmp_win->icon_name, len,
	               &ink_rect, &logical_rect);
	twidth = logical_rect.width;
	mwidth = width - 2 * (Scr->IconManagerShadowDepth + ICON_MGR_IBORDER);
	if(Scr->use3Diconmanagers) {
		Draw3DBorder(icon->w, x, icon->height, width,
		             Scr->IconFont.height +
		             2 * (Scr->IconManagerShadowDepth + ICON_MGR_IBORDER),
		             Scr->IconManagerShadowDepth, icon->iconc, off, false, false);
	}
	while((len > 0) && (twidth > mwidth)) {
		len--;
		XmbTextExtents(Scr->IconFont.font_set,
		               tmp_win->icon_name, len,
		               &ink_rect, &logical_rect);
		twidth = logical_rect.width;
	}
	FB(icon->iconc.fore, icon->iconc.back);
	XmbDrawString(dpy, icon->w, Scr->IconFont.font_set, Scr->NormalGC,
	              x + ((mwidth - twidth) / 2) +
	              Scr->IconManagerShadowDepth + ICON_MGR_IBORDER,
	              icon->y, tmp_win->icon_name, len);
}


/*
 * Handling for ShrinkIconTitles; when pointer is away from them, shrink
 * the titles down to the width of the image, and expand back out when it
 * enters.
 */
void
ShrinkIconTitle(TwmWindow *tmp_win)
{
	Icon        *icon;
	XRectangle  rect;

	if(!tmp_win || !tmp_win->icon) {
		return;
	}
	icon = tmp_win->icon;
	if(!icon->has_title) {
		return;
	}
	if(icon->w_width == icon->width) {
		return;
	}
	if(icon->height  == 0) {
		return;
	}

	rect.x      = GetIconOffset(icon);
	rect.y      = 0;
	rect.width  = icon->width;
	rect.height = icon->w_height;
	XShapeCombineRectangles(dpy, icon->w, ShapeBounding, 0, 0, &rect, 1,
	                        ShapeIntersect, 0);
	icon->title_shrunk = true;
	XClearArea(dpy, icon->w, 0, icon->height, icon->w_width,
	           icon->w_height - icon->height, True);
}


void
ExpandIconTitle(TwmWindow *tmp_win)
{
	Icon        *icon;
	XRectangle  rect;

	if(!tmp_win || !tmp_win->icon) {
		return;
	}
	icon = tmp_win->icon;
	if(!icon->has_title) {
		return;
	}
	if(icon->w_width == icon->width) {
		return;
	}
	if(icon->height  == 0) {
		return;
	}

	rect.x      = 0;
	rect.y      = icon->height;
	rect.width  = icon->w_width;
	rect.height = icon->w_height - icon->height;
	XShapeCombineRectangles(dpy, icon->w, ShapeBounding, 0, 0, &rect, 1, ShapeUnion,
	                        0);
	icon->title_shrunk = false;
	XClearArea(dpy, icon->w, 0, icon->height, icon->w_width,
	           icon->w_height - icon->height, True);
}


/*
 * Setup X Shape'ing around icons and their titles.
 *
 * XXX should this be checking HasShape?  It seems to be called
 * unconditionally...
 */
static void
ReshapeIcon(Icon *icon)
{
	int x;
	XRectangle  rect;

	if(!icon) {
		return;
	}
	x = GetIconOffset(icon);
	XMoveWindow(dpy, icon->bm_w, x, 0);

	if(icon->image && icon->image->mask) {
		XShapeCombineMask(dpy, icon->w, ShapeBounding, x, 0, icon->image->mask,
		                  ShapeSet);
	}
	else {
		rect.x      = x;
		rect.y      = 0;
		rect.width  = icon->width;
		rect.height = icon->height;
		XShapeCombineRectangles(dpy, icon->w, ShapeBounding, 0, 0, &rect, 1, ShapeSet,
		                        0);
	}
	rect.x      = x;
	rect.y      = icon->height;
	rect.width  = icon->width;
	rect.height = icon->w_height - icon->height;
	XShapeCombineRectangles(dpy, icon->w, ShapeBounding, 0, 0, &rect, 1, ShapeUnion,
	                        0);
}


/*
 * Figure horizontal positioning/offset for the icon image within its
 * window.
 */
int
GetIconOffset(Icon *icon)
{
	TitleJust justif;

	if(!icon) {
		return 0;
	}

	justif = icon->ir ? icon->ir->TitleJustification : Scr->IconJustification;
	switch(justif) {
		case TJ_LEFT:
			return 0;

		case TJ_CENTER:
			return ((icon->w_width - icon->width) / 2);

		case TJ_RIGHT:
			return (icon->w_width - icon->width);

		default:
			/* Can't happen? */
			fprintf(stderr, "%s(): Invalid TitleJustification %d\n",
			        __func__, justif);
			return 0;
	}
}


/*
 * [Re-]lookup the image for an icon and [re-]layout it.
 */
void
RedoIcon(TwmWindow *win)
{
	Icon *icon, *old_icon;
	char *pattern;

	old_icon = win->icon;

	if(old_icon && (old_icon->w_not_ours || old_icon->match != match_list)) {
		RedoIconName(win);
		return;
	}
	icon = NULL;
	if((pattern = LookPatternInNameList(Scr->IconNames, win->icon_name))) {
		icon = LookInNameList(win->iconslist, pattern);
	}
	else if((pattern = LookPatternInNameList(Scr->IconNames, win->name))) {
		icon = LookInNameList(win->iconslist, pattern);
	}
	else if((pattern = LookPatternInList(Scr->IconNames, win->name,
	                                     &win->class))) {
		icon = LookInNameList(win->iconslist, pattern);
	}
	if(pattern == NULL) {
		RedoIconName(win);
		return;
	}
	if(icon != NULL) {
		if(old_icon == icon) {
			RedoIconName(win);
			return;
		}
		if(win->icon_on && visible(win)) {
			IconDown(win);
			if(old_icon && old_icon->w) {
				XUnmapWindow(dpy, old_icon->w);
			}
			win->icon = icon;
			OtpReassignIcon(win, old_icon);
			IconUp(win);
			OtpRaise(win, IconWin);
			XMapWindow(dpy, win->icon->w);
		}
		else {
			win->icon = icon;
			OtpReassignIcon(win, old_icon);
		}
		RedoIconName(win);
	}
	else {
		if(win->icon_on && visible(win)) {
			IconDown(win);
			if(old_icon && old_icon->w) {
				XUnmapWindow(dpy, old_icon->w);
			}
			/*
			 * If the icon name/class was found on one of the above lists,
			 * the call to CreateIconWindow() will find it again there
			 * and keep track of it on win->iconslist for eventual
			 * deallocation. (It is now checked that the current struct
			 * Icon is also already on that list)
			 */
			OtpFreeIcon(win);
			bool saveForceIcon = Scr->ForceIcon;
			Scr->ForceIcon = true;
			CreateIconWindow(win, -100, -100);
			Scr->ForceIcon = saveForceIcon;
			OtpRaise(win, IconWin);
			XMapWindow(dpy, win->icon->w);
		}
		else {
			OtpFreeIcon(win);
			win->icon = NULL;
			WMapUpdateIconName(win);
		}
		RedoIconName(win);
	}
}


/*
 * Resize the icon window, and reposition the image and name within it.
 * (a lot of the actual repositioning gets done during the later expose).
 */
void
RedoIconName(TwmWindow *win)
{
	int x;
	XRectangle ink_rect;
	XRectangle logical_rect;

	if(Scr->NoIconTitlebar ||
	                LookInNameList(Scr->NoIconTitle, win->icon_name) ||
	                LookInList(Scr->NoIconTitle, win->name, &win->class)) {
		WMapUpdateIconName(win);
		return;
	}
	if(win->iconmanagerlist) {
		/* let the expose event cause the repaint */
		XClearArea(dpy, win->iconmanagerlist->w, 0, 0, 0, 0, True);

		if(Scr->SortIconMgr) {
			SortIconManager(win->iconmanagerlist->iconmgr);
		}
	}

	if(!win->icon  || !win->icon->w) {
		WMapUpdateIconName(win);
		return;
	}

	if(win->icon->w_not_ours) {
		WMapUpdateIconName(win);
		return;
	}

	XmbTextExtents(Scr->IconFont.font_set,
	               win->icon_name, strlen(win->icon_name),
	               &ink_rect, &logical_rect);
	win->icon->w_width = logical_rect.width;
	win->icon->w_width += 2 * (Scr->IconManagerShadowDepth + ICON_MGR_IBORDER);
	if(win->icon->w_width > Scr->MaxIconTitleWidth) {
		win->icon->w_width = Scr->MaxIconTitleWidth;
	}

	if(win->icon->w_width < win->icon->width) {
		win->icon->x = (win->icon->width - win->icon->w_width) / 2;
		win->icon->x += Scr->IconManagerShadowDepth + ICON_MGR_IBORDER;
		win->icon->w_width = win->icon->width;
	}
	else {
		win->icon->x = Scr->IconManagerShadowDepth + ICON_MGR_IBORDER;
	}

	x = GetIconOffset(win->icon);
	win->icon->y = win->icon->height + Scr->IconFont.height +
	               Scr->IconManagerShadowDepth;
	win->icon->w_height = win->icon->height + Scr->IconFont.height +
	                      2 * (Scr->IconManagerShadowDepth + ICON_MGR_IBORDER);

	XResizeWindow(dpy, win->icon->w, win->icon->w_width,
	              win->icon->w_height);
	if(win->icon->bm_w) {
		XRectangle rect;

		XMoveWindow(dpy, win->icon->bm_w, x, 0);
		XMapWindow(dpy, win->icon->bm_w);
		if(win->icon->image && win->icon->image->mask) {
			XShapeCombineMask(dpy, win->icon->bm_w, ShapeBounding, 0, 0,
			                  win->icon->image->mask, ShapeSet);
			XShapeCombineMask(dpy, win->icon->w, ShapeBounding, x, 0,
			                  win->icon->image->mask, ShapeSet);
		}
		else if(win->icon->has_title) {
			rect.x      = x;
			rect.y      = 0;
			rect.width  = win->icon->width;
			rect.height = win->icon->height;
			XShapeCombineRectangles(dpy, win->icon->w, ShapeBounding,
			                        0, 0, &rect, 1, ShapeSet, 0);
		}
		if(win->icon->has_title) {
			if(Scr->ShrinkIconTitles && win->icon->title_shrunk) {
				rect.x      = x;
				rect.y      = win->icon->height;
				rect.width  = win->icon->width;
				rect.height = win->icon->w_height - win->icon->height;
			}
			else {
				rect.x      = 0;
				rect.y      = win->icon->height;
				rect.width  = win->icon->w_width;
				rect.height = win->icon->w_height - win->icon->height;
			}
			XShapeCombineRectangles(dpy,  win->icon->w, ShapeBounding, 0,
			                        0, &rect, 1, ShapeUnion, 0);
		}
	}
	if(Scr->ShrinkIconTitles &&
	                win->icon->title_shrunk &&
	                win->icon_on && (visible(win))) {
		IconDown(win);
		IconUp(win);
	}
	if(win->isicon) {
		XClearArea(dpy, win->icon->w, 0, 0, 0, 0, True);
	}

	WMapUpdateIconName(win);
}




/*
 ****************************************************************
 *
 * Misc internal utils.
 *
 ****************************************************************
 */


/*
 * What it says on the tin.
 */
static int
roundUp(int v, int multiple)
{
	return ((v + multiple - 1) / multiple) * multiple;
}


/*
 * Find the image set in Icons{} for a TwmWindow if possible.  Return the
 * image, record its provenance inside *icon, and pass back what pattern
 * it matched in **pattern.
 */
static Image *
LookupIconNameOrClass(TwmWindow *tmp_win, Icon *icon, char **pattern)
{
	char *icon_name = NULL;
	Image *image;
	Matchtype matched = match_none;

	icon_name = LookInNameList(Scr->IconNames, tmp_win->icon_name);
	if(icon_name != NULL) {
		*pattern = LookPatternInNameList(Scr->IconNames, tmp_win->icon_name);
		matched = match_list;
	}

	if(matched == match_none) {
		icon_name = LookInNameList(Scr->IconNames, tmp_win->name);
		if(icon_name != NULL) {
			*pattern = LookPatternInNameList(Scr->IconNames, tmp_win->name);
			matched = match_list;
		}
	}

	if(matched == match_none) {
		icon_name = LookInList(Scr->IconNames, tmp_win->name, &tmp_win->class);
		if(icon_name != NULL) {
			*pattern = LookPatternInList(Scr->IconNames, tmp_win->name,
			                             &tmp_win->class);
			matched = match_list;
		}
	}

	if((image  = GetImage(icon_name, icon->iconc)) != NULL) {
		icon->match  = matched;
		icon->image  = image;
		icon->width  = image->width;
		icon->height = image->height;
		tmp_win->forced = true;
	}
	else {
		icon->match = match_none;
		*pattern = NULL;
	}

	return image;
}
