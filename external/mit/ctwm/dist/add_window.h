/*
 * AddWindow include file
 *
 *
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * $XConsortium: add_window.h,v 1.7 90/04/17 14:04:33 jim Exp $
 *
 * 31-Mar-88 Tom LaStrange        Initial Version.
 *
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_ADD_WINDOW_H
#define _CTWM_ADD_WINDOW_H

extern char NoName[];
extern bool resizeWhenAdd;

typedef enum {
	AWT_NORMAL,
	AWT_ICON_MANAGER,
	AWT_WINDOWBOX,
	AWT_WORKSPACE_MANAGER,
	AWT_OCCUPY,
} AWType;

TwmWindow *AddWindow(Window w, AWType wtype, IconMgr *iconp,
                     VirtualScreen *vs);
void GrabButtons(TwmWindow *tmp_win);
void GrabKeys(TwmWindow *tmp_win);

extern int AddingX;
extern int AddingY;
extern unsigned int AddingW;
extern unsigned int AddingH;


#endif /* _CTWM_ADD_WINDOW_H */

