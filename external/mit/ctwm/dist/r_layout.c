/*
 * Copyright notice...
 */

#include "ctwm.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "r_layout.h"
#include "r_area_list.h"
#include "r_area.h"
#include "util.h"


/*
 * Prototype internal funcs
 */
static void _RLayoutFreeNames(RLayout *self);
static RAreaList *_RLayoutRecenterVertically(const RLayout *self,
                const RArea *far_area);
static RAreaList *_RLayoutRecenterHorizontally(const RLayout *self,
                const RArea *far_area);
static RAreaList *_RLayoutVerticalIntersect(const RLayout *self,
                const RArea *area);
static RAreaList *_RLayoutHorizontalIntersect(const RLayout *self,
                const RArea *area);

/* Foreach() callbacks used in various lookups */
static bool _findMonitorByXY(const RArea *cur, void *vdata);
static bool _findMonitorBottomEdge(const RArea *cur, void *vdata);
static bool _findMonitorTopEdge(const RArea *cur, void *vdata);
static bool _findMonitorLeftEdge(const RArea *cur, void *vdata);
static bool _findMonitorRightEdge(const RArea *cur, void *vdata);




/************************
 *
 * First, some funcs for creating and destroying RLayout's in various
 * ways.
 *
 ************************/


/**
 * Create an RLayout for a given set of monitors.
 *
 * This stashes up the list of monitors, and precalculates the
 * horizontal/vertical stripes that compose it.
 */
RLayout *
RLayoutNew(RAreaList *monitors)
{
	RLayout *layout = malloc(sizeof(RLayout));
	if(layout == NULL) {
		abort();
	}

	layout->monitors = monitors;
	layout->horiz = RAreaListHorizontalUnion(monitors);
	layout->vert = RAreaListVerticalUnion(monitors);
	layout->names = NULL;

	return layout;
}


/**
 * Create a copy of an RLayout with given amounts cropped off the sides.
 * This is used anywhere we need to pretend our display area is smaller
 * than it actually is (e.g., via the BorderBottom/Top/Left/Right config
 * params)
 */
RLayout *
RLayoutCopyCropped(const RLayout *self, int left_margin, int right_margin,
                   int top_margin, int bottom_margin)
{
	RAreaList *cropped_monitors = RAreaListCopyCropped(self->monitors,
	                              left_margin, right_margin,
	                              top_margin, bottom_margin);
	if(cropped_monitors == NULL) {
		return NULL;        // nothing to crop, same layout as passed
	}

	return RLayoutNew(cropped_monitors);
}


/**
 * Clean up and free any RLayout.names there might be in an RLayout.
 */
static void
_RLayoutFreeNames(RLayout *self)
{
	if(self == NULL) {
		return;
	}
	if(self->names != NULL) {
		free(self->names);
		self->names = NULL;
	}
}


/**
 * Clean up and free an RLayout.
 */
void
RLayoutFree(RLayout *self)
{
	if(self == NULL) {
		return;
	}

	RAreaListFree(self->monitors);
	RAreaListFree(self->horiz);
	RAreaListFree(self->vert);
	_RLayoutFreeNames(self);
	free(self);
}


/**
 * Set the names for our monitors in an RLayout.  This is only used for
 * the RLayout that describes our complete monitor layout, which fills in
 * the RANDR names for each output.
 */
RLayout *
RLayoutSetMonitorsNames(RLayout *self, char **names)
{
	_RLayoutFreeNames(self);
	self->names = names;
	return self;
}



/************************
 *
 * Next, a few util funcs for dealing with RArea's that are outside our
 * RLayout, but we want to find the nearest way to move them inside, then
 * return a list of which RArea's they'd be intersecting with.
 *
 ************************/


