/* $XFree86: xc/programs/Xserver/GL/glx/global.c,v 1.2 1999/06/14 07:31:24 dawes Exp $ */
/*
** The contents of this file are subject to the GLX Public License Version 1.0
** (the "License"). You may not use this file except in compliance with the
** License. You may obtain a copy of the License at Silicon Graphics, Inc.,
** attn: Legal Services, 2011 N. Shoreline Blvd., Mountain View, CA 94043
** or at http://www.sgi.com/software/opensource/glx/license.html.
**
** Software distributed under the License is distributed on an "AS IS"
** basis. ALL WARRANTIES ARE DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY
** IMPLIED WARRANTIES OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
** PURPOSE OR OF NON- INFRINGEMENT. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Software is GLX version 1.2 source code, released February,
** 1999. The developer of the Original Software is Silicon Graphics, Inc.
** Those portions of the Subject Software created by Silicon Graphics, Inc.
** are Copyright (c) 1991-9 Silicon Graphics, Inc. All Rights Reserved.
**
** $SGI$
*/

#define NEED_REPLIES
#include "glxserver.h"

/*
** The last context used by the server.  It is the context that is current
** from the server's perspective.
*/
__GLXcontext *__glXLastContext;

/*
** X resources.
*/
RESTYPE __glXContextRes;
RESTYPE __glXClientRes;
RESTYPE __glXPixmapRes;

/*
** Error codes with the extension error base already added in.
*/
int __glXBadContext, __glXBadContextState, __glXBadDrawable, __glXBadPixmap;
int __glXBadContextTag, __glXBadCurrentWindow;
int __glXBadRenderRequest, __glXBadLargeRequest;
int __glXUnsupportedPrivateRequest;

/*
** Reply for most singles.
*/
xGLXSingleReply __glXReply;

/*
** A set of state for each client.  The 0th one is unused because client
** indices start at 1, not 0.
*/
__GLXclientState *__glXClients[MAXCLIENTS+1];

