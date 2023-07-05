/*
 * Copyright notice...
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "r_area_list.h"
#include "r_area.h"


/*
 * Prototype internal funcs
 */
static RAreaList *RAreaListCopy(const RAreaList *self);
static void RAreaListDelete(RAreaList *self, int index);
static void RAreaListAddList(RAreaList *self, const RAreaList *other);
static RAreaList *RAreaListIntersectCrop(const RAreaList *self,
                const RArea *area);

/* Sorts and their internal comparison routines */
static int _cmpX(const void *av, const void *bv);
static void RAreaListSortX(const RAreaList *self);
static int _cmpY(const void *av, const void *bv);
static void RAreaListSortY(const RAreaList *self);



/**
 * Create an RAreaList from a set of RArea's.
 * \param cap Hint as to the number of RArea's being passed
 * \param ... Sequence of RArea * to put in it.  Don't forget a trailing
 *            NULL.
 */
RAreaList *
RAreaListNew(int cap, ...)
{
	va_list ap;
	RAreaList *list;
	RArea *area;

	list = malloc(sizeof(RAreaList));
	if(list == NULL) {
		abort();
	}
	list->len = 0;

	if(cap <= 0) {
		cap = 1;
	}
	list->cap = cap;
	list->areas = malloc(cap * sizeof(RArea));
	if(list->areas == NULL) {
		abort();
	}

	va_start(ap, cap);

	while((area = va_arg(ap, RArea *)) != NULL) {
		RAreaListAdd(list, area);
	}

	va_end(ap);

	return list;
}


/**
 * Create a copy of a given RAreaList.
 */
static RAreaList *
RAreaListCopy(const RAreaList *self)
{
	RAreaList *new = RAreaListNew(self->cap, NULL);

	RAreaListAddList(new, self);

	return new;
}


/**
 * Create a copy of an RAreaList with given amounts cropped off the
 * sides.  This is used principally during startup, to handle the
 * BorderBottom/Top/Left/Right config params.
 */
RAreaList *
RAreaListCopyCropped(const RAreaList *self, int left_margin,
                     int right_margin,
                     int top_margin, int bottom_margin)
{
	if(left_margin > 0 || right_margin > 0
	                || top_margin > 0 || bottom_margin > 0) {
		// Start with a big spanning square
		RArea big_area = RAreaListBigArea(self);

		// Guard against negative margins
		if(left_margin < 0) {
			left_margin = 0;
		}
		if(right_margin < 0) {
			right_margin = 0;
		}
		if(top_margin < 0) {
			top_margin = 0;
		}
		if(bottom_margin < 0) {
			bottom_margin = 0;
		}

		// Squeeze down the big square by the asked for amounts
		big_area.x += left_margin;
		big_area.width -= left_margin + right_margin;
		big_area.y += top_margin;
		big_area.height -= top_margin + bottom_margin;

		// If we cropped down to nothing, that's a RAreaList with nothing
		// in it, so give back that.
		if(big_area.width <= 0 || big_area.height <= 0) {
			return RAreaListNew(0, NULL); // empty list
		}

		// Make a new RAreaList cropped down to that size.
		return RAreaListIntersectCrop(self, &big_area);
	}

	// Nothing to do; our callers expect getting nothing back.
	return NULL;
}


/**
 * Clean up and free an RAreaList.
 */
void
RAreaListFree(RAreaList *self)
{
	if(self == NULL) {
		return;
	}
	free(self->areas);
	free(self);
}


/**
 * Delete an RArea from inside an RAreaList.
 */
static void
RAreaListDelete(RAreaList *self, int index)
{
	if(index >= self->len) {
		return;
	}

	self->len--;

	if(index == self->len) {
		return;
	}

	memcpy(&self->areas[index], &self->areas[index + 1],
	       (self->len - index) * sizeof(RArea));
}


/**
 * Add an RArea onto an RAreaList.
 */
void
RAreaListAdd(RAreaList *self, const RArea *area)
{
	if(self->cap == self->len) {
		RArea *new_list = realloc(self->areas, (self->cap + 1) * sizeof(RArea));
		if(new_list == NULL) {
			abort();
		}

		self->cap++;
		self->areas = new_list;
	}

	self->areas[self->len++] = *area;
}


/**
 * Add the RArea's from one RAreaList onto another.
 */
static void
RAreaListAddList(RAreaList *self, const RAreaList *other)
{
	if(self->cap - self->len < other->len) {
		RArea *new_list = realloc(self->areas,
		                          (self->len + other->len) * sizeof(RArea));
		if(new_list == NULL) {
			abort();
		}

		self->cap = self->len + other->len;
		self->areas = new_list;
	}

	memcpy(&self->areas[self->len], other->areas, other->len * sizeof(RArea));

	self->len += other->len;
}


