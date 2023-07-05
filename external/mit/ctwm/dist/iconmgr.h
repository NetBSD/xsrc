/*
 * Icon Manager includes
 *
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * $XConsortium: iconmgr.h,v 1.11 89/12/10 17:47:02 jim Exp $
 *
 * 09-Mar-89 Tom LaStrange              File Created
 *
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_ICONMGR_H
#define _CTWM_ICONMGR_H

struct WList {
	struct WList *next;
	struct WList *prev;
	struct WList *nextv;                /* pointer to the next virtual Wlist C.L. */
	struct TwmWindow *twm;
	struct IconMgr *iconmgr;
	Window w;
	Window icon;
	int x, y, width, height;
	int row, col;
	int me;
	ColorPair cp;
	Pixel highlight;
	Pixmap iconifypm;
	unsigned top, bottom;
	bool active;
	bool down;
};

struct IconMgr {
	struct IconMgr *next;  ///< Next iconmgr in this workspace
	struct IconMgr *prev;  ///< Prev iconmgr in this workspace
	struct IconMgr *lasti; ///< Last iconmgr in this workspace
	struct IconMgr *nextv; ///< Next workspace's icon manager head

	struct WList *first;                /* first window in the list */
	struct WList *last;                 /* last window in the list */
	struct WList *active;               /* the active entry */
	TwmWindow *twm_win;                 /* back pointer to the new parent */
	struct ScreenInfo *scr;             /* the screen this thing is on */
	int vScreen;                        /* the virtual screen this thing is on */
	Window w;                           /* this icon manager window */
	char *geometry;                     /* geometry string */
	char *name;
	char *icon_name;
	int x, y, width, height;
	int columns, cur_rows, cur_columns;
	int count;
};

extern WList *DownIconManager;

void CreateIconManagers(void);
IconMgr *AllocateIconManager(char *name, char *geom, char *icon_name,
                             int columns);
void AllocateOtherIconManagers(void);
void MoveIconManager(int dir);
void MoveMappedIconManager(int dir);
void JumpIconManager(int dir);
WList *AddIconManager(TwmWindow *tmp_win);
void InsertInIconManager(IconMgr *ip, WList *tmp, TwmWindow *tmp_win);
void RemoveFromIconManager(IconMgr *ip, WList *tmp);
void RemoveIconManager(TwmWindow *tmp_win);
void CurrentIconManagerEntry(WList *current);
void ActiveIconManager(WList *active);
void NotActiveIconManager(WList *active);
void DrawIconManagerBorder(WList *tmp, bool fill);
void SortIconManager(IconMgr *ip);
void PackIconManager(IconMgr *ip);
void PackIconManagers(void);
void dump_iconmanager(IconMgr *mgr, char *label);
void DrawIconManagerIconName(TwmWindow *tmp_win);
void ShowIconifiedIcon(TwmWindow *tmp_win);


/* Spacing between the text and the outer border.  */
#define ICON_MGR_IBORDER 3
/* Thickness of the outer border (3d or not).  */
#define ICON_MGR_OBORDER \
    (Scr->use3Diconmanagers ? Scr->IconManagerShadowDepth : 2)


#endif /* _CTWM_ICONMGR_H */