/**
 * Given an RArea that doesn't reside in any of the areas in our RLayout,
 * create a list of maximally-tall RArea slices out of our layout where
 * it would wind up if we brought it onto the nearest screen edge.  This
 * yields a RAreaList as tall as the slice[es] the window would touch if
 * we moved it in.
 *
 * If we had the move the window horizontally (it was off-screen to the
 * right or left), it results in a 1-pixel-wide slice of the right- or
 * left-most self->vert.
 *
 * If we had to move it vertically (it was off to the top or bottom), it
 * winds up being whatever horizontal intersection with self->vert would
 * result from the window's x and width, with the full height of the
 * involved slices.
 *
 * This is the vertical-stripe-returning counterpart of
 * _RLayoutRecenterHorizontally().
 *
 * This is called only by \_RLayoutVerticalIntersect() when given an RArea
 * that doesn't already intersect the RLayout.  Will probably not tell
 * you something useful if given a far_area that already _does_ intersect
 * self.
 *
 * \param self     Our current monitor layout
 * \param far_area The area to act on
 */
static RAreaList *
_RLayoutRecenterVertically(const RLayout *self, const RArea *far_area)
{
	RArea big = RAreaListBigArea(self->monitors), tmp;

	// We assume far_area is outside of self.  So it's in one of the
	// three labelled areas:
	//
	//  |_V_|   ___ tmp.top
	//  |   |     |
	// L|   |R    |
	//  |___|   ___ tmp.bottom
	//  | V |
	//
	// So we'll create an RArea that's the y and height of big (a giant
	// rectangle covering all the monitors), so that its intersection
	// with self->vert will always cover a full vertical stripe.  Then
	// we'll set its x/width so that it's shifted to be at least
	// minimally inside big somehow.

	// Where did it wind up?
	if((far_area->x >= big.x && far_area->x <= RAreaX2(&big))
	                || (RAreaX2(far_area) >= big.x && RAreaX2(far_area) <= RAreaX2(&big))) {
		// In one of the V areas.  It's already in a horizontal position
		// that would fit, so we just keep x/width.
		tmp = RAreaNew(far_area->x, big.y,
		               far_area->width, big.height);
	}
	else if(RAreaX2(far_area) < big.x) {
		// Off to the left side in L, so move it over just far enough
		// that 1 pixel of it protrudes into the left side.
		tmp = RAreaNew(big.x - far_area->width + 1, big.y,
		               far_area->width, big.height);
	}
	else {
		// Off to the right side in R, so move it over just far enough
		// that 1 pixel of it protrudes into the right side.
		tmp = RAreaNew(RAreaX2(&big), big.y,
		               far_area->width, big.height);
	}

	// Then intersect that (full height, at least 1 pixel horizontally
	// somewhere) with our collection of vertical stripes, to yield an
	// answer.  If the window was off to the left or right, this will
	// yield a 1-pixel-wide slice of either the left- or right-most
	// ->vert of our layout.  If it were off the top of bottom, though,
	// it'll yield some slice of 1 (or more) of our ->vert's, as wide as
	// the window itself was.
	return RAreaListIntersect(self->vert, &tmp);

	// n.b.; _RLayoutRecenterHorizontally() is the counterpart to this
	// with horizontal slices.  The comments in the two have been written
	// independently with somewhat different explanatory styles, so if
	// the description here was confusing, try reading the other one and
	// transposing.
}


/**
 * Given an RArea that doesn't reside in any of the areas in our RLayout,
 * create a list of maximally-wide RArea slices out of our layout where
 * it would wind up if we brought it onto the nearest screen edge.  This
 * yields a RAreaList as wide as the slice[es] the window would touch if
 * we moved it in.
 *
 * If we had the move the window vertically (it was off-screen to the top
 * or bottom), it results in a 1-pixel-wide slice of the top- or
 * bottom-most self->horiz.
 *
 * If we had to move it horizontally (it was off to the left or right),
 * it winds up being whatever vertical intersection with self->horiz
 * would result from the window's y and height, with the full width of
 * the involved slices.
 *
 * This is the horizontal-stripe-returning counterpart of
 * _RLayoutRecenterVertically().
 *
 * This is called only by \_RLayoutVerticalIntersect() when given an RArea
 * that doesn't already intersect the RLayout.  Will probably not tell
 * you something useful if given a far_area that already _does_ intersect
 * self.
 *
 * \param self     Our current monitor layout
 * \param far_area The area to act on
 */
