/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/
/* 
 *  [ ctwm ]
 *
 *  Copyright 1992 Claude Lecommandeur.
 *            
 * Permission to use, copy, modify  and distribute this software  [ctwm] and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above  copyright notice appear  in all copies and that both that
 * copyright notice and this permission notice appear in supporting documen-
 * tation, and that the name of  Claude Lecommandeur not be used in adverti-
 * sing or  publicity  pertaining to  distribution of  the software  without
 * specific, written prior permission. Claude Lecommandeur make no represen-
 * tations  about the suitability  of this software  for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * Claude Lecommandeur DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL  Claude Lecommandeur  BE LIABLE FOR ANY SPECIAL,  INDIRECT OR
 * CONSEQUENTIAL  DAMAGES OR ANY  DAMAGES WHATSOEVER  RESULTING FROM LOSS OF
 * USE, DATA  OR PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR
 * OTHER  TORTIOUS ACTION,  ARISING OUT OF OR IN  CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Claude Lecommandeur [ lecom@sic.epfl.ch ][ April 1992 ]
 */


/***********************************************************************
 *
 * $XConsortium: events.h,v 1.14 91/05/10 17:53:58 dave Exp $
 *
 * twm event handler include file
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef _EVENTS_
#define _EVENTS_

#include <X11/Xlib.h>
#include <X11/Xproto.h>

typedef void (*event_proc)(void);

extern void InitEvents(void);
extern Bool StashEventTime(register XEvent *ev);
extern Time lastTimestamp;
extern void SimulateMapRequest(Window w);
extern void AutoRaiseWindow(TwmWindow *tmp);
extern void SetRaiseWindow (TwmWindow *tmp);
extern void AutoLowerWindow(TwmWindow *tmp);
#define LastTimestamp() lastTimestamp
extern Window WindowOfEvent (XEvent *e);
extern void FixRootEvent (XEvent *e);
extern Bool DispatchEvent(void);
extern Bool DispatchEvent2(void);
extern void HandleEvents(void);
extern void HandleExpose(void);
extern void HandleDestroyNotify(void);
extern void HandleMapRequest(void);
extern void HandleMapNotify(void);
extern void HandleUnmapNotify(void);
extern void HandleMotionNotify(void);
extern void HandleButtonRelease(void);
extern void HandleButtonPress(void);
extern void HandleEnterNotify(void);
extern void HandleLeaveNotify(void);
extern void HandleConfigureRequest(void);
extern void HandleClientMessage(void);
extern void HandlePropertyNotify(void);
extern void HandleKeyPress(void);
extern void HandleKeyRelease(void);
extern void HandleColormapNotify(void);
extern void HandleVisibilityNotify(void);
extern void HandleUnknown(void);
extern void HandleFocusIn(XFocusInEvent *event);
extern void HandleFocusOut(XFocusOutEvent *event);
extern void SynthesiseFocusOut(Window w);
extern void SynthesiseFocusIn(Window w);
extern int Transient(Window w, Window *propw);

extern ScreenInfo *FindScreenInfo(Window w);

extern int InstallWindowColormaps (int type, TwmWindow *tmp);
extern int InstallColormaps (int type, Colormaps *cmaps);
extern void InstallRootColormap(void);
extern void UninstallRootColormap(void);
extern void ConfigureRootWindow (XEvent *ev);

extern void free_cwins (TwmWindow *tmp);

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

extern int ButtonPressed;
extern int Cancel;

extern XEvent Event;

#endif /* _EVENTS_ */
