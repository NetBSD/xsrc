/*
 * Copyright 1992, 2005, 2007 Stefan Monnier.
 *
 * $Id: otp.h,v 1.1.1.1 2021/04/11 08:36:51 nia Exp $
 *
 * handles all the OnTopPriority-related issues.
 *
 */

#ifndef _CTWM_OTP_H
#define _CTWM_OTP_H

/* kind of window */
typedef enum WinType { WinWin, IconWin } WinType;

/* Flags that might alter OTP (currently only EWMH bits) */
#ifdef EWMH
#define OTP_AFLAG_ABOVE      (1 << 0)
#define OTP_AFLAG_BELOW      (1 << 1)
#define OTP_AFLAG_FULLSCREEN (1 << 2)
#endif


/* Wrapper functions to maintain the internal list uptodate.  */
int ReparentWindow(Display *display, TwmWindow *twm_win,
                   WinType wintype, Window parent, int x, int y);
void ReparentWindowAndIcon(Display *display, TwmWindow *twm_win,
                           Window parent, int win_x, int win_y,
                           int icon_x, int icon_y);

/* misc functions that are not specific to OTP */
bool isTransientOf(TwmWindow *, TwmWindow *);
bool isSmallTransientOf(TwmWindow *, TwmWindow *);
bool isGroupLeaderOf(TwmWindow *, TwmWindow *);
bool isGroupLeader(TwmWindow *);

/* functions to "move" windows */
void OtpRaise(TwmWindow *, WinType);
void OtpLower(TwmWindow *, WinType);
void OtpRaiseLower(TwmWindow *, WinType);
void OtpTinyRaise(TwmWindow *, WinType);
void OtpTinyLower(TwmWindow *, WinType);
void OtpCirculateSubwindows(VirtualScreen *vs, int direction);
void OtpHandleCirculateNotify(VirtualScreen *vs, TwmWindow *twm_win,
                              WinType wintype, int place);

/* functions to change a window's OTP value */
void OtpSetPriority(TwmWindow *, WinType, int, int);
void OtpChangePriority(TwmWindow *, WinType, int);
void OtpSwitchPriority(TwmWindow *, WinType);
void OtpToggleSwitching(TwmWindow *, WinType);
void OtpRecomputePrefs(TwmWindow *);
void OtpForcePlacement(TwmWindow *, int, TwmWindow *);

void OtpReassignIcon(TwmWindow *twm_win, Icon *old_icon);
void OtpFreeIcon(TwmWindow *twm_win);

void OtpSetAflagMask(TwmWindow *twm_win, unsigned mask, unsigned setto);
void OtpSetAflag(TwmWindow *twm_win, unsigned flag);
void OtpClearAflag(TwmWindow *twm_win, unsigned flag);
void OtpStashAflagsFirstTime(TwmWindow *twm_win);
void OtpRestackWindow(TwmWindow *twm_win);

void OtpUnfocusWindow(TwmWindow *twm_win);
void OtpFocusWindow(TwmWindow *twm_win);

/* functions to manage the preferences. The second arg specifies icon prefs */
void OtpScrInitData(ScreenInfo *);
name_list **OtpScrSwitchingL(ScreenInfo *, WinType);
name_list **OtpScrPriorityL(ScreenInfo *, WinType, int);
void OtpScrSetSwitching(ScreenInfo *, WinType, bool);
void OtpScrSetZero(ScreenInfo *, WinType, int);

/* functions to inform OTP-manager of window creation/destruction */
void OtpAdd(TwmWindow *, WinType);
void OtpRemove(TwmWindow *, WinType);

/* Iterators.  */
TwmWindow *OtpBottomWin(void);
TwmWindow *OtpTopWin(void);
TwmWindow *OtpNextWinUp(TwmWindow *);
TwmWindow *OtpNextWinDown(TwmWindow *);

/* Other access functions */
int OtpEffectiveDisplayPriority(TwmWindow *twm_win);
int OtpEffectivePriority(TwmWindow *twm_win);
bool OtpIsFocusDependent(TwmWindow *twm_win);

/* Other debugging functions */
bool OtpCheckConsistency(void);

#endif /* _CTWM_OTP_H */
