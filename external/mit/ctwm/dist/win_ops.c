/*
 * Various operations done on windows.
 */

#include "ctwm.h"

#include <stdio.h>

#include "animate.h"
#include "colormaps.h"
#include "drawing.h"
#include "events.h"
#include "iconmgr.h"
#include "image.h"
#include "otp.h"
#include "screen.h"
#include "win_decorations.h"
#include "win_iconify.h"
#include "win_ops.h"
#include "win_utils.h"


/*
 * Update the visuals of a window (e.g., its own decorations and its
 * representation in the icon manager) for having/losing focus.
 *
 * Formerly in util.c
 */
void
SetFocusVisualAttributes(TwmWindow *tmp_win, bool focus)
{
	if(! tmp_win) {
		return;
	}

	if(focus == tmp_win->hasfocusvisible) {
		return;
	}
	if(tmp_win->highlight) {
		if(Scr->use3Dborders) {
			PaintBorders(tmp_win, focus);
		}
		else {
			if(focus) {
				XSetWindowBorder(dpy, tmp_win->frame, tmp_win->borderC.back);
				if(tmp_win->title_w) {
					XSetWindowBorder(dpy, tmp_win->title_w, tmp_win->borderC.back);
				}
			}
			else {
				/*
				 * XXX It seems possible this could be replaced by a
				 * single global 'gray' pixmap; I don't think it actually
				 * varies per window, and I don't see any obvious reason
				 * it can't be reused, so we may be able to save an
				 * allocation for each window by doing so...
				 */
				XSetWindowBorderPixmap(dpy, tmp_win->frame, tmp_win->gray);
				if(tmp_win->title_w) {
					XSetWindowBorderPixmap(dpy, tmp_win->title_w, tmp_win->gray);
				}
			}
		}
	}

	if(focus) {
		bool hil = false;

		if(tmp_win->lolite_wl) {
			XUnmapWindow(dpy, tmp_win->lolite_wl);
		}
		if(tmp_win->lolite_wr) {
			XUnmapWindow(dpy, tmp_win->lolite_wr);
		}
		if(tmp_win->hilite_wl) {
			XMapWindow(dpy, tmp_win->hilite_wl);
			hil = true;
		}
		if(tmp_win->hilite_wr) {
			XMapWindow(dpy, tmp_win->hilite_wr);
			hil = true;
		}
		if(hil && tmp_win->HiliteImage && tmp_win->HiliteImage->next) {
			MaybeAnimate = true;
		}
		if(tmp_win->iconmanagerlist) {
			ActiveIconManager(tmp_win->iconmanagerlist);
		}
	}
	else {
		if(tmp_win->hilite_wl) {
			XUnmapWindow(dpy, tmp_win->hilite_wl);
		}
		if(tmp_win->hilite_wr) {
			XUnmapWindow(dpy, tmp_win->hilite_wr);
		}
		if(tmp_win->lolite_wl) {
			XMapWindow(dpy, tmp_win->lolite_wl);
		}
		if(tmp_win->lolite_wr) {
			XMapWindow(dpy, tmp_win->lolite_wr);
		}
		if(tmp_win->iconmanagerlist) {
			NotActiveIconManager(tmp_win->iconmanagerlist);
		}
	}
	if(Scr->use3Dtitles && Scr->SunkFocusWindowTitle && tmp_win->title_height) {
		ButtonState bs;

		bs = focus ? on : off;
		Draw3DBorder(tmp_win->title_w, Scr->TBInfo.titlex, 0,
		             tmp_win->title_width - Scr->TBInfo.titlex -
		             Scr->TBInfo.rightoff - Scr->TitlePadding,
		             Scr->TitleHeight, Scr->TitleShadowDepth,
		             tmp_win->title, bs, false, false);
	}
	tmp_win->hasfocusvisible = focus;
}


/*
 * Shift the focus to a given window, and do whatever subsidiary ops that
 * entails.
 *
 * Formerly in util.c
 */
