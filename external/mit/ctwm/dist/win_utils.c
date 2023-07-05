/*
 * Window-handling utility funcs
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xatom.h>

#include "add_window.h" // NoName
#include "ctwm_atoms.h"
#include "drawing.h"
#include "events.h"
#include "event_internal.h" // Temp?
#ifdef EWMH
# include "ewmh_atoms.h"
#endif
#include "icons.h"
#include "list.h"
#include "occupation.h"
#include "otp.h"
#include "r_area.h"
#include "r_area_list.h"
#include "r_layout.h"
#include "screen.h"
#include "util.h"
#include "win_decorations.h"
#include "win_ops.h"
#include "win_utils.h"
#include "workspace_utils.h"


/*
 * Fill in size hints for a window from WM_NORMAL_HINTS prop.
 *
 * Formerly in add_window.c
 */
void
GetWindowSizeHints(TwmWindow *tmp)
{
	long supplied = 0;
	XSizeHints *hints = &tmp->hints;

	if(!XGetWMNormalHints(dpy, tmp->w, hints, &supplied)) {
		hints->flags = 0;
	}

	if(hints->flags & PResizeInc) {
		if(hints->width_inc == 0) {
			hints->width_inc = 1;
		}
		if(hints->height_inc == 0) {
			hints->height_inc = 1;
		}
	}

	if(!(supplied & PWinGravity) && (hints->flags & USPosition)) {
		static int gravs[] = { SouthEastGravity, SouthWestGravity,
		                       NorthEastGravity, NorthWestGravity
		                     };
		int right =  tmp->attr.x + tmp->attr.width + 2 * tmp->old_bw;
		int bottom = tmp->attr.y + tmp->attr.height + 2 * tmp->old_bw;
		hints->win_gravity =
		        gravs[((Scr->rooth - bottom <
		                tmp->title_height + 2 * tmp->frame_bw3D) ? 0 : 2) |
		              ((Scr->rootw - right   <
		                tmp->title_height + 2 * tmp->frame_bw3D) ? 0 : 1)];
		hints->flags |= PWinGravity;
	}

	/* Check for min size < max size */
	if((hints->flags & (PMinSize | PMaxSize)) == (PMinSize | PMaxSize)) {
		if(hints->max_width < hints->min_width) {
			if(hints->max_width > 0) {
				hints->min_width = hints->max_width;
			}
			else if(hints->min_width > 0) {
				hints->max_width = hints->min_width;
			}
			else {
				hints->max_width = hints->min_width = 1;
			}
		}

		if(hints->max_height < hints->min_height) {
			if(hints->max_height > 0) {
				hints->min_height = hints->max_height;
			}
			else if(hints->min_height > 0) {
				hints->max_height = hints->min_height;
			}
			else {
				hints->max_height = hints->min_height = 1;
			}
		}
	}
}


/*
 * Fill in info from WM_PROTOCOLS property
 *
 * Formerly in add_window.c
 */
void
FetchWmProtocols(TwmWindow *tmp)
{
	unsigned long flags = 0L;
	Atom *protocols = NULL;
	int n;

	if(XGetWMProtocols(dpy, tmp->w, &protocols, &n)) {
		int i;
		Atom *ap;

		for(i = 0, ap = protocols; i < n; i++, ap++) {
			if(*ap == XA_WM_TAKE_FOCUS) {
				flags |= DoesWmTakeFocus;
			}
			if(*ap == XA_WM_SAVE_YOURSELF) {
				flags |= DoesWmSaveYourself;
			}
			if(*ap == XA_WM_DELETE_WINDOW) {
				flags |= DoesWmDeleteWindow;
			}
		}
		if(protocols) {
			XFree(protocols);
		}
	}
	tmp->protocols = flags;
}


/*
 * Figure signs for calculating location offsets for a window dependent
 * on its gravity.
 *
 * Depending on how its gravity is set, offsets to window coordinates for
 * e.g. border widths may need to move either down (right) or up (left).
 * Or possibly not at all.  So we write multipliers into passed vars for
 * callers.
 *
 * Formerly in add_window.c
 */
void
GetGravityOffsets(TwmWindow *tmp, int *xp, int *yp)
{
	static struct _gravity_offset {
		int x, y;
	} gravity_offsets[] = {
		[ForgetGravity]    = {  0,  0 },
		[NorthWestGravity] = { -1, -1 },
		[NorthGravity]     = {  0, -1 },
		[NorthEastGravity] = {  1, -1 },
		[WestGravity]      = { -1,  0 },
		[CenterGravity]    = {  0,  0 },
		[EastGravity]      = {  1,  0 },
		[SouthWestGravity] = { -1,  1 },
		[SouthGravity]     = {  0,  1 },
		[SouthEastGravity] = {  1,  1 },
		[StaticGravity]    = {  0,  0 },
	};
	int g = ((tmp->hints.flags & PWinGravity)
	         ? tmp->hints.win_gravity : NorthWestGravity);

	if(g < ForgetGravity || g > StaticGravity) {
		*xp = *yp = 0;
	}
	else {
		*xp = gravity_offsets[g].x;
		*yp = gravity_offsets[g].y;
	}
}


