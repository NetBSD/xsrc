/* $TOG: panoramiXext.h /main/3 1998/02/13 13:08:51 kaleb $ */
/****************************************************************
*                                                               *
*    Copyright (c) Digital Equipment Corporation, 1991, 1997    *
*                                                               *
*   All Rights Reserved.  Unpublished rights  reserved  under   *
*   the copyright laws of the United States.                    *
*                                                               *
*   The software contained on this media  is  proprietary  to   *
*   and  embodies  the  confidential  technology  of  Digital   *
*   Equipment Corporation.  Possession, use,  duplication  or   *
*   dissemination of the software and media is authorized only  *
*   pursuant to a valid written license from Digital Equipment  *
*   Corporation.                                                *
*                                                               *
*   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
*   by the U.S. Government is subject to restrictions  as  set  *
*   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
*   or  in  FAR 52.227-19, as applicable.                       *
*                                                               *
*****************************************************************/
/*  
 *	PanoramiX definitions
 */
/* $XFree86: xc/include/extensions/panoramiXext.h,v 3.5 2000/03/01 01:04:21 dawes Exp $ */

/* THIS IS NOT AN X PROJECT TEAM SPECIFICATION */

#define PANORAMIX_MAJOR_VERSION         1       /* current version number */
#define PANORAMIX_MINOR_VERSION         1

typedef struct {
    Window  window;         /* PanoramiX window - may not exist */
    int	    screen;
    int     State;          /* PanroamiXOff, PanoramiXOn */
    int	    width;	    /* width of this screen */
    int     height;	    /* height of this screen */
    int     ScreenCount;    /* real physical number of screens */
    XID     eventMask;      /* selected events for this client */
} XPanoramiXInfo;    

extern XPanoramiXInfo *XPanoramiXAllocInfo (
#if NeedFunctionPrototypes
    void
#endif
);        
