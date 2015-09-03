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

#ifdef GNOME
/* 6/19/1999 nhd for GNOME compliance */
#include "gnome.h"
#endif /* GNOME */
#include "vscreen.h"

#ifndef _WORKMGR_
#define _WORKMGR_

#define MAXWORKSPACE 32
#define MAPSTATE      0
#define BUTTONSSTATE  1

#define STYLE_NORMAL	0
#define STYLE_STYLE1	1
#define STYLE_STYLE2	2
#define STYLE_STYLE3	3

struct winList {
    struct WorkSpace	*wlist;
    Window		w;
    int			x, y;
    int			width, height;
    TwmWindow		*twm_win;
    ColorPair		cp;
    MyFont		font;
    struct winList	*next;
};

struct WorkSpaceMgr {
    struct WorkSpace	   *workSpaceList;
    struct WorkSpaceWindow *workSpaceWindowList;
    struct OccupyWindow    *occupyWindow;
    MyFont     	    buttonFont;
    MyFont     	    windowFont;
    ColorPair  	    windowcp;
    Bool       	    windowcpgiven;
    ColorPair       cp;
    long	    count;
    char	    *geometry;
    int		    lines, columns;
    int		    noshowoccupyall;
    int             initialstate;
    short      	    buttonStyle;
    name_list	    *windowBackgroundL;
    name_list	    *windowForegroundL;
    /* The fields below have been moved from WorkSpaceWindow */
    ColorPair    	curColors;
    Image		*curImage;
    unsigned long	curBorderColor;
    Bool		curPaint;

    ColorPair    	defColors;
    Image	       *defImage;
    unsigned long	defBorderColor;
    int			hspace, vspace;
    char	       *name;
    char	       *icon_name;
};

struct WorkSpace {
  long	              number;
  char	              *name;
  char	              *label;
  Image	              *image;
  name_list           *clientlist;
  IconMgr             *iconmgr;
  ColorPair           cp;
  ColorPair           backcp;
  TwmWindow	      *save_focus;  /* Used by SaveWorkspaceFocus feature */
  struct WindowRegion *FirstWindowRegion;
  struct WorkSpace *next;
};

struct MapSubwindow {
  Window  w;
  int     x, y;
  WinList wl;
};

struct ButtonSubwindow {
  Window w;
};

struct WorkSpaceWindow {		/* There is one per virtual screen */
  virtualScreen   *vs;
  Window	  w;
  TwmWindow       *twm_win;
  MapSubwindow    **mswl;		/* MapSubWindow List */
  ButtonSubwindow **bswl;		/* ButtonSubwindow List */
  WorkSpace       *currentwspc;

  int	       	state;

  int	       	width, height;
  int	       	bwidth, bheight;
  int	       	wwidth, wheight;
};

struct OccupyWindow {
  Window       	w;
  TwmWindow    	*twm_win;
  char		*geometry;
  Window       	*obuttonw;
  Window       	OK, cancel, allworkspc;
  int	       	width, height;
  char		*name;
  char		*icon_name;
  int	       	lines, columns;
  int	       	hspace, vspace;		/* space between workspaces */
  int	       	bwidth, bheight;
  int	       	owidth;			/* oheight == bheight */
  ColorPair    	cp;
  MyFont       	font;
  int	       	tmpOccupation;
};

struct CaptiveCTWM {
  Window	root;
  String	name;
};

void InitWorkSpaceManager (void);
void ConfigureWorkSpaceManager (void);
void CreateWorkSpaceManager (void);
void GotoWorkSpaceByName (virtualScreen *vs, char *wname);
void GotoWorkSpaceByNumber (virtualScreen *vs, int workspacenum);
void GotoPrevWorkSpace (virtualScreen *vs);
void GotoNextWorkSpace (virtualScreen *vs);
void GotoRightWorkSpace (virtualScreen *vs);
void GotoLeftWorkSpace (virtualScreen *vs);
void GotoUpWorkSpace (virtualScreen *vs);
void GotoDownWorkSpace (virtualScreen *vs);
void GotoWorkSpace (virtualScreen *vs, WorkSpace *ws);
void AddWorkSpace (char *name,
		   char *background, char *foreground,
		   char *backback, char *backfore, char *backpix);
