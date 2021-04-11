/*
 * Window de/iconification routines
 */
#ifndef _CTWM_WIN_ICONIFY_H
#define _CTWM_WIN_ICONIFY_H


/* De/Iconifying */
void Iconify(TwmWindow *tmp_win, int def_x, int def_y);
void DeIconify(TwmWindow *tmp_win);

/* Lower-level utils, but the squeeze code uses them too */
void ReMapTransients(TwmWindow *tmp_win);
void UnmapTransients(TwmWindow *tmp_win, bool iconify, long eventMask);


#endif /* _CTWM_WIN_ICONIFY_H */
