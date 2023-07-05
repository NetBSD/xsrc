/*
 * Internal bits for function handling
 */

#ifndef _CTWM_FUNCTIONS_INTERNAL_H
#define _CTWM_FUNCTIONS_INTERNAL_H


/* Keep in sync with ExecuteFunction() in external functions.h */
#define EF_FULLPROTO \
      int   func,   void *  action,   Window   w,   TwmWindow *  tmp_win, \
      XEvent *  eventp,   int   context,   bool   pulldown
#define EF_ARGS \
    /*int */func, /*void **/action, /*Window */w, /*TwmWindow **/tmp_win, \
    /*XEvent **/eventp, /*int */context, /*bool */pulldown

typedef void (ExFunc)(EF_FULLPROTO);

#define DFHANDLER(func) void f_##func##_impl(EF_FULLPROTO)


/*
 * Various handlers
 */

/* functions_icmgr_wsmgr.c */
DFHANDLER(upiconmgr);
DFHANDLER(downiconmgr);
DFHANDLER(lefticonmgr);
DFHANDLER(righticonmgr);
DFHANDLER(forwiconmgr);
DFHANDLER(backiconmgr);
DFHANDLER(forwmapiconmgr);
DFHANDLER(backmapiconmgr);
DFHANDLER(nexticonmgr);
DFHANDLER(previconmgr);
DFHANDLER(showiconmgr);
DFHANDLER(hideiconmgr);
DFHANDLER(sorticonmgr);

DFHANDLER(showworkspacemgr);
DFHANDLER(hideworkspacemgr);
DFHANDLER(toggleworkspacemgr);
DFHANDLER(togglestate);
DFHANDLER(setbuttonsstate);
DFHANDLER(setmapstate);


/* functions_win_moveresize.c */
DFHANDLER(move);
DFHANDLER(forcemove);
DFHANDLER(movepack);
DFHANDLER(movepush);
DFHANDLER(pack);
DFHANDLER(jumpleft);
DFHANDLER(jumpright);
DFHANDLER(jumpdown);
DFHANDLER(jumpup);
DFHANDLER(resize);
DFHANDLER(zoom);
DFHANDLER(horizoom);
DFHANDLER(fullzoom);
DFHANDLER(fullscreenzoom);
DFHANDLER(leftzoom);
DFHANDLER(rightzoom);
DFHANDLER(topzoom);
DFHANDLER(bottomzoom);
DFHANDLER(xhorizoom);
DFHANDLER(xfullzoom);
DFHANDLER(xfullscreenzoom);
DFHANDLER(xleftzoom);
DFHANDLER(xrightzoom);
DFHANDLER(xtopzoom);
DFHANDLER(xbottomzoom);
DFHANDLER(xzoom);
DFHANDLER(fill);
DFHANDLER(initsize);
DFHANDLER(moveresize);
DFHANDLER(changesize);
DFHANDLER(savegeometry);
DFHANDLER(restoregeometry);


/* functions_workspaces.c */
DFHANDLER(occupy);
DFHANDLER(occupyall);
DFHANDLER(addtoworkspace);
DFHANDLER(removefromworkspace);
DFHANDLER(toggleoccupation);
DFHANDLER(vanish);
DFHANDLER(warphere);
DFHANDLER(movetonextworkspace);
DFHANDLER(movetoprevworkspace);
DFHANDLER(movetonextworkspaceandfollow);
DFHANDLER(movetoprevworkspaceandfollow);
DFHANDLER(gotoworkspace);
DFHANDLER(prevworkspace);
DFHANDLER(nextworkspace);
DFHANDLER(rightworkspace);
DFHANDLER(leftworkspace);
DFHANDLER(upworkspace);
DFHANDLER(downworkspace);


#ifdef CAPTIVE
/* functions_captive.c */
DFHANDLER(adoptwindow);
DFHANDLER(hypermove);
#endif


/* functions_identify.c */
DFHANDLER(identify);
DFHANDLER(version);


/* functions_win.c */
DFHANDLER(autoraise);
DFHANDLER(autolower);
DFHANDLER(raise);
DFHANDLER(raiseorsqueeze);
DFHANDLER(lower);
DFHANDLER(raiselower);
DFHANDLER(tinyraise);
DFHANDLER(tinylower);
DFHANDLER(circleup);
DFHANDLER(circledown);
DFHANDLER(deiconify);
DFHANDLER(iconify);
DFHANDLER(popup);
DFHANDLER(focus);
DFHANDLER(unfocus);
DFHANDLER(delete);
DFHANDLER(destroy);
DFHANDLER(deleteordestroy);
DFHANDLER(priorityswitching);
DFHANDLER(switchpriority);
DFHANDLER(setpriority);
DFHANDLER(changepriority);
DFHANDLER(saveyourself);
DFHANDLER(colormap);
DFHANDLER(refresh);
DFHANDLER(winrefresh);
DFHANDLER(squeeze);
DFHANDLER(unsqueeze);
DFHANDLER(movetitlebar);


/* functions_warp.c */
DFHANDLER(warpto);
DFHANDLER(warptoiconmgr);
DFHANDLER(ring);
DFHANDLER(warpring);
DFHANDLER(winwarp);


/* functions_misc.c */
DFHANDLER(startanimation);
DFHANDLER(stopanimation);
DFHANDLER(speedupanimation);
DFHANDLER(slowdownanimation);
DFHANDLER(menu);
DFHANDLER(pin);
DFHANDLER(altkeymap);
DFHANDLER(altcontext);
DFHANDLER(quit);
DFHANDLER(restart);
DFHANDLER(beep);
DFHANDLER(trace);
#ifdef WINBOX
DFHANDLER(fittocontent);
#endif
DFHANDLER(showbackground);
DFHANDLER(raiseicons);
DFHANDLER(rescuewindows);
DFHANDLER(warptoscreen);
#ifdef SOUNDS
DFHANDLER(togglesound);
DFHANDLER(rereadsounds);
#endif
DFHANDLER(exec);



/*
 * Extra exported from functions_icmgr_wsmgr.c for use in
 * f.delete{,ordestroy}.
 */
void HideIconManager(void);


/* Several different sections of window handling need this */
extern Time last_time;

/* Several places need to frob this to leave the cursor alone */
extern bool func_reset_cursor;

#endif /* _CTWM_FUNCTIONS_INTERNAL_H */
