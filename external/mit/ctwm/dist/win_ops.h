/*
 * Window operations
 */
#ifndef _CTWM_WIN_OPS_H
#define _CTWM_WIN_OPS_H


void SetFocusVisualAttributes(TwmWindow *tmp_win, bool focus);
void SetFocus(TwmWindow *tmp_win, Time tim);
void FocusOnRoot(void);
void AutoSqueeze(TwmWindow *tmp_win);
void Squeeze(TwmWindow *tmp_win);
void MoveOutline(Window root, int x, int y, int width, int height,
                 int bw, int th);


#endif /* _CTWM_WIN_OPS_H */