/*
 * Finds the TwmWindow structure associated with a Window (if any), or
 * NULL.
 *
 * This is a relatively cheap function since it does not involve
 * communication with the server. Probably faster than walking the list
 * of TwmWindows, since the lookup is by a hash table.
 *
 * Formerly in add_window.c
 */
TwmWindow *
GetTwmWindow(Window w)
{
	TwmWindow *twmwin;
	int stat;

	stat = XFindContext(dpy, w, TwmContext, (XPointer *)&twmwin);
	if(stat == XCNOENT) {
		twmwin = NULL;
	}

	return twmwin;
}


/***********************************************************************
 *
 *  Procedure:
 *      GetWMPropertyString - Get Window Manager text property and
 *                              convert it to a string.
 *
 *  Returned Value:
 *      (char *) - pointer to the malloc'd string or NULL
 *
 *  Inputs:
 *      w       - the id of the window whose property is to be retrieved
 *      prop    - property atom (typically WM_NAME or WM_ICON_NAME)
 *
 ***********************************************************************
 *
 * Formerly in util.c
 */
char *
GetWMPropertyString(Window w, Atom prop)
{
	XTextProperty       text_prop;
	char                *stringptr;

	XGetTextProperty(dpy, w, &text_prop, prop);
	if(text_prop.value == NULL) {
		return NULL;
	}

	if(text_prop.encoding == XA_STRING
	                || text_prop.encoding == XA_UTF8_STRING
	                || text_prop.encoding == XA_COMPOUND_TEXT) {
		/* property is encoded as compound text - convert to locale string */
		char **text_list;
		int  text_list_count;
		int status;

		/* Check historical strictness */
		if(Scr->StrictWinNameEncoding) {
			bool fail = false;

			if((prop == XA_WM_NAME || prop == XA_WM_ICON_NAME)
			                && text_prop.encoding != XA_STRING
			                && text_prop.encoding != XA_COMPOUND_TEXT) {
				fail = true;
			}

#ifdef EWMH
			if((prop == XA__NET_WM_NAME || prop == XA__NET_WM_ICON_NAME)
			                && text_prop.encoding != XA_UTF8_STRING) {
				fail = true;
			}
#endif // EWMH

			if(fail) {
				fprintf(stderr, "%s: Invalid encoding for property %s "
				        "of window 0x%lx\n", ProgramName,
				        XGetAtomName(dpy, prop), w);
				XFree(text_prop.value);
				return NULL;
			}
		}


		status = XmbTextPropertyToTextList(dpy, &text_prop, &text_list,
		                                   &text_list_count);
		if(text_list_count == 0
		                || text_list == NULL
		                || text_list[0] == NULL) {
			// Got nothing
			XFree(text_prop.value);
			return NULL;
		}
		else if(status < 0 || text_list_count < 0) {
			// Got an error statuf
			switch(status) {
				case XConverterNotFound:
					fprintf(stderr,
					        "%s: Converter not found; unable to convert property %s of window ID %lx.\n",
					        ProgramName, XGetAtomName(dpy, prop), w);
					break;
				case XNoMemory:
					fprintf(stderr,
					        "%s: Insufficient memory; unable to convert property %s of window ID %lx.\n",
					        ProgramName, XGetAtomName(dpy, prop), w);
					break;
				case XLocaleNotSupported:
					fprintf(stderr,
					        "%s: Locale not supported; unable to convert property %s of window ID %lx.\n",
					        ProgramName, XGetAtomName(dpy, prop), w);
					break;
			}
			stringptr = NULL;
			/*
			   don't call XFreeStringList - text_list appears to have
			   invalid address if status is bad
			   XFreeStringList(text_list);
			*/
		}
		else {
			// Actually got the data!
			stringptr = strdup(text_list[0]);
			XFreeStringList(text_list);
		}
	}
	else {
		/* property is encoded in a format we don't understand */
		fprintf(stderr,
		        "%s: Encoding not STRING or COMPOUND_TEXT; unable to decode property %s of window ID %lx.\n",
		        ProgramName, XGetAtomName(dpy, prop), w);
		stringptr = NULL;
	}
	XFree(text_prop.value);

	return stringptr;
}


/*
 * Cleanup something stored that we got from the above originally.
 *
 * Formerly in util.c
 */
void
FreeWMPropertyString(char *prop)
{
	if(prop && prop != NoName) {
		free(prop);
	}
}


/*
 * Window mapped on some virtual screen?
 *
 * Formerly in util.c
 */
bool
visible(const TwmWindow *tmp_win)
{
	return (tmp_win->vs != NULL);
}


/*
 * Various code paths do a dance of "mask off notifications of event type
 * ; do something that triggers that event (but we're doing it, so we
 * don't need the notification) ; restore previous mask".  So have some
 * util funcs to make it more visually obvious.
 *
 * e.g.:
 *     long prev_mask = mask_out_event(w, PropertyChangeMask);
 *     do_something_that_changes_properties();
 *     restore_mask(prev_mask);
 *
 * We're cheating a little with the -1 return on mask_out_event(), as
 * that's theoretically valid for the data type.  It's not as far as I
 * can tell for X or us though; having all the bits set (well, I guess
 * I'm assuming 2s-complement too) is pretty absurd, and there are only
 * 25 defined bits in Xlib, so even on 32-bit systems, it shouldn't fill
 * up long.
 */
