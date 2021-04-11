/*
 * Workspace-related structures and definitions
 */

#ifndef _CTWM_WORKSPACE_STRUCTS_H
#define _CTWM_WORKSPACE_STRUCTS_H

#define MAXWORKSPACE 32

typedef enum {
	WMS_map,
	WMS_buttons,
} WMgrState;

typedef enum {
	STYLE_NORMAL,
	STYLE_STYLE1,
	STYLE_STYLE2,
	STYLE_STYLE3,
} ButtonStyle;

struct winList {
	struct WorkSpace    *wlist;
	Window              w;
	int                 x, y;
	int                 width, height;
	TwmWindow           *twm_win;
	ColorPair           cp;
	MyFont              font;
	struct winList      *next;
};

struct WorkSpaceMgr {
	struct WorkSpace       *workSpaceList;
	struct WorkSpaceWindow *workSpaceWindowList;
	struct OccupyWindow    *occupyWindow;
	MyFont          buttonFont;
	MyFont          windowFont;
	ColorPair       windowcp;
	bool            windowcpgiven;
	ColorPair       cp;
	long            count;
	char            *geometry;
	int             lines, columns;
	bool            noshowoccupyall;
	WMgrState       initialstate;
	ButtonStyle     buttonStyle;
	name_list       *windowBackgroundL;
	name_list       *windowForegroundL;
	/* The fields below have been moved from WorkSpaceWindow */
	ColorPair           curColors;
	Image               *curImage;
	Pixel               curBorderColor;
	bool                curPaint;

	ColorPair           defColors;
	Image              *defImage;
	Pixel               defBorderColor;
	int                 hspace, vspace;
	char               *name;
	char               *icon_name;
};

struct WorkSpace {
	int                 number;
	char                *name;
	char                *label;
	Image               *image;
	name_list           *clientlist;
	IconMgr             *iconmgr;
	ColorPair           cp;
	ColorPair           backcp;
	TwmWindow           *save_focus;  /* Used by SaveWorkspaceFocus feature */
	struct WindowRegion *FirstWindowRegion;
	struct WorkSpace *next;
};

struct MapSubwindow {
	Window  w;
	int     x, y;
	WinList *wl;
};

struct ButtonSubwindow {
	Window w;
};

struct WorkSpaceWindow {                /* There is one per virtual screen */
	VirtualScreen   *vs;
	Window          w;
	TwmWindow       *twm_win;
	MapSubwindow    **mswl;               /* MapSubWindow List */
	ButtonSubwindow **bswl;               /* ButtonSubwindow List */
	WorkSpace       *currentwspc;

	WMgrState     state;

	int           width, height;   // Window dimensions
	int           bwidth, bheight; // Button dimensions
	int           wwidth, wheight; // Map dimensions
};

#endif /* _CTWM_WORKSPACE_STRUCTS_H */
