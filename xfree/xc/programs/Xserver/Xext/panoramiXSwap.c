/* $TOG: panoramiXSwap.c /main/1 1997/10/29 13:26:35 kaleb $ */
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
/* $XFree86: xc/programs/Xserver/Xext/panoramiXSwap.c,v 3.6 2000/02/27 23:15:30 mvojkovi Exp $ */

#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "cursor.h"
#include "cursorstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "gc.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "window.h"
#include "windowstr.h"
#include "pixmapstr.h"
#if 0
#include <sys/workstation.h>
#include <X11/Xserver/ws.h> 
#endif
#include "panoramiX.h"
#include "panoramiXproto.h"
#include "panoramiXsrv.h"
#include "globals.h"


/*
 *	External references for data variables
 */

extern WindowPtr *WindowTable;
extern int defaultBackingStore;
extern char *ConnectionInfo;
extern int connBlockScreenStart;
extern int (* ProcVector[256]) ();

#if NeedFunctionPrototypes
#define PROC_EXTERN(pfunc)      extern int pfunc(ClientPtr)
#else
#define PROC_EXTERN(pfunc)      extern int pfunc()
#endif     

PROC_EXTERN(ProcPanoramiXQueryVersion);
PROC_EXTERN(ProcPanoramiXGetState);
PROC_EXTERN(ProcPanoramiXGetScreenCount);
PROC_EXTERN(ProcPanoramiXGetScreenSize);

PROC_EXTERN(ProcXineramaIsActive);
PROC_EXTERN(ProcXineramaQueryScreens);

static int
SProcPanoramiXQueryVersion (ClientPtr client)
{
	REQUEST(xPanoramiXQueryVersionReq);
	register int n;

	swaps(&stuff->length,n);
	REQUEST_SIZE_MATCH (xPanoramiXQueryVersionReq);
	return ProcPanoramiXQueryVersion(client);
}

static int
SProcPanoramiXGetState(ClientPtr client)
{
	REQUEST(xPanoramiXGetStateReq);
	register int n;

 	swaps (&stuff->length, n);	
	REQUEST_SIZE_MATCH(xPanoramiXGetStateReq);
	return ProcPanoramiXGetState(client);
}

static int 
SProcPanoramiXGetScreenCount(ClientPtr client)
{
	REQUEST(xPanoramiXGetScreenCountReq);
	register int n;

	swaps (&stuff->length, n);
	REQUEST_SIZE_MATCH(xPanoramiXGetScreenCountReq);
	return ProcPanoramiXGetScreenCount(client);
}

static int 
SProcPanoramiXGetScreenSize(ClientPtr client)
{
	REQUEST(xPanoramiXGetScreenSizeReq);
	register int n;

	swaps (&stuff->length, n);
	REQUEST_SIZE_MATCH(xPanoramiXGetScreenSizeReq);
	return ProcPanoramiXGetScreenSize(client);
}


static int 
SProcXineramaIsActive(ClientPtr client)
{
	REQUEST(xXineramaIsActiveReq);
	register int n;

	swaps (&stuff->length, n);
	REQUEST_SIZE_MATCH(xXineramaIsActiveReq);
	return ProcXineramaIsActive(client);
}


static int 
SProcXineramaQueryScreens(ClientPtr client)
{
	REQUEST(xXineramaQueryScreensReq);
	register int n;

	swaps (&stuff->length, n);
	REQUEST_SIZE_MATCH(xXineramaQueryScreensReq);
	return ProcXineramaQueryScreens(client);
}


int
SProcPanoramiXDispatch (ClientPtr client)
{   REQUEST(xReq);
    switch (stuff->data)
    {
	case X_PanoramiXQueryVersion:
	     return SProcPanoramiXQueryVersion(client);
	case X_PanoramiXGetState:
	     return SProcPanoramiXGetState(client);
	case X_PanoramiXGetScreenCount:
	     return SProcPanoramiXGetScreenCount(client);
	case X_PanoramiXGetScreenSize:
	     return SProcPanoramiXGetScreenSize(client);
	case X_XineramaIsActive:
	     return SProcXineramaIsActive(client);
	case X_XineramaQueryScreens:
	     return SProcXineramaQueryScreens(client);
    }
    return BadRequest;
}