static RAreaList *
_RLayoutRecenterHorizontally(const RLayout *self, const RArea *far_area)
{
	RArea big = RAreaListBigArea(self->monitors), tmp;

	// far_area is outside self, so it's in one of the 3 labelled areas:
	//
	// ___T___
	//  |   |
	// H|   |H
	// _|___|_
	//    B
	//
	// We create an RArea that's the x and width of big, so it always
	// covers the entire width of any member of ->horiz.  Then we move
	// the far_area in to the nearest edge to figure the y/height to set.

	if((far_area->y >= big.y && far_area->y <= RAreaY2(&big))
	                || (RAreaY2(far_area) >= big.y && RAreaY2(far_area) <= RAreaY2(&big))) {
		// In one of the H areas.  Already in a valid place vertically,
		// so make a horizontal strip that position/tall.
		tmp = RAreaNew(big.x, far_area->y,
		               big.width, far_area->height);
	}
	else if(RAreaY2(far_area) < big.y) {
		// Off the top (T); move it down just far enough that it's bottom
		// protrudes 1 pixel into the top.
		tmp = RAreaNew(big.x, big.y - far_area->height + 1,
		               big.width, far_area->height);
	}
	else {
		// Off the bottom (B); move it up just enough that it's top
		// protrudes 1 pixel into the bottom.
		tmp = RAreaNew(big.x, RAreaY2(&big),
		               big.width, far_area->height);
	}

	// And intersect that RArea with self->horiz.  This results in a
	// full-width overlap with 1 pixel at the bottom of the bottom-most,
	// 1 pixel at the top of the top-most, or 1..(far_area->height)
	// overlap somewhere.  In that last case (far_area was in H), the
	// intersection may yield multiple areas.
	return RAreaListIntersect(self->horiz, &tmp);

	// n.b.; _RLayoutRecenterVertically() is the counterpart to this with
	// vertical slices.  The comments in the two have been written
	// independently with somewhat different explanatory styles, so if
	// the description here was confusing, try reading the other one and
	// transposing.
}



/************************
 *
 * Some wrappers called when we need to Insersect an RArea with our
 * RLayout, but also handle the case (using the above funcs) when the
 * RArea doesn't Intersect our layout by finding the nearest border we
 * could shuffle it over.
 *
 ************************/


/**
 * Find which vertical regions of our monitor layout a given RArea (often
 * a window) is in.  If it's completely off the screen, we move it until
 * it's just over the nearest edge, and return the vertical stripe(s) it
 * would be in then.
 *
 * This function is used only by RLayoutFindTopBottomEdges()
 */
static RAreaList *
_RLayoutVerticalIntersect(const RLayout *self, const RArea *area)
{
	RAreaList *mit = RAreaListIntersect(self->vert, area);

	if(mit->len == 0) {
		// Not on screen.  Move it to just over the nearest edge so it
		// is, and give the slices it's in then.
		RAreaListFree(mit);
		mit = _RLayoutRecenterVertically(self, area);
	}
	return mit;
}


/**
 * Find which horizontal regions of our monitor layout a given RArea
 * (often a window) is in.  If it's completely off the screen, we move it
 * until it's just over the nearest edge, and return the horizontal
 * stripe(s) it would be in then.
 *
 * This function is used only by RLayoutFindLeftRightEdges()
 */
static RAreaList *
_RLayoutHorizontalIntersect(const RLayout *self, const RArea *area)
{
	RAreaList *mit = RAreaListIntersect(self->horiz, area);

	if(mit->len == 0) {
		// Not on screen.  Move it to just over the nearest edge so it
		// is, and give the slices it's in then.
		RAreaListFree(mit);
		mit = _RLayoutRecenterHorizontally(self, area);
	}

	return mit;
}



