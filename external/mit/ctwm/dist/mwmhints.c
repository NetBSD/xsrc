/* 
 *  [ ctwm ]
 *
 *  Copyright 2014 Olaf Seibert
 *
 * Permission to use, copy, modify and distribute this software [ctwm]
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Olaf Seibert not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission. Olaf Seibert
 * makes no representations about the suitability of this software for
 * any purpose. It is provided "as is" without express or implied
 * warranty.
 *
 * Olaf Seibert DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL Olaf Seibert BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Olaf Seibert [ rhialto@falu.nl ][ May 2014 ]
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

#include <stdio.h>

#include "twm.h"
#include "mwmhints.h"

static Atom MOTIF_WM_HINTS = None;

int GetMWMHints(Window w, MotifWmHints *mwmHints)
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

    if (MOTIF_WM_HINTS == (Atom)None) {
	MOTIF_WM_HINTS = XInternAtom(dpy, "_MOTIF_WM_HINTS", True);
    }

    success = XGetWindowProperty(
			dpy, w, MOTIF_WM_HINTS,
			0, 5, 		/* long_offset, long long_length, */
			False,		/* Bool delete, */
			AnyPropertyType,/* Atom req_type */
			&actual_type, 	/* Atom *actual_type_return, */
			&actual_format, /* int *actual_format_return, */
			&nitems, 	/* unsigned long *nitems_return,  */
			&bytes_after, 	/* unsigned long * */
			(unsigned char **)&prop);	/* unsigned char ** */

    if (success == Success &&
	    actual_type == MOTIF_WM_HINTS &&
	    actual_format == 32 &&
	    nitems >= 3) {
	mwmHints->flags = (int)prop[0];
	mwmHints->functions = (int)prop[1];
	mwmHints->decorations = (int)prop[2];
#ifdef FULL_MWM_DATA
	mwmHints->input_mode = (int)prop[3];
	mwmHints->status = (int)prop[4];
#endif

	if (mwmHints->flags & MWM_HINTS_FUNCTIONS) {
	    if (mwmHints->functions & MWM_FUNC_ALL) {
		mwmHints->functions ^= ~0;
	    }
	}
	if (mwmHints->flags & MWM_HINTS_DECORATIONS) {
	    if (mwmHints->decorations & MWM_DECOR_ALL) {
		mwmHints->decorations ^= ~0;
	    }
	}

	success = True;
    } else {
	success = False;
    }

    if (prop != NULL) {
    	XFree(prop);
    }

    return success;
}

