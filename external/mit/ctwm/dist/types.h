/*
 * Copyright 2004 Richard Levitte.
 */

#ifndef _CTWM_TYPES_H
#define _CTWM_TYPES_H

/* From icons.h */
typedef struct Icon Icon;
typedef struct IconRegion IconRegion;
typedef struct IconEntry IconEntry;

/* From menus.h */
typedef struct MenuItem MenuItem;
typedef struct MenuRoot MenuRoot;
typedef struct MouseButton MouseButton;
typedef struct FuncButton FuncButton;
typedef struct FuncKey FuncKey;

/* From iconmgr.h */
typedef struct WList WList;
typedef struct IconMgr IconMgr;

/* From list.h */
typedef struct name_list name_list;

/* From screen.h */
typedef struct StdCmap StdCmap;
typedef struct TitlebarPixmaps TitlebarPixmaps;
typedef struct ScreenInfo ScreenInfo;

/* From ctwm.h */
typedef struct MyFont MyFont;
typedef struct ColorPair ColorPair;
typedef struct TitleButtonFunc TitleButtonFunc;
typedef struct TitleButton TitleButton;
typedef struct TBWindow TBWindow;
typedef struct SqueezeInfo SqueezeInfo;
typedef struct TwmColormap TwmColormap;
typedef struct ColormapWindow ColormapWindow;
typedef struct Colormaps Colormaps;
typedef struct WindowRegion WindowRegion;
typedef struct WindowEntry WindowEntry;
typedef struct WindowBox WindowBox;
typedef struct TwmWindow TwmWindow;

/* From image.h */
typedef struct Image Image;

/* From vscreen.h */
typedef struct VirtualScreen VirtualScreen;

/* From workspace_structs.h */
typedef struct winList WinList;
typedef struct WorkSpaceMgr WorkSpaceMgr;
typedef struct WorkSpace WorkSpace;
typedef struct MapSubwindow MapSubwindow;
typedef struct ButtonSubwindow ButtonSubwindow;
typedef struct WorkSpaceWindow WorkSpaceWindow;

/* From occupation.h */
typedef struct OccupyWindow OccupyWindow;

/* From otp.h */
typedef struct OtpWinList OtpWinList;
typedef struct OtpPreferences OtpPreferences;

/* From r_structs.h */
typedef struct RArea RArea;
typedef struct RAreaList RAreaList;
typedef struct RLayout RLayout;

#endif /* _CTWM_TYPES_H */
