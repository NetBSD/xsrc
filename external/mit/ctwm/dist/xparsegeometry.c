/*
 * Copyright notice...
 */

#include "ctwm.h"

#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "r_layout.h"
#include "r_area.h"
#include "xparsegeometry.h"


/**
 * Parse an X Geometry out to get the positions and sizes.
 *
 * This generally wraps and replaces our uses of XParseGeometry in order
 * to allow positioning relative to a XRANDR output name.  This allows
 * specifying a geometry relative to a particular monitor, rather than on
 * the whole composite multi-screen output meta-display.
 */
int
RLayoutXParseGeometry(RLayout *layout, const char *geometry, int *x, int *y,
                      unsigned int *width, unsigned int *height)
{
	char *sep;

	// Got something that looks like a display?
	sep = strchr(geometry, ':');
	if(sep != NULL) {
		RArea mon = RLayoutGetAreaByName(layout, geometry, sep - geometry);
		if(RAreaIsValid(&mon)) {
			// Yep, one of our monitors; figure the placement on our
			// whole root where that part of this monitor lies.
			int mask = XParseGeometry(sep + 1, x, y, width, height);
			RArea big = RLayoutBigArea(layout);

			if(mask & XValue) {
				if(mask & XNegative) {
					*x -= big.width - mon.width - (mon.x - big.x);
				}
				else {
					*x += mon.x - big.x;
				}
			}

			if(mask & YValue) {
				if(mask & YNegative) {
					*y -= big.height - mon.height - (mon.y - big.y);
				}
				else {
					*y += mon.y - big.y;
				}
			}

			return mask;
		}

		// Name not found, keep the geometry part as-is
		geometry = sep + 1;
	}

	return XParseGeometry(geometry, x, y, width, height);
}
