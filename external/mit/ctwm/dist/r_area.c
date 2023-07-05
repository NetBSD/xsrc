/*
 * Copyright notice...
 */

#include "ctwm.h"

#include <stdlib.h>
#include <stdio.h>

#include "r_area.h"
#include "r_area_list.h"
#include "util.h"


/**
 * Construct an RArea from given components.
 */
RArea
RAreaNew(int x, int y, int width, int height)
{
	RArea area = { x, y, width, height };
	return area;
}


/**
 * Return a pointer to a static newly constructed RArea.
 *
 * This is a thin wrapper around RAreaNew() that returns a static
 * pointer.  This is used in places that need to take RArea pointers, but
 * we don't want to futz with making local intermediate vars.  Currently
 * exclusively used inline in RAreaListNew() calls.
 */
RArea *
RAreaNewStatic(int x, int y, int width, int height)
{
	static RArea area;
	area = RAreaNew(x, y, width, height);
	return &area;
}


/**
 * Return a facially-invalid RArea.
 *
 * This is used in places that need a sentinel value.
 */
RArea
RAreaInvalid(void)
{
	RArea area = { -1, -1, -1, -1 };
	return area;
}


/**
 * Is an RArea facially valid?
 *
 * Mostly used to check against sentinel values in places that may or may
 * not have a real value to work with.
 */
bool
RAreaIsValid(const RArea *self)
{
	return self->width >= 0 && self->height >= 0;
}


/**
 * Return the right edge of an RArea.
 */
int
RAreaX2(const RArea *self)
{
	return self->x + self->width - 1;
}


/**
 * Return the bottom edge of an RArea.
 */
int
RAreaY2(const RArea *self)
{
	return self->y + self->height - 1;
}


/**
 * Return the area of an RArea.
 */
int
RAreaArea(const RArea *self)
{
	return self->width * self->height;
}


/**
 * Return an RArea describing the intersection of two RArea's.
 */