long
mask_out_event(Window w, long ignore_event)
{
	XWindowAttributes wattr;

	/* Get current mask */
	if(XGetWindowAttributes(dpy, w, &wattr) == 0) {
		return -1;
	}

	/*
	 * If we're ignoring nothing, nothing to do.  This is probably not
	 * strictly speaking a useful thing to ask for in general, but it's
	 * the right thing for us to do if we're asked to do nothing.
	 */
	if(ignore_event == 0) {
		return wattr.your_event_mask;
	}

	/* Delegate */
	return mask_out_event_mask(w, ignore_event, wattr.your_event_mask);
}

long
mask_out_event_mask(Window w, long ignore_event, long curmask)
{
	/* Set to the current, minus what we're wanting to ignore */
	XSelectInput(dpy, w, (curmask & ~ignore_event));

	/* Return what it was */
	return curmask;
}

int
restore_mask(Window w, long restore)
{
	return XSelectInput(dpy, w, restore);
}


/*
 * Setting and getting WM_STATE property.
 *
 * x-ref ICCCM section 4.1.3.1
 * https://tronche.com/gui/x/icccm/sec-4.html#s-4.1.3.1
 *
 * XXX These should probably be named more alike, as they're
 * complementary ops.
 */
void
SetMapStateProp(TwmWindow *tmp_win, int state)
{
	unsigned long data[2];              /* "suggested" by ICCCM version 1 */

	data[0] = (unsigned long) state;
	data[1] = (unsigned long)(tmp_win->iconify_by_unmapping ? None :
	                          (tmp_win->icon ? tmp_win->icon->w : None));

	XChangeProperty(dpy, tmp_win->w, XA_WM_STATE, XA_WM_STATE, 32,
	                PropModeReplace, (unsigned char *) data, 2);
}


bool
GetWMState(Window w, int *statep, Window *iwp)
{
	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytesafter;
	unsigned long *datap = NULL;
	bool retval = false;

	if(XGetWindowProperty(dpy, w, XA_WM_STATE, 0L, 2L, False, XA_WM_STATE,
	                      &actual_type, &actual_format, &nitems, &bytesafter,
	                      (unsigned char **) &datap) != Success || !datap) {
		return false;
	}

	if(nitems <= 2) {                   /* "suggested" by ICCCM version 1 */
		*statep = (int) datap[0];
		*iwp = (Window) datap[1];
		retval = true;
	}

	XFree(datap);
	return retval;
}


/*
 * Display a window's position in the dimensions window.  This is used
 * during various window positioning (during new window popups, moves,
 * etc).
 *
 * This reuses the same window for the position as is used during
 * resizing for the dimesions of the window in DisplaySize().  The
 * innards of the funcs can probably be collapsed together a little, and
 * the higher-level knowledge of Scr->SizeWindow (e.g., for unmapping
 * after ths op is done) should probably be encapsulated a bit better.
 */
void
DisplayPosition(const TwmWindow *_unused_tmp_win, int x, int y)
{
	char str [100];
	char signx = '+';
	char signy = '+';

	if(x < 0) {
		x = -x;
		signx = '-';
	}
	if(y < 0) {
		y = -y;
		signy = '-';
	}
	sprintf(str, " %c%-4d %c%-4d ", signx, x, signy, y);
	XRaiseWindow(dpy, Scr->SizeWindow);

	Draw3DBorder(Scr->SizeWindow, 0, 0,
	             Scr->SizeStringOffset + Scr->SizeStringWidth + SIZE_HINDENT,
	             Scr->SizeFont.height + SIZE_VINDENT * 2,
	             2, Scr->DefaultC, off, false, false);

	FB(Scr->DefaultC.fore, Scr->DefaultC.back);
	XmbDrawImageString(dpy, Scr->SizeWindow, Scr->SizeFont.font_set,
	                   Scr->NormalGC, Scr->SizeStringOffset,
	                   Scr->SizeFont.ascent + SIZE_VINDENT, str, 13);
}

void
MoveResizeSizeWindow(int x, int y, unsigned int width, unsigned int height)
{
	XResizeWindow(dpy, Scr->SizeWindow, width, height);

	if(Scr->CenterFeedbackWindow) {
		RArea monitor = RLayoutGetAreaAtXY(Scr->BorderedLayout, x, y);

		XMoveWindow(dpy, Scr->SizeWindow,
		            monitor.x + monitor.width / 2 - width / 2,
		            monitor.y + monitor.height / 2 - height / 2);
	}
}


/*
 * Various funcs for adjusting coordinates for windows based on
 * resistances etc.
 *
 * XXX In desperate need of better commenting.
 */
