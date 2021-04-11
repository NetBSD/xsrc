/*
 * Various internal bits shared among the event code
 */
#ifndef _CTWM_EVENT_INTERNAL_H
#define _CTWM_EVENT_INTERNAL_H


/* event_utils.c */
/* AutoRaiseWindow in events.h (temporarily?) */
void SetRaiseWindow(TwmWindow *tmp);
void AutoPopupMaybe(TwmWindow *tmp);
void AutoLowerWindow(TwmWindow *tmp);
Window WindowOfEvent(XEvent *e);
ScreenInfo *GetTwmScreen(XEvent *event);
void SynthesiseFocusOut(Window w);
void SynthesiseFocusIn(Window w);


extern TwmWindow *Tmp_win;
extern bool ColortableThrashing;
extern bool enter_flag;
extern bool leave_flag;
extern TwmWindow *enter_win, *raise_win, *leave_win, *lower_win;


/* SynthesiseFocus*() and focus handlers look at this */
//#define TRACE_FOCUS


#endif /* _CTWM_EVENT_INTERNAL_H */
