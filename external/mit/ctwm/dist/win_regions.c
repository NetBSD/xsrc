/*
 * WindowRegion handling
 */

#include "ctwm.h"

#include <stdlib.h>

#include "list.h"
#include "screen.h"
#include "win_regions.h"


static void splitWindowRegionEntry(WindowEntry *we,
                                   RegGravity grav1, RegGravity grav2,
                                   int w, int h);
static WindowEntry *findWindowEntry(WorkSpace *wl,
                                    TwmWindow *tmp_win, WindowRegion **wrp);
static WindowEntry *prevWindowEntry(WindowEntry *we, WindowRegion *wr);
static void mergeWindowEntries(WindowEntry *old, WindowEntry *we);



/*
 * Backend for the parser when it hits WindowRegion
 */
name_list **
AddWindowRegion(char *geom, RegGravity grav1, RegGravity grav2)
{
	WindowRegion *wr;
	int mask;

	wr = malloc(sizeof(WindowRegion));
	wr->next = NULL;

	if(!Scr->FirstWindowRegion) {
		Scr->FirstWindowRegion = wr;
	}

	wr->entries    = NULL;
	wr->clientlist = NULL;
	wr->grav1      = grav1;
	wr->grav2      = grav2;
	wr->x = wr->y = wr->w = wr->h = 0;

	mask = XParseGeometry(geom, &wr->x, &wr->y, (unsigned int *) &wr->w,
	                      (unsigned int *) &wr->h);

	if(mask & XNegative) {
		wr->x += Scr->rootw - wr->w;
	}
	if(mask & YNegative) {
		wr->y += Scr->rooth - wr->h;
	}

	return (&(wr->clientlist));
}


/*
 * Called during startup after the config parsing (which would hit
 * AddWindowRegion() above) to do some further setup.
 */
void
CreateWindowRegions(void)
{
	WindowRegion  *wr, *wr1 = NULL, *wr2 = NULL;
	WorkSpace *wl;

	for(wl = Scr->workSpaceMgr.workSpaceList; wl != NULL; wl = wl->next) {
		wl->FirstWindowRegion = NULL;
		wr2 = NULL;
		for(wr = Scr->FirstWindowRegion; wr != NULL; wr = wr->next) {
			wr1  = malloc(sizeof(WindowRegion));
			*wr1 = *wr;
			wr1->entries = calloc(1, sizeof(WindowEntry));
			wr1->entries->x = wr1->x;
			wr1->entries->y = wr1->y;
			wr1->entries->w = wr1->w;
			wr1->entries->h = wr1->h;
			if(wr2) {
				wr2->next = wr1;
			}
			else {
				wl->FirstWindowRegion = wr1;
			}
			wr2 = wr1;
		}
		if(wr1) {
			wr1->next = NULL;
		}
	}
}


/*
 * Funcs for putting windows into and taking them out of regions.
 * Similarly to icons in IconRegion's, this writes the coordinates into
 * final_[xy] after setting up the Window Region/Entry structures and
 * stashing them in tmp_win as necessary.  Or it doesn't have anything to
 * do (like if the user doesn't have WindowRegion's config'd), and it
 * doesn't touch anything and returns false.
 */
bool
PlaceWindowInRegion(TwmWindow *tmp_win, int *final_x, int *final_y)
{
	WindowRegion  *wr;
	WindowEntry   *we;
	int           w, h;
	WorkSpace     *wl;

	if(!Scr->FirstWindowRegion) {
		return false;
	}
	for(wl = Scr->workSpaceMgr.workSpaceList; wl != NULL; wl = wl->next) {
		if(OCCUPY(tmp_win, wl)) {
			break;
		}
	}
	if(!wl) {
		return false;
	}
	w = tmp_win->frame_width;
	h = tmp_win->frame_height;
	we = NULL;
	for(wr = wl->FirstWindowRegion; wr; wr = wr->next) {
		if(LookInList(wr->clientlist, tmp_win->name, &tmp_win->class)) {
			for(we = wr->entries; we; we = we->next) {
				if(we->used) {
					continue;
				}
				if(we->w >= w && we->h >= h) {
					break;
				}
			}
			if(we) {
				break;
			}
		}
	}
	tmp_win->wr = NULL;
	if(!we) {
		return false;
	}

	splitWindowRegionEntry(we, wr->grav1, wr->grav2, w, h);
	we->used = true;
	we->twm_win = tmp_win;
	*final_x = we->x;
	*final_y = we->y;
	tmp_win->wr = wr;
	return true;
}