static void
_tryToPack(RArea *final, const RArea *cur_win)
{
	if(final->x >= cur_win->x + cur_win->width) {
		return;
	}
	if(final->y >= cur_win->y + cur_win->height) {
		return;
	}
	if(final->x + final->width <= cur_win->x) {
		return;
	}
	if(final->y + final->height <= cur_win->y) {
		return;
	}

	if(final->x + Scr->MovePackResistance > cur_win->x +
	                cur_win->width) {  /* left */
		final->x = MAX(final->x, cur_win->x + cur_win->width);
		return;
	}
	if(final->x + final->width < cur_win->x +
	                Scr->MovePackResistance) {  /* right */
		final->x = MIN(final->x, cur_win->x - final->width);
		return;
	}
	if(final->y + Scr->MovePackResistance > cur_win->y +
	                cur_win->height) {  /* top */
		final->y = MAX(final->y, cur_win->y + cur_win->height);
		return;
	}
	if(final->y + final->height < cur_win->y +
	                Scr->MovePackResistance) {  /* bottom */
		final->y = MIN(final->y, cur_win->y - final->height);
	}
}

static bool
_tryToPackVsEachMonitor(const RArea *monitor_area, void *vfinal)
{
	_tryToPack((RArea *)vfinal, monitor_area);
	return false;
}

void
TryToPack(TwmWindow *tmp_win, int *x, int *y)
{
	TwmWindow   *t;
	RArea cur_win;
	RArea final = RAreaNew(*x, *y,
	                       tmp_win->frame_width  + 2 * tmp_win->frame_bw,
	                       tmp_win->frame_height + 2 * tmp_win->frame_bw);

	/* Global layout is not a single rectangle, check against the
	 * monitor borders */
	if(Scr->BorderedLayout->horiz->len > 1) {
		RAreaListForeach(
		        Scr->BorderedLayout->monitors, _tryToPackVsEachMonitor, &final);
	}

	for(t = Scr->FirstWindow; t != NULL; t = t->next) {
		if(t == tmp_win) {
			continue;
		}
#ifdef WINBOX
		if(t->winbox != tmp_win->winbox) {
			continue;
		}
#endif
		if(t->vs != tmp_win->vs) {
			continue;
		}
		if(!t->mapped) {
			continue;
		}

		cur_win = RAreaNew(t->frame_x, t->frame_y,
		                   t->frame_width  + 2 * t->frame_bw,
		                   t->frame_height + 2 * t->frame_bw);

		_tryToPack(&final, &cur_win);
	}

	*x = final.x;
	*y = final.y;
}


/*
 * Directionals for TryToPush_be().  These differ from the specs for
 * jump/pack/fill in functions. because there's an indeterminate option.
 */
typedef enum {
	PD_ANY,
	PD_BOTTOM,
	PD_LEFT,
	PD_RIGHT,
	PD_TOP,
} PushDirection;
static void TryToPush_be(TwmWindow *tmp_win, int x, int y, PushDirection dir);

void
TryToPush(TwmWindow *tmp_win, int x, int y)
{
	TryToPush_be(tmp_win, x, y, PD_ANY);
}

static void
TryToPush_be(TwmWindow *tmp_win, int x, int y, PushDirection dir)
{
	TwmWindow   *t;
	int         newx, newy, ndir;
	bool        move;
	int         w, h;
	int         winw = tmp_win->frame_width  + 2 * tmp_win->frame_bw;
	int         winh = tmp_win->frame_height + 2 * tmp_win->frame_bw;

	for(t = Scr->FirstWindow; t != NULL; t = t->next) {
		if(t == tmp_win) {
			continue;
		}
#ifdef WINBOX
		if(t->winbox != tmp_win->winbox) {
			continue;
		}
#endif
		if(t->vs != tmp_win->vs) {
			continue;
		}
		if(!t->mapped) {
			continue;
		}

		w = t->frame_width  + 2 * t->frame_bw;
		h = t->frame_height + 2 * t->frame_bw;
		if(x >= t->frame_x + w) {
			continue;
		}
		if(y >= t->frame_y + h) {
			continue;
		}
		if(x + winw <= t->frame_x) {
			continue;
		}
		if(y + winh <= t->frame_y) {
			continue;
		}

		move = false;
		if((dir == PD_ANY || dir == PD_LEFT) &&
		                (x + Scr->MovePackResistance > t->frame_x + w)) {
			newx = x - w;
			newy = t->frame_y;
			ndir = PD_LEFT;
			move = true;
		}
		else if((dir == PD_ANY || dir == PD_RIGHT) &&
		                (x + winw < t->frame_x + Scr->MovePackResistance)) {
			newx = x + winw;
			newy = t->frame_y;
			ndir = PD_RIGHT;
			move = true;
		}
		else if((dir == PD_ANY || dir == PD_TOP) &&
		                (y + Scr->MovePackResistance > t->frame_y + h)) {
			newx = t->frame_x;
			newy = y - h;
			ndir = PD_TOP;
			move = true;
		}
		else if((dir == PD_ANY || dir == PD_BOTTOM) &&
		                (y + winh < t->frame_y + Scr->MovePackResistance)) {
			newx = t->frame_x;
			newy = y + winh;
			ndir = PD_BOTTOM;
			move = true;
		}
		if(move) {
			TryToPush_be(t, newx, newy, ndir);
			TryToPack(t, &newx, &newy);
			ConstrainByBorders(tmp_win,
			                   &newx, t->frame_width  + 2 * t->frame_bw,
			                   &newy, t->frame_height + 2 * t->frame_bw);
			SetupWindow(t, newx, newy, t->frame_width, t->frame_height, -1);
		}
	}
}


