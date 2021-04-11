/*
 * Occupation handling routines
 */

#ifndef _CTWM_OCCUPATION_H
#define _CTWM_OCCUPATION_H


struct OccupyWindow {
	Window        w;
	TwmWindow     *twm_win;
	char          *geometry;
	Window        *obuttonw;
	Window        OK, cancel, allworkspc;
	int           width, height;
	int           minwidth, minheight;
	char          *name;
	char          *icon_name;
	int           lines, columns;
	int           hspace, vspace;         /* space between workspaces */
	int           bwidth, bheight;
	int           owidth;                 /* oheight == bheight */
	ColorPair     cp;
	MyFont        font;
	int           tmpOccupation;
};


/* Setting occupation bits */
void SetupOccupation(TwmWindow *twm_win, int occupation_hint);
void AddToWorkSpace(char *wname, TwmWindow *twm_win);
void RemoveFromWorkSpace(char *wname, TwmWindow *twm_win);
void ToggleOccupation(char *wname, TwmWindow *twm_win);
void MoveToNextWorkSpace(VirtualScreen *vs, TwmWindow *twm_win);
void MoveToPrevWorkSpace(VirtualScreen *vs, TwmWindow *twm_win);
void MoveToNextWorkSpaceAndFollow(VirtualScreen *vs, TwmWindow *twm_win);
void MoveToPrevWorkSpaceAndFollow(VirtualScreen *vs, TwmWindow *twm_win);
void WmgrRedoOccupation(TwmWindow *win);
void WMgrRemoveFromCurrentWorkSpace(VirtualScreen *vs, TwmWindow *win);
void WMgrAddToCurrentWorkSpaceAndWarp(VirtualScreen *vs, char *winname);
void OccupyAll(TwmWindow *twm_win);

/* Occupation editing window */
void CreateOccupyWindow(void);
void ResizeOccupyWindow(TwmWindow *win);
void PaintOccupyWindow(void);
void OccupyHandleButtonEvent(XEvent *event);
void Occupy(TwmWindow *twm_win);

/* Backend/util */
void ChangeOccupation(TwmWindow *tmp_win, int newoccupation);
bool AddToClientsList(char *workspace, char *client);
unsigned int GetMaskFromProperty(unsigned char *_prop, unsigned long len);
int GetPropertyFromMask(unsigned int mask, char **prop);



/* Various other code needs to look at this */
extern int fullOccupation;

/* Hopefully temporary; x-ref comment in .c */
extern TwmWindow *occupyWin;

#endif // _CTWM_OCCUPATION_H