RArea
RAreaIntersect(const RArea *self, const RArea *other)
{
	// Do they even intersect?
	if(RAreaIsIntersect(self, other)) {
		int x1, x2, y1, y2;

		x1 = max(other->x, self->x);
		x2 = min(RAreaX2(other), RAreaX2(self));

		y1 = max(other->y, self->y);
		y2 = min(RAreaY2(other), RAreaY2(self));

		return RAreaNew(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	}

	// Nope, so nothing
	return RAreaInvalid();
}


/**
 * Do two areas intersect?
 */
bool
RAreaIsIntersect(const RArea *self, const RArea *other)
{
	// [other][self]
	if(RAreaX2(other) < self->x) {
		return false;
	}

	// [self][other]
	if(other->x > RAreaX2(self)) {
		return false;
	}

	// [other]
	// [self]
	if(RAreaY2(other) < self->y) {
		return false;
	}

	// [self]
	// [other]
	if(other->y > RAreaY2(self)) {
		return false;
	}

	return true;
}


/**
 * Is a given coordinate inside a RArea?
 */
bool
RAreaContainsXY(const RArea *self, int x, int y)
{
	return x >= self->x && x <= RAreaX2(self)
	       && y >= self->y && y <= RAreaY2(self);
}


/**
 * Create a list of maximal horizontal stripes of two RArea's.
 *
 * This yields a set of RArea's that completely cover (without overlap)
 * the pair of input RArea's (or NULL if the inputs are disjoint).  That
 * could be just a single RArea if e.g. they're the same height and
 * touch/overlap horizontally, or they're the same width and
 * touch/overlap vertically.  Otherwise it will wind up being multiple
 * rows (2 or 3).
 *
 * Only used in startup to populate the RLayout.horiz list.
 */
RAreaList *
RAreaHorizontalUnion(const RArea *self, const RArea *other)
{
	// If there's horizontal space between them, they can't possibly
	// combine.
	// [other]  [self]    or     [self]  [other]
	if(RAreaX2(other) < self->x - 1) {
		return NULL;
	}
	if(other->x > RAreaX2(self) + 1) {
		return NULL;
	}

	// No vertical overlap (though maybe they touch?)
	// [other] or [self]
	// [self]     [other]
	if(RAreaY2(other) < self->y || other->y > RAreaY2(self)) {
		// Special case: if they're the same width, and start at the same
		// X coordinate, _and_ are touching each other vertically, we can
		// combine them into a single block.
		if(self->width == other->width && self->x == other->x) {
			// [other]
			// [self ]
			if(RAreaY2(other) + 1 == self->y) {
				return RAreaListNew(
				               1,
				               RAreaNewStatic(self->x, other->y,
				                              self->width, self->height + other->height),
				               NULL);
			}

			// [self ]
			// [other]
			if(RAreaY2(self) + 1 == other->y) {
				return RAreaListNew(
				               1,
				               RAreaNewStatic(self->x, self->y,
				                              self->width, self->height + other->height),
				               NULL);
			}

			// Nope, there must be vertical space between us
		}

		// Can't combine
		return NULL;
	}

	// No horizontal space between the two (may be touching, or
	// overlapping), and there's some vertical overlap.
	// So there must be some horizontal stripes we can create.
	{
		// left- and right-most x coords, which define the maximum width
		// a union could be.
		const int min_x = min(self->x, other->x);
		const int max_x = max(RAreaX2(self), RAreaX2(other));
		const int max_width = max_x - min_x + 1;

		RAreaList *res = RAreaListNew(3, NULL);

		// Figure which starts higher.
		const RArea *top, *bot;
		if(self->y < other->y) {
			top = self;
			bot = other;
		}
		else {
			top = other;
			bot = self;
		}
		// bot now starts even with or below top

		//      [   ]    [   ]
		// [bot][top] or [top][bot]
		//      [   ]         [   ]

		// Room in top before bot starts?  That's one stripe.
		if(bot->y != top->y) {
			RAreaListAdd(res, RAreaNewStatic(top->x, top->y,
			                                 top->width, bot->y - top->y));
		}

		// Next there's a stripe across both of them.
		RAreaListAdd(res,
		             RAreaNewStatic(min_x, bot->y,
		                            max_width,
		                            min(RAreaY2(top), RAreaY2(bot)) - max(top->y, bot->y) + 1));

		// If their bottoms aren't coincident, there's another stripe
		// below them of whichever one is taller.
		if(RAreaY2(top) != RAreaY2(bot)) {
			if(RAreaY2(bot) < RAreaY2(top)) {
				//      [   ]    [   ]
				// [bot][top] or [top][bot]
				//      [   ]    [   ]
				RAreaListAdd(res,
				             RAreaNewStatic(top->x, RAreaY2(bot) + 1,
				                            top->width, RAreaY2(top) - RAreaY2(bot)));
			}
			else {
				//      [   ]    [   ]
				// [bot][top] or [top][bot]
				// [  ]               [   ]
				RAreaListAdd(res,
				             RAreaNewStatic(bot->x, RAreaY2(top) + 1,
				                            bot->width, RAreaY2(bot) - RAreaY2(top)));
			}
		}

		// And there they are.
		return res;
	}
}


/**
 * Create a list of maximal vertical stripes of two RArea's.
 *
 * This yields a set of RArea's that completely cover (without overlap)
 * the pair of input RArea's (or NULL if the inputs are disjoint).  This
 * is the equivalent of RAreaHorizontalUnion(), except with vertical
 * stripes.
 *
 * Only used in startup to populate the RLayout.vert list.
 */
RAreaList *
RAreaVerticalUnion(const RArea *self, const RArea *other)
{
	// Vertical space between them; can't possibly combine.
	if(RAreaY2(other) < self->y - 1) {
		return NULL;
	}
	if(other->y > RAreaY2(self) + 1) {
		return NULL;
	}

	// No horizontal overlap (though perhaps touching)
	// [other][self] or [self][other]
	if(RAreaX2(other) < self->x || other->x > RAreaX2(self)) {
		// Special case: if they're the same height, and start at the same
		// Y coordinate, _and_ are touching each other horizontally, we can
		// combine them into a single block.
		if(self->height == other->height && self->y == other->y) {
			// [other][self]
			if(RAreaX2(other) + 1 == self->x) {
				return RAreaListNew(
				               1,
				               RAreaNewStatic(other->x, self->y,
				                              self->width + other->width, self->height),
				               NULL);
			}

			// [self][other]
			if(RAreaX2(self) + 1 == other->x) {
				return RAreaListNew(
				               1,
				               RAreaNewStatic(self->x, self->y,
				                              self->width + other->width, self->height),
				               NULL);
			}

			// Nope, not touching; horizontal space means no combining.
		}
		return NULL;
	}

	// No vertical space (touching or overlap), and some horizontal
	// overlap.  So there are vertical stripes we can make.
	{
		// top- and bottom-most y coords, giving a maximum height
		const int min_y = min(self->y, other->y);
		const int max_y = max(RAreaY2(self), RAreaY2(other));
		const int max_height = max_y - min_y + 1;

		RAreaList *res = RAreaListNew(3, NULL);

		// Which starts left-most
		const RArea *left, *right;
		if(self->x < other->x) {
			left = self;
			right = other;
		}
		else {
			left = other;
			right = self;
		}

		// [--left--] or  [right]  or    [right] or [left]
		//  [right]     [--left--]    [left]          [right]

		// Room to the left before right starts?  That's one stripe.
		if(right->x != left->x) {
			RAreaListAdd(res,
			             RAreaNewStatic(left->x, left->y,
			                            right->x - left->x, left->height));
		}

		// There's a stripe of their overlap.
		RAreaListAdd(res,
		             RAreaNewStatic(right->x, min_y,
		                            min(RAreaX2(left), RAreaX2(right)) - max(left->x, right->x) + 1,
		                            max_height));

		// If they don't end at the same x coord, there's a third stripe
		// of a piece to the right of one or the other.
		if(RAreaX2(left) != RAreaX2(right)) {
			if(RAreaX2(right) < RAreaX2(left)) {
				// [--left--] or  [right]
				//  [right]     [--left--]
				RAreaListAdd(res,
				             RAreaNewStatic(RAreaX2(right) + 1, left->y,
				                            RAreaX2(left) - RAreaX2(right), left->height));
			}
			else {
				//     [right] or [left]
				//  [left]          [right]
				RAreaListAdd(res,
				             RAreaNewStatic(RAreaX2(left) + 1, right->y,
				                            RAreaX2(right) - RAreaX2(left), right->height));
			}
		}

		return res;
	}
}


/**
 * Pretty-print an RArea.
 *
 * Used for dev/debug.
 */
void
RAreaPrint(const RArea *self)
{
	fprintf(stderr, "[x=%d y=%d w=%d h=%d]", self->x, self->y, self->width,
	        self->height);
}
