/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/
/* 
 *  [ ctwm ]
 *
 *  Copyright 1992 Claude Lecommandeur.
 *            
 * Permission to use, copy, modify  and distribute this software  [ctwm] and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above  copyright notice appear  in all copies and that both that
 * copyright notice and this permission notice appear in supporting documen-
 * tation, and that the name of  Claude Lecommandeur not be used in adverti-
 * sing or  publicity  pertaining to  distribution of  the software  without
 * specific, written prior permission. Claude Lecommandeur make no represen-
 * tations  about the suitability  of this software  for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * Claude Lecommandeur DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL  Claude Lecommandeur  BE LIABLE FOR ANY SPECIAL,  INDIRECT OR
 * CONSEQUENTIAL  DAMAGES OR ANY  DAMAGES WHATSOEVER  RESULTING FROM LOSS OF
 * USE, DATA  OR PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR
 * OTHER  TORTIOUS ACTION,  ARISING OUT OF OR IN  CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Claude Lecommandeur [ lecom@sic.epfl.ch ][ April 1992 ]
 */


/***********************************************************************
 *
 * $XConsortium: menus.h,v 1.24 89/12/10 17:46:26 jim Exp $
 *
 * twm menus include file
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef _MENUS_
#define _MENUS_

#define TWM_ROOT	"bLoB_GoOp"	/* my private root menu */
#define TWM_WINDOWS	"TwmWindows"	/* for f.menu "TwmWindows" */
#define TWM_ICONS	"TwmIcons"	/* for f.menu "TwmIcons" */
#define TWM_WORKSPACES	"TwmWorkspaces"	/* for f.menu "TwmWorkspaces" */
#define TWM_ALLWINDOWS	"TwmAllWindows"	/* for f.menu "TwmAllWindows" */

/* Added by dl 2004 */
#define TWM_ALLICONS	"TwmAllIcons"	/* for f.menu "TwmAllIcons" */

/*******************************************************************/
/* Added by Dan Lilliehorn (dl@dl.nu) 2000-02-29                   */
#define TWM_KEYS	"TwmKeys"	/* for f.menu "TwmKeys"    */
#define TWM_VISIBLE	"TwmVisible"	/* for f.menu "TwmVisible" */

#define MAX_FILE_SIZE 4096	/* max chars to read from file for cut */

struct MenuItem
{
    struct MenuItem *next;	/* next menu item */
    struct MenuItem *prev;	/* prev menu item */
    struct MenuRoot *sub;	/* MenuRoot of a pull right menu */
    struct MenuRoot *root;	/* back pointer to my MenuRoot */
    char *item;			/* the character string displayed */
    char *action;		/* action to be performed */
    ColorPair normal;		/* unhiglight colors */
    ColorPair highlight;	/* highlight colors */
    short item_num;		/* item number of this menu */
    short x;			/* x coordinate for text */
    short func;			/* twm built in function */
    short state;		/* video state, 0 = normal, 1 = reversed */
    short strlen;		/* strlen(item) */
    short user_colors;		/* colors were specified */
    short separated;		/* separated from the next item */
};

struct MenuRoot
{
    struct MenuItem *first;	/* first item in menu */
    struct MenuItem *last;	/* last item in menu */
    struct MenuItem *lastactive; /* last active item in menu */
    struct MenuItem *defaultitem;	/* default item in menu */
    struct MenuRoot *prev;	/* previous root menu if pull right */
    struct MenuRoot *next;	/* next in list of root menus */
    char *name;			/* name of root */
    Window w;			/* the window of the menu */
    Window shadow;		/* the shadow window */
    ColorPair highlight;	/* highlight colors */
    short mapped;		/* NEVER_MAPPED, UNMAPPED, or MAPPED */
    short height;		/* height of the menu */
    short width;		/* width of the menu */
    short items;		/* number of items in the menu */
    short pull;			/* is there a pull right entry ? */
    short entered;		/* EnterNotify following pop up */
    short real_menu;		/* this is a real menu */
    short x, y;			/* position (for pinned menus) */
    short pinned;		/* is this a pinned menu*/
    struct MenuRoot *pmenu;	/* the associated pinned menu */
};

#define NEVER_MAPPED	0	/* constants for mapped field of MenuRoot */
#define UNMAPPED	1
#define MAPPED		2


struct MouseButton
{
    int func;			/* the function number */
    int mask;			/* modifier mask */
    MenuRoot *menu;		/* menu if func is F_MENU */
    MenuItem *item;		/* action to perform if func != F_MENU */
};

struct FuncButton
{
    struct FuncButton *next;	/* next in the list of function buttons */
    int num;			/* button number */
    int cont;			/* context */
    int mods;			/* modifiers */
    int func;			/* the function number */
    MenuRoot *menu;		/* menu if func is F_MENU */
    MenuItem *item;		/* action to perform if func != F_MENU */
};

struct FuncKey
{
    struct FuncKey *next;	/* next in the list of function keys */
    char *name;			/* key name */
    KeySym keysym;		/* X keysym */
    KeyCode keycode;		/* X keycode */
    int cont;			/* context */
    int mods;			/* modifiers */
    int func;			/* function to perform */
    char *win_name;		/* window name (if any) */
    char *action;		/* action string (if any) */
    MenuRoot *menu;		/* menu if func is F_MENU */
};

