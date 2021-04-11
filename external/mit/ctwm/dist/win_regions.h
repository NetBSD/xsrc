/*
 * WindowRegion related funcs
 */

#ifndef _CTWM_WIN_REGIONS_H
#define _CTWM_WIN_REGIONS_H

name_list **AddWindowRegion(char *geom, RegGravity grav1, RegGravity grav2);
void CreateWindowRegions(void);
bool PlaceWindowInRegion(TwmWindow *tmp_win, int *final_x, int *final_y);
void RemoveWindowFromRegion(TwmWindow *tmp_win);

#endif /* _CTWM_WIN_REGIONS_H */

