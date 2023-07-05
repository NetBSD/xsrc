/*
 * Functions related to the window ring.
 *
 * Invariants:
 * - If a window is not on the ring, its TwmWindow::ring.next and .prev
 *   are both NULL.
 * - If a window is on the ring, they are both not NULL and point to a
 *   window which is also on the ring.
 * - Corollary: if a window is the only one on the ring, .next and .prev
 *   point to itself.
 * - Functions which act on the "current" ring window, i.e. the window
 *   that has most recently been entered and is on the ring, use
 *   Scr->RingLeader.
 * - If RingLeader is NULL, fall back to Scr->Ring.
 * - If Ring is NULL, the ring is empty (and RingLeader is also NULL).
 */

#include "ctwm.h"

#include <assert.h>

#include "screen.h"
#include "win_ring.h"

void
UnlinkWindowFromRing(TwmWindow *win)
{
	TwmWindow *prev = win->ring.prev;
	TwmWindow *next = win->ring.next;

	// We call this unconditionally at various window deletion times, and
	// if it's not on the ring, there's nothing to do.  e.g., if we don't
	// have any WindowRing config enabled...
	if(!WindowIsOnRing(win)) {
		return;
	}

	// But if it is, prev/next should always exist.
	assert(prev != NULL);
	assert(next != NULL);

	/*
	* 1. Unlink window
	* 2. If window was only thing in ring, null out ring
	* 3. If window was ring leader, set to next (or null)
	*
	* If the window is the only one in the ring, prev == next == win,
	* so the unlinking effectively is a NOP, but that doesn't matter.
	*/
	prev->ring.next = next;
	next->ring.prev = prev;

	win->ring.next = win->ring.prev = NULL;

	if(Scr->Ring == win) {
		Scr->Ring = (next != win ? next : NULL);
	}

	if(!Scr->Ring || Scr->RingLeader == win) {
		Scr->RingLeader = Scr->Ring;
	}
}

static void
AddWindowToRingUnchecked(TwmWindow *win, TwmWindow *after)
{
	TwmWindow *before = after->ring.next;

	win->ring.next = before;
	win->ring.prev = after;

	after->ring.next = win;
	before->ring.prev = win;
}

void
AddWindowToRing(TwmWindow *win)
{
	assert(win->ring.next == NULL);
	assert(win->ring.prev == NULL);

	if(Scr->Ring) {
		AddWindowToRingUnchecked(win, Scr->Ring);
	}
	else {
		win->ring.next = win->ring.prev = Scr->Ring = win;
	}
}
