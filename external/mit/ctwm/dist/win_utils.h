/*
 * Window util funcs
 */
#ifndef _CTWM_WIN_UTILS_H
#define _CTWM_WIN_UTILS_H


void GetWindowSizeHints(TwmWindow *tmp_win);
void FetchWmProtocols(TwmWindow *tmp);
void GetGravityOffsets(TwmWindow *tmp, int *xp, int *yp);
TwmWindow *GetTwmWindow(Window w);
char *GetWMPropertyString(Window w, Atom prop);
void FreeWMPropertyString(char *prop);
bool visible(const TwmWindow *tmp_win);
long mask_out_event(Window w, long ignore_event);
long mask_out_event_mask(Window w, long ignore_event, long curmask);
int restore_mask(Window w, long restore);
void SetMapStateProp(TwmWindow *tmp_win, int state);
bool GetWMState(Window w, int *statep, Window *iwp);
void DisplayPosition(const TwmWindow *_unused_tmp_win, int x, int y);
void MoveResizeSizeWindow(int x, int y, unsigned int width,
                          unsigned int height);
void TryToPack(TwmWindow *tmp_win, int *x, int *y);
void TryToPush(TwmWindow *tmp_win, int x, int y);
void TryToGrid(TwmWindow *tmp_win, int *x, int *y);
bool ConstrainByLayout(RLayout *layout, int move_off_res, int *left, int width,
                       int *top, int height);
void ConstrainByBorders1(int *left, int width, int *top, int height);
void ConstrainByBorders(TwmWindow *twmwin, int *left, int width,
                        int *top, int height);
void WarpToWindow(TwmWindow *t, bool must_raise);
void send_clientmessage(Window w, Atom a, Time timestamp);
XWMHints *gen_synthetic_wmhints(TwmWindow *win);
XWMHints *munge_wmhints(TwmWindow *win, XWMHints *hints);
bool set_window_name(TwmWindow *win);
void apply_window_name(TwmWindow *win);
bool set_window_icon_name(TwmWindow *win);
void apply_window_icon_name(TwmWindow *win);


#endif /* _CTWM_WIN_UTILS_H */