/************************
 *
 * Some funcs using the above (layers of) utils to find info about which
 * stripes of the RLayout an Area appears in.   These are used mostly as
 * backend utils for figuring various f.*zoom's.
 *
 ************************/


/**
 * Figure the position (or nearest practical position) of an area in our
 * screen layout, and return info about the bottom/top stripes it fits
 * into.
 *
 * Note that the return values (params) are slightly counterintuitive;
 * top tells you where the top of the lowest stripe that area intersects
 * with is, and bottom tells you the bottom of the highest.
 *
 * This is used as a backend piece of various calculations trying to be
 * sure something winds up on-screen and when figuring out how to zoom
 * it.
 *
 * \param[in]  self   The monitor layout to work from
 * \param[in]  area   The area to be fit into the monitors
 * \param[out] top    The top of the lowest stripe area fits into.
 * \param[out] bottom The bottom of the highest stripe area fits into.
 */
void
RLayoutFindTopBottomEdges(const RLayout *self, const RArea *area, int *top,
                          int *bottom)
{
	RAreaList *mit = _RLayoutVerticalIntersect(self, area);

	if(top != NULL) {
		*top = RAreaListMaxY(mit);
	}

	if(bottom != NULL) {
		*bottom = RAreaListMinY2(mit);
	}

	RAreaListFree(mit);
}


/**
 * Find the bottom of the top stripe of self that area fits into.  A
 * shortcut to get only the second return value of
 * RLayoutFindTopBottomEdges().
 */
int
RLayoutFindBottomEdge(const RLayout *self, const RArea *area)
{
	int min_y2;
	RLayoutFindTopBottomEdges(self, area, NULL, &min_y2);
	return min_y2;
}


/**
 * Find the top of the bottom stripe of self that area fits into.  A
 * shortcut to get only the first return value of
 * RLayoutFindTopBottomEdges().
 */
int
RLayoutFindTopEdge(const RLayout *self, const RArea *area)
{
	int max_y;
	RLayoutFindTopBottomEdges(self, area, &max_y, NULL);
	return max_y;
}


/**
 * Figure the position (or nearest practical position) of an area in our
 * screen layout, and return info about the left/rightmost stripes it fits
 * into.
 *
 * As with RLayoutFindTopBottomEdges(), the return values (params) are
 * slightly counterintuitive.  left tells you where the left-side of the
 * right-most stripe that area intersects with is, and right tells you
 * the right side of the left-most.
 *
 * This is used as a backend piece of various calculations trying to be
 * sure something winds up on-screen and when figuring out how to zoom
 * it.
 *
 * \param[in]  self   The monitor layout to work from
 * \param[in]  area   The area to be fit into the monitors
 * \param[out] left   The left edge of the right-most stripe area fits into.
 * \param[out] right  The right edge of the left-most stripe area fits into.
 */
void
RLayoutFindLeftRightEdges(const RLayout *self, const RArea *area, int *left,
                          int *right)
{
	RAreaList *mit = _RLayoutHorizontalIntersect(self, area);

	if(left != NULL) {
		*left = RAreaListMaxX(mit);
	}

	if(right != NULL) {
		*right = RAreaListMinX2(mit);
	}

	RAreaListFree(mit);
}


/**
 * Find the left edge of the right-most stripe of self that area fits
 * into.  A shortcut to get only the first return value of
 * RLayoutFindLeftRightEdges().
 */
int
RLayoutFindLeftEdge(const RLayout *self, const RArea *area)
{
	int max_x;
	RLayoutFindLeftRightEdges(self, area, &max_x, NULL);
	return max_x;
}


/**
 * Find the right edge of the left-most stripe of self that area fits
 * into.  A shortcut to get only the second return value of
 * RLayoutFindLeftRightEdges().
 */
int
RLayoutFindRightEdge(const RLayout *self, const RArea *area)
{
	int min_x2;
	RLayoutFindLeftRightEdges(self, area, NULL, &min_x2);
	return min_x2;
}



