/*
 * Icon releated definitions
 *
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * $XConsortium: icons.h,v 1.4 89/07/18 17:16:24 jim Exp $
 *
 * 10-Apr-89 Tom LaStrange        Initial Version.
 *
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_ICONS_H
#define _CTWM_ICONS_H

/* Where did the Image for the Icon come from? */
typedef enum {
	match_none,
	match_list,                 /* shared Image: iconslist and Scr->ImageCache */
	match_icon_pixmap_hint,     /* Pixmap copied from IconPixmapHint */
	match_net_wm_icon,          /* Pixmap created from NET_WM_ICON */
	match_unknown_default,      /* shared Image: Scr->UnknownImage */
} Matchtype;

struct Icon {
	Matchtype   match;
	Window      w;              /* the icon window */
	OtpWinList *otp;            /* OnTopPriority info for the icon */
	Window      bm_w;           /* the icon bitmap window */
	Image       *image;         /* image icon structure */
	int         x;              /* icon text x coordinate */
	int         y;              /* icon text y coordiante */
	int         w_x;            /* x coor of the icon window !!untested!! */
	int         w_y;            /* y coor of the icon window !!untested!! */
	int         w_width;        /* width of the icon window */
	int         w_height;       /* height of the icon window */
	int         width;          /* width of the icon bitmap */
	int         height;         /* height of the icon bitmap */
	Pixel       border;         /* border color */
	ColorPair   iconc;
	int         border_width;
	struct IconRegion   *ir;
	bool        has_title, title_shrunk;
	bool        w_not_ours;     /* Icon.w comes from IconWindowHint */
};

struct IconRegion {
	struct IconRegion   *next;
	int                 x, y, w, h;
	RegGravity          grav1, grav2;
	int                 stepx, stepy;       // allocation granularity
	TitleJust           TitleJustification;
	IRJust              Justification;
	IRAlignement        Alignement;
	name_list           *clientlist;
	struct IconEntry    *entries;
};

struct IconEntry {
	struct IconEntry    *next;
	int                 x, y, w, h;
	TwmWindow           *twm_win;
	bool                used;
};


/* Placement and IconsRegion handling */
name_list **AddIconRegion(const char *geom, RegGravity grav1, RegGravity grav2,
                          int stepx, int stepy, const char *ijust,
                          const char *just, const char *align);

/* Icon [window] creation/destruction */
void CreateIconWindow(TwmWindow *tmp_win, int def_x, int def_y);
void DeleteIconsList(TwmWindow *tmp_win);
void DeleteIcon(Icon *icon);
void ReleaseIconImage(Icon *icon);

/* Handling for bringing them up or down */
void IconUp(TwmWindow *tmp_win);
void IconDown(TwmWindow *tmp_win);

/* Drawing */
void PaintIcon(TwmWindow *tmp_win);
void ShrinkIconTitle(TwmWindow *tmp_win);
void ExpandIconTitle(TwmWindow *tmp_win);
int GetIconOffset(Icon *icon);
void RedoIcon(TwmWindow *win);
void RedoIconName(TwmWindow *win);

#endif /* _CTWM_ICONS_H */