/**
 * qsort comparison function to sort by RArea.x
 */
static int
_cmpX(const void *av, const void *bv)
{
	const RArea *a = (const RArea *)av, *b = (const RArea *)bv;

	if(a->x < b->x) {
		return -1;
	}

	if(a->x > b->x) {
		return 1;
	}

	return (a->y > b->y) - (a->y < b->y);
}


/**
 * Sort the RArea's in an RAreaList by their x coordinate.
 */
static void
RAreaListSortX(const RAreaList *self)
{
	if(self->len <= 1) {
		return;
	}

	qsort(self->areas, self->len, sizeof(RArea), _cmpX);
}


/**
 * qsort comparison function to sort by RArea.t
 */
static int
_cmpY(const void *av, const void *bv)
{
	const RArea *a = (const RArea *)av, *b = (const RArea *)bv;

	if(a->y < b->y) {
		return -1;
	}

	if(a->y > b->y) {
		return 1;
	}

	return (a->x > b->x) - (a->x < b->x);
}


/**
 * Sort the RArea's in an RAreaList by their y coordinate.
 */
static void
RAreaListSortY(const RAreaList *self)
{
	if(self->len <= 1) {
		return;
	}

	qsort(self->areas, self->len, sizeof(RArea), _cmpY);
}


/**
 * Create an RAreaList whose RArea's are the horizontal union of our
 * RArea's.
 */
RAreaList *
RAreaListHorizontalUnion(const RAreaList *self)
{
	RAreaList *copy = RAreaListCopy(self);

refine:
	// Two areas can't form a horizontal stripe if there's any space
	// between them.  So start by putting them all in x-coord order to be
	// sure any gaps there are necessary.
	RAreaListSortX(copy);

	// Try HorizontalUnion'ing each area with the next one.  If we can
	// create a union, replace them with it, and hop back to the top of
	// the process to start over.
	for(int i = 0; i < copy->len - 1; i++) {
		for(int j = i + 1; j < copy->len; j++) {
			RAreaList *repl = RAreaHorizontalUnion(&copy->areas[i], &copy->areas[j]);
			if(repl != NULL) {
				if(repl->len) {
					RAreaListDelete(copy, j);
					RAreaListDelete(copy, i);
					RAreaListAddList(copy, repl);
					RAreaListFree(repl);
					goto refine;
				}
				RAreaListFree(repl);
			}
		}
	}

	return copy;
}


/**
 * Create an RAreaList whose RArea's are the vertical union of our
 * RArea's.
 */
RAreaList *
RAreaListVerticalUnion(const RAreaList *self)
{
	RAreaList *copy = RAreaListCopy(self);

refine:
	// X-ref logic above in RAreaListHorizontalUnion()
	RAreaListSortY(copy);

	for(int i = 0; i < copy->len - 1; i++) {
		for(int j = i + 1; j < copy->len; j++) {
			RAreaList *repl = RAreaVerticalUnion(&copy->areas[i], &copy->areas[j]);
			if(repl != NULL) {
				if(repl->len) {
					RAreaListDelete(copy, j);
					RAreaListDelete(copy, i);
					RAreaListAddList(copy, repl);
					RAreaListFree(repl);
					goto refine;
				}
				RAreaListFree(repl);
			}
		}
	}

	return copy;
}


/**
 * Create an RAreaList of all the areas in an RAreaList that a given
 * RArea intersects with.
 */
RAreaList *
RAreaListIntersect(const RAreaList *self, const RArea *area)
{
	RAreaList *new = RAreaListNew(self->len, NULL);

	for(int i = 0; i < self->len; i++) {
		if(RAreaIsIntersect(&self->areas[i], area)) {
			RAreaListAdd(new, &self->areas[i]);
		}
	}

	return new;
}


/**
 * Run a function over each RArea in an RAreaList until one returns true,
 * allowing them a place to stash other internal data.
 */
void
RAreaListForeach(const RAreaList *self,
                 bool (*func)(const RArea *cur_area, void *data),
                 void *data)
{
	for(int i = 0 ; i < self->len ; i++) {
		if(func(&(self->areas[i]), data) == true) {
			break;
		}
	}
}



/**
 * Create an RAreaList from another, cropped to a certain area defined by
 * an RArea.
 */