void
TryToGrid(TwmWindow *tmp_win, int *x, int *y)
{
	int w    = tmp_win->frame_width  + 2 * tmp_win->frame_bw;
	int h    = tmp_win->frame_height + 2 * tmp_win->frame_bw;
	int grav = ((tmp_win->hints.flags & PWinGravity)
	            ? tmp_win->hints.win_gravity : NorthWestGravity);

	switch(grav) {
		case ForgetGravity :
		case StaticGravity :
		case NorthWestGravity :
		case NorthGravity :
		case WestGravity :
		case CenterGravity :
			*x = ((*x - Scr->BorderLeft) / Scr->XMoveGrid) * Scr->XMoveGrid
			     + Scr->BorderLeft;
			*y = ((*y - Scr->BorderTop) / Scr->YMoveGrid) * Scr->YMoveGrid
			     + Scr->BorderTop;
			break;
		case NorthEastGravity :
		case EastGravity :
			*x = (((*x + w - Scr->BorderLeft) / Scr->XMoveGrid) *
			      Scr->XMoveGrid) - w + Scr->BorderLeft;
			*y = ((*y - Scr->BorderTop) / Scr->YMoveGrid) *
			     Scr->YMoveGrid + Scr->BorderTop;
			break;
		case SouthWestGravity :
		case SouthGravity :
			*x = ((*x - Scr->BorderLeft) / Scr->XMoveGrid) * Scr->XMoveGrid
			     + Scr->BorderLeft;
			*y = (((*y + h - Scr->BorderTop) / Scr->YMoveGrid) * Scr->YMoveGrid)
			     - h + Scr->BorderTop;
			break;
		case SouthEastGravity :
			*x = (((*x + w - Scr->BorderLeft) / Scr->XMoveGrid) *
			      Scr->XMoveGrid) - w + Scr->BorderLeft;
			*y = (((*y + h - Scr->BorderTop) / Scr->YMoveGrid) *
			      Scr->YMoveGrid) - h + Scr->BorderTop;
			break;
	}
}



#ifdef WINBOX
/*
 * Functions related to keeping windows from being placed off-screen (or
 * off-screen too far).  Involved in handling of params like DontMoveOff
 * and MoveOffResistance, etc.
 */
static void ConstrainLeftTop(int *value, int border);
static void ConstrainRightBottom(int *value, int size1, int border, int size2);
#endif

bool
ConstrainByLayout(RLayout *layout, int move_off_res, int *left, int width,
                  int *top, int height)
{
	RArea area = RAreaNew(*left, *top, width, height);
	int limit;
	bool clipped = false;

	limit = RLayoutFindBottomEdge(layout, &area) - height + 1;
	if(area.y > limit) {
		if(move_off_res >= 0 && area.y >= limit + move_off_res) {
			area.y -= move_off_res;
		}
		else {
			area.y = limit;
			clipped = true;
		}
	}

	limit = RLayoutFindRightEdge(layout, &area) - width + 1;
	if(area.x > limit) {
		if(move_off_res >= 0 && area.x >= limit + move_off_res) {
			area.x -= move_off_res;
		}
		else {
			area.x = limit;
			clipped = true;
		}
	}

	limit = RLayoutFindLeftEdge(layout, &area);
	if(area.x < limit) {
		if(move_off_res >= 0 && area.x <= limit - move_off_res) {
			area.x += move_off_res;
		}
		else {
			area.x = limit;
			clipped = true;
		}
	}

	limit = RLayoutFindTopEdge(layout, &area);
	if(area.y < limit) {
		if(move_off_res >= 0 && area.y <= limit - move_off_res) {
			area.y += move_off_res;
		}
		else {
			area.y = limit;
			clipped = true;
		}
	}

	*left = area.x;
	*top = area.y;

	return clipped;
}

void
ConstrainByBorders1(int *left, int width, int *top, int height)
{
	ConstrainByLayout(Scr->BorderedLayout, Scr->MoveOffResistance,
	                  left, width, top, height);
}

void
ConstrainByBorders(TwmWindow *twmwin, int *left, int width,
                   int *top, int height)
{
	if(false) {
		// Dummy
	}
#ifdef WINBOX
	else if(twmwin->winbox) {
		XWindowAttributes attr;
		XGetWindowAttributes(dpy, twmwin->winbox->window, &attr);
		ConstrainRightBottom(left, width, 0, attr.width);
		ConstrainLeftTop(left, 0);
		ConstrainRightBottom(top, height, 0, attr.height);
		ConstrainLeftTop(top, 0);
	}
#endif
	else {
		ConstrainByBorders1(left, width, top, height);
	}
}

#ifdef WINBOX
static void
ConstrainLeftTop(int *value, int border)
{
	if(*value < border) {
		if(Scr->MoveOffResistance < 0 ||
		                *value > border - Scr->MoveOffResistance) {
			*value = border;
		}
		else if(Scr->MoveOffResistance > 0 &&
		                *value <= border - Scr->MoveOffResistance) {
			*value = *value + Scr->MoveOffResistance;
		}
	}
}

