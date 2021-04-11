/*
 * Window decoration bits
 */

#ifndef _CTWM_DECORATIONS_H
#define _CTWM_DECORATIONS_H


void SetupWindow(TwmWindow *tmp_win,
                 int x, int y, int w, int h, int bw);
void SetupFrame(TwmWindow *tmp_win,
                int x, int y, int w, int h, int bw, bool sendEvent);
void SetFrameShape(TwmWindow *tmp);

void ComputeTitleLocation(TwmWindow *tmp);
void CreateWindowTitlebarButtons(TwmWindow *tmp_win);
void DeleteHighlightWindows(TwmWindow *tmp_win);

void PaintTitle(TwmWindow *tmp_win);
void PaintTitleButtons(TwmWindow *tmp_win);
void PaintTitleButton(TwmWindow *tmp_win, TBWindow  *tbw);

void PaintBorders(TwmWindow *tmp_win, bool focus);
void SetBorderCursor(TwmWindow *tmp_win, int x, int y);


#endif /* _CTWM_DECORATIONS_H */