static RAreaList *
RAreaListIntersectCrop(const RAreaList *self, const RArea *area)
{
	RAreaList *new = RAreaListNew(self->len, NULL);

	for(int i = 0; i < self->len; i++) {
		RArea it = RAreaIntersect(&self->areas[i], area);
		if(RAreaIsValid(&it)) {
			RAreaListAdd(new, &it);
		}
	}

	return new;
}


/**
 * Create a maximal RArea describing the union of an RAreaList.
 *
 * This is used to construct a giant square that contains all our
 * monitors (and the dead area necessary to cover them).  It winds up
 * being the equivalent of a spanning pseudo-Root window, and is used
 * when we need to figure some sort of "overall" positioning, like when
 * figuring "real" x/y coordinates.
 */
RArea
RAreaListBigArea(const RAreaList *self)
{
	int x, y, x2, y2;

	// Guard; probably impossible
	if(self->len < 1) {
		return RAreaInvalid();
	}

	for(int i = 0 ; i < self->len ; i++) {
		const RArea *area = &(self->areas[i]);
		if(i == 0 || area->x < x) {
			x = area->x;
		}

		if(i == 0 || area->y < y) {
			y = area->y;
		}

		if(i == 0 || RAreaX2(area) > x2) {
			x2 = RAreaX2(area);
		}

		if(i == 0 || RAreaY2(area) > y2) {
			y2 = RAreaY2(area);
		}
	}

	return RAreaNew(x, y, x2 - x + 1, y2 - y + 1);
}


/**
 * Find the RArea in an RAreaList that has the largest intersection with
 * a given RArea.  Colloquially, which area in an RAreaList does our
 * RArea mostly fit into?  This is used to resize a window to fill one
 * monitor, by finding which monitor it's on.
 */
RArea
RAreaListBestTarget(const RAreaList *self, const RArea *area)
{
	RArea full_area = RAreaInvalid();
	int max_area = -1;

	for(int i = 0; i < self->len; i++) {
		RArea it = RAreaIntersect(area, &self->areas[i]);
		if(RAreaIsValid(&it) && (max_area < 0 || RAreaArea(&it) > max_area)) {
			max_area = RAreaArea(&it);
			full_area = self->areas[i];
		}
	}

	return full_area;
}


/**
 * Find the x coordinate of the right-most RArea in an RAreaList.
 */
int
RAreaListMaxX(const RAreaList *self)
{
	RArea *cur_area = &self->areas[0], *area_end = &self->areas[self->len];
	int max_x = self->len ? cur_area->x : 0;

	// While a for(i=0 ; i<self->len ; i++) loop is generally nicer for
	// these iterations, it winds up being a little trickier here, so we
	// leave it as a pointer-stepper.
	while(++cur_area < area_end) {
		if(cur_area->x > max_x) {
			max_x = cur_area->x;
		}
	}

	return max_x;
}


/**
 * Find the y coordinate of the bottom-most RArea in an RAreaList.
 */
int
RAreaListMaxY(const RAreaList *self)
{
	RArea *cur_area = &self->areas[0], *area_end = &self->areas[self->len];
	int max_y = self->len ? cur_area->y : 0;

	while(++cur_area < area_end) {
		if(cur_area->y > max_y) {
			max_y = cur_area->y;
		}
	}

	return max_y;
}


/**
 * Find the x coordinate of the right edge of the left-most RArea in an
 * RAreaList.
 */
int
RAreaListMinX2(const RAreaList *self)
{
	RArea *cur_area = &self->areas[0], *area_end = &self->areas[self->len];
	int min_x = self->len ? RAreaX2(cur_area) : 0;

	while(++cur_area < area_end) {
		if(RAreaX2(cur_area) < min_x) {
			min_x = RAreaX2(cur_area);
		}
	}

	return min_x;
}


/**
 * Find the y coordinate of the bottom edge of the top-most RArea in an
 * RAreaList.
 */
int
RAreaListMinY2(const RAreaList *self)
{
	RArea *cur_area = &self->areas[0], *area_end = &self->areas[self->len];
	int min_y = self->len ? RAreaY2(cur_area) : 0;

	while(++cur_area < area_end) {
		if(RAreaY2(cur_area) < min_y) {
			min_y = RAreaY2(cur_area);
		}
	}

	return min_y;
}


/**
 * Pretty-print an RAreaList.
 *
 * Used for dev/debug.
 */
void
RAreaListPrint(const RAreaList *self)
{
	fprintf(stderr, "[len=%d cap=%d", self->len, self->cap);

	for(int i = 0 ; i < self->len ; i++) {
		RArea *area = &self->areas[i];
		fprintf(stderr, " ");
		RAreaPrint(area);
	}

	fprintf(stderr, "]");
}
