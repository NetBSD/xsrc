/*
 * Various drawing routines.
 *
 * These are generalized routines that are used in multiple places in the
 * codebase.  A number of other functions that are "drawing" related are
 * left in various places through the codebase; some are only used in one
 * file, and are left there for internal linkage, while others are
 * part of a more abstract subdivision (e.g., the window decoration bits
 * in win_decorations.c) and so belong there.
 */

#include "ctwm.h"


#include "screen.h"
#include "gram.tab.h"
#include "occupation.h"
#include "vscreen.h"

#include "drawing.h"



/*
 * "3D border" drawings are used all over the place, not just in the
 * obvious usage as window borders.
 */
#define FBGC(gc, fix_fore, fix_back)\
    Gcv.foreground = fix_fore;\
    Gcv.background = fix_back;\
    XChangeGC(dpy, gc, GCForeground|GCBackground,&Gcv)

void
Draw3DBorder(Window w, int x, int y, int width, int height, int bw,
             ColorPair cp, ButtonState state, bool fill, bool forcebw)
{
	int           i;
	XGCValues     gcv;
	unsigned long gcm;

	if((width < 1) || (height < 1)) {
		return;
	}
	if(Scr->Monochrome != COLOR) {
		if(fill) {
			gcm = GCFillStyle;
			gcv.fill_style = FillOpaqueStippled;
			XChangeGC(dpy, Scr->BorderGC, gcm, &gcv);
			XFillRectangle(dpy, w, Scr->BorderGC, x, y, width, height);
		}
		gcm  = 0;
		gcm |= GCLineStyle;
		gcv.line_style = (state == on) ? LineSolid : LineDoubleDash;
		gcm |= GCFillStyle;
		gcv.fill_style = FillSolid;
		XChangeGC(dpy, Scr->BorderGC, gcm, &gcv);
		for(i = 0; i < bw; i++) {
			XDrawLine(dpy, w, Scr->BorderGC, x,                 y + i,
			          x + width - i - 1, y + i);
			XDrawLine(dpy, w, Scr->BorderGC, x + i,                  y,
			          x + i, y + height - i - 1);
		}

		gcm  = 0;
		gcm |= GCLineStyle;
		gcv.line_style = (state == on) ? LineDoubleDash : LineSolid;
		gcm |= GCFillStyle;
		gcv.fill_style = FillSolid;
		XChangeGC(dpy, Scr->BorderGC, gcm, &gcv);
		for(i = 0; i < bw; i++) {
			XDrawLine(dpy, w, Scr->BorderGC, x + width - i - 1,          y + i,
			          x + width - i - 1, y + height - 1);
			XDrawLine(dpy, w, Scr->BorderGC, x + i,         y + height - i - 1,
			          x + width - 1, y + height - i - 1);
		}
		return;
	}

	if(fill) {
		FBGC(Scr->BorderGC, cp.back, cp.fore);
		XFillRectangle(dpy, w, Scr->BorderGC, x, y, width, height);
	}
	if(Scr->BeNiceToColormap) {
		int dashoffset = 0;

		gcm  = 0;
		gcm |= GCLineStyle;
		gcv.line_style = (forcebw) ? LineSolid : LineDoubleDash;
		gcm |= GCBackground;
		gcv.background = cp.back;
		XChangeGC(dpy, Scr->BorderGC, gcm, &gcv);

		if(state == on) {
			XSetForeground(dpy, Scr->BorderGC, Scr->Black);
		}
		else {
			XSetForeground(dpy, Scr->BorderGC, Scr->White);
		}
		for(i = 0; i < bw; i++) {
			XDrawLine(dpy, w, Scr->BorderGC, x + i,     y + dashoffset,
			          x + i, y + height - i - 1);
			XDrawLine(dpy, w, Scr->BorderGC, x + dashoffset,    y + i,
			          x + width - i - 1, y + i);
			dashoffset = 1 - dashoffset;
		}
		XSetForeground(dpy, Scr->BorderGC, ((state == on) ? Scr->White : Scr->Black));
		for(i = 0; i < bw; i++) {
			XDrawLine(dpy, w, Scr->BorderGC, x + i,         y + height - i - 1,
			          x + width - 1, y + height - i - 1);
			XDrawLine(dpy, w, Scr->BorderGC, x + width - i - 1,          y + i,
			          x + width - i - 1, y + height - 1);
		}
		return;
	}
	if(state == on) {
		FBGC(Scr->BorderGC, cp.shadd, cp.shadc);
	}
	else             {
		FBGC(Scr->BorderGC, cp.shadc, cp.shadd);
	}
	for(i = 0; i < bw; i++) {
		XDrawLine(dpy, w, Scr->BorderGC, x,                 y + i,
		          x + width - i - 1, y + i);
		XDrawLine(dpy, w, Scr->BorderGC, x + i,                  y,
		          x + i, y + height - i - 1);
	}

	if(state == on) {
		FBGC(Scr->BorderGC, cp.shadc, cp.shadd);
	}
	else             {
		FBGC(Scr->BorderGC, cp.shadd, cp.shadc);
	}
	for(i = 0; i < bw; i++) {
		XDrawLine(dpy, w, Scr->BorderGC, x + width - i - 1,          y + i,
		          x + width - i - 1, y + height - 1);
		XDrawLine(dpy, w, Scr->BorderGC, x + i,         y + height - i - 1,
		          x + width - 1, y + height - i - 1);
	}
	return;
}