/************************
 *
 * Lookups to find areas in an RLayout by various means.
 *
 ************************/


/// Internal structure for callback in RLayoutGetAreaAtXY().
struct monitor_finder_xy {
	const RArea *area;
	int x, y;
};

/// Callback util for RLayoutGetAreaAtXY().
static bool
_findMonitorByXY(const RArea *cur, void *vdata)
{
	struct monitor_finder_xy *data = (struct monitor_finder_xy *)vdata;

	if(RAreaContainsXY(cur, data->x, data->y)) {
		data->area = cur;
		return true;
	}
	return false;
}

/**
 * Find the RArea in a RLayout that a given coordinate falls into.  In
 * practice, the RArea's in self are the monitors of the desktop, so this
 * answers "Which monitor is this position on?"
 */
RArea
RLayoutGetAreaAtXY(const RLayout *self, int x, int y)
{
	struct monitor_finder_xy data = { .area = NULL, .x = x, .y = y };

	RAreaListForeach(self->monitors, _findMonitorByXY, &data);

	return data.area == NULL ? self->monitors->areas[0] : *data.area;
}


/**
 * Return the index'th RArea in an RLayout, or RAreaInvalid() with an out
 * of range index.
 */
RArea
RLayoutGetAreaIndex(const RLayout *self, int index)
{
	if(index >= self->monitors->len || index < 0) {
		return RAreaInvalid();
	}

	return self->monitors->areas[index];
}


/**
 * Return the RArea in self with the name given by the string of length
 * len at name.  This is only used in RLayoutXParseGeometry() to parse a
 * fragment of a larger string, hence the need for len.  It's used to
 * find the monitor with a given name (RANDR output name).
 */
RArea
RLayoutGetAreaByName(const RLayout *self, const char *name, int len)
{
	if(self->names != NULL) {
		if(len < 0) {
			len = strlen(name);
		}

		for(int i = 0; i < self->monitors->len
		                && self->names[i] != NULL; i++) {
			if(strncmp(self->names[i], name, len) == 0) {
				return self->monitors->areas[i];
			}
		}
	}

	return RAreaInvalid();
}



/************************
 *
 * Now some utils for finding various edges of the monitors a given RArea
 * intersects with.
 *
 ************************/


/// Internal struct for use in FindMonitor*Edge() callbacks.
struct monitor_edge_finder {
	const RArea *area;
	union {
		int max_x;
		int max_y;
		int min_x2;
		int min_y2;
	} u;
	bool found;
};

/// Callback util for RLayoutFindMonitorBottomEdge()
static bool
_findMonitorBottomEdge(const RArea *cur, void *vdata)
{
	struct monitor_edge_finder *data = (struct monitor_edge_finder *)vdata;

	// Does the area we're looking for intersect this piece of the
	// RLayout, is the bottom of the area shown on it, and is the bottom
	// of this piece the highest we've yet found that satisfies those
	// conditions?
	if(RAreaIsIntersect(cur, data->area)
	                && RAreaY2(cur) > RAreaY2(data->area)
	                && (!data->found || RAreaY2(cur) < data->u.min_y2)) {
		data->u.min_y2 = RAreaY2(cur);
		data->found = true;
	}
	return false;
}

/**
 * Find the bottom edge of the top-most monitor that contains the most of
 * a given RArea.  Generally, the area would be a window.
 *
 * That is, we find the monitor whose bottom is the highest up, but that
 * still shows the bottom edge of the window, and return that monitor's
 * bottom.  If the bottom of the window is off all the monitors, that's
 * just the highest-ending monitor that contains the window.
 */
int
RLayoutFindMonitorBottomEdge(const RLayout *self, const RArea *area)
{
	struct monitor_edge_finder data = { .area = area };

	RAreaListForeach(self->monitors, _findMonitorBottomEdge, &data);

	return data.found ? data.u.min_y2 : RLayoutFindBottomEdge(self, area);
}


