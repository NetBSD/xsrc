/*
 * Functions related to the window ring.
 */
#ifndef _CTWM_WIN_RING_H
#define _CTWM_WIN_RING_H

void UnlinkWindowFromRing(TwmWindow *win);
void AddWindowToRing(TwmWindow *win);

#define WindowIsOnRing(win) ((win) && (win)->ring.next)
#define InitWindowNotOnRing(win) ((win)->ring.next = (win)->ring.prev = NULL)

#endif