#undef FBGC



/*
 * Paint a button representing a workspace.  This is used in the
 * workspace manager window when it button mode, as well as in the
 * f.occupy window.
 */
void
PaintWsButton(PWBType which, VirtualScreen *vs, Window w,
              char *label, ColorPair cp, ButtonState state)
{
	int    bwidth, bheight;
	MyFont font;
	int    hspace, vspace;

	if(which == WSPCWINDOW) {
		bwidth  = vs->wsw->bwidth;
		bheight = vs->wsw->bheight;
		font    = Scr->workSpaceMgr.buttonFont;
	}
	else if(which == OCCUPYWINDOW) {
		OccupyWindow *occwin = Scr->workSpaceMgr.occupyWindow;
		bwidth  = occwin->bwidth;
		bheight = occwin->bheight;
		font    = occwin->font;
	}
	else if(which == OCCUPYBUTTON) {
		OccupyWindow *occwin = Scr->workSpaceMgr.occupyWindow;
		bwidth  = occwin->owidth;
		bheight = occwin->bheight;
		font    = occwin->font;
	}
	else {
		return;
	}

	{
		int        strWid, strHei;
		XRectangle inc_rect;
		XRectangle logical_rect;

		XmbTextExtents(font.font_set, label, strlen(label), &inc_rect,
		               &logical_rect);
		strHei = logical_rect.height;
		vspace = ((bheight + strHei - font.descent) / 2);
		strWid = logical_rect.width;
		hspace = (bwidth - strWid) / 2;
	}

	if(hspace < (Scr->WMgrButtonShadowDepth + 1)) {
		hspace = Scr->WMgrButtonShadowDepth + 1;
	}
	XClearWindow(dpy, w);

	if(Scr->Monochrome == COLOR) {
		Draw3DBorder(w, 0, 0, bwidth, bheight, Scr->WMgrButtonShadowDepth,
		             cp, state, true, false);

		switch(Scr->workSpaceMgr.buttonStyle) {
			case STYLE_NORMAL:
				break;

			case STYLE_STYLE1:
				Draw3DBorder(w,
				             Scr->WMgrButtonShadowDepth - 1,
				             Scr->WMgrButtonShadowDepth - 1,
				             bwidth  - 2 * Scr->WMgrButtonShadowDepth + 2,
				             bheight - 2 * Scr->WMgrButtonShadowDepth + 2,
				             1, cp, (state == on) ? off : on, true, false);
				break;

			case STYLE_STYLE2:
				Draw3DBorder(w,
				             Scr->WMgrButtonShadowDepth / 2,
				             Scr->WMgrButtonShadowDepth / 2,
				             bwidth  - Scr->WMgrButtonShadowDepth,
				             bheight - Scr->WMgrButtonShadowDepth,
				             1, cp, (state == on) ? off : on, true, false);
				break;

			case STYLE_STYLE3:
				Draw3DBorder(w,
				             1,
				             1,
				             bwidth  - 2,
				             bheight - 2,
				             1, cp, (state == on) ? off : on, true, false);
				break;
		}
		FB(cp.fore, cp.back);
		XmbDrawString(dpy, w, font.font_set, Scr->NormalGC, hspace, vspace,
		              label, strlen(label));
	}
	else {
		Draw3DBorder(w, 0, 0, bwidth, bheight, Scr->WMgrButtonShadowDepth,
		             cp, state, true, false);
		if(state == on) {
			FB(cp.fore, cp.back);
			XmbDrawImageString(dpy, w, font.font_set, Scr->NormalGC, hspace, vspace,
			                   label, strlen(label));
		}
		else {
			FB(cp.back, cp.fore);
			XmbDrawImageString(dpy, w, font.font_set, Scr->NormalGC, hspace, vspace,
			                   label, strlen(label));
		}
	}
}
