/*
 * Captive ctwm handling bits.
 */

#ifndef _CTWM_CAPTIVE_H
#define _CTWM_CAPTIVE_H

// Guard to help catch non-ifdef'd references
#ifndef CAPTIVE
#error "You're unconditionally including captive.h!"
#endif


typedef struct CaptiveCTWM {
	Window        root;
	char          *name;
} CaptiveCTWM;


bool RedirectToCaptive(Window window);
char *AddToCaptiveList(const char *cptname);
void RemoveFromCaptiveList(const char *cptname);
void SetPropsIfCaptiveCtwm(TwmWindow *win);
CaptiveCTWM GetCaptiveCTWMUnderPointer(void);
void SetNoRedirect(Window window);
void ConfigureCaptiveRootWindow(XEvent *ev);
void AdoptWindow(void);


#endif //_CTWM_CAPTIVE_H