static void
ConstrainRightBottom(int *value, int size1, int border, int size2)
{
	if(*value + size1 > size2 - border) {
		if(Scr->MoveOffResistance < 0 ||
		                *value + size1 < size2 - border + Scr->MoveOffResistance) {
			*value = size2 - size1 - border;
		}
		else if(Scr->MoveOffResistance > 0 &&
		                *value + size1 >= size2 - border + Scr->MoveOffResistance) {
			*value = *value - Scr->MoveOffResistance;
		}
	}
}
#endif


/*
 * Zoom over to a particular window.
 */
void
WarpToWindow(TwmWindow *t, bool must_raise)
{
	int x, y;

	if(t->ring.cursor_valid) {
		x = t->ring.curs_x;
		y = t->ring.curs_y;
#ifdef DEBUG
		fprintf(stderr, "WarpToWindow: cursor_valid; x == %d, y == %d\n", x, y);
#endif

		/*
		 * XXX is this correct with 3D borders? Easier check possible?
		 * frame_bw is for the left border.
		 */
		if(x < t->frame_bw) {
			x = t->frame_bw;
		}
		if(x >= t->frame_width + t->frame_bw) {
			x  = t->frame_width + t->frame_bw - 1;
		}
		if(y < t->title_height + t->frame_bw) {
			y = t->title_height + t->frame_bw;
		}
		if(y >= t->frame_height + t->frame_bw) {
			y  = t->frame_height + t->frame_bw - 1;
		}
#ifdef DEBUG
		fprintf(stderr, "WarpToWindow: adjusted    ; x := %d, y := %d\n", x, y);
#endif
	}
	else {
		x = t->frame_width / 2;
		y = t->frame_height / 2;
#ifdef DEBUG
		fprintf(stderr, "WarpToWindow: middle; x := %d, y := %d\n", x, y);
#endif
	}
#if 0
	int dest_x, dest_y;
	Window child;

	/*
	 * Check if the proposed position actually is visible. If not, raise the window.
	 * "If the coordinates are contained in a mapped
	 * child of dest_w, that child is returned to child_return."
	 * We'll need to check for the right child window; the frame probably.
	 * (What about XXX window boxes?)
	 *
	 * Alternatively, use XQueryPointer() which returns the root window
	 * the pointer is in, but XXX that won't work for VirtualScreens.
	 */
	if(XTranslateCoordinates(dpy, t->frame, Scr->Root, x, y, &dest_x, &dest_y,
	                         &child)) {
		if(child != t->frame) {
			must_raise = true;
		}
	}
#endif
	if(t->auto_raise || must_raise) {
		AutoRaiseWindow(t);
	}
	if(! visible(t)) {
		WorkSpace *wlist;

		for(wlist = Scr->workSpaceMgr.workSpaceList; wlist != NULL;
		                wlist = wlist->next) {
			if(OCCUPY(t, wlist)) {
				break;
			}
		}
		if(wlist != NULL) {
			GotoWorkSpace(Scr->currentvs, wlist);
		}
	}

	XWarpPointer(dpy, None, Scr->Root, 0, 0, 0, 0, x + t->frame_x, y + t->frame_y);
	SetFocus(t, EventTime);

#ifdef DEBUG
	{
		Window root_return;
		Window child_return;
		int root_x_return;
		int root_y_return;
		int win_x_return;
		int win_y_return;
		unsigned int mask_return;

		if(XQueryPointer(dpy, t->frame, &root_return, &child_return, &root_x_return,
		                 &root_y_return, &win_x_return, &win_y_return, &mask_return)) {
			fprintf(stderr,
			        "XQueryPointer: root_return=%x, child_return=%x, root_x_return=%d, root_y_return=%d, win_x_return=%d, win_y_return=%d\n",
			        root_return, child_return, root_x_return, root_y_return, win_x_return,
			        win_y_return);
		}
	}
#endif
}


/*
 * ICCCM Client Messages - Section 4.2.8 of the ICCCM dictates that all
 * client messages will have the following form:
 *
 *     event type       ClientMessage
 *     message type     XA_WM_PROTOCOLS
 *     window           tmp->w
 *     format           32
 *     data[0]          message atom
 *     data[1]          time stamp
 */
void
send_clientmessage(Window w, Atom a, Time timestamp)
{
	XClientMessageEvent ev;

	ev.type = ClientMessage;
	ev.window = w;
	ev.message_type = XA_WM_PROTOCOLS;
	ev.format = 32;
	ev.data.l[0] = a;
	ev.data.l[1] = timestamp;
	XSendEvent(dpy, w, False, 0L, (XEvent *) &ev);
}


