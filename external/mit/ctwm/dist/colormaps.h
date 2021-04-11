/*
 * Colormap handling funcs
 */
#ifndef _CTWM_COLORMAPS_H
#define _CTWM_COLORMAPS_H


bool InstallWindowColormaps(int type, TwmWindow *tmp);
bool InstallColormaps(int type, Colormaps *cmaps);
void InstallRootColormap(void);
void UninstallRootColormap(void);

TwmColormap *CreateTwmColormap(Colormap c);
ColormapWindow *CreateColormapWindow(Window w, bool creating_parent,
                                     bool property_window);
void FetchWmColormapWindows(TwmWindow *tmp);

void BumpWindowColormap(TwmWindow *tmp, int inc);

void InsertRGBColormap(Atom a, XStandardColormap *maps, int nmaps,
                       bool replace);
void RemoveRGBColormap(Atom a);
void LocateStandardColormaps(void);

void free_cwins(TwmWindow *tmp);

#endif /* _CTWM_COLORMAPS_H */
