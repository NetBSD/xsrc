/* $Xconsortium: $ */





/* $XFree86: xc/programs/Xserver/hw/xfree86/XF86Setup/tclmisc.h,v 3.1 1996/06/30 10:44:12 dawes Exp $ */

#include <X11/Intrinsic.h>
#include <X11/Xmd.h>
#include <X11/extensions/xf86misc.h>
#include <tcl.h>
#include <tk.h>

int TCL_XF86MiscQueryVersion(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86MiscQueryExtension(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86MiscGetSaver(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86MiscSetSaver(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86MiscGetKbdSettings(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86MiscSetKbdSettings(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86MiscGetMouseSettings(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int TCL_XF86MiscSetMouseSettings(
#if NeedNestedPrototypes
    ClientData	clientData,
    Tcl_Interp	*interp,
    int		argc,
    char	**argv
#endif
);

int StrCaseCmp(
#if NeedNestedPrototypes
    char	*string1,
    char	*string2
#endif
);