/*
 * Create synthetic WM_HINTS info for windows.  When a window specifies
 * stuff, we should probably pay attention to it (though we don't
 * always; x-ref comments in AddWindow() especially about focus).
 * However, when it doesn't tell us anything at all, we should assume
 * something useful.  "Window managers are free to assume convenient
 * values for all fields of the WM_HINTS property if a window is mapped
 * without one."  (ICCCM Ch. 4,
 * <https://www.x.org/releases/X11R7.7/doc/xorg-docs/icccm/icccm.html#Client_Properties>).
 *
 * Specifically, we assume it wants us to give it focus.  It's fairly
 * bogus for a window not to tell us anything, but e.g current versions
 * of Chrome do (don't do) just that.  So we better make up something
 * useful.
 *
 * Should probably be some configurability for this, so make the func
 * take the window, even though we don't currently do anything useful
 * with it...
 */
XWMHints *
gen_synthetic_wmhints(TwmWindow *win)
{
	XWMHints *hints;

	hints = XAllocWMHints();
	if(!hints) {
		return NULL;
	}

	/*
	 * Reasonable defaults.  Takes input, in normal state.
	 *
	 * XXX Make configurable?
	 */
	hints->flags = InputHint | StateHint;
	hints->input = True;
	hints->initial_state = NormalState;

	return hints;
}


/**
 * Perform whatever adaptations of WM_HINTS info we do.
 *
 * Most of these relate to focus, but we also fiddle with group
 * membership.
 */
XWMHints *
munge_wmhints(TwmWindow *win, XWMHints *hints)
{
	/*
	 * If we have WM_HINTS, but they don't tell us anything about focus,
	 * force it to true for our purposes.
	 *
	 * CL: Having with not willing focus cause problems with AutoSqueeze
	 * and a few others things. So I suppress it. And the whole focus
	 * thing is buggy anyway.
	 */
	if(!(hints->flags & InputHint)) {
		hints->input = True;
	}

	/*
	 * Now we're expecting to give the window focus if it asked for it
	 * via WM_HINTS, if it didn't say anything one way or the other in
	 * WM_HINTS, or if it didn't give us any WM_HINTS at all.  But if it
	 * explicitly asked not to, we don't give it unless overridden by
	 * config.
	 */
	if(Scr->ForceFocus || IsInList(Scr->ForceFocusL, win)) {
		hints->input = True;
	}


	/* Setup group bits */
	if(hints->flags & WindowGroupHint) {
		win->group = hints->window_group;
		if(win->group) {
			/*
			 * GTK windows often have a spurious "group leader" window which is
			 * never reported to us and therefore does not really exist.  This
			 * is annoying because we treat group members a lot like transient
			 * windows.  Look for that here. It is in fact a duplicate of the
			 * WM_CLIENT_LEADER property.
			 */
			if(win->group != win->w && !GetTwmWindow(win->group)) {
				win->group = 0;
			}
		}
	}
	else {
		win->group = 0;
	}

	return hints;
}


/**
 * [Re]set a window's name.  This goes over the available naming sources
 * for the window and points the TwmWindow::name at the appropriate one.
 * It may also set a property to signal other EWMH-aware clients when
 * we're naming it a way they can't see themselves.
 *
 * \note This should rarely be called directly; apply_window_name()
 * should be used instead.  It's split out because we need to do this
 * step individually in AddWindow().
 *
 * \note Note also that we never need to worry about freeing the
 * TwmWindow::name; it always points to one of the TwmWindow::names
 * values (which are free'd by the event handler when they change) or to
 * NoName (which is static).  So we can just casually flip it around at
 * will.
 */
bool
set_window_name(TwmWindow *win)
{
	char *newname = NULL;
#define TRY(fld) { \
                if(newname == NULL && win->names.fld != NULL) { \
                        newname = win->names.fld; \
                } \
        }
	TRY(ctwm_wm_name)
#ifdef EWMH
	TRY(net_wm_name)
#endif
	TRY(wm_name)
#undef TRY

	if(newname == NULL) {
		newname = NoName;
	}
	if(win->name == newname) {
		return false; // Nothing to do
	}

	// Now we know what to call it
	win->name = newname;

#ifdef EWMH
	// EWMH says we set an additional property on any windows where what
	// we consider the name isn't what's in _NET_WM_NAME, so pagers etc
	// can call it the same as we do.
	//
	// The parts of the text describing it conflict a little; at one
	// place, it implies this should be set unless we're using
	// _NET_WM_NAME, in another it seems to suggest WM_NAME should be
	// considered applicable too.  I choose to implement it excluding
	// both, so this only gets set if we're overriding either standard
	// naming (probably rare).
	if(win->name != win->names.net_wm_name && win->name != win->names.wm_name) {
		// XXX We're not doing any checking of the encoding here...  I
		// don't see that Xlib helps us any, so we probably have to fall
		// back to iconv?  That came into the base in POSIX 2008, but was
		// in XSI back into the 90's I believe?
		XChangeProperty(dpy, win->w, XA__NET_WM_VISIBLE_NAME, XA_UTF8_STRING,
		                8, PropModeReplace, (unsigned char *)win->name,
		                strlen(win->name));
	}
	else {
		XDeleteProperty(dpy, win->w, XA__NET_WM_VISIBLE_NAME);
	}
#endif // EWMH

	// We set a name
	return true;
}


/**
 * [Re]set and apply changes to a window's name.  This is called after
 * we've received a new WM_NAME (or other name-setting) property, to
 * update our titlebars, icon managers, etc.
 */
