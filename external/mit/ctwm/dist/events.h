/*
 * twm event handler include file
 *
 *
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * $XConsortium: events.h,v 1.14 91/05/10 17:53:58 dave Exp $
 *
 * 17-Nov-87 Thomas E. LaStrange                File created
 *
 * Copyright 1992 Claude Lecommandeur.
 *
 */

#ifndef _CTWM_EVENTS_H
#define _CTWM_EVENTS_H

typedef void (*event_proc)(void);

void InitEvents(void);
bool DispatchEvent(void);
bool DispatchEvent2(void);
void HandleEvents(void) __attribute__((noreturn));

/* Bits in event_utils.c */
/*
 * This should maybe be in event_internal.h, but a few other places use
 * it.  TBD: figure out why and whether they should
 */
void AutoRaiseWindow(TwmWindow *tmp);

void FixRootEvent(XEvent *e);
void SimulateMapRequest(Window w);


extern event_proc EventHandler[];
extern Window DragWindow;
extern int origDragX;
extern int origDragY;
extern int DragX;
extern int DragY;
extern unsigned int DragWidth;
extern unsigned int DragHeight;
extern unsigned int DragBW;
extern int CurrentDragX;
extern int CurrentDragY;
extern int Context;

extern int ButtonPressed;
extern bool Cancel;

extern XEvent Event;
extern Time EventTime;

#endif /* _CTWM_EVENTS_H */