extern int RootFunction;
extern MenuRoot *ActiveMenu;
extern MenuItem *ActiveItem;
extern int MoveFunction;
extern int WindowMoved;
extern int ConstMove;
extern int ConstMoveDir;
extern int ConstMoveX;
extern int ConstMoveY;
extern int ConstMoveXL;
extern int ConstMoveXR;
extern int ConstMoveYT;
extern int ConstMoveYB;

#define MAXMENUDEPTH	10	/* max number of nested menus */
extern int MenuDepth;

#define MOVE_NONE	0	/* modes of constrained move */
#define MOVE_VERT	1
#define MOVE_HORIZ	2

#define WARPSCREEN_NEXT "next"
#define WARPSCREEN_PREV "prev"
#define WARPSCREEN_BACK "back"

#define COLORMAP_NEXT "next"
#define COLORMAP_PREV "prev"
#define COLORMAP_DEFAULT "default"

extern void InitTitlebarButtons(void);
extern void InitMenus(void);
extern MenuRoot *NewMenuRoot(char *name);
extern MenuItem *AddToMenu(MenuRoot *menu, char *item, char *action,
			   MenuRoot *sub, int func, char *fore, char *back);
extern Bool PopUpMenu(MenuRoot *menu, int x, int y, Bool center);
extern void MakeWorkspacesMenu (void);
extern MenuRoot *FindMenuRoot(char *name);
extern Bool AddFuncKey(char *name, int cont, int mods, int func,
		       MenuRoot *menu, char *win_name, char *action);
extern Bool AddFuncButton(int num, int cont, int mods, int func,
			  MenuRoot *menu, MenuItem *item);
extern void DestroyMenu (MenuRoot *menu);
extern int PopDownMenu(void);
extern int HideMenu(MenuRoot *menu);
extern int ExecuteFunction(int func, void *action,
			   Window w, TwmWindow *tmp_win,
			   XEvent *eventp,
			   int context, int pulldown);
extern int DeferExecution(int context, int func, Cursor cursor);
extern int NeedToDefer(MenuRoot *root);
extern void ReGrab(void);
extern int CreateTitleButton(char *name, int func, char *action,
			     MenuRoot *menuroot, Bool rightside,
			     Bool append);
extern void PaintEntry(MenuRoot *mr, MenuItem *mi, int exposure);
extern void Paint3DEntry(MenuRoot *mr, MenuItem *mi, int exposure);
extern void PaintNormalEntry(MenuRoot *mr, MenuItem *mi, int exposure);
extern void PaintMenu(MenuRoot *mr, XEvent *e);
extern int UpdateMenu(void);
extern void MakeMenus(void);
extern int MakeMenu(MenuRoot *mr);
extern int MoveMenu(XEvent *eventp);
extern void DeIconify(TwmWindow *tmp_win);
extern void Iconify(TwmWindow *tmp_win, int def_x, int def_y);
extern int WarpToScreen(int n, int inc);
extern int BumpWindowColormap(TwmWindow *tmp, int inc);
extern void SetMapStateProp(TwmWindow *tmp_win, int state);
extern void SendDeleteWindowMessage (TwmWindow *tmp, Time timestamp);
extern void SendSaveYourselfMessage (TwmWindow *tmp, Time timestamp);
extern void SendTakeFocusMessage (TwmWindow *tmp, Time timestamp);
extern int FindConstraint (TwmWindow *tmp_win, int direction);
extern void MosaicFade (TwmWindow *tmp_win, Window blanket);
extern void ZoomInWindow (TwmWindow *tmp_win, Window blanket);
extern void ZoomOutWindow (TwmWindow *tmp_win, Window blanket);
extern void FadeWindow (TwmWindow *tmp_win, Window blanket);
extern void SweepWindow (TwmWindow *tmp_win, Window blanket);
extern int WarpCursorToDefaultEntry (MenuRoot *menu);
extern void PlaceTransients(TwmWindow *tmp_win, int where);
extern void PlaceOntop (int ontop, int where);
extern void ModifyCurrentTB(int button, int mods, int func, char *action,
			    MenuRoot *menuroot);
extern void Execute(char *s);
extern void ShowIconManager (void);
extern void HideIconManager (void);
extern void RaiseWindow(TwmWindow *tmp_win);
extern void LowerWindow(TwmWindow *tmp_win);
extern void RaiseLower(TwmWindow *tmp_win);
extern void RaiseLowerFrame(Window frame, int ontop);
extern void MapRaised(TwmWindow *tmp_win);
extern void RaiseFrame(Window frame);
extern void FocusOnRoot(void);
extern void TryToPack (TwmWindow *tmp_win, int *x, int *y);
extern void TryToPush (TwmWindow *tmp_win, int x, int y, int dir);
extern void TryToGrid (TwmWindow *tmp_win, int *x, int *y);
extern void resizeFromCenter(Window w, TwmWindow *tmp_win);
extern void WarpAlongRing (XButtonEvent *ev, Bool forward);
extern void WarpToWindow (TwmWindow *t, int must_raise);
extern void DisplayPosition (TwmWindow *tmp_win, int x, int y);
extern void packwindow (TwmWindow *tmp_win, char *direction);
extern void fillwindow (TwmWindow *tmp_win, char *direction);
#if 0 /* Not implemented!!! */
extern Boolean TryNotToMoveOff ();
#endif
extern void AutoSqueeze (TwmWindow *tmp_win);
extern void Squeeze(TwmWindow *tmp_win);

#endif /* _MENUS_ */