void
apply_window_name(TwmWindow *win)
{
	/* [Re]set ->name */
	if(set_window_name(win) == false) {
		// No change
		return;
	}
	win->nameChanged = true;


	/* Update the active name */
	{
		XRectangle inc_rect;
		XRectangle logical_rect;

		XmbTextExtents(Scr->TitleBarFont.font_set,
		               win->name, strlen(win->name),
		               &inc_rect, &logical_rect);
		win->name_width = logical_rect.width;
	}

	/* recompute the priority if necessary */
	if(Scr->AutoPriority) {
		OtpRecomputePrefs(win);
	}

	SetupWindow(win, win->frame_x, win->frame_y,
	            win->frame_width, win->frame_height, -1);

	if(win->title_w) {
		XClearArea(dpy, win->title_w, 0, 0, 0, 0, True);
	}
	if(Scr->AutoOccupy) {
		WmgrRedoOccupation(win);
	}

#if 0
	/* Experimental, not yet working. */
	{
		ColorPair cp;
		int f, b;

		f = GetColorFromList(Scr->TitleForegroundL, win->name,
		                     &win->class, &cp.fore);
		b = GetColorFromList(Scr->TitleBackgroundL, win->name,
		                     &win->class, &cp.back);
		if(f || b) {
			if(Scr->use3Dtitles  && !Scr->BeNiceToColormap) {
				GetShadeColors(&cp);
			}
			win->title = cp;
		}
		f = GetColorFromList(Scr->BorderColorL, win->name,
		                     &win->class, &cp.fore);
		b = GetColorFromList(Scr->BorderColorL, win->name,
		                     &win->class, &cp.back);
		if(f || b) {
			if(Scr->use3Dborders && !Scr->BeNiceToColormap) {
				GetShadeColors(&cp);
			}
			win->borderC = cp;
		}

		f = GetColorFromList(Scr->BorderTileForegroundL, win->name,
		                     &win->class, &cp.fore);
		b = GetColorFromList(Scr->BorderTileBackgroundL, win->name,
		                     &win->class, &cp.back);
		if(f || b) {
			if(Scr->use3Dborders && !Scr->BeNiceToColormap) {
				GetShadeColors(&cp);
			}
			win->border_tile = cp;
		}
	}
#endif

	/*
	 * If we haven't set a separate icon name, we use the window name, so
	 * we need to update it.
	 */
	if(win->names.icon_set == false) {
		apply_window_icon_name(win);
	}
	AutoPopupMaybe(win);

	return;
}


/**
 * [Re]set a window's icon name.  As with the window name version in
 * set_window_name(), this is mostly separate so the AddWindow() process
 * can call it.
 *
 * \note As with TwmWindow::name, we never want to try free()'ing or the
 * like TwmWindow::icon_name.
 *
 * \sa set_window_name() for details; this is just the icon name
 * equivalent of it.
 */
bool
set_window_icon_name(TwmWindow *win)
{
	char *newname = NULL;
#define TRY(fld) { \
                if(newname == NULL && win->names.fld != NULL) { \
                        newname = win->names.fld; \
                        win->names.icon_set = true; \
                } \
        }
	TRY(ctwm_wm_icon_name)
#ifdef EWMH
	TRY(net_wm_icon_name)
#endif
	TRY(wm_icon_name)
#undef TRY

	// Our fallback for icon names is the window name.  Flag when we're
	// doing that, so the window name handler can know when it needs to
	// call us.
	if(newname == NULL) {
		newname = win->name;
		win->names.icon_set = false;
	}
	if(win->icon_name == newname) {
		return false; // Nothing to do
	}

	// A name is chosen
	win->icon_name = newname;

#ifdef EWMH
	// EWMH asks for _NET_WM_VISIBLE_ICON_NAME in various cases where
	// we're not using 'standard' properties' values.  x-ref comments above in
	// set_window_name() about the parallel property for the window name
	// for various caveats.
	if(win->icon_name != win->names.net_wm_icon_name
	                && win->icon_name != win->names.wm_icon_name) {
		// XXX Still encoding questionable; x-ref above.
		XChangeProperty(dpy, win->w, XA__NET_WM_VISIBLE_ICON_NAME,
		                XA_UTF8_STRING,
		                8, PropModeReplace, (unsigned char *)win->icon_name,
		                strlen(win->icon_name));
	}
	else {
		XDeleteProperty(dpy, win->w, XA__NET_WM_VISIBLE_ICON_NAME);
	}
#endif // EWMH

	// Did it
	return true;
}


/**
 * [Re]set and apply changes to a window's icon name.  This is called
 * after we've received a new WM_ICON_NAME (or other name-setting)
 * property, to update our titlebars, icon managers, etc.
 *
 * \sa apply_window_name() which does the same for the window title.
 */
void
apply_window_icon_name(TwmWindow *win)
{
	/* [Re]set ->icon_name */
	if(set_window_icon_name(win) == false) {
		// No change
		return;
	}


	/* Lot less to do for icons... */
	RedoIcon(Tmp_win);
	AutoPopupMaybe(Tmp_win);

	return;
}