void
SetFocus(TwmWindow *tmp_win, Time tim)
{
	Window w = (tmp_win ? tmp_win->w : PointerRoot);
	bool f_iconmgr = false;

	if(Scr->Focus && (Scr->Focus->isiconmgr)) {
		f_iconmgr = true;
	}
	if(Scr->SloppyFocus && (w == PointerRoot) && (!f_iconmgr)) {
		return;
	}

	XSetInputFocus(dpy, w, RevertToPointerRoot, tim);
#ifdef EWMH
	EwmhSet_NET_ACTIVE_WINDOW(w);
#endif
	if(Scr->Focus == tmp_win) {
		return;
	}

	if(Scr->Focus) {
		if(Scr->Focus->AutoSqueeze && !Scr->Focus->squeezed) {
			AutoSqueeze(Scr->Focus);
		}
		SetFocusVisualAttributes(Scr->Focus, false);
#ifdef EWMH
		// Priority may change when focus does
		if(OtpIsFocusDependent(Scr->Focus)) {
			OtpUnfocusWindow(Scr->Focus);
			// That Scr->Focus = NULL's internally for us, but we don't
			// care, since we're about to reset it if we need to.
		}
#endif
	}

	if(tmp_win) {
		if(tmp_win->AutoSqueeze && tmp_win->squeezed) {
			AutoSqueeze(tmp_win);
		}
		SetFocusVisualAttributes(tmp_win, true);
#ifdef EWMH
		// Priority may change when focus does
		if(OtpIsFocusDependent(tmp_win)) {
			OtpFocusWindow(tmp_win);
			// Pre-sets Scr->Focus
		}
#endif
	}

	// in the EWMH cases, this was already done.
	Scr->Focus = tmp_win;

	return;
}


/*
 * Move the focus straight to the root, with associated cleanup.
 *
 * Formerly in menus.c
 */
void FocusOnRoot(void)
{
	SetFocus(NULL, EventTime);
	InstallColormaps(0, &Scr->RootColormaps);
	if(! Scr->ClickToFocus) {
		Scr->FocusRoot = true;
	}
}


/*
 * Handle doing squeezing bits for AutoSqueeze{} windows.
 *
 * Formerly in menus.c
 */
void
AutoSqueeze(TwmWindow *tmp_win)
{
	if(tmp_win->isiconmgr) {
		return;
	}
	if(Scr->RaiseWhenAutoUnSqueeze && tmp_win->squeezed) {
		OtpRaise(tmp_win, WinWin);
	}
	Squeeze(tmp_win);
}


/*
 * Toggle a window's squeezed state.
 *
 * Formerly in menus.c
 */
void
Squeeze(TwmWindow *tmp_win)
{
	long fx, fy, savex, savey;
	int  neww, newh;
	bool south;
	int  grav = ((tmp_win->hints.flags & PWinGravity)
	             ? tmp_win->hints.win_gravity : NorthWestGravity);
	long eventMask;
	if(tmp_win->squeezed) {
		tmp_win->squeezed = false;
#ifdef EWMH
		EwmhSet_NET_WM_STATE(tmp_win, EWMH_STATE_SHADED);
#endif /* EWMH */
		if(!tmp_win->isicon) {
			XMapWindow(dpy, tmp_win->w);
		}
		SetupWindow(tmp_win, tmp_win->actual_frame_x, tmp_win->actual_frame_y,
		            tmp_win->actual_frame_width, tmp_win->actual_frame_height, -1);
		ReMapTransients(tmp_win);
		return;
	}

	newh = tmp_win->title_height + 2 * tmp_win->frame_bw3D;
	if(newh < 3) {
		XBell(dpy, 0);
		return;
	}
	switch(grav) {
		case SouthWestGravity :
		case SouthGravity :
		case SouthEastGravity :
			south = true;
			break;
		default :
			south = false;
			break;
	}
	if(tmp_win->title_height && !tmp_win->AlwaysSqueezeToGravity) {
		south = false;
	}

	tmp_win->squeezed = true;
	tmp_win->actual_frame_width  = tmp_win->frame_width;
	tmp_win->actual_frame_height = tmp_win->frame_height;
	savex = fx = tmp_win->frame_x;
	savey = fy = tmp_win->frame_y;
	neww  = tmp_win->actual_frame_width;
	if(south) {
		fy += tmp_win->frame_height - newh;
	}
	if(tmp_win->squeeze_info) {
		fx  += tmp_win->title_x + tmp_win->frame_bw - tmp_win->frame_bw3D;
		neww = tmp_win->title_width + 2 * tmp_win->frame_bw3D;
	}

	eventMask = mask_out_event(tmp_win->w, StructureNotifyMask);
#ifdef EWMH
	EwmhSet_NET_WM_STATE(tmp_win, EWMH_STATE_SHADED);
#endif /* EWMH */
	XUnmapWindow(dpy, tmp_win->w);
	restore_mask(tmp_win->w, eventMask);

	if(fx + neww >= Scr->rootw - Scr->BorderRight) {
		fx = Scr->rootw - Scr->BorderRight - neww;
	}
	if(fy + newh >= Scr->rooth - Scr->BorderBottom) {
		fy = Scr->rooth - Scr->BorderBottom - newh;
	}
	SetupWindow(tmp_win, fx, fy, neww, newh, -1);
	tmp_win->actual_frame_x = savex;
	tmp_win->actual_frame_y = savey;

	/* Now make the group members disappear */
	UnmapTransients(tmp_win, false, eventMask);
}


