/*
 * resize function externs
 *
 *
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * $XConsortium: resize.h,v 1.7 90/03/23 11:42:32 jim Exp $
 *
 *  8-Apr-88 Tom LaStrange        Initial Version.
 *
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_RESIZE_H
#define _CTWM_RESIZE_H

void OpaqueResizeSize(TwmWindow *tmp_win);
void MenuStartResize(TwmWindow *tmp_win, int x, int y, int w, int h);
void StartResize(XEvent *evp, TwmWindow *tmp_win,
                 bool fromtitlebar, bool from3dborder);
void AddStartResize(TwmWindow *tmp_win, int x, int y, int w, int h);
void MenuDoResize(int x_root, int y_root, TwmWindow *tmp_win);
void DoResize(int x_root, int y_root, TwmWindow *tmp_win);
void EndResize(void);
void MenuEndResize(TwmWindow *tmp_win);
void AddEndResize(TwmWindow *tmp_win);
void ConstrainSize(TwmWindow *tmp_win, unsigned
                   int *widthp, unsigned int *heightp);

void fullzoom(TwmWindow *tmp_win, int func);
void unzoom(TwmWindow *tmp_win);
void savegeometry(TwmWindow *tmp_win);
void restoregeometry(TwmWindow *tmp_win);

void ChangeSize(char *in_string, TwmWindow *tmp_win);

void resizeFromCenter(Window w, TwmWindow *tmp_win);

#endif /* _CTWM_RESIZE_H */