void SetupOccupation (TwmWindow *twm_win, int occupation_hint);
void Occupy (TwmWindow *twm_win);
void OccupyHandleButtonEvent (XEvent *event);
void OccupyAll (TwmWindow *twm_win);
void AddToWorkSpace (char *wname, TwmWindow *twm_win);
void RemoveFromWorkSpace (char *wname, TwmWindow *twm_win);
void ToggleOccupation (char *wname, TwmWindow *twm_win);
void AllocateOthersIconManagers (void);
void ChangeOccupation (TwmWindow *tmp_win, int newoccupation);
void WmgrRedoOccupation (TwmWindow *win);
void WMgrRemoveFromCurrentWorkSpace (virtualScreen *vs, TwmWindow *win);
#ifdef VMS /* Because that name is longer than 31 characters */
#define WMgrAddToCurrentWorkSpaceAndWarp WMgrAddToCurrWorkSpaceAndWarp
#endif
void WMgrAddToCurrentWorkSpaceAndWarp (virtualScreen *vs, char *winname);
void WMgrHandleExposeEvent (virtualScreen *vs, XEvent *event);
void PaintWorkSpaceManager (virtualScreen *vs);
void PaintOccupyWindow (void);
unsigned int GetMaskFromProperty (unsigned char *prop, unsigned long len);
void AddToClientsList (char *workspace, char *client);
void WMapToggleState (virtualScreen *vs);
void WMapSetMapState (virtualScreen *vs);
void WMapSetButtonsState (virtualScreen *vs);
void WMapAddWindow (TwmWindow *win);
void WMapDestroyWindow (TwmWindow *win);
void WMapMapWindow (TwmWindow *win);
void WMapSetupWindow (TwmWindow *win, int x, int y, int w, int h);
void WMapIconify (TwmWindow *win);
void WMapDeIconify (TwmWindow *win);
void WMapRaiseLower (TwmWindow *win);
void WMapLower (TwmWindow *win);
void WMapRaise (TwmWindow *win);
void WMapRestack (WorkSpace *ws);
void WMapUpdateIconName (TwmWindow *win);
void WMgrHandleKeyReleaseEvent (virtualScreen *vs, XEvent *event);
void WMgrHandleKeyPressEvent (virtualScreen *vs, XEvent *event);
void WMgrHandleButtonEvent (virtualScreen *vs, XEvent *event);
void InvertColorPair (ColorPair *cp);
void WMapRedrawName (virtualScreen *vs, WinList   wl);
void WMapCreateCurrentBackGround (char *border,
				  char *background, char *foreground,
				  char *pixmap);
void WMapCreateDefaultBackGround (char *border,
				  char *background, char *foreground,
				  char *pixmap);
char *GetCurrentWorkSpaceName (virtualScreen *vs);
Bool AnimateRoot (void);
void AddToCaptiveList (void);
void RemoveFromCaptiveList (void);
Bool RedirectToCaptive (Window window);
void SetPropsIfCaptiveCtwm (TwmWindow *win);
Window CaptiveCtwmRootWindow (Window window);

void MoveToNextWorkSpace (virtualScreen *vs, TwmWindow *twm_win);
void MoveToPrevWorkSpace (virtualScreen *vs, TwmWindow *twm_win);
void MoveToNextWorkSpaceAndFollow (virtualScreen *vs, TwmWindow *twm_win);
void MoveToPrevWorkSpaceAndFollow (virtualScreen *vs, TwmWindow *twm_win);

CaptiveCTWM GetCaptiveCTWMUnderPointer (void);
void SetNoRedirect (Window window);

extern void ShowBackground (virtualScreen *vs);

Bool visible (TwmWindow *tmp_win);

#endif /* _WORKMGR_ */