/***********************************************************************
 *
 *  Procedure:
 *      MoveOutline - move a window outline
 *
 *  Inputs:
 *      root        - the window we are outlining
 *      x           - upper left x coordinate
 *      y           - upper left y coordinate
 *      width       - the width of the rectangle
 *      height      - the height of the rectangle
 *      bw          - the border width of the frame
 *      th          - title height
 *
 ***********************************************************************
 */
void
MoveOutline(Window root, int x, int y, int width, int height, int bw, int th)
{
	static int  lastx = 0;
	static int  lasty = 0;
	static int  lastWidth = 0;
	static int  lastHeight = 0;
	static int  lastBW = 0;
	static int  lastTH = 0;
	int         xl, xr, yt, yb, xinnerl, xinnerr, yinnert, yinnerb;
	int         xthird, ythird;
	XSegment    outline[18];
	XSegment   *r;

	if(x == lastx && y == lasty && width == lastWidth && height == lastHeight
	                && lastBW == bw && th == lastTH) {
		return;
	}

	r = outline;

#define DRAWIT() \
    if (lastWidth || lastHeight)                        \
    {                                                   \
        xl = lastx;                                     \
        xr = lastx + lastWidth - 1;                     \
        yt = lasty;                                     \
        yb = lasty + lastHeight - 1;                    \
        xinnerl = xl + lastBW;                          \
        xinnerr = xr - lastBW;                          \
        yinnert = yt + lastTH + lastBW;                 \
        yinnerb = yb - lastBW;                          \
        xthird = (xinnerr - xinnerl) / 3;               \
        ythird = (yinnerb - yinnert) / 3;               \
                                                        \
        r->x1 = xl;                                     \
        r->y1 = yt;                                     \
        r->x2 = xr;                                     \
        r->y2 = yt;                                     \
        r++;                                            \
                                                        \
        r->x1 = xl;                                     \
        r->y1 = yb;                                     \
        r->x2 = xr;                                     \
        r->y2 = yb;                                     \
        r++;                                            \
                                                        \
        r->x1 = xl;                                     \
        r->y1 = yt;                                     \
        r->x2 = xl;                                     \
        r->y2 = yb;                                     \
        r++;                                            \
                                                        \
        r->x1 = xr;                                     \
        r->y1 = yt;                                     \
        r->x2 = xr;                                     \
        r->y2 = yb;                                     \
        r++;                                            \
                                                        \
        r->x1 = xinnerl + xthird;                       \
        r->y1 = yinnert;                                \
        r->x2 = r->x1;                                  \
        r->y2 = yinnerb;                                \
        r++;                                            \
                                                        \
        r->x1 = xinnerl + (2 * xthird);                 \
        r->y1 = yinnert;                                \
        r->x2 = r->x1;                                  \
        r->y2 = yinnerb;                                \
        r++;                                            \
                                                        \
        r->x1 = xinnerl;                                \
        r->y1 = yinnert + ythird;                       \
        r->x2 = xinnerr;                                \
        r->y2 = r->y1;                                  \
        r++;                                            \
                                                        \
        r->x1 = xinnerl;                                \
        r->y1 = yinnert + (2 * ythird);                 \
        r->x2 = xinnerr;                                \
        r->y2 = r->y1;                                  \
        r++;                                            \
                                                        \
        if (lastTH != 0) {                              \
            r->x1 = xl;                                 \
            r->y1 = yt + lastTH;                        \
            r->x2 = xr;                                 \
            r->y2 = r->y1;                              \
            r++;                                        \
        }                                               \
    }

	/* undraw the old one, if any */
	DRAWIT();

	lastx = x;
	lasty = y;
	lastWidth = width;
	lastHeight = height;
	lastBW = bw;
	lastTH = th;

	/* draw the new one, if any */
	DRAWIT();

#undef DRAWIT


	if(r != outline) {
		XDrawSegments(dpy, root, Scr->DrawGC, outline, r - outline);
	}
}
