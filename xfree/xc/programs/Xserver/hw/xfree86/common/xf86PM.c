/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86PM.c,v 3.4 2000/12/08 20:13:34 eich Exp $ */


#include "X.h"
#include "xf86.h"
#include "xf86Priv.h"

int (*xf86PMGetEventFromOs)(int fd,pmEvent *events,int num) = NULL;
pmWait (*xf86PMConfirmEventToOs)(int fd,pmEvent event) = NULL;

static char *
eventName(pmEvent event)
{
    switch(event) {
    case XF86_APM_SYS_STANDBY: return ("System Standby Request");
    case XF86_APM_SYS_SUSPEND: return ("System Suspend Request");
    case XF86_APM_CRITICAL_SUSPEND: return ("Critical Suspend");
    case XF86_APM_USER_STANDBY: return ("User System Standby Request");
    case XF86_APM_USER_SUSPEND: return ("User System Suspend Request");
    case XF86_APM_STANDBY_RESUME: return ("System Standby Resume");
    case XF86_APM_NORMAL_RESUME: return ("Normal Resume System");
    case XF86_APM_CRITICAL_RESUME: return ("Critical Resume System");
    case XF86_APM_LOW_BATTERY: return ("Battery Low");
    case XF86_APM_POWER_STATUS_CHANGE: return ("Power Status Change");
    case XF86_APM_UPDATE_TIME: return ("Update Time");
    case XF86_APM_CAPABILITY_CHANGED: return ("Capability Changed");
    case XF86_APM_STANDBY_FAILED: return ("Standby Request Failed");
    case XF86_APM_SUSPEND_FAILED: return ("Suspend Request Failed");
    default: return ("Unknown Event");
    }
}

static void
DoApmEvent(pmEvent event)
{
    /* 
     * we leave that as a global function for now. I don't know if 
     * this might cause problems in the future. It is a global server 
     * variable therefore it needs to be in a server info structure
     */
    static Bool suspended;
    int i;
    
    switch(event) {
    case XF86_APM_SYS_STANDBY:
    case XF86_APM_SYS_SUSPEND:
    case XF86_APM_CRITICAL_SUSPEND: /*do we want to delay a critical suspend?*/
    case XF86_APM_USER_STANDBY:
    case XF86_APM_USER_SUSPEND:
	/* should we do this ? */
	if (!suspended) {
	    for (i = 0; i < xf86NumScreens; i++) {
		xf86EnableAccess(xf86Screens[i]);
		if (xf86Screens[i]->EnableDisableFBAccess)
		    (*xf86Screens[i]->EnableDisableFBAccess) (i, FALSE);
	    }
	    xf86EnterServerState(SETUP);
	    for (i = 0; i < xf86NumScreens; i++) {
		xf86EnableAccess(xf86Screens[i]);
		if (xf86Screens[i]->PMEvent)
		    xf86Screens[i]->PMEvent(i,event);
		else
		    xf86Screens[i]->LeaveVT(i, 0);
	    }
	    xf86AccessLeave();      
	    xf86AccessLeaveState(); 
	    suspended = TRUE;
	}
	break;
    case XF86_APM_STANDBY_RESUME:
    case XF86_APM_NORMAL_RESUME:
    case XF86_APM_CRITICAL_RESUME:
    case XF86_APM_STANDBY_FAILED:
    case XF86_APM_SUSPEND_FAILED:
	if (suspended) {
	    xf86AccessEnter();
	    xf86EnterServerState(SETUP);
	    for (i = 0; i < xf86NumScreens; i++) {
		xf86EnableAccess(xf86Screens[i]);
		if (xf86Screens[i]->PMEvent)
		    xf86Screens[i]->PMEvent(i,event);
		else
		    xf86Screens[i]->EnterVT(i, 0);
	    }
	    xf86EnterServerState(OPERATING);
	    for (i = 0; i < xf86NumScreens; i++) {
		xf86EnableAccess(xf86Screens[i]);
		if (xf86Screens[i]->EnableDisableFBAccess)
		    (*xf86Screens[i]->EnableDisableFBAccess) (i, TRUE);
	    }
	    SaveScreens(SCREEN_SAVER_FORCER, ScreenSaverReset);
	    suspended = FALSE;
	}
	break;
    default:
	xf86EnterServerState(SETUP);
	for (i = 0; i < xf86NumScreens; i++) {
	    xf86EnableAccess(xf86Screens[i]);
	    if (xf86Screens[i]->PMEvent)
		xf86Screens[i]->PMEvent(i,event);
	}
	xf86EnterServerState(OPERATING);
	break;
    }
}

#define MAX_NO_EVENTS 8

void
xf86HandlePMEvents(int fd, pointer data)
{
    pmEvent events[MAX_NO_EVENTS];
    int i,n;
    Bool wait = FALSE;
    
    if (!xf86PMGetEventFromOs)
	return;

    if ((n = xf86PMGetEventFromOs(fd,events,MAX_NO_EVENTS))) {
	do {
	    for (i = 0; i < n; i++) {
		xf86MsgVerb(X_INFO,3,"PM Event received: %s\n",
			    eventName(events[i]));
		DoApmEvent(events[i]);
		switch (xf86PMConfirmEventToOs(fd,events[i])) {
		case PM_WAIT:
		    wait = TRUE;
		    break;
		case PM_CONTINUE:
		    wait = FALSE;
		    break;
		default:
		    break;
		}
	    }
	    if (wait)
		n = xf86PMGetEventFromOs(fd,events,MAX_NO_EVENTS);
	    else
		break;
	} while (1);
    }
}