/// Callback util for RLayoutFindMonitorTopEdge()
static bool
_findMonitorTopEdge(const RArea *cur, void *vdata)
{
	struct monitor_edge_finder *data = (struct monitor_edge_finder *)vdata;

	// Does the area we're looking for intersect this piece of the
	// RLayout, is the top of the area shown on it, and is the top
	// of this piece the lowest we've yet found that satisfies those
	// conditions?
	if(RAreaIsIntersect(cur, data->area)
	                && cur->y < data->area->y
	                && (!data->found || cur->y > data->u.max_y)) {
		data->u.max_y = cur->y;
		data->found = true;
	}
	return false;
}

/**
 * Find the top edge of the bottom-most monitor that contains the most of
 * a given RArea.  Generally, the area would be a window.
 *
 * That is, we find the monitor whose top is the lowest down, but that
 * still shows the top edge of the window, and return that monitor's top.
 * If the top of the window is off all the monitors, that's just the
 * lowest-ending monitor that contains part of the window.
 */
int
RLayoutFindMonitorTopEdge(const RLayout *self, const RArea *area)
{
	struct monitor_edge_finder data = { .area = area };

	RAreaListForeach(self->monitors, _findMonitorTopEdge, &data);

	return data.found ? data.u.max_y : RLayoutFindTopEdge(self, area);
}


/// Callback util for RLayoutFindMonitorLeftEdge()
static bool
_findMonitorLeftEdge(const RArea *cur, void *vdata)
{
	struct monitor_edge_finder *data = (struct monitor_edge_finder *)vdata;

	// Does the area we're looking for intersect this piece of the
	// RLayout, is the left of the area shown on it, and is the left of
	// this piece the right-most we've yet found that satisfies those
	// conditions?
	if(RAreaIsIntersect(cur, data->area)
	                && cur->x < data->area->x
	                && (!data->found || cur->x > data->u.max_x)) {
		data->u.max_x = cur->x;
		data->found = true;
	}
	return false;
}

/**
 * Find the left edge of the right-most monitor that contains the most of
 * a given RArea.  Generally, the area would be a window.
 *
 * That is, we find the monitor whose left is the furthest right, but
 * that still shows the left edge of the window, and return that
 * monitor's left.  If the left edge of the window is off all the
 * monitors, that's just the right-most-ending monitor that contains the
 * window.
 */
int
RLayoutFindMonitorLeftEdge(const RLayout *self, const RArea *area)
{
	struct monitor_edge_finder data = { .area = area };

	RAreaListForeach(self->monitors, _findMonitorLeftEdge, &data);

	return data.found ? data.u.max_x : RLayoutFindLeftEdge(self, area);
}


/// Callback util for RLayoutFindMonitorRightEdge()
static bool
_findMonitorRightEdge(const RArea *cur, void *vdata)
{
	struct monitor_edge_finder *data = (struct monitor_edge_finder *)vdata;

	// Does the area we're looking for intersect this piece of the
	// RLayout, is the right of the area shown on it, and is the right of
	// this piece the left-most we've yet found that satisfies those
	// conditions?
	if(RAreaIsIntersect(cur, data->area)
	                && RAreaX2(cur) > RAreaX2(data->area)
	                && (!data->found || RAreaX2(cur) < data->u.min_x2)) {
		data->u.min_x2 = RAreaX2(cur);
		data->found = true;
	}
	return false;
}

/**
 * Find the right edge of the left-most monitor that contains the most of
 * a given RArea.  Generally, the area would be a window.
 *
 * That is, we find the monitor whose right is the furthest left, but
 * that still shows the right edge of the window, and return that
 * monitor's right.  If the right edge of the window is off all the
 * monitors, that's just the left-most-ending monitor that contains the
 * window.
 */
int
RLayoutFindMonitorRightEdge(const RLayout *self, const RArea *area)
{
	struct monitor_edge_finder data = { .area = area };

	RAreaListForeach(self->monitors, _findMonitorRightEdge, &data);

	return data.found ? data.u.min_x2 : RLayoutFindRightEdge(self, area);
}



