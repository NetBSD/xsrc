/* $XFree86: xc/programs/Xserver/hw/xfree86/XF86Setup/tclvidmode.h,v 3.1 1996/06/30 10:44:15 dawes Exp $ */

#include <X11/Intrinsic.h>
#include <X11/Xmd.h>
#include <X11/extensions/xf86vmode.h>
#include <tcl.h>
#include <tk.h>

int TCL_XF86VidModeQueryVersion(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86VidModeQueryExtension(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86VidModeGetModeLine(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86VidModeGetAllModeLines(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86VidModeGetMonitor(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86VidModeLockModeSwitch(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86VidModeSwitchMode(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);
