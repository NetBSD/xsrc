/*
 * Copyright 2014 Olaf Seibert
 */

/*
 * Code to look at a few Motif Window Manager hints.
 *
 * Only the bits marked [v] are actually looked at.
 * For the rest, ctwm has no concept, really.
 *
 * For some information about the meaning of the flags, see
 * the manual page VendorShell(3) from the Motif library.
 */

#include "ctwm.h"

#include <stdio.h>

#include "ctwm_atoms.h"
#include "list.h"
#include "mwmhints.h"
#include "screen.h"

bool
GetMWMHints(Window w, MotifWmHints *mwmHints)
{
	int success;
	Atom actual_type;
	int actual_format;
	unsigned long nitems;
	unsigned long bytes_after;
	unsigned long *prop = NULL;

	/* Defaults for when not found */
	mwmHints->flags = 0;
	mwmHints->functions = 0;
	mwmHints->decorations = 0;
#ifdef FULL_MWM_DATA
	mwmHints->input_mode = 0;
	mwmHints->status = 0;
#endif

	success = XGetWindowProperty(
	                  dpy, w, XA__MOTIF_WM_HINTS,
	                  0, 5,           /* long_offset, long long_length, */
	                  False,          /* Bool delete, */
	                  AnyPropertyType,/* Atom req_type */
	                  &actual_type,   /* Atom *actual_type_return, */
	                  &actual_format, /* int *actual_format_return, */
	                  &nitems,        /* unsigned long *nitems_return,  */
	                  &bytes_after,   /* unsigned long * */
	                  (unsigned char **)&prop);       /* unsigned char ** */

	if(success == Success &&
	                actual_type == XA__MOTIF_WM_HINTS &&
	                actual_format == 32 &&
	                nitems >= 3) {
		mwmHints->flags = (int)prop[0];
		mwmHints->functions = (int)prop[1];
		mwmHints->decorations = (int)prop[2];
#ifdef FULL_MWM_DATA
		mwmHints->input_mode = (int)prop[3];
		mwmHints->status = (int)prop[4];
#endif

		if(mwmHints->flags & MWM_HINTS_FUNCTIONS) {
			if(mwmHints->functions & MWM_FUNC_ALL) {
				mwmHints->functions ^= ~0;
			}
		}
		if(mwmHints->flags & MWM_HINTS_DECORATIONS) {
			if(mwmHints->decorations & MWM_DECOR_ALL) {
				mwmHints->decorations ^= ~0;
			}
		}

		success = true;
	}
	else {
		success = false;
	}

	if(prop != NULL) {
		XFree(prop);
	}

	return success;
}



/*
 * Simple test wrappers
 */
static bool
mwm_sets_decorations(MotifWmHints *hints)
{
	return (hints->flags & MWM_HINTS_DECORATIONS) ? true : false;
}


/* 1 = yes   0 = no   -1 = no opinion */
int
mwm_has_border(MotifWmHints *hints)
{
	/* No opinion if hints don't set decoration info */
	if(!mwm_sets_decorations(hints)) {
		return -1;
	}

	/* No opinion if the user told us to ignore it */
	if(LookInNameList(Scr->MWMIgnore, "DECOR_BORDER")) {
		return -1;
	}

	/* No border if hints said so */
	if((hints->decorations & MWM_DECOR_BORDER) == 0) {
		return 0;
	}

	/* Else border */
	return 1;
}


bool
mwm_sets_title(MotifWmHints *hints)
{
	/* Not if we don't have decoration info */
	if(!mwm_sets_decorations(hints)) {
		return false;
	}

	/* Not if the user wants to ignore title frobbing */
	if(LookInNameList(Scr->MWMIgnore, "DECOR_TITLE")) {
		return false;
	}

	/* Else we do have info to use */
	return true;
}


bool
mwm_has_title(MotifWmHints *hints)
{
	if(mwm_sets_decorations(hints)
	                && ((hints->decorations & MWM_DECOR_TITLE) == 0)) {
		return false;
	}
	return true;
}