/************************
 *
 * Backend funcs called by the f.*zoom handlers to figure the area we
 * should zoom into.
 *
 ************************/


/**
 * Figure the best way to stretch an area across the full horizontal
 * width of an RLayout.  This is the backend for the f.xhorizoom ctwm
 * function, zooming a window to the full width of all monitors.
 */
RArea
RLayoutFullHoriz(const RLayout *self, const RArea *area)
{
	int max_x, min_x2;

	RLayoutFindLeftRightEdges(self, area, &max_x, &min_x2);

	return RAreaNew(max_x, area->y, min_x2 - max_x + 1, area->height);

	/**
	 * This yields an area:
	 * ~~~
	 * TL   W
	 *   *-----*
	 *   |     |
	 *  H|     |
	 *   |     |
	 *   *-----*
	 * ~~~
	 *
	 * The precise construction of the area can be tricky.
	 *
	 * In the simplest case, the area is entirely in one horizontal
	 * stripe to start with.  In that case, max_x is the left side of
	 * that box, min_x2 is the right side, so the resulting area starts
	 * at (left margin, area y), with the height of y and the width of
	 * the whole stripe.  Easy.
	 *
	 * When it spans multiple, it's more convoluted.  Let's consider an
	 * example layout (of horizontal stripes, so that top stripe may be
	 * across 2 monitors) to make it a little clearer:
	 *
	 * ~~~
	 * *--------------------------*
	 * |             |......2.....|
	 * |                          |  <-----.
	 * |             1 =========  |         .
	 * *-------------*-=========--*-*        >-- 2 horiz stripes
	 *               | =========    |       '
	 *               |  /           |  <---'
	 *       area  --+-'            |
	 *               *--------------*
	 * ~~~
	 *
	 * So in this case, we're trying to stretch area out as far
	 * horizontal as it can go, crossing monitors if possible.
	 *
	 * So, the top-left corner of our box (TL) has the X coordinate of
	 * the right-most strip we started with (the lower), and the Y
	 * coordinate of the top of the area, yielding point (1) above (not
	 * the asterisk; specifically where (1) sits).
	 *
	 * The width W is the difference between the right of the
	 * left-most-ending (in this case, the top) stripe, and the left of
	 * the right-most-starting (the bottom) (plus 1 because math).
	 * That's the width of the intersecting horizontal area (2) above.
	 *
	 * And the height H is just the height of the original area.  And so,
	 * our resulting area is the height of that original area (in ='s),
	 * and stretched to the left and right until it runs into one or the
	 * other monitor edge (1 space to the left, 2 to the right, in our
	 * diagram).
	 */
}


/**
 * Figure the best way to stretch an area across the full vertical height
 * of an RLayout.  This is the backend for the f.xzoom ctwm function,
 * zooming a window to the full height of all monitors.
 */
RArea
RLayoutFullVert(const RLayout *self, const RArea *area)
{
	int max_y, min_y2;

	RLayoutFindTopBottomEdges(self, area, &max_y, &min_y2);

	return RAreaNew(area->x, max_y, area->width, min_y2 - max_y + 1);

	// X-ref long comment above in RLayoutFullHoriz() for worked example.
	// This is just rotated 90 degrees, but the logic works out about the
	// same.
}


/**
 * Figure the best way to stretch an area across the largest horizontal
 * and vertical space it can from its current position.  Essentially,
 * stretch it in all directions until it hits the edge of our available
 * space.
 *
 * This is the backend for the f.xfullzoom function.
 */
RArea
RLayoutFull(const RLayout *self, const RArea *area)
{
	RArea full_horiz, full_vert, full1, full2;

	// Get the boxes for full horizontal and vertical zooms, using the
	// above functions.
	full_horiz = RLayoutFullHoriz(self, area);
	full_vert = RLayoutFullVert(self, area);

	// Now stretch each of those in the other direction...
	full1 = RLayoutFullVert(self, &full_horiz);
	full2 = RLayoutFullHoriz(self, &full_vert);

	// And return whichever was bigger.
	return RAreaArea(&full1) > RAreaArea(&full2) ? full1 : full2;
}