/*
 * Taking a window out of a region.  Doesn't do anything with the
 * _window_, just disconnects it from the data structures describing the
 * regions and entries.
 */
void
RemoveWindowFromRegion(TwmWindow *tmp_win)
{
	WindowEntry  *we, *wp, *wn;
	WindowRegion *wr;
	WorkSpace    *wl;

	if(!Scr->FirstWindowRegion) {
		return;
	}
	we = NULL;
	for(wl = Scr->workSpaceMgr.workSpaceList; wl != NULL; wl = wl->next) {
		we = findWindowEntry(wl, tmp_win, &wr);
		if(we) {
			break;
		}
	}
	if(!we) {
		return;
	}

	we->twm_win = NULL;
	we->used = false;
	wp = prevWindowEntry(we, wr);
	wn = we->next;
	for(;;) {
		if(wp && wp->used == false &&
		                ((wp->x == we->x && wp->w == we->w) ||
		                 (wp->y == we->y && wp->h == we->h))) {
			wp->next = we->next;
			mergeWindowEntries(we, wp);
			free(we);
			we = wp;
			wp = prevWindowEntry(wp, wr);
		}
		else if(wn && wn->used == false &&
		                ((wn->x == we->x && wn->w == we->w) ||
		                 (wn->y == we->y && wn->h == we->h))) {
			we->next = wn->next;
			mergeWindowEntries(wn, we);
			free(wn);
			wn = we->next;
		}
		else {
			break;
		}
	}
}


/*
 * Creating a new space inside a region.
 *
 * x-ref comment on splitIconRegionEntry() for grodiness.
 */
static void
splitWindowRegionEntry(WindowEntry *we, RegGravity grav1, RegGravity grav2,
                       int w, int h)
{
	switch(grav1) {
		case GRAV_NORTH:
		case GRAV_SOUTH:
			if(w != we->w) {
				splitWindowRegionEntry(we, grav2, grav1, w, we->h);
			}
			if(h != we->h) {
				WindowEntry *new = calloc(1, sizeof(WindowEntry));
				new->next = we->next;
				we->next  = new;
				new->x    = we->x;
				new->h    = (we->h - h);
				new->w    = we->w;
				we->h     = h;
				if(grav1 == GRAV_SOUTH) {
					new->y = we->y;
					we->y  = new->y + new->h;
				}
				else {
					new->y = we->y + we->h;
				}
			}
			break;
		case GRAV_EAST:
		case GRAV_WEST:
			if(h != we->h) {
				splitWindowRegionEntry(we, grav2, grav1, we->w, h);
			}
			if(w != we->w) {
				WindowEntry *new = calloc(1, sizeof(WindowEntry));
				new->next = we->next;
				we->next  = new;
				new->y    = we->y;
				new->w    = (we->w - w);
				new->h    = we->h;
				we->w = w;
				if(grav1 == GRAV_EAST) {
					new->x = we->x;
					we->x  = new->x + new->w;
				}
				else {
					new->x = we->x + we->w;
				}
			}
			break;
	}
}


/*
 * Utils for finding and merging various WindowEntry's
 */
static WindowEntry *
findWindowEntry(WorkSpace *wl, TwmWindow *tmp_win, WindowRegion **wrp)
{
	WindowRegion *wr;
	WindowEntry  *we;

	for(wr = wl->FirstWindowRegion; wr; wr = wr->next) {
		for(we = wr->entries; we; we = we->next) {
			if(we->twm_win == tmp_win) {
				if(wrp) {
					*wrp = wr;
				}
				return we;
			}
		}
	}
	return NULL;
}


static WindowEntry *
prevWindowEntry(WindowEntry *we, WindowRegion *wr)
{
	WindowEntry *wp;

	if(we == wr->entries) {
		return 0;
	}
	for(wp = wr->entries; wp->next != we; wp = wp->next);
	return wp;
}


static void
mergeWindowEntries(WindowEntry *old, WindowEntry *we)
{
	if(old->y == we->y) {
		we->w = old->w + we->w;
		if(old->x < we->x) {
			we->x = old->x;
		}
	}
	else {
		we->h = old->h + we->h;
		if(old->y < we->y) {
			we->y = old->y;
		}
	}
}
