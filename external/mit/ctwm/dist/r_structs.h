/*
 * Copyright notice...
 */

#ifndef _CTWM_R_STRUCTS_H
#define _CTWM_R_STRUCTS_H


/**
 * A particular extent of space.
 *
 * This defines an area on the abstract display.  It commonly represents
 * a monitor when defining our screen layout, and is also used to
 * represent a window when we're manipulating it on our screen space.
 */
struct RArea {
	int x; ///< X position
	int y; ///< Y position
	int width;  ///< X dimension
	int height; ///< Y dimension
};


/**
 * A set of RArea's.
 *
 * This is generally used to define a contiguous region formed of various
 * stitched-together subareas.
 */
struct RAreaList {
	int len; ///< How many we're using
	int cap; ///< How many we have space for
	RArea *areas; ///< Array of RArea members of this list
};


/**
 * The layout of our display.
 *
 * This may encompass multiple monitors, of differing sizes.  It's
 * generally only used by a few vars at startup describing the layout,
 * which gets referred to when we need to find various borders of our
 * output.
 */
struct RLayout {
	RAreaList *monitors; ///< List of all output monitors
	RAreaList *horiz; ///< List of all unique horizontal regions
	RAreaList *vert;  ///< List of all unique vertical regions

	/// List of names of the monitors.  `names[i]` corresponds with
	/// `monitors->areas[i]`.  This is used for looking up geometries
	/// with output names via RLayoutXParseGeometry(); e.g,
	/// "HDMI1:800x600+20+50".
	char **names;
};

#endif  /* _CTWM_R_STRUCTS_H */