/**
 * Figure the best way to stretch an area horizontally without crossing
 * monitors.
 *
 * This is the backend for the f.horizoom ctwm function.
 */
RArea
RLayoutFullHoriz1(const RLayout *self, const RArea *area)
{
	// Cheat by using RLayoutFull1() to find the RArea for the monitor
	// it's most on.
	RArea target = RLayoutFull1(self, area);
	int max_y, min_y2;

	// We're stretching horizontally, so the x and width of target (that
	// monitor) are already right.  But we have to figure the y and
	// height...

	// Generally, the y is the window's original y, unless we had to move
	// it down to get onto the target monitor.  XXX Wait, what if we
	// moved it _up_?
	max_y = max(area->y, target.y);
	target.y = max_y;

	// The bottom would be the bottom of the area, clipped to the bottom
	// of the monitor.  So the height is the diff.
	min_y2 = min(RAreaY2(area), RAreaY2(&target));
	target.height = min_y2 - max_y + 1;

	return target;
}


/**
 * Figure the best way to stretch an area vertically without crossing
 * monitors.
 *
 * This is the backend for the f.zoom ctwm function.
 */
RArea
RLayoutFullVert1(const RLayout *self, const RArea *area)
{
	// Let RLayoutFull1() find the right monitor.
	RArea target = RLayoutFull1(self, area);
	int max_x, min_x2;

	// Stretching vertically, so the y/height of the monitor are already
	// right.

	// x is where the window was, unless we had to move it right to get
	// onto the monitor.  XXX What if we moved it left?
	max_x = max(area->x, target.x);
	target.x = max_x;

	// Right side is where it was, unless we have to clip to the monitor.
	min_x2 = min(RAreaX2(area), RAreaX2(&target));
	target.width = min_x2 - max_x + 1;

	return target;
}


/**
 * Figure the best way to resize an area to fill one monitor.
 *
 * This is the backend for the f.fullzoom ctwm function.
 *
 * \param self  Monitor layout
 * \param area  Area (window) to zoom out
 */
RArea
RLayoutFull1(const RLayout *self, const RArea *area)
{
	RArea target;
	RAreaList *mit = RAreaListIntersect(self->monitors, area);
	// Start with a list of all the monitors the window is on now.

	if(mit->len == 0) {
		// Not on any screens.  Find the "nearest" place it would wind
		// up.
		RAreaListFree(mit);
		mit = _RLayoutRecenterHorizontally(self, area);
	}

	// Of the monitors it's on, find the one that it's "most" on, and
	// return the RArea of it.
	target = RAreaListBestTarget(mit, area);
	RAreaListFree(mit);
	return target;
}



/************************
 *
 * Finally, some small misc utils.
 *
 ************************/


/**
 * Generate maximal spanning RArea.
 *
 * This is a trivial wrapper of RAreaListBigArea() to hide knowledge of
 * RLayout internals.  Currently only used once; maybe should just be
 * deref'd there...
 */
RArea
RLayoutBigArea(const RLayout *self)
{
	return RAreaListBigArea(self->monitors);
}


/**
 * How many monitors does a given RLayout contain?
 */
int
RLayoutNumMonitors(const RLayout *self)
{
	return self->monitors->len;
}


/**
 * Pretty-print an RLayout.
 *
 * Used for dev/debug.
 */
void
RLayoutPrint(const RLayout *self)
{
	fprintf(stderr, "[monitors=");
	RAreaListPrint(self->monitors);
	fprintf(stderr, "\n horiz=");
	RAreaListPrint(self->horiz);
	fprintf(stderr, "\n vert=");
	RAreaListPrint(self->vert);
	fprintf(stderr, "]\n");
}
